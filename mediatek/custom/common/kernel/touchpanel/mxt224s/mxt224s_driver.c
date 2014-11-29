	/********************************************************************************************
** mxt224s_driver.c
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
** 2013/1/21   Albert Wu
**  Creation.
**===========================================================================================
** Copyright (C) 2012 Arimacomm Corporation.
*********************************************************************************************/
#ifndef _MXT224S_DRIVER_C_
#define _MXT224S_DRIVER_C_
/********************************************************************************************
** Include files
*********************************************************************************************/
 
#include  "mxt224s_config.h"
#include <linux/kernel.h>
#include <linux/input/mt.h>
#include <linux/string.h>
/**********************************************************************************
** tpd_hwPower_ctrl
**---------------------------------------------------------------------------------
** The power control of touch panel.
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** ctrl_OnOff   input.
**    - Power on/off.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The result for setting.
**  - 0: Success.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
#define DRIVER_VERSION	108

//#define MXT224S_DEBUG 
//#define DEBUG_PRINT 

#define MXT224S_DELAY_TIME 		2500//10000 //10s
#define TOUCH_RESET_TIME    80 //80ms
#define CRC_ERROR 		 0
#define CRC_PASS               1



#ifdef AUTO_DETECT_PANEL_ID //==============================
 uint8_t Panel_ID =0x00;

#else //==================================================

#if defined( ARIMA_PROJECT_HAWK35 )
       #define BILE_PANEL
	//#define ELK_PANEL 
#elif defined( ARIMA_PROJECT_HAWK40 )
	//#define BILE_PANEL
	#define CANDO_PANEL
#endif

#endif //===================================================

#include <linux/workqueue.h>

uint8_t check_sum_num1,check_sum_num2,check_sum_num3;
//static bool verifyingCal;
//unsigned long toc ,Timestamp;
static bool verifyingCal=TRUE;
unsigned long toc ,Timestamp=0;


#ifdef DEBUG_PRINT
	#define dbg_printk( format, arg...)		\
				printk(format, ##arg)
#else
	#define dbg_printk(format, arg...)	
#endif

static int  atmel_ts_stop_ic_calibration(ATMEL_TS *ts)
{
     uint8_t data[10] = { 0x00 };
	 
     i2c_atmel_write_byte_data(ts->client, get_object_address(ts,GEN_ACQUISITIONCONFIG_T8) + 4, 0x96); 
     i2c_atmel_write_byte_data(ts->client, get_object_address(ts,GEN_ACQUISITIONCONFIG_T8) + 6, 0x0A);  
     i2c_atmel_write_byte_data(ts->client, get_object_address(ts,GEN_ACQUISITIONCONFIG_T8) + 7, 0x0F);
     i2c_atmel_write_byte_data(ts->client, get_object_address(ts,GEN_ACQUISITIONCONFIG_T8) + 8, 0x00);
     i2c_atmel_write_byte_data(ts->client, get_object_address(ts,GEN_ACQUISITIONCONFIG_T8) + 9, 0x00); 

     i2c_atmel_read( ts->client, get_object_address( ts, GEN_ACQUISITIONCONFIG_T8 ),(uint8_t*)&data[0], 10 );

/*
{
     		 printk("\n T8[0]=%x",data[0]); 
			 printk("\n T8[1]=%x",data[1]);
			 printk("\n T8[2]=%x",data[2]);
			 printk("\n T8[3]=%x",data[3]);
			 printk("\n T8[4]=%x",data[4]);
			 printk("\n T8[5]=%x",data[5]);
			 printk("\n T8[6]=%x",data[6]);
			 printk("\n T8[7]=%x",data[7]);
			 printk("\n T8[8]=%x",data[8]);
			 printk("\n T8[9]=%x",data[9]); 	 

}	
*/
     if(data[4]==0x96 && data[6]==0x0A && data[7]==0x0F && data[8]==0x00 && data[9]==0x00){
		verifyingCal=FALSE;
		return  0;
     }
     else
	 	return -1;
	 
     	 
}




static int tpd_hwPower_ctrl(int ctrl_OnOff)
{
  
  switch( ctrl_OnOff )
  {
    case TPD_HWPWM_ON:
    {
      if( TPD_HWPWM_OFF == tpd_hwpwm_ctrl )
      {
    #if defined( ARIMA_PROJECT_HAWK40 )
      #if defined( HAWK40_EP0 ) || defined( HAWK40_EP1 )
        /* GPIO for VPS EN: VPS_CTP_3V3. 3.3V */
        mt_set_gpio_mode( GPIO_VPS_EN_PIN, GPIO_VPS_EN_PIN_M_GPIO );
        mt_set_gpio_dir( GPIO_VPS_EN_PIN, GPIO_DIR_OUT );
        mt_set_gpio_out( GPIO_VPS_EN_PIN, GPIO_OUT_ONE );
      #else
        hwPowerOn( MT65XX_POWER_LDO_VGP2,  VOL_3300,  TPD_HWPWM_NAME );
      #endif
    #elif defined( ARIMA_PROJECT_HAWK35 )
      #if defined( HAWK35_EP0 ) || defined( HAWK35_EP1 )
        hwPowerOn( MT65XX_POWER_LDO_VGP2,  VOL_1800, TPD_HWPWM_NAME );
        hwPowerOn( MT65XX_POWER_LDO_VCAMD, VOL_3300, TPD_HWPWM_NAME );
      #else
        hwPowerOn( MT65XX_POWER_LDO_VGP2,  VOL_3300,  TPD_HWPWM_NAME );
      #endif
    #else
        hwPowerOn( MT65XX_POWER_LDO_VCAMD, VOL_3300,  TPD_HWPWM_NAME );
    #endif
        tpd_hwpwm_ctrl = TPD_HWPWM_ON;
      }
    } break;

    case TPD_HWPWM_OFF:
    {
      if( TPD_HWPWM_ON == tpd_hwpwm_ctrl )
      {
    #if defined( ARIMA_PROJECT_HAWK40 )
      #if defined( HAWK40_EP0 )
        /* GPIO for VPS EN */
        mt_set_gpio_mode( GPIO_VPS_EN_PIN, GPIO_VPS_EN_PIN_M_GPIO );
        mt_set_gpio_dir( GPIO_VPS_EN_PIN, GPIO_DIR_OUT );
        mt_set_gpio_out( GPIO_VPS_EN_PIN, GPIO_OUT_ZERO );
      #else
        hwPowerDown( MT65XX_POWER_LDO_VGP2,  TPD_HWPWM_NAME );
      #endif
    #elif defined( ARIMA_PROJECT_HAWK35 )
      #if defined( HAWK35_EP0 ) || defined( HAWK35_EP1 )
        hwPowerDown( MT65XX_POWER_LDO_VGP2,  TPD_HWPWM_NAME );
        hwPowerDown( MT65XX_POWER_LDO_VCAMD, TPD_HWPWM_NAME );
      #else
        hwPowerDown( MT65XX_POWER_LDO_VGP2,  TPD_HWPWM_NAME );
      #endif
    #else
        hwPowerDown( MT65XX_POWER_LDO_VCAMD, TPD_HWPWM_NAME );
    #endif
        tpd_hwpwm_ctrl = TPD_HWPWM_OFF;
      }
    } break;

    default:
      break;
  }
  return  0;
} /* End.. tpd_hwPower_ctrl() */

/**********************************************************************************
** i2c_atmel_read
**---------------------------------------------------------------------------------
** Read data from touch panel.
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** client     input.
** address    input.
** data       output.
**    - The buffer that data was read.
** length     input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The result for setting.
**  - 0: Success.
**  - Other: Fail.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
int i2c_atmel_read(struct i2c_client *client, uint16_t address, uint8_t *data, uint8_t length)
{
	struct i2c_msg  msg[2];

	int       vRet = 0;
       u8 buf[2];

	  //pr_info( "[ATMEL] %s ...\n", __func__ );
	  
	  memset( data, 0x00, length );
	  
	  if( NULL == data ){
		dbg_printk("\n data=NULL.................");
		return  -EINVAL;
	  }	
	  buf[0] = address & 0xff;   
	  buf[1] = (address >> 8) & 0xff;
		
	   /* Write register */
	  msg[0].addr = client->addr;
  	  msg[0].flags = 0;
	  msg[0].timing = client->timing;
	  msg[0].len = 2;
	  msg[0].buf = buf;
	  msg[0].ext_flag = client->ext_flag;

	   /* Read data */
	  msg[1].addr = client->addr;
	  msg[1].flags = I2C_M_RD;
	  msg[1].len = length;
	  msg[1].buf = data;
	  msg[1].timing = client->timing;
	  msg[1].ext_flag = client->ext_flag;
	  vRet	=i2c_transfer(client->adapter, msg, 2); 
         if (vRet!= 2) {
		dev_err(&client->dev, "%s: i2c transfer failed\n", __func__);
		return -EINVAL;
	}

  	return  length;
} /* End.. i2c_atmel_read() */

/**********************************************************************************
** i2c_atmel_write
**---------------------------------------------------------------------------------
** Write data into touch panel.
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** client     input.
** address    input.
** data       output.
**    - The buffer that data will be write.
** length     input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The result for setting.
**  - 0: Success.
**  - Other: Fail.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
int i2c_atmel_write(struct i2c_client *client, uint16_t address, uint8_t *data, uint8_t length)
{
	struct i2c_msg  msg;
	int    vRet = 0;
	u8    buf[length+2];
       u8    loop_i;
	   
	  
	  if( NULL == data ){
		dbg_printk("\n data=NULL.................");
		return  -EINVAL;
	  }
	  
	  	buf[0] = address & 0xff;   
	  	buf[1] = (address >> 8) & 0xff;

		for (loop_i = 0; loop_i < length; loop_i++)
			buf[loop_i + 2] = data[loop_i];
		
		/* Write register */
		msg.addr = client->addr;
		msg.flags = 0;
		msg.timing = client->timing;
		msg.len = length + 2;
		msg.buf = buf;
		msg.ext_flag = client->ext_flag;
		vRet	=i2c_transfer(client->adapter, &msg, 1); 
		if (vRet!= 1) {
			dev_err(&client->dev, "%s: i2c transfer failed\n", __func__);
			return -EINVAL;
		}
  

  	return  length;
} /* End.. i2c_atmel_write() */

