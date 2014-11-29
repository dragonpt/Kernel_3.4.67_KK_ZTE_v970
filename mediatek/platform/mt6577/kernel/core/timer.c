#include <linux/of.h>
#include <asm/mach/time.h>
#include <mach/timer.h>
#include <asm/smp_twd.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_irq.h>


#ifdef CONFIG_HAVE_ARM_TWD
#define LOCAL_TIMER_BASE (SCU_BASE + 0x600)
static DEFINE_TWD_LOCAL_TIMER(mtk_twd_local_timer,
			      LOCAL_TIMER_BASE, GIC_PPI_PRIVATE_TIMER);
//static struct twd_local_timer mtk_twd_local_timer ;
void __init mtk_twd_init(void)
{
	struct twd_local_timer *twd_local_timer;
	int err;

	twd_local_timer = &mtk_twd_local_timer;

	if (of_have_populated_dt())
		twd_local_timer_of_register();
	else {
		err = twd_local_timer_register(twd_local_timer);
		if (err)
			pr_err("twd_local_timer_register failed %d\n", err);
	}
}
#endif

extern struct mt65xx_clock mtk_gpt;

struct mt65xx_clock *mtk_clocks[] =
{
    &mtk_gpt,
};

static void __init mtk_timer_init(void)
{
    int i;
    struct mt65xx_clock *clock;
    int ret;

    for (i = 0; i < ARRAY_SIZE(mtk_clocks); i++) {
        clock = mtk_clocks[i];

        clock->init_func();

        if (clock->clocksource.name) {
            ret = clocksource_register(&(clock->clocksource));
            if (ret) {
                printk(KERN_ERR "mtk_timer_init: clocksource_register failed for %s\n", clock->clocksource.name);
            }
        }

        ret = setup_irq(clock->irq.irq, &(clock->irq));
        if (ret) {
            printk(KERN_ERR "mtk_timer_init: setup_irq failed for %s\n", clock->irq.name);
        }

        if (clock->clockevent.name)
            clockevents_register_device(&(clock->clockevent));
    }
#ifdef CONFIG_HAVE_ARM_TWD    
    mtk_twd_init();
#endif   
}

struct sys_timer mt6577_timer = {
    .init = mtk_timer_init,
};

