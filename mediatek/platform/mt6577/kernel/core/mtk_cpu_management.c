#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/dmi.h>
#include <linux/acpi.h>
#include <linux/thermal.h>
#include <linux/platform_device.h>
#include <linux/aee.h>
#include <linux/xlog.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>

#include "mach/mtk_thermal_monitor.h"
#include <mach/system.h>
#include "mach/mt_typedefs.h"
#include "mach/mt_thermal.h"
#include "mach/mtk_cpu_management.h"

#ifdef CONFIG_SMP
#include <mach/hotplug.h>
#endif

extern void cpufreq_thermal_protect(int limited_freq);
static int mtk_cpu_management_debug_log = 1;

#define mtk_cpu_management_dprintk(fmt, args...)   \
do {                                    \
    if (mtk_cpu_management_debug_log) {                \
        xlog_printk(ANDROID_LOG_INFO, "Power/cpu_management", fmt, ##args); \
    }                                   \
} while(0)


static DEFINE_MUTEX(management_lock);
struct module_info info_ori;
int cpu_opp_mask(char *name, int *mask, bool flag)
{
	struct list_head *p = NULL;
	struct module_info *info = NULL;
	bool new_module = true;
	int Final_Mask[11]={1,1,1,1,1,1,1,1,1,1,1};
	int i, lowest_step=0;

#ifndef CONFIG_SMP
	return 1;
#endif

	mtk_cpu_management_dprintk("cpu_opp_mask: name=%s, flag=%d\n",name, flag);

	mutex_lock(&management_lock);
	list_for_each(p, &info_ori.link)
	{
		info = list_entry(p, struct module_info, link);
		if (!strcmp(info->module_name, name))
		{
			mtk_cpu_management_dprintk("cpu_opp_mask: find old module name=%s\n",name);
			new_module = false;
			break;
		}
	}

	if((new_module==true) && (flag==true))
	{
		info = NULL;
		info = kmalloc(sizeof(struct module_info), GFP_KERNEL);
		if(!info)
		{
			mtk_cpu_management_dprintk("cpu_opp_mask:%s malloc fail\n",name);
			mutex_unlock(&management_lock);
			return 1;
		}
		else
		{
			snprintf(info->module_name,sizeof(info->module_name) ,"%s", name);
			list_add_tail(&info->link, &info_ori.link);
			for(i=0; i<11; i++)
				info->mask[i] = mask[i];
			mtk_cpu_management_dprintk("cpu_opp_mask: Add new module--%s\n",info->module_name);
			mtk_cpu_management_dprintk("cpu_opp_mask: new module mask----%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", 
										info->mask[0],info->mask[1],info->mask[2],info->mask[3],
										info->mask[4],info->mask[5],info->mask[6],info->mask[7],
										info->mask[8],info->mask[9], info->mask[10]);
		}
	}
	else if((new_module==false) && (flag==false))
	{
		mtk_cpu_management_dprintk("cpu_opp_mask: Remove old module--%s\n",info->module_name);
		list_del(&info->link);
		kfree(info);
	}
	else if((new_module==true) && (flag==false))
	{
		mtk_cpu_management_dprintk("cpu_opp_mask: This module name not exist and need be created first\n");
		mutex_unlock(&management_lock);
		return 1;
	}
	else if((new_module==false) && (flag==true))
	{
		mtk_cpu_management_dprintk("cpu_opp_mask: Change module mask\n");
		for(i=0; i<11; i++)
				info->mask[i] = mask[i];
	}
	
	list_for_each(p, &info_ori.link)
	{
		info = NULL;
		info = list_entry(p, struct module_info, link);
		mtk_cpu_management_dprintk("cpu_opp_mask: exist module--%s\n",info->module_name);
		mtk_cpu_management_dprintk("cpu_opp_mask: exist module mask--%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
										info->mask[0],info->mask[1],info->mask[2],info->mask[3],
										info->mask[4],info->mask[5],info->mask[6],info->mask[7],
										info->mask[8],info->mask[9],info->mask[10]);
		for(i=0; i<11; i++)
			Final_Mask[i] &= info->mask[i];
	}
	mtk_cpu_management_dprintk("cpu_opp_mask: Final mask---------%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
										Final_Mask[0],Final_Mask[1],Final_Mask[2],Final_Mask[3],
										Final_Mask[4],Final_Mask[5],Final_Mask[6],Final_Mask[7],
										Final_Mask[8],Final_Mask[9],Final_Mask[10]);

	for(i=0; i<11; i++)
	{
		if(Final_Mask[i] != 0)
		{
			lowest_step = i;
			mtk_cpu_management_dprintk("cpu_opp_mask: lowest step=%d\n", lowest_step);
			break;
		}
	}

	switch(lowest_step)
	{
		case 0:
		{
#ifdef CONFIG_SMP			
			mtk_hotplug_mechanism_thermal_protect(2);
#endif			
			cpufreq_thermal_protect(DVFS_OPP0);
			break;
		}	
		case 1:	
		{
#ifdef CONFIG_SMP			
			mtk_hotplug_mechanism_thermal_protect(2);	
#endif			
			cpufreq_thermal_protect(DVFS_OPP1);
			break;
		}
		case 2:
		{
#ifdef CONFIG_SMP			
			mtk_hotplug_mechanism_thermal_protect(2);
#endif			
			cpufreq_thermal_protect(DVFS_OPP2);
			break;
		}	
		case 3:
		{	
#ifdef CONFIG_SMP			
			mtk_hotplug_mechanism_thermal_protect(2);
#endif			
			cpufreq_thermal_protect(DVFS_OPP3);
			break;
		}	
		case 4:
		{
#ifdef CONFIG_SMP			
			mtk_hotplug_mechanism_thermal_protect(2);
#endif			
			cpufreq_thermal_protect(DVFS_OPP4);
			break;
		}	
		case 5:	
		{	
#ifdef CONFIG_SMP			
			mtk_hotplug_mechanism_thermal_protect(1);
#endif			
			cpufreq_thermal_protect(DVFS_OPP1);
			break;
		}	
		case 6:
		{
#ifdef CONFIG_SMP			
			cpufreq_thermal_protect(DVFS_OPP2);
#endif			
			mtk_hotplug_mechanism_thermal_protect(1);
			break;
		}	
		case 7:	
		{
#ifdef CONFIG_SMP			
			cpufreq_thermal_protect(DVFS_OPP3);
#endif			
			mtk_hotplug_mechanism_thermal_protect(1);
			break;
		}
		case 8:
		{
#ifdef CONFIG_SMP			
			cpufreq_thermal_protect(DVFS_OPP4);
#endif			
			mtk_hotplug_mechanism_thermal_protect(1);
			break;
		}
		case 9:	
		{
#ifdef CONFIG_SMP			
			cpufreq_thermal_protect(DVFS_OPP5);
#endif			
			mtk_hotplug_mechanism_thermal_protect(1);
			break;
		}
		case 10:	
		{
#ifdef CONFIG_SMP			
			cpufreq_thermal_protect(DVFS_OPP6);
#endif			
			mtk_hotplug_mechanism_thermal_protect(1);
			break;
		}
		default:
			break;
	}
	mutex_unlock(&management_lock);
	return 0;
}
EXPORT_SYMBOL(cpu_opp_mask);


static int __init mtk_cpu_management_init(void)
{
	INIT_LIST_HEAD(&info_ori.link);
	return 0;
}

static void __exit mtk_cpu_management_exit(void)
{
	
}

module_init(mtk_cpu_management_init);
module_exit(mtk_cpu_management_exit);
