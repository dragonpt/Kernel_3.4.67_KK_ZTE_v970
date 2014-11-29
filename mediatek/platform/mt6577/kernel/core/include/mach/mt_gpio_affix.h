#ifndef _MT_GPIO_AFFIX_H_
#define _MT_GPIO_AFFIX_H_


#define GPIO_WR32(addr, data)   mt65xx_reg_sync_writel(data, addr)
#define GPIO_RD32(addr)         __raw_readl(addr)


/*For MD GPIO customization only, can be called by CCCI driver*/
int mt_get_md_gpio(char * gpio_name, int len);
#endif
