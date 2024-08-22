/* SPDX-License-Identifier: Apache-2.0 */
#include "sysfs.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <phosphor-logging/lg2.hpp>

#include <fstream>
#include <utility>

std::expected<uint32_t, int>
    SysFSImpl::read(const std::filesystem::path& file) const
{
    std::error_code e{};
    if (!std::filesystem::exists(file, e))
    {
        lg2::error("Read: Scratch register file {FILE} doesn't exist!", "FILE",
                   file);
        return std::unexpected{ENOENT};
    }

    std::ifstream stream{file};
    if (!stream.is_open())
    {
        auto err = errno;
        lg2::error("Read: Opening file {FILE} failed with errno {ERRNO}",
                   "FILE", file, "ERRNO", err);
        return std::unexpected{err};
    }

    // The sysfs file returns a string that needs to be converted to a uint32_t.
    std::string stringVal;
    stream >> stringVal;

    if (stream.fail())
    {
        auto err = errno;
        lg2::error("Read of {FILE} failed with errno {ERRNO}", "FILE", file,
                   "ERRNO", err);
        return std::unexpected{err};
    }

    uint32_t value{};

    auto [_, ec] =
        std::from_chars(&*stringVal.begin(), &*stringVal.end(), value, 16);
    if (ec != std::errc())
    {
        lg2::error("Could not extract value from {STRING}, error = {ERROR}",
                   "STRING", stringVal, "ERROR", std::to_underlying(ec));
        return std::unexpected{std::to_underlying(ec)};
    }

    return value;
}

struct Closer
{
    explicit Closer(int fd) : fd(fd) {}
    ~Closer()
    {
        if (fd != -1)
        {
            close(fd);
        }
    }

    int fd = -1;
};

int SysFSImpl::write(const std::filesystem::path& file, uint32_t value) const
{
    std::error_code e{};
    if (!std::filesystem::exists(file, e))
    {
        lg2::error("Write: Scratch register file {FILE} doesn't exist!", "FILE",
                   file);
        return ENOENT;
    }

    // Note: fstreams don't work here.

    int fd = open(file.c_str(), O_WRONLY, O_SYNC);
    if (fd < 0)
    {
        auto err = errno;
        lg2::error("open: Writing file {FILE} failed with errno {ERRNO}",
                   "FILE", file, "ERRNO", err);
        return fd;
    }

    Closer c{fd};

    ssize_t rc = ::write(fd, &value, sizeof(value));
    if (rc < 0)
    {
        auto err = errno;
        lg2::error("Writing FILE {FILE} failed with errno {ERRNO}", "FILE",
                   file, "ERRNO", err);
        return static_cast<int>(rc);
    }

    return 0;
}
