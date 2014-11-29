#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/fs.h>


#include "mach/mt_reg_base.h"
#include "mach/mt_device_apc.h"
#include "mach/mt_typedefs.h"
#include "mach/mt_boot.h"
#include "mach/sync_write.h"
#include "mach/irqs.h"

static DEFINE_SPINLOCK(g_devapc_lock);
static unsigned long g_devapc_flags;

#define MOD_NO_IN_1_DEVAPC                  16
#define DEVAPC_MODULE_MAX_NUM               32  


// device apc permissions 
typedef enum
{
	E_NO_PROT=0,
	E_SEC_RW,
	E_SEC_RW_NS_R,
	E_NO_ACCESS_BY_DOM,
	E_MAX_APC_ATTR
}APC_ATTR;

// device apc index number
typedef enum
{
	E_DEVAPC0=0,
	E_DEVAPC1,	
	E_DEVMM1APC0,
	E_DEVMM1APC1,
	E_DEVMDAPC0,
	E_DEVMDAPC1,
	E_DEVDSPAPC0,
	E_MAX_DEVAPC
}DEVAPC_NUM;

// domain index number
typedef enum
{
	E_AP_MCU = 0,
	E_FCORE ,
	E_MDMCU ,	
}E_MASK_DOM;


typedef struct {
    const char      *index;
    bool            fbd;
} DEVICE_INFO;


static DEVICE_INFO D_APC0_Devices[] = {
    {"1",                                           FALSE    },
    {"2",                                           FALSE    },
    {"3",                                           TRUE    },
    {"4",                                           TRUE    },
    {"5",                                           TRUE    },
    {"6",                                           TRUE    },
    {"7",                                           FALSE    },
    {"8",                                           TRUE    },
    {"9",                                           TRUE    },
    {"10",                                          TRUE    },
    {"11",                                          TRUE    },
    {"12",                                          TRUE    },
    {"13",                                          TRUE    },
    {"14",                                          FALSE    },
    {"15",                                          TRUE    },
    {"16",                                          TRUE    },
    {"17",                                          FALSE    },
    {"18",                                          FALSE    },
    {"19",                                          TRUE    },
    {"20",                                          TRUE    },
    {"21",                                          FALSE    },
    {"22",                                          TRUE    },
    {"23",                                          TRUE    },
    {"24",                                          FALSE    },
    {"25",                                          FALSE    },
    {"26",                                          FALSE    },
    {"27",                                          TRUE    },
    {"28",                                          TRUE    },
    {"29",                                          TRUE    },
    {"30",                                          TRUE    },    
    {NULL,                                          FALSE    },
};


static DEVICE_INFO D_APC1_Devices[] = {
    {"1",                                           TRUE    },
    {"2",                                           FALSE    },
    {"3",                                           TRUE    },
    {"4",                                           FALSE    },
    {"5",                                           FALSE    },
    {"6",                                           FALSE    },
    {"7",                                           TRUE    },
    {"8",                                           FALSE    },
    {"9",                                           TRUE    },
    {"10",                                          FALSE    },
    {"11",                                          TRUE    },
    {"12",                                          TRUE    },
    {"13",                                          FALSE    },
    {"14",                                          FALSE    },
    {"15",                                          TRUE    },
    {"16",                                          TRUE    },
    {"17",                                          TRUE    },    
    {"18",                                          TRUE    },
    {"19",                                          TRUE    },
    {"20",                                          TRUE    },
    {"21",                                          TRUE    },
    {"22",                                          TRUE    },
    {"23",                                          TRUE    },    
    {"24",                                          TRUE    },    
    {"25",                                          TRUE    },
    {"26",                                          TRUE    },    
    {"27",                                          FALSE    },
    {NULL,                                          FALSE    },
};


static struct cdev* g_devapc_ctrl = NULL;
static dev_t g_devapc_devno;


/*
 * set_module_apc: set module permission on device apc.
 * @module: the moudle to specify permission
 * @devapc_num: device apc index number (device apc 0 or 1)
 * @domain_num: domain index number (AP or MD domain)
 * @permission_control: specified permission
 * no return value.
 */