/**********************************************************************************
** i2c_atmel_write_byte_data
**---------------------------------------------------------------------------------
** Write a byte data into touch panel.
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** client     input.
** address    input.
** value      input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The result for setting.
**  - 0: Success.
**  - Other: Fail.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
int i2c_atmel_write_byte_data(struct i2c_client *client, uint16_t address, uint8_t value)
{
int   vRet = 0;

  vRet = i2c_atmel_write( client, address, (uint8_t*)&value, 1 );

  return  vRet;
} /* End.. i2c_atmel_write_byte_data() */

/**********************************************************************************
** get_object_address
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** ts           input.
** object_type  input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The object address.
**  - Object address.
**  - 0: None-address.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
uint16_t get_object_address(ATMEL_TS *ts, uint8_t object_type)
{
uint8_t   i = 0;

  for( i = 0; i < ts->id->num_declared_objects; i++ )
  {
    if( ts->object_table[i].object_type == object_type ){
      return  ts->object_table[i].i2c_address;
    }	  
  }
  return  0;
} /* End.. get_object_address() */

/**********************************************************************************
** get_object_size
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** ts           input.
** object_type  input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The object size.
**  - Object size.
**  - 0.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
uint8_t get_object_size(ATMEL_TS *ts, uint8_t object_type)
{
uint8_t   i = 0;

  for( i = 0; i < ts->id->num_declared_objects; i++ )
  {
    if( ts->object_table[i].object_type == object_type )
      return  ts->object_table[i].size;
  }

  return  0;
} /* End.. get_object_size() */

/**********************************************************************************
** get_report_ids_size
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** ts           input.
** object_type  input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The object id.
**  - Object id.
**  - 0.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
uint8_t get_report_ids_size(ATMEL_TS *ts, uint8_t object_type)
{
uint8_t   i = 0;

  for( i = 0; i < ts->id->num_declared_objects; i++ )
  {
    if( ts->object_table[i].object_type == object_type )
      return  ts->object_table[i].report_ids;
  }
  return  0;
} /* End.. get_report_ids_size() */

/**********************************************************************************
** atmel_ts_poll
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** client     input.
** UpDown     input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The status of interrupt pin.
**  - 0   LOW.
**  - Timeout
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static int atmel_ts_poll(struct i2c_client *client)
{
int status = 0, retry = 0;

  pr_info( "[ATMEL] %s ...\n", __func__ );
  /*reset device IC and check INT pin state again*/
  do
  {
    status = mt_get_gpio_in( GPIO_CTP_EINT_PIN );
    retry += 1;
    mdelay( 10 );
  } while(( status != 0 ) && ( retry < ATMEL_I2C_RETRY_TIMES ));

  pr_info( "[ATMEL] %s: poll interrupt status %s\n", __func__,
      status == 1 ? "high" : "low" );

  return ( status == 0 ? 0 : -ETIMEDOUT );
} /* End.. atmel_ts_poll() */

#if defined( ATMEL_EN_SYSFS )
/**********************************************************************************
** atmel_gpio_show
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** dev    input.
** attr   input.
** buf    output.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The string size.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static ssize_t atmel_gpio_show(struct device *dev,
      struct device_attribute *attr, char *buf)
{
int   vRet = 0;

  if( NULL == buf )
  {
    pr_info( "[ATMEL] %s: Parameter error\n", __func__ );
    return  vRet;
  }

  vRet = mt_get_gpio_in( GPIO_CTP_EINT_PIN );
  printk( KERN_DEBUG "GPIO_TP_INT_N=%d\n", GPIO_CTP_EINT_PIN );
  sprintf( buf, "GPIO_TP_INT_N=%d\n", vRet );
  vRet = strlen( buf ) + 1;

  return  vRet;
} /* End.. atmel_gpio_show() */
static DEVICE_ATTR(gpio, 0444, atmel_gpio_show, NULL);

/**********************************************************************************
** atmel_vendor_show
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** dev    input.
** attr   input.
** buf    output.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The string size.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static ssize_t atmel_vendor_show(struct device *dev,
      struct device_attribute *attr, char *buf)
{
ATMEL_TS  * ts = NULL;
int   vRet = 0;

  if( NULL == private_ts )
    return  vRet;

  ts = private_ts;
  sprintf( buf, "%s_x%4.4X_x%4.4X\n", "ATMEL", ts->id->family_id, ts->id->version );
  vRet = strlen( buf ) + 1;

  return  vRet;
} /* End.. atmel_vendor_show() */
static DEVICE_ATTR(vendor, 0444, atmel_vendor_show, NULL);

/**********************************************************************************
** atmel_register_show
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** dev    input.
** attr   input.
** buf    output.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The string size.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static ssize_t atmel_register_show(struct device *dev,
      struct device_attribute *attr, char *buf)
{
ATMEL_TS  * ts = NULL;
int     vRet = 0;
uint8_t ptr[1] = { 0 };

  if( NULL == private_ts )
    return  vRet;

  ts = private_ts;
  if( i2c_atmel_read( ts->client, atmel_reg_addr, (uint8_t*)&ptr, 1 ) < 0 )
  {
    printk( KERN_WARNING "%s: read fail\n", __func__ );
    return  vRet;
  }
  vRet += sprintf( buf, "addr: %d, data: %d\n", atmel_reg_addr, ptr[0] );

  return vRet;
} /* End.. atmel_register_show() */

/**********************************************************************************
** atmel_register_store
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** dev    input.
** attr   input.
** buf    input.
** count  input
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static ssize_t atmel_register_store(struct device *dev,
      struct device_attribute *attr, const char *buf, size_t count)
{
ATMEL_TS  * ts = NULL;
int   vRet = 0;
char  buf_tmp[4] = { 0x00 }, buf_zero[200] = { 0x00 };
uint8_t write_da = 0; /* Write out data */


  if( NULL == private_ts )
    return  vRet;

  ts = private_ts;
  memset( buf_tmp, 0x00, sizeof( buf_tmp ));
  if((( buf[0] == 'r' ) || ( buf[0] == 'w' )) && ( buf[1] == ':' ) &&
     (( buf[5] == ':' ) || ( buf[5] == '\n' )))
  {
    memcpy( buf_tmp, buf + 2, 3 );
    atmel_reg_addr = simple_strtol( buf_tmp, NULL, 10 );
    printk( KERN_DEBUG "read addr: 0x%X\n", atmel_reg_addr );
    if( !atmel_reg_addr )
    {
      printk( KERN_WARNING "%s: string to number fail\n", __func__ );
      return  count;
    }
    printk( KERN_DEBUG "%s: set atmel_reg_addr is: %d\n", __func__, atmel_reg_addr );
    if( buf[0] == 'w' && buf[5] == ':' && buf[9] == '\n' )
    {
      memcpy( buf_tmp, buf + 6, 3 );
      write_da = simple_strtol( buf_tmp, NULL, 10 );
      printk( KERN_DEBUG "write addr: 0x%X, data: 0x%X\n", atmel_reg_addr, write_da );
      vRet = i2c_atmel_write_byte_data( ts->client, atmel_reg_addr, write_da );
      if( vRet < 0 )
      {
        printk( KERN_ERR "%s: write fail(%d)\n", __func__, vRet );
      }
    }
  }

  if(( buf[0] == '0' ) && ( buf[1] == ':' ) && ( buf[5] == ':' ))
  {
    memcpy( buf_tmp, buf + 2, 3 );
    atmel_reg_addr = simple_strtol( buf_tmp, NULL, 10 );
    memcpy( buf_tmp, buf + 6, 3 );
    memset( buf_zero, 0x00, sizeof( buf_zero ));
    vRet = i2c_atmel_write( ts->client, atmel_reg_addr, (uint8_t*)&buf_zero[0],
                simple_strtol( buf_tmp, NULL, 10 ) - atmel_reg_addr + 1 );
    if( buf[9] == 'r' )
    {
      i2c_atmel_write_byte_data( ts->client,
          get_object_address( ts, GEN_COMMANDPROCESSOR_T6 ) + 1, 0x55 );
      i2c_atmel_write_byte_data( ts->client,
          get_object_address( ts, GEN_COMMANDPROCESSOR_T6 ), 0x11 );
    }
  }
  return  count;
} /* End.. atmel_register_show() */
static DEVICE_ATTR(register, 0644, atmel_register_show, atmel_register_store);

//=======================================================================

static ssize_t atmel_adb_ctrl_read(struct device *dev,
      struct device_attribute *attr, char *buf)
{
	ATMEL_TS  * ts = NULL;

	printk( KERN_DEBUG "\n atmel_adb_ctrl_read.........");
	return 1;
} /* End.. atmel_register_show() */

static ssize_t atmel_adb_ctrl_write(struct device *dev,
      struct device_attribute *attr, const char *buf, size_t count)
{
	ATMEL_TS  * ts = NULL;
	int   vRet = 0;
	char  buf_tmp;

	buf_tmp = *buf -48;

	 if( NULL == private_ts )
    		return  vRet;

  	ts = private_ts;
	
	printk( KERN_DEBUG "\n atmel_adb_ctrl_write.........");

	if(buf_tmp == 0)
	{
		atmel_ts_suspend_function(ts->client);
	}
	else if(buf_tmp == 1)
	{
		atmel_ts_resume_function(ts->client);
	}
	else
	{
		printk( KERN_DEBUG "\n buf_tmp value is error..............");
	}
		
	return 1;
  
} /* End.. atmel_register_show() */
static DEVICE_ATTR(adb_ctrl, 0644, atmel_adb_ctrl_read, atmel_adb_ctrl_write);
//=======================================================================

/**********************************************************************************
** atmel_regdump_show
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** dev    input.
** attr   input.
** buf    input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static ssize_t atmel_regdump_show(struct device *dev,
      struct device_attribute *attr, char *buf)
{
ATMEL_TS  * ts = NULL;
int       count = 0, ret_t = 0;
uint16_t  loop_i = 0;
uint8_t   ptr[1] = { 0 };

  if( NULL == private_ts )
    return  count;

  ts = private_ts;
  if( ts->id->version >= MXT_VER_20 )
  {
    for( loop_i = 230; loop_i <= 425; loop_i++ )
    {
      ret_t = i2c_atmel_read( ts->client, loop_i, (uint8_t*)&ptr[0], 1 );
      if( ret_t < 0 )
      {
        printk( KERN_WARNING "dump fail, addr: %d\n", loop_i );
      }
      count += sprintf( buf + count, "addr[%3d]: %3d, ", loop_i , *ptr );
      if((( loop_i - 230 ) % 4 ) == 3 )
        count += sprintf( buf + count, "\n" );
    }
    count += sprintf( buf + count, "\n" );
  }
  return count;
} /* End.. atmel_regdump_show() */

