#include <linux/pm.h>
#include <linux/bug.h>
#include <linux/mm.h>
#include <linux/sched.h>

#include <asm/page.h>
#include <asm/tlbflush.h>
#include <asm/smp_scu.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#include <asm/hardware/cache-l2x0.h>

#include <mach/mt_reg_base.h>
#include <mach/irqs.h>
#include <mach/memory.h>

#define PMD_SECT_NS (1 << 19)
#define PMD_TABLE_NS (1 << 3)

extern void arm_machine_restart(char mode, const char *cmd);
extern struct sys_timer mt6577_timer;
extern void mt_fixup(struct tag *tags, char **cmdline, struct meminfo *mi);
#ifndef CONFIG_EARLY_LINUX_PORTING
extern void mt_power_off(void);
#endif

void __init mt_init(void)
{
#ifdef CONFIG_ARCH_MT6577
    unsigned int tmp;
#endif

#ifndef CONFIG_EARLY_LINUX_PORTING
    pm_power_off = mt_power_off;
#endif 
    panic_on_oops = 1;
#if defined(CONFIG_CACHE_L2X0)
#ifdef CONFIG_ARCH_MT6577
    writel(L2X0_DYNAMIC_CLK_GATING_EN, PL310_BASE + L2X0_POWER_CTRL);
    writel(readl(PL310_BASE + L2X0_PREFETCH_CTRL) | 0x40000000, PL310_BASE + L2X0_PREFETCH_CTRL);

    /*L2C data ram access latency*/
    tmp = readl(PL310_BASE + L2X0_DATA_LATENCY_CTRL);
    tmp &= ~((0x7 << 4) | 0x7); // clear bit[6:4] and bit[2:0]
    tmp |= ((0x2 << 4) | 0x1); //3T read access latency & 2T setup latency

    writel(tmp, PL310_BASE + L2X0_DATA_LATENCY_CTRL);
 
    l2x0_init((void __iomem *)PL310_BASE, 0x70400000, 0x8FBFFFFF);
#else
    writel(L2X0_DYNAMIC_CLK_GATING_EN, PL310_BASE + L2X0_POWER_CTRL);
    writel(readl(PL310_BASE + L2X0_PREFETCH_CTRL) | 0x40000000, PL310_BASE + L2X0_PREFETCH_CTRL);
    l2x0_init((void __iomem *)PL310_BASE, 0x70000000, 0x8FFFFFFF);
#endif
#endif  /* CONFIG_CACHE_L2X0 */

#if defined(CONFIG_HAVE_ARM_SCU)
    scu_enable((void *)SCU_BASE);

    /* set INFRA_ACP to 0x00003333 for receiving transactions to ACP */
    writel(0x00003333, INFRA_SYS_CFG_BASE + 0x0F04);
#endif  /* CONFIG_HAVE_ARM_SCU */
}

static struct map_desc mt_io_desc[] __initdata = 
{
    {
        .virtual = AP_RGU_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(AP_RGU_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    {
        .virtual = PERICFG_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(PERICFG_BASE)),
        .length = SZ_16M,
        .type = MT_DEVICE
    },
    {
        .virtual = MMSYS1_CONFIG_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(MMSYS1_CONFIG_BASE)),
        .length = SZ_256K,
        .type = MT_DEVICE
    },
    {
        .virtual = MMSYS2_CONFIG_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(MMSYS2_CONFIG_BASE)),
        .length = SZ_32K,
        .type = MT_DEVICE
    },
    {
        .virtual = SYSRAM_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(SYSRAM_BASE)),
        .length = SZ_256K,
        .type = MT_DEVICE_WC
    },
    {
        .virtual = ABB_MDSYS_BASE,
        .pfn = __phys_to_pfn(IO_ABB_MDSYS_VIRT_TO_PHYS(ABB_MDSYS_BASE)),
        .length = SZ_4K,
        .type = MT_DEVICE
    },
    {
        .virtual = AUDIO_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(AUDIO_BASE)),
        .length = SZ_4K,
        .type = MT_DEVICE
    },
    {
        .virtual = MFG_AXI_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(MFG_AXI_BASE)),
        .length = SZ_64K,
        .type = MT_DEVICE
    },
    {
        .virtual = VER_BASE,
        .pfn = __phys_to_pfn(IO_VER_VIRT_TO_PHYS(VER_BASE)),
        .length = SZ_4K,
        .type = MT_DEVICE
    },
    {
        .virtual = INTERNAL_SRAM_BASE,
        .pfn = __phys_to_pfn(0xF0000000),
        .length = SZ_64K,
        .type = MT_DEVICE_WC
    },
    {
        .virtual = NS_GIC_CPU_BASE,
        .pfn = __phys_to_pfn(IO_VIRT_TO_PHYS(GIC_CPU_BASE)),
        .length = SZ_8K,
        .type = MT_DEVICE
    },
};

void __init mt_map_io(void)
{
    pgd_t *pgd;
    pud_t *pud;
    pmd_t *pmd;

    iotable_init(mt_io_desc, ARRAY_SIZE(mt_io_desc));

    /* set NS=1 for NS_GIC_CPU_BASE */
    pgd = pgd_offset(&init_mm, NS_GIC_CPU_BASE);
    pud = pud_offset(pgd, NS_GIC_CPU_BASE);
    pmd = pmd_offset(pud, NS_GIC_CPU_BASE);
    if ((pmd_val(*pmd) & PMD_TYPE_MASK) == PMD_TYPE_TABLE) {
        __raw_writel(__raw_readl(pmd) | PMD_TABLE_NS, pmd);
    } else {
        __raw_writel(__raw_readl(pmd) | PMD_SECT_NS, pmd);
    }
    flush_pmd_entry(pmd);
}

MACHINE_START(MT6577, "MT6577")
    .atag_offset    = PHYS_OFFSET + 0x00000100,
    .map_io         = mt_map_io,
    .init_irq       = mt_init_irq,
    .timer          = &mt6577_timer,
    .init_machine   = mt_init,
    .fixup          = mt_fixup,
    .restart        = arm_machine_restart
MACHINE_END

MACHINE_START(MT6575, "MT6575")
    .atag_offset    = PHYS_OFFSET + 0x00000100,
    .map_io         = mt_map_io,
    .init_irq       = mt_init_irq,
    .timer          = &mt6577_timer,
    .init_machine   = mt_init,
    .fixup          = mt_fixup,
    .restart        = arm_machine_restart
MACHINE_END

//FIXME Workaround for build pass
int dump_idle_info(char *buffer, int size)  { return 0; }