static void set_module_apc(unsigned int module, DEVAPC_NUM devapc_num, 
    E_MASK_DOM domain_num , APC_ATTR permission_control)
{
	if (E_DEVAPC0 == devapc_num )
	{
		if ( module < (MOD_NO_IN_1_DEVAPC) )
		{
			if (E_AP_MCU == domain_num)
			{
				writel(readl(AP_DEVAPC0_D0_APC_0) & ~(0x3 << (2 * module)), AP_DEVAPC0_D0_APC_0);
				writel(readl(AP_DEVAPC0_D0_APC_0) | (permission_control << (2 * module)), AP_DEVAPC0_D0_APC_0);
			}
            else if (E_MDMCU == domain_num)
            {
            	writel(readl(AP_DEVAPC0_D2_APC_0) & ~(0x3 << (2 * module)), AP_DEVAPC0_D2_APC_0);
				writel(readl(AP_DEVAPC0_D2_APC_0) | (permission_control << (2 * module)), AP_DEVAPC0_D2_APC_0);
            }
            else
            {
                printk("[DEVAPC] ERROR, The setting is error, please check if domain master setting is correct or not !\n");
            }
		}
		else
		{
		    module -= MOD_NO_IN_1_DEVAPC;
			if (E_AP_MCU == domain_num)
			{
				writel(readl(AP_DEVAPC0_D0_APC_1) & ~(0x3 << (2 * module)), AP_DEVAPC0_D0_APC_1);
				writel(readl(AP_DEVAPC0_D0_APC_1) | (permission_control << (2 * module)), AP_DEVAPC0_D0_APC_1);
			}
            else if (E_MDMCU == domain_num)
            {
            	writel(readl(AP_DEVAPC0_D2_APC_1) & ~(0x3 << (2 * module)), AP_DEVAPC0_D2_APC_1);
				writel(readl(AP_DEVAPC0_D2_APC_1) | (permission_control << (2 * module)), AP_DEVAPC0_D2_APC_1);
            }
            else
            {
                printk("[DEVAPC] ERROR, The setting is error, please check if domain master setting is correct or not !\n");
            }
		}
	}
	else if (E_DEVAPC1 == devapc_num )
	{
		if ( module < (MOD_NO_IN_1_DEVAPC) )
		{
			if (E_AP_MCU == domain_num)
			{
				writel(readl(AP_DEVAPC1_D0_APC_0) & ~(0x3 << (2 * module)), AP_DEVAPC1_D0_APC_0);
				writel(readl(AP_DEVAPC1_D0_APC_0) | (permission_control << (2 * module)), AP_DEVAPC1_D0_APC_0);
			}
            else if (E_MDMCU == domain_num)
            {
            	writel(readl(AP_DEVAPC1_D2_APC_0) & ~(0x3 << (2 * module)), AP_DEVAPC1_D2_APC_0);
				writel(readl(AP_DEVAPC1_D2_APC_0) | (permission_control << (2 * module)), AP_DEVAPC1_D2_APC_0);
            }
            else
            {
                printk("[DEVAPC] ERROR, The setting is error, please check if domain master setting is correct or not !\n");                  
            }
		}
		else
		{
            module -= MOD_NO_IN_1_DEVAPC;
			if (E_AP_MCU == domain_num)
			{
				writel(readl(AP_DEVAPC1_D0_APC_1) & ~(0x3 << (2 * module)), AP_DEVAPC1_D0_APC_1);
				writel(readl(AP_DEVAPC1_D0_APC_1) | (permission_control << (2 * module)), AP_DEVAPC1_D0_APC_1);
			}
            else if (E_MDMCU == domain_num)
            {
            	writel(readl(AP_DEVAPC1_D2_APC_1) & ~(0x3 << (2 * module)), AP_DEVAPC1_D2_APC_1);
				writel(readl(AP_DEVAPC1_D2_APC_1) | (permission_control << (2 * module)), AP_DEVAPC1_D2_APC_1);
            }
            else
            {
                printk("[DEVAPC] ERROR, The setting is error, please check if domain master setting is correct or not !\n");                   
            }
		}
	}
}


/*
 * unmask_module_irq: unmask device apc irq for specified module.
 * @module: the moudle to unmask
 * @devapc_num: device apc index number (device apc 0 or 1)
 * @domain_num: domain index number (AP or MD domain)
 * no return value.
 */
