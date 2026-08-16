// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <net/if.h>

#include <bpf/libbpf.h>
#include <bpf/bpf.h>

#include "xdp_cli/sdk_common.hxx"
#include "xdp_cli/sdk_bpf.h"

#include "xdp_cli/data_plane.skel.h"

#include "xdp_cli/controller_xdp.hxx"
#include "xdp_cli/controller_command_parser.hxx"

void Controller_XDP::hook_command_parser_initialize()
{
    instance_->controller_command_parser_->register_command("/tx_port_redirect_map", {"<true|false>"}, [] (const std::vector<std::string> arguments)
    {
        if (arguments.size() == 1)
        {
            if (arguments.at(0) == "true")
            {
                instance_->data_plane_bpf_skeleton_->bss->tx_port_redirect_map = 1;

                std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Now using TX port map to redirect interfaces" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
            }

            else if (arguments.at(0) == "false")
            {
                instance_->data_plane_bpf_skeleton_->bss->tx_port_redirect_map = 0;

                std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Now passing packets to kernel" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
            }
        }
    });

    instance_->controller_command_parser_->register_command("/map_ip_address_key_reverse_lookup", {"<true|false>"}, [] (const std::vector<std::string> arguments)
    {
        if (arguments.size() == 1)
        {
            if (arguments.at(0) == "true")
            {
                instance_->data_plane_bpf_skeleton_->bss->map_ip_address_key_reverse_lookup = 1;

                std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Reversed map IP address key lookup" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
            }

            else if (arguments.at(0) == "false")
            {
                instance_->data_plane_bpf_skeleton_->bss->map_ip_address_key_reverse_lookup = 0;

                std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Reversed map IP address key lookup" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
            }
        }
    });

    instance_->controller_command_parser_->register_command("/blacklist", {"<enable|disable> <ipv4|ipv6> <ip address> <prefix length>"}, [] (const std::vector<std::string> arguments)
    {
        if (arguments.size() == 4)
        {
            if (arguments.at(0) == "enable")
            {
                if (arguments.at(1) == "ipv4")
                {
                    if (!instance_->sdk_common_->is_numeric_string(arguments.at(3)))
                    {
                        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                        return;
                    }

                    instance_->update_blacklist_ipv4_addresses_map(arguments.at(2).c_str(), std::stoi(arguments.at(3)), true);
                }

                else if (arguments.at(1) == "ipv6")
                {
                    if (!instance_->sdk_common_->is_numeric_string(arguments.at(3)))
                    {
                        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                        return;
                    }

                    instance_->update_blacklist_ipv6_addresses_map(arguments.at(2).c_str(), std::stoi(arguments.at(3)), true);
                }
            }

            else if (arguments.at(0) == "disable")
            {
                if (arguments.at(1) == "ipv4")
                {
                    if (!instance_->sdk_common_->is_numeric_string(arguments.at(3)))
                    {
                        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                        return;
                    }

                    instance_->update_blacklist_ipv4_addresses_map(arguments.at(2).c_str(), std::stoi(arguments.at(3)), false);
                }

                else if (arguments.at(1) == "ipv6")
                {
                    if (!instance_->sdk_common_->is_numeric_string(arguments.at(3)))
                    {
                        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                        return;
                    }

                    instance_->update_blacklist_ipv6_addresses_map(arguments.at(2).c_str(), std::stoi(arguments.at(3)), false);
                }
            }
        }
    });

    instance_->controller_command_parser_->register_command("/traffic_shaper", {"<ipv4|ipv6> <ip address> <prefix length> <rate mbps> <burst mb> <allowance mb>"}, [] (const std::vector<std::string> arguments)
    {
        if (arguments.size() == 6)
        {
            if (arguments.at(0) == "ipv4")
            {
                if (!instance_->sdk_common_->is_numeric_string(arguments.at(2)) || !instance_->sdk_common_->is_numeric_string(arguments.at(3)) || !instance_->sdk_common_->is_numeric_string(arguments.at(4)) || !instance_->sdk_common_->is_numeric_string(arguments.at(5)))
                {
                    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                    return;
                }

                instance_->update_traffic_shaper_ipv4_addresses_map(arguments.at(1).c_str(), std::stoi(arguments.at(2)), std::stoull(arguments.at(3)), std::stoull(arguments.at(4)), std::stoull(arguments.at(5)));
            }

            else if (arguments.at(0) == "ipv6")
            {
                if (!instance_->sdk_common_->is_numeric_string(arguments.at(2)) || !instance_->sdk_common_->is_numeric_string(arguments.at(3)) || !instance_->sdk_common_->is_numeric_string(arguments.at(4)) || !instance_->sdk_common_->is_numeric_string(arguments.at(5)))
                {
                    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                    return;
                }

                instance_->update_traffic_shaper_ipv6_addresses_map(arguments.at(1).c_str(), std::stoi(arguments.at(2)), std::stoull(arguments.at(3)), std::stoull(arguments.at(4)), std::stoull(arguments.at(5)));
            }
        }
    });

    instance_->reload_command_list_file();

    instance_->controller_command_parser_->register_command("/reload", {"reload command list file"}, [] (const std::vector<std::string> arguments) {
        instance_->reload_command_list_file();
    });
}

