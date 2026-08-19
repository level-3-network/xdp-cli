// SPDX-License-Identifier: GPL-2.0-only

#pragma once

class Controller_XDP
{
private:
    bool is_initialized_;

public:
    struct data_plane_bpf* data_plane_bpf_skeleton_;

private:
    struct bpf_link* link_1_;
    struct bpf_link* link_2_;

    unsigned int wan_if_index_;
    unsigned int lan_if_index_;

public:
    Controller_XDP() = default;
    ~Controller_XDP() = default;

    auto& is_initialized() const {
        return this->is_initialized_;
    }

    bool initialize(const std::string& wan_interface, const std::string& lan_interface);
    bool destroy();

    bool reload_command_list_file(const std::string& path);

    TrafficShaper_TokenBucket compute_token_bucket(const uint64_t rate_mbps, const uint64_t burst_mb);

    bool update_blacklist_ipv4_addresses_map(const char* ip_address, const unsigned int prefix_length, const bool enable);
    bool update_blacklist_ipv6_addresses_map(const char* ip_address, const unsigned int prefix_length, const bool enable);

    bool update_traffic_shaper_ipv4_addresses_map(const char* ip_address, const unsigned int prefix_length, const uint64_t rate_mbps, const uint64_t burst_mb, const uint64_t allowance_mb);
    bool update_traffic_shaper_ipv6_addresses_map(const char* ip_address, const unsigned int prefix_length, const uint64_t rate_mbps, const uint64_t burst_mb, const uint64_t allowance_mb);
};