static void unmask_module_irq(unsigned int module, DEVAPC_NUM devapc_num, E_MASK_DOM 
    domain_num)
{
    unsigned int module_index = (0x1 << module);
    
	if (E_DEVAPC0 == devapc_num )
	{
		if (E_AP_MCU == domain_num)
		{
		    writel(readl(AP_DEVAPC0_D0_VIO_MASK) & ~module_index, AP_DEVAPC0_D0_VIO_MASK);
		}
        else if (E_MDMCU == domain_num)
        {
            writel(readl(AP_DEVAPC0_D2_VIO_MASK) & ~module_index, AP_DEVAPC0_D2_VIO_MASK);
        }
	}
	else if (E_DEVAPC1 == devapc_num )
	{
		if (E_AP_MCU == domain_num)
		{
		    writel(readl(AP_DEVAPC1_D0_VIO_MASK) & ~module_index, AP_DEVAPC1_D0_VIO_MASK);
		}
		else if (E_MDMCU == domain_num)
		{
		    writel(readl(AP_DEVAPC1_D2_VIO_MASK) & ~module_index, AP_DEVAPC1_D2_VIO_MASK);
		}
	}
}


/*
 * clear_vio_status: clear violation status for each module.
 * @module: the moudle to clear violation status
 * @devapc_num: device apc index number (device apc 0 or 1)
 * @domain_num: domain index number (AP or MD domain)
 * no return value.
 */
static void clear_vio_status(unsigned int module, DEVAPC_NUM devapc_num, E_MASK_DOM 
    domain_num)
{
    unsigned int module_index = (0x1 << module);

    if (E_DEVAPC0 == devapc_num )
	{
	    if (E_AP_MCU == domain_num)
		{
			writel(readl(AP_DEVAPC0_D0_VIO_STA) | module_index, AP_DEVAPC0_D0_VIO_STA);
			while((readl(AP_DEVAPC0_D0_VIO_STA) & module_index) != 0);
		}
		else if (E_MDMCU == domain_num)
		{
			writel(readl(AP_DEVAPC0_D2_VIO_STA) | module_index, AP_DEVAPC0_D2_VIO_STA);
			while((readl(AP_DEVAPC0_D2_VIO_STA) & module_index) != 0);
		}
	}
    else if (E_DEVAPC1 == devapc_num )
	{
        if (E_AP_MCU == domain_num)
        {
            writel(readl(AP_DEVAPC1_D0_VIO_STA) | module_index, AP_DEVAPC1_D0_VIO_STA);
			while((readl(AP_DEVAPC1_D0_VIO_STA) & module_index) != 0);
        }
        else if (E_MDMCU == domain_num)
        {
            writel(readl(AP_DEVAPC1_D2_VIO_STA) | module_index, AP_DEVAPC1_D2_VIO_STA);
            while((readl(AP_DEVAPC1_D2_VIO_STA) & module_index) != 0);
        }
	}
	else
    {
        printk("[DEVAPC] ERROR: clear_vio_status , The setting is error, please check if the setting is correct or not !\n");  
    }
}



