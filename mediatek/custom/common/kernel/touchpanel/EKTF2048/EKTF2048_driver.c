/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/byteorder/generic.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif 
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/rtpm_prio.h>

#include "tpd.h"
#include <cust_eint.h>
//<2011/12/07-1198-kevincheng,Fix FM LCM Rotate Issue
//<2012/05/15 yaotsulin, add argonmini driver
#if defined(MT6573)
#include <mach/mt6573_boot.h>
#endif
#if defined(MT6575)
#include <mach/mt_boot.h>
#include <mach/mt_pm_ldo.h>
#endif
#if defined(MT6577)
#include <mach/mt_boot.h>
#include <mach/mt_pm_ldo.h>
#endif
//>2012/05/15 yaotsulin, add argonmini driver
//>2011/12/07-1198-kevincheng

//<2012/5/23-joshualee, the TP customization for 7909
#if defined(ARIMA_PROJECT_7909)
#undef pr_info
#define pr_info printk
#endif
//>2012/5/23-joshualee

#ifndef TPD_NO_GPIO 
#include "cust_gpio_usage.h"
#endif

#define CMD_S_PKT			0x52
#define CMD_R_PKT			0x53
//IAP
//<2012/04/30 Yuting Shih,Modified when based on Android 4.x
#define VELOCITY_CUSTOM_EKTF2K
#if defined(VELOCITY_CUSTOM_EKTF2K)
  #include <linux/device.h>
  #include <linux/miscdevice.h>
  #include <asm/uaccess.h>
#else
#include <linux/uaccess.h>
//#include "tpd_EKTF2048.h"
#include <linux/uaccess.h>
//#include "tpd_EKTF2048.h"
#endif /* End.. (VELOCITY_CUSTOM_EKTF2K) */
//>2012/04/30 Yuting Shih.
int RECOVERY=0x00;
int work_lock=0x00;
#define PWR_STATE_DEEP_SLEEP	0
#define PWR_STATE_NORMAL		1
#define PWR_STATE_MASK			BIT(3)
#define ELAN_IOCTLID	0xD0
#define IOCTL_I2C_SLAVE	_IOW(ELAN_IOCTLID,  1, int)
#define IOCTL_MAJOR_FW_VER  _IOR(ELAN_IOCTLID, 2, int)
#define IOCTL_MINOR_FW_VER  _IOR(ELAN_IOCTLID, 3, int)
#define IOCTL_RESET  _IOR(ELAN_IOCTLID, 4, int)
#define IOCTL_IAP_MODE_LOCK  _IOR(ELAN_IOCTLID, 5, int)
#define IOCTL_CHECK_RECOVERY_MODE  _IOR(ELAN_IOCTLID, 6, int)
#define IOCTL_FW_VER  _IOR(ELAN_IOCTLID, 7, int)
#define IOCTL_X_RESOLUTION  _IOR(ELAN_IOCTLID, 8, int)
#define IOCTL_Y_RESOLUTION  _IOR(ELAN_IOCTLID, 9, int)
#define IOCTL_FW_ID  _IOR(ELAN_IOCTLID, 10, int)
#define IOCTL_IAP_MODE_UNLOCK  _IOR(ELAN_IOCTLID, 11, int)
#define CMD_W_PKT			0x54
extern long strncpy_from_user(char *dst, const char __user *src, long count);
//IAP
#define HELLO_PKT			0x55
#define NORMAL_PKT			0x5A   
#define FINGER_ID_INDEX 7
#define KEY_ID_INDEX 7
#define FINGER_NUM	2	  
#define IDX_FINGER	0x01	
#define FINGER_ID       0x01 
#define PACKET_SIZE	8

//wingwei 2011/12/09
#if defined(ARIMA_PROJECT_ARGONMINI_6575) || defined(ARIMA_PROJECT_ARGONMINI_6577) || \
    defined(ARIMA_PROJECT_7909) || defined(ARIMA_PROJECT_HAWK35)
#define FRAME_X	320
#define FRAME_Y	480
#else
#define FRAME_X	320
#define FRAME_Y	480
#endif
 
//<2012/05/15 Yuting Shih, Modified for the define of key event
//#define EKTF2048_KEY_EVENT 0x78
//<2011/12/08-1252-kevincheng
#if defined(EVB)
#define EKTF2048_KEY_SEARCH 0x40
#define EKTF2048_KEY_BACK   0x20
#define EKTF2048_KEY_HOME   0x10
#define EKTF2048_KEY_MENU   0x08
#else
#define EKTF2048_KEY_SEARCH 0x20
#define EKTF2048_KEY_BACK   0x10
#define EKTF2048_KEY_HOME   0x08
#define EKTF2048_KEY_MENU   0x04
#endif
//>2011/12/08--kevincheng
#define EKTF2048_KEY_PRESS  0x01
#define EKTF2048_KEY_EVENT  (EKTF2048_KEY_SEARCH|EKTF2048_KEY_BACK|EKTF2048_KEY_HOME|EKTF2048_KEY_MENU)
//>2012/05/15 Yuting Shih.

//<2012/05/23-samhuang, temp solution to change home key's key code
#undef KEY_HOME
#define KEY_HOME 172
//>2012/05/23-samhuang

#define EKTF2048_KEY_RELEASE 0x0
int button = 0;
int ELAN_X_MAX = 0;
int ELAN_Y_MAX = 0;
int fw_ver = 0;
int fw_id = 0;
int bc_ver = 0;
int VIBRATOR_FLAG = 0;

//<2012/07/16 Yuting Shih, Masked if non-usage
#if 0
//<2012/02/14-3224-kevincheng,Porting TP Self Test
int RESO_ERR = -1;
int RESB_ERR = -2;
int RESD_ERR = -3;
int VEND_ERR = -4;
int ERR_OFFSET = 2;
int ERR_BASE = 4;
int ERR_dV = 5;
int TP_ST = 0;
int _add = 0;
int _data = 0;
int _ERROR = -5;
//>2012/02/14-3224-kevincheng
//<2012/03/03-3795-kevincheng,TP ESD Mechanism
int _TP_Thr = 0;
int _TP_init = 0;
//>2012/03/03-3795-kevincheng
//<2012/03/28-4654-kevincheng,Add TP Supplier checking mechanism
#define supplier_checking 1
//>2012/03/28-4654-kevincheng

#endif
//>2012/07/16 Yuting Shih.