void Controller_XDP::hook_command_parser_destroy()
{
    instance_->controller_command_parser_->unregister_command("/tx_port_redirect_map");
    instance_->controller_command_parser_->unregister_command("/map_ip_address_key_reverse_lookup");

    instance_->controller_command_parser_->unregister_command("/blacklist");
    instance_->controller_command_parser_->unregister_command("/traffic_shaper");

    instance_->controller_command_parser_->unregister_command("/reload");
}

bool Controller_XDP::initialize(const char* in_interface, const char* out_interface)
{
    this->running_ = true;

    for (const auto& hook : this->hooks_initialize_)
    {
        if (hook) {
            hook();
        }
    }

    this->in_if_index_ = if_nametoindex(in_interface);

    if (this->in_if_index_ == 0)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to find IN interface [" << in_interface << "]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    this->out_if_index_ = if_nametoindex(out_interface);

    if (this->out_if_index_ == 0)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to find OUT interface [" << out_interface << "]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    this->data_plane_bpf_skeleton_ = data_plane_bpf__open_and_load();

    if (!this->data_plane_bpf_skeleton_)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to open and load BPF skeleton" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    this->data_plane_bpf_skeleton_->bss->in_if_index = this->in_if_index_;

    const int tx_port_map_fd = bpf_map__fd(this->data_plane_bpf_skeleton_->maps.tx_port_map);

    if (bpf_map_update_elem(tx_port_map_fd, &this->in_if_index_, &this->out_if_index_, BPF_ANY) < 0 || bpf_map_update_elem(tx_port_map_fd, &this->out_if_index_, &this->in_if_index_, BPF_ANY) < 0)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to update TX port map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "eBPF bytecode verified and loaded successfully" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

    this->link_1_ = bpf_program__attach_xdp(this->data_plane_bpf_skeleton_->progs.xdp_main, this->in_if_index_);

    if (!link_1_)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to attach XDP program for IN interface" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    else {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "XDP program successfully attached for IN interface" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
    }

    this->link_2_ = bpf_program__attach_xdp(this->data_plane_bpf_skeleton_->progs.xdp_main, this->out_if_index_);

    if (!link_2_)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to attach XDP program for OUT interface" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    else {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "XDP program successfully attached for OUT interface" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
    }

    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Successfully initialized XDP" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

    return true;
}

void Controller_XDP::destroy()
{
    for (const auto& hook : this->hooks_destroy_)
    {
        if (hook) {
            hook();
        }
    }

    bpf_link__destroy(this->link_1_);
    bpf_link__destroy(this->link_2_);

    data_plane_bpf__destroy(this->data_plane_bpf_skeleton_);

    this->running_ = false;
}

bool Controller_XDP::reload_command_list_file()
{
    if (!this->running_) {
        return false;
    }

    const std::string path = "./command_list.txt";

    std::ifstream ifs(path);

    if (!ifs.is_open())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to open command list file" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    std::string line;

    while (std::getline(ifs, line))
    {
        const size_t first_non_space = line.find_first_not_of(" \t");

        if (first_non_space == std::string::npos || line[first_non_space] == '#') {
            continue;
        }

        if (line.empty() || line[0] == '#') {
            continue;
        }

        this->controller_command_parser_->execute_command(line);
    }

    ifs.close();

    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Reloaded command list file" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

    return true;
}

