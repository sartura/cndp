/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2023 Sartura Ltd.
 */

#include "../chnl/chnl_priv.h"
#include <cne_inet.h>         // for inet_ntop4, in_caddr_copy, in_cad...
#include <cnet.h>             // for cnet_add_instance
#include <cnet_chnl.h>        // for AF_INET, SOCK_DGRAM, CH_IP_DONTFRAG
#include <cnet_icmp6.h>
#include <cnet_ip_common.h>        // for ip_info
#include <cnet_netif.h>            // for net_addr, cnet_ipv4_compare, netif
#include <cnet_route.h>            // for rt_funcs, rt_lookup_t
#include <cnet_route4.h>           // for
#include <cnet_stk.h>              // for stk_entry, per_thread_stk, proto_...
#include <endian.h>                // for be16toh, htobe16
#include <netinet/in.h>            // for IPPROTO_ICMPV6
#include <stdlib.h>                // for NULL, calloc, free

#include "cne_branch_prediction.h"        // for unlikely, likely
#include "cne_log.h"                      // for CNE_LOG, CNE_LOG_DEBUG, CNE_LOG_W...
#include "cne_vec.h"                      // for vec_len, vec_free_mbuf_at_index
#include "cnet_fib_info.h"
#include "cnet_ipv4.h"           // for _OFF_DF
#include "cnet_protosw.h"        // for
#include "cnet_reg.h"
#include "net/cne_ip.h"           // for cne_ipv4_hdr, cne_ipv4_icmp6tcp_cksum
#include "pktmbuf.h"              // for pktmbuf_data_len, pktmbuf_t, pktm...
#include <net/cne_ether.h>        // for cne_ether_hdr

static int
icmp6_create(void *_stk)
{
    stk_t *stk = _stk;
    struct protosw_entry *psw;

    stk->icmp6 = calloc(1, sizeof(struct icmp6_entry));
    if (stk->icmp6 == NULL) {
        CNE_ERR("Allocation of ICMP6 structure failed\n");
        return -1;
    }

    psw = cnet_protosw_add("ICMP6", AF_INET6, SOCK_DGRAM, IPPROTO_ICMPV6);
    if (psw == NULL)
        psw = cnet_protosw_add("ICMP6", AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);

    CNE_ASSERT(psw != NULL);

    cnet_ipproto_set(IPPROTO_ICMPV6, psw);

    stk->icmp6->cksum_on            = 1;
    stk->icmp6->rcv_size            = MAX_ICMP6_RCV_SIZE;
    stk->icmp6->snd_size            = MAX_ICMP6_SND_SIZE;
    stk->icmp6->icmp6_hd.local_port = _IPPORT_RESERVED;

    return 0;
}

static int
icmp6_destroy(void *_stk)
{
    stk_t *stk = _stk;

    if (stk->icmp6) {
        struct pcb_entry *p;

        vec_foreach_ptr (p, stk->icmp6->icmp6_hd.vec)
            free(p);
        vec_free(stk->icmp6->icmp6_hd.vec);
        stk->icmp6->icmp6_hd.vec = NULL;
        free(stk->icmp6);
        stk->icmp6 = NULL;
    }
    return 0;
}

CNE_INIT_PRIO(cnet_icmp6_constructor, STACK)
{
    cnet_add_instance("icmp6", CNET_ICMP6_PRIO, icmp6_create, icmp6_destroy);
}
