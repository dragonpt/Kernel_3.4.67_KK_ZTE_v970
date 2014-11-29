#ifndef __MT_SMP_H
#define __MT_SMP_H

static inline int get_HW_cpuid(void)

{
    int id;

    asm ("mrc     p15, 0, %0, c0, c0, 5 @ Get CPUID\n"

            : "=r" (id));

    return (id&0x3)+((id&0xF00)>>6);
}
                                                                                
#endif  /* !__MT_SMP_H */
