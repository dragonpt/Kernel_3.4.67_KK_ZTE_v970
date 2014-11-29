#include <linux/init.h>        /* For init/exit macros */
#include <linux/module.h>      /* For MODULE_ marcros  */
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/watchdog.h>
#include <linux/platform_device.h>

#include <asm/uaccess.h>
#include <mach/irqs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_wdt.h>
#include <linux/delay.h>

#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include <linux/aee.h>
#include <mach/sync_write.h>
#include <mach/ext_wd_drv.h>

#include <mach/wd_api.h>





//#undef  DRV_WriteReg32
//#define DRV_WriteReg32(a, v)			mt65xx_reg_sync_writel(v, a) //write32

extern void aee_wdt_printf(const char *fmt, ...);

/**---------------------------------------------------------------------
 * Sub feature switch region
 *----------------------------------------------------------------------
 */
#define NO_DEBUG 1

/*----------------------------------------------------------------------
 *   IRQ ID 
 *--------------------------------------------------------------------*/
#define AP_RGU_WDT_IRQ_ID    MT_WDT_IRQ_ID

/* 
 * internal variables 
 */
//static char expect_close; // Not use
//static spinlock_t rgu_reg_operation_spinlock = SPIN_LOCK_UNLOCKED;
static DEFINE_SPINLOCK(rgu_reg_operation_spinlock);
static unsigned short timeout;
static volatile BOOL  rgu_wdt_intr_has_trigger; // For test use
static int g_last_time_time_out_value = 0;
static int g_wdt_enable = 1;

enum {
	WDT_NORMAL_MODE,
	WDT_EXP_MODE
} g_wdt_mode = WDT_NORMAL_MODE;

/* 
 * module parameters 
 */
static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
//MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default=CONFIG_WATCHDOG_NOWAYOUT)");

#ifndef __USING_DUMMY_WDT_DRV__ /* FPGA will set this flag */
/*
    this function set the timeout value.
    value: second
*/
	void mtk_wdt_set_time_out_value(unsigned int value)

{
	/*
	 * TimeOut = BitField 15:5
	 * Key	   = BitField  4:0 = 0x08
	 */	
	spin_lock(&rgu_reg_operation_spinlock);
	
	// 1 tick means 512 * T32K -> 1s = T32/512 tick = 64
	// --> value * (1<<6)
	timeout = (unsigned short)(value * ( 1 << 6) );

	timeout = timeout << 5; 

	DRV_WriteReg(MTK_WDT_LENGTH, (timeout | MTK_WDT_LENGTH_KEY) );

	spin_unlock(&rgu_reg_operation_spinlock);
}
//EXPORT_SYMBOL(mtk_wdt_set_time_out_value);



/*
    watchdog mode:
    debug_en:   debug module reset enable. 
    irq:        generate interrupt instead of reset
    ext_en:     output reset signal to outside
    ext_pol:    polarity of external reset signal
    wdt_en:     enable watch dog timer
*/
void mtk_wdt_mode_config(	BOOL dual_mode_en, 
						BOOL irq, 
						BOOL ext_en, 
						BOOL ext_pol, 
						BOOL wdt_en )
{
	unsigned short tmp;

	spin_lock(&rgu_reg_operation_spinlock);
	printk("fwq mtk_wdt_mode_config 1111 mode value=%x\n",DRV_Reg16(MTK_WDT_MODE));
	tmp = DRV_Reg16(MTK_WDT_MODE);
	tmp |= MTK_WDT_MODE_KEY;

	// Bit 0 : Whether enable watchdog or not
	if(wdt_en == TRUE)
		tmp |= MTK_WDT_MODE_ENABLE;
	else
		tmp &= ~MTK_WDT_MODE_ENABLE;

	// Bit 1 : Configure extern reset signal polarity.
	if(ext_pol == TRUE)
		tmp |= MTK_WDT_MODE_EXT_POL;
	else
		tmp &= ~MTK_WDT_MODE_EXT_POL;

	// Bit 2 : Whether enable external reset signal
	if(ext_en == TRUE)
		tmp |= MTK_WDT_MODE_EXTEN;
	else
		tmp &= ~MTK_WDT_MODE_EXTEN;

	// Bit 3 : Whether generating interrupt instead of reset signal
	// do not let other moudle set this config
	/*
	if(irq == TRUE)
		tmp |= MTK_WDT_MODE_IRQ;
	else
		tmp &= ~MTK_WDT_MODE_IRQ;
	*/
	#ifdef CONFIG_ARCH_MT6577
	tmp &= ~MTK_WDT_MODE_IRQ; //always use reset mode when use this api 6577
	#else
	if(irq == TRUE)
		tmp |= MTK_WDT_MODE_IRQ;
	else
		tmp &= ~MTK_WDT_MODE_IRQ;
	#endif
/*
	// Bit 6 : Whether enable debug module reset
	if(debug_en == TRUE)
		tmp |= MTK_WDT_MODE_DEBUG_EN;
	else
		tmp &= ~MTK_WDT_MODE_DEBUG_EN;
*/
	// Bit 4: WDT_Auto_restart, this is a reserved bit, we use it as bypass powerkey flag.
	//		Because HW reboot always need reboot to kernel, we set it always.
	tmp |= MTK_WDT_MODE_AUTO_RESTART;

	DRV_WriteReg(MTK_WDT_MODE,tmp);
	printk("fwq mtk_wdt_mode_config  mode value=%x, tmp:%x\n",DRV_Reg16(MTK_WDT_MODE), tmp);
	spin_unlock(&rgu_reg_operation_spinlock);
}
//EXPORT_SYMBOL(mtk_wdt_mode_config);

