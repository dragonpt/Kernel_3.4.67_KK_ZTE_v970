#ifndef __MT_IRQS_H
#define __MT_IRQS_H

#include "mt_irq.h"

/*
 * Define constants.
 */
#define FIQ_START 0
#define FIQ_SMP_CALL_SGI 13
#define FIQ_DBG_SGI 14
#define NR_IRQS NR_MT_IRQ_LINE
#define MT_EDGE_SENSITIVE 0
#define MT_LEVEL_SENSITIVE 1
#define MT_POLARITY_LOW   0
#define MT_POLARITY_HIGH  1

#if !defined(__ASSEMBLY__)

/*
 * Define data structures.
 */
enum 
{
    IRQ_MASK_HEADER = 0xF1F1F1F1, 
    IRQ_MASK_FOOTER = 0xF2F2F2F2
};

struct mtk_irq_mask
{
    unsigned int header;   /* for error checking */
    __u32 mask0;
    __u32 mask1;
    __u32 mask2;
    __u32 mask3;
    __u32 mask4;
    unsigned int footer;   /* for error checking */
};

typedef void (*fiq_isr_handler)(void *arg, void *regs, void *svc_sp);

/*
 * Define function prototypes.
 */
extern void mt_init_irq(void);
extern void mt_irq_dump(void);
extern int mt_irq_is_active(const unsigned int irq);
extern int request_fiq(int irq, fiq_isr_handler handler, unsigned long irq_flags, void *arg);
#if defined(CONFIG_FIQ_GLUE)
extern void trigger_sw_irq(int irq);
extern int mt_enable_fiq(int irq);
extern int mt_disable_fiq(int irq);
#else
#define trigger_sw_irq(__irq) (0)
#define mt_enable_fiq(__irq) (0)
#define mt_disable_fiq(__irq) (0)
#endif
extern void mt_enable_ppi(int irq);

#endif  /* !__ASSEMBLY__ */

#endif  /* !__MT_IRQS_H */
