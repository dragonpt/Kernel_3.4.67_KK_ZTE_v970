#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/wakelock.h>
#include <linux/module.h>
#include <asm/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <cust_eint.h>

#include "mach/mt_reg_base.h"
#include "mach/eint.h"
#include "mach/irqs.h"
#include "mach/sync_write.h"
#include "mach/eint_drv.h"
#define EINT_DEBUG   0
#if(EINT_DEBUG == 1)
#define dbgmsg printk
#else
#define dbgmsg(...)
#endif

/*
 * Define internal data structures.
 */


#if 0
typedef enum tagDEINT_GROUP{
    DEINT_GROUP0 = 0, DEINT_GROUP1, DEINT_GROUP2, DEINT_GROUP3,
    DEINT_GROUP4, DEINT_GROUP5, DEINT_GROUP6, DEINT_GROUP7,
    DEINT_GROUP_MAX
} deint_group;
#endif
/*
 * Define global variables.
 */

#ifdef EINT_TEST
eint_func EINT_FUNC;
#else
static eint_func EINT_FUNC;
#endif
struct wake_lock EINT_suspend_lock;
static unsigned int cur_eint_num;
static int  mt_eint_init(void);
static void mt_eint_dis_hw_debounce(unsigned int eint_num);
 static void mt_eint_en_hw_debounce(unsigned int eint_num);
#if 0
static eint_func DEINT_FUNC;
static const deint_group deint_group_id[DEINT_GROUP_MAX] = {
    DEINT_GROUP0, DEINT_GROUP1, DEINT_GROUP2, DEINT_GROUP3,
    DEINT_GROUP4, DEINT_GROUP5, DEINT_GROUP6, DEINT_GROUP7
};
#endif


#if defined(EINT_TEST)
/*
 * mt_eint_soft_set: Trigger the specified EINT number.
 * @eint_num: EINT number to set
 */
void mt_eint_soft_set(unsigned int eint_num)
{

    unsigned int base;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = EINT_SOFT_SET;
    } else {
            dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return;
    }
    writel(bit, IOMEM(base));

    dbgmsg("[EINT] soft set addr:%x = %x\n", base, bit);

}

/*
 * mt_eint_soft_clr: Unmask the specified EINT number.
 * @eint_num: EINT number to clear
 */
void mt_eint_soft_clr(unsigned int eint_num)
{
    unsigned int base;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = EINT_SOFT_CLR;
    } else {
            dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return;
    }
    writel(bit, IOMEM(base));

    dbgmsg("[EINT] soft clr addr:%x = %x\n", base, bit);

}
#endif


/*
 * mt_eint_dis_hw_debounce: To disable hw debounce
 * @eint_num: the EINT number to set
 */
static void mt_eint_dis_hw_debounce(unsigned int eint_num)
{
    unsigned int clr_base, bit;
    clr_base = EINT_CON(eint_num);
    bit = readl(EINT_CON(eint_num)) & ~EINT_DEB_EN_BIT;
    writel(bit, IOMEM(clr_base));
    EINT_FUNC.is_deb_en[eint_num] = 0;
}


static void mt_eint_disable_debounce(unsigned int cur_eint_num)
{
    mt_eint_mask(cur_eint_num);
    mt_eint_dis_hw_debounce(cur_eint_num);
    mt_eint_unmask(cur_eint_num);
}


static void mt_eint_enable_debounce(unsigned int cur_eint_num)
{
    mt_eint_mask(cur_eint_num);
    mt_eint_en_hw_debounce(cur_eint_num);
    mt_eint_unmask(cur_eint_num);
}

static int mt_eint_is_debounce_en(unsigned int cur_eint_num)
{
    unsigned int base, val, en;
    base = EINT_CON(cur_eint_num);
    val = readl(IOMEM(base));
    if (val & EINT_DEB_EN_BIT) {
        en = 1;
    } else {
        en = 0;
    }
    return en;
}

