#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kallsyms.h>
#include <asm/cacheflush.h>
#include <asm/outercache.h>
#include <asm/system.h>
#include <mach/mt_reg_base.h>
#include <asm/hardware/cache-l2x0.h>
#include <mach/mt_clock_manager.h>
#include <mach/mt_dramc.h>
#include <linux/module.h>


#define DRAMC_PD_CTRL      (DRAM_CTR_BASE + 0x01DC)
#define DRAMC_PHYCTL1      (DRAM_CTR_BASE + 0x00F0)
#define DRAMC_GDDR3CTL1    (DRAM_CTR_BASE + 0x00F4)
#define REG_DRAMC_PD_CTRL  *((volatile unsigned int*)DRAMC_PD_CTRL)

#ifdef CONFIG_CACHE_L2X0
// For avoiding imprecise external abort [L2 Slave Error]
// In order to use l2x0_lock, the lock must be exported from arch/arm/mm/cache-l2x0.c
extern raw_spinlock_t l2x0_lock;
#endif

static spinlock_t dram_lock;
static unsigned int default_free_run_clock;
static unsigned int default_dram_clock;
static unsigned int cur_dram_clk;
extern void *mtk_uart_update_sysclk(void)__attribute__((weak));
int get_ddr_type (void)
{
    return (*(volatile unsigned int *)(DRAM_CTR_BASE + 0x00D8) & 0xC0000000)? ((*(volatile unsigned int *)(APMIXED_BASE + 0x0204) & 0x00000020) ? 1 : 3): 2;
}

static int lpddr_downgrade_clock(int clk)
{
    char str[KSYM_SYMBOL_LEN];
    unsigned long size = 0;
    unsigned long flags;
    unsigned int addr, end, data;
    register int delay, reg_pll_con0;

    switch (clk)
    {
        case 156 ... 160:
            reg_pll_con0 = 0x1D60;
            cur_dram_clk = 156;
            break;
        case 161 ... 165:
            reg_pll_con0 = 0x1E60;
            cur_dram_clk = 161;
            break;
        case 166 ... 170:
            reg_pll_con0 = 0x1F60;
            cur_dram_clk = 166;
            break;
        case 171 ... 175:
            reg_pll_con0 = 0x2060;
            cur_dram_clk = 171;
            break;
        case 176 ... 181:
            reg_pll_con0 = 0x2160;
            cur_dram_clk = 176;
            break;
        case 182 ... 186:
            reg_pll_con0 = 0x2260;
            cur_dram_clk = 182;
            break;
        case 187 ... 191:
            reg_pll_con0 = 0x2360;
            cur_dram_clk = 187;
            break;
        case 192 ... 196:
            reg_pll_con0 = 0x2460;
            cur_dram_clk = 192;
            break;
        default:
            return -1;
            
    }

    kallsyms_lookup((unsigned long)lpddr_downgrade_clock, &size, NULL, NULL, str);
    addr = (unsigned int)lpddr_downgrade_clock;
    /* preload addtional 256 bytes to cover CPU/cache prefetch */
    end = addr + size + 256;

    /*
     * NoteXXX: The DRAM controller is not allowed to change the DRAM clock rate directly. Need to follow below procedures:
     *          1. Flush cache.
     *          2. Run code internally to enforce not to access DRAM. 
     *             (Lockdown code and data in the cache.)
     *          3. Memory barrier.
     *          4. DRAM enters the self refresh state. 
     *          5. Change the clock rate.
     *          6. Set new AC timing parameters.
     *          7. DRAM leaves the self refresh state.
     */

    spin_lock_irqsave(&dram_lock, flags);

    flush_cache_all();
    outer_flush_all();

    /* disable L1 I cache */
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        "bic %0, %0, #1<<12\n\t"
        "mcr p15, 0, %0, c1, c0, 0\n\t"
        : "=r" (delay)
        );

    for (; addr < end; addr += 4) {
        data = *(volatile unsigned int *)addr;
    }

#ifdef CONFIG_CACHE_L2X0
    raw_spin_lock(&l2x0_lock); 
    *(volatile unsigned int *)(PL310_BASE + L2X0_LOCKDOWN_WAY_D_BASE) = 0x0000FFFF;
    *(volatile unsigned int *)(PL310_BASE + L2X0_LOCKDOWN_WAY_I_BASE) = 0x0000FFFF;

    dsb();
    *(volatile unsigned int*)(PL310_BASE + L2X0_CACHE_SYNC) = 0;
    raw_spin_unlock(&l2x0_lock); 
#endif
	
    /* enter DRAM self-refresh */
    *(volatile unsigned int *)(DRAM_CTR_BASE + 0x0004) |= 0x04000000;

    /* delay for at least 10us */
    asm volatile (
        "mov %0, #2\n"
        "mov %0, %0, LSL #15\n"
        "1:\n"
        "subs %0, %0, #1\n"
        "bne 1b\n"
        : "=r" (delay)
        :
        : "cc"
    );

    /* overclock */