/**********************************************************************************
** atmel_regdump_dump
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** dev    input.
** attr   input.
** buf    input.
** count  input
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static ssize_t atmel_regdump_dump(struct device *dev,
      struct device_attribute *attr, const char *buf, size_t count)
{
ATMEL_TS  * ts = NULL;

  if( NULL == private_ts )
    return  0;

  ts = private_ts;
  if(( buf[0] >= '0' ) && ( buf[0] <= '9' ) && ( buf[1] == '\n' ))
    ts->debug_log_level = buf[0] - 0x30; /* Number character to digital */

  return  count;
} /* End.. atmel_regdump_dump() */
static DEVICE_ATTR(regdump, 0644, atmel_regdump_show, atmel_regdump_dump);

/**********************************************************************************
** atmel_debug_level_show
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** dev    input.
** attr   input.
** buf    input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static ssize_t atmel_debug_level_show(struct device *dev,
      struct device_attribute *attr, char *buf)
{
ATMEL_TS  * ts = NULL;
size_t  count = 0;

  if( NULL == private_ts )
    return  count;

  ts = private_ts;

  count += sprintf( buf, "%d\n", ts->debug_log_level );

  return  count;
} /* End.. atmel_debug_level_show() */

/**********************************************************************************
** atmel_debug_level_dump
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** dev    input.
** attr   input.
** buf    input.
** count  input
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static ssize_t atmel_debug_level_dump(struct device *dev,
      struct device_attribute *attr, const char *buf, size_t count)
{
ATMEL_TS  * ts = NULL;

  count = 0;
  if( NULL == private_ts )
    return  count;

  ts = private_ts;
  if(( buf[0] >= '0' ) && ( buf[0] <= '9' ) && ( buf[1] == '\n' ))
  {
    ts->debug_log_level = buf[0] - 0x30; /* Number character to digital */
    count = 1;
  }

  return  count;
} /* End.. atmel_debug_level_dump() */
static DEVICE_ATTR(debug_level, 0644, atmel_debug_level_show, atmel_debug_level_dump);

/**********************************************************************************
** atmel_touch_sysfs_init
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** None.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static int atmel_touch_sysfs_init(void) 
{
int   vRet = 0;

  pr_info( "[ATMEL] %s ...\n", __func__ );

  android_touch_kobj = kobject_create_and_add( "android_touch", NULL );
  if( NULL == android_touch_kobj )
  {
    printk( KERN_ERR "%s: subsystem_register failed\n", __func__ );
    vRet = -ENOMEM;
    return  vRet;
  }
  vRet = sysfs_create_file( android_touch_kobj, &dev_attr_gpio.attr );
  if( vRet )
  {
    printk( KERN_ERR "%s: sysfs_create_file dev_attr_gpio failed\n", __func__ );
    return  vRet;
  }
  vRet = sysfs_create_file( android_touch_kobj, &dev_attr_vendor.attr );
  if( vRet )
  {
    printk( KERN_ERR "%s: sysfs_create_file dev_attr_vendor failed\n", __func__ );
    return  vRet;
  }
  atmel_reg_addr = 0;
  vRet = sysfs_create_file( android_touch_kobj, &dev_attr_register.attr );
  if (vRet) {
    printk( KERN_ERR "%s: sysfs_create_file dev_attr_register failed\n", __func__);
    return  vRet;
  }
  vRet = sysfs_create_file( android_touch_kobj, &dev_attr_regdump.attr );
  if( vRet )
  {
    printk( KERN_ERR "%s: sysfs_create_file dev_attr_regdump failed\n", __func__ );
    return  vRet;
  }
  vRet = sysfs_create_file( android_touch_kobj, &dev_attr_debug_level.attr );
  if( vRet )
  {
    printk( KERN_ERR "%s: sysfs_create_file dev_attr_debug_level failed\n", __func__ );
    return  vRet;
  }

   vRet = sysfs_create_file( android_touch_kobj, &dev_attr_adb_ctrl.attr );
  if (vRet) {
    printk( KERN_ERR "%s: sysfs_create_file dev_attr_adb_ctrl failed\n", __func__);
    return  vRet;
  }
  return  0;
} /* End.. atmel_touch_sysfs_init() */

/**********************************************************************************
** atmel_touch_sysfs_deinit
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** None.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static void atmel_touch_sysfs_deinit(void)
{
  sysfs_remove_file( android_touch_kobj, &dev_attr_debug_level.attr );
  sysfs_remove_file( android_touch_kobj, &dev_attr_regdump.attr );
  sysfs_remove_file( android_touch_kobj, &dev_attr_register.attr );
  sysfs_remove_file( android_touch_kobj, &dev_attr_vendor.attr );
  sysfs_remove_file( android_touch_kobj, &dev_attr_gpio.attr );
  kobject_del( android_touch_kobj );
} /* End.. atmel_touch_sysfs_deinit() */

#endif /* End.. (ATMEL_EN_SYSFS) */

static int atmel_ts_CRC_judge(ATMEL_TS *ts)
{
	int loop_i;
	uint8_t data[6]={0x00,0x00,0x00,0x00,0x00,0x00};

#ifdef AUTO_DETECT_PANEL_ID //===================AUTO_DETECT_PANEL_ID==============================
#if defined( ARIMA_PROJECT_HAWK35 )

if(Panel_ID == HAWK35_BIEL_NITTO_PANEL_ID)
{
	check_sum_num1=HAWK35_BIEL_P2_CHECK_SUM_NUM1;
	check_sum_num2=HAWK35_BIEL_P2_CHECK_SUM_NUM2;
	check_sum_num3=HAWK35_BIEL_P2_CHECK_SUM_NUM3;
}
else if(Panel_ID == HAWK35_ELK_PANEL_ID)
{
	check_sum_num1=HAWK35_ELK_CHECK_SUM_NUM1;
	check_sum_num2=HAWK35_ELK_CHECK_SUM_NUM2;
	check_sum_num3=HAWK35_ELK_CHECK_SUM_NUM3;
}

else
{
	check_sum_num1=HAWK35_BIEL_P1_CHECK_SUM_NUM1;
	check_sum_num2=HAWK35_BIEL_P1_CHECK_SUM_NUM2;
	check_sum_num3=HAWK35_BIEL_P1_CHECK_SUM_NUM3;
	printk("\n HAWK35 atmel_ts_CRC_judge......... panel ID error.......judge Biel P1 check sum....");
}

#elif defined( ARIMA_PROJECT_HAWK40 )

 if(Panel_ID ==HAWK40_CANDO_PANEL_ID)
{
	check_sum_num1=HAWK40_CANDO_CHECK_SUM_NUM1;
	check_sum_num2=HAWK40_CANDO_CHECK_SUM_NUM2;
	check_sum_num3=HAWK40_CANDO_CHECK_SUM_NUM3;
}
 else if(Panel_ID == HAWK40_BIEL_NITTO_PANEL_ID)
{
	check_sum_num1=HAWK40_BIEL_P2_CHECK_SUM_NUM1;
	check_sum_num2=HAWK40_BIEL_P2_CHECK_SUM_NUM2;
	check_sum_num3=HAWK40_BIEL_P2_CHECK_SUM_NUM3;
 }
 
else
{
	check_sum_num1=HAWK40_CANDO_CHECK_SUM_NUM1;
	check_sum_num2=HAWK40_CANDO_CHECK_SUM_NUM2;
	check_sum_num3=HAWK40_CANDO_CHECK_SUM_NUM3;
	printk("\n HAWK40 atmel_ts_CRC_judge......... panel ID error.......judge Cando check sum....");
}

#endif



#else //=================================AUTO_DETECT_PANEL_ID====================================

#if defined( ARIMA_PROJECT_HAWK35 ) //################################################

#if defined (BILE_PANEL)  

check_sum_num1=HAWK35_BIEL_P1_CHECK_SUM_NUM1;
check_sum_num2=HAWK35_BIEL_P1_CHECK_SUM_NUM2;
check_sum_num3=HAWK35_BIEL_P1_CHECK_SUM_NUM3;

#elif defined(ELK_PANEL )  

check_sum_num1=HAWK35_ELK_CHECK_SUM_NUM1;
check_sum_num2=HAWK35_ELK_CHECK_SUM_NUM2;
check_sum_num3=HAWK35_ELK_CHECK_SUM_NUM3;

#endif  

#elif defined( ARIMA_PROJECT_HAWK40 )//#################################################

#if defined(CANDO_PANEL )   

check_sum_num1=HAWK40_CANDO_CHECK_SUM_NUM1;
check_sum_num2=HAWK40_CANDO_CHECK_SUM_NUM2;
check_sum_num3=HAWK40_CANDO_CHECK_SUM_NUM3;

#elif defined (BILE_PANEL)  

check_sum_num1=HAWK40_BIEL_P2_CHECK_SUM_NUM1;
check_sum_num2=HAWK40_BIEL_P2_CHECK_SUM_NUM2;
check_sum_num3=HAWK40_BIEL_P2_CHECK_SUM_NUM3;

#endif 

#endif //############################################################################

#endif //=================================AUTO_DETECT_PANEL_ID=====================================	
	ts->crc_check_sum[0]=check_sum_num1;
       ts->crc_check_sum[1]=check_sum_num2;
	ts->crc_check_sum[2]=check_sum_num3; 

	i2c_atmel_write_byte_data(ts->client,	get_object_address(ts, GEN_COMMANDPROCESSOR_T6) + 2, 0x55);   //T6_CFG_CALIBRATE
	for (loop_i = 0; loop_i < 10; loop_i++) {
		if (!mt_get_gpio_in( GPIO_CTP_EINT_PIN )) {
		       i2c_atmel_read(ts->client, get_object_address(ts,GEN_MESSAGEPROCESSOR_T5), data, 5);
			printk("\n CRC...... 0x%2.2X 0x%2.2X 0x%2.2X 0x%2.2X 0x%2.2X \n",data[0],data[1],data[2],data[3],data[4]); 
			
			if (data[0] == 0x01) //T6 Repord ID
				break;
		}
		msleep(10);
	}
	if (loop_i == 10) 
		printk("Touch: No checksum read\n");
	else {
		for (loop_i = 0; loop_i < 3; loop_i++) {
			if (ts->crc_check_sum[loop_i] != data[loop_i + 2]) {   //T6_MSG_CHECKSUM
				printk("CRC Error: %x, %x %x \n", ts->crc_check_sum[0], ts->crc_check_sum[1],ts->crc_check_sum[2]);
				return CRC_ERROR;
			}
		}
		if (loop_i == 3) {
			printk("\n CRC passed: ");
			for (loop_i = 0; loop_i < 3; loop_i++)
				printk("0x%2.2X ", ts->crc_check_sum[loop_i]);
			printk("\n");
			return CRC_PASS;
		}
	}

}


