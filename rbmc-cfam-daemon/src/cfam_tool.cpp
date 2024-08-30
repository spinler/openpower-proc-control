/* SPDX-License-Identifier: Apache-2.0 */
#include "local_cfam.hpp"

#include <CLI/CLI.hpp>

#include <format>
#include <iostream>
#include <string_view>

void displayCFAMFields(std::string_view name, size_t link)
{
    SysFSImpl sysfs;
    LocalCFAM cfam{link, sysfs};

    std::cout << std::format("{} CFAM scratchpad registers\n", name);

    for (const auto& [field, meta] : LocalCFAM::cfamFields)
    {
        auto data = cfam.readField(field);
        if (!data.has_value())
        {
            std::cerr << std::format("Unable to read {} CFAM ({})\n", name,
                                     data.error());
            return;
        }

        std::cout << std::format("{:20} {:#x}\n", meta.desc, data.value());
    }
}

void displayCFAMs()
{
    displayCFAMFields("Local BMC", 0);
    std::cout << '\n';
    displayCFAMFields("Sibling BMC", 1);
}

int main(int argc, char** argv)
{
    CLI::App app{"CFAM Tool"};
    bool display{};

    app.add_flag("-d", display, "Display CFAM Scratch Pad Register Contents");

    CLI11_PARSE(app, argc, argv);

    if (display)
    {
        displayCFAMs();
    }
    else
    {
        std::cout << app.help();
    }

    return 0;
}
