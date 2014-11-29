#ifndef __MT6577_EINT_H__
#define __MT6577_EINT_H__

/*
 * Define hardware registers.
 */
#include "mach/irqs.h"
#define EINT_STA        (EINT_BASE + 0x0000)
#define EINT_INTACK     (EINT_BASE + 0x0008)
#define EINT_EEVT       (EINT_BASE + 0x0010)
#define EINT_MASK       (EINT_BASE + 0x0018)
#define EINT_MASK_SET   (EINT_BASE + 0x0020)
#define EINT_MASK_CLR   (EINT_BASE + 0x0028)
#define EINT_SENS       (EINT_BASE + 0x0030)
#define EINT_SENS_SET   (EINT_BASE + 0x0038)
#define EINT_SENS_CLR   (EINT_BASE + 0x0040)
#define EINT_SOFT       (EINT_BASE + 0x0048)
#define EINT_SOFT_SET   (EINT_BASE + 0x0050)
#define EINT_SOFT_CLR   (EINT_BASE + 0x0058)
#define EINT_D0EN       (EINT_BASE + 0x0060)
#define EINT_D1EN       (EINT_BASE + 0x0068)
#define EINT_D2EN       (EINT_BASE + 0x0070)
#define EINT_CON(n)     (EINT_BASE + 0x0080 + 4 * (n))
#define EINT_DMASK      (EINT_BASE + 0x0100)
#define EINT_DMASK_SET  (EINT_BASE + 0x0110)
#define EINT_DMASK_CLR  (EINT_BASE + 0x0120)

#define  EINT_IRQ  MT_EINT_IRQ_ID
#define  EINT_DEB_CNT_MASK   0x000007FF
#define  EINT_DEB_PRESCALER_MASK   0x00007000
#define  EINT_DEB_PRESCALER_SHIFT   12
#define  EINT_DEB_EN_BIT   0x00008000
#define  EINT_POL_BIT   0x00000800
#define  MAX_MS_FOR_PRESCALER_0   64
#define  MAX_MS_FOR_PRESCALER_1   127
#define  MAX_MS_FOR_PRESCALER_2   255
#define  MAX_MS_FOR_PRESCALER_3   511
#define  MAX_MS_FOR_PRESCALER_4   1023
#define  MAX_MS_FOR_PRESCALER_5   2047
#define  MAX_MS_FOR_PRESCALER_6   4095
#define  MAX_MS_FOR_PRESCALER_7   8000
#define  EINT_PRESCALER_SHIFT   12
#define  EINT_DEB_RST_BIT  0x80000000

/*
 * Define constants.
 */

#define EINT_MAX_CHANNEL    (32)
#define MT65XX_EINT_POL_NEG (0)
#define MT65XX_EINT_POL_POS (1)
#define EINT_AP_MAXNUMBER 32
#define DEINT_MAX_CHANNEL 8
#define MT_EINT_POL_NEG (0)
#define MT_EINT_POL_POS (1)
#define MAX_HW_DEBOUNCE_CNT 32   //ask:
#define MAX_DEINT_CNT 8
#define EINTF_TRIGGER_RISING     0x00000001
#define EINTF_TRIGGER_FALLING    0x00000002
#define EINTF_TRIGGER_HIGH       0x00000004
#define EINTF_TRIGGER_LOW        0x00000008


/*Configuration*/
//#define EINT_TEST

typedef struct
{
    void (*eint_func[EINT_MAX_CHANNEL]) (void);
    unsigned int eint_auto_umask[EINT_MAX_CHANNEL];
    /*is_deb_en: 1 means enable, 0 means disable */
    unsigned int is_deb_en[EINT_MAX_CHANNEL];
    unsigned int deb_time[EINT_MAX_CHANNEL];
#if defined(EINT_TEST)
    unsigned int softisr_called[EINT_MAX_CHANNEL];
#endif
    struct timer_list eint_sw_deb_timer[EINT_MAX_CHANNEL];
} eint_func;

/*
 * Define function prototypes.
 */
/*
extern void mt65xx_eint_mask(unsigned int eint_num);
extern void mt65xx_eint_unmask(unsigned int eint_num);
extern void mt65xx_deint_mask(unsigned int eint_num);
extern void mt65xx_deint_unmask(unsigned int eint_num);
extern void mt65xx_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt65xx_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt65xx_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
extern void mt65xx_deint_registration(unsigned int eint_num, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
*/
extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
extern int get_td_eint_num(char *td_name, int len);
extern void mt_eint_print_status(void);

extern void mt_eint_mask(unsigned int eint_num);
extern void mt_eint_unmask(unsigned int eint_num);
extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
extern void mt_eint_print_status(void);

#endif  /*!__MT6577_EINT_H__ */
