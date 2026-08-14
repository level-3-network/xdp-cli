// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>
#include <thread>
#include <chrono>

#include "sdk/sdk_common.hxx"

#include "controller/xdp.hxx"
#include "controller/command_parser.hxx"

SDKCommon* sdk_common;

XDPController* xdp_controller;
CommandParserController* command_parser_controller;

int main(int number_of_arguments, char* arguments[])
{
    sdk_common = new SDKCommon();

    if (sdk_common == nullptr)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to initialize class [SDKCommon]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return EXIT_FAILURE;
    }

    xdp_controller = new XDPController();

    if (xdp_controller == nullptr)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to initialize class [XDPController]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        delete sdk_common;

        return EXIT_FAILURE;
    }

    command_parser_controller = new CommandParserController();

    if (command_parser_controller == nullptr)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to initialize class [CommandParserController]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        delete xdp_controller;

        delete sdk_common;

        return EXIT_FAILURE;
    }

    sdk_common->initialize_application_arguments(number_of_arguments, arguments);

    const auto in_interface = sdk_common->get_application_argument_value("--in-interface", "");
    const auto out_interface = sdk_common->get_application_argument_value("--out-interface", "");

    if (in_interface.empty())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Application argument [--in-interface] cannot be empty" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        delete xdp_controller;
        delete command_parser_controller;

        delete sdk_common;

        return EXIT_FAILURE;
    }

    if (out_interface.empty())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Application argument [--out-interface] cannot be empty" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        delete xdp_controller;
        delete command_parser_controller;

        delete sdk_common;

        return EXIT_FAILURE;
    }

    if (!xdp_controller->initialize(in_interface.c_str(), out_interface.c_str()))
    {
        delete xdp_controller;
        delete command_parser_controller;

        delete sdk_common;

        return EXIT_FAILURE;
    }

    command_parser_controller->set_forward_on_initialize(xdp_controller->forward_command_parser_initialize);
    command_parser_controller->set_forward_on_destroy(xdp_controller->forward_command_parser_destroy);

    command_parser_controller->initialize();

    command_parser_controller->destroy();
    xdp_controller->destroy();

    delete xdp_controller;
    delete command_parser_controller;

    delete sdk_common;

    return EXIT_SUCCESS;
}