static unsigned int mt_eint_get_debounce_cnt(unsigned int cur_eint_num)
{
    unsigned int dbnc, deb, base,prescalar,deb_con;
    base = EINT_CON(cur_eint_num);
  
        deb_con = readl(IOMEM(base));
        dbnc = (deb_con & EINT_DEB_CNT_MASK);
        prescalar = (deb_con & EINT_DEB_PRESCALER_MASK) >> EINT_PRESCALER_SHIFT;
        switch (prescalar) {
        case 0:
            deb = dbnc >> 5 ;    /* 0.5 actually, but we don't allow user to set. */
            break;
        case 1:
            deb = dbnc >> 4;
            break;
        case 2:
            deb = dbnc >> 3;
            break;
        case 3:
            deb = dbnc >> 2;
            break;
        case 4:
            deb = dbnc >> 1;
            break;
        case 5:
            deb = dbnc;
            break;
        case 6:
            deb = dbnc << 1;
            break;
        case 7:
            deb = dbnc << 2;
            break;
        default:
            deb = 0;
            printk("invalid deb time in the EIN_CON register, dbnc:%d, deb:%d\n", dbnc, deb);
            break;
        }
        printk("prescalar=0x%x, dbnc=0x%x,deb=0x%x\n",prescalar,dbnc, deb);
    return deb;
}

/*
 * mt_eint_set_polarity: Set the polarity for the EINT number.
 * @eint_num: EINT number to set
 * @pol: polarity to set
 */

void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol)
{
    unsigned int count;
    unsigned int base;
    unsigned int bit = 1 << (eint_num % 32);

    if (pol == MT_EINT_POL_NEG) {
        if (eint_num < EINT_AP_MAXNUMBER) {
            base = EINT_CON(eint_num);
            mt65xx_reg_sync_writel(readl(IOMEM(base)) & (~EINT_POL_BIT) , base);
        } else {
            dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
            return;
        }
    } else {
        if (eint_num < EINT_AP_MAXNUMBER) {
            base = EINT_CON(eint_num);
            mt65xx_reg_sync_writel(readl(IOMEM(base)) | EINT_POL_BIT , base);
        } else {
            dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
            return;
        }
    }
    
    for (count = 0; count < 250; count++) ;

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = EINT_INTACK;
    } else {
        dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return;
    }
    mt65xx_reg_sync_writel(bit, base);
    dbgmsg("[EINT] %s :%x, bit: %x\n", __func__, base, bit);
}

/*
 * mt_eint_get_mask: To get the eint mask
 * @eint_num: the EINT number to get
 */
static unsigned int mt_eint_get_mask(unsigned int eint_num)
{
    unsigned int base;
    unsigned int st;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = EINT_MASK;
    } else {
        dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return 0;
    }

    st = readl(IOMEM(base));
    if (st & bit) {
        st = 1; //masked
    } else {
        st = 0; //unmasked
    }

    return st;
}


/*
 * mt_eint_get_polarity: Set the polarity for the EINT number.
 * @eint_num: EINT number to get
 * Return: polarity type
 */
unsigned int mt_eint_get_polarity(unsigned int eint_num)
{
    unsigned int val;
    unsigned int base;
    unsigned int bit = EINT_POL_BIT;
    unsigned int pol;

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = EINT_CON(eint_num);
    } else {
        dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return 0;
    }
    val = readl(IOMEM(base));

    dbgmsg("[EINT] %s :%x, bit:%x, val:%x\n", __func__, base, bit, val);
    if (val & bit) {
        pol = MT_EINT_POL_POS;
    } else {
        pol = MT_EINT_POL_NEG;
    }
    return pol;
}

/*
 * mt_eint_get_sens: To get the eint sens
 * @eint_num: the EINT number to get
 */
static unsigned int mt_eint_get_sens(unsigned int eint_num)
{
    unsigned int base, sens;
    unsigned int bit = 1 << (eint_num % 32), st;

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = EINT_SENS;
    } else {
        dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return 0;
    }
    st = readl(IOMEM(base));
    if (st & bit) {
        sens = MT_LEVEL_SENSITIVE;
    } else {
        sens = MT_EDGE_SENSITIVE;
    }
    return sens;
}


/*
 * mt65xx_eint_print_status: Print the EINT status register.
 */
void mt_eint_print_status(void)
{
    unsigned int status;
    status = readl(EINT_STA);
    printk("EINT_STA = 0x%x\n", status);
}

/*
 * mt_eint_mask: Mask the specified EINT number.
 * @eint_num: EINT number to mask
 */
void mt_eint_mask(unsigned int eint_num)
{
    mt65xx_reg_sync_writel(1 << eint_num, EINT_MASK_SET);
}

/*
 * mt_deint_mask: Mask the specified direct EINT number.
 * @eint_num: EINT number to mask
 */