#ifdef MXT224S_DEBUG
static void atmel_ts_dump_register(ATMEL_TS *ts)
{
	int i,index;
	uint8_t   data[300] = { 0x00 };
	for( i = 0; i < ts->id->num_declared_objects; i++ )
	{
		i2c_atmel_read( ts->client, ts->object_table[i].i2c_address, (uint8_t*)&data[0], ts->object_table[i].size );
		dbg_printk("\n Object Type:%2.2d",ts->object_table[i].object_type);
		dbg_printk(" Object Size:%2.2d \n",ts->object_table[i].size );
		dbg_printk("\n Contents: \n");

		for(index=0;index < ts->object_table[i].size ;index++)
			dbg_printk("Id:%2.2d ",index);
		dbg_printk("\n");
		for(index=0;index < ts->object_table[i].size ;index++)
			dbg_printk("   %2.2x ",data[index]);

		dbg_printk("\n =============================================== \n");
	       
	}
}
#endif


#if defined( ARIMA_PROJECT_HAWK35 )

static void atmel_ts_write_Hawk35_Biel_P1_Config(ATMEL_TS *ts) 
{
	uint8_t i,obj_type,write_obj_num; 

	printk(" \n %s : .............\n", __FUNCTION__);
	
	write_obj_num = ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_init_obj); 

	uint16_t address;
       dbg_printk("\n write_obj_num=%d",write_obj_num);
	for(i=0; i<write_obj_num ; i++){
		obj_type=Hawk35_Biel_mxt224s_p1_init_obj[i];
		address=get_object_address( ts, obj_type);
		dbg_printk("\n obj_type=%d........address=%d",obj_type,address);
		switch(obj_type){
				
			case DIAGNOSTIC_T37:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T37_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T37_value) );  
				 printk("\n T37 Obj.........");
				 break;
/*				 
			case SPT_USERDATA_T38:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&mxt224s_T38_value, ARRAY_SIZE(mxt224s_T38_value) ); 
				 printk("\n T38 Obj.........");
				 break;
*/				 
			case GEN_POWERCONFIG_T7:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T7_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T7_value) );   
				 break;
			case GEN_ACQUISITIONCONFIG_T8:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T8_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T8_value) );   
				 break;
			case TOUCH_MULTITOUCHSCREEN_T9:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T9_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T9_value) );   
				 break;
			case TOUCH_KEYARRAY_T15:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T15_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T15_value) );   
				 break;
			case SPT_COMCONFIG_T18:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T18_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T18_value) );   
				 break;
			case SPT_GPIOPWM_T19:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T19_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T19_value) );   
				 break;
			case TOUCH_PROXIMITY_T23:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T23_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T23_value) );   
				 break;	 
			case SPT_SELFTEST_T25:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T25_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T25_value) );   
				 break;
			case PROCI_GRIP_T40:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T40_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T40_value) );   
				 break;
			case PROCI_TOUCHSUPPRESSION_T42:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T42_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T42_value) );   
				 break;
			case SPT_CTECONFIG_T46:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T46_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T46_value) );   
				 break;
			case PROCI_STYLUS_T47:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T47_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T47_value) );   
				 break;
			case PROCI_ADAPTIVETHRESHOLD_T55:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T55_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T55_value) );   
				 break;
			case PROCI_SHIELDLESS_T56:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T56_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T56_value) );   
				 break;
			case PROCI_EXTRATOUCHSCREENDATA_T57:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T57_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T57_value) );   
				 break;
			case SPT_TIMER_T61:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T61_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T61_value) );   
				 break;
			case PROCG_NOISESUPPRESSION_T62:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p1_T62_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p1_T62_value) );   
				 break;
			default:
				 dbg_printk("\n Not found mxt224s object............."); 
				 break;	 
				
		}	
		
	}
}

static void atmel_ts_write_Hawk35_Biel_P2_Config(ATMEL_TS *ts) 
{
	uint8_t i,obj_type,write_obj_num; 

	printk(" \n %s : .............\n", __FUNCTION__);
	
	write_obj_num = ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_init_obj); 

	uint16_t address;
       dbg_printk("\n write_obj_num=%d",write_obj_num);
	for(i=0; i<write_obj_num ; i++){
		obj_type=Hawk35_Biel_mxt224s_p2_init_obj[i];
		address=get_object_address( ts, obj_type);
		dbg_printk("\n obj_type=%d........address=%d",obj_type,address);
		switch(obj_type){
				
			case DIAGNOSTIC_T37:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T37_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T37_value) );  
				 printk("\n T37 Obj.........");
				 break;
/*				 
			case SPT_USERDATA_T38:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&mxt224s_T38_value, ARRAY_SIZE(mxt224s_T38_value) ); 
				 printk("\n T38 Obj.........");
				 break;
*/				 
			case GEN_POWERCONFIG_T7:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T7_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T7_value) );   
				 break;
			case GEN_ACQUISITIONCONFIG_T8:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T8_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T8_value) );   
				 break;
			case TOUCH_MULTITOUCHSCREEN_T9:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T9_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T9_value) );   
				 break;
			case TOUCH_KEYARRAY_T15:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T15_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T15_value) );   
				 break;
			case SPT_COMCONFIG_T18:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T18_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T18_value) );   
				 break;
			case SPT_GPIOPWM_T19:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T19_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T19_value) );   
				 break;
			case TOUCH_PROXIMITY_T23:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T23_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T23_value) );   
				 break;	 
			case SPT_SELFTEST_T25:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T25_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T25_value) );   
				 break;
			case PROCI_GRIP_T40:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T40_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T40_value) );   
				 break;
			case PROCI_TOUCHSUPPRESSION_T42:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T42_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T42_value) );   
				 break;
			case SPT_CTECONFIG_T46:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T46_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T46_value) );   
				 break;
			case PROCI_STYLUS_T47:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T47_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T47_value) );   
				 break;
			case PROCI_ADAPTIVETHRESHOLD_T55:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T55_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T55_value) );   
				 break;
			case PROCI_SHIELDLESS_T56:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T56_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T56_value) );   
				 break;
			case PROCI_EXTRATOUCHSCREENDATA_T57:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T57_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T57_value) );   
				 break;
			case SPT_TIMER_T61:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T61_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T61_value) );   
				 break;
			case PROCG_NOISESUPPRESSION_T62:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_Biel_mxt224s_p2_T62_value, ARRAY_SIZE(Hawk35_Biel_mxt224s_p2_T62_value) );   
				 break;
			default:
				 dbg_printk("\n Not found mxt224s object............."); 
				 break;	 
				
		}	
		
	}
}


static void atmel_ts_write_Hawk35_ELK_Config(ATMEL_TS *ts) 
{
	uint8_t i,obj_type,write_obj_num; 

	printk(" \n %s : .............\n", __FUNCTION__);
	
	write_obj_num = ARRAY_SIZE(Hawk35_ELK_mxt224s_init_obj); 

	uint16_t address;
       dbg_printk("\n write_obj_num=%d",write_obj_num);
	for(i=0; i<write_obj_num ; i++){
		obj_type=Hawk35_ELK_mxt224s_init_obj[i];
		address=get_object_address( ts, obj_type);
		dbg_printk("\n obj_type=%d........address=%d",obj_type,address);
		switch(obj_type){
				
			case DIAGNOSTIC_T37:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T37_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T37_value) );  
				 printk("\n T37 Obj.........");
				 break;
/*				 
			case SPT_USERDATA_T38:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&mxt224s_T38_value, ARRAY_SIZE(mxt224s_T38_value) ); 
				 printk("\n T38 Obj.........");
				 break;
*/				 
			case GEN_POWERCONFIG_T7:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T7_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T7_value) );   
				 break;
			case GEN_ACQUISITIONCONFIG_T8:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T8_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T8_value) );   
				 break;
			case TOUCH_MULTITOUCHSCREEN_T9:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T9_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T9_value) );   
				 break;
			case TOUCH_KEYARRAY_T15:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T15_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T15_value) );   
				 break;
			case SPT_COMCONFIG_T18:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T18_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T18_value) );   
				 break;
			case SPT_GPIOPWM_T19:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T19_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T19_value) );   
				 break;
			case TOUCH_PROXIMITY_T23:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T23_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T23_value) );   
				 break;	 
			case SPT_SELFTEST_T25:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T25_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T25_value) );   
				 break;
			case PROCI_GRIP_T40:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T40_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T40_value) );   
				 break;
			case PROCI_TOUCHSUPPRESSION_T42:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T42_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T42_value) );   
				 break;
			case SPT_CTECONFIG_T46:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T46_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T46_value) );   
				 break;
			case PROCI_STYLUS_T47:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T47_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T47_value) );   
				 break;
			case PROCI_ADAPTIVETHRESHOLD_T55:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T55_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T55_value) );   
				 break;
			case PROCI_SHIELDLESS_T56:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T56_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T56_value) );   
				 break;
			case PROCI_EXTRATOUCHSCREENDATA_T57:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T57_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T57_value) );   
				 break;
			case SPT_TIMER_T61:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T61_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T61_value) );   
				 break;
			case PROCG_NOISESUPPRESSION_T62:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk35_ELK_mxt224s_T62_value, ARRAY_SIZE(Hawk35_ELK_mxt224s_T62_value) );   
				 break;
			default:
				 dbg_printk("\n Not found mxt224s object............."); 
				 break;	 
				
		}	
		
	}
}

#elif defined( ARIMA_PROJECT_HAWK40 )

