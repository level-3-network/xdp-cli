// SPDX-License-Identifier: GPL-2.0-only

#include "xdp_cli/program_core.hxx"

extern "C"
{
    SDK_Common* sdk_common;

    Controller_PluginLoader* controller_plugin_loader;
    Controller_CommandParser* controller_command_parser;
    Controller_XDP* controller_xdp;
}

bool is_running;

void signal_handler(int signal) {
    is_running = false;
}

void delete_classes()
{
    if (sdk_common != nullptr) {
        delete sdk_common;
    }

    if (controller_plugin_loader != nullptr) {
        delete controller_plugin_loader;
    }

    if (controller_command_parser != nullptr) {
        delete controller_command_parser;
    }

    if (controller_xdp != nullptr) {
        delete controller_xdp;
    }
}

int main(int number_of_arguments, char* arguments[])
{
    is_running = true;

    sdk_common = new SDK_Common();

    if (sdk_common == nullptr) {
        return EXIT_FAILURE;
    }

    controller_plugin_loader = new Controller_PluginLoader();

    if (controller_plugin_loader == nullptr) {
        return EXIT_FAILURE;
    }

    controller_command_parser = new Controller_CommandParser();

    if (controller_command_parser == nullptr) {
        return EXIT_FAILURE;
    }

    controller_xdp = new Controller_XDP();

    if (controller_xdp == nullptr) {
        return EXIT_FAILURE;
    }

    sdk_common->initialize_program_arguments(number_of_arguments, arguments);

    const auto wan_interface = sdk_common->get_program_argument_value("--wan-interface", "");
    const auto lan_interface = sdk_common->get_program_argument_value("--lan-interface", "");

    if (wan_interface.empty())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Program argument [--wan-interface] cannot be empty" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        delete_classes();

        return EXIT_FAILURE;
    }

    if (lan_interface.empty())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Program argument [--lan-interface] cannot be empty" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        delete_classes();

        return EXIT_FAILURE;
    }

    controller_plugin_loader->initialize();

    if (std::filesystem::exists("plugins"))
    {
        int loaded_plugins = 0;

        for (const auto& entry : std::filesystem::directory_iterator("plugins"))
        {
            if (entry.path().extension() == ".so")
            {
                std::string path = entry.path().string();

                if (controller_plugin_loader->load_plugin(path) != nullptr) {
                    loaded_plugins++;
                }

                else {
                    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to load plugin [" << path << "]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
                }
            }
        }

        if (loaded_plugins > 0) {
            std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Loaded [" << loaded_plugins << "] plugin(s)" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
        }
    }

    controller_command_parser->initialize();

    controller_command_parser->register_command("/exit", {"exit/quit the program"}, [&] (const std::vector<std::string>) {
        is_running = false;
    });

    controller_command_parser->register_command("/quit", {"exit/quit the program"}, [&] (const std::vector<std::string>) {
        is_running = false;
    });

    if (!controller_xdp->initialize(wan_interface, lan_interface))
    {
        delete_classes();

        return EXIT_FAILURE;
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    while (is_running)
    {
        if (!controller_command_parser->loop()) {
            continue;
        }
    }

    controller_command_parser->destroy();
    controller_xdp->destroy();
    controller_plugin_loader->destroy();

    delete_classes();

    return EXIT_SUCCESS;
}