#if 0
void mt_deint_mask(unsigned int eint_num)
{
    mt65xx_reg_sync_writel(1 << eint_num, EINT_DMASK_SET);
}
#endif
/*
 * mt_eint_unmask: Unmask the specified EINT number.
 * @eint_num: EINT number to unmask
 */
void mt_eint_unmask(unsigned int eint_num)
{
    mt65xx_reg_sync_writel(1 << eint_num, EINT_MASK_CLR);
}

/*
 * mt_deint_unmask: Unmask the specified EINT number.
 * @eint_num: EINT number to unmask
 */
#if 0
void mt_deint_unmask(unsigned int eint_num)
{
    mt65xx_reg_sync_writel(1 << eint_num, EINT_DMASK_CLR);
}
#endif
/*
 * mt_eint_en_hw_debounce: To enable hw debounce
 * @eint_num: the EINT number to set
 */
static void mt_eint_en_hw_debounce(unsigned int eint_num)
{
    unsigned int base, bit;
    base = EINT_CON(eint_num);
    bit = readl(IOMEM(base))|EINT_DEB_EN_BIT;
    writel(bit, IOMEM(base));
    EINT_FUNC.is_deb_en[eint_num] = 1;
}


/*
 * mt_eint_set_hw_debounce: Set the hardware de-bounce time for the specified EINT number.
 * @eint_num: EINT number to acknowledge
 * @ms: the de-bounce time to set (in miliseconds)
 */
void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms)
{
    unsigned int cnt, prescaler;
    unsigned int mask; 
    EINT_FUNC.deb_time[eint_num] = ms;
    if (ms == 0) {
        cnt = 1;
        prescaler = 0;
    } else if (ms < MAX_MS_FOR_PRESCALER_0) {
        /* calculate cnt value based on 32KHz clock cycles */
        cnt = ms << 5;
        prescaler = 0;
    } else if (ms < MAX_MS_FOR_PRESCALER_1) {
        /* calculate cnt value based on 16KHz clock cycles */
        cnt = ms << 4;
        prescaler = 1;
    } else if (ms < MAX_MS_FOR_PRESCALER_2) {
        /* calculate cnt value based on 8KHz clock cycles */
        cnt = ms << 3;
        prescaler = 2;
    } else if (ms < MAX_MS_FOR_PRESCALER_3) {
        /* calculate cnt value based on 4KHz clock cycles */
        cnt = ms << 2;
        prescaler = 3;
    } else if (ms < MAX_MS_FOR_PRESCALER_4) {
        /* calculate cnt value based on 2KHz clock cycles */
        cnt = ms << 1;
        prescaler = 4;
    } else if (ms < MAX_MS_FOR_PRESCALER_5) {
        /* calculate cnt value based on 1KHz clock cycles */
        cnt = ms;
        prescaler = 5;
    } else if (ms < MAX_MS_FOR_PRESCALER_6) {
        /* calculate cnt value based on 512Hz clock cycles */
        cnt = ms >> 1;
        prescaler = 6;
    } else if (ms < MAX_MS_FOR_PRESCALER_7) {
        /* calculate cnt value based on 256Hz clock cycles */
        cnt = ms >> 2;
        prescaler = 7;
    } else {
        ms = MAX_MS_FOR_PRESCALER_7;
        /* calculate cnt value based on 256Hz clock cycles */
        cnt = ms >> 2;
        prescaler = 7;
    }
    cnt &= EINT_DEB_CNT_MASK;

    /* step 0: Restore the Mask setting*/
    mask = readl(EINT_MASK);
    
    /* step 1: mask the EINT */
    writel(1 << eint_num, EINT_MASK_SET);
    
    /* step 2.1: set hw debounce flag*/
    EINT_FUNC.is_deb_en[eint_num] = 1;

    /* step 2: disable debounce */
    mt65xx_reg_sync_writel(readl(EINT_CON(eint_num)) & ~EINT_DEB_EN_BIT, EINT_CON(eint_num));


    /* step 3: set new debounce value */
    writel((readl(EINT_CON(eint_num)) & ~(EINT_DEB_PRESCALER_MASK | EINT_DEB_CNT_MASK)) | \
(prescaler << EINT_PRESCALER_SHIFT) | (cnt & EINT_DEB_CNT_MASK)|EINT_DEB_EN_BIT, EINT_CON(eint_num));

    /* step 4: delay at least 5 32k cycles */
    udelay(150);

    /* step 5: reset hw debounce counter */
    mt65xx_reg_sync_writel(readl(EINT_CON(eint_num)) | EINT_DEB_RST_BIT, EINT_CON(eint_num));

    /* step 6: delay for reseting hw debounce counter */
    udelay(150);


    /* step 7: unmask the EINT if need*/
    if(mask & (1 << eint_num))
        mt65xx_reg_sync_writel(1 << eint_num, EINT_MASK_CLR);
}