static void atmel_ts_write_Hawk40_Cando_Config(ATMEL_TS *ts) 
{
	uint8_t i,obj_type,write_obj_num; 
	
	printk(" \n %s : .............\n", __FUNCTION__);

	write_obj_num = ARRAY_SIZE(Hawk40_Cando_mxt224s_init_obj); 

	uint16_t address;
       dbg_printk("\n write_obj_num=%d",write_obj_num);
	for(i=0; i<write_obj_num ; i++){
		obj_type=Hawk40_Cando_mxt224s_init_obj[i];
		address=get_object_address( ts, obj_type);
		dbg_printk("\n obj_type=%d........address=%d",obj_type,address);
		switch(obj_type){
				
			case DIAGNOSTIC_T37:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T37_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T37_value) );  
				 printk("\n T37 Obj.........");
				 break;
/*				 
			case SPT_USERDATA_T38:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&mxt224s_T38_value, ARRAY_SIZE(mxt224s_T38_value) ); 
				 printk("\n T38 Obj.........");
				 break;
*/				 
			case GEN_POWERCONFIG_T7:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T7_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T7_value) );   
				 break;
			case GEN_ACQUISITIONCONFIG_T8:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T8_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T8_value) );   
				 break;
			case TOUCH_MULTITOUCHSCREEN_T9:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T9_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T9_value) );   
				 break;
			case TOUCH_KEYARRAY_T15:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T15_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T15_value) );   
				 break;
			case SPT_COMCONFIG_T18:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T18_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T18_value) );   
				 break;
			case SPT_GPIOPWM_T19:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T19_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T19_value) );   
				 break;
			case TOUCH_PROXIMITY_T23:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T23_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T23_value) );   
				 break;	 
			case SPT_SELFTEST_T25:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T25_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T25_value) );   
				 break;
			case PROCI_GRIP_T40:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T40_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T40_value) );   
				 break;
			case PROCI_TOUCHSUPPRESSION_T42:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T42_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T42_value) );   
				 break;
			case SPT_CTECONFIG_T46:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T46_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T46_value) );   
				 break;
			case PROCI_STYLUS_T47:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T47_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T47_value) );   
				 break;
			case PROCI_ADAPTIVETHRESHOLD_T55:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T55_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T55_value) );   
				 break;
			case PROCI_SHIELDLESS_T56:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T56_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T56_value) );   
				 break;
			case PROCI_EXTRATOUCHSCREENDATA_T57:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T57_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T57_value) );   
				 break;
			case SPT_TIMER_T61:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T61_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T61_value) );   
				 break;
			case PROCG_NOISESUPPRESSION_T62:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Cando_mxt224s_T62_value, ARRAY_SIZE(Hawk40_Cando_mxt224s_T62_value) );   
				 break;
			default:
				 dbg_printk("\n Not found mxt224s object............."); 
				 break;	 
				
		}	
		
	}
}


static void atmel_ts_write_Hawk40_Biel_P2_Config(ATMEL_TS *ts) 
{
	uint8_t i,obj_type,write_obj_num; 

	printk(" \n %s : .............\n", __FUNCTION__);
	
	write_obj_num = ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_init_obj); 

	uint16_t address;
       dbg_printk("\n write_obj_num=%d",write_obj_num);
	for(i=0; i<write_obj_num ; i++){
		obj_type=Hawk40_Biel_mxt224s_p2_init_obj[i];
		address=get_object_address( ts, obj_type);
		dbg_printk("\n obj_type=%d........address=%d",obj_type,address);
		switch(obj_type){
				
			case DIAGNOSTIC_T37:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T37_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T37_value) );  
				 printk("\n T37 Obj.........");
				 break;
/*				 
			case SPT_USERDATA_T38:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&mxt224s_T38_value, ARRAY_SIZE(mxt224s_T38_value) ); 
				 printk("\n T38 Obj.........");
				 break;
*/				 
			case GEN_POWERCONFIG_T7:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T7_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T7_value) );   
				 break;
			case GEN_ACQUISITIONCONFIG_T8:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T8_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T8_value) );   
				 break;
			case TOUCH_MULTITOUCHSCREEN_T9:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T9_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T9_value) );   
				 break;
			case TOUCH_KEYARRAY_T15:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T15_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T15_value) );   
				 break;
			case SPT_COMCONFIG_T18:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T18_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T18_value) );   
				 break;
			case SPT_GPIOPWM_T19:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T19_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T19_value) );   
				 break;
			case TOUCH_PROXIMITY_T23:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T23_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T23_value) );   
				 break;	 
			case SPT_SELFTEST_T25:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T25_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T25_value) );   
				 break;
			case PROCI_GRIP_T40:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T40_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T40_value) );   
				 break;
			case PROCI_TOUCHSUPPRESSION_T42:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T42_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T42_value) );   
				 break;
			case SPT_CTECONFIG_T46:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T46_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T46_value) );   
				 break;
			case PROCI_STYLUS_T47:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T47_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T47_value) );   
				 break;
			case PROCI_ADAPTIVETHRESHOLD_T55:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T55_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T55_value) );   
				 break;
			case PROCI_SHIELDLESS_T56:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T56_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T56_value) );   
				 break;
			case PROCI_EXTRATOUCHSCREENDATA_T57:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T57_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T57_value) );   
				 break;
			case SPT_TIMER_T61:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T61_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T61_value) );   
				 break;
			case PROCG_NOISESUPPRESSION_T62:
				 i2c_atmel_write( ts->client, address, (uint8_t*)&Hawk40_Biel_mxt224s_p2_T62_value, ARRAY_SIZE(Hawk40_Biel_mxt224s_p2_T62_value) );   
				 break;
			default:
				 dbg_printk("\n Not found mxt224s object............."); 
				 break;	 
				
		}	
		
	}
}


#endif

static uint8_t  atmel_ts_read_panel_ID(ATMEL_TS *ts) 
{
uint8_t panel_ID;

i2c_atmel_read( ts->client, get_object_address( ts, SPT_USERDATA_T38 ),(uint8_t*)&panel_ID, 1 );

printk("Read panelID=0x%x",panel_ID);

return panel_ID;

}


static int atmel_ts_get_info(ATMEL_TS *ts)
{
	int ret,i;
	uint8_t   type_count = 0;
	uint8_t   data[16] = { 0x00 };
	
	ret=atmel_ts_poll( ts->client );
  	if( ret < 0 ){
		dbg_printk("\n atmel_ts_poll Fail........");
		return  -EINVAL; 	
  	}
       ret = i2c_atmel_read( ts->client, 0x00, (uint8_t*)&data[0], 7 );
     	if(ret < 0)
     	{
		 printk("\n atmel_ts_get_info fail.........");
		 return  -EINVAL; 
	}
     
       ts->id->family_id   = data[0];
       ts->id->variant_id  = data[1];
       ts->id->version   = data[2];
       ts->id->build       = data[3];
       ts->id->matrix_x_size = data[4];
       ts->id->matrix_y_size = data[5];
       ts->id->num_declared_objects = data[6];
  
      printk( "\n info block: 0x%2.2X 0x%2.2X 0x%2.2X 0x%2.2X 0x%2.2X 0x%2.2X 0x%2.2X\n",
                   ts->id->family_id, ts->id->variant_id, ts->id->version, ts->id->build,
                   ts->id->matrix_x_size, ts->id->matrix_y_size, ts->id->num_declared_objects );
       printk("\n atmel mxt224s FW version =%x", ts->id->version);
       ts->object_table = kzalloc( sizeof(struct object_t)*ts->id->num_declared_objects, GFP_KERNEL );
       if( NULL == ts->object_table )
       {
     	    dbg_printk("\n %s: allocate object_table failed\n", __func__ );
     	    return -ENOMEM;;
       }
       for( i = 0; i < ts->id->num_declared_objects; i++ )
       {
     	    ret = i2c_atmel_read( ts->client, i * 6 + 0x07, (uint8_t*)&data[0], 6 );
     	    ts->object_table[i].object_type = data[0];
     	    ts->object_table[i].i2c_address = data[1] | ( data[2] << 8 );
     	    ts->object_table[i].size      = data[3] + 1;
     	    ts->object_table[i].instances = data[4];
     	    ts->object_table[i].num_report_ids = data[5];
			
     	    if( data[5] )
     	    {
     	      ts->object_table[i].report_ids = type_count + 1;
     	      type_count += data[5];
     	    }
     	    if( data[0] == TOUCH_MULTITOUCHSCREEN_T9 )
     	       ts->finger_type = ts->object_table[i].report_ids;
     	    dbg_printk("\n Type: %2.2X, Start: %4.4X, Size: %2X, Instance: %2X, RD#: %2X, %2X\n",
     	          ts->object_table[i].object_type , ts->object_table[i].i2c_address,
     	          ts->object_table[i].size, ts->object_table[i].instances,
     	          ts->object_table[i].num_report_ids, ts->object_table[i].report_ids );
       }

#ifdef AUTO_DETECT_PANEL_ID //=================================================


     Panel_ID=atmel_ts_read_panel_ID(ts);
#if defined( ARIMA_PROJECT_HAWK35 )

if(Panel_ID == HAWK35_BIEL_NITTO_PANEL_ID)
	printk("\n HAWK35 mxt224s Biel P2 config Version = %d",HAWK35_BIEL_P2_CONFIG_VERSION);
else if(Panel_ID == HAWK35_ELK_PANEL_ID)
	printk("\n HAWK35 mxt224s ELK P2 config Version = %d",HAWK35_ELK_CONFIG_VERSION);
else
	printk("\n Not found Panel ID...........................");

#elif defined( ARIMA_PROJECT_HAWK40 )

if(Panel_ID == HAWK40_CANDO_PANEL_ID)
	printk("\n HAWK40 mxt224s Cando P2 config Version = %d",HAWK40_CANDO_CONFIG_VERSION);
else if(Panel_ID == HAWK40_BIEL_NITTO_PANEL_ID)
	printk("\n HAWK40 mxt224s Biel P2 config Version = %d",HAWK40_BIEL_P2_CONFIG_VERSION);
else
	printk("\n  Not found Panel ID...........................");

#endif

#else //=====================================================================

	   
#if defined( ARIMA_PROJECT_HAWK35 )

#if defined (BILE_PANEL)
	 printk("\n HAWK35 mxt224s Biel P1 config Version = %d",HAWK35_BIEL_P1_CONFIG_VERSION);
#elif defined(ELK_PANEL )
	printk("\n  HAWK35 mxt224s ELK  config Version = %d",HAWK35_ELK_CONFIG_VERSION);
#endif

#elif defined( ARIMA_PROJECT_HAWK40 )

#if defined (CANDO_PANEL)
	printk("\n HAWK40 mxt224s Cando config Version = %d",HAWK40_CANDO_CONFIG_VERSION);
#elif defined (BILE_PANEL)
	printk("\n HAWK40 mxt224s Biel P2 config Version = %d",HAWK40_BIEL_P2_CONFIG_VERSION);
#endif
#endif

#endif //======================================================================
	printk("\n mxt224s driver Version = %d", DRIVER_VERSION);
	return 0;
}

