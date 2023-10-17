
/* Put processor into low power WFE(Wait For Event) state. */
#define __INTR_ARM_WFE()                    \
    {                                       \
        asm volatile("wfe" : : : "memory"); \
    }

/*
 * Atomic exclusive load from addr, it returns the 32-bit content of
 * *addr while making it 'monitored', when it is written by someone
 * else, the 'monitored' state is cleared and an event is generated
 * implicitly to exit WFE.
 */
#define __INTR_ARM_LOAD_EXC_32(src, dst, memorder)   \
    {                                                \
        if (memorder == __ATOMIC_RELAXED) {          \
            asm volatile("ldxr %w[tmp], [%x[addr]]"  \
                         : [tmp] "=&r"(dst)          \
                         : [addr] "r"(src)           \
                         : "memory");                \
        } else {                                     \
            asm volatile("ldaxr %w[tmp], [%x[addr]]" \
                         : [tmp] "=&r"(dst)          \
                         : [addr] "r"(src)           \
                         : "memory");                \
        }                                            \
    }