/*
 * mt_eint_set_sens: Set the sensitivity for the EINT number.
 * @eint_num: EINT number to set
 * @sens: sensitivity to set
 * Always return 0.
 */
unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens)
{
    if (sens == MT_EDGE_SENSITIVE) {
        mt65xx_reg_sync_writel(1 << eint_num, EINT_SENS_CLR);
    } else if (sens == MT_LEVEL_SENSITIVE) {
        mt65xx_reg_sync_writel(1 << eint_num, EINT_SENS_SET);
    }

    return 0;
}

/*
 * eint_do_tasklet: EINT tasklet function.
 * @unused: not use.
 */
static void eint_do_tasklet(unsigned long unused)
{
    wake_lock_timeout(&EINT_suspend_lock, HZ / 2);
}

DECLARE_TASKLET(eint_tasklet, eint_do_tasklet, 0);

/*
 * mt_eint_isr: EINT interrupt service routine.
 * @irq: EINT IRQ number
 * @dev_id:
 * Return IRQ returned code.
 */
static irqreturn_t mt_eint_isr(int irq, void *dev_id)
{
    unsigned int index;
    unsigned int status;
    unsigned int domain;
    /* 
     * NoteXXX: Need to get the wake up for 0.5 seconds when an EINT intr tirggers.
     *          This is used to prevent system from suspend such that other drivers
     *          or applications can have enough time to obtain their own wake lock.
     *          (This information is gotten from the power management owner.)
     */
    tasklet_schedule(&eint_tasklet);

    status = readl(EINT_STA);
    domain = readl(EINT_D0EN);
    dbgmsg("EINT Module - %s ISR Start\n", __func__);
    dbgmsg("EINT Module - EINT_STA = 0x%x\n", status);

    for (index = 0; index < EINT_MAX_CHANNEL; index++) {
        if ((status & (1 << (index))) && (domain & (1 << (index)))) {
            mt_eint_mask(index);
            if (EINT_FUNC.eint_func[index]) {
                EINT_FUNC.eint_func[index]();
            }

            mt65xx_reg_sync_writel(1 << index, EINT_INTACK);

#if(EINT_DEBUG == 1)
            status = readl(EINT_STA);
            printk("EINT Module - EINT_STA after ack = 0x%x\n", status);
#endif

            if (EINT_FUNC.eint_auto_umask[index]) {
                mt_eint_unmask(index);
            }
        }
    }

    dbgmsg("EINT Module - %s ISR END\n", __func__);

    return IRQ_HANDLED;
}

#if 0
/*
 * mt_deint_isr: Direct EINT interrupt service routine.
 * @irq: EINT IRQ number
 * @dev_id: Direct EINT group ID, 0 ~ 7
 * Return IRQ returned code.
 */
static irqreturn_t mt_deint_isr(int irq, void *dev_id)
{
    unsigned int index;
    unsigned int status;
    const int iGroup = dev_id ? *(deint_group *)dev_id : -1;
    
    if (iGroup < 0) {
        printk(KERN_ERR "DEINT IRQ GROUP IS NOT VALID!!\n");
    }

    /* 
     * NoteXXX: Need to get the wake up for 0.5 seconds when an EINT intr tirggers.
     *          This is used to prevent system from suspend such that other drivers
     *          or applications can have enough time to obtain their own wake lock.
     *          (This information is gotten from the power management owner.)
     */
    /* FIXME: use the same tasklet?? */
    tasklet_schedule(&eint_tasklet);

    status = readl(EINT_STA);

    dbgmsg("DEINT Module - %s ISR Start\n", __func__);
    dbgmsg("DEINT Module - EINT_STA = 0x%x\n", status);

    for (index = iGroup * EINT_MAX_CHANNEL / DEINT_GROUP_MAX; index < (iGroup + 1) * EINT_MAX_CHANNEL / DEINT_GROUP_MAX ; index++) {
        if (status & (1 << (index))) {
            mt_deint_mask(index);

            if (DEINT_FUNC.eint_func[index]) {
                DEINT_FUNC.eint_func[index]();
            }

            mt65xx_reg_sync_writel(1 << index, EINT_INTACK);

#if(EINT_DEBUG == 1)
            status = readl(EINT_STA);
            printk("EINT Module - EINT_STA after ack = 0x%x\n", status);
#endif

            if (DEINT_FUNC.eint_auto_umask[index]) {
                mt_deint_unmask(index);
            }
        }
    }

    dbgmsg("DEINT Module - %s ISR END\n", __func__);

    return IRQ_HANDLED;
}
#endif


