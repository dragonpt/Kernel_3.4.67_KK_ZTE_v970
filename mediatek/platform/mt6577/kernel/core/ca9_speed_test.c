#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/wakelock.h>
#include <linux/module.h>
#include <asm/delay.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/gfp.h>
#include <asm/io.h>
#include <asm/memory.h>
#include <asm/outercache.h>
#include <linux/spinlock.h>

#include <linux/leds-mt65xx.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include "mach/ca9_slt.h"

/*
README:
    Modify 'CONFIG_MAX_DRAM_SIZE_SUPPORT' in 'mediatek/config/mt6575/autoconfig/kconfig/platform' to reserve a region
    mt6575_fixup() in mediatek/platform/mt6575/kernel/core/mt6575_devs.c reference this CONFIG_MAX_DRAM_SIZE_SUPPORT
*/

extern int max_power_loop(int cpu_id);
extern int speed_indicative_loop(int cpu_id);

static int g_iCPU0_FinalResult, g_iCPU1_FinalResult;
static int g_iCPU0_PassFail, g_iCPU1_PassFail;
static int g_iMaxPowerLoopCount;

int g_iADDR1_CPU0, g_iADDR2_CPU0, g_iADDR3_CPU0, g_iADDR4_CPU0, g_iADDR5_CPU0, g_iADDR6_CPU0, g_iADDR7_CPU0, g_iADDR8_CPU0;
int g_iADDR1_CPU1, g_iADDR2_CPU1, g_iADDR3_CPU1, g_iADDR4_CPU1, g_iADDR5_CPU1, g_iADDR6_CPU1, g_iADDR7_CPU1, g_iADDR8_CPU1;
int g_iDATA1_CPU0, g_iDATA5_CPU0;
int g_iDATA2_CPU1, g_iDATA6_CPU1;

DEFINE_SPINLOCK(cpu0_speed_lock);
DEFINE_SPINLOCK(cpu1_speed_lock);
unsigned long cpu0_speed_flags;
unsigned long cpu1_speed_flags;

int ca9_speed_test(int cpu_id)
{
    int iResult;
    
    if(cpu_id == 0)
    {
        spin_lock_irqsave(&cpu0_speed_lock, cpu0_speed_flags);        
        iResult = max_power_loop(cpu_id);
        
        if(iResult == g_iCPU0_FinalResult)
        {
            iResult = speed_indicative_loop(cpu_id);                
            if(iResult == 0)
                printk("cpu0 speed_indicative_loop fail\n");
            else if(iResult == -1)
                printk("current CPU is not cpu0\n");             
        }
        else if(iResult == -1)
        {
            printk("current CPU is not cpu0\n");
        }
        else
        { 
            printk("cpu0 max_power_loop fail (result = 0x%x)\n",iResult);
            iResult = 0;
        }  
        spin_unlock_irqrestore(&cpu0_speed_lock, cpu0_speed_flags);     
    }
    else
    {
        spin_lock_irqsave(&cpu1_speed_lock, cpu1_speed_flags);        
        iResult = max_power_loop(cpu_id);
        
        if(iResult == g_iCPU1_FinalResult)
        {    
            iResult = speed_indicative_loop(cpu_id);
                
            if(iResult == 0)
                printk("cpu1 speed_indicative_loop fail\n");     
            else if(iResult == -1)
                printk("current CPU is not cpu1\n");                          
        }
        else if(iResult == -1)
        {
            printk("current CPU is not cpu1\n");
            iResult = -1;
        }        
        else
        { 
            printk("cpu1 max_power_loop fail (result = 0x%x)\n",iResult);
            iResult = 0;
        }  
        spin_unlock_irqrestore(&cpu1_speed_lock, cpu1_speed_flags);         
    }
      
    return iResult;
}

