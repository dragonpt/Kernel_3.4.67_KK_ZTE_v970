/******************************************************************************
 * mt_gpio_debug.c - MTKLinux GPIO Device Driver
 * 
 * Copyright 2008-2009 MediaTek Co.,Ltd.
 * 
 * DESCRIPTION:
 *     This file provid the other drivers GPIO debug functions
 *
 ******************************************************************************/

#include <linux/slab.h>

#include <mach/mt_reg_base.h>
#include <mach/mt_gpio.h>
#include <mach/mt_gpio_core.h>


void mt_gpio_load(GPIO_REGS *regs) 
{
    GPIO_REGS *pReg = (GPIO_REGS*)(GPIO_BASE);
    int idx;
    
    if (!regs)
        GPIOERR("%s: null pointer\n", __func__);
    memset(regs, 0x00, sizeof(*regs));
    for (idx = 0; idx < sizeof(pReg->dir)/sizeof(pReg->dir[0]); idx++)
        regs->dir[idx].val = __raw_readl(&pReg->dir[idx]);
    for (idx = 0; idx < sizeof(pReg->pullen)/sizeof(pReg->pullen[0]); idx++)
        regs->pullen[idx].val = __raw_readl(&pReg->pullen[idx]);
    for (idx = 0; idx < sizeof(pReg->pullsel)/sizeof(pReg->pullsel[0]); idx++)
        regs->pullsel[idx].val =__raw_readl(&pReg->pullsel[idx]);
    for (idx = 0; idx < sizeof(pReg->dinv)/sizeof(pReg->dinv[0]); idx++)
        regs->dinv[idx].val =__raw_readl(&pReg->dinv[idx]);
    for (idx = 0; idx < sizeof(pReg->dout)/sizeof(pReg->dout[0]); idx++)
        regs->dout[idx].val = __raw_readl(&pReg->dout[idx]);
    for (idx = 0; idx < sizeof(pReg->mode)/sizeof(pReg->mode[0]); idx++)
        regs->mode[idx].val = __raw_readl(&pReg->mode[idx]);
    for (idx = 0; idx < sizeof(pReg->din)/sizeof(pReg->din[0]); idx++)
        regs->din[idx].val = __raw_readl(&pReg->din[idx]);
}

void gpio_dump_regs(void)
{
    int idx = 0;	
    int num, bit; 
    int mode, pullsel, din, dout, pullen, dir, dinv;
    u32 mask = (1L << GPIO_MODE_BITS) - 1;     
    GPIO_REGS *reg = (GPIO_REGS*)GPIO_BASE;   
    for (idx = 0; idx < MAX_GPIO_PIN; idx++) {
        num = idx / MAX_GPIO_REG_BITS;
        bit = idx % MAX_GPIO_REG_BITS;
        pullsel = (reg->pullsel[num].val & (1L << bit)) ? (1) : (0);
        din     = (reg->din[num].val & (1L << bit)) ? (1) : (0);
        dout    = (reg->dout[num].val & (1L << bit)) ? (1) : (0);
        pullen  = (reg->pullen[num].val & (1L << bit)) ? (1) : (0);
        dir     = (reg->dir[num].val & (1L << bit)) ? (1) : (0);
        dinv    = (reg->dinv[num].val & (1L << bit)) ? (1) : (0);
        num = idx / MAX_GPIO_MODE_PER_REG;        
        bit = idx % MAX_GPIO_MODE_PER_REG;
        mode = (reg->mode[num].val >> (GPIO_MODE_BITS*bit)) & mask;
        printk("idx = %3d: mode = %d, pullsel = %d, din = %d, dout = %d, pullen = %d, dir = %d, dinv = %d\n", idx, mode, pullsel, din, dout, pullen, dir, dinv); 
    }
}

/*----------------------------------------------------------------------------*/
static ssize_t mt_gpio_dump_regs(char *buf, ssize_t bufLen, GPIO_REGS *cur)
{
    int idx = 0, len = 0;
    int num, bit; 
    int mode, pullsel, din, dout, pullen, dir, dinv;
    u32 mask = (1L << GPIO_MODE_BITS) - 1;        
    for (idx = 0; idx < MAX_GPIO_PIN; idx++) {
        num = idx / MAX_GPIO_REG_BITS;
        bit = idx % MAX_GPIO_REG_BITS;
        pullsel = (cur->pullsel[num].val & (1L << bit)) ? (1) : (0);
        din     = (cur->din[num].val & (1L << bit)) ? (1) : (0);
        dout    = (cur->dout[num].val & (1L << bit)) ? (1) : (0);
        pullen  = (cur->pullen[num].val & (1L << bit)) ? (1) : (0);
        dir     = (cur->dir[num].val & (1L << bit)) ? (1) : (0);
        dinv    = (cur->dinv[num].val & (1L << bit)) ? (1) : (0);
        num = idx / MAX_GPIO_MODE_PER_REG;        
        bit = idx % MAX_GPIO_MODE_PER_REG;
        mode = (cur->mode[num].val >> (GPIO_MODE_BITS*bit)) & mask;
        len += snprintf(buf+len, bufLen-len, "%3d: %d %d %d %d %d %d %d\n",
               idx, mode, pullsel, din, dout, pullen, dir, dinv); 
    }
    return len;
}

ssize_t mt_gpio_show_pin(struct device* dev, 
                                struct device_attribute *attr, char *buf)
{
    GPIO_REGS *reg = (GPIO_REGS*)GPIO_BASE;
    return mt_gpio_dump_regs(buf, PAGE_SIZE, reg);;
}

ssize_t mt_gpio_store_pin(struct device* dev, struct device_attribute *attr,  
                                 const char *buf, size_t count)
{
    return count;    
}

/*---------------MD convert gpio-name to gpio-number--------*/
typedef struct md_gpio_info {
#ifdef CONFIG_ARCH_MT6577
	char m_gpio_name[34];
#endif
#ifdef CONFIG_ARCH_MT6575
	char m_gpio_name[32];
#endif
	int m_gpio_num;
}MD_GPIO_INFO;

#ifdef CONFIG_ARCH_MT6577
#define GPIO_NAME_MAX 12
#endif
#ifdef CONFIG_ARCH_MT6575
#define GPIO_NAME_MAX	9
#endif
MD_GPIO_INFO gpio_info[GPIO_NAME_MAX];

int mt_get_md_gpio(char * gpio_name, int len)
{
	unsigned int i;

	for (i = 0; i < GPIO_NAME_MAX; i++)
	{
		if (!strncmp (gpio_name, gpio_info[i].m_gpio_name, len))
		{
			GPIOMSG("gpio_info[i].m_gpio_num==%d\n", gpio_info[i].m_gpio_num);
			return (gpio_info[i].m_gpio_num);
		}
	}
	GPIOERR("There is no this gpio name!!!\n");
	return -1;
}

