// SPDX-License-Identifier: GPL-2.0-only

#include "xdp_cli/program_core.hxx"

extern Controller_CommandParser* controller_command_parser;

bool Controller_XDP::initialize(const std::string& wan_interface, const std::string& lan_interface)
{
    if (this->is_initialized_) {
        return false;
    }

    this->is_initialized_ = true;

    this->wan_if_index_ = if_nametoindex(wan_interface.c_str());

    if (this->wan_if_index_ == 0)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to find WAN interface [" << wan_interface << "]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    this->lan_if_index_ = if_nametoindex(lan_interface.c_str());

    if (this->lan_if_index_ == 0)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to find LAN interface [" << lan_interface << "]" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    this->data_plane_bpf_skeleton_ = data_plane_bpf__open_and_load();

    if (!this->data_plane_bpf_skeleton_)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to open and load BPF skeleton" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    this->data_plane_bpf_skeleton_->bss->wan_if_index = this->wan_if_index_;

    const int tx_port_map_fd = bpf_map__fd(this->data_plane_bpf_skeleton_->maps.tx_port_map);

    if (bpf_map_update_elem(tx_port_map_fd, &this->wan_if_index_, &this->lan_if_index_, BPF_ANY) < 0 || bpf_map_update_elem(tx_port_map_fd, &this->lan_if_index_, &this->wan_if_index_, BPF_ANY) < 0)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to update TX port map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "eBPF bytecode verified and loaded successfully" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

    this->link_1_ = bpf_program__attach_xdp(this->data_plane_bpf_skeleton_->progs.xdp_main, this->wan_if_index_);

    if (!link_1_)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to attach XDP program to WAN interface" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    else {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "XDP program successfully attached to WAN interface" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
    }

    this->link_2_ = bpf_program__attach_xdp(this->data_plane_bpf_skeleton_->progs.xdp_main, this->lan_if_index_);

    if (!link_2_)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to attach XDP program to LAN interface" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    else {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "XDP program successfully attached to LAN interface" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;
    }

    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Successfully initialized XDP" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

    return true;
}

bool Controller_XDP::destroy()
{
    if (!this->is_initialized_) {
        return false;
    }

    this->is_initialized_ = false;

    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << "Destroying XDP controller..." << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

    bpf_link__destroy(this->link_1_);
    bpf_link__destroy(this->link_2_);

    data_plane_bpf__destroy(this->data_plane_bpf_skeleton_);

    return true;
}

bool Controller_XDP::reload_command_list_file(const std::string& path)
{
    if (!this->is_initialized_) {
        return false;
    }

    std::ifstream ifs(path);

    if (!ifs.is_open())
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to open command list file" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

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

        controller_command_parser->execute_command(line);
    }

    ifs.close();

    std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_GREEN << "Reloaded command list file" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

    return true;
}

TrafficShaper_TokenBucket Controller_XDP::compute_token_bucket(const uint64_t rate_mbps, const uint64_t burst_mb)
{
    struct TrafficShaper_TokenBucket token_bucket = {};

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
    if (!this->is_initialized_) {
        return false;
    }

    struct LPM_TrieKey_IPv4 key = {};

    key.prefix_length = prefix_length;

    if (inet_pton(AF_INET, ip_address, &key.ip_address) != 1)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Invalid IPv4 address format" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    const int map_fd = bpf_map__fd(this->data_plane_bpf_skeleton_->maps.blacklist_ipv4_addresses_map);

    unsigned int value = enable ? 1 : 0;

    if (bpf_map_update_elem(map_fd, &key, &value, BPF_ANY) < 0)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to update blacklist IPv4 addresses map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    return true;
}

bool Controller_XDP::update_blacklist_ipv6_addresses_map(const char* ip_address, const unsigned int prefix_length, const bool enable)
{
    if (!this->is_initialized_) {
        return false;
    }

    struct LPM_TrieKey_IPv6 key = {};

    key.prefix_length = prefix_length;

    if (inet_pton(AF_INET6, ip_address, &key.ip_address) != 1)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Invalid IPv6 address format" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    const int map_fd = bpf_map__fd(this->data_plane_bpf_skeleton_->maps.blacklist_ipv6_addresses_map);

    unsigned int value = enable ? 1 : 0;

    if (bpf_map_update_elem(map_fd, &key, &value, BPF_ANY) < 0)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to update blacklist IPv6 addresses map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    return true;
}

bool Controller_XDP::update_traffic_shaper_ipv4_addresses_map(const char* ip_address, const unsigned int prefix_length, const uint64_t rate_mbps, const uint64_t burst_mb, const uint64_t allowance_mb)
{
    if (!this->is_initialized_) {
        return false;
    }

    struct LPM_TrieKey_IPv4 key = {};

    key.prefix_length = prefix_length;

    if (inet_pton(AF_INET, ip_address, &key.ip_address) != 1)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Invalid IPv4 address format" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    const int map_fd = bpf_map__fd(this->data_plane_bpf_skeleton_->maps.traffic_shaper_ipv4_addresses_map);

    struct TrafficShaper_TokenBucket token_bucket = this->compute_token_bucket(rate_mbps, burst_mb);

    token_bucket.allowance = allowance_mb * 1048576;

    if (bpf_map_update_elem(map_fd, &key, &token_bucket, BPF_ANY))
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to update traffic shaper IPv4 addresses map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    return true;
}

bool Controller_XDP::update_traffic_shaper_ipv6_addresses_map(const char* ip_address, const unsigned int prefix_length, const uint64_t rate_mbps, const uint64_t burst_mb, const uint64_t allowance_mb)
{
    if (!this->is_initialized_) {
        return false;
    }

    struct LPM_TrieKey_IPv6 key = {};

    key.prefix_length = prefix_length;

    if (inet_pton(AF_INET6, ip_address, &key.ip_address) != 1)
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Invalid IPv6 address format" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    const int map_fd = bpf_map__fd(this->data_plane_bpf_skeleton_->maps.traffic_shaper_ipv6_addresses_map);

    struct TrafficShaper_TokenBucket token_bucket = this->compute_token_bucket(rate_mbps, burst_mb);

    token_bucket.allowance = allowance_mb * 1048576;

    if (bpf_map_update_elem(map_fd, &key, &token_bucket, BPF_ANY))
    {
        std::cout << SDK_COMMON_CONSOLE_TEXT_BOLD << SDK_COMMON_CONSOLE_TEXT_RED << "[!] Failed to update traffic shaper IPv6 addresses map" << SDK_COMMON_CONSOLE_TEXT_DEFAULT << std::endl;

        return false;
    }

    return true;
}
