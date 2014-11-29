/********************************************************************************************
** mxt224s_driver.h
**  - ATMEL maxTouch 224-channel Touchsreen Sensor driver
**===========================================================================================
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
**===========================================================================================
** History
**-------------------------------------------------------------------------------------------
** 2012/8/9   Yuting shih
**  Creation.
**===========================================================================================
** Copyright (C) 2012 Arimacomm Corporation.
*********************************************************************************************/
#ifndef _MXT224S_DRIVER_H_
#define _MXT224S_DRIVER_H_
/********************************************************************************************
** Include files
*********************************************************************************************/
#if defined( _MXT224S_DRIVER_C_ )
  #if defined( VELOCITY_MXT224S )
    #undef  VELOCITY_MXT224S
  #endif
    #include  <linux/init.h>
    #include  <linux/module.h>
    #include  <linux/i2c.h>
    #include  <linux/input.h>
    #include  <linux/interrupt.h>
    #include  <linux/slab.h>
    #include  <linux/gpio.h>
    #include  <linux/sched.h>
    #include  <linux/kthread.h>
    #include  <linux/bitops.h>
    #include  <linux/kernel.h>
    #include  <linux/delay.h>
    #include  <linux/byteorder/generic.h>
  #if defined(CONFIG_HAS_EARLYSUSPEND)
    #include  <linux/earlysuspend.h>
  #endif
  
    #include  <linux/time.h>
    #include  <linux/rtpm_prio.h>

    #include  "tpd.h"
    #include  <cust_eint.h>
  #if defined( MT6573 )
    #include  <mach/mt6573_boot.h>
 #else
    #include  <mach/mt_pm_ldo.h>
    #include  <mach/mt_typedefs.h>
    #include  <mach/mt_boot.h>
  #endif

  #if !defined( TPD_NO_GPIO )
    #include  "cust_gpio_usage.h"
  #endif

    #define   VELOCITY_MXT224S
  #if defined( VELOCITY_MXT224S )
    #include  <linux/device.h>
    #include  <linux/miscdevice.h>
    #include  <asm/uaccess.h>
  #else
    #include  <linux/uaccess.h>
  #endif

    #include  <linux/jiffies.h>
    #include  <linux/atmel_mxt224s_ts.h>
#endif /** End.. if(_MXT224S_DRIVER_C_) **/

/********************************************************************************************
** Macro-defined
*********************************************************************************************/
#if defined( GLOBAL )
    #undef    GLOBAL
#endif
 
#if defined( _MXT224S_DRIVER_C_ )
    #define   TPD_HWPWM_NAME              "MXT224S"
 
    /* SCL clock frequency. 100~400 KHz */
    #define   I2C_MASTER_CLOCK            380 //400   

    /** Version **/
    #define   MXT_VER_20                  20  /* 0x14 */
    #define   MXT_VER_21                  21
    #define   MXT_VER_22                  22

    /** Slave addresses **/
    #define   MXT_APP_LOW                 0x4A
    #define   MXT_APP_HIGH                0x4B
    #define   MXT_BOOT_LOW                0x24
    #define   MXT_BOOT_HIGH               0x25