/* This function will disable watch dog */
void mtk_wdt_disable(void)
{
	unsigned short tmp;

	spin_lock(&rgu_reg_operation_spinlock);

	tmp = DRV_Reg16(MTK_WDT_MODE);
	printk("fwq mtk_wdt_disable  mode value=%x\n",tmp);
	tmp |= MTK_WDT_MODE_KEY;
	tmp &= ~MTK_WDT_MODE_ENABLE;

	DRV_WriteReg(MTK_WDT_MODE,tmp);

	spin_unlock(&rgu_reg_operation_spinlock);
}

/* This function will return WDT EN setting */
int mtk_wdt_get_en_setting(void)
{
	return (DRV_Reg16(MTK_WDT_MODE)&MTK_WDT_MODE_ENABLE);
}

/*Kick the watchdog*/
void mtk_wdt_restart(enum wd_restart_type type)

{
	if(type == WD_TYPE_NORMAL) 
	{
		spin_lock(&rgu_reg_operation_spinlock);
		DRV_WriteReg(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);
		spin_unlock(&rgu_reg_operation_spinlock);
	}
	else if(type == WD_TYPE_NOLOCK) 
	{
	    *(volatile u32 *)( MTK_WDT_RESTART) =MTK_WDT_RESTART_KEY ;
	}
	else
	{
	    //printk("WDT:[mtk_wdt_restart] type error pid =%d\n",type,current->pid);
	}
}
/*                                                 */
#if 1
EXPORT_SYMBOL(mtk_wdt_restart);
#else
//EXPORT_SYMBOL(mtk_wdt_restart);
#endif
/*                                                 */


void wdt_arch_reset(char mode)
{
       unsigned short wdt_mode_val;
	printk("wdt_arch_reset called@Kernel\n");
	
	spin_lock(&rgu_reg_operation_spinlock);
	/* Watchdog Rest */
	DRV_WriteReg(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);
	wdt_mode_val = DRV_Reg(MTK_WDT_MODE);
	printk("wdt_arch_reset called MTK_WDT_MODE =%x \n",wdt_mode_val);
	/* clear autorestart bit: autoretart: 1, bypass power key, 0: not bypass power key */
	wdt_mode_val &=(~MTK_WDT_MODE_AUTO_RESTART);
	/* make sure WDT mode is hw reboot mode, can not config isr mode  */
	wdt_mode_val &=(~(MTK_WDT_MODE_IRQ|MTK_WDT_MODE_ENABLE));
	if(mode){
		/* mode != 0 means by pass power key reboot, We using auto_restart bit as by pass power key flag */
		 wdt_mode_val = wdt_mode_val | (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN|MTK_WDT_MODE_AUTO_RESTART);
		 //DRV_WriteReg(MTK_WDT_MODE, wdt_mode_val);
		//DRV_WriteReg(MTK_WDT_MODE, (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN|MTK_WDT_MODE_AUTO_RESTART));
	}else{
	       wdt_mode_val = wdt_mode_val | (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN);
		 //DRV_WriteReg(MTK_WDT_MODE,wdt_mode_val); 
		// DRV_WriteReg(MTK_WDT_MODE, (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN));
	}

	DRV_WriteReg(MTK_WDT_MODE,wdt_mode_val);
	//DRV_WriteReg(MTK_WDT_LENGTH, MTK_WDT_LENGTH_KEY);
	printk("wdt_arch_reset called end  MTK_WDT_MODE =%x \n",wdt_mode_val);
	udelay(100);
	DRV_WriteReg(MTK_WDT_SWRST, MTK_WDT_SWRST_KEY);
        printk("wdt_arch_reset: SW_reset happen\n");
	spin_unlock(&rgu_reg_operation_spinlock);

	while (1)
	{
		printk("wdt_arch_reset error\n");
	}
}
EXPORT_SYMBOL(wdt_arch_reset);
int mtk_wdt_request_mode_set(int mark_bit,WD_REQ_MODE mode)
{
	printk("mtk_wdt_request_mode_set:6577 not support \n");
	return 0;
}
int mtk_wdt_request_en_set(int mark_bit,WD_REQ_CTL en)
{
	printk("mtk_wdt_request_en_set:6577 not support \n");
	return 0;
}

