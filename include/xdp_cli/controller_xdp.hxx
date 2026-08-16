// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <csignal>

#include "xdp_cli/sdk_common.hxx"
#include "xdp_cli/sdk_bpf.h"

#include "xdp_cli/data_plane.skel.h"

#include "xdp_cli/controller_command_parser.hxx"

class Controller_XDP
{
private:
    inline static Controller_XDP* instance_;

    SDK_Common* sdk_common_;

    Controller_CommandParser* controller_command_parser_;

    std::vector<std::function<void()>> hooks_initialize_;
    std::vector<std::function<void()>> hooks_destroy_;

    bool running_;

    struct data_plane_bpf* data_plane_bpf_skeleton_;

    struct bpf_link* link_1_;
    struct bpf_link* link_2_;

    unsigned int in_if_index_;
    unsigned int out_if_index_;

    static void signal_handler(int signal)
    {
        if (instance_) {
            instance_->running_ = false;
        }
    }

public:
    Controller_XDP()
    {
        this->instance_ = this;

        std::signal(SIGINT, &Controller_XDP::signal_handler);
        std::signal(SIGTERM, &Controller_XDP::signal_handler);
    };

    ~Controller_XDP() = default;

    void set_sdk_common(SDK_Common* sdk_common) {
        this->sdk_common_ = sdk_common;
    }

    void set_controller_command_parser(Controller_CommandParser* controller_command_parser) {
        this->controller_command_parser_ = controller_command_parser;
    }

    auto get_running() const {
        return this->running_;
    }

    static void hook_command_parser_initialize();
    static void hook_command_parser_destroy();

    bool initialize(const char* in_interface, const char* out_interface);
    void destroy();

    bool reload_command_list_file();

    TokenBucket compute_token_bucket(const uint64_t rate_mbps, const uint64_t burst_mb);

    bool update_blacklist_ipv4_addresses_map(const char* ip_address, const unsigned int prefix_length, const bool enable);
    bool update_blacklist_ipv6_addresses_map(const char* ip_address, const unsigned int prefix_length, const bool enable);

    bool update_traffic_shaper_ipv4_addresses_map(const char* ip_address, const unsigned int prefix_length, const uint64_t rate_mbps, const uint64_t burst_mb, const uint64_t allowance_mb);
    bool update_traffic_shaper_ipv6_addresses_map(const char* ip_address, const unsigned int prefix_length, const uint64_t rate_mbps, const uint64_t burst_mb, const uint64_t allowance_mb);
};