//<2012/09/10 Albert Wu,add atmel mxt224 driver
//#define   MXT_I2C_SLAVE_ADDR          0x4A
#define   MXT_I2C_SLAVE_ADDR          0x94
//#define   MXT_I2C_SLAVE_ADDR          0x96
//<2012/09/10 Albert Wu.
    
    /* Registers */
    #define   MXT_FAMILY_ID                 0x00
    #define   MXT_VARIANT_ID               0x01
    #define   MXT_VERSION                     0x02
    #define   MXT_BUILD                         0x03
    #define   MXT_MATRIX_X_SIZE         0x04
    #define   MXT_MATRIX_Y_SIZE         0x05
    #define   MXT_OBJECT_NUM              0x06
    #define   MXT_OBJECT_START           0x07

    /* Orient */
    #define   MXT_NORMAL                  		0x00
    #define   MXT_DIAGONAL                		0x01
    #define   MXT_HORIZONTAL_FLIP        	0x02
    #define   MXT_ROTATED_90_COUNTER      0x03
    #define   MXT_VERTICAL_FLIP          		0x04
    #define   MXT_ROTATED_90             		0x05
    #define   MXT_ROTATED_180             		0x06
    #define   MXT_DIAGONAL_COUNTER          0x07
 
    #define   MXT_KEY_RELEASE                      0x00
    #define   MXT_KEY_SEARCH              	 	0x00 //0x01
    #define   MXT_KEY_BACK                		0x01 //0x02
    #define   MXT_KEY_HOME                           0x02 //0x04 
    #define   MXT_KEY_MENU                          0x04 //0x08

    #define   MXT_KEY_EVENT               (MXT_KEY_SEARCH|MXT_KEY_BACK|MXT_KEY_HOME|MXT_KEY_MENU)
 
    #define   ATMEL_MXT_TPD_NAME          "atmel_mxt224s"
  
    #define   ATMEL_TOUCHSCREEN_COMPATIBLE_REPORT
    #define   ATMEL_EN_SYSFS
  //#define   ATMEL_TDP_CABLE_CTRL

  #if defined( ARIMA_PROJECT_HAWK40 )
    #define   ATMEL_I2C_CHANNEL_ID        0
  #else
    #define   ATMEL_I2C_CHANNEL_ID        0
  #endif
    #define   ATMEL_I2C_RETRY_TIMES       10
    #define   ATMEL_GPIO_IRQ              GPIO_CTP_EINT_PIN
    #define   ATMEL_GPIO_IRQ_M            GPIO_CTP_EINT_PIN_M_EINT

  #if !defined( GPIO_VPS_EN_PIN )
    #define   GPIO_VPS_EN_PIN             GPIO48
  #endif
  #if !defined( GPIO_VPS_EN_PIN_M_GPIO )
    #define   GPIO_VPS_EN_PIN_M_GPIO      GPIO_MODE_00
  #endif

  /* config_setting */
    #define   NONE                             0
    #define   CONNECTED                   1

  /* anti-touch calibration */
    #define   RECALIB_NEED              0
    #define   RECALIB_NG                  1
    #define   RECALIB_DONE              2

  /* phone call status */
    #define   PHONE_NONE                  0
    #define   PHONE_END_CALL          1
    #define   PHONE_IN_CALL             2

    #define   GLOBAL
#else
    #define   GLOBAL                    extern
#endif /** End.. (_MXT224S_DRIVER_C_) **/

  typedef enum  tpd_hwPower_enable_ctrl
  {
    TPD_HWPWM_OFF = 0
   ,TPD_HWPWM_ON
  };

  typedef struct atmel_ts_data
  {
    struct i2c_client * client;
  #if defined( CONFIG_HAS_EARLYSUSPEND )
    struct early_suspend  early_suspend;
  #endif
    struct info_id_t  * id;
    struct object_t   * object_table;
    uint8_t   crc_check_sum[3];	
#if 0
    uint8_t   finger_count;
    uint16_t  abs_x_min;
    uint16_t  abs_x_max;
    uint16_t  abs_y_min;
    uint16_t  abs_y_max;
    uint8_t   abs_pressure_min;
    uint8_t   abs_pressure_max;
    uint8_t   abs_width_min;
    uint8_t   abs_width_max;
    uint8_t   first_pressed;
#endif		
    uint8_t   debug_log_level;

    struct atmel_finger_data  finger_data[10];
    uint8_t   finger_type;
#if 0	
    uint8_t   finger_support;
    uint16_t  finger_pressed;
    uint8_t   face_suppression;
    uint8_t   grip_suppression;
    uint8_t   noise_status[2];
    uint16_t *filter_level;
    uint8_t   calibration_confirm;
    uint64_t  timestamp;
    struct atmel_config_data  config_setting[2];
  //<2011/06/08-joshualee, update data to Firmware ver2.0
    int8_t    noise_config[3];
    uint8_t   cal_tchthr[2];
    uint8_t   noisethr_config;
    uint8_t   diag_command;
    uint8_t   psensor_status;
    uint8_t * ATCH_EXT;
    int       pre_data[11];
  //>2011/06/08-joshualee
    uint8_t   status;
    uint8_t   GCAF_sample;
    uint8_t * GCAF_level;
    uint8_t   noisethr;
#endif	
  #if defined( ATMEL_EN_SYSFS )
    struct device   dev;
  #endif
  //<2011/05/24-joshualee, for multiple touchpanel in STE
  //struct regulator  * regulator;
  //>2011/05/24-joshualee
  } ATMEL_TS;
  
  typedef struct atmel_i2c_platform_data    ATMEL_PLAT;