int  mtk_wdt_confirm_hwreboot(void)
{
	printk("mtk_wdt_request_en_set:6577 not support \n");
	return 0;
}
int mtk_wdt_enable(enum wk_wdt_en en)
{
	unsigned int tmp =0;
	spin_lock(&rgu_reg_operation_spinlock);
	
	tmp = DRV_Reg32(MTK_WDT_MODE);
	tmp |= MTK_WDT_MODE_KEY;
	
	if(WK_WDT_EN == en)
	{
	  	tmp |= MTK_WDT_MODE_ENABLE;
	  	g_wdt_enable = 1;
	}
	if(WK_WDT_DIS == en)
	{
	  	tmp &= ~MTK_WDT_MODE_ENABLE;
	  	g_wdt_enable = 0;
	}
    printk("mtk_wdt_enable value=%x,pid=%d\n",tmp,current->pid);
	
	DRV_WriteReg32(MTK_WDT_MODE,tmp);
	spin_unlock(&rgu_reg_operation_spinlock);
	return 0;
}
void wdt_dump_reg(void)
{
	printk("wdt_dump_reg:6577 not support \n");
}

int mtk_wdt_swsysret_config(int bit,int set_value)
{

	//spin_lock(&rgu_reg_operation_spinlock);
	
	printk("mtk_wdt_swsysret_config(0x%x,%d)\n",bit,set_value);
	switch(bit)
	{
	
	case 0x20000000:
		#ifdef CONFIG_ARCH_MT6577
		printk("fwq 6577 not setting this config\n");
		#else
		
		if(1 ==  set_value)
		{
			//irq mode
			mtk_wdt_mode_config(TRUE, TRUE, FALSE, FALSE, TRUE);
			g_wdt_mode = WDT_EXP_MODE;
		}
		if(0 ==  set_value)
		{
		   // reboot mode
		   mtk_wdt_mode_config(TRUE, FALSE, FALSE, FALSE, TRUE);
		   g_wdt_mode = WDT_NORMAL_MODE;
		}
		#endif
		break;
	}
	//spin_unlock(&rgu_reg_operation_spinlock);

	mdelay(10);
	printk("after set wdt_sys_val =%d,MODE(0x%x)\n",set_value,DRV_Reg32(MTK_WDT_MODE));
    return 0;
}


#else 
//-------------------------------------------------------------------------------------------------
//      Dummy functions
//-------------------------------------------------------------------------------------------------
void mtk_wdt_set_time_out_value(unsigned short value){}
static void mtk_wdt_set_reset_length(unsigned short value){}
void mtk_wdt_mode_config(BOOL debug_en,BOOL irq,	BOOL ext_en, BOOL ext_pol, BOOL wdt_en){}
void mtk_wdt_disable(void){}
int mtk_wdt_get_en_setting(void){return 0;}
void mtk_wdt_restart(enum wd_restart_type type){}
static void mtk_wdt_sw_trigger(void){}
static unsigned char mtk_wdt_check_status(void){}
void wdt_arch_reset(char mode){}
EXPORT_SYMBOL(wdt_arch_reset);

#endif //#ifndef __USING_DUMMY_WDT_DRV__

#ifdef __ENABLE_WDT_TEST__
/*
 * Test Program :
 *				1. WDT_HW_Reset_test
 *				2. WDT_SW_Reset_test
 *				3. WDT_count_test
 */

