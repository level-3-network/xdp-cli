// SPDX-License-Identifier: GPL-2.0-only

#ifndef _SDK_BPF_H
#define _SDK_BPF_H

#include <arpa/inet.h>

#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/icmpv6.h>
#include <linux/bpf.h>

struct BPF_PacketContext
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

struct LPM_TrieKey_IPv4
{
    __u32 prefix_length;

    __u32 ip_address;
} __attribute__((packed));

struct LPM_TrieKey_IPv6
{
    __u32 prefix_length;

    struct in6_addr ip_address;
} __attribute__((packed));

struct TrafficShaper_TokenBucket
{
    __u64 nanosecond_per_byte;

    __u64 max_burst;
    __u64 tokens;

    __u64 last_updated;

    __u64 allowance;

    __u64 padding_1;
    __u64 padding_2;
    __u64 padding_3;
} __attribute__((packed));

#endif