/*
 * mt_eint_ack: To ack the interrupt
 * @eint_num: the EINT number to set
 */
static unsigned int mt_eint_ack(unsigned int eint_num)
{
    unsigned int base;
    unsigned int bit = 1 << (eint_num % 32);

    if (eint_num < EINT_AP_MAXNUMBER) {
        base = (eint_num / 32) * 4 + EINT_INTACK;
    } else {
        dbgmsg("Error in %s [EINT] num:%d is larger than EINT_AP_MAXNUMBER\n", __func__, eint_num);
        return 0;
    }
    mt65xx_reg_sync_writel(bit, base);

    dbgmsg("[EINT] %s :%x, bit: %x\n", __func__, base, bit);
    return 0;
}

#if 0
/*
 * mt_deint_registration: register a direct EINT.
 * @eint_num: the EINT number to register
 * @pol: polarity value
 * @EINT_FUNC_PTR: the ISR callback function
 * @is_auto_unmask: the indication flag of auto unmasking after ISR callback is processed
 */
static void mt_deint_registration(unsigned int eint_num, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask)
{
    if (pol == MT_EINT_POL_POS) { 
        mt65xx_reg_sync_writel(readl(EINT_CON(eint_num)) | EINT_POL_BIT, EINT_CON(eint_num));
    } else {
        mt65xx_reg_sync_writel(readl(EINT_CON(eint_num)) & ~EINT_POL_BIT, EINT_CON(eint_num));
    }

    DEINT_FUNC.eint_func[eint_num] = EINT_FUNC_PTR;
    DEINT_FUNC.eint_auto_umask[eint_num] = is_auto_umask;

    writel(1 << eint_num, EINT_INTACK);
    mt65xx_reg_sync_writel(1 << eint_num, EINT_DMASK_CLR);
}
#endif


/*
 * mt65xx_eint_init: initialize EINT driver.
 * Always return 0.
 */
typedef struct{
  char name[24];
  int  eint_num;
}MD_EINT_INFO;

#define MD_EINT_MAX 3
MD_EINT_INFO md_info[MD_EINT_MAX];
#if 1
int get_td_eint_num(char *td_name, int len)
{
  unsigned int i;

  dbgmsg("[get_td_eint] name = %s\n", td_name);
  
  for(i=0; i<MD_EINT_MAX; i++)
  {
    if (!strncmp(td_name, md_info[i].name, len))
    {
      return md_info[i].eint_num;
    }
  }
  return -1;
}
#endif
typedef struct{
  char  name[24];
  int   eint_num;
  int   eint_deb;
  int   eint_pol;
  int   eint_sens;
  int   socket_type;
}MD_SIM_HOTPLUG_INFO;

#define MD_SIM_MAX 2
MD_SIM_HOTPLUG_INFO md_sim_info[MD_SIM_MAX];
unsigned int md_sim_counter = 0;

typedef enum
{
    SIM_HOT_PLUG_EINT_NUMBER,
    SIM_HOT_PLUG_EINT_DEBOUNCETIME,
    SIM_HOT_PLUG_EINT_POLARITY,
    SIM_HOT_PLUG_EINT_SENSITIVITY,
    SIM_HOT_PLUG_EINT_SOCKETTYPE,
}sim_hot_plug_eint_queryType;

typedef enum
{
    ERR_SIM_HOT_PLUG_NULL_POINTER=-13,
    ERR_SIM_HOT_PLUG_QUERY_TYPE,
    ERR_SIM_HOT_PLUG_QUERY_STRING,
}sim_hot_plug_eint_queryErr;

