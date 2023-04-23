/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2023 Sartura Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "ip6_flowlabel.h"

static inline void
do_srhash_init(uint32_t seed)
{
    time_t now;

    if (seed > 0) {

        srand(seed);
    } else {
        now = time(NULL);
        srand(now);
    }
}

static inline void
srhash_init0(void)
{
    do_srhash_init(0);
}

static inline uint32_t
get_new_srhash(void)
{
    uint32_t rhash;

    rhash = (rand() % IP6_MAX_FLOW_LABEL_RANGE) + 1;

    return rhash;
}

/**
 * rol32 - rotate a 32-bit value left
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline __u32
rol32(__u32 word, unsigned int shift)
{
    return (word << (shift & 31)) | (word >> ((-shift) & 31));
}

static inline int
ip6_default_np_autolabel(void)
{
    int auto_flowlabels_val;

    auto_flowlabels_val = get_sysctl_value(IP6_AUTO_FLOWLABELS_PATH);
    switch (auto_flowlabels_val) {
    case IP6_AUTO_FLOW_LABEL_OFF:
    case IP6_AUTO_FLOW_LABEL_OPTIN:
    default:
        return 0;
    case IP6_AUTO_FLOW_LABEL_OPTOUT:
    case IP6_AUTO_FLOW_LABEL_FORCED:
        return 1;
    }
}

bool
ip6_autoflowlabel(struct pcb_entry *pcb)
{
    /*
     * If no ipv6 socket option set then read directly from sysctl
     * */

    if (pcb->ip6_fl_entry == NULL || !pcb->ip6_fl_entry->autoflowlabel_set)
        return ip6_default_np_autolabel();
    else
        return pcb->ip6_fl_entry->autoflowlabel;
}

int
get_sysctl_value(const char *path)
{

    FILE *fp;
    int val = -1;

    if (access(path, R_OK) != 0) {
        return val;
    }

    fp  = fopen(path, "r");
    fscanf(fp, "%d", &val);

    fclose(fp);

    return val;
}

uint32_t
ip6_make_flowlabel(uint32_t flowlabel, bool autolabel)
{
    __u32 hash;
    int auto_flowlabels_val, flowlabel_state_ranges_val;

    /* @flowlabel may include more than a flow label, eg, the traffic class.
     * Here we want only the flow label value.
     */
    flowlabel &= IPV6_FLOWLABEL_MASK;
    auto_flowlabels_val = get_sysctl_value(IP6_AUTO_FLOWLABELS_PATH);

    if (flowlabel || auto_flowlabels_val == IP6_AUTO_FLOW_LABEL_OFF ||
        (!autolabel && auto_flowlabels_val != IP6_AUTO_FLOW_LABEL_FORCED))
        return flowlabel;

    /* skb_get_hash_flowi6() is a complex operation in kernel, right now for CNDP we are just
     * getting a random hash.
     */
    hash = get_new_srhash(); /* hash = skb_get_hash_flowi6(skb, fl6);//Ref:include/linux/skbuff.h */

    /* Since this is being sent on the wire obfuscate hash a bit
     * to minimize possbility that any useful information to an
     * attacker is leaked. Only lower 20 bits are relevant.
     */
    hash = rol32(hash, 16);

    flowlabel = (__be32)hash & IPV6_FLOWLABEL_MASK;

    flowlabel_state_ranges_val = get_sysctl_value(IP6_FLOWLABEL_STATE_RANGES_PATH);
    if (flowlabel_state_ranges_val > 0)
        flowlabel |= IPV6_FLOWLABEL_STATELESS_FLAG;

    return flowlabel;
}