#define CHR_CON0	(0xF7000000+0x2FA00)
extern struct tpd_device *tpd;
extern int tpd_show_version;
extern int tpd_em_debuglog;
extern int tpd_register_flag;
extern int tpd_load_status;
extern int tpd_em_set_freq;

#if defined(CTP_CHECK_FM)
static int16_t fm_current_frequency=0;
#endif
static int tpd_flag = 0;
static int tpd_halt=0;
static struct task_struct *thread = NULL;
static DECLARE_WAIT_QUEUE_HEAD(waiter);

#ifdef TPD_HAVE_BUTTON 
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;
#endif
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;
#endif
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;
#endif

static void tpd_eint_interrupt_handler(void);
static int touch_event_handler(void *unused);
static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int tpd_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int tpd_i2c_remove(struct i2c_client *client);
extern void mt65xx_eint_unmask(unsigned int line);
extern void mt65xx_eint_mask(unsigned int line);
extern void mt65xx_eint_set_hw_debounce(kal_uint8 eintno, kal_uint32 ms);
extern kal_uint32 mt65xx_eint_set_sens(kal_uint8 eintno, kal_bool sens);
extern void mt65xx_eint_registration(kal_uint8 eintno, kal_bool Dbounce_En,
                                     kal_bool ACT_Polarity, void (EINT_FUNC_PTR)(void),
                                     kal_bool auto_umask);

//<2012/4/25 Yuting Shih, Add for the I2C channel id and table information
  #define   ELAN_I2C_SLAVE_ADDR         0x2A

#if defined(ARIMA_PROJECT_ARGONMINI_6575) || defined(ARIMA_PROJECT_ARGONMINI_6577)
  #define   ELAN_I2C_CHANNEL_ID         1
#elif defined(ARIMA_PROJECT_7909) || defined(ARIMA_PROJECT_HAWK35)
  #define   ELAN_I2C_CHANNEL_ID         0
#else
  #define   ELAN_I2C_CHANNEL_ID         0
#endif

#if defined(TPD_DEVICE)
  /** string maximum size: 20 **/
  #define   ELAN_TPD_DEVICE             TPD_DEVICE
#else
  #define   ELAN_TPD_DEVICE             "mtk-tpd" //"ektf2k-tpd"
#endif
//>2012/4/25 Yuting Shih,Add.
static struct i2c_client *i2c_client = NULL;
static const struct i2c_device_id tpd_i2c_id[] = {{ ELAN_TPD_DEVICE /*"mtk-tpd"*/,0},{}};
//<2012/05/15 Yuting Shih,Masked when based on Android 4.x
//#if 1//defined(EVB) 
//static unsigned short force[] = {1, 0x2a, I2C_CLIENT_END,I2C_CLIENT_END};
//#else
//static unsigned short force[] = {ELAN_I2C_CHANNEL_ID, ELAN_I2C_SLAVE_ADDR, I2C_CLIENT_END,I2C_CLIENT_END};
//#endif
//static const unsigned short * const forces[] = { force, NULL };
////static struct i2c_client_address_data addr_data = { .forces = forces,};
//>2012/05/15 Yuting Shih.
static struct i2c_board_info __initdata ektf2048_i2c_tpd={ I2C_BOARD_INFO( ELAN_TPD_DEVICE, (ELAN_I2C_SLAVE_ADDR >> 1))};
struct i2c_driver tpd_i2c_driver = {                       
    .probe = tpd_i2c_probe,                                   
    .remove = tpd_i2c_remove,                           
    .detect = tpd_i2c_detect,                           
    .driver.name = ELAN_TPD_DEVICE, //"mtk-tpd",
    .id_table = tpd_i2c_id,                             
  //.address_data = &addr_data,                        
}; 

//IAP
static struct elan_ktf2k_ts_data *private_ts;
static int __fw_packet_handler(struct i2c_client *client);

//<2012/05/16 Yuting Shih, Add for touch panel power supply
/*************************************************
** For MT6575 ArgonMini 6575,Android 4.x
**  The I2C1 power
**    - DVDD_CAM(VCAMD2) : (I2C1)SCL1, SDA1
**
** For MT6575 Hawk35,Android 4.x
**  The power
**    - VGAM_AF(VAF) : VCC(2.8V)
**
**************************************************/
#define   TPD_HWPWM_NAME        "EKTF2048"

typedef enum
tpd_hwPower_enable_ctrl
{
   TPD_HWPWM_OFF = 0
  ,TPD_HWPWM_ON
};
static BOOL   tpd_hwpwm_enable  = FALSE;

static int
tpd_hwPower_ctrl(
int   ctrl_OnOff
)
{
  switch( ctrl_OnOff )
  {
    case TPD_HWPWM_ON:
    {
      if( FALSE == tpd_hwpwm_enable )
      {
    #if defined(ARIMA_PROJECT_ARGONMINI_6575) || defined(ARIMA_PROJECT_ARGONMINI_6577)
        hwPowerOn( MT65XX_POWER_LDO_VCAM_IO, VOL_2800, TPD_HWPWM_NAME );
    #elif defined(ARIMA_PROJECT_7909)
        hwPowerOn( MT65XX_POWER_LDO_VGP2 ,VOL_3300, TPD_HWPWM_NAME );
    #elif defined(ARIMA_PROJECT_HAWK35)
//<2012/09/06 Albert Wu, Update EKTF2048
	  #if defined (HAWK35_EP0) || defined (HAWK35_EP1)
//>2012/09/06 Albert Wu.
        hwPowerOn( MT65XX_POWER_LDO_VGP2 , VOL_1800, TPD_HWPWM_NAME );
        hwPowerOn( MT65XX_POWER_LDO_VCAMD, VOL_3300, TPD_HWPWM_NAME );
      #else
        hwPowerOn( MT65XX_POWER_LDO_VCAM_AF, VOL_2800, TPD_HWPWM_NAME );
      #endif
    #else
        hwPowerOn( MT65XX_POWER_LDO_VCAMD, VOL_3300, TPD_HWPWM_NAME );
    #endif
        tpd_hwpwm_enable = TRUE;
      }
    } break;

    case TPD_HWPWM_OFF:
    {
      if( FALSE != tpd_hwpwm_enable )
      {
    #if defined(ARIMA_PROJECT_ARGONMINI_6575) || defined(ARIMA_PROJECT_ARGONMINI_6577)
        hwPowerDown( MT65XX_POWER_LDO_VCAM_IO, TPD_HWPWM_NAME );
    #elif defined(ARIMA_PROJECT_7909)
        hwPowerDown( MT65XX_POWER_LDO_VGP2 , TPD_HWPWM_NAME );
    #elif defined(ARIMA_PROJECT_HAWK35)
//<2012/09/06 Albert Wu, Update EKTF2048
      #if defined (HAWK35_EP0) || defined (HAWK35_EP1)
//>2012/09/06 Albert Wu.	  
        hwPowerDown( MT65XX_POWER_LDO_VGP2 , TPD_HWPWM_NAME );
        hwPowerDown( MT65XX_POWER_LDO_VCAMD, TPD_HWPWM_NAME );
      #else
        hwPowerDown( MT65XX_POWER_LDO_VCAM_AF, TPD_HWPWM_NAME );
      #endif
    #else
        hwPowerDown( MT65XX_POWER_LDO_VCAMD, TPD_HWPWM_NAME );
    #endif
        tpd_hwpwm_enable = FALSE;
      }
    } break;

    default:
      break;
  }
  return  0;
}
//>2012/05/16 Yuting Shih.

