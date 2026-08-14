// SPDX-License-Identifier: GPL-2.0-only

#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include "sdk/sdk_common.hxx"

#include <net/if.h>

#include <bpf/libbpf.h>
#include <bpf/bpf.h>

#include "sdk/sdk_bpf.h"

#include "controller/xdp.hxx"
#include "controller/command_parser.hxx"

#include "data_plane.skel.h"

extern SDKCommon* sdk_common;

extern CommandParserController* command_parser_controller;

bool XDPController::initialize(const char* in_interface, const char* out_interface)
{
    if (!this->running_) {
        return false;
    }

    this->in_if_index_ = if_nametoindex(in_interface);

    if (this->in_if_index_ == 0)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to find [IN] interface [" << in_interface << "]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    this->out_if_index_ = if_nametoindex(out_interface);

    if (this->out_if_index_ == 0)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to find [OUT] interface [" << in_interface << "]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    this->data_plane_bpf_skeleton_ = data_plane_bpf__open_and_load();

    if (!this->data_plane_bpf_skeleton_)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to open and load BPF skeleton" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "eBPF bytecode verified and loaded successfully" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

    this->link_1_ = bpf_program__attach_xdp(this->data_plane_bpf_skeleton_->progs.xdp_main, this->in_if_index_);

    if (!link_1_)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to attach XDP program [IN]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    else {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "XDP program [IN] successfully attached" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
    }

    this->link_2_ = bpf_program__attach_xdp(this->data_plane_bpf_skeleton_->progs.xdp_main, this->out_if_index_);

    if (!link_2_)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to attach XDP program [OUT]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    else {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "XDP program [OUT] successfully attached" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
    }

    return true;
}

void XDPController::destroy()
{
    bpf_link__destroy(this->link_1_);
    bpf_link__destroy(this->link_2_);

    data_plane_bpf__destroy(this->data_plane_bpf_skeleton_);

    this->running_ = false;
}

bool XDPController::reload_command_list_file()
{
    if (!this->running_) {
        return false;
    }

    const std::string path = "command_list.txt";

    std::ifstream ifs(path);

    if (!ifs.is_open())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to open file [" << path << "]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

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

        command_parser_controller->execute_command(line);
    }

    ifs.close();

    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Reloaded command list file" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

    return true;
}

void XDPController::forward_command_parser_initialize()
{
    command_parser_controller->register_command("/blacklist", {"<enable|disable> <ipv4|ipv6> <ip address> <prefix length>"}, [] (const std::vector<std::string> arguments)
    {
        if (arguments.size() == 4)
        {
            if (arguments.at(0) == "enable")
            {
                if (arguments.at(1) == "ipv4")
                {
                    if (!sdk_common->is_numeric_string(arguments.at(3)))
                    {
                        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                        return;
                    }

                    instance_->update_blacklist_ipv4_address(arguments.at(2).c_str(), std::stoi(arguments.at(3)), true);
                }

                else if (arguments.at(1) == "ipv6")
                {
                    if (!sdk_common->is_numeric_string(arguments.at(3)))
                    {
                        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                        return;
                    }

                    instance_->update_blacklist_ipv6_address(arguments.at(2).c_str(), std::stoi(arguments.at(3)), true);
                }
            }

            else if (arguments.at(0) == "disable")
            {
                if (arguments.at(1) == "ipv4")
                {
                    if (!sdk_common->is_numeric_string(arguments.at(3)))
                    {
                        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                        return;
                    }

                    instance_->update_blacklist_ipv4_address(arguments.at(2).c_str(), std::stoi(arguments.at(3)), false);
                }

                else if (arguments.at(1) == "ipv6")
                {
                    if (!sdk_common->is_numeric_string(arguments.at(3)))
                    {
                        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                        return;
                    }

                    instance_->update_blacklist_ipv6_address(arguments.at(2).c_str(), std::stoi(arguments.at(3)), false);
                }
            }
        }
    });

    command_parser_controller->register_command("/traffic_shaper", {"<update> <ipv4|ipv6> <ip address> <prefix length> <rate mbps> <burst mb>"}, [] (const std::vector<std::string> arguments)
    {
        if (arguments.size() == 6)
        {
            if (arguments.at(0) == "update")
            {
                if (arguments.at(1) == "ipv4")
                {
                    if (!sdk_common->is_numeric_string(arguments.at(3)) || !sdk_common->is_numeric_string(arguments.at(4)) || !sdk_common->is_numeric_string(arguments.at(5)))
                    {
                        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                        return;
                    }

                    instance_->update_traffic_shaper_ipv4_address(arguments.at(2).c_str(), std::stoi(arguments.at(3)), std::stoull(arguments.at(4)), std::stoull(arguments.at(5)));
                }

                else if (arguments.at(1) == "ipv6")
                {
                    if (!sdk_common->is_numeric_string(arguments.at(3)) || !sdk_common->is_numeric_string(arguments.at(4)) || !sdk_common->is_numeric_string(arguments.at(5)))
                    {
                        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid command argument" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

                        return;
                    }

                    instance_->update_traffic_shaper_ipv6_address(arguments.at(2).c_str(), std::stoi(arguments.at(3)), std::stoull(arguments.at(4)), std::stoull(arguments.at(5)));
                }
            }
        }
    });

    instance_->reload_command_list_file();

    command_parser_controller->register_command("/reload", {"reload the command list file"}, [] (const std::vector<std::string> arguments) {
        instance_->reload_command_list_file();
    });
}