TokenBucket Controller_XDP::compute_token_bucket(const uint64_t rate_mbps, const uint64_t burst_mb)
{
    struct TokenBucket token_bucket = {};

    const uint64_t bytes_per_second = (rate_mbps * 1000000ULL) / 8ULL;
    const uint64_t burst_bytes = (burst_mb * 1000000ULL) / 8ULL;

    token_bucket.nanosecond_per_byte = 1000000000ULL / bytes_per_second;

    token_bucket.max_burst = burst_bytes * token_bucket.nanosecond_per_byte;
    token_bucket.tokens = token_bucket.max_burst;

    token_bucket.last_updated = 0;

    return token_bucket;
}

bool Controller_XDP::update_blacklist_ipv4_addresses_map(const char* ip_address, const unsigned int prefix_length, const bool enable)
{
    struct LPM_TrieKey_IPv4 key = {};

    key.prefix_length = prefix_length;

    if (inet_pton(AF_INET, ip_address, &key.ip_address) != 1)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid IPv4 address format" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    const int map_fd = bpf_map__fd(this->data_plane_bpf_skeleton_->maps.blacklist_ipv4_addresses_map);

    unsigned int value = enable ? 1 : 0;

    if (bpf_map_update_elem(map_fd, &key, &value, BPF_ANY) < 0)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to update blacklist IPv4 addresses map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    return true;
}

bool Controller_XDP::update_blacklist_ipv6_addresses_map(const char* ip_address, const unsigned int prefix_length, const bool enable)
{
    struct LPM_TrieKey_IPv6 key = {};

    key.prefix_length = prefix_length;

    if (inet_pton(AF_INET6, ip_address, &key.ip_address) != 1)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid IPv6 address format" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    const int map_fd = bpf_map__fd(this->data_plane_bpf_skeleton_->maps.blacklist_ipv6_addresses_map);

    unsigned int value = enable ? 1 : 0;

    if (bpf_map_update_elem(map_fd, &key, &value, BPF_ANY) < 0)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to update blacklist IPv6 addresses map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    return true;
}

bool Controller_XDP::update_traffic_shaper_ipv4_addresses_map(const char* ip_address, const unsigned int prefix_length, const uint64_t rate_mbps, const uint64_t burst_mb, const uint64_t allowance_mb)
{
    struct LPM_TrieKey_IPv4 key = {};

    key.prefix_length = prefix_length;

    if (inet_pton(AF_INET, ip_address, &key.ip_address) != 1)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid IPv4 address format" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    const int map_fd = bpf_map__fd(this->data_plane_bpf_skeleton_->maps.traffic_shaper_ipv4_addresses_map);

    struct TokenBucket token_bucket = this->compute_token_bucket(rate_mbps, burst_mb);

    token_bucket.allowance = allowance_mb * 1048576;

    if (bpf_map_update_elem(map_fd, &key, &token_bucket, BPF_ANY))
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to update traffic shaper IPv4 addresses map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    return true;
}

bool Controller_XDP::update_traffic_shaper_ipv6_addresses_map(const char* ip_address, const unsigned int prefix_length, const uint64_t rate_mbps, const uint64_t burst_mb, const uint64_t allowance_mb)
{
    struct LPM_TrieKey_IPv6 key = {};

    key.prefix_length = prefix_length;

    if (inet_pton(AF_INET6, ip_address, &key.ip_address) != 1)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid IPv6 address format" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    const int map_fd = bpf_map__fd(this->data_plane_bpf_skeleton_->maps.traffic_shaper_ipv6_addresses_map);

    struct TokenBucket token_bucket = this->compute_token_bucket(rate_mbps, burst_mb);

    token_bucket.allowance = allowance_mb * 1048576;

    if (bpf_map_update_elem(map_fd, &key, &token_bucket, BPF_ANY))
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to update traffic shaper IPv6 addresses map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    return true;
}