static struct device_driver cpu0_slt_ca9_max_power_drv =
{
    .name = "cpu0_ca9_max_power",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

static struct device_driver cpu1_slt_ca9_max_power_drv =
{
    .name = "cpu1_ca9_max_power",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

static struct device_driver ca9_max_power_loop_count_drv =
{
    .name = "ca9_max_power_loop_count",
    .bus = &platform_bus_type,
    .owner = THIS_MODULE,
};

static ssize_t cpu0_slt_ca9_max_power_show(struct device_driver *driver, char *buf)
{
    if(g_iCPU0_PassFail == -1)
        return snprintf(buf, PAGE_SIZE, "CPU0 CA9_MAX_POWER - CPU0 is powered off\n");
    else
        return snprintf(buf, PAGE_SIZE, "CPU0 CA9_MAX_POWER - %s(loop_count = %d)\n", g_iCPU0_PassFail != g_iMaxPowerLoopCount ? "FAIL" : "PASS", g_iCPU0_PassFail);
}

static ssize_t cpu1_slt_ca9_max_power_show(struct device_driver *driver, char *buf)
{
    if(g_iCPU1_PassFail == -1)
        return snprintf(buf, PAGE_SIZE, "CPU1 CA9_MAX_POWER - CPU1 is powered off\n");
    else    
        return snprintf(buf, PAGE_SIZE, "CPU1 CA9_MAX_POWER - %s(loop_count = %d)\n", g_iCPU1_PassFail != g_iMaxPowerLoopCount ? "FAIL" : "PASS", g_iCPU1_PassFail);
}

static ssize_t ca9_max_power_loop_count_show(struct device_driver *driver, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "CA9_MAX_POWER Loop Count = %d\n", g_iMaxPowerLoopCount);
}

static ssize_t cpu0_slt_ca9_max_power_store(struct device_driver *driver, const char *buf, size_t count)
{
    unsigned int i, ret;
        
    unsigned long mask;
    int ResultR0, ResultR1;
    unsigned int pTestMem;  
    int retry=0;       

    const int ResultR2 = 0x55555555;
    const int ResultR3 = 0x55555555;
    const int ResultR4 = 0xFAFAFAFB;
    const int ResultR10 = 0x5F5F5F60;

    g_iCPU0_PassFail = 0;  
    
    pTestMem = (unsigned int)vmalloc(17*1024);
    if((void*)pTestMem == NULL)
    {
        printk("allocate memory for cpu0 speed test fail\n");
        return 0;
    }     

    g_iADDR1_CPU0 = pTestMem + 0x0040;  //size 64 byte
    g_iADDR2_CPU0 = pTestMem + 0x0080;  //size 64 byte
    g_iADDR3_CPU0 = pTestMem + 0x00C0;  //size 64 byte
    g_iADDR4_CPU0 = pTestMem + 0x00E0;  //size 64 byte
    g_iADDR5_CPU0 = pTestMem + 0x0100;  //size 64 byte
    g_iADDR6_CPU0 = pTestMem + 0x0120;  //size 64 byte
    g_iADDR7_CPU0 = pTestMem + 0x0140;  //size 64 byte
    g_iADDR8_CPU0 = pTestMem + 0x0160;  //size 64 byte

    ResultR0 = g_iADDR1_CPU0 + 8;
    ResultR1 = g_iADDR2_CPU0 + 8;

    g_iCPU0_FinalResult = ResultR0 + ResultR1 + ResultR2 + ResultR3 + ResultR4 + ResultR10;
    
    g_iDATA5_CPU0 = pTestMem + 0x0180;  //size 64 byte
    g_iDATA1_CPU0 = pTestMem + 0x0400;  //size 16 KB
                  
    //mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, 255);
       
    mask = 1; /* processor 0 */
    while(sched_setaffinity(0, (struct cpumask*) &mask) < 0) 
    {      
        printk("Could not set cpu 0 affinity for current process(%d).\n", retry); 
        g_iCPU0_PassFail = -1;
        retry++;
        if(retry > 100)
        {
            vfree((void*)pTestMem);           
            return count;            
        }               
    }
    
    printk("\n>> CPU0 speed test start (cpu id = %d) <<\n\n", raw_smp_processor_id()); 
                   
    for (i = 0, g_iCPU0_PassFail = 0; i < g_iMaxPowerLoopCount; i++) {        
        ret = ca9_speed_test(0);    // 1: PASS, 0:Fail, -1: target CPU power off
        if(ret != -1)
        {
             g_iCPU0_PassFail += ret; 
        }
        else
        {
             g_iCPU0_PassFail = -1;
             break;
        }                
    }

    if (g_iCPU0_PassFail == g_iMaxPowerLoopCount) {
        printk("\n>> CPU0 speed test pass <<\n\n"); 
    }else {
        printk("\n>> CPU0 speed test fail (loop count = %d)<<\n\n",g_iCPU0_PassFail); 

        //mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, 0);
        //while (1);
    }
    
    vfree((void*)pTestMem);
            
    return count;
}

static ssize_t cpu1_slt_ca9_max_power_store(struct device_driver *driver, const char *buf, size_t count)
{
    unsigned int i, ret;
        
    unsigned long mask;
    int ResultR0, ResultR1;
    unsigned int pTestMem;    
    int retry=0;
        
    const int ResultR2 = 0x55555555;
    const int ResultR3 = 0x55555555;
    const int ResultR4 = 0xFAFAFAFB;
    const int ResultR10 = 0x5F5F5F60;

    g_iCPU1_PassFail = 0;  
    
    pTestMem = (unsigned int)vmalloc(17*1024);
    if((void*)pTestMem == NULL)
    {
        printk("allocate memory for cpu1 speed test fail\n");
        return 0;
    }   
    
    g_iADDR1_CPU1 = pTestMem + 0x0040;  //size 64 byte
    g_iADDR2_CPU1 = pTestMem + 0x0080;  //size 64 byte
    g_iADDR3_CPU1 = pTestMem + 0x00C0;  //size 32 byte
    g_iADDR4_CPU1 = pTestMem + 0x00E0;  //size 32 byte
    g_iADDR5_CPU1 = pTestMem + 0x0100;  //size 32 byte
    g_iADDR6_CPU1 = pTestMem + 0x0120;  //size 32 byte
    g_iADDR7_CPU1 = pTestMem + 0x0140;  //size 32 byte
    g_iADDR8_CPU1 = pTestMem + 0x0160;  //size 32 byte

    ResultR0 = g_iADDR1_CPU1 + 8;
    ResultR1 = g_iADDR2_CPU1 + 8;

    g_iCPU1_FinalResult = ResultR0 + ResultR1 + ResultR2 + ResultR3 + ResultR4 + ResultR10;

    g_iDATA6_CPU1 = pTestMem + 0x0180;  //size 64 byte     
    g_iDATA2_CPU1 = pTestMem + 0x0400;  //size 16 KB              
     
    //mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, 255);

    mask = 2; /* processor 1 */
    while(sched_setaffinity(0, (struct cpumask*)&mask) < 0) 
    {      
        printk("Could not set cpu 1 affinity for current process(%d).\n", retry);    
        g_iCPU1_PassFail = -1;
        retry++;
        if(retry > 100)
        {
            vfree((void*)pTestMem);           
            return count;            
        }      
    }

    printk("\n>> CPU1 speed test start (cpu id = %d) <<\n\n", raw_smp_processor_id()); 
           
    for (i = 0, g_iCPU1_PassFail = 0; i < g_iMaxPowerLoopCount; i++) {        
        ret = ca9_speed_test(1);    // 1: PASS, 0:Fail, -1: target CPU power off
        if(ret != -1)
        {
             g_iCPU1_PassFail += ret; 
        }
        else
        {
             g_iCPU1_PassFail = -1;
             break;
        }       
    }

    if (g_iCPU1_PassFail == g_iMaxPowerLoopCount) {
        printk("\n>> CPU1 speed test pass <<\n\n"); 
    }else {
        printk("\n>> CPU1 speed test fail (loop count = %d)<<\n\n",g_iCPU1_PassFail); 

        //mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, 0);
        //while (1);
    }

    vfree((void*)pTestMem);
            
    return count;
}

static ssize_t ca9_max_power_loop_count_store(struct device_driver *driver, const char *buf, size_t count)
{ 
    int result;
     
    if ((result = sscanf(buf, "%d", &g_iMaxPowerLoopCount)) == 1)
    {
        printk("set CA9 max power test loop count = %d successfully\n", g_iMaxPowerLoopCount);
    }
    else
    {
        printk("bad argument!!\n");
        return -EINVAL;   
    }
     
    return count;    
}
    
DRIVER_ATTR(cpu0_slt_ca9_max_power, 0644, cpu0_slt_ca9_max_power_show, cpu0_slt_ca9_max_power_store);
DRIVER_ATTR(cpu1_slt_ca9_max_power, 0644, cpu1_slt_ca9_max_power_show, cpu1_slt_ca9_max_power_store);
DRIVER_ATTR(ca9_max_power_loop_count, 0644, ca9_max_power_loop_count_show, ca9_max_power_loop_count_store);

int __init cpu0_slt_ca9_max_power_init(void)
{
    int ret;
    
    ret = driver_register(&cpu0_slt_ca9_max_power_drv);
    if (ret) {
        printk("fail to create CA9 max power SLT driver (CPU0)\n");
    }
    else
    {
        printk("success to create CA9 max power SLT driver (CPU0)\n");    
    }
    

    ret = driver_create_file(&cpu0_slt_ca9_max_power_drv, &driver_attr_cpu0_slt_ca9_max_power);
    if (ret) {
        printk("fail to create CA9 max power sysfs files (CPU0)\n");
    }
    else
    {
        printk("success to create CA9 max power sysfs files (CPU0)\n");        
    }
    
    return 0;
}

int __init cpu1_slt_ca9_max_power_init(void)
{
    int ret;

    ret = driver_register(&cpu1_slt_ca9_max_power_drv);
    if (ret) {
        printk("fail to create CA9 max power SLT driver (CPU1)\n");
    }
    else
    {
        printk("success to create CA9 max power SLT driver (CPU1)\n");    
    }
    

    ret = driver_create_file(&cpu1_slt_ca9_max_power_drv, &driver_attr_cpu1_slt_ca9_max_power);
    if (ret) {
        printk("fail to create CA9 max power sysfs files (CPU1)\n");
    }
    else
    {
        printk("success to create CA9 max power sysfs files (CPU1)\n");        
    }

    return 0;
}

int __init ca9_max_power_loop_count_init(void)
{
    int ret;

    ret = driver_register(&ca9_max_power_loop_count_drv);
    if (ret) {
        printk("fail to create CA9 max power loop count driver\n");
    }
    else
    {
        printk("success to create CA9 max power loop count driver\n");    
    }
    

    ret = driver_create_file(&ca9_max_power_loop_count_drv, &driver_attr_ca9_max_power_loop_count);
    if (ret) {
        printk("fail to create CA9 max power loop count sysfs files\n");
    }
    else
    {
        printk("success to create CA9 max power loop count sysfs files\n");        
    }

    g_iMaxPowerLoopCount = SLT_LOOP_CNT;
    
    return 0;
}
arch_initcall(cpu0_slt_ca9_max_power_init);
arch_initcall(cpu1_slt_ca9_max_power_init);
arch_initcall(ca9_max_power_loop_count_init);
