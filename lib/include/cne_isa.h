/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2019-2023 Intel Corporation
 */

#include <sys/syscall.h>
#include <stdint.h>        // for uint64_t

#ifndef _CNE_ISA_H_
#define _CNE_ISA_H_

#include <cne_common.h>        // for CNDP_API
#include <cne_log.h>

#if __aarch64__
#include <sse2neon.h>
#include <arm_intrinsics.h>
#endif

/**
 * @file
 *
 * APIs for ISA instructions
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set the address for UMONITOR instruction.
 *
 * For more information about usage of these instructions, please refer to
 * Intel(R) 64 and IA-32 Architectures Software Developer's Manual.
 *
 * @param addr
 *   Address to use for umonitor.
 * @return
 *   None.
 */
static __cne_always_inline void
cne_umonitor(volatile void *addr)
{
#if __x86_64__
    /* UMONITOR */
    asm volatile(".byte 0xf3, 0x0f, 0xae, 0xf7;" : : "D"(addr));
#elif __aarch64__
    CNE_SET_USED(addr);
    /*
     * Probably __INTR_ARM_WFE() can be used for this.
     */
    CNE_ERR("Not supported yet in ARM\n");
#endif
}

/**
 * Execute UMWAIT given the timestamp value.
 *
 * This function will enter C0.2 state.
 *
 * For more information about usage of these instructions, please refer to
 * Intel(R) 64 and IA-32 Architectures Software Developer's Manual.
 *
 * @param timestamp
 *   The number of cycles to wait.
 * @return
 *   None.
 */
static __cne_always_inline void
cne_umwait(const uint64_t timestamp)
{
#if __x86_64__
    const uint32_t l = (uint32_t)timestamp;
    const uint32_t h = (uint32_t)(timestamp >> 32);

    /* UMWAIT */
    asm volatile(".byte 0xf2, 0x0f, 0xae, 0xf7;"
                 :         /* ignore rflags */
                 : "D"(0), /* enter C0.2 */
                   "a"(l), "d"(h));
#elif __aarch64__
    CNE_SET_USED(timestamp);
    /*
     * Probably __INTR_ARM_WFE() can be used for this.
     */
    CNE_ERR("Not supported yet in ARM\n");
#endif
}

/**
 * Execute TPAUSE given the timestamp value.
 *
 * This function uses TPAUSE instruction  and will enter C0.2 state. For more
 * information about usage of this instruction, please refer to Intel(R) 64 and
 * IA-32 Architectures Software Developer's Manual.
 *
 * @param  timestamp
 *   The number of cycles to wait.
 * @return
 *   None.
 */
static __cne_always_inline void
cne_tpause(const uint64_t timestamp)
{
#if __x86_64__
    const uint32_t l = (uint32_t)timestamp;
    const uint32_t h = (uint32_t)(timestamp >> 32);

    /* TPAUSE */
    asm volatile(".byte 0x66, 0x0f, 0xae, 0xf7;"
                 :         /* ignore rflags */
                 : "D"(0), /* enter C0.2 */
                   "a"(l), "d"(h));
#elif __aarch64__
    CNE_SET_USED(timestamp);
    asm volatile("yield" ::: "memory");
#endif
}

/**
 * MOVDIRI instruction.
 *
 * @param addr
 *   The address to put the value.
 * @param value
 *   The value to move to the given address.
 * @return
 *   None.
 */
static __cne_always_inline void
cne_movdiri(volatile void *addr, uint32_t value)
{
#if __x86_64__
    /* MOVDIRI */
    asm volatile(".byte 0x40, 0x0f, 0x38, 0xf9, 0x02" : : "a"(value), "d"(addr));
#elif __aarch64__
    // vst1_u32((volatile uint32_t *)addr, vreinterpretq_u32_m128i(value));
    // vst1_u32((uint32_t *)addr, value);
    CNE_SET_USED(addr);
    CNE_SET_USED(value);
    CNE_ERR("Not supported yet in ARM\n");
#endif
}

/**
 * Use movdir64b instruction to move data from source to destination
 *
 * @param dst
 *   The destination address to put the source data
 * @param src
 *   The source address to get the data from.
 * @return
 *   None.
 */
static __cne_always_inline void
cne_movdir64b(volatile void *dst, const void *src)
{
#if __x86_64__
    /* MOVDIR64B */
    asm volatile(".byte 0x66, 0x0f, 0x38, 0xf8, 0x02" : : "a"(dst), "d"(src) : "memory");
#elif __aarch64__
    // void *dst2 = dst;
    // memcpy(dst2, src, 64);
    // dst = dst2;
    // vst1_u64((uint64_t *)dst, *(uint64_t *)src);
    CNE_SET_USED(dst);
    CNE_SET_USED(src);
    CNE_ERR("Not supported yet in ARM\n");
#endif
}

/**
 * Demote a cacheline entry
 *
 * @param p
 *   The address of the cacheline to demote.
 * @return
 *   None.
 */
static __cne_always_inline void
cne_cldemote(const volatile void *p)
{
#if __x86_64__
    /* CLDEMOTE */
    asm volatile(".byte 0x0f, 0x1c, 0x06" ::"S"(p));
#elif __aarch64__
    CNE_SET_USED(p);
    CNE_ERR("Not supported yet in ARM\n");
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* _CNE_ISA_H_ */