static int atmel_ts_reinit(ATMEL_TS *ts)
{
	int ret,loop_i;

	printk(" \n %s : .............\n", __FUNCTION__);
	mt_set_gpio_out( GPIO_CTP_RST_PIN, GPIO_OUT_ONE );
  	msleep( 5 );
  	mt_set_gpio_out( GPIO_CTP_RST_PIN, GPIO_OUT_ZERO );
  	msleep( 40 );
  	mt_set_gpio_out( GPIO_CTP_RST_PIN, GPIO_OUT_ONE );

	mdelay(TOUCH_RESET_TIME );   
	for (loop_i = 0; loop_i < 30; loop_i++) {
		if (!mt_get_gpio_in( GPIO_CTP_EINT_PIN )) {
		      atmel_ts_get_info(ts);
		      break;	  
		}
		msleep(10);
	}
  	printk("Atmel reinit ...loopi=%d",loop_i); 
}

static void  atmel_ts_init(ATMEL_TS *ts)
{

#ifdef MXT224S_DEBUG
	atmel_ts_dump_register(ts);
#endif

#ifdef AUTO_DETECT_PANEL_ID //=================================================
#if defined( ARIMA_PROJECT_HAWK35 )

if(Panel_ID == HAWK35_BIEL_NITTO_PANEL_ID)
	atmel_ts_write_Hawk35_Biel_P2_Config(ts);
else if(Panel_ID == HAWK35_ELK_PANEL_ID)
	atmel_ts_write_Hawk35_ELK_Config(ts);
else
	atmel_ts_write_Hawk35_Biel_P1_Config(ts);


#elif defined( ARIMA_PROJECT_HAWK40 )

if(Panel_ID == HAWK40_CANDO_PANEL_ID)
	atmel_ts_write_Hawk40_Cando_Config(ts);
else if(Panel_ID == HAWK40_BIEL_NITTO_PANEL_ID)
	atmel_ts_write_Hawk40_Biel_P2_Config(ts);
else
	atmel_ts_write_Hawk40_Cando_Config(ts);

#endif

#else //=====================================================================


#if defined( ARIMA_PROJECT_HAWK35 )

#if defined (BILE_PANEL)
	atmel_ts_write_Hawk35_Biel_P1_Config(ts);
#elif defined(ELK_PANEL )
	atmel_ts_write_Hawk35_ELK_Config(ts);
#endif

#elif defined( ARIMA_PROJECT_HAWK40 )

#if defined (CANDO_PANEL)
	atmel_ts_write_Hawk40_Cando_Config(ts);
#elif defined(BILE_PANEL )
	atmel_ts_write_Hawk40_Biel_P2_Config(ts);
#endif

#endif

#endif //=======================================================================
       i2c_atmel_write_byte_data( ts->client,get_object_address( ts, GEN_COMMANDPROCESSOR_T6 ) + 1, 0x55 );//Back Up
       msleep(100);
	i2c_atmel_write_byte_data( ts->client,get_object_address( ts, GEN_COMMANDPROCESSOR_T6 ) , 0x01 );//SW Reset
	msleep(100);
   

	
}




/**********************************************************************************
**  atmel_ts_finger_data
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** fdata  input.
** data   input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static void  atmel_ts_finger_data(struct atmel_finger_data *fdata, uint8_t *data)
{

#if 1 
  fdata->x = (data[T9_MSG_XPOSMSB] << 2) | ((data[T9_MSG_XYPOSLSB] & ~0x3f) >> 6);  /* 10bit */
  fdata->y = (data[T9_MSG_YPOSMSB] << 2) | ((data[T9_MSG_XYPOSLSB] & ~0xf3 ) >>2); /* 10bit */
  fdata->w = data[T9_MSG_TCHAREA];
  fdata->z = data[T9_MSG_TCHAMPLITUDE];
#else
  fdata->x = (data[T9_MSG_XPOSMSB] << (2+2)) | (data[T9_MSG_XYPOSLSB] >> (6-2));  /* 12bit */
  fdata->y = (data[T9_MSG_YPOSMSB] << (2+2)) | (data[T9_MSG_XYPOSLSB] & 0x0F ); /* 12bit */
  fdata->w = data[T9_MSG_TCHAREA];
  fdata->z = data[T9_MSG_TCHAMPLITUDE];
#endif

} /* End..  atmel_ts_finger_data() */




static void atmel_ts_input_report(ATMEL_TS *ts, uint8_t *data , int single_id)
{

	int status = ts->finger_data[single_id].status;
	int finger_num = 0;
	int id; 
	int err;

 
	for (id = 0; id < MXT_MAX_FINGER; id++) {
		if (!ts->finger_data[id].status)
			continue;
		
		if (ts->finger_data[id].status == T9_MSG_STATUS_RELEASE){
			ts->finger_data[id].status = 0;
			continue;
		}	
		else 
			finger_num++;  
		  
		printk( KERN_INFO "\n X[%d].x=%d",id,ts->finger_data[id].x); 
		printk( KERN_INFO "\n Y[%d].y=%d",id,ts->finger_data[id].y);

		input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, id);
		input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR,ts->finger_data[id].z );
		input_report_abs( tpd->dev, ABS_MT_WIDTH_MAJOR, ts->finger_data[id].w );
		input_report_abs(tpd->dev, ABS_MT_POSITION_X,ts->finger_data[id].x);
		input_report_abs(tpd->dev, ABS_MT_POSITION_Y,ts->finger_data[id].y);
		input_mt_sync(tpd->dev);
	}
	input_report_key(tpd->dev, BTN_TOUCH, finger_num > 0);
	input_mt_sync(tpd->dev);
	input_sync(tpd->dev);
	
    	if(verifyingCal==TRUE){
		toc = jiffies;

		if(Timestamp == 0)
			Timestamp = toc;
		printk(KERN_DEBUG "\n HZ=%u , toc =%u  ,  Timestamp = %u ",HZ ,toc,Timestamp); 
		if(((toc -Timestamp) * 1000 / HZ) >= 2500){
			err=atmel_ts_stop_ic_calibration(ts);
			if(err < 0)
					printk(KERN_ERR "\n Stop calibration fail.........\n");
			else
					printk(KERN_ERR "\n Stop calibration success....\n");
		}
	}   	

}

static void atmel_ts_input_touchevent(ATMEL_TS *ts, uint8_t *data , int id)
{

       uint8_t status =data[1];

	/* Check the touch is present on the screen */
	if (!(status & T9_MSG_STATUS_DETECT)) {
		if (status & T9_MSG_STATUS_RELEASE) {  
			ts->finger_data[id].status = T9_MSG_STATUS_RELEASE;
			atmel_ts_input_report(ts,(uint8_t*)&data[0],id);
		}
		return;
	}

	/* Check only AMP detection */ 
	if (!(status & (T9_MSG_STATUS_PRESS | T9_MSG_STATUS_MOVE)))
		return;
       atmel_ts_finger_data( &ts->finger_data[id], data );
	
	ts->finger_data[id].status = status & T9_MSG_STATUS_MOVE ? T9_MSG_STATUS_MOVE : T9_MSG_STATUS_PRESS;   
	atmel_ts_input_report(ts,(uint8_t*)&data[0],id); 

}