/**********************************************************************************
** Function Declaration
***********************************************************************************/
#if defined( _MXT224S_DRIVER_C_ )
    static int  tpd_hwPower_ctrl(int ctrl_OnOff);
    int i2c_atmel_read(struct i2c_client *client, uint16_t address,uint8_t *data,uint8_t length);
    int i2c_atmel_write(struct i2c_client *client, uint16_t address,uint8_t *data,uint8_t length);
    int i2c_atmel_write_byte_data(struct i2c_client *client,uint16_t address,uint8_t value);
    uint16_t  get_object_address(ATMEL_TS *ts,uint8_t object_type);
    uint8_t   get_object_size(ATMEL_TS *ts,uint8_t object_type);
    uint8_t   get_report_ids_size(ATMEL_TS *ts,uint8_t object_type);
    static int atmel_ts_poll(struct i2c_client *client);
  #if defined( ATMEL_EN_SYSFS )
    static ssize_t atmel_gpio_show(struct device *dev,struct device_attribute *attr,char *buf);
    static ssize_t atmel_vendor_show(struct device *dev,struct device_attribute *attr,char *buf);
    static ssize_t atmel_register_show(struct device *dev,struct device_attribute *attr,char *buf);
    static ssize_t atmel_register_store(struct device *dev,struct device_attribute *attr,const char *buf,size_t count);
    static ssize_t atmel_regdump_show(struct device *dev,struct device_attribute *attr,char *buf);
    static ssize_t atmel_regdump_dump(struct device *dev,struct device_attribute *attr,const char *buf,size_t count);
    static ssize_t atmel_debug_level_show(struct device *dev,struct device_attribute *attr,char *buf);
    static ssize_t atmel_debug_level_dump(struct device *dev,struct device_attribute *attr,const char *buf,size_t count);
    static int  atmel_touch_sysfs_init(void);
    static void atmel_touch_sysfs_deinit(void);
  #endif /* End.. (ATMEL_EN_SYSFS) */
    static int  check_delta(ATMEL_TS *ts);
    static void check_calibration(ATMEL_TS *ts);
    static void confirm_calibration(ATMEL_TS *ts);
    static void compatible_input_report(struct input_dev *idev,struct atmel_finger_data *fdata,uint8_t press,uint8_t last);
    static void msg_process_finger_data(struct atmel_finger_data *fdata,uint8_t *data);
  #if !defined( ATMEL_TOUCHSCREEN_COMPATIBLE_REPORT )
    static void htc_input_report(struct input_dev *idev,struct atmel_finger_data *fdata,uint8_t press,uint8_t last);
  #endif /* End.. !(ATMEL_TOUCHSCREEN_COMPATIBLE_REPORT) */
    static void multi_input_report(ATMEL_TS *ts);
    static void msg_process_multitouch(ATMEL_TS *ts,uint8_t *data,uint8_t idx);
    static void msg_process_multitouch_legacy(ATMEL_TS *ts,uint8_t *data,uint8_t idx);
    static void msg_process_noisesuppression(ATMEL_TS *ts,uint8_t *data);
    static void atmel_ts_work_report(struct i2c_client *client);
    static int  atmel_ts_probe(struct i2c_client *client,const struct i2c_device_id *id);
    static int  atmel_ts_remove(struct i2c_client *client);
    static int  atmel_ts_suspend(struct i2c_client *client, pm_message_t mesg);
    static int  atmel_ts_resume(struct i2c_client *client);
    static int  atmel_ts_detect(struct i2c_client *client,int kind,struct i2c_board_info *pInfo);
  #if defined( ATMEL_TDP_CABLE_CTRL )
    static void cable_tp_status_handler_func(int connected);
  #endif
    
    static int atmel_ts_suspend_function(struct i2c_client *client);
    static int atmel_ts_resume_function(struct i2c_client *client);
  
  /*<<EricHsieh,using the early suspend/resume for touch panel*/
   #if defined( CONFIG_HAS_EARLYSUSPEND )
    static void atmel_ts_early_suspend(struct early_suspend *h);
    static void atmel_ts_late_resume(struct early_suspend *h);
  #else
    static void tpd_suspend(struct early_suspend *h);
    static void tpd_resume(struct early_suspend *h);
  #endif  
  /*>>EricHsieh,using the early suspend/resume for touch panel*/
    static int  tpd_event_handler(void *unused);
    static void tpd_eint_interrupt_handler(void);
    static int  tpd_local_init(void);

 extern void mt_eint_unmask(unsigned int line);