int get_eint_attribute(char *name, unsigned int name_len, unsigned int type, char *result, unsigned int *len)
{
    int i;
    int ret = 0;
    int *sim_info = (int *)result;
    
    if (len == NULL || name == NULL || result == NULL)
    	return ERR_SIM_HOT_PLUG_NULL_POINTER;

    for (i = 0; i < md_sim_counter; i++){
        if (!strncmp(name, md_sim_info[i].name, name_len))
        {
            switch(type)
            {
                case SIM_HOT_PLUG_EINT_NUMBER:
                    *len = sizeof(md_sim_info[i].eint_num);
                    memcpy(sim_info, &md_sim_info[i].eint_num, *len);
                    break;

                case SIM_HOT_PLUG_EINT_DEBOUNCETIME:
                    *len = sizeof(md_sim_info[i].eint_deb);
                    memcpy(sim_info, &md_sim_info[i].eint_deb, *len);
                    break;
                    
                case SIM_HOT_PLUG_EINT_POLARITY:
                    *len = sizeof(md_sim_info[i].eint_pol);
                    memcpy(sim_info, &md_sim_info[i].eint_pol, *len);
                    break;
                    
                case SIM_HOT_PLUG_EINT_SENSITIVITY:
                    *len = sizeof(md_sim_info[i].eint_sens);
                    memcpy(sim_info, &md_sim_info[i].eint_sens, *len);
                    break;
                    
                case SIM_HOT_PLUG_EINT_SOCKETTYPE:
                    *len = sizeof(md_sim_info[i].socket_type);
                    memcpy(sim_info, &md_sim_info[i].socket_type, *len);
                    break;
                    
                default:
                    ret = ERR_SIM_HOT_PLUG_QUERY_TYPE;
                    *len = sizeof(int);
                    memset(sim_info, 0xff, *len);
                    break;
            }
            return ret;
        }
    }

    *len = sizeof(int);
    memset(sim_info, 0xff, *len);
 
    return ERR_SIM_HOT_PLUG_QUERY_STRING;
}
#if 1
//This function unmasks Direct External Interrupt
static void DEINT_Unmask(unsigned int deint_no)
{
  writel(1<<deint_no, EINT_DMASK_CLR);
}
#endif

static int mt_eint_max_channel(void)
{
    return EINT_MAX_CHANNEL;
}
/*
 * mt_eint_dis_debounce: To disable debounce.
 * @eint_num: the EINT number to disable
 */

static void mt_eint_dis_debounce(unsigned int eint_num)
{
        mt_eint_dis_hw_debounce(eint_num);
}

/*
 * mt_eint_registration: register a EINT.
 * @eint_num: the EINT number to register
 * @flag: the interrupt line behaviour to select
 * @EINT_FUNC_PTR: the ISR callback function
 * @is_auto_unmask: the indication flag of auto unmasking after ISR callback is processed
 */
void mt_eint_registration(unsigned int eint_num, unsigned int flag,
              void (EINT_FUNC_PTR) (void), unsigned int is_auto_umask)
{
    if (eint_num < EINT_MAX_CHANNEL) {
        mt_eint_mask(eint_num);

        if (flag & (EINTF_TRIGGER_RISING | EINTF_TRIGGER_FALLING)) {
            mt_eint_set_polarity(eint_num, (flag & EINTF_TRIGGER_FALLING) ? MT_EINT_POL_NEG : MT_EINT_POL_POS);
            mt_eint_set_sens(eint_num, MT_EDGE_SENSITIVE);
        } else if (flag & (EINTF_TRIGGER_HIGH | EINTF_TRIGGER_LOW)) {
            mt_eint_set_polarity(eint_num, (flag & EINTF_TRIGGER_LOW) ? MT_EINT_POL_NEG : MT_EINT_POL_POS);
            mt_eint_set_sens(eint_num, MT_LEVEL_SENSITIVE);
        } else {
            printk("[EINT]: Wrong EINT Pol/Sens Setting 0x%x\n", flag);
            return ;
        }

        EINT_FUNC.eint_func[eint_num] = EINT_FUNC_PTR;
        EINT_FUNC.eint_auto_umask[eint_num] = is_auto_umask;
        mt_eint_ack(eint_num);
        mt_eint_unmask(eint_num);
    } else {
        printk("[EINT]: Wrong EINT Number %d\n", eint_num);
    }
}