int elan_iap_open(struct inode *inode, struct file *filp){ 
	printk("[ELAN]into elan_iap_open\n");
		if (private_ts == NULL)  printk("private_ts is NULL~~~");
		
	return 0;
}

int elan_iap_release(struct inode *inode, struct file *filp){    
	return 0;
}

ssize_t elan_iap_write(struct file *filp, const char *buff,    size_t count, loff_t *offp){  
    int ret;
    char *tmp;
    printk("[ELAN]into elan_iap_write\n");

    if (count > 8192)
        count = 8192;

    tmp = kmalloc(count, GFP_KERNEL);
    
    if (tmp == NULL)
        return -ENOMEM;

    if (copy_from_user(tmp, buff, count)) {
        return -EFAULT;
    }

    ret = i2c_master_send(i2c_client, tmp, count);
    if (ret != count) printk("ELAN i2c_master_send fail, ret=%d \n", ret);
    kfree(tmp);
    return ret;

}

ssize_t elan_iap_read(struct file *filp, char *buff,    size_t count, loff_t *offp){    
    char *tmp;
    int ret;  
    long rc;
    printk("[ELAN]into elan_iap_read\n");
   
    if (count > 8192)
        count = 8192;

    tmp = kmalloc(count, GFP_KERNEL);

    if (tmp == NULL)
        return -ENOMEM;

    ret = i2c_master_recv(i2c_client, tmp, count);

    if (ret >= 0)
        rc = copy_to_user(buff, tmp, count);
    
    kfree(tmp);
	
    return ret;
}

//<2012/4/18 Yuting Shih, Modified the IOCTL define base on Android 4.x
#if defined( VELOCITY_CUSTOM_EKTF2K )
static long elan_iap_unlocked_ioctl(struct file * pFile,unsigned int  cmd,unsigned long arg){
	long rc = 0;
#else
static int elan_iap_ioctl(struct inode *inode, struct file *filp,    unsigned int cmd, unsigned long arg){
	int rc;
#endif
//>2012/4/18 Yuting Shih.
        int __user *ip = (int __user *)arg;
	printk("==== [ELAN]into elan_iap_ioctl : %d\n",cmd);
	
	switch (cmd) {        
		case IOCTL_I2C_SLAVE:
			//private_ts->client->addr = (int __user *)arg;
			//i2c_client->addr = 0x15;
			break;   
		case IOCTL_MAJOR_FW_VER:            
			break;        
		case IOCTL_MINOR_FW_VER:            
			break;        
		case IOCTL_RESET:
			break;
		case IOCTL_IAP_MODE_LOCK:
			work_lock=1;
			break;
		case IOCTL_IAP_MODE_UNLOCK:
			work_lock=0;
                     pr_info(">>>>>>>>>>>  IAP IOCTL_IAP_MODE_UNLOCK\n");
                     #if 0
			if (gpio_get_value(private_ts->intr_gpio))
    			{
        			enable_irq(private_ts->client->irq);
			}
                     #endif
               //<2011/12/15-1472-kevincheng,Improve IAP Recovery Mode
                     #if !defined(EVB)
                        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
                        mdelay(500);
                        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
                     #else
                        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
                        mdelay(500);
                        mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
                     #endif
                        mdelay(500);
                        rc = __fw_packet_handler(i2c_client);
                        if(rc < 0)
                           pr_info(">>>>>>>  IAP re-init fail\n");
                //>2011/12/15-1472-kevincheng
			break;
		case IOCTL_CHECK_RECOVERY_MODE:
			return RECOVERY;
			break;
		case IOCTL_FW_VER:
			__fw_packet_handler(i2c_client);
			return fw_ver;
			break;
		case IOCTL_X_RESOLUTION:
			__fw_packet_handler(i2c_client);
			return ELAN_X_MAX;
			break;
		case IOCTL_Y_RESOLUTION:
			__fw_packet_handler(i2c_client);
			return ELAN_Y_MAX;
			break;
		case IOCTL_FW_ID:
			__fw_packet_handler(i2c_client);
			return fw_id;
			break;

		default:            
		//<2012/4/18 Yuting Shih,Add when based on Android 4.x
		#if defined( VELOCITY_CUSTOM_EKTF2K )
		  pr_info("tpd: unknown IOCTL: 0x%08x\n", cmd);
			rc = -ENOIOCTLCMD;
		#endif
		//<2012/4/18 Yuting Shih.
			break;   
	}       
	return 0;
}

struct file_operations elan_touch_fops = {    
        .open =         elan_iap_open,    
        .write =        elan_iap_write,    
        .read = elan_iap_read,    
        .release =     elan_iap_release,    
      //<2012/4/18 Yuting Shih,Modified when based on Android 4.x
      #if defined( VELOCITY_CUSTOM_EKTF2K )
        .unlocked_ioctl = elan_iap_unlocked_ioctl,
      #else
        .ioctl =         elan_iap_ioctl,  
      #endif /** End.. (VELOCITY_CUSTOM_EKTF2K) **/
      //<2012/4/18 Yuting Shih.
};

//<2012/4/25 Yuting Shih,Modified when based on Android 4.x
#if defined( VELOCITY_CUSTOM_EKTF2K )
static struct miscdevice  tpd_misc_device =
{
  .minor = MISC_DYNAMIC_MINOR,
  .name = "elan-iap",
  .fops = &elan_touch_fops,
  .mode = S_IRWXUGO,
};
#endif /** End.. (VELOCITY_CUSTOM_EKTF2K) **/
//>2012/4/25 Yuting Shih.

static ssize_t elan_ktf2k_gpio_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int ret = 0;
	struct elan_ktf2k_ts_data *ts = private_ts;

//<2012/05/11 Yuting Shih,Modified for the GPIO status getting.
	ret = mt_get_gpio_in( GPIO_CTP_EINT_PIN );  //gpio_get_value(GPIO_CTP_EINT_PIN);
//>2012/05/11 Yuting Shih.
	printk(KERN_DEBUG "GPIO_TP_INT_N=%d\n", GPIO_CTP_EINT_PIN);
	sprintf(buf, "GPIO_TP_INT_N=%d\n", ret);
	ret = strlen(buf) + 1;
	return ret;
}

