/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2023 Sartura Ltd.
 */

#include <cne_common.h>        // for CNE_SET_USED,
#include <string.h>
#include <stdlib.h>
#include <sched.h>

#define __cpuid_count(level, count, a, b, c, d) ___cpuid_count()
//        abs(0)

/*#define __cpuid_count(level, count, a, b, c, d) \
    __asm volatile("cpuid\n\t" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "0"(level), "2"(count))
*/

static __inline int
___cpuid_count(void)
{

    return 0;
}

static __inline unsigned int
__get_cpuid_max(unsigned int __ext, unsigned int *__sig)
{

    CNE_SET_USED(__ext);
    CNE_SET_USED(__sig);
    return 0;
}

static __inline int
__builtin_cpu_supports(const char *feature)
{

    /* Supports all SSE series by using sse2neon.h */
    if (strncmp(feature, "sse", 3) == 0)
        return 1;

    if (strncmp(feature, "ssse3", 5) == 0)
        return 1;

    if (strncmp(feature, "neon", 4) == 0)
        return 1;

    return 0;
}

/*
 * Migrate the current thread to another scheduler running
 * on the specified thread.
 */
static __inline int
__arm_cthread_set_affinity(int threadid)
{
    struct sched_param sp;
    int ret, sched;

    sched = sched_getscheduler(threadid);
    if (sched == -1)
        return EINVAL;

    sp.sched_priority = sched_get_priority_max(sched);
    /* Do it for current thread */
    ret = sched_setscheduler(0, sched, &sp);
    if (ret != 0)
        return EINVAL;

    return 0;
}
