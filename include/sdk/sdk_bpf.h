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

struct LPMTrieKeyIPv4
{
    __u32 prefix_length;

    __u32 ip_address;
} __attribute__((packed));

struct LPMTrieKeyIPv6
{
    __u32 prefix_length;

    struct in6_addr ip_address;
} __attribute__((packed));

struct TokenBucket
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