void WDT_HW_Reset_test(void)
{
	printk("WDT_HW_Reset_test : System will reset after 5 secs\n");
	mtk_wdt_set_time_out_value(5);
	//enable WDT reset, auto-restart enable ,disable interrupt.
	//mtk_wdt_ModeSelection(KAL_TRUE, KAL_TRUE, KAL_FALSE);
	// en debug, dis irq, dis ext, low pol, en wdt
	mtk_wdt_mode_config(TRUE, FALSE, FALSE, FALSE, TRUE);
	mtk_wdt_restart(WK_WDT_EXT_TYPE);
	
	while(1);
}
EXPORT_SYMBOL(WDT_HW_Reset_test);


void WDT_HW_Reset_kick_6times_test(void)
{
	int kick_times = 6;
	
	mtk_wdt_set_time_out_value(5);	
	//enable WDT reset, auto-restart enable ,disable intr
	//mtk_wdt_ModeSelection(KAL_TRUE, KAL_TRUE, KAL_FALSE);
	// en debug, dis irq, dis ext, low pol, en wdt
	mtk_wdt_mode_config(TRUE, FALSE, FALSE, FALSE, TRUE);
	mtk_wdt_restart(WK_WDT_EXT_TYPE);

	// kick WDT test.
	while(kick_times >= 0)
	{
		mdelay(3000);
		printk("WDT_HW_Reset_test : reset after %d times !\n", kick_times);
		mtk_wdt_restart(WK_WDT_EXT_TYPE);
		kick_times--;
	}

	printk("WDT_HW_Reset_test : Kick stop,System will reset after 5 secs!!\n");
	while(1);
	
}
EXPORT_SYMBOL(WDT_HW_Reset_kick_6times_test);


void WDT_SW_Reset_test(void)
{
	printk("WDT_SW_Reset_test : System will reset Immediately\n");
		
	// disable WDT reset, auto-restart disable ,disable intr
	//mtk_wdt_ModeSelection(KAL_FALSE, KAL_FALSE, KAL_FALSE);
	// en debug, dis irq, dis ext, low pol, dis wdt
	mtk_wdt_mode_config(TRUE, FALSE, FALSE, FALSE, FALSE);
	
	mtk_wdt_set_time_out_value(1);
	mtk_wdt_restart(1);
	
	mtk_wdt_sw_trigger();
	
	while(1);
}
EXPORT_SYMBOL(WDT_SW_Reset_test);

void WDT_count_test(void)
{
	/*
	 * Try DVT testsuite : WDT_count_test (Non-reset test)
	 */
	printk("WDT_count_test Start..........\n");

	mtk_wdt_set_time_out_value(10);
	// enable WDT reset, auto-restart disable, enable intr
	//mtk_wdt_ModeSelection(KAL_TRUE, KAL_FALSE, KAL_TRUE);
	// en debug, en irq, dis ext, low pol, en wdt
	mtk_wdt_mode_config(TRUE, TRUE, FALSE, FALSE, TRUE);
	mtk_wdt_restart(WK_WDT_EXT_TYPE);

	printk("1/2 : Waiting 10 sec. for WDT with WDT_status.\n");
	
	// wait and check WDT status
	while(DRV_Reg16(MTK_WDT_STATUS) != MTK_WDT_STATUS_HWWDT_RST);	
	printk("WDT_count_test done by WDT_STATUS!!\n");

	/*status is checked*/

	/*check interrupt.*/ 
	rgu_wdt_intr_has_trigger = 0; // set to zero before WDT counting down

	/*can continue.*/
	mtk_wdt_restart(WK_WDT_EXT_TYPE);
	
	printk("2/2 : Waiting 10 sec. for WDT with IRQ.\n");

	//need a ISR, when interrupt, set rgu_wdt_intr_has_trigger to 1.
	while(rgu_wdt_intr_has_trigger == 0);
	printk("WDT_count_test done by IRQ!!\n");
	
	printk("WDT_count_test Finish !!\n");	
}
EXPORT_SYMBOL(WDT_count_test);
#endif //__ENABLE_WDT_TEST__