static int __init mt_eint_init(void)
{
    unsigned int i;
    int ret;
    unsigned int ap_domain = 0xFFFFFFFF;
#if defined(CUST_EINT_AST_DATA_INTR_NUM) || defined(CUST_EINT_AST_WAKEUP_INTR_NUM) || defined(CUST_EINT_AST_RFCONFLICT_INTR_NUM)
    unsigned int md_counter = 0;
#endif

#if defined(MTK_SIM1_SOCKET_TYPE) || defined(MTK_SIM2_SOCKET_TYPE)
    char *p;
    int type;
#endif
    struct mt_eint_driver *eint_drv;
#ifdef CUST_EINT_AST_DATA_INTR_NUM
    ap_domain &= ~(1<<CUST_EINT_AST_DATA_INTR_NUM);
    sprintf(md_info[md_counter].name, "AST_DATA_INTR");
    md_info[md_counter].eint_num = CUST_EINT_AST_DATA_INTR_NUM;
    mt_eint_set_sens(CUST_EINT_AST_DATA_INTR_NUM, CUST_EINT_AST_DATA_INTR_SENSITIVE);
    mt_eint_set_polarity(CUST_EINT_AST_DATA_INTR_NUM, CUST_EINT_AST_DATA_INTR_POLARITY);
    DEINT_Unmask(CUST_EINT_AST_DATA_INTR_NUM);
    dbgmsg("[EINT] name = %s\n", md_info[md_counter].name);
    md_counter++;
#endif
#ifdef CUST_EINT_AST_WAKEUP_INTR_NUM
    ap_domain &= ~(1<<CUST_EINT_AST_WAKEUP_INTR_NUM);
    sprintf(md_info[md_counter].name, "AST_WAKEUP_INTR");
    md_info[md_counter].eint_num = CUST_EINT_AST_WAKEUP_INTR_NUM;
    mt_eint_set_sens(CUST_EINT_AST_WAKEUP_INTR_NUM, CUST_EINT_AST_WAKEUP_INTR_SENSITIVE);
    mt_eint_set_polarity(CUST_EINT_AST_WAKEUP_INTR_NUM, CUST_EINT_AST_WAKEUP_INTR_POLARITY);
    DEINT_Unmask(CUST_EINT_AST_WAKEUP_INTR_NUM);
    dbgmsg("[EINT] name = %s\n", md_info[md_counter].name);
    md_counter++;
#endif
#ifdef CUST_EINT_AST_RFCONFLICT_INTR_NUM
    ap_domain &= ~(1<<CUST_EINT_AST_RFCONFLICT_INTR_NUM);
    sprintf(md_info[md_counter].name, "AST_RFCONFLICT_INTR");
    md_info[md_counter].eint_num = CUST_EINT_AST_RFCONFLICT_INTR_NUM;
    mt_eint_set_sens(CUST_EINT_AST_RFCONFLICT_INTR_NUM, CUST_EINT_AST_RFCONFLICT_INTR_SENSITIVE);
    mt_eint_set_polarity(CUST_EINT_AST_RFCONFLICT_INTR_NUM, CUST_EINT_AST_RFCONFLICT_INTR_POLARITY);
    DEINT_Unmask(CUST_EINT_AST_RFCONFLICT_INTR_NUM);
    dbgmsg("[EINT] name = %s\n", md_info[md_counter].name);
    md_counter++;
#endif

#ifdef MTK_SIM1_SOCKET_TYPE
    p = (char *)MTK_SIM1_SOCKET_TYPE;
    type = simple_strtoul(p, &p, 10);
    if (type > 0){
#if (CUST_EINT_SIM1_HOT_PLUG_NUM!=15)
#warning CUST_EINT_SIM1_HOT_PLUG_NUM should be 15
#else
        ap_domain &= ~(1<<CUST_EINT_SIM1_HOT_PLUG_NUM);
        sprintf(md_sim_info[md_sim_counter].name, "SIM1_HOT_PLUG_EINT");
        md_sim_info[md_sim_counter].eint_num = CUST_EINT_SIM1_HOT_PLUG_NUM;
        md_sim_info[md_sim_counter].eint_pol= CUST_EINT_SIM1_HOT_PLUG_POLARITY;
        md_sim_info[md_sim_counter].eint_sens = CUST_EINT_SIM1_HOT_PLUG_SENSITIVE;
        md_sim_info[md_sim_counter].socket_type = type;
        md_sim_info[md_sim_counter].eint_deb = CUST_EINT_SIM1_HOT_PLUG_DEBOUNCE_CN;
        dbgmsg("[EINT] SIM1 name = %s\n", md_sim_info[md_sim_counter].name);
        md_sim_counter++;
#endif
    }
#endif

#ifdef MTK_SIM2_SOCKET_TYPE
    p = (char *)MTK_SIM2_SOCKET_TYPE;
    type = simple_strtoul(p, &p, 10);
    if (type > 0){
#if (CUST_EINT_SIM2_HOT_PLUG_NUM!=13)
#warning CUST_EINT_SIM2_HOT_PLUG_NUM should be 13
#else 
        ap_domain &= ~(1<<CUST_EINT_SIM2_HOT_PLUG_NUM);
        sprintf(md_sim_info[md_sim_counter].name, "SIM2_HOT_PLUG_EINT");
        md_sim_info[md_sim_counter].eint_num = CUST_EINT_SIM2_HOT_PLUG_NUM;
        md_sim_info[md_sim_counter].eint_pol= CUST_EINT_SIM2_HOT_PLUG_POLARITY;
        md_sim_info[md_sim_counter].eint_sens = CUST_EINT_SIM2_HOT_PLUG_SENSITIVE;
        md_sim_info[md_sim_counter].socket_type = type;
        md_sim_info[md_sim_counter].eint_deb = CUST_EINT_SIM2_HOT_PLUG_DEBOUNCE_CN;
        dbgmsg("[EINT] SIM2 name = %s\n", md_sim_info[md_sim_counter].name);
        md_sim_counter++;
#endif        
    }
#endif

    wake_lock_init(&EINT_suspend_lock, WAKE_LOCK_SUSPEND, "EINT wakelock");

    for (i = 0; i< EINT_MAX_CHANNEL; i++) {
        EINT_FUNC.eint_func[i] = NULL;
        EINT_FUNC.eint_auto_umask[i] = 0;
    }

    
    /* assign to domain 0 for AP */
    dbgmsg("[EINT]ap_domain = %x\n", ap_domain);
    mt65xx_reg_sync_writel(ap_domain, EINT_D0EN);

    /* normal EINT IRQ registration */
    if (request_irq(EINT_IRQ, mt_eint_isr, IRQF_TRIGGER_HIGH, "EINT", NULL)) {
        printk(KERN_ERR "EINT IRQ LINE NOT AVAILABLE!!\n");
    }

#if 0
    /* direct EINT IRQs registration */
    for (i = 0; i < DEINT_GROUP_MAX; i++) {
        if (request_irq(MT_EINT_DIRECT0_IRQ_ID + i, mt_deint_isr, IRQF_TRIGGER_HIGH, "DEINT", (void *)&deint_group_id[i])) {
            printk(KERN_ERR "DEINT IRQ LINE NOT AVAILABLE!!\n");
        }
    }
#endif


    /* register EINT driver */
    eint_drv = get_mt_eint_drv();
    eint_drv->eint_max_channel = mt_eint_max_channel;
    eint_drv->enable = mt_eint_unmask;
    eint_drv->disable = mt_eint_mask;
    eint_drv->is_disable = mt_eint_get_mask;
    eint_drv->get_sens =  mt_eint_get_sens;
    eint_drv->set_sens = mt_eint_set_sens;
    eint_drv->get_polarity = mt_eint_get_polarity;
    eint_drv->set_polarity = mt_eint_set_polarity;
    eint_drv->get_debounce_cnt =  mt_eint_get_debounce_cnt;
    eint_drv->set_debounce_cnt = mt_eint_set_hw_debounce;
    eint_drv->is_debounce_en = mt_eint_is_debounce_en;
    eint_drv->enable_debounce = mt_eint_enable_debounce;
    eint_drv->disable_debounce = mt_eint_disable_debounce;

    return 0;
}

arch_initcall(mt_eint_init);

EXPORT_SYMBOL(mt_eint_dis_debounce);
EXPORT_SYMBOL(mt_eint_registration);
EXPORT_SYMBOL(mt_eint_set_hw_debounce);
EXPORT_SYMBOL(mt_eint_set_polarity);
EXPORT_SYMBOL(mt_eint_set_sens);
EXPORT_SYMBOL(mt_eint_mask);
EXPORT_SYMBOL(mt_eint_unmask);
EXPORT_SYMBOL(mt_eint_print_status);

//EXPORT_SYMBOL(get_td_eint_num);
