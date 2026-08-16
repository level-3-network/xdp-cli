// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>
#include <thread>
#include <chrono>

#include "xdp_cli/sdk_common.hxx"

#include "xdp_cli/controller_xdp.hxx"
#include "xdp_cli/controller_command_parser.hxx"

extern "C"
{
    SDK_Common* sdk_common;

    Controller_XDP* controller_xdp;
    Controller_CommandParser* controller_command_parser;
}

void delete_classes()
{
    if (controller_command_parser != nullptr) {
        delete controller_command_parser;
    }

    if (controller_xdp != nullptr) {
        delete controller_xdp;
    }

    if (sdk_common != nullptr) {
        delete sdk_common;
    }
}

int main(int number_of_arguments, char* arguments[])
{
    sdk_common = new SDK_Common();

    if (sdk_common == nullptr)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to initialize class [SDK_Common]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return EXIT_FAILURE;
    }

    controller_xdp = new Controller_XDP();

    if (controller_xdp == nullptr)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to initialize class [Controller_XDP]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        delete_classes();

        return EXIT_FAILURE;
    }

    controller_xdp->set_sdk_common(sdk_common);

    controller_command_parser = new Controller_CommandParser();

    if (controller_command_parser == nullptr)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to initialize class [Controller_CommandParser]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        delete_classes();

        return EXIT_FAILURE;
    }

    controller_xdp->set_controller_command_parser(controller_command_parser);

    sdk_common->initialize_application_arguments(number_of_arguments, arguments);

    const auto in_interface = sdk_common->get_application_argument_value("--in-interface", "");
    const auto out_interface = sdk_common->get_application_argument_value("--out-interface", "");

    if (in_interface.empty())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Argument [--in-interface] cannot be empty" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        delete_classes();

        return EXIT_FAILURE;
    }

    if (out_interface.empty())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Argument [--out-interface] cannot be empty" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        delete_classes();

        return EXIT_FAILURE;
    }

    if (!controller_xdp->initialize(in_interface.c_str(), out_interface.c_str()))
    {
        delete_classes();

        return EXIT_FAILURE;
    }

    controller_command_parser->hook_initialize(controller_xdp->hook_command_parser_initialize);
    controller_command_parser->hook_destroy(controller_xdp->hook_command_parser_destroy);

    controller_command_parser->initialize();

    controller_xdp->destroy();
    controller_command_parser->destroy();

    delete_classes();

    return EXIT_SUCCESS;
}
