/* SPDX-License-Identifier: Apache-2.0 */
#include "sysfs.hpp"

#include <format>
#include <fstream>

#include <gtest/gtest.h>

class SysFSTest : public ::testing::Test
{
  public:
    static void SetUpTestCase()
    {
        char d[] = "/tmp/sysfstestXXXXXX";
        dir = mkdtemp(d);
    }

    static void TearDownTestCase()
    {
        std::filesystem::remove_all(dir);
    }

    void initFile(std::string_view name, uint32_t value)
    {
        // Writes 8 bytes so that a read will work:
        // $ hexdump -C scratch1
        // 00000000  31 32 33 34 35 36 37 38    |12345678|
        auto path = dir / name;
        std::ofstream file{path};
        file << std::hex << value;
    }

    uint32_t getWriteData(std::string_view name)
    {
        // A write only writes 4 raw bytes, so get that.
        FILE* fp = fopen(name.data(), "r");
        uint32_t value{};
        auto rc = fread(&value, 4, 1, fp);
        fclose(fp);

        if (rc != 1)
        {
            throw std::runtime_error(
                std::format("reading {} returned {}", name, rc));
        }

        return value;
    }

    static std::filesystem::path dir;
};

std::filesystem::path SysFSTest::dir;

TEST_F(SysFSTest, ReadTest)
{
    SysFSImpl sysfs;

    SysFSTest::initFile("scratch1", 0x12345678);

    EXPECT_EQ(sysfs.read(SysFSTest::dir / "scratch1"), 0x12345678);

    // Doesn't exist
    EXPECT_EQ(sysfs.read(SysFSTest::dir / "scratch2"),
              std::unexpected<int>{ENOENT});
}

TEST_F(SysFSTest, WriteTest)
{
    SysFSImpl sysfs;

    SysFSTest::initFile("scratch1", 0x0000);

    auto path = SysFSTest::dir / "scratch1";
    EXPECT_EQ(sysfs.write(path, 0xA000000B), 0);

    EXPECT_EQ(getWriteData(path.c_str()), 0xA000000B);

    // Doesn't exist
    path = SysFSTest::dir / "scratch2";
    EXPECT_EQ(sysfs.write(path, 0xA000000B), ENOENT);
}
