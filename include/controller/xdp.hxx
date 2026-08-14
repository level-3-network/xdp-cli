// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <csignal>

#include "sdk/sdk_bpf.h"

#include "data_plane.skel.h"

class XDPController
{
private:
    inline static XDPController* instance_;

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
    XDPController()
    {
        this->instance_ = this;

        std::signal(SIGINT, &XDPController::signal_handler);
        std::signal(SIGTERM, &XDPController::signal_handler);

        this->running_ = true;
    };

    ~XDPController() = default;

    auto get_running() const {
        return this->running_;
    }

    void set_running(const bool value) {
        this->running_ = value;
    }

    bool initialize(const char* in_interface, const char* out_interface);
    void destroy();

    bool reload_command_list_file();

    static void forward_command_parser_initialize();
    static void forward_command_parser_destroy();

    bool update_blacklist_ipv4_address(const char* ip_address, const unsigned int prefix_length, const bool enable);
    bool update_blacklist_ipv6_address(const char* ip_address, const unsigned int prefix_length, const bool enable);

    TokenBucket compute_token_bucket(const uint64_t rate_mbps, const uint64_t burst_mb);

    bool update_traffic_shaper_ipv4_address(const char* ip_address, const unsigned int prefix_length, const uint64_t rate_mbps, const uint64_t burst_mb);
    bool update_traffic_shaper_ipv6_address(const char* ip_address, const unsigned int prefix_length, const uint64_t rate_mbps, const uint64_t burst_mb);
};