/**********************************************************************************
** atmel_ts_work_report
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** work   input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static void atmel_ts_work_report(struct i2c_client *client) 
{

	ATMEL_TS  * ts = i2c_get_clientdata(client);
	int ret = 0;
	uint8_t data[7] = { 0x00 };
	int8_t  report_type = 0;
	uint8_t loop_i = 0, loop_j = 0 ;
	uint16_t addr;
	memset( data, 0x00, sizeof( data ));
	ret = i2c_atmel_read( ts->client, get_object_address( ts, GEN_MESSAGEPROCESSOR_T5 ),(uint8_t*)&data[0], 7 ); 

#ifdef  ARIMA_PCBA_RELEASE		 
	printk("\n  0x%2.2X 0x%2.2X 0x%2.2X 0x%2.2X 0x%2.2X 0x%2.2X 0x%2.2X\n",data[0],data[1],data[2],data[3],data[4],data[5],data[6]);	
#endif

	 if(verifyingCal==TRUE && data[0] == 0x01  ){  //T6 Message
		Timestamp = 0;
	 }
	dbg_printk("\n ts->finger_type:%d",ts->finger_type); 
	
	report_type = data[MSG_RID] - ts->finger_type;
	if(( report_type >= 0 ) && ( report_type < MXT_MAX_FINGER )) 
	{
			/* Touch fingers */
		dbg_printk("\n atmel_ts_input_touchevent..."); 		
		atmel_ts_input_touchevent(ts,(uint8_t*)&data[0],report_type);
	}	
	else  {
		if(( data[MSG_RID] == 0x0C ) && ( data[1] == 0x80 )) /* Softkey press */
		{
			dbg_printk("\n Ready to execute one of the 4 softkeys function- \n" );
			switch( data[2] )
			{
			      case MXT_KEY_SEARCH:
			      {
			            input_report_key( tpd->dev, KEY_SEARCH, 1 );
			            softkeys_status = softkeys_status | ( MXT_KEY_SEARCH );
			            dbg_printk("\n report search key\n" );
			      } 
			   	break;
			      case MXT_KEY_BACK:
			      {
			            input_report_key( tpd->dev, KEY_BACK, 1 );
			            softkeys_status = softkeys_status | ( MXT_KEY_BACK );
			            dbg_printk( "\n >>>Report back key\n" );
						
			      } 
			   	break;
			      case MXT_KEY_HOME:
			      {        	
			            input_report_key( tpd->dev, KEY_HOMEPAGE, 1 ); //CWTEST
			            softkeys_status = softkeys_status | ( MXT_KEY_HOME );
			            dbg_printk("\n >>>Report home key \n");
			      }
			   	break;
			      case MXT_KEY_MENU:
			      {
			//<2012/12/19-percyhong, add the menu to change the action of the recent key
			            //<CWB013, [Hawk35] Modified the menu hardkey to recents app, and make the options softkey appears
			            input_report_key( tpd->dev, KEY_MENU, 1 );
			//				            input_report_key( tpd->dev, KEY_APP_SWITCH, 1 );
			            //>CWB013
			//>2012/12/19-percyhong
			            softkeys_status = softkeys_status | ( MXT_KEY_MENU );
			            dbg_printk( "\n >>>Report menu key\n" );
			      }
			   	break;
			      default:
			      {
			        	     dbg_printk("\n >>>Error of the softkey function\n" );
			      } 
			  	break;
			} /* End.. switch( data[2] ) */

			input_sync( tpd->dev );
			//mt65xx_eint_unmask( CUST_EINT_TOUCH_PANEL_NUM );
			 return;
		}
		else if(( data[MSG_RID] == 0x0C ) && ( data[1] == 0x00 )) /* Softkey release */
		{
			dbg_printk("\n status of softkeys before up:0x%x\n", softkeys_status );
			if( softkeys_status & MXT_KEY_SEARCH )
			{
			      input_report_key( tpd->dev, KEY_SEARCH, 0 );
			      softkeys_status = softkeys_status & (~MXT_KEY_SEARCH)/*0xFE*/;
			}
			else if( softkeys_status & MXT_KEY_BACK )
			{
			      input_report_key( tpd->dev, KEY_BACK, 0 );//CWTEST
			      softkeys_status = softkeys_status & (~MXT_KEY_BACK)/*0xFD*/; 
			}
			else if( softkeys_status & MXT_KEY_HOME )
			{
			      input_report_key( tpd->dev, KEY_HOMEPAGE, 0 ); //CWTEST
			      softkeys_status = softkeys_status & (~MXT_KEY_HOME)/*0xFB*/;
			}
			else if( softkeys_status & MXT_KEY_MENU )
			{
			//<2012/12/19-percyhong, add the menu to change the action of the recent key
				  //<CWB013, [Hawk35] Modified the menu hardkey to recents app, and make the options softkey appears
			      input_report_key(tpd->dev,KEY_MENU , 0);
			//			          input_report_key(tpd->dev,KEY_APP_SWITCH , 0);					  
			      //>CWB013
			//>2012/12/19-percyhong
			      softkeys_status = softkeys_status & (~MXT_KEY_MENU)/*0xF7*/;
			}
			input_sync( tpd->dev );
			//mt65xx_eint_unmask( CUST_EINT_TOUCH_PANEL_NUM ); //enable_irq( ts->client->irq );
			return;
		}
	}	  
	//mt65xx_eint_unmask( CUST_EINT_TOUCH_PANEL_NUM ); //enable_irq( ts->client->irq );

}	



/**********************************************************************************
** atmel_ts_probe
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** client   input.
** id       input
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static int atmel_ts_probe(struct i2c_client *client,
       const struct i2c_device_id *id)
{
  ATMEL_TS    * ts = NULL;
  ATMEL_PLAT  * pdata = NULL;
  struct i2c_msg msg[2];
  int       ret = 0, err = 0, i = 0, intr = GPIO_CTP_EINT_PIN;
  uint8_t   loop_i = 0;
  uint8_t   data[16] = { 0x00 };
  uint8_t   C_check = 0;
  

  client->timing  = I2C_MASTER_CLOCK; /* SCL clock frequency. 100~400 KHz */
  client->addr   |= I2C_ENEXT_FLAG ; 
  client->irq     = GPIO_CTP_EINT_PIN;
  mxt_i2c_client  = client;
   
  	
  pdata = client->dev.platform_data;
  if( NULL != pdata )
  {
    pdata->gpio_irq = GPIO_CTP_EINT_PIN;
  }

  tpd_hwPower_ctrl( TPD_HWPWM_ON );
  msleep( 50 );
 


/** Hardware GPIO reset **/
  mt_set_gpio_mode( GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO );
  mt_set_gpio_dir( GPIO_CTP_RST_PIN, GPIO_DIR_OUT );
  mt_set_gpio_out( GPIO_CTP_RST_PIN, GPIO_OUT_ONE );
  msleep( 5 );
  mt_set_gpio_out( GPIO_CTP_RST_PIN, GPIO_OUT_ZERO );
  msleep( 40 );
  mt_set_gpio_out( GPIO_CTP_RST_PIN, GPIO_OUT_ONE );

  mt_set_gpio_mode( GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT );
  msleep( 60 );

  if( !i2c_check_functionality( client->adapter, I2C_FUNC_I2C ))
  {
    dbg_printk("%s: need I2C_FUNC_I2C\n", __func__ );
    ret = -ENODEV;
    goto  err_check_functionality_failed;
  }

  ts = kzalloc( sizeof( ATMEL_TS ), GFP_KERNEL );
  if( NULL == ts )
  {
    dbg_printk("%s: allocate atmel_ts_data failed\n", __func__ );
    ret = -ENOMEM;
    goto  err_alloc_data_failed;
  }
  memset( ts, 0x00, sizeof( ATMEL_TS ));

  thread = kthread_run( tpd_event_handler, 0, ATMEL_MXT_TPD_NAME );
  if( IS_ERR( thread ))
  {
    err = PTR_ERR( thread );
    pr_info( "[ATMEL]Failed to create kernel thread: %d\n", err );
  }
  //<2013/2/21--jtao.lee, Adjust TP auto detect
  //tpd_load_status = 1;
//mdelay( 100 );
//>2013/2/21--jtao.lee
  
  ts->client = client;
  i2c_set_clientdata( client, ts );

  /* Read the info block data. */
  ts->id = kzalloc( sizeof( struct info_id_t ), GFP_KERNEL );
  if( NULL == ts->id )
  {
    dbg_printk("%s: allocate info_id_t failed\n", __func__ );
    goto  err_alloc_failed;
  }
  mdelay( 800 );
  
  ret=atmel_ts_get_info(ts);

  if(ret == (-EINVAL) )
  	goto err_detect_failed;
  else if(ret == (-ENOMEM) )
  	goto err_alloc_data_failed;
  
//<2013/2/21--jtao.lee, Adjust TP auto detect
tpd_load_status = 1;
mdelay( 100 );
//>2013/2/21--jtao.lee

  if(atmel_ts_CRC_judge(ts)==CRC_ERROR)
	atmel_ts_init(ts);
  
 
  set_bit( EV_SYN, tpd->dev->evbit );
  set_bit( EV_KEY, tpd->dev->evbit );
  set_bit( EV_ABS, tpd->dev->evbit );
  set_bit( BTN_TOUCH, tpd->dev->keybit );
  set_bit( KEY_BACK, tpd->dev->keybit );
  set_bit( KEY_HOMEPAGE, tpd->dev->keybit );
//<2012/12/19-percyhong, add the menu to change the action of the recent key
  //<CWB013, [Hawk35] Modified the menu hardkey to recents app, and make the options softkey appears
  set_bit( KEY_MENU, tpd->dev->keybit );
//  set_bit( KEY_APP_SWITCH, tpd->dev->keybit );
  //>CWB013
//>2012/12/19-percyhong
  set_bit( KEY_SEARCH, tpd->dev->keybit );
  
   
  input_set_abs_params(tpd->dev, ABS_X, 0, MXT_MAX_XC, 0, 0);
  input_set_abs_params(tpd->dev, ABS_Y, 0, MXT_MAX_YC, 0, 0);
  
  input_set_abs_params( tpd->dev, ABS_MT_POSITION_X, 0, MXT_MAX_XC, 0, 0 );
  input_set_abs_params( tpd->dev, ABS_MT_POSITION_Y, 0, MXT_MAX_YC, 0, 0 );
  input_set_abs_params( tpd->dev, ABS_MT_TOUCH_MAJOR, 0, MXT_MAX_AREA, 0, 0 );
    
  mt_eint_set_sens( CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE );
  mt_eint_set_hw_debounce( CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN );
//  mt_eint_registration( CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_TYPE, tpd_eint_interrupt_handler, 1 );
mt65xx_eint_registration( CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_EN,
                            CUST_EINT_TOUCH_PANEL_POLARITY, tpd_eint_interrupt_handler, 1 );
  mt_eint_unmask( CUST_EINT_TOUCH_PANEL_NUM );

  msleep( 100 ); 
  ret = i2c_atmel_read( ts->client, get_object_address( ts, GEN_MESSAGEPROCESSOR_T5 ),
            (uint8_t*)&data[0], 7 );

 /*<<EricHsieh,using the early suspend/resume for touch panel*/
  #if defined(CONFIG_HAS_EARLYSUSPEND)	
  ts->early_suspend.level   = EARLY_SUSPEND_LEVEL_BLANK_SCREEN - 50;
  ts->early_suspend.suspend = atmel_ts_early_suspend;
  ts->early_suspend.resume  = atmel_ts_late_resume;
  register_early_suspend( &ts->early_suspend );
  #endif/*>>EricHsieh,using the early suspend/resume for touch panel*/
  
  private_ts = ts;
#if defined( ATMEL_EN_SYSFS )
  atmel_touch_sysfs_init();
#endif
  dev_info( &client->dev, "Start touchscreen %s in interrupt mode\n",
      tpd->dev->name );
 
#if defined( ATMEL_TDP_CABLE_CTRL )
  usb_register_notifier( &cable_status_handler );
#endif
  printk("\n Atmel mxt224s probe done...........");
  return 0;



//err_input_dev_alloc_failed:
err_alloc_failed:

err_detect_failed:
//destroy_workqueue( ts->atmel_wq );
err_alloc_data_failed:
  if( NULL != ts )
    kfree( ts );


	
err_check_functionality_failed:
  return  ret;
} /* End.. atmel_ts_probe() */

/**********************************************************************************
** atmel_ts_remove
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** client   input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static int atmel_ts_remove(struct i2c_client *client)
{
ATMEL_TS * ts = i2c_get_clientdata( client );

#if defined( ATMEL_EN_SYSFS )
  atmel_touch_sysfs_deinit();
#endif

  kfree( ts );

  return  0;
} /* End.. atmel_ts_remove() */

	   



