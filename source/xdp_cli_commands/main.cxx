// SPDX-License-Identifier: GPL-2.0-only

#include "xdp_cli/program_core.hxx"

#define EXPORT_LIBRARY extern "C" __attribute__((visibility("default")))

extern "C"
{
    SDK_Common* sdk_common;

    Controller_PluginLoader* controller_plugin_loader;
    Controller_CommandParser* controller_command_parser;
    Controller_XDP* controller_xdp;
}

EXPORT_LIBRARY Controller_PluginLoader_HookResult hook_command_parser_initialize()
{
    controller_command_parser->register_command("/tx_port_redirect_map", {"<true|false>"}, [] (const std::vector<std::string> arguments)
    {
        if (arguments.size() == 1)
        {
            if (arguments.at(0) == "true")
            {
                controller_xdp->data_plane_bpf_skeleton_->bss->tx_port_redirect_map = 1;

                std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Now using TX port map to redirect interfaces" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
            }

            else if (arguments.at(0) == "false")
            {
                controller_xdp->data_plane_bpf_skeleton_->bss->tx_port_redirect_map = 0;

                std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Now passing packets to kernel" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
            }
        }
    });

    controller_command_parser->register_command("/map_ip_address_key_reverse_lookup", {"<true|false>"}, [] (const std::vector<std::string> arguments)
    {
        if (arguments.size() == 1)
        {
            if (arguments.at(0) == "true")
            {
                controller_xdp->data_plane_bpf_skeleton_->bss->map_ip_address_key_reverse_lookup = 1;

                std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Reversed map IP address key lookup" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
            }

            else if (arguments.at(0) == "false")
            {
                controller_xdp->data_plane_bpf_skeleton_->bss->map_ip_address_key_reverse_lookup = 0;

                std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Reversed map IP address key lookup" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
            }
        }
    });

    controller_command_parser->register_command("/blacklist", {"<enable|disable> <ipv4|ipv6> <ip address> <prefix length>"}, [] (const std::vector<std::string> arguments)
    {
        if (arguments.size() == 4)
        {
            if (arguments.at(0) == "enable")
            {
                if (arguments.at(1) == "ipv4")
                {
                    if (!sdk_common->is_numeric_string(arguments.at(3)))
                    {
                        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                        return;
                    }

                    controller_xdp->update_blacklist_ipv4_addresses_map(arguments.at(2).c_str(), std::stoi(arguments.at(3)), true);

                    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Updated blacklist IPv4 addresses map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
                }

                else if (arguments.at(1) == "ipv6")
                {
                    if (!sdk_common->is_numeric_string(arguments.at(3)))
                    {
                        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                        return;
                    }

                    controller_xdp->update_blacklist_ipv6_addresses_map(arguments.at(2).c_str(), std::stoi(arguments.at(3)), true);

                    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Updated blacklist IPv6 addresses map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
                }
            }

            else if (arguments.at(0) == "disable")
            {
                if (arguments.at(1) == "ipv4")
                {
                    if (!sdk_common->is_numeric_string(arguments.at(3)))
                    {
                        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                        return;
                    }

                    controller_xdp->update_blacklist_ipv4_addresses_map(arguments.at(2).c_str(), std::stoi(arguments.at(3)), false);

                    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Updated blacklist IPv4 addresses map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
                }

                else if (arguments.at(1) == "ipv6")
                {
                    if (!sdk_common->is_numeric_string(arguments.at(3)))
                    {
                        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                        return;
                    }

                    controller_xdp->update_blacklist_ipv6_addresses_map(arguments.at(2).c_str(), std::stoi(arguments.at(3)), false);

                    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Updated blacklist IPv6 addresses map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
                }
            }
        }
    });

    controller_command_parser->register_command("/traffic_shaper", {"<ipv4|ipv6> <ip address> <prefix length> <rate mbps> <burst mb> <allowance mb>"}, [] (const std::vector<std::string> arguments)
    {
        if (arguments.size() == 6)
        {
            if (arguments.at(0) == "ipv4")
            {
                if (!sdk_common->is_numeric_string(arguments.at(2)) || !sdk_common->is_numeric_string(arguments.at(3)) || !sdk_common->is_numeric_string(arguments.at(4)) || !sdk_common->is_numeric_string(arguments.at(5)))
                {
                    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                    return;
                }

                controller_xdp->update_traffic_shaper_ipv4_addresses_map(arguments.at(1).c_str(), std::stoi(arguments.at(2)), std::stoull(arguments.at(3)), std::stoull(arguments.at(4)), std::stoull(arguments.at(5)));

                std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Updated traffic shaper IPv4 addresses map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
            }

            else if (arguments.at(0) == "ipv6")
            {
                if (!sdk_common->is_numeric_string(arguments.at(2)) || !sdk_common->is_numeric_string(arguments.at(3)) || !sdk_common->is_numeric_string(arguments.at(4)) || !sdk_common->is_numeric_string(arguments.at(5)))
                {
                    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                    return;
                }

                controller_xdp->update_traffic_shaper_ipv6_addresses_map(arguments.at(1).c_str(), std::stoi(arguments.at(2)), std::stoull(arguments.at(3)), std::stoull(arguments.at(4)), std::stoull(arguments.at(5)));

                std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Updated traffic shaper IPv6 addresses map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
            }
        }
    });

    controller_xdp->reload_command_list_file("./command_list.txt");

    controller_command_parser->register_command("/reload", {"reload command list file"}, [] (const std::vector<std::string> arguments) {
        controller_xdp->reload_command_list_file("./command_list.txt");
    });

    return Controller_PluginLoader_HookResult::kHookIgnored;
}

EXPORT_LIBRARY Controller_PluginLoader_HookResult hook_command_parser_destroy()
{
    controller_command_parser->unregister_command("/tx_port_redirect_map");
    controller_command_parser->unregister_command("/map_ip_address_key_reverse_lookup");
    controller_command_parser->unregister_command("/blacklist");
    controller_command_parser->unregister_command("/traffic_shaper");
    controller_command_parser->unregister_command("/reload");

    return Controller_PluginLoader_HookResult::kHookIgnored;
}