#ifndef CONFIG_FIQ_GLUE
static void wdt_report_info (void)
{
    //extern struct task_struct *wk_tsk;
    struct task_struct *task ;
    task = &init_task ;
    
    printk ("Qwdt: -- watchdog time out\n") ;
    for_each_process (task)
    {
        if (task->state == 0)
        {
            printk ("PID: %d, name: %s\n backtrace:\n", task->pid, task->comm) ;
            show_stack (task, NULL) ;
            printk ("\n") ;
        }
    }
    
    
    printk ("backtrace of current task:\n") ;
    show_stack (NULL, NULL) ;
        
    printk ("Qwdt: -- watchdog time out\n") ;    
}

#endif

#ifdef CONFIG_FIQ_GLUE
static void wdt_fiq(void *arg, void *regs, void *svc_sp)
{
     register int sp asm("sp");
     unsigned int *preg = (unsigned int*)regs;
//	 printk("bei:wdt fiq is going\n");

//     emit_log_string("<0>/******************************************************/\n");
 //    emit_log_string("<0>/*                        WDT                          /\n");
 //    emit_log_string("<0>/******************************************************/\n");

     asm volatile("mov %0, %1\n\t"
                  "mov fp, %2\n\t"
                 : "=r" (sp)
                 : "r" (svc_sp), "r" (preg[11])
                 );

     *((volatile unsigned int*)(0x00000000)); /* trigger exception */
}
#else
static irqreturn_t mtk_wdt_isr(int irq, void *dev_id)

{
#ifndef __USING_DUMMY_WDT_DRV__ /* FPGA will set this flag */
	//mt65xx_irq_mask(AP_RGU_WDT_IRQ_ID);
	rgu_wdt_intr_has_trigger = 1;

	//Monkey.QHQ
	wdt_report_info () ;
	//Monkey.QHQ
	BUG();

	//need to modift. 
	//aee_bug("WATCHDOG", "Watch Dog Timeout");
	//mt65xx_irq_ack(AP_RGU_WDT_IRQ_ID);
	//mt65xx_irq_unmask(AP_RGU_WDT_IRQ_ID);
#endif	
	return IRQ_HANDLED;
}
#endif

/* 
 * Device interface 
 */
static int mtk_wdt_probe(struct platform_device *dev)
{
	int ret=0;
	unsigned short interval_val;

	
	printk("******** MTK WDT driver probe!! ********\n" );

#ifndef __USING_DUMMY_WDT_DRV__ /* FPGA will set this flag */
#ifndef CONFIG_FIQ_GLUE	
		
	ret = request_irq(AP_RGU_WDT_IRQ_ID, (irq_handler_t)mtk_wdt_isr, IRQF_TRIGGER_FALLING, "mtk_watchdog", NULL);
#else
	ret = request_fiq(AP_RGU_WDT_IRQ_ID, wdt_fiq, IRQF_TRIGGER_FALLING, NULL);
#endif	
        if(ret != 0)
	{
		printk( "mtk_wdt_probe : failed to request irq (%d)\n", ret);
		return ret;
	}
	printk("mtk_wdt_probe : Success to request irq\n");

	/* Set timeout vale and restart counter */
	g_last_time_time_out_value=30;
	mtk_wdt_set_time_out_value(30);
	mtk_wdt_restart(WD_TYPE_NORMAL);

	/**
	 * Set the reset lenght: we will set a special magic key.
	 * For Power off and power on reset, the INTERVAL default value is 0x7FF.
	 * We set Interval[1:0] to different value to distinguish different stage.
	 * Enter pre-loader, we will set it to 0x0
	 * Enter u-boot, we will set it to 0x1
	 * Enter kernel, we will set it to 0x2
	 * And the default value is 0x3 which means reset from a power off and power on reset
	 */
	#define POWER_OFF_ON_MAGIC	(0x3)
	#define PRE_LOADER_MAGIC	(0x0)
	#define U_BOOT_MAGIC		(0x1)
	#define KERNEL_MAGIC		(0x2)
	#define MAGIC_NUM_MASK		(0x3)

#ifdef  CONFIG_MTK_WD_KICKER	// Initialize to enable wdt
  	//#ifdef CONFIG_MTK_AEE_FEATURE	// Initialize to interrupt mode
	//mtk_wdt_mode_config(TRUE, TRUE, FALSE, FALSE, TRUE);
	//printk("mtk_wdt_probe :init to irq mode\n");
	//#else				// Initialize to reset mode
	mtk_wdt_mode_config(TRUE, FALSE, FALSE, FALSE, TRUE);
	printk("mtk_wdt_probe :init to reset mode\n");
	//#endif
#else //disable wdt
	mtk_wdt_mode_config(FALSE, FALSE, FALSE, FALSE, FALSE);
#endif

	/* Update interval register value and check reboot flag */
	interval_val = DRV_Reg(MTK_WDT_INTERVAL);
	interval_val &= ~(MAGIC_NUM_MASK);
	interval_val |= (KERNEL_MAGIC);
	/* Write back INTERVAL REG */
	DRV_WriteReg(MTK_WDT_INTERVAL, interval_val);
#endif

	return ret;
}