static irqreturn_t devapc_violation_irq(int irq, void *dev_id)
{  
    unsigned int dbg0 = 0, dbg1 = 0;
    unsigned int master_ID;
    unsigned int domain_ID;
    unsigned int r_w_violation;
    DEVAPC_NUM apc_index = E_MAX_DEVAPC;
    int module_index;

    
    if ( (readl(AP_DEVAPC0_DXS_VIO_STA) & 0x3) == 0)
    {
        printk("[DEVAPC] ERROR AP_DEVAPC0_DXS_VIO_STA not device apc 0 or 1 violation!");
        return IRQ_NONE;
    }

    spin_lock_irqsave(&g_devapc_lock, g_devapc_flags);

    
    if ( (readl(AP_DEVAPC0_DXS_VIO_STA) & 0x1) > 0)
    {
        dbg0 = readl(AP_DEVAPC0_VIO_DBG0);
        dbg1 = readl(AP_DEVAPC0_VIO_DBG1);
        apc_index = E_DEVAPC0;
        printk("[DEVAPC]apc 0 vio \n");
    }
    else if ( (readl(AP_DEVAPC0_DXS_VIO_STA) & 0x2) > 0)
    {
        dbg0 = readl(AP_DEVAPC1_VIO_DBG0);
        dbg1 = readl(AP_DEVAPC1_VIO_DBG1);
        apc_index = E_DEVAPC1;
        printk("[DEVAPC]apc 1 vio \n");
    }
    
    master_ID = dbg0 & 0x000007FF;
    domain_ID = (dbg0 >>12) & 0x00000003;
    r_w_violation = (dbg0 >> 28) & 0x00000003;
    
    //printk("\r\n [DEVAPC] Violation!!! DBG0 = %x, DBG1 = %x\n\r", dbg0, dbg1);
    printk("Current Proc is \"%s \" (pid: %i) \n", current->comm, current->pid);
    printk("Vio Addr is 0x%x \n", dbg1);
    printk("Vio Master ID is 0x%x \n", master_ID);
    printk("Vio Domain ID is 0x%x \n", domain_ID);
    //printk("\r n  AP_DEVAPC0_D0_VIO_STA is 0x%x \n\r", readl(AP_DEVAPC0_D0_VIO_STA));
    //printk("\r n  AP_DEVAPC1_D0_VIO_STA is 0x%x \n\r", readl(AP_DEVAPC1_D0_VIO_STA));

    if(r_w_violation == 1)
    {
      printk("R/W Vio : Write \n");
    }
    else
    {
      printk("R/W Vio : Read \n");
    }

    if (E_DEVAPC0 == apc_index)
    {
        for (module_index = 0; module_index< DEVAPC_MODULE_MAX_NUM; module_index++)
        {
            if (NULL == D_APC0_Devices[module_index].index)
                break;
                   
            if (TRUE == D_APC0_Devices[module_index].fbd)
                clear_vio_status(module_index, E_DEVAPC0, E_AP_MCU);
        }
        
        writel(0x1, AP_DEVAPC0_DXS_VIO_STA);
        while(( readl(AP_DEVAPC0_DXS_VIO_STA) & 0x1) != 0);
        
        mt65xx_reg_sync_writel(0x80000000 , AP_DEVAPC0_VIO_DBG0);
        dbg0 = readl(AP_DEVAPC0_VIO_DBG0);
        dbg1 = readl(AP_DEVAPC0_VIO_DBG1);
    }
    else if (E_DEVAPC1 == apc_index)
    {
        for (module_index = 0; module_index< DEVAPC_MODULE_MAX_NUM; module_index++)
        {
            if (NULL == D_APC1_Devices[module_index].index)
                break;
                   
            if (TRUE == D_APC1_Devices[module_index].fbd)
                clear_vio_status(module_index, E_DEVAPC1, E_AP_MCU);
        }

        writel(0x2, AP_DEVAPC0_DXS_VIO_STA);
        while(( readl(AP_DEVAPC0_DXS_VIO_STA) & 0x2) != 0);

        mt65xx_reg_sync_writel(0x80000000 , AP_DEVAPC1_VIO_DBG0);        
        dbg0 = readl(AP_DEVAPC1_VIO_DBG0);
        dbg1 = readl(AP_DEVAPC1_VIO_DBG1);
    }

    if ((dbg0 !=0) || (dbg1 !=0)) 
    {
        printk("[DEVAPC] FAILED!\n");
        printk("[DEVAPC] DBG0 = %x, DBG1 = %x\n", dbg0, dbg1);
    }

    spin_unlock_irqrestore(&g_devapc_lock, g_devapc_flags);
  
    return IRQ_HANDLED;
}





/*
 * devapc_init: module init function.
 */