#if 1
    /*
     * NoteXXX: Cannot change the EMI's and DRAMC's clock rate directly.
     *          Need to set the PLL to change the clock rate first.
     *          Then use the jitter-free MUX to switch the clock source.
     */
    *(volatile unsigned int *)MAINPLL_CON0 = reg_pll_con0;
    asm volatile (
        "mov %0, #2\n"
        "mov %0, %0, LSL #15\n"
        "1:\n"
        "subs %0, %0, #1\n"
        "bne 1b\n"
        : "=r" (delay)
        :
        : "cc"
    );
    *(volatile unsigned int *)TOP_CKDIV0 = 0x10;
#endif

    /* PHY RESET */
    *(volatile unsigned int *)DRAMC_PHYCTL1 |= (1 << 28); 
    *(volatile unsigned int *)DRAMC_GDDR3CTL1 |= (1 << 25);

    asm volatile (
        "mov %0, #2\n"
        "mov %0, %0, LSL #10\n"
        "1:\n"
        "subs %0, %0, #1\n"
        "bne 1b\n"
        : "=r" (delay)
        :
        : "cc"
    );
    *(volatile unsigned int *)DRAMC_PHYCTL1 &= ~(1 << 28); 
    *(volatile unsigned int *)DRAMC_GDDR3CTL1 &= ~(1 << 25);

    /* exit DRAM self-refresh */
    *(volatile unsigned int *)(DRAM_CTR_BASE + 0x0004) &= ~0x04000000;

#ifdef CONFIG_CACHE_L2X0
    raw_spin_lock(&l2x0_lock); 

    *(volatile unsigned int *)(PL310_BASE + L2X0_LOCKDOWN_WAY_D_BASE) = 0x00000000;
    *(volatile unsigned int *)(PL310_BASE + L2X0_LOCKDOWN_WAY_I_BASE) = 0x00000000;

    raw_spin_unlock(&l2x0_lock); 
#endif
	
    /* enable L1 I cache */
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        "orr %0, %0, #1<<12\n\t"
        "mcr p15, 0, %0, c1, c0, 0\n\t"
        : "=r" (delay)
        );

    spin_unlock_irqrestore(&dram_lock, flags);


    if(mtk_uart_update_sysclk)
    {
       mtk_uart_update_sysclk();
        /* END */
    }
    else
    {
      printk("UART Driver has not provide mtk_uart_update_sysclk\r\nPlease contact with Haow Wang \n");
    }

    return 0;
}

static int lpddr2_downgrade_clock(int clk)
{
    char str[KSYM_SYMBOL_LEN];
    unsigned long size = 0;
    unsigned long flags;
    unsigned int addr, end, data;
    register int delay, reg_pll_con0;

    switch (clk)
    {
        case 195 ... 200:
            reg_pll_con0 = 0x1D60;
            cur_dram_clk = 195;
            break;
        case 201 ... 207:
            reg_pll_con0 = 0x1E60;
            cur_dram_clk = 201;
            break;
        case 208 ... 213:
            reg_pll_con0 = 0x1F60;
            cur_dram_clk = 208;
            break;
        case 214 ... 220:
            reg_pll_con0 = 0x2060;
            cur_dram_clk = 214;
            break;
        case 221 ... 226:
            reg_pll_con0 = 0x2160;
            cur_dram_clk = 221;
            break;
        case 227 ... 233:
            reg_pll_con0 = 0x2260;
            cur_dram_clk = 227;
            break;
        case 234 ... 239:
            reg_pll_con0 = 0x2360;
            cur_dram_clk = 234;
            break;
        case 240 ... 246:
            reg_pll_con0 = 0x2460;
            cur_dram_clk = 240;
            break;
        case 247 ... 252:
            reg_pll_con0 = 0x2560;
            cur_dram_clk = 247;
            break;
        case 253 ... 259:
            reg_pll_con0 = 0x2660;
            cur_dram_clk = 253;
            break;
        case 260 ... 265:
            reg_pll_con0 = 0x2760;
            cur_dram_clk = 260;
            break;
        default:
            return -1;
    }

    kallsyms_lookup((unsigned long)lpddr2_downgrade_clock, &size, NULL, NULL, str);
    addr = (unsigned int)lpddr2_downgrade_clock;
    /* preload addtional 256 bytes to cover CPU/cache prefetch */
    end = addr + size + 256;

    /*
     * NoteXXX: The DRAM controller is not allowed to change the DRAM clock rate directly. Need to follow below procedures:
     *          1. Flush cache.
     *          2. Run code internally to enforce not to access DRAM. 
     *             (Lockdown code and data in the cache.)
     *          3. Memory barrier.
     *          4. DRAM enters the self refresh state. 
     *          5. Change the clock rate.
     *          6. Set new AC timing parameters.
     *          7. DRAM leaves the self refresh state.
     */

    spin_lock_irqsave(&dram_lock, flags);

    flush_cache_all();
    outer_flush_all();

    /* disable L1 I cache */
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        "bic %0, %0, #1<<12\n\t"
        "mcr p15, 0, %0, c1, c0, 0\n\t"
        : "=r" (delay)
        );

    for (; addr < end; addr += 4) {
        data = *(volatile unsigned int *)addr;
    }