void XDPController::forward_command_parser_destroy() {
    command_parser_controller->unregister_command("/reload");
}

bool XDPController::update_blacklist_ipv4_address(const char* ip_address, const unsigned int prefix_length, const bool enable)
{
    struct LPMTrieKeyIPv4 key = {};

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
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to update blacklist IPv4 address" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    return true;
}

bool XDPController::update_blacklist_ipv6_address(const char* ip_address, const unsigned int prefix_length, const bool enable)
{
    struct LPMTrieKeyIPv6 key = {};

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
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to insert blacklist IPv6 address" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    return true;
}

TokenBucket XDPController::compute_token_bucket(const uint64_t rate_mbps, const uint64_t burst_mb)
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

bool XDPController::update_traffic_shaper_ipv4_address(const char* ip_address, const unsigned int prefix_length, const uint64_t rate_mbps, const uint64_t burst_mb)
{
    struct LPMTrieKeyIPv4 key = {};

    key.prefix_length = prefix_length;

    if (inet_pton(AF_INET, ip_address, &key.ip_address) != 1)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid IPv4 address format" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    const int map_fd = bpf_map__fd(this->data_plane_bpf_skeleton_->maps.traffic_shaper_ipv4_addresses_map);

    struct TokenBucket token_bucket = this->compute_token_bucket(rate_mbps, burst_mb);

    if (bpf_map_update_elem(map_fd, &key, &token_bucket, BPF_ANY))
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to update traffic shaper IPv4 address" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    return true;
}

bool XDPController::update_traffic_shaper_ipv6_address(const char* ip_address, const unsigned int prefix_length, const uint64_t rate_mbps, const uint64_t burst_mb)
{
    struct LPMTrieKeyIPv6 key = {};

    key.prefix_length = prefix_length;

    if (inet_pton(AF_INET6, ip_address, &key.ip_address) != 1)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Invalid IPv6 address format" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    const int map_fd = bpf_map__fd(this->data_plane_bpf_skeleton_->maps.traffic_shaper_ipv6_addresses_map);

    struct TokenBucket token_bucket = this->compute_token_bucket(rate_mbps, burst_mb);

    if (bpf_map_update_elem(map_fd, &key, &token_bucket, BPF_ANY))
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "Failed to update traffic shaper IPv6 address" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    return true;
}