static DEVICE_ATTR(gpio, S_IRUGO, elan_ktf2k_gpio_show, NULL);

static ssize_t elan_ktf2k_vendor_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;
	struct elan_ktf2k_ts_data *ts = private_ts;

	sprintf(buf, "%s_x%4.4x\n", "ELAN_KTF2K", fw_ver);
	ret = strlen(buf) + 1;
	return ret;
}

static DEVICE_ATTR(vendor, S_IRUGO, elan_ktf2k_vendor_show, NULL);

static struct kobject *android_touch_kobj;

static int elan_ktf2k_touch_sysfs_init(void)
{
	int ret ;

	android_touch_kobj = kobject_create_and_add("android_touch", NULL) ;
	if (android_touch_kobj == NULL) {
		printk(KERN_ERR "[elan]%s: subsystem_register failed\n", __func__);
		ret = -ENOMEM;
		return ret;
	}
	ret = sysfs_create_file(android_touch_kobj, &dev_attr_gpio.attr);
	if (ret) {
		printk(KERN_ERR "[elan]%s: sysfs_create_file failed\n", __func__);
		return ret;
	}
	ret = sysfs_create_file(android_touch_kobj, &dev_attr_vendor.attr);
	if (ret) {
		printk(KERN_ERR "[elan]%s: sysfs_create_group failed\n", __func__);
		return ret;
	}
	return 0 ;
}

static void elan_touch_sysfs_deinit(void)
{
	sysfs_remove_file(android_touch_kobj, &dev_attr_vendor.attr);
	sysfs_remove_file(android_touch_kobj, &dev_attr_gpio.attr);
	kobject_del(android_touch_kobj);
}

//IAP
static void tpd_haptics(void)
{
//<2011/12/01-992-kevincheng, Remove haptics function in driver part
#if 0
   hwPowerOn( MT65XX_POWER_LDO_VIBR,VOL_2800, TPD_HWPWM_NAME );
   msleep(20);
   hwPowerDown( MT65XX_POWER_LDO_VIBR, TPD_HWPWM_NAME );
   VIBRATOR_FLAG = 1;
#endif
//>2011/12/01-992-kevincheng
}

static int tpd_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {
    strcpy(info->type, ELAN_TPD_DEVICE /*"mtk-tpd"*/ );
    return 0;
}

static int __elan_ktf2k_ts_poll(struct i2c_client *client)
{
        struct elan_ktf2k_ts_data *ts = i2c_get_clientdata(client);
        int status = 0, retry = 10;

        //pr_info(">>>>>>>>>>>>> _elan_ts_poll\n");
        /*reset elan IC and check INT pin state again*/
        do {
          //<2012/05/15 yaotsulin, add argonmini driver
                status = mt_get_gpio_in(GPIO_CTP_EINT_PIN); //gpio_get_value(GPIO_CTP_EINT_PIN);
          //<2012/05/15 yaotsulin.
                //pr_info(">>>>>>>>>>>>>>>>>>> %s: status = %d\n", __func__, status);
                retry--;
                mdelay(20);
        } while (status == 1 && retry > 0);

        /*pr_info("[elan]%s: poll interrupt status %s\n",
                        __func__, status == 1 ? "high" : "low");*/
        return (status == 0 ? 0 : -ETIMEDOUT);
}

static int elan_ktf2k_ts_poll(struct i2c_client *client)
{
        return __elan_ktf2k_ts_poll(client);
}

static int elan_ktf2k_ts_get_data(struct i2c_client *client, int *cmd,
			char *buf, int size)
{
	int rc;

	pr_info(">>>>>>>>>>>>>>>>>>>>>>> [elan]%s: enter\n", __func__);

	if (buf == NULL)
		return -EINVAL;

	if ((i2c_master_send(client, cmd, 4)) != 4) {
		pr_info("[elan]%s: i2c_master_send failed\n", __func__);
		return -EINVAL;
	}

	rc = elan_ktf2k_ts_poll(client);
	if (rc < 0)
		return -EINVAL;
	else {
		if (i2c_master_recv(client, buf, size) != size ||
		    buf[0] != CMD_S_PKT)
			return -EINVAL;
	}

	return 0;
}

static int __hello_packet_handler(struct i2c_client *client)
{
	int rc;
	char buf_recv[4];
          
        pr_info(">>>>>>>>>>>>> _hello_packet_handler\n");
	rc = elan_ktf2k_ts_poll(client);
	if (rc < 0) {
		pr_info("-- ektf poll check fail\n");
          return -EINVAL;
	}

	rc = i2c_master_recv(client, buf_recv, 4);
       pr_info("--  YO YO YA  _hello packet , %x %x %x %x , rc : %d\n",buf_recv[0],buf_recv[1],buf_recv[2],buf_recv[3],rc);

if(buf_recv[0]==0x55 && buf_recv[1]==0x55 && buf_recv[2]==0x80 && buf_recv[3]==0x80)
	{
		rc = elan_ktf2k_ts_poll(client);
		if (rc < 0) {
			dev_err(&client->dev, "[elan] %s: failed!\n", __func__);
			return -EINVAL;
		}
		rc = i2c_master_recv(client, buf_recv, 4);
		printk("[elan] %s: recovery hello packet %2x:%2X:%2x:%2x\n", __func__, buf_recv[0], buf_recv[1], buf_recv[2], buf_recv[3]);
		RECOVERY=0x80;
		return RECOVERY;
	}
//IAP
	return 0;
}

//<2012/01/09-2031-kevincheng,Power Consumption Improvement
static int get_power_state(struct i2c_client *client)
{
        char get_power_cmd[] = {CMD_R_PKT, 0x50, 0x00, 0x01};
        char buf[4];
        int rc,power_satae;

        pr_info("-- enter get power state\n");
        rc = elan_ktf2k_ts_get_data(client,get_power_cmd,buf,4);
        if(rc < 0)
           {
            pr_info("-- get power state fail\n");
            return rc;
           }

        pr_info("-- get power state feedback : %x %x %x %x\n",buf[0],buf[1],buf[2],buf[3]);
        pr_info("-- power state = %s\n",buf[1] == 0x50?"Deep sleep mode":"Normal/idle mode");
        
        if(buf[1] == 0x50)
           return 0;
        return 1;
}