extern void mt_eint_mask(unsigned int line);
extern void mt_eint_set_hw_debounce(unsigned int eintno, unsigned int ms);
extern unsigned int mt_eint_set_sens(unsigned int eintno, unsigned int sens);
extern void mt_eint_registration(unsigned int eint_num, unsigned int flag, 
              void (EINT_FUNC_PTR) (void), unsigned int is_auto_umask);	

#endif /** End.. (_MXT224S_DRIVER_C_) **/

/*******************************************************************************
** Variable Declaration
********************************************************************************/
#if defined( _MXT224S_DRIVER_C_ )
    ATMEL_TS  * private_ts = NULL;
    static struct kobject * android_touch_kobj = NULL;
  #if defined( ATMEL_TDP_CABLE_CTRL )
    static struct t_usb_status_notifier cable_status_handler = {
      .name = "usb_tp_connected",
      .func = cable_tp_status_handler_func,
    };
  #endif
    static struct i2c_board_info __initdata atmel_mxt_i2c_tpd = {
      I2C_BOARD_INFO( ATMEL_MXT_TPD_NAME, ( MXT_I2C_SLAVE_ADDR >> 1 ))
    };

    static const struct i2c_device_id atml_ts_i2c_id[] = {
      { ATMEL_MXT_TPD_NAME, 0 },
      { }
    };

    static struct i2c_driver atmel_i2c_ts_driver = {
      .id_table = atml_ts_i2c_id,
      .probe    = atmel_ts_probe,
      .remove   = atmel_ts_remove,
      .detect   = atmel_ts_detect,
      .driver = {
        .name   = ATMEL_MXT_TPD_NAME,
      },
    //.address_data = NULL,//&addr_data,
    };

    static struct tpd_driver_t tpd_device_driver = {
      .tpd_device_name  = ATMEL_MXT_TS_NAME, //"atmel_mxt_ts",
      .tpd_local_init   = tpd_local_init,

/*<<EricHsieh,using the early suspend/resume for touch panel*/
/*
    #if defined( CONFIG_HAS_EARLYSUSPEND )
      .suspend          = atmel_ts_early_suspend,
      .resume           = atmel_ts_late_resume,
    #else
      .suspend          = tpd_suspend,
      .resume           = tpd_resume,
    #endif
*/
/*>>EricHsieh,using the early suspend/resume for touch panel*/

    #if defined( TPD_HAVE_BUTTON )
      .tpd_have_button  = 1,
    #else
      .tpd_have_button  = 0,
    #endif
    };

    static struct i2c_client  * mxt_i2c_client = NULL;
    static struct task_struct * thread = NULL;
    static DECLARE_WAIT_QUEUE_HEAD(waiter);

    static int  tpd_hwpwm_ctrl  = TPD_HWPWM_OFF;
    static uint16_t   atmel_reg_addr = 0;
    unsigned int      softkeys_status = 0x00;
    static int        tpd_flag = 0;

    extern struct tpd_device  * tpd;
    extern int  tpd_show_version;
    extern int  tpd_em_debuglog;
    extern int  tpd_register_flag;
    extern int  tpd_load_status;
    extern int  tpd_em_set_freq;
#endif /** End.. (_MXT224S_DRIVER_C_) **/

    #undef  GLOBAL
/**********************************************************************************
** Ending
***********************************************************************************/
#endif /** End.. (_MXT224S_DRIVER_H_) **/