static int __init devapc_init(void)
{
    int ret;
    int module_index = 0;
    unsigned int apc0_dbg_0, apc0_dbg_1, apc1_dbg_0, apc1_dbg_1;
    CHIP_VER ver = get_chip_ver();

    printk("[DEVAPC] DEVAPC module init. \n");
    
    if ((CHIP_6575_E3 >= ver) && (CHIP_6575_E1 <= ver))
    {
        printk("[DEVAPC] No need to init on mt6575 \n");
        return 0;
    }


    if (alloc_chrdev_region(&g_devapc_devno, 0, 1,"devapc"))
    {
        printk("[DEVAPC] Allocate devapc device no failed\n");
        return -EAGAIN;
    }
    else
    {
        printk("[DEVAPC] Device number is: %d\n", (int)g_devapc_devno);
    }
    
    g_devapc_ctrl = cdev_alloc();
    g_devapc_ctrl->owner = THIS_MODULE;
    ret = cdev_add(g_devapc_ctrl, g_devapc_devno, 1);

    if(ret != 0)
    {
        printk( "[DEVAPC] Failed to add devapc device! (%d)\n", ret);
        return ret;
    }
  
    // clear the violation
    writel(0x80000000, AP_DEVAPC0_VIO_DBG0); // clear apc0 dbg info if any
    writel(0x80000000, AP_DEVAPC1_VIO_DBG0); // clear apc1 dbg info if any
    
    apc0_dbg_0 = readl(AP_DEVAPC0_VIO_DBG0);
    apc0_dbg_1 = readl(AP_DEVAPC0_VIO_DBG1);
    apc1_dbg_0 = readl(AP_DEVAPC1_VIO_DBG0);
    apc1_dbg_1 = readl(AP_DEVAPC1_VIO_DBG1);
    
    if ((apc0_dbg_0 !=0) || (apc0_dbg_1 !=0) || (apc1_dbg_0 != 0) 
        || (apc1_dbg_1 != 0)) 
    {
        printk("\r\n [DEVAPC] FAILED!\n\r");
        printk("\r\n [DEVAPC] DEVAPC0_VIO_DBG0 = %x, DEVAPC0_VIO_DBG1 = %x , DEVAPC1_VIO_DBG0 = %x, DEVAPC1_VIO_DBG1 = %x  ", apc0_dbg_0, 
            apc0_dbg_1, apc1_dbg_0, apc1_dbg_1);
    }


    writel(readl(AP_DEVAPC0_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), AP_DEVAPC0_APC_CON);
    writel(readl(AP_DEVAPC1_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), AP_DEVAPC1_APC_CON);
    writel(readl(MM_DEVAPC0_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), MM_DEVAPC0_APC_CON);
    writel(readl(MM_DEVAPC1_APC_CON) &  (0xFFFFFFFF ^ (1<<2)), MM_DEVAPC1_APC_CON);
    // clean violation status & unmask device apc 0 & 1 
    writel(0x0000007F, AP_DEVAPC0_DXS_VIO_STA);
    writel(0x00FF00FC, AP_DEVAPC0_DXS_VIO_MASK);

    for (module_index = 0; module_index< DEVAPC_MODULE_MAX_NUM; module_index++)
    {
        if (NULL == D_APC0_Devices[module_index].index)
            break;
            
        if (TRUE == D_APC0_Devices[module_index].fbd)
        {
            clear_vio_status(module_index, E_DEVAPC0, E_AP_MCU);
            unmask_module_irq(module_index, E_DEVAPC0 , E_AP_MCU);
            set_module_apc(module_index, E_DEVAPC0, E_MDMCU, E_NO_ACCESS_BY_DOM);
        }
    }

    for (module_index = 0; module_index< DEVAPC_MODULE_MAX_NUM; module_index++)
    {
        if (NULL == D_APC1_Devices[module_index].index)
            break;
            
        if (TRUE == D_APC1_Devices[module_index].fbd)
        {
            clear_vio_status(module_index, E_DEVAPC1, E_AP_MCU);
            unmask_module_irq(module_index, E_DEVAPC1 , E_AP_MCU);
            set_module_apc(module_index, E_DEVAPC1, E_MDMCU, E_NO_ACCESS_BY_DOM);
        }
    }

    /* 
     * NoteXXX: Interrupts of vilation (including SPC in SMI, or EMI MPU) are triggered by the device APC.
     *          Need to share the interrupt with the SPC driver. 
     */
    ret = request_irq(MT_APARM_DOMAIN_IRQ_ID, (irq_handler_t)devapc_violation_irq, IRQF_TRIGGER_LOW | IRQF_SHARED, 
        "mt6577_devapc", &g_devapc_ctrl);    
    disable_irq(MT_APARM_DOMAIN_IRQ_ID);
    enable_irq(MT_APARM_DOMAIN_IRQ_ID);
    
    if(ret != 0)
    {
        printk( "[DEVAPC] Failed to request irq! (%d)\n", ret);
        return ret;
    }
  
    return 0;
}

/*
 * devapc_exit: module exit function.
 */
static void __exit devapc_exit(void)
{

    CHIP_VER ver = get_chip_ver();
       
    if ((CHIP_6575_E3 >= ver) && (CHIP_6575_E1 <= ver))
    {
       return;
    }
    
    free_irq(MT_APARM_DOMAIN_IRQ_ID ,  g_devapc_ctrl);   
    cdev_del(g_devapc_ctrl);
    printk("[DEVAPC] DEVAPC module exit\n");
}

module_init(devapc_init);
module_exit(devapc_exit);
MODULE_DESCRIPTION("Mediatek Device APC Driver");
MODULE_LICENSE("GPL");