static int set_power_state(struct i2c_client *client,int state)
{
        char set_power_cmd[] = {CMD_W_PKT, 0x50, 0x00, 0x01};
        char buf[4];
        int rc;

        set_power_cmd[1] |= (state << 3); 
    
        if ((i2c_master_send(client, set_power_cmd, 4)) != 4)
           {
            pr_info("-- %s i2c master send fail\n",__func__);
            return -EINVAL;
           }
        
        return 0;        
}
//>2012/01/09-2031-kevincheng

static int __fw_packet_handler(struct i2c_client *client)
{
//<2011/12/15-1470-kevincheng, Add TP Boot Code Data
	//struct elan_ktf2k_ts_data *ts = i2c_get_clientdata(client);
        int rc;
        int major, minor;
        char fw_cmd[] = {CMD_R_PKT, 0x00, 0x00, 0x01};
        char bc_fw_cmd[] = {CMD_R_PKT, 0x01, 0x00, 0x01};
        char id_cmd[] = {CMD_R_PKT, 0xF0, 0x00, 0x01};
        char x_max_cmd[] = {CMD_R_PKT, 0x60, 0x00, 0x00};
        char y_max_cmd[] = {CMD_R_PKT, 0x63, 0x00, 0x00};
        char buf_recv[4] = {0};

        pr_info(">>>>>>>>>>>>>> _fw_packet_handler\n");
// Firmware Version
        rc = elan_ktf2k_ts_get_data(client, fw_cmd, buf_recv, 4);
        pr_info(">>>>>>>>>>>>>>>>> --- fw_cmd , rc : %d\n",rc);
        if (rc < 0)
                return rc;

        major = ((buf_recv[1] & 0x0f) << 4) | ((buf_recv[2] & 0xf0) >> 4);
        minor = ((buf_recv[2] & 0x0f) << 4) | ((buf_recv[3] & 0xf0) >> 4);
        fw_ver = major << 8 | minor;
        pr_info(">>>>>>>>>>>>>>>>>>> [elan] %s: firmware version: 0x%4.4x\n",
                        __func__, fw_ver);
// BootCode Version
        rc = elan_ktf2k_ts_get_data(client, bc_fw_cmd, buf_recv, 4);
        pr_info(">>>>>>>>>>>>>>>>> --- bc_fw_cmd , rc : %d\n",rc);
        if (rc < 0)
                return rc;

        major = ((buf_recv[1] & 0x0f) << 4) | ((buf_recv[2] & 0xf0) >> 4);
        minor = ((buf_recv[2] & 0x0f) << 4) | ((buf_recv[3] & 0xf0) >> 4);
        bc_ver = major << 8 | minor;
        pr_info(">>>>>>>>>>>>>>>>>>> [elan] %s: Boot Code version: 0x%4.4x\n",
                        __func__, bc_ver);
// Firmware ID
        rc = elan_ktf2k_ts_get_data(client, id_cmd, buf_recv, 4);
        pr_info(">>>>>>>>>>>>>>>>>> id_cmd , rc : %d\n",rc);
        if (rc < 0)
                return rc;

        major = ((buf_recv[1] & 0x0f) << 4) | ((buf_recv[2] & 0xf0) >> 4);
        minor = ((buf_recv[2] & 0x0f) << 4) | ((buf_recv[3] & 0xf0) >> 4);
        fw_id = major << 8 | minor;
        pr_info(">>>>>>>>>>>>>>>>>>> [elan] %s: firmware ID: 0x%4.4x\n",
                        __func__, fw_id);
// X Resolution
        rc = elan_ktf2k_ts_get_data(client, x_max_cmd, buf_recv, 4);
        pr_info(">>>>>>>>>>>>>>>>>>> x_max , rc : %d\n",rc);
        if (rc < 0)
                return rc;

        major = (buf_recv[3] & 0xf0) >> 4;
        ELAN_X_MAX = major << 8 | buf_recv[2] ;
        pr_info(">>>>>>>>>>>>>>>>>> [elan] %s: X Resolution: %d\n",
                        __func__, ELAN_X_MAX);
// Y Resolution
        rc = elan_ktf2k_ts_get_data(client, y_max_cmd, buf_recv, 4);
        pr_info(">>>>>>>>>>>>>>>> y_max , rc : %d\n",rc);
        if (rc < 0)
                return rc;

        major = (buf_recv[3] & 0xf0) >> 4;
        ELAN_Y_MAX = major << 8 | buf_recv[2] ;
        pr_info(">>>>>>>>>>>>>>>> [elan] %s: Y resolution: %d\n",
                        __func__, ELAN_Y_MAX);
        return 0;
//>2011/12/15-1470-kevincheng
}

static int tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) 
{             
    int err = 0;
    char buffer[2];
    int buf_recv[4];
    int status=0;

    pr_info(">>>>>>>>>>>>>>>>>>>>>>>>  Elan tpd i2c probe\n");
  //<2012/05/15 yaotsulin, add argonmini driver
    client->timing = 400; //100;  /* SCL clock frequency. 100~400 KHz */
  //>2012/05/15 yaotsulin, add argonmini driver
    client->addr |= I2C_ENEXT_FLAG;
    client->irq = GPIO_CTP_EINT_PIN;
    i2c_client = client;    

//<2012/05/02 Yuting Shih,Modified for power supply.
    tpd_hwPower_ctrl( TPD_HWPWM_ON );
    //msleep( 50 );
//>2012/05/02 Yuting Shih.

//<2012/09/06 Albert Wu, Update EKTF2048
#if defined (HAWK35_EP0)

#else
    mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);
#endif    
//>2012/09/06 Albert Wu.
    mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);
    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