/**********************************************************************************
** atmel_ts_suspend
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** client   input.
** mesg     input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/


static int atmel_ts_suspend_function(struct i2c_client *client)
{
	 int     ret = 0;
	 uint8_t data[16] = { 0x00 };
	 ATMEL_TS * ts;

	  printk(" \n %s : .............\n", __FUNCTION__);
	  ts= i2c_get_clientdata( client );
	  mt_eint_mask( CUST_EINT_TOUCH_PANEL_NUM ); //disable_irq( client->irq );

	  ret = i2c_atmel_read( ts->client, get_object_address( ts, GEN_MESSAGEPROCESSOR_T5 ), (uint8_t*)&data[0], 7 );
	  i2c_atmel_write_byte_data( client, get_object_address( ts, GEN_POWERCONFIG_T7 ), 0x0 );
	  i2c_atmel_write_byte_data( client, get_object_address( ts, GEN_POWERCONFIG_T7 ) + 1, 0x0 );



  return  0;
}

static int atmel_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
  	  return atmel_ts_suspend_function(client);
} /* End.. atmel_ts_suspend() */

static int atmel_ts_resume_function(struct i2c_client *client)
{

	ATMEL_TS * ts = i2c_get_clientdata(client); 
  	int     vRet = 0;
 	uint8_t ret,data[16] = { 0x00 };
	uint8_t calibration_count,loop_i;
	
  	printk(" \n %s : .............\n", __FUNCTION__);
#ifdef  AUTO_DETECT_PANEL_ID	
	printk("\n Panel_ID=0x%x",Panel_ID);
#endif

 	 
	input_report_key(tpd->dev, BTN_TOUCH, 0);
	input_mt_sync(tpd->dev);
	input_sync(tpd->dev);

	for(loop_i=0;loop_i < MXT_MAX_FINGER ; loop_i++)
		ts->finger_data[loop_i].status = 0; 
	
	atmel_ts_reinit(ts);  

	calibration_count = 0; 
	if(data[0] == 0x01 && (data[1]== 0x10 ||data[1]== 0x90)  ){

		do{
			i2c_atmel_read( client, get_object_address( ts, GEN_MESSAGEPROCESSOR_T5 ),(uint8_t*)&data[0], 7 );
			//printk( "\n Calibration block: 0x%2.2X 0x%2.2X 0x%2.2X 0x%2.2X 0x%2.2X 0x%2.2X 0x%2.2X\n",data[0],data[1],data[2],data[3], data[4],data[5],data[6]);      
			if(data[0] == 0x01 && data[1]== 0x00)
				break;
			calibration_count++;
			msleep(10);
			printk("\n calibration_count=%d",calibration_count);
		}while((calibration_count  <=10));	

	}	
	if(calibration_count > 10)
		printk("\n Calibration not comple........");
	else
		printk("\n Calibration is OK........");
	
	i2c_atmel_write_byte_data(client,get_object_address(ts, GEN_ACQUISITIONCONFIG_T8) + 4, 0x05); //0x05
	i2c_atmel_write_byte_data(client,get_object_address(ts, GEN_ACQUISITIONCONFIG_T8) + 6, 0x00); //0x05
	i2c_atmel_write_byte_data(client,get_object_address(ts, GEN_ACQUISITIONCONFIG_T8) + 7, 0x00); //0x30
  	i2c_atmel_write_byte_data(client,get_object_address(ts, GEN_ACQUISITIONCONFIG_T8) + 8, 0x01); //0x03
	i2c_atmel_write_byte_data(client,get_object_address(ts, GEN_ACQUISITIONCONFIG_T8) + 9, 0x80); //0x88	
	
	verifyingCal=TRUE;
	Timestamp=0;		

	i2c_atmel_write_byte_data(client,get_object_address(ts, GEN_COMMANDPROCESSOR_T6) + 2, 0x01); //Calibration

  	mt_eint_unmask( CUST_EINT_TOUCH_PANEL_NUM ); //enable_irq( client->irq );

	msleep(5);
   	vRet = i2c_atmel_read( client, get_object_address( ts, GEN_MESSAGEPROCESSOR_T5 ),(uint8_t*)&data[0], 7 );
	
  	return  0;

}
static int atmel_ts_resume(struct i2c_client *client)
{
	return atmel_ts_resume_function(client);
  
} /* End.. atmel_ts_resume() */
/*<<EricHsieh,using the early suspend/resume for touch panel*/
#if defined( CONFIG_HAS_EARLYSUSPEND )
/**********************************************************************************
** atmel_ts_early_suspend
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** h    input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static void atmel_ts_early_suspend(struct early_suspend *h)
{
	ATMEL_TS * ts = container_of( h, struct atmel_ts_data, early_suspend );

  	printk( ">>>>> [ATMEL] %s\n", __func__ );
  	atmel_ts_suspend(mxt_i2c_client, PMSG_SUSPEND );
} /* End.. atmel_ts_early_suspend() */

/**********************************************************************************
** atmel_ts_late_resume
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** h    input.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static void atmel_ts_late_resume(struct early_suspend *h)
{
	ATMEL_TS * ts = container_of( h, struct atmel_ts_data, early_suspend );

  	pr_info( ">>>>> [ATMEL] %s\n", __func__ );
  	atmel_ts_resume(mxt_i2c_client);
} /* End.. atmel_ts_late_resume() */

#endif /* End.. (CONFIG_HAS_EARLYSUSPEND) */
/*>>EricHsieh,using the early suspend/resume for touch panel*/


/**********************************************************************************
** atmel_ts_detect
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** client   input
** kind     input
** info     input
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The result for setting.
**  - 0: Success.
**  - Other: Fail.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static int atmel_ts_detect(struct i2c_client *client,int kind,struct i2c_board_info *pInfo)
{
  strcpy( pInfo->type, ATMEL_MXT_TPD_NAME );
  return  0;
} /** End.. atmel_ts_detect() **/

/**********************************************************************************
** tpd_eint_interrupt_handler
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
**
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The result for setting.
**
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static void tpd_eint_interrupt_handler(void)
{
  TPD_DEBUG_PRINT_INT;
  tpd_flag = 1;
  wake_up_interruptible( &waiter );
}  /** End.. tpd_eint_interrupt_handler() **/

/**********************************************************************************
** tpd_event_handler
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
**
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The result for setting.
**  - 0: Success.
**  - Other: Fail.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static int tpd_event_handler(void *unused)
{
struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };

  sched_setscheduler( current, SCHED_RR, &param );
  do
  {
    set_current_state( TASK_INTERRUPTIBLE );
    wait_event_interruptible( waiter, tpd_flag != 0 );

    //mt65xx_eint_mask( CUST_EINT_TOUCH_PANEL_NUM );
    tpd_flag = 0;
    TPD_DEBUG_SET_TIME;

    set_current_state( TASK_RUNNING );
   
    atmel_ts_work_report( mxt_i2c_client );
   	
  } while( !kthread_should_stop());

  return 0;
} /** End.. tpd_event_handler() **/

/**********************************************************************************
** tpd_suspend
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
**
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The result for setting.
**  - 0: Success.
**  - Other: Fail.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static void tpd_suspend(struct early_suspend *h)
{
  pr_info( ">>>>> [ATMEL] %s\n", __func__ );
  atmel_ts_suspend( mxt_i2c_client, PMSG_SUSPEND );
} /** End.. tpd_suspend() **/

/**********************************************************************************
** tpd_resume
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** None.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The result for setting.
**  - 0: Success.
**  - Other: Fail.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static void tpd_resume(struct early_suspend *h)
{
  pr_info( ">>>>> [ATMEL] %s\n", __func__ );
  atmel_ts_resume( mxt_i2c_client );
} /** End.. tpd_suspend() **/

/**********************************************************************************
** tpd_local_init
**---------------------------------------------------------------------------------
**
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** None.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The result for setting.
**  - 0: Success.
**  - Other: Fail.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static int tpd_local_init(void)
{
  pr_info(">>>>>> ATMEL TPD local init\n");

  if( i2c_add_driver( &atmel_i2c_ts_driver ) != 0 )
  {
    pr_info(">>>>>> unable to add ATMEL i2c driver.\n");
    return  -1;
  }

  TPD_DMESG( ">>>>>> End %s, %d\n", __FUNCTION__, __LINE__ );
  tpd_type_cap = 1;

  return  0;
} /* End.. tpd_local_init() */

/**********************************************************************************
** tpd_driver_init
**---------------------------------------------------------------------------------
** The function was called when loaded into kernel for module initialization.
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** None.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** The result for setting.
**  - 0: Success.
**  - Other: Fail.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static int __init tpd_driver_init( void )
{
  printk( KERN_INFO ">>>>> ATMEL mXT224S touch panel driver init <<<<<\n" );

  i2c_register_board_info( ATMEL_I2C_CHANNEL_ID, &atmel_mxt_i2c_tpd, 1 );
  if( tpd_driver_add( &tpd_device_driver ) < 0 )
    TPD_DMESG( "Add the mXT224S driver failed\n" );

  return  0;
} /** End.. tpd_driver_init() **/
  
/**********************************************************************************
** tpd_driver_exit
**---------------------------------------------------------------------------------
** The function for module exit.
** It should never be called.
**=================================================================================
** Parameter
**---------------------------------------------------------------------------------
** None.
**=================================================================================
** Return value
**---------------------------------------------------------------------------------
** None.
**=================================================================================
** History
**---------------------------------------------------------------------------------
** 2013/1/21   Albert Wu
**  Creation.
***********************************************************************************/
static void __exit tpd_driver_exit( void )
{
  TPD_DMESG( ">>>>> ATMEL mXT224S touch panel driver exit <<<<<\n" );
  tpd_driver_remove( &tpd_device_driver );
} /** End.. tpd_driver_exit() **/

/**********************************************************************************
** Module description
***********************************************************************************/
module_init( tpd_driver_init );
module_exit( tpd_driver_exit );

MODULE_DESCRIPTION("ATMEL Touchscreen Driver");
MODULE_LICENSE("GPL");

/**********************************************************************************
** Ending
***********************************************************************************/
#undef _MXT224S_DRIVER_C_
#endif /** End.. (_MXT224S_DRIVER_C_) **/