static int mtk_wdt_remove(struct platform_device *dev)
{
	printk("******** MTK wdt driver remove!! ********\n" );

#ifndef __USING_DUMMY_WDT_DRV__ /* FPGA will set this flag */
	free_irq(AP_RGU_WDT_IRQ_ID, NULL);
#endif
	return 0;
}

static void mtk_wdt_shutdown(struct platform_device *dev)
{	
	printk("******** MTK WDT driver shutdown!! ********\n" );

	//mtk_wdt_ModeSelection(KAL_FALSE, KAL_FALSE, KAL_FALSE);
	// en debug, dis irq, dis ext, low pol, dis wdt
	mtk_wdt_mode_config(TRUE, FALSE, FALSE, FALSE, FALSE);
	mtk_wdt_restart(WD_TYPE_NORMAL);

}

void mtk_wd_suspend(void)
{

	//mtk_wdt_ModeSelection(KAL_FALSE, KAL_FALSE, KAL_FALSE);
	// en debug, dis irq, dis ext, low pol, dis wdt
	mtk_wdt_mode_config(TRUE, FALSE, FALSE, FALSE, FALSE);
	mtk_wdt_restart(WD_TYPE_NORMAL);
	aee_sram_printk("[WDT] suspend\n");
	printk("[WDT] suspend\n");
}

void mtk_wd_resume(void)
{
	if ( g_wdt_enable == 1 ) 
	{
		mtk_wdt_set_time_out_value(g_last_time_time_out_value);
		
		if (g_wdt_mode == WDT_EXP_MODE){
			//mtk_wdt_ModeSelection(KAL_TRUE, KAL_FALSE, KAL_TRUE);
			// en debug, en irq, dis ext, low pol, en wdt
			mtk_wdt_mode_config(TRUE, TRUE, FALSE, FALSE, TRUE);
		}else{
			//mtk_wdt_ModeSelection(KAL_TRUE, KAL_TRUE, KAL_FALSE);
			// en debug, dis irq, dis ext, low pol, en wdt
			mtk_wdt_mode_config(TRUE, FALSE, FALSE, FALSE, TRUE);
		}
		mtk_wdt_restart(WD_TYPE_NORMAL);	
	}

	aee_sram_printk("[WDT] resume(%d)\n", g_wdt_enable);
	printk("[WDT] resume(%d)\n", g_wdt_enable);
}


static struct platform_driver mtk_wdt_driver =
{
	.driver     = {
		.name	= "mtk-wdt",
	},
	.probe	= mtk_wdt_probe,
	.remove	= mtk_wdt_remove,
	.shutdown	= mtk_wdt_shutdown,
//	.suspend	= mtk_wdt_suspend,
//	.resume	= mtk_wdt_resume,
};

struct platform_device mtk_device_wdt = {
		.name		= "mtk-wdt",
		.id		= 0,
		.dev		= {
		}
};

/*
 * init and exit function
 */
static int __init mtk_wdt_init(void)
{
	
	int ret;
	
	ret = platform_device_register(&mtk_device_wdt);
	if (ret) {
		printk("****[mtk_wdt_driver] Unable to device register(%d)\n", ret);
		return ret;
	}

	ret = platform_driver_register(&mtk_wdt_driver);
	if (ret) {
		printk("****[mtk_wdt_driver] Unable to register driver (%d)\n", ret);
		return ret;
	}

	return 0;	
}

static void __exit mtk_wdt_exit (void)
{
}

module_init(mtk_wdt_init);
module_exit(mtk_wdt_exit);
MODULE_AUTHOR("MTK");
MODULE_DESCRIPTION("MT6577 Watchdog Device Driver");
MODULE_LICENSE("GPL");
//FIXME Workaround for build pass
int hw_reboot_mode(void)
{
    return 0;
}

int dump_localtimer_info(char* buffer, int size)
{
    return 0;
}