#if !defined(EVB)
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
#endif
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE); 
    mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);

    msleep(200);

	if (!i2c_check_functionality(i2c_client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) 
	{
		 pr_info("elan No supported i2c func what we need?!!\n");
	}

    status =  __hello_packet_handler(i2c_client);    
    if(status<0)
    	{
    	  pr_info("----- EKTF probe hello pactet fail\n");
	  return status;
    	}
	else if(status==0x80)	/*Enter elan IC recovery mode */
	{
          pr_info(">>> TP Recovery Mode\n");;
	}
        else
        {
          status = __fw_packet_handler(i2c_client);        
          if(status<0)
            {
             pr_info("----- EKTF probe fw check fail\n");
             return status;
            }
        }

        pr_info(">>>>>>>>>>>>>>>>>>> [mtk-tpd], EKTF tpd_i2c_probe success!!\n");		
	tpd_load_status = 1;

    thread = kthread_run(touch_event_handler, 0, ELAN_TPD_DEVICE /*TPD_DEVICE*/ );
    if (IS_ERR(thread)) { 
        err = PTR_ERR(thread);
        pr_info(TPD_DEVICE " failed to create kernel thread: %d\n", err);
    }

    set_bit(KEY_BACK, tpd->dev->keybit);
    set_bit(KEY_MENU, tpd->dev->keybit);
    set_bit(KEY_HOME, tpd->dev->keybit);
    set_bit(KEY_SEARCH, tpd->dev->keybit);
 
    mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
    mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);
    mt65xx_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN, CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 1);
    mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
    
//<2012/7/16 Yuting Shih, Modified when based on Android 4.x
#if defined( VELOCITY_CUSTOM_EKTF2K )
    //if( misc_register( &tpd_misc_device ) < 0 )
#else
    //i2c_client->firmware.minor = MISC_DYNAMIC_MINOR;
//<2012/04/20-5785-kevincheng,Add IAP MP Key defune
//<2012/05/02-5860-kevincheng,Add ATCMD For Efuse
//<2012/06/13-8830-kevincheng,Modify Elan-iap From root to System
  #if 0 //!defined(KEY_TYPE_MP) || defined(ARIMA_PCBA_RELEASE) || defined(ARIMA_PROJECT_SILVERSMART)
    i2c_client->firmware.name = "elan-iap";
  #endif
//>2012/06/13-8830-kevincheng
//>2012/05/02-5860-kevincheng
//>2012/04/20-5785-kevincheng
  //i2c_client->firmware.fops = &elan_touch_fops;
  //i2c_client->firmware.mode = S_IRWXUGO;

  //if (misc_register(&i2c_client->firmware) < 0)
#endif /** End.. (VELOCITY_CUSTOM_EKTF2K) **/
//>2012/7/16 Yuting Shih.
  //    printk("[ELAN]misc_register failed!!");
  //else
  //{
  //    printk("[ELAN]misc_register finished!!");
//<2012/06/21-9332-kevincheng,Add TP Vendor Info File
  //    create_proc_read_entry("TP", S_IRUGO, NULL, TP_vendor_proc, NULL);
//>2012/06/21-9332-kevincheng
  //}
    return 0;
}

void tpd_eint_interrupt_handler(void) 
{ 
    TPD_DEBUG_PRINT_INT; 
    tpd_flag=1; 
    wake_up_interruptible(&waiter);
} 
static int tpd_i2c_remove(struct i2c_client *client) {return 0;}


static inline int elan_ktf2k_ts_parse_xy(uint8_t *data,
			int *x, int *y)
{
	*x = *y = 0;

	*x = (data[0] & 0xf0);
	*x <<= 4;
	*x |= data[1];

	*y = (data[0] & 0x0f);
	*y <<= 8;
	*y |= data[2];
        
	*x = *x * (FRAME_X-1)/ELAN_X_MAX;
	*y = *y * FRAME_Y/ELAN_Y_MAX;

	if(*x == 0)
		*x += 1;
	pr_info(">>>>>>>>>>>> ELAN x : %d , y : %d\n",*x,*y);
	return 0;
}

static void elan_ktf2k_ts_report_data(struct i2c_client *client, char *buf)
{
//<2012/05/17 Yuting Shih,Modified for coding style.
#if 0
	int x, y;
	int fbits=0, finger_num=0;
	int i, num;
	int rc, bytes_to_recv = PACKET_SIZE;
          
        pr_info(">>>>>>>>>>>>>>>>>>>>>>>>>>>>  elan report data\n");     
//<2011/12/08-1252-kevincheng,Fix TP key function abnirmal issue
#if defined(EVB) 
        num = buf[FINGER_ID_INDEX]& 0x06; 
	if (num == 0x02)
		fbits= 0x01;
	else if (num == 0x4)
		fbits= 0x03;
#else
        num = buf[FINGER_ID_INDEX]& 0x03;
        if (num == 0x01)
                fbits= 0x01;
        else if ((num == 0x2)||(num == 0x03))
                fbits= 0x03;
#endif
//>2011/12/08-1252-kevincheng
	else 
		fbits= 0;

   switch (buf[0]) {
	   case NORMAL_PKT:

		if (num == 0) {
      if (buf[KEY_ID_INDEX] & EKTF2048_KEY_SEARCH) {
	   button = EKTF2048_KEY_SEARCH;
           input_report_key(tpd->dev, KEY_SEARCH, 1);
      } else if (buf[KEY_ID_INDEX] & EKTF2048_KEY_BACK) {
           button = EKTF2048_KEY_BACK;
	   input_report_key(tpd->dev, KEY_BACK, 1);
      } else if (buf[KEY_ID_INDEX] & EKTF2048_KEY_HOME) {
           button = EKTF2048_KEY_HOME;
           input_report_key(tpd->dev, KEY_HOME, 1);
      } else if (buf[KEY_ID_INDEX] & EKTF2048_KEY_MENU) {
           button = EKTF2048_KEY_MENU;
           input_report_key(tpd->dev, KEY_MENU, 1);
      }else if (button == EKTF2048_KEY_SEARCH) {
           button = EKTF2048_KEY_RELEASE;
	   input_report_key(tpd->dev, KEY_SEARCH, 0);
      } else if (button == EKTF2048_KEY_BACK) {
           button = EKTF2048_KEY_RELEASE;
	   input_report_key(tpd->dev, KEY_BACK, 0);
      }else if (button == EKTF2048_KEY_HOME) {
           button = EKTF2048_KEY_RELEASE;
	   input_report_key(tpd->dev, KEY_HOME, 0);
      } else if (button == EKTF2048_KEY_MENU) {
           button = EKTF2048_KEY_RELEASE;
	   input_report_key(tpd->dev, KEY_MENU, 0);
      }


			//<2012/05/15 yaotsulin, add argonmini driver
			//input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);//yaotsulin
			//input_report_abs(tpd->dev, ABS_MT_WIDTH_MAJOR, 0);//yaotsulin
			//>2012/05/15 yaotsulin, add argonmini driver

			input_mt_sync(tpd->dev);
			rc = i2c_master_recv(client, buf, bytes_to_recv);

		} else {
			uint8_t idx;
			uint8_t reported = 0;

                        idx=IDX_FINGER;

			for (i = 0; i < FINGER_NUM; i++) {
			  if ((fbits & FINGER_ID)) {						
			     elan_ktf2k_ts_parse_xy(&buf[idx], &x, &y);

			     if (!((x<=0) || (y<=0) || (x>=ELAN_X_MAX) || (y>=ELAN_Y_MAX))) {   
                                finger_num = finger_num + 1;			     	
    				input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, i);
				input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 8);
				input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);
				input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);
				input_mt_sync(tpd->dev);
				reported++;
                             pr_info("=== data report\n");
			     } // end if border
 			  } // end if finger status

			  fbits = fbits >> 1;
			  idx += 3;
			} // end for

			if (reported)
			{
				input_report_key(tpd->dev, BTN_TOUCH, finger_num);
				input_sync(tpd->dev);
			}
			else {
				input_mt_sync(tpd->dev);
				input_sync(tpd->dev);
			}

		}

		input_sync(tpd->dev);

		break;
	   default:
		dev_err(&client->dev,
			"[elan] %s: unknown packet type: %0x\n", __func__, buf[0]);
		rc = i2c_master_recv(client, buf, bytes_to_recv);
		break;
	   } // end switch

