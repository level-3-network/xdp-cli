// SPDX-License-Identifier: GPL-2.0-only

#include "xdp_cli/sdk_bpf.h"

#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

__u32 in_if_index;

__u8 tx_port_redirect_map;
__u8 map_ip_address_key_reverse_lookup;

struct PacketContext
{
    __u64 packet_length;

    struct ethhdr* ethernet_header;

    struct iphdr* ipv4_header;
    struct ipv6hdr* ipv6_header;

    struct tcphdr* tcp_header;
    struct udphdr* udp_header;

    struct icmphdr* icmp_header;
    struct icmp6hdr* icmpv6_header;

    void* data_end;
};

struct
{
    __uint(type, BPF_MAP_TYPE_DEVMAP);
    __uint(max_entries, 64);
    __type(key, __u32);
    __type(value, __u32);
} tx_port_map SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_LPM_TRIE);
    __type(key, struct LPM_TrieKey_IPv4);
    __type(value, __u32);
    __uint(max_entries, 65536);
    __uint(map_flags, BPF_F_NO_PREALLOC);
} blacklist_ipv4_addresses_map SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_LPM_TRIE);
    __type(key, struct LPM_TrieKey_IPv6);
    __type(value, __u32);
    __uint(max_entries, 65536);
    __uint(map_flags, BPF_F_NO_PREALLOC);
} blacklist_ipv6_addresses_map SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_LPM_TRIE);
    __uint(max_entries, 2048);
    __type(key, struct LPM_TrieKey_IPv4);
    __type(value, struct TokenBucket);
    __uint(map_flags, BPF_F_NO_PREALLOC);
} traffic_shaper_ipv4_addresses_map SEC(".maps");

struct
{
    __uint(type, BPF_MAP_TYPE_LPM_TRIE);
    __uint(max_entries, 2048);
    __type(key, struct LPM_TrieKey_IPv6);
    __type(value, struct TokenBucket);
    __uint(map_flags, BPF_F_NO_PREALLOC);
} traffic_shaper_ipv6_addresses_map SEC(".maps");

static __always_inline int parse_packet(struct xdp_md* ctx, struct PacketContext* packet_ctx)
{
    void* data = (void*)(long)ctx->data;

    packet_ctx->data_end = (void*)(long)ctx->data_end;

    packet_ctx->packet_length = packet_ctx->data_end - data;

    packet_ctx->ethernet_header = data;

    if ((void*)(packet_ctx->ethernet_header + 1) > packet_ctx->data_end) {
        return 0;
    }

    __u16 l2_protocol = bpf_ntohs(packet_ctx->ethernet_header->h_proto);

    void* next_header = packet_ctx->ethernet_header + 1;

    __u8 l4_protocol = 0;

    if (l2_protocol == ETH_P_IP)
    {
        packet_ctx->ipv4_header = next_header;

        if ((void*)(packet_ctx->ipv4_header + 1) > packet_ctx->data_end) {
            return 0;
        }

        l4_protocol = packet_ctx->ipv4_header->protocol;

        next_header = packet_ctx->ipv4_header + 1;
    }

    else if (l2_protocol == ETH_P_IPV6)
    {
        packet_ctx->ipv6_header = next_header;
        
        if ((void*)(packet_ctx->ipv6_header + 1) > packet_ctx->data_end) {
            return 0;
        }

        l4_protocol = packet_ctx->ipv6_header->nexthdr;

        next_header = packet_ctx->ipv6_header + 1;
    }

    else {
        return 1;
    }

    if (l4_protocol == IPPROTO_TCP)
    {
        packet_ctx->tcp_header = next_header;

        if ((void*)(packet_ctx->tcp_header + 1) > packet_ctx->data_end) {
            return 0;
        }
    }

    else if (l4_protocol == IPPROTO_UDP)
    {
        packet_ctx->udp_header = next_header;

        if ((void*)(packet_ctx->udp_header + 1) > packet_ctx->data_end) {
            return 0;
        }
    }

    else if (l4_protocol == IPPROTO_ICMP)
    {
        packet_ctx->icmp_header = next_header;

        if ((void*)(packet_ctx->icmp_header + 1) > packet_ctx->data_end) {
            return 0;
        }
    }

    else if (l4_protocol == IPPROTO_ICMPV6)
    {
        packet_ctx->icmpv6_header = next_header;

        if ((void*)(packet_ctx->icmpv6_header + 1) > packet_ctx->data_end) {
            return 0;
        }
    }

    return 1;
}

static __always_inline int check_and_consume_tokens(struct TokenBucket* token_bucket, __u64 packet_length)
{
    __u64 now = bpf_ktime_get_ns();

    __u64 elapsed = now - token_bucket->last_updated;

    if (token_bucket->last_updated == 0)
    {
        token_bucket->tokens = token_bucket->max_burst;

        token_bucket->last_updated = now;

        elapsed = 0;
    }

    if (token_bucket->allowance < packet_length) {
        return 1;
    }

    token_bucket->allowance -= packet_length;

    token_bucket->tokens += elapsed;

    if (token_bucket->tokens > token_bucket->max_burst) {
        token_bucket->tokens = token_bucket->max_burst;
    }

    token_bucket->last_updated = now;

    __u64 token_cost = packet_length * token_bucket->nanosecond_per_byte;

    if (token_bucket->tokens >= token_cost)
    {
        token_bucket->tokens -= token_cost;

        return 0;
    }

    return 1;
}

