#include <linux/init.h>
#include <linux/smp.h>
#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <asm/smp_scu.h>
#include <asm/localtimer.h>
#include <asm/fiq_glue.h>
#include <mach/mt_reg_base.h>
#include <mach/smp.h>
#include <mach/irqs.h>
#include <mach/sync_write.h>
#include <mach/hotplug.h>
#include <mach/wd_api.h>


#define SLAVE_MAGIC_REG 0xF011A008
#define SLAVE_MAGIC_NUM 0x534C4156
#define SLAVE_JUMP_REG 0xF011A000


extern void mt_secondary_startup(void);
extern void irq_raise_softirq(const struct cpumask *mask, unsigned int irq);
extern void mt_gic_secondary_init(void);

#ifdef CONFIG_MT65XX_TRACER
extern void pmu_regs_restore(void);
#endif


/*
 * control for which core is the next to come out of the secondary
 * boot "holding pen".
 */
volatile int pen_release = -1;

static DEFINE_SPINLOCK(boot_lock);

void __cpuinit platform_secondary_init(unsigned int cpu)
{
    struct wd_api *wd_api = NULL;
    
    printk(KERN_CRIT "Slave cpu init\n");
    HOTPLUG_INFO("platform_secondary_init, cpu: %d\n", cpu);
    
    mt_gic_secondary_init();
    
    pen_release = -1;
    smp_wmb();

    get_wd_api(&wd_api);
    if (wd_api)
        wd_api->wd_cpu_hot_plug_on_notify(cpu);    

#ifdef CONFIG_MT65XX_TRACER
    pmu_regs_restore();
#endif

    fiq_glue_resume();

    /*
     * Synchronise with the boot thread.
     */
    spin_lock(&boot_lock);
    spin_unlock(&boot_lock);
}

int __cpuinit boot_secondary(unsigned int cpu, struct task_struct *idle)
{
    unsigned long timeout;
    static int is_first_boot = 1;

    printk(KERN_CRIT "Boot slave CPU\n");

    /*
     * Set synchronisation state between this boot processor
     * and the secondary one
     */
    spin_lock(&boot_lock);

    HOTPLUG_INFO("boot_secondary, cpu: %d\n", cpu);
    /*
    * The secondary processor is waiting to be released from
    * the holding pen - release it, then wait for it to flag
    * that it has been released by resetting pen_release.
    *
    * Note that "pen_release" is the hardware CPU ID, whereas
    * "cpu" is Linux's internal ID.
    */
    pen_release = cpu;
    __cpuc_flush_dcache_area((void *)&pen_release, sizeof(pen_release));
    outer_clean_range(__pa(&pen_release), __pa(&pen_release + 1));

    if (is_first_boot)
    {
        mt65xx_reg_sync_writel(SLAVE_MAGIC_NUM, SLAVE_MAGIC_REG);
        is_first_boot = 0;
    }
    else
    {
        mt65xx_reg_sync_writel(virt_to_phys(mt_secondary_startup), BOOTROM_BOOT_ADDR);
    }
    power_on_cpu1();

    irq_raise_softirq(cpumask_of(cpu), IPI_CPU_START);

    /*
     * Now the secondary core is starting up let it run its
     * calibrations, then wait for it to finish
     */
    spin_unlock(&boot_lock);

    timeout = jiffies + (1 * HZ);
    while (time_before(jiffies, timeout)) {
        smp_rmb();
        if (pen_release == -1)
            break;
        
        udelay(10);
    }

    return pen_release != -1 ? -ENOSYS : 0;
}

void __init smp_init_cpus(void)
{
    unsigned int i, ncores;
    
    /*
     * NoteXXX: CPU 1 may not be reset clearly after power-ON.
     *          Need to apply a S/W workaround to manualy reset it first.
     */
    u32 val;
    val = *(volatile u32 *)0xF0009010;
    mt65xx_reg_sync_writel(val | 0x2, 0xF0009010);
    udelay(10);
    mt65xx_reg_sync_writel(val & ~0x2, 0xF0009010);
    udelay(10);

    ncores = scu_get_core_count((void *)SCU_BASE);

    if (ncores > NR_CPUS) {
        printk(KERN_WARNING
               "SCU core count (%d) > NR_CPUS (%d)\n", ncores, NR_CPUS);
        printk(KERN_WARNING
               "set nr_cores to NR_CPUS (%d)\n", NR_CPUS);
        ncores = NR_CPUS;
    }

    for (i = 0; i < ncores; i++)
        set_cpu_possible(i, true);

    set_smp_cross_call(irq_raise_softirq);
    
}

void __init platform_smp_prepare_cpus(unsigned int max_cpus)
{
    int i;

    for (i = 0; i < max_cpus; i++)
        set_cpu_present(i, true);

    scu_enable((void *)SCU_BASE);

    /* write the address of slave startup into the system-wide flags register */
    mt65xx_reg_sync_writel(virt_to_phys(mt_secondary_startup), SLAVE_JUMP_REG);
}