/*****************************************/
#else // coding style.

int   x = 0, y = 0;
int   num = 0, fbits = 0;
int   i = 0;
int   rc = 0, bytes_to_recv = PACKET_SIZE;

  printk(">>>>>>>>>>>>>>>>>>>>>>>>>>>>  elan report data\n");     
  num = *( buf + FINGER_ID_INDEX );
#if defined( EVB )
  num = num >> 1;
#endif
  num &= 0x0003;
  if( num == 0x0001 )
    fbits = 0x01;
  else if (( num == 0x0002 ) || ( num == 0x0003 ))
    fbits = 0x03;
	else
		fbits = 0x00;

  switch( *buf )
  {
    case NORMAL_PKT:
    {
      if( num == 0x0000 )
      {
      char  key_id = *( buf + KEY_ID_INDEX );

        key_id &= EKTF2048_KEY_EVENT;
        if( key_id & EKTF2048_KEY_SEARCH )
        {
          button = EKTF2048_KEY_SEARCH;
          input_report_key( tpd->dev, KEY_SEARCH, 1 );
        }
        else if( key_id & EKTF2048_KEY_BACK )
        {
          button = EKTF2048_KEY_BACK;
          input_report_key( tpd->dev, KEY_BACK, 1 );
        }
        else if( key_id & EKTF2048_KEY_HOME )
        {
          button = EKTF2048_KEY_HOME;
          input_report_key( tpd->dev, KEY_HOME, 1 );
        }
        else if( key_id & EKTF2048_KEY_MENU )
        {
          button = EKTF2048_KEY_MENU;
          input_report_key( tpd->dev, KEY_MENU, 1 );
        }
        else if( button == EKTF2048_KEY_SEARCH )
        {
          button = EKTF2048_KEY_RELEASE;
          input_report_key( tpd->dev, KEY_SEARCH, 0 );
        }
        else if( button == EKTF2048_KEY_BACK )
        {
          button = EKTF2048_KEY_RELEASE;
          input_report_key( tpd->dev, KEY_BACK, 0 );
        }
        else if( button == EKTF2048_KEY_HOME )
        {
          button = EKTF2048_KEY_RELEASE;
          input_report_key( tpd->dev, KEY_HOME, 0 );
        }
        else if( button == EKTF2048_KEY_MENU )
        {
          button = EKTF2048_KEY_RELEASE;
          input_report_key( tpd->dev, KEY_MENU, 0 );
        }

        if( key_id & EKTF2048_KEY_EVENT )
        {
          if( !VIBRATOR_FLAG )
            tpd_haptics();
        }
        else
          VIBRATOR_FLAG = 0;

    //<2012/05/17 Yuting Shih,Masked when based on Andorid 4.x.
      #if 0
        input_report_abs( tpd->dev, ABS_MT_TOUCH_MAJOR, 0 );
        input_report_abs( tpd->dev, ABS_MT_WIDTH_MAJOR, 0 );
      #endif
    //>2012/05/17 Yuting Shih.
        input_mt_sync( tpd->dev );
        rc = i2c_master_recv( client, buf, bytes_to_recv ); /* Response? */
      }
      else
      {
      uint8_t   idx = IDX_FINGER;
      uint8_t   report_abs = 0;

        for( i = 0; i < FINGER_NUM; i++ )
        {
          if(( fbits & FINGER_ID ) == FINGER_ID )
          {
            elan_ktf2k_ts_parse_xy((uint8_t*)( buf + idx ), (int*)&x, (int*)&y );
            if( !(( x <= 0 ) || ( y <= 0 ) || ( x >= FRAME_X ) || ( y >= FRAME_Y )))
            {
              report_abs += 1;

              input_report_abs( tpd->dev, ABS_MT_TRACKING_ID, report_abs /*i*/ );
              input_report_abs( tpd->dev, ABS_MT_TOUCH_MAJOR, 8 );  /* Finger area 8x8 mm */
              input_report_abs( tpd->dev, ABS_MT_POSITION_X, x );
              input_report_abs( tpd->dev, ABS_MT_POSITION_Y, y );
              input_mt_sync( tpd->dev );

              TPD_EM_PRINT( x, y, x, y, report_abs, 1 );
            } /* End.. if(!(x...)) */
          } /* End.. if(FINGER_ID) */
          fbits = fbits >> 1;
          idx += 3;
        }/* End.. for() */

        input_report_key( tpd->dev, BTN_TOUCH, report_abs );
        if( !report_abs )
        {
      //<2012/05/17 Yuting Shih,Masked when based on Andorid 4.x.
        #if 0
          input_report_abs( tpd->dev, ABS_MT_TOUCH_MAJOR, 0 );
        #endif
      //>2012/05/17 Yuting Shih.
          input_mt_sync( tpd->dev );
          TPD_EM_PRINT( x, y, x, y, 0, 0 );
        }
        //input_sync( tpd->dev );
      }
      input_sync( tpd->dev );
    } break;

    default:
    {
      dev_err(&client->dev, "[elan] %s: unknown packet type: %0x\n", __func__, buf[0]);
      rc = i2c_master_recv( client, buf, bytes_to_recv ); /* Response? */
    } break;
  } /** End.. switch() **/