#ifdef CONFIG_CACHE_L2X0
    raw_spin_lock(&l2x0_lock); 

    *(volatile unsigned int *)(PL310_BASE + L2X0_LOCKDOWN_WAY_D_BASE) = 0x0000FFFF;
    *(volatile unsigned int *)(PL310_BASE + L2X0_LOCKDOWN_WAY_I_BASE) = 0x0000FFFF;

    dsb();
    *(volatile unsigned int*)(PL310_BASE + L2X0_CACHE_SYNC) = 0;

    raw_spin_unlock(&l2x0_lock); 
#endif
	
    /* enter DRAM self-refresh */
    *(volatile unsigned int *)(DRAM_CTR_BASE + 0x0004) |= 0x04000000;

    /* delay for at least 10us */
    asm volatile (
        "mov %0, #2\n"
        "mov %0, %0, LSL #15\n"
        "1:\n"
        "subs %0, %0, #1\n"
        "bne 1b\n"
        : "=r" (delay)
        :
        : "cc"
    );

    /* overclock */
    *(volatile unsigned int *)0xF000706C = 0x0001;
    *(volatile unsigned int *)MAINPLL_CON0 = reg_pll_con0;
    asm volatile (
        "mov %0, #2\n"
        "mov %0, %0, LSL #15\n"
        "1:\n"
        "subs %0, %0, #1\n"
        "bne 1b\n"
        : "=r" (delay)
        :
        : "cc"
    );
    *(volatile unsigned int *)(TOP_CKDIV0) = 0x8;

    /* PHY RESET */
    *(volatile unsigned int *)DRAMC_PHYCTL1 |= (1 << 28); 
    *(volatile unsigned int *)DRAMC_GDDR3CTL1 |= (1 << 25);
    asm volatile (
        "mov %0, #2\n"
        "mov %0, %0, LSL #10\n"
        "1:\n"
        "subs %0, %0, #1\n"
        "bne 1b\n"
        : "=r" (delay)
        :
        : "cc"
    );
    *(volatile unsigned int *)DRAMC_PHYCTL1 &= ~(1 << 28); 
    *(volatile unsigned int *)DRAMC_GDDR3CTL1 &= ~(1 << 25);

    /* exit DRAM self-refresh */
    *(volatile unsigned int *)(DRAM_CTR_BASE + 0x0004) &= ~0x04000000;

#ifdef CONFIG_CACHE_L2X0
    raw_spin_lock(&l2x0_lock); 

    *(volatile unsigned int *)(PL310_BASE + L2X0_LOCKDOWN_WAY_D_BASE) = 0x00000000;
    *(volatile unsigned int *)(PL310_BASE + L2X0_LOCKDOWN_WAY_I_BASE) = 0x00000000;

    raw_spin_unlock(&l2x0_lock); 
#endif
	
    /* enable L1 I cache */
    asm volatile (
        "mrc p15, 0, %0, c1, c0, 0\n\t"
        "orr %0, %0, #1<<12\n\t"
        "mcr p15, 0, %0, c1, c0, 0\n\t"
        : "=r" (delay)
        );

    spin_unlock_irqrestore(&dram_lock, flags);


    if(mtk_uart_update_sysclk)
    {
       mtk_uart_update_sysclk();
        /* END */
    }
    else
    {
      printk("UART Driver has not provide mtk_uart_update_sysclk\r\nPlease contact with Haow Wang \n");
    }

    return 0;
}


int get_dram_clock(void)
{
    return cur_dram_clk;
}

int get_dram_default_clock(void)
{
    return default_dram_clock;
}

static ssize_t dram_clock_show(struct device_driver *driver, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", get_dram_clock());
}

int set_dram_clock(int clock)
{
    int ret;

    if (get_ddr_type() == 1)
    {
        ret = lpddr_downgrade_clock(clock);
    }
    else
    {
        ret = lpddr2_downgrade_clock(clock);
    }
    
    if (ret)
    {
        printk("invalid dram clock %d\n", clock);
    }

    return ret;
}

static ssize_t dram_clock_store(struct device_driver *driver, const char *buf, size_t count)
{
    int clock;

    clock = simple_strtol(buf, 0, 10);

    set_dram_clock(clock);

    return count;
}