static __always_inline int process_blacklisted_ipv4_addresses(struct PacketContext* packet_ctx, __u32* ingress_ifindex)
{
    struct LPM_TrieKey_IPv4 key = {};

    key.prefix_length = 32;

    if (*ingress_ifindex == in_if_index)
    {
        if (map_ip_address_key_reverse_lookup == 1) {
            key.ip_address = packet_ctx->ipv4_header->saddr;
        }

        else {
            key.ip_address = packet_ctx->ipv4_header->daddr;
        }        
    }

    else
    {
        if (map_ip_address_key_reverse_lookup == 1) {
            key.ip_address = packet_ctx->ipv4_header->daddr;
        }

        else {
            key.ip_address = packet_ctx->ipv4_header->saddr;
        }        
    }

    __u32* return_value = bpf_map_lookup_elem(&blacklist_ipv4_addresses_map, &key);

    if (return_value && *return_value == 1) {
        return 1;
    }

    return 0;
}

static __always_inline int process_blacklisted_ipv6_addresses(struct PacketContext* packet_ctx, __u32* ingress_ifindex)
{
    struct LPM_TrieKey_IPv6 key = {};

    key.prefix_length = 128;

    if (*ingress_ifindex == in_if_index)
    {
        if (map_ip_address_key_reverse_lookup == 1) {
            __builtin_memcpy(&key.ip_address, &packet_ctx->ipv6_header->saddr, sizeof(struct in6_addr));
        }

        else {
            __builtin_memcpy(&key.ip_address, &packet_ctx->ipv6_header->daddr, sizeof(struct in6_addr));
        }
    }

    else
    {
        if (map_ip_address_key_reverse_lookup == 1) {
            __builtin_memcpy(&key.ip_address, &packet_ctx->ipv6_header->daddr, sizeof(struct in6_addr));
        }

        else {
            __builtin_memcpy(&key.ip_address, &packet_ctx->ipv6_header->saddr, sizeof(struct in6_addr));
        }
    }

    __u32* return_value = bpf_map_lookup_elem(&blacklist_ipv6_addresses_map, &key);

    if (return_value && *return_value == 1) {
        return 1;
    }

    return 0;
}

static __always_inline int process_traffic_shaped_ipv4_addresses(struct PacketContext* packet_ctx, __u32* ingress_ifindex)
{
    struct LPM_TrieKey_IPv4 key = {};

    key.prefix_length = 32;

    if (*ingress_ifindex == in_if_index)
    {
        if (map_ip_address_key_reverse_lookup == 1) {
            key.ip_address = packet_ctx->ipv4_header->saddr;
        }

        else {
            key.ip_address = packet_ctx->ipv4_header->daddr;
        }        
    }

    else
    {
        if (map_ip_address_key_reverse_lookup == 1) {
            key.ip_address = packet_ctx->ipv4_header->daddr;
        }

        else {
            key.ip_address = packet_ctx->ipv4_header->saddr;
        }        
    }

    struct TokenBucket* token_bucket = bpf_map_lookup_elem(&traffic_shaper_ipv4_addresses_map, &key);

    if (token_bucket) {
        return check_and_consume_tokens(token_bucket, packet_ctx->packet_length);
    }

    return 0;
}

static __always_inline int process_traffic_shaped_ipv6_addresses(struct PacketContext* packet_ctx, __u32* ingress_ifindex)
{
    struct LPM_TrieKey_IPv6 key = {};

    key.prefix_length = 128;

    if (*ingress_ifindex == in_if_index)
    {
        if (map_ip_address_key_reverse_lookup == 1) {
            __builtin_memcpy(&key.ip_address, &packet_ctx->ipv6_header->saddr, sizeof(struct in6_addr));
        }

        else {
            __builtin_memcpy(&key.ip_address, &packet_ctx->ipv6_header->daddr, sizeof(struct in6_addr));
        }
    }

    else
    {
        if (map_ip_address_key_reverse_lookup == 1) {
            __builtin_memcpy(&key.ip_address, &packet_ctx->ipv6_header->daddr, sizeof(struct in6_addr));
        }

        else {
            __builtin_memcpy(&key.ip_address, &packet_ctx->ipv6_header->saddr, sizeof(struct in6_addr));
        }
    }

    struct TokenBucket* token_bucket = bpf_map_lookup_elem(&traffic_shaper_ipv6_addresses_map, &key);

    if (token_bucket) {
        return check_and_consume_tokens(token_bucket, packet_ctx->packet_length);
    }

    return 0;
}

SEC("xdp")
int xdp_main(struct xdp_md* ctx)
{
    struct PacketContext packet_ctx = {};

    if (!parse_packet(ctx, &packet_ctx)) {
        return XDP_DROP;
    }

    if (packet_ctx.ipv4_header)
    {
        if (process_blacklisted_ipv4_addresses(&packet_ctx, &ctx->ingress_ifindex)) {
            return XDP_DROP;
        }

        if (process_traffic_shaped_ipv4_addresses(&packet_ctx, &ctx->ingress_ifindex)) {
            return XDP_DROP;
        }
    }

    else if (packet_ctx.ipv6_header)
    {
        if (process_blacklisted_ipv6_addresses(&packet_ctx, &ctx->ingress_ifindex)) {
            return XDP_DROP;
        }

        if (process_traffic_shaped_ipv6_addresses(&packet_ctx, &ctx->ingress_ifindex)) {
            return XDP_DROP;
        }
    }

    if (tx_port_redirect_map == 1)
    {
        __u32 output_if_index = ctx->ingress_ifindex;

        if (bpf_map_lookup_elem(&tx_port_map, &output_if_index)) {
            return bpf_redirect_map(&tx_port_map, output_if_index, 0);
        }
    }

    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