#endif
//>2012/05/17 Yuting Shih.coding style.

	return;
}

static int elan_ktf2k_ts_recv_data(struct i2c_client *client, char *buf)
{
	int rc, bytes_to_recv = PACKET_SIZE;

	if (buf == NULL)
		return -EINVAL;

	memset(buf, 0, bytes_to_recv);
	rc = i2c_master_recv(client, buf, bytes_to_recv);
   #if 1
        pr_info(">>>>>>>>>>>>>>>>>>>  Ektf recv data , %x %x %x %x %x %x %x %x\n",\
	   	buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
   #endif	   
	if (rc != bytes_to_recv) {
		dev_err(&client->dev,
			"[elan] %s: i2c_master_recv error?! \n", __func__);

		return -EINVAL;
	}

	return rc;
}
#if defined(CTP_CHECK_FM)
void tpd_get_fm_frequency(int16_t freq) {
		fm_current_frequency = freq;
}
EXPORT_SYMBOL(tpd_get_fm_frequency);
#endif
extern volatile unsigned long g_temptimerdiff;

//< 2012/05/23 Yuting Shih, Masked if non-usage
#if 0
int x_min=0, y_min=0, x_max=0, y_max=0, counter_pointer=0;
#endif
//> 2012/05/23 Yuting Shih.
static int touch_event_handler(void *unused) 
{
    struct sched_param param = { .sched_priority = RTPM_PRIO_TPD }; 
//< 2012/05/23 Yuting Shih, Masked if non-usage
#if 0
    int x1=0, y1=0, x2=0, y2=0, x3=0, y3=0, x4=0, y4=0, p1=0, p2=0, p3=0, p4=0, id1=0xf, id2=0xf, id3=0xf, id4 = 0xf, pre_id1 = 0xf, pre_id2 = 0xf, pre_id3 = 0xf, pre_id4 = 0xf, pre_tt_mode = 0, finger_num = 0, pre_num = 0;
    int raw_x1=0, raw_y1=0, raw_x2=0, raw_y2=0, raw_x3=0, raw_y3=0, raw_x4=0, raw_y4=0;
    static char toggle;
    static char buffer[32];//[16];
#endif
//>2012/05/23 Yuting Shih.
    char buf[PACKET_SIZE];
    
    sched_setscheduler(current, SCHED_RR, &param);
    do{
	   set_current_state(TASK_INTERRUPTIBLE);   
	   wait_event_interruptible(waiter, tpd_flag != 0);
	   
	    tpd_flag = 0; 
	 //<2012/04/30 Yuting Shih, Add when based on Android 4.x ICS
	   TPD_DEBUG_SET_TIME;
	 //>2012/04/30 Yuting Shih.
		
	   set_current_state(TASK_RUNNING);
	   
          elan_ktf2k_ts_recv_data(i2c_client, buf);
          elan_ktf2k_ts_report_data(i2c_client, buf); 
    } while (!kthread_should_stop());

    return 0;
}

int tpd_local_init(void) 
{
  pr_info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Elan local init\n");

     if(i2c_add_driver(&tpd_i2c_driver)!=0) {
      pr_info(">>>>>>>>>>>>>>>>>>>>>>>>>>> unable to add Elan i2c driver.\n");
      return -1;
    }

#ifdef TPD_HAVE_BUTTON     
    tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data
#endif   
  
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    
    TPD_DO_WARP = 1;
    memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);
    memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);
#endif 

#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))
    memcpy(tpd_calmat, tpd_calmat_local, 8*4);
    memcpy(tpd_def_calmat, tpd_def_calmat_local, 8*4);	
#endif
		TPD_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  
		tpd_type_cap = 1;
    return 0;
}

//<2012/01/09-2031-kevincheng, Power Consumption Improvement
void tpd_suspend(struct early_suspend *h)
{
    //<2012/1/6-2308-DavidDA,  fix issue: TP can not sleep	
    char sleep[4] = {0x54,0x50,0x00,0x01};
    //>2012/1/6-2308-DavidDA
    pr_info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Elan suspend\n");

    mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);
   //<2012/1/6-2308-DavidDA,  fix issue: TP can not sleep	
   #if defined(ARGON_Q_P_0_0)
	i2c_master_send(i2c_client, sleep, 4) ;
   #else
    set_power_state(i2c_client,PWR_STATE_DEEP_SLEEP);
    get_power_state(i2c_client);
   #endif
   //>2012/1/6-2308-DavidDA
}

void tpd_resume(struct early_suspend *h)
{
    pr_info(">>>>>>>>>>>>>>>>>>>>>>>>>>> Elan resume\n");
    //<2012/1/6-2308-DavidDA,  fix issue: TP can not sleep	
#if defined(ARGON_Q_P_0_0)
      char sleep[4] = {0x54,0x58,0x00,0x01};
	i2c_master_send(i2c_client, sleep, 4) ;
       mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);
       msleep(5);
       mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);
       msleep(5);
#else
    set_power_state(i2c_client,PWR_STATE_NORMAL);
#endif
    //>2012/1/6-2308-DavidDA
    //<2012/2/6-3022-kevincheng,Reduce TP Resume time
    msleep(250);
    //>2012/2/6-3022-kevincheng
    get_power_state(i2c_client);
    mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
}
//>2012/01/09-2031-kevincheng

static struct tpd_driver_t tpd_device_driver = {
		.tpd_device_name = "elan-ktf2k",
		.tpd_local_init = tpd_local_init,
		.suspend = tpd_suspend,
		.resume = tpd_resume,
#ifdef TPD_HAVE_BUTTON
		.tpd_have_button = 1,
#else
		.tpd_have_button = 0,
#endif		
};
/* called when loaded into kernel */
static int __init tpd_driver_init(void) {
    pr_info(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> MediaTek elan touch panel driver init\n");
//<2012/4/25 Yuting Shih, Add when based on Android 4.x
		i2c_register_board_info( ELAN_I2C_CHANNEL_ID, &ektf2048_i2c_tpd, 1 );
//>2012/4/25 Yuting Shih.
		if(tpd_driver_add(&tpd_device_driver) < 0)
			TPD_DMESG("add generic driver failed\n");
    return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void) {
    pr_info(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> MediaTek elan touch panel driver exit\n");
    //input_unregister_device(tpd->dev);
    tpd_driver_remove(&tpd_device_driver);
}

module_init(tpd_driver_init);
module_exit(tpd_driver_exit);