DRIVER_ATTR(dram_clock, 0664, dram_clock_show, dram_clock_store);

int set_dram_refresh_clock(int clock)
{
    unsigned long flags;

    if (clock <= 0 || clock > 0xFF)
    {
        printk("invalid dram refresh clock\n");
        return -1;
    }

    spin_lock_irqsave(&dram_lock , flags);
    
    REG_DRAMC_PD_CTRL = (REG_DRAMC_PD_CTRL & 0xFF00FFFF) | (0 << 16);
    REG_DRAMC_PD_CTRL = (REG_DRAMC_PD_CTRL & 0xFF00FFFF) | (clock << 16);
    dsb();

    spin_unlock_irqrestore(&dram_lock , flags);

    return 0;
}

int get_dram_refresh_clock(void)
{
    return (REG_DRAMC_PD_CTRL & 0x00FF0000) >> 16;
}

int get_dram_default_refresh_clock(void)
{
    return default_free_run_clock;
}

static ssize_t dram_refresh_show(struct device_driver *driver, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", get_dram_refresh_clock());
}

static ssize_t dram_refresh_store(struct device_driver *driver, const char *buf, size_t count)
{
    int clock;

    clock = simple_strtol(buf, 0, 10);

    set_dram_refresh_clock(clock);

    return count;
}

DRIVER_ATTR(refresh_clock, 0664, dram_refresh_show, dram_refresh_store);

#define DRAMC_MIOCKCTRLOFF      (1 << 26)

unsigned int get_dram_clk_gating(void)
{
    return ((REG_DRAMC_PD_CTRL & DRAMC_MIOCKCTRLOFF) >> 26);
}

int set_dram_clk_gating(int val)
{
    if(val == 1){
        REG_DRAMC_PD_CTRL |= DRAMC_MIOCKCTRLOFF;
    } else if(val == 0) {
        REG_DRAMC_PD_CTRL &= ~DRAMC_MIOCKCTRLOFF;
    } else {
        printk("Invalid DRAMC Register Setting\n");
        return -1;
    }

    return 0;
}

static ssize_t dram_clk_gating_show(struct device_driver *driver, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%s\n", ((REG_DRAMC_PD_CTRL & DRAMC_MIOCKCTRLOFF) >> 26) == 1 ? "Always No Gating" : "Controlled by DRAMC" );
}

static ssize_t dram_clk_gating_store(struct device_driver *driver, const char *buf, size_t count)
{
    unsigned int bit;

    bit = simple_strtol(buf, 0, 10);

    if(bit == 1){
        REG_DRAMC_PD_CTRL |= DRAMC_MIOCKCTRLOFF;
    } else if(bit == 0) {
        REG_DRAMC_PD_CTRL &= ~DRAMC_MIOCKCTRLOFF;
    } else {
        printk("Invalid DRAMC Register Setting\n");
    }

    return count;
}

DRIVER_ATTR(dram_clk_gating, 0664, dram_clk_gating_show, dram_clk_gating_store);

struct dramc_driver {
    struct device_driver driver;
    const struct platform_device_id *id_table;
};

static struct dramc_driver dram_clock_drv =
{
    .driver = {
        .name = "dramc_clock_test",
        .bus = &platform_bus_type,
        .owner = THIS_MODULE,
    },
    .id_table = NULL,
};

int __init dram_clock_init(void)
{
    int ret;

    spin_lock_init (&dram_lock);
    
    default_free_run_clock = (REG_DRAMC_PD_CTRL >> 16) & 0xFF;
    cur_dram_clk = default_dram_clock = (get_ddr_type() == 1) ? 192 : 260;

    ret = driver_register(&dram_clock_drv.driver);
    if (ret)
    {
        printk("fail to create the dram_clock driver\n");
        return ret;
    }

    ret = driver_create_file(&dram_clock_drv.driver, &driver_attr_refresh_clock);
    if (ret)
    {
        printk("fail to create the dram refresh sysfs file\n");
        return ret;
    }

    ret = driver_create_file(&dram_clock_drv.driver, &driver_attr_dram_clock);
    if (ret)
    {
        printk("fail to create the dram clock sysfs file\n");
        return ret;
    }

    ret = driver_create_file(&dram_clock_drv.driver, &driver_attr_dram_clk_gating);
    if (ret)
    {
        printk("fail to create the dram clock gating sysfs file\n");
        return ret;
    }

    return 0;
}

arch_initcall(dram_clock_init);
EXPORT_SYMBOL(set_dram_refresh_clock);
EXPORT_SYMBOL(get_dram_refresh_clock);
EXPORT_SYMBOL(get_dram_default_refresh_clock);
EXPORT_SYMBOL(set_dram_clock);
EXPORT_SYMBOL(get_dram_clock);
EXPORT_SYMBOL(get_dram_default_clock);

