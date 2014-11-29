/***************************************************************************
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *    File  	: mediatek\custom\common\kernel\alsps\apds9130.c
 *    Author(s)   :  Kang Jun Mo < junmo.kang@lge.com >
 *    Description :
 *
 ***************************************************************************/

/****************************************************************************
* Include Files
****************************************************************************/
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>

#ifdef MT6516
#include <mach/mt6516_devs.h>
#include <mach/mt6516_typedefs.h>
#include <mach/mt6516_gpio.h>
#include <mach/mt6516_pll.h>
#endif

#ifdef MT6573
#include <mach/mt6573_devs.h>
#include <mach/mt6573_typedefs.h>
#include <mach/mt6573_gpio.h>
#include <mach/mt6573_pll.h>
#endif

#include <mach/mt_devs.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>

#ifdef MT6516
#define POWER_NONE_MACRO MT6516_POWER_NONE
#endif

#ifdef MT6573
#define POWER_NONE_MACRO MT65XX_POWER_NONE
#endif

#define POWER_NONE_MACRO MT65XX_POWER_NONE

#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include <asm/io.h>
#include <cust_eint.h>
#include <cust_alsps.h>
#include "apds9130.h"

/****************************************************************************
* Manifest Constants / Defines
****************************************************************************/
#define APDS9130_DEV_NAME     "APDS9130"

#define APDS9130_ENABLE_REG 0x00
#define APDS9130_PTIME_REG  0x02
#define APDS9130_WTIME_REG  0x03
#define APDS9130_PILTL_REG  0x08
#define APDS9130_PILTH_REG  0x09
#define APDS9130_PIHTL_REG  0x0A
#define APDS9130_PIHTH_REG  0x0B
#define APDS9130_PERS_REG 0x0C
#define APDS9130_CONFIG_REG 0x0D
#define APDS9130_PPCOUNT_REG  0x0E
#define APDS9130_CONTROL_REG  0x0F
#define APDS9130_REV_REG  0x11
#define APDS9130_ID_REG   0x12
#define APDS9130_STATUS_REG 0x13
#define APDS9130_PDATAL_REG 0x18
#define APDS9130_PDATAH_REG 0x19

#define CMD_BYTE  0x80
#define CMD_WORD  0xA0
#define CMD_SPECIAL 0xE0

#define CMD_CLR_PS_INT  0xE5
#define CMD_CLR_ALS_INT 0xE6
#define CMD_CLR_PS_ALS_INT  0xE7

#define APDS9130_PINT 0x20
#define APDS9130_PVALID 0x02

#define APDS9130_PSAT 0x40  /* PS saturation bit check */

/****************************************************************************
 * Macros
 ****************************************************************************/
#define APS_DEBUG

#define APS_TAG "[ALS/PS] "

#define APS_ERR(fmt, args...)    printk(KERN_ERR APS_TAG"[ERROR] %s() line=%d : "fmt, __FUNCTION__, __LINE__, ##args)

#if defined ( APS_DEBUG )
/* You need to select proper loglevel to see the log what you want. ( Currently, you can't see "KERN_INFO" level ) */
#define APS_FUN(f)  	   printk(KERN_ERR APS_TAG"%s()\n", __FUNCTION__)
#define APS_LOG(fmt, args...)    printk(KERN_ERR APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)    printk(KERN_INFO APS_TAG fmt, ##args)
#else
#define APS_FUN(f)  	   printk(KERN_INFO APS_TAG"%s()\n", __FUNCTION__)
#define APS_LOG(fmt, args...)    printk(KERN_INFO APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)    printk(KERN_INFO APS_TAG fmt, ##args)
#endif

/****************************************************************************
* Type Definitions
****************************************************************************/
typedef enum
{
	PS_NEAR = 0,
	PS_FAR = 1,
	PS_UNKNOWN = 2
} PS_STATUS;

struct apds9130_priv
{
	struct i2c_client *client;
	struct work_struct eint_work;

	unsigned int activate; /* 1 = activate, 0 = deactivate */

	/* variables to store APDS9130 register value - begin */
	unsigned int enable;
	unsigned int atime;
	unsigned int ptime;
	unsigned int wtime;
	unsigned int ailt;
	unsigned int aiht;
	unsigned int pilt;
	unsigned int piht;
	unsigned int pers;
	unsigned int config;
	unsigned int ppcount;
	unsigned int control;
	/* variables to store APDS9130 register value - end */

	/* CAUTION : in case of strong sunlight, ps_state is not same as ps_th_status. */
	unsigned int ps_status; /* current status of poximity detection : 0 = near, 1 = far */
	unsigned int ps_th_status; /* current threshold status of poximity detection : 0 = near, 1 = far */

	/* threshold value to detect "near-to-far" event */
	unsigned int ps_th_near_low;
	unsigned int ps_th_near_high;

	/* threshold value to detect "far-to-near" event */
	unsigned int ps_th_far_low;
	unsigned int ps_th_far_high;

	unsigned int ps_cross_talk; /* a result value of calibration. it will be used to compensate threshold value. */

	#if defined(CONFIG_HAS_EARLYSUSPEND)
	struct early_suspend early_drv;
	#endif
	
};


/****************************************************************************
* Variables
****************************************************************************/
static struct i2c_client *apds9130_i2c_client = NULL; /* for general file I/O service. will be init on apds9130_i2c_probe() */
static struct apds9130_priv *g_apds9130_ptr = NULL; /* for interrupt service call. will be init on apds9130_i2c_probe() */
static struct platform_driver apds9130_alsps_driver;

/****************************************************************************
* Extern Function Prototypes
****************************************************************************/
extern void mt_eint_unmask ( unsigned int line );
extern void mt_eint_mask ( unsigned int line );
extern void mt_eint_set_polarity ( unsigned int eint_num, unsigned int pol );
extern void mt_eint_set_hw_debounce ( unsigned int eint_num, unsigned int ms );
extern unsigned int mt_eint_set_sens ( unsigned int eint_num, unsigned int sens );
extern void mt_eint_registration ( unsigned int eint_num, unsigned int flag, void ( EINT_FUNC_PTR ) ( void ), unsigned int is_auto_umask );

#ifdef MT6516
extern void MT6516_EINTIRQUnmask ( unsigned int line );
extern void MT6516_EINTIRQMask ( unsigned int line );
extern void MT6516_EINT_Set_Polarity ( kal_uint8 eintno, kal_bool ACT_Polarity );
extern void MT6516_EINT_Set_HW_Debounce ( kal_uint8 eintno, kal_uint32 ms );
extern kal_uint32 MT6516_EINT_Set_Sensitivity ( kal_uint8 eintno, kal_bool sens );
extern void MT6516_EINT_Registration ( kal_uint8 eintno, kal_bool Dbounce_En, kal_bool ACT_Polarity, void ( EINT_FUNC_PTR ) ( void ),
									   kal_bool auto_umask );
#endif

/****************************************************************************
* Local Function Prototypes
****************************************************************************/
void apds9130_eint_func ( void );


/****************************************************************************
* Local Functions
****************************************************************************/

//==========================================================
// Platform(AP) dependent functions
//==========================================================
static void apds9130_setup_eint ( void )
{
	APS_FUN ();

	/* Configure GPIO settings for external interrupt pin  */
	mt_set_gpio_dir ( GPIO_ALS_EINT_PIN, GPIO_DIR_IN );
	mt_set_gpio_mode ( GPIO_ALS_EINT_PIN, GPIO_ALS_EINT_PIN_M_EINT );
	mt_set_gpio_pull_enable ( GPIO_ALS_EINT_PIN, TRUE );
	mt_set_gpio_pull_select ( GPIO_ALS_EINT_PIN, GPIO_PULL_UP );

	/* Configure external interrupt settings for external interrupt pin */
	mt_eint_set_sens ( CUST_EINT_ALS_NUM, CUST_EINT_ALS_SENSITIVE );
	mt_eint_set_polarity ( CUST_EINT_ALS_NUM, CUST_EINT_ALS_POLARITY );
	mt_eint_set_hw_debounce ( CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN );

	mt_eint_registration ( CUST_EINT_ALS_NUM, CUST_EINT_ALS_TYPE, apds9130_eint_func, 0);

	/* Mask external interrupt to avoid un-wanted interrupt. Unmask it after initialization of APDS9130 */
	mt_eint_mask ( CUST_EINT_ALS_NUM );
}

static void apds9130_main_power ( struct alsps_hw *hw, unsigned int on )
{
	static unsigned int main_power_on = 0xFF;

	APS_FUN ();

	if ( main_power_on != on )
	{
		if ( on )
		{
			if ( !hwPowerOn ( hw->power_id, hw->power_vol, "APDS9130" ) )
			{
				APS_ERR ( "failed to power on ( VCAM_AF )\n" );
				goto EXIT_ERR;
			}

			APS_LOG( "turned on the power ( VCAM_AF )" );
		}
		else
		{
			if ( !hwPowerDown ( hw->power_id, "APDS9130" ) )
			{
				APS_ERR ( "failed to power down ( VCAM_AF )\n" );
				goto EXIT_ERR;
			}

			APS_LOG( "turned off the power ( VCAM_AF )" );
		}

		main_power_on = on;
	}

	EXIT_ERR:
		return;
}

static void apds9130_led_power ( unsigned int on )
{
	static unsigned int led_power_on = 0xFF;

	APS_FUN ();

	if ( led_power_on != on )
	{
		if ( on )
		{
			if ( !hwPowerOn ( MT65XX_POWER_LDO_VCAMA, VOL_2800, "APDS9130LEDA" ) )
			{
				APS_ERR ( "failed to power on ( VCAMA )\n" );
				goto EXIT_ERR;
			}

			APS_LOG("turned on the power ( VCAMA )");
		}
		else
		{
			if ( !hwPowerDown ( MT65XX_POWER_LDO_VCAMA, "APDS9130LEDA" ) )
			{
				APS_ERR ( "failed to power down ( VCAMA )\n" );
				goto EXIT_ERR;
			}

			APS_LOG("turned off the power ( VCAMA )\n");
		}

		led_power_on = on;
	}

	EXIT_ERR:
		return;
}

//==========================================================
// APDS9130 Register Read / Write Funtions
//==========================================================
static int apds9130_write_cmd ( struct i2c_client *client, u8 val )
{
	int res = 0;

	res = i2c_master_send ( client, &val, 1 );
	if ( res == 1 )
	{
		APS_DBG ( "I2C write ( val=0x%02x )\n", val );
		return APDS9130_SUCCESS;
	}
	else
	{
		APS_ERR ( "failed to write to APDS9130 ( err=%d, cmd=0x%02x )\n", res, val );
		return APDS9130_ERR_I2C;
	}
}

static int apds9130_write_byte ( struct i2c_client *client, u8 reg, u8 val )
{
	int res = 0;
	u8 pBuf[2] = { 0 };

	pBuf[0] = reg;
	pBuf[1] = val;

	res = i2c_master_send ( client, pBuf, 2 );
	if ( res == 2 )
	{
		APS_DBG ( "I2C write ( reg=0x%02x, val=0x%02x )\n", reg, val );
		return APDS9130_SUCCESS;
	}
	else
	{
		APS_ERR ( "failed to write to APDS9130 ( err=%d, reg=0x%02x, val=0x%02x )\n", res, reg, val );
		return APDS9130_ERR_I2C;
	}

}

static int apds9130_write_word ( struct i2c_client *client, u8 reg, u16 val )
{
	int res = 0;
	u8 pBuf[3] = { 0 };

	pBuf[0] = reg ;
	pBuf[1] = val & 0xFF ;
	pBuf[2] = ( val >> 8 ) & 0xFF ;

	res = i2c_master_send ( client, pBuf, 3 );
	if ( res == 3 )
	{
		APS_DBG ( "I2C write ( reg=0x%02x, val=0x%04x )\n", reg, val );
		return APDS9130_SUCCESS;
	}
	else
	{
		APS_ERR ( "failed to write to APDS9130 ( err=%d, reg=0x%02x, val=0x%04x )\n", res, reg, val );
		return APDS9130_ERR_I2C;
	}

}

static int apds9130_read_byte ( struct i2c_client *client, u8 reg, u8 *pVal )
{
	int res = 0;

	if ( pVal == NULL )
	{
		APS_ERR ( "invalid input ( pVal=NULL )" );
		goto EXIT_ERR;
	}

	res = i2c_master_send ( client, &reg, 1 );
	if ( res != 1 )
	{
		goto EXIT_ERR;
	}

	res = i2c_master_recv ( client, pVal, 1 );
	if ( res != 1 )
	{
		goto EXIT_ERR;
	}

	APS_DBG ( "I2C read ( reg=0x%02x, val=0x%02x )\n", reg, *pVal );
	return APDS9130_SUCCESS;

	EXIT_ERR:
	APS_ERR ( "failed to read from APDS9130 ( err=%d, reg=0x%02x )\n", res, reg );
	return APDS9130_ERR_I2C;
	
}

static int apds9130_read_word ( struct i2c_client *client, u8 reg, u16 *pVal )
{
	int res = 0;
	u8 pBuf[2] = { 0 };

	if ( pVal == NULL )
	{
		APS_ERR ( "invalid input ( pVal=NULL )" );
		goto EXIT_ERR;
	}

	res = i2c_master_send ( client, &reg, 1 );
	if ( res != 1 )
	{
		goto EXIT_ERR;
	}

	res = i2c_master_recv ( client, pBuf, 2 );
	if ( res != 2 )
	{
		goto EXIT_ERR;
	}

	*pVal = ( ( u16 ) pBuf[1] << 8 ) | pBuf[0] ;

	APS_DBG ( "I2C read ( reg=0x%02x, val=0x%04x )\n", reg, *pVal );
	return APDS9130_SUCCESS;

	EXIT_ERR:
	APS_ERR ( "failed to read from APDS9130 ( err=%d, reg=0x%02x )\n", res, reg );
	return APDS9130_ERR_I2C;
	
}

//==========================================================
// APDS9130 Basic Read / Write Funtions
//==========================================================
static int apds9130_set_enable ( struct i2c_client *client, int enable )
{
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	int res = 0;

	res = apds9130_write_byte ( client, CMD_BYTE | APDS9130_ENABLE_REG, ( u8 ) enable );
	if ( res == APDS9130_SUCCESS )
	{
		obj->enable = enable;
	}

	return res;
}

static int apds9130_set_ptime ( struct i2c_client *client, int ptime )
{
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	int res = 0;

	res = apds9130_write_byte ( client, CMD_BYTE | APDS9130_PTIME_REG, ( u8 ) ptime );
	if ( res == APDS9130_SUCCESS )
	{
		obj->ptime = ptime;
	}

	return res;
}

static int apds9130_set_wtime ( struct i2c_client *client, int wtime )
{
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	int res = 0;

	res = apds9130_write_byte ( client, CMD_BYTE | APDS9130_WTIME_REG, ( u8 ) wtime );
	if ( res == APDS9130_SUCCESS )
	{
		obj->wtime = wtime;
	}

	return res;
}

static int apds9130_set_pilt ( struct i2c_client *client, int threshold )
{
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	int res = 0;

	res = apds9130_write_word ( client, CMD_WORD | APDS9130_PILTL_REG, ( u16 ) threshold );
	if ( res == APDS9130_SUCCESS )
	{
		obj->pilt = threshold;
	}

	return res;
}

static int apds9130_set_piht ( struct i2c_client *client, int threshold )
{
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	int res = 0;

	res = apds9130_write_word ( client, CMD_WORD | APDS9130_PIHTL_REG, ( u16 ) threshold );
	if ( res == APDS9130_SUCCESS )
	{
		obj->piht = threshold;
	}

	return res;
}

static int apds9130_set_pers ( struct i2c_client *client, int pers )
{
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	int res = 0;

	res = apds9130_write_byte ( client, CMD_BYTE | APDS9130_PERS_REG, ( u8 ) pers );
	if ( res == APDS9130_SUCCESS )
	{
		obj->pers = pers;
	}

	return res;
}

static int apds9130_set_config ( struct i2c_client *client, int config )
{
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	int res = 0;

	res = apds9130_write_byte ( client, CMD_BYTE | APDS9130_CONFIG_REG, ( u8 ) config );
	if ( res == APDS9130_SUCCESS )
	{
		obj->config = config;
	}

	return res;
}

static int apds9130_set_ppcount ( struct i2c_client *client, int ppcount )
{
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	int res = 0;

	res = apds9130_write_byte ( client, CMD_BYTE | APDS9130_PPCOUNT_REG, ( u8 ) ppcount );
	if ( res == APDS9130_SUCCESS )
	{
		obj->ppcount = ppcount;
	}

	return res;
}

static int apds9130_set_control ( struct i2c_client *client, int control )
{
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	int res = 0;

	res = apds9130_write_byte ( client, CMD_BYTE | APDS9130_CONTROL_REG, ( u8 ) control );
	if ( res == APDS9130_SUCCESS )
	{
		obj->control = control;
	}

	return res;
}

static int apds9130_get_status ( struct i2c_client *client, int *pData )
{
	int res = 0;

	res = apds9130_read_byte ( client, CMD_BYTE | APDS9130_STATUS_REG, ( u8 * ) pData );
	if ( res == APDS9130_SUCCESS )
	{
		APS_LOG ( "STATUS=0x%02x\n", ( u8 ) * pData );
	}

	return res;
}

static int apds9130_get_pdata ( struct i2c_client *client, int *pData )
{
	int res = 0;

	res = apds9130_read_word ( client, CMD_WORD | APDS9130_PDATAL_REG, ( u16 * ) pData );
	if ( res == APDS9130_SUCCESS )
	{
		APS_DBG ( "PDATA=%u\n", ( u16 ) * pData );
	}

	return res;
}

//                                                              
static int apds9130_get_deivceid( struct i2c_client *client, int *pData )
{
	int res = 0;

	res = apds9130_read_byte ( client, CMD_BYTE | APDS9130_ID_REG, ( u8* ) pData );
	if ( res == APDS9130_SUCCESS )
	{
		APS_DBG ( "DEVICEID=0x%02x\n", ( u8 ) * pData );
	}

	return res;
}
//                                                              


static int apds9130_clear_interrupt ( struct i2c_client *client )
{
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	int res = 0;

	res = apds9130_write_cmd ( client, ( u8 ) CMD_CLR_PS_ALS_INT );
	if ( res == APDS9130_SUCCESS )
	{
		APS_DBG ( "APDS9130 interrupte was cleared\n" );
	}

	return res;
}

//==========================================================
// APDS9130 Data Processign Funtions
//==========================================================
static int apds9130_decide_ps_state ( struct i2c_client *client, int pdata, int int_status )
{
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	int ps_status = obj->ps_status;

	if ( obj->ps_status == PS_FAR )
	{
		if ( ( pdata >= obj->ps_th_far_high ) && ( ( int_status & APDS9130_PSAT ) != APDS9130_PSAT ) )
		{
			ps_status = PS_NEAR;
			APS_LOG ( "PS = NEAR\n" );
		}
		else
		{
			APS_ERR ( "Unknown Event State\n" );
		}
	}
	else
	{
		if ( pdata <= obj->ps_th_near_low )
		{
			ps_status = PS_FAR;
			APS_LOG ( "PS = FAR\n" );
		}
		else
		{
			APS_ERR ( "Unknown Event State\n" );
		}
	}

	return ps_status;
	
}

static long apds9130_initialize ( struct i2c_client *client  )
{
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	int res = 0;
	int id = 0;

	res = apds9130_read_byte ( client, CMD_BYTE | APDS9130_ID_REG, ( u8 * ) &id );
	if ( res != APDS9130_SUCCESS )
	{
		APS_ERR ( "failed to read Device ID and it means I2C error happened\n", ( u8 ) id );
		return res;
	}
	
	APS_LOG ( "APDS9130 Device ID = 0x%02x\n", ( u8 ) id );

	obj->activate = 0;

	/* disable proximity */
	apds9130_set_enable ( client, 0 );

	/* initialize registers of proximity */
	apds9130_set_ptime ( client, 0xFF ); /* 2.72ms Prox integration time */
	apds9130_set_wtime ( client, 0xDC ); /* 100ms Wait time */
	apds9130_set_pers ( client, 0x20 ); /* 2 consecutive Interrupt persistence */
	apds9130_set_config ( client, 0 );
	apds9130_set_ppcount ( client, 0x10 ); /* 16-Pulse for proximity */
	apds9130_set_control ( client, 0x64 ); /* 50mA, IR-diode, 2X PGAIN */

	/* crosstalk value shall be set by LGP Server using I/O so init here to 600 */
    obj->ps_cross_talk = 600;

	/* initialize threshold value of PS */
	if ( obj->ps_cross_talk > 710 )
	{
		obj->ps_cross_talk = 710;
	}
	if ( obj->ps_cross_talk < 0 )
	{
		obj->ps_cross_talk = 0;
	}

	obj->ps_th_far_low = 0;
	obj->ps_th_far_high = 300 + obj->ps_cross_talk;
	obj->ps_th_near_low = obj->ps_th_far_high - 50;
	obj->ps_th_near_high = 1023;

	return res;

}


static long apds9130_enable ( struct i2c_client *client  )
{
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	hwm_sensor_data sensor_data;
	int res = 0;
	int status = 0;
	int pdata = 0;

	/* enable ADC but block interrupt */
	apds9130_set_enable ( client, 0x0D );

	mdelay ( 100 );

	apds9130_get_status( client, &status);

	if ( ( status & APDS9130_PVALID ) == APDS9130_PVALID )
	{
		/* read sensor data */
		apds9130_get_pdata ( client, &pdata );

		/* decide current PS threshold state and set PS thershold to proper range */
		if ( pdata >= obj->ps_th_far_high )
		{
			obj->ps_th_status = PS_NEAR;
			apds9130_set_pilt ( client, obj->ps_th_near_low );
			apds9130_set_piht ( client, obj->ps_th_near_high );
			APS_LOG ( "PS_TH=NEAR\n" );
		}
		else
		{
			obj->ps_th_status = PS_FAR;
			apds9130_set_pilt ( client, obj->ps_th_far_low );
			apds9130_set_piht ( client, obj->ps_th_far_high );
			APS_LOG ( "PS_TH=FAR\n" );
		}

		/* decide current PS status */
		if ( ( pdata >= obj->ps_th_far_high ) && ( (status&0x40) != 0x40 ) )
		{
			obj->ps_status = PS_NEAR;
			APS_LOG ( "PS=NEAR\n" );
		}
		else
		{
			obj->ps_status = PS_FAR;
			APS_LOG ( "PS=FAR\n" );
		}

	}
	else
	{
		APS_ERR("ADC value is invalid so set to PS_FAR\n");

		obj->ps_th_status = PS_FAR;
		obj->ps_status = PS_FAR;
		apds9130_set_pilt ( client, obj->ps_th_far_low );
		apds9130_set_piht ( client, obj->ps_th_far_high );
	}

	/* inform to upper layer ( hwmsen ) */
	sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
	sensor_data.value_divide = 1;
	sensor_data.values[0] = obj->ps_status;
	if ( ( res = hwmsen_get_interrupt_data ( ID_PROXIMITY, &sensor_data ) ) )
	{
		APS_ERR ( "failed to send inform ( err = %d )\n", res );
	}
		
	/* enable APDS9130 */
	res = apds9130_set_enable ( client, 0x2D );
	if ( res == APDS9130_SUCCESS )
	{
		/* unmask external interrupt */
		mt_eint_unmask ( CUST_EINT_ALS_NUM );
		
		APS_LOG ( "APDS9130 was enabled\n" );
		
	}
	else
	{
		APS_ERR ( "failed to enable APDS9130\n" );
	}

	return res;
	
}

static long apds9130_disable ( struct i2c_client *client  )
{
	int res = 0;
	
	/* mask external interrupt */
	mt_eint_mask ( CUST_EINT_ALS_NUM );

	/* disable APDS9130 */
	res = apds9130_set_enable ( client, 0 );
	if ( res == APDS9130_SUCCESS )
	{
		APS_LOG ( "APDS9130 was disabled\n" );
	}
	else
	{
		APS_ERR ( "failed to disable APDS9130\n" );
	}

	return res;
	
}

void apds9130_swap(int *x, int *y)
{
     int temp = *x;
     *x = *y;
     *y = temp;
}

static int apds9130_do_calibration ( struct i2c_client *client, int *value )
{
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	unsigned int sum_of_pdata = 0;
	int temp_pdata[20] = {0};
	int temp_state[20] = {0};
	unsigned int i=0;
	unsigned int j=0;
	unsigned int ArySize = 20;
	unsigned int cal_check_flag = 0;
	unsigned int old_enable = 0;

	apds9130_led_power ( 1 );
	old_enable = obj->enable;

RE_CALIBRATION:
	sum_of_pdata = 0;

	/* Enable PS and Mask interrupt */
	apds9130_set_enable ( client, 0x0D );

	mdelay ( 50 );

	/* Read pdata */
	for ( i = 0 ; i < 20 ; i++ )
	{
		apds9130_get_status ( client, &( temp_state[i] ) );
		apds9130_get_pdata ( client, &( temp_pdata[i] ) );
		mdelay ( 6 );
	}

	#if defined ( APS_DEBUG )
	APS_LOG ( "State Value = " );
	for ( i = 0 ; i < 20 ; i++ )
	{
		APS_LOG ( "%d ", temp_state[i] );
	}
	APS_LOG ( "\n" );
	APS_LOG ( "Read Value = " );
	for ( i = 0 ; i < 20 ; i++ )
	{
		APS_LOG ( "%d ", temp_pdata[i] );
	}
	APS_LOG ( "\n" );
	#endif

	/* sort pdata */
	for ( i = 0 ; i < ArySize - 1 ; i++ )
	{
		for ( j = i + 1 ; j < ArySize ; j++ )
		{
			if ( temp_pdata[i] > temp_pdata[j] )
			{
				apds9130_swap ( temp_pdata+i, temp_pdata+j );
			}
		}
	}

	#if defined(APS_DEBUG)
#if 1 /*                              */
	APS_LOG ( "Read Value = " );
	for ( i = 0 ; i < 20 ; i++ )
	{
		APS_LOG ( "%d ", temp_pdata[i] );
	}
	APS_LOG ( "\n" );
#else
	APS_DBG ( "Read Value = " );
	for ( i = 0 ; i < 20 ; i++ )
	{
		APS_DBG ( "%d ", temp_pdata[i] );
	}
	APS_DBG ( "\n" );
#endif
	#endif

	/* take ten middle data only */
	for ( i = 5 ; i < 15 ; i++ )
	{
		sum_of_pdata = sum_of_pdata + temp_pdata[i];
	}

	/* calculate average */
	obj->ps_cross_talk = sum_of_pdata / 10;
	APS_LOG ( "New calibrated cross talk = %d\n", obj->ps_cross_talk );

	/* check if average is acceptable */
	if ( obj->ps_cross_talk > 710 )
	{
		if ( cal_check_flag == 0 )
		{
			cal_check_flag = 1;
			goto RE_CALIBRATION;
		}
		else
		{
			APS_ERR ( "failed to calibrate cross talk/n" );
			apds9130_set_enable ( client, 0x00 );
			apds9130_set_enable ( client, old_enable );
			*value = obj->ps_cross_talk;
			return -1;
		}
	}

	apds9130_set_enable ( client, 0x00 ); /* Power Off */
	apds9130_set_enable ( client, old_enable );
	apds9130_led_power ( 0 );

	obj->ps_th_far_high = 300 + obj->ps_cross_talk;
	obj->ps_th_near_low = obj->ps_th_far_high - 50;

	/* we should store it to storage ( it should be free from factory reset ) but ATCI Demon will store it through LGP Demon */

	*value = obj->ps_cross_talk;
	return 0;
}

//==========================================================
// APDS9130 General Control Funtions
//==========================================================
static long apds9130_activate ( struct i2c_client *client, int enable )
{
	APS_FUN ();

	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	long res = 0;

	if ( obj->activate != enable )
	{
		if ( enable )
		{
			apds9130_led_power ( 1 );

			res = apds9130_enable ( client );
			if ( res == APDS9130_SUCCESS )
			{
				APS_LOG ( "APDS9130 was enabled\n" );
			}
			else
			{
				APS_ERR ( "failed to enable APDS9130\n" );
			}
		}
		else
		{
			res = apds9130_disable ( client );
			if ( res == APDS9130_SUCCESS )
			{
				APS_LOG ( "APDS9130 was disabled\n" );
				apds9130_led_power ( 0 );
			}
			else
			{
				APS_ERR ( "failed to disable APDS9130\n" );
			}
		}

		if ( res == APDS9130_SUCCESS )
		{
			obj->activate = enable;
		}
		
	}

	return res;
	
}

//==========================================================
// APDS9130 Interrupt Service Routines
//==========================================================
void apds9130_eint_func ( void )
{
	APS_FUN ();
	struct apds9130_priv *obj = g_apds9130_ptr;
	if ( !obj )
	{
		return;
	}
	schedule_work ( &obj->eint_work );
}

static void apds9130_eint_work ( struct work_struct *work )
{
	APS_FUN ();
	struct apds9130_priv *obj = ( struct apds9130_priv * ) container_of ( work, struct apds9130_priv, eint_work );
	struct i2c_client *client = obj->client;
	hwm_sensor_data sensor_data;

	int err;

	int int_status = 0;
	int pdata = 0;
	int new_ps_status = 0;

	APS_LOG ( "External interrupt happened\n" );

	/* read status register */
	apds9130_get_status ( client, &int_status );

	if ( ( int_status & APDS9130_PVALID ) != ( APDS9130_PVALID ) )
	{
		APS_ERR("ADC value is not valid so just skip this interrupt");
		return;
	}

	/* disable ADC first */
	apds9130_set_enable ( client, 0x01 );

	/* read sensor data */
	apds9130_get_pdata ( client, &pdata );

	/* process PS interrupt */
	if ( ( int_status & APDS9130_PINT ) == APDS9130_PINT )
	{
		APS_LOG ( "PS interrupt happened\n" );

		/* change threshold to avoid frequent interrupt */
		if ( obj->ps_th_status == PS_FAR )
		{
			if ( pdata >= obj->ps_th_far_high )
			{
				APS_LOG ( "PS_TH = NEAR\n" );
				obj->ps_th_status = PS_NEAR;
				apds9130_set_pilt ( client, obj->ps_th_near_low );
				apds9130_set_piht ( client, obj->ps_th_near_high );
			}
		}
		else
		{
			if ( pdata <= obj->ps_th_near_low )
			{
				APS_LOG ( "PS_TH = FAR\n" );
				obj->ps_th_status = PS_FAR;
				apds9130_set_pilt ( client, obj->ps_th_far_low );
				apds9130_set_piht ( client, obj->ps_th_far_high );
			}
		}
		
		/* make a decision if it is near or far */
		new_ps_status = apds9130_decide_ps_state( client, pdata, int_status );

		/* inform to upper layer ( hwmsen ), if status was changed */
		if ( new_ps_status != obj->ps_status )
		{
			obj->ps_status = new_ps_status;
			
			sensor_data.status = SENSOR_STATUS_ACCURACY_MEDIUM;
			sensor_data.value_divide = 1;
			sensor_data.values[0] = obj->ps_status;
			if ( ( err = hwmsen_get_interrupt_data ( ID_PROXIMITY, &sensor_data ) ) )
			{
				APS_ERR ( "failed to send inform ( err = %d )\n", err );
			}
		}
		
	}
	
	/* clear interrupt of proximity */
	apds9130_clear_interrupt ( client );

	/* unmask external interrupt */
	mt_eint_unmask ( CUST_EINT_ALS_NUM );

	/* activate proximity */
	apds9130_set_enable ( client, 0x2D );
	
}

//==========================================================
// APDS9130 ADB Shell command function
//==========================================================
static ssize_t apds9130_show_cali_value ( struct device_driver *dev, char *buf )
{
	struct i2c_client *client = apds9130_i2c_client;
	struct apds9130_priv *data = i2c_get_clientdata ( client );

	return sprintf ( buf, "%u\n", data->ps_cross_talk );
}

static ssize_t apds9130_store_cali_value ( struct device_driver *dev, char *buf, size_t count )
{
	struct i2c_client *client = apds9130_i2c_client;
	int ret;
	int data;

	ret = apds9130_do_calibration ( client, &data );

	return count;
}

static ssize_t apds9130_show_ptime ( struct device_driver *dev, char *buf )
{
	struct i2c_client *client = apds9130_i2c_client;
	struct apds9130_priv *data = i2c_get_clientdata ( client );

	return sprintf ( buf, "0x%02x\n", data->ptime );
}

static ssize_t apds9130_store_ptime ( struct device_driver *dev, char *buf, size_t count )
{
	struct i2c_client *client = apds9130_i2c_client;
	unsigned long val = simple_strtoul ( buf, NULL, 10 );
	int ret;

	ret = apds9130_set_ptime ( client, val );

	if ( ret < 0 )
		return ret;

	return count;
}

static ssize_t apds9130_show_wtime ( struct device_driver *dev, char *buf )
{
	struct i2c_client *client = apds9130_i2c_client;
	struct apds9130_priv *data = i2c_get_clientdata ( client );

	return sprintf ( buf, "0x%02x\n", data->wtime );
}

static ssize_t apds9130_store_wtime ( struct device_driver *dev, char *buf, size_t count )
{
	struct i2c_client *client = apds9130_i2c_client;
	unsigned long val = simple_strtoul ( buf, NULL, 10 );
	int ret;

	ret = apds9130_set_wtime ( client, val );

	if ( ret < 0 )
		return ret;

	return count;
}

static ssize_t apds9130_show_pilt ( struct device_driver *dev, char *buf )
{
	struct i2c_client *client = apds9130_i2c_client;
	struct apds9130_priv *data = i2c_get_clientdata ( client );

	return sprintf ( buf, "%d\n", data->pilt );
}

static ssize_t apds9130_store_pilt ( struct device_driver *dev, char *buf, size_t count )
{
	struct i2c_client *client = apds9130_i2c_client;
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	unsigned long val = simple_strtoul ( buf, NULL, 10 );
	int ret;

	ret = apds9130_set_pilt ( client, val );

	if ( ret < 0 )
		return ret;

	obj->ps_th_near_low = val;

	return count;
}

static ssize_t apds9130_show_piht ( struct device_driver *dev, char *buf )
{
	struct i2c_client *client = apds9130_i2c_client;
	struct apds9130_priv *data = i2c_get_clientdata ( client );

	return sprintf ( buf, "%d\n", data->piht );
}

static ssize_t apds9130_store_piht ( struct device_driver *dev, char *buf, size_t count )
{
	struct i2c_client *client = apds9130_i2c_client;
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	unsigned long val = simple_strtoul ( buf, NULL, 10 );
	int ret;

	ret = apds9130_set_piht ( client, val );

	if ( ret < 0 )
		return ret;

	obj->ps_th_far_high = val;

	return count;
}

static ssize_t apds9130_show_pers ( struct device_driver *dev, char *buf )
{
	struct i2c_client *client = apds9130_i2c_client;
	struct apds9130_priv *data = i2c_get_clientdata ( client );

	return sprintf ( buf, "0x%02x\n", data->pers );
}

static ssize_t apds9130_store_pers ( struct device_driver *dev, char *buf, size_t count )
{
	struct i2c_client *client = apds9130_i2c_client;
	unsigned long val = simple_strtoul ( buf, NULL, 10 );
	int ret;

	ret = apds9130_set_pers ( client, val );

	if ( ret < 0 )
		return ret;

	return count;
}

static ssize_t apds9130_show_config ( struct device_driver *dev, char *buf )
{
	struct i2c_client *client = apds9130_i2c_client;
	struct apds9130_priv *data = i2c_get_clientdata ( client );

	return sprintf ( buf, "0x%02x\n", data->config );
}

static ssize_t apds9130_store_config ( struct device_driver *dev, char *buf, size_t count )
{
	struct i2c_client *client = apds9130_i2c_client;
	unsigned long val = simple_strtoul ( buf, NULL, 10 );
	int ret;

	ret = apds9130_set_config ( client, val );

	if ( ret < 0 )
		return ret;

	return count;
}

static ssize_t apds9130_show_ppcount ( struct device_driver *dev, char *buf )
{
	struct i2c_client *client = apds9130_i2c_client;
	struct apds9130_priv *data = i2c_get_clientdata ( client );

	return sprintf ( buf, "0x%02x\n", data->ppcount );
}

static ssize_t apds9130_store_ppcount ( struct device_driver *dev, char *buf, size_t count )
{
	struct i2c_client *client = apds9130_i2c_client;
	unsigned long val = simple_strtoul ( buf, NULL, 10 );
	int ret;

	ret = apds9130_set_ppcount ( client, val );

	if ( ret < 0 )
		return ret;

	return count;
}

static ssize_t apds9130_show_control ( struct device_driver *dev, char *buf )
{
	struct i2c_client *client = apds9130_i2c_client;
	struct apds9130_priv *data = i2c_get_clientdata ( client );

	return sprintf ( buf, "0x%02x\n", data->control );
}

static ssize_t apds9130_store_control ( struct device_driver *dev, char *buf, size_t count )
{
	struct i2c_client *client = apds9130_i2c_client;
	unsigned long val = simple_strtoul ( buf, NULL, 10 );
	int ret;

	ret = apds9130_set_control ( client, val );

	if ( ret < 0 )
		return ret;

	return count;
}

static ssize_t apds9130_show_status ( struct device_driver *dev, char *buf )
{
    struct i2c_client *client = apds9130_i2c_client;
    int status = 0;

    apds9130_get_status ( client, &status );

    return sprintf ( buf, "0x%02x\n", status );
}

static ssize_t apds9130_show_pdata ( struct device_driver *dev, char *buf )
{
    struct i2c_client *client = apds9130_i2c_client;
    int data = 0;

    apds9130_get_pdata ( client, &data );

    return sprintf ( buf, "%d\n", data );
}

//                                                              
static ssize_t apds9130_show_deviceid ( struct device_driver *dev, char *buf )
{
    struct i2c_client *client = apds9130_i2c_client;
    int data = 0;

    apds9130_get_deivceid ( client, &data );

    return sprintf ( buf, "%02x\n", ( u8 )data );
}
//                                                              
static DRIVER_ATTR ( cali, S_IWUSR | S_IRUGO, apds9130_show_cali_value, apds9130_store_cali_value );
static DRIVER_ATTR ( ptime, S_IWUSR | S_IRUGO | S_IWGRP | S_IRGRP | S_IROTH, apds9130_show_ptime, apds9130_store_ptime );
static DRIVER_ATTR ( wtime, S_IWUSR | S_IRUGO | S_IWGRP | S_IRGRP | S_IROTH, apds9130_show_wtime, apds9130_store_wtime );
static DRIVER_ATTR ( pilt, S_IWUSR | S_IRUGO | S_IWGRP | S_IRGRP | S_IROTH, apds9130_show_pilt, apds9130_store_pilt );
static DRIVER_ATTR ( piht, S_IWUSR | S_IRUGO | S_IWGRP | S_IRGRP | S_IROTH, apds9130_show_piht, apds9130_store_piht );
static DRIVER_ATTR ( pers, S_IWUSR | S_IRUGO | S_IWGRP | S_IRGRP | S_IROTH, apds9130_show_pers, apds9130_store_pers );
static DRIVER_ATTR ( config, S_IWUSR | S_IRUGO | S_IWGRP | S_IRGRP | S_IROTH, apds9130_show_config, apds9130_store_config );
static DRIVER_ATTR ( ppcount, S_IWUSR | S_IRUGO | S_IWGRP | S_IRGRP | S_IROTH, apds9130_show_ppcount, apds9130_store_ppcount );
static DRIVER_ATTR ( control, S_IWUSR | S_IRUGO | S_IWGRP | S_IRGRP | S_IROTH, apds9130_show_control, apds9130_store_control );
static DRIVER_ATTR ( status, S_IWUSR | S_IRUGO | S_IWGRP | S_IRGRP | S_IROTH, apds9130_show_status, NULL );
static DRIVER_ATTR ( pdata, S_IWUSR | S_IRUGO | S_IWGRP | S_IRGRP | S_IROTH, apds9130_show_pdata, NULL );
//                                                              
static DRIVER_ATTR ( deviceid, S_IWUSR | S_IRUGO | S_IWGRP | S_IRGRP | S_IROTH, apds9130_show_deviceid, NULL );  //chulho.park
//                                                              

static struct driver_attribute *apds9130_attr_list[] = {
	&driver_attr_cali,		   /*show calibration data*/
	&driver_attr_ptime,
	&driver_attr_wtime,
	&driver_attr_pilt,
	&driver_attr_piht,
	&driver_attr_pers,
	&driver_attr_config,
	&driver_attr_ppcount,
	&driver_attr_control,
	&driver_attr_status,
	&driver_attr_pdata,
//                                                              
	&driver_attr_deviceid,
//                                                              
};

static int apds9130_create_attr ( struct device_driver *driver )
{
	int idx;
	int err = 0;
	int num = ( int ) ( sizeof ( apds9130_attr_list ) / sizeof ( apds9130_attr_list[0] ) );

	if ( driver == NULL )
	{
		return -EINVAL;
	}

	for ( idx = 0 ; idx < num ; idx++ )
	{
		if ( err = driver_create_file ( driver, apds9130_attr_list[idx] ) )
		{
			APS_ERR ( "driver_create_file (%s) = %d\n", apds9130_attr_list[idx]->attr.name, err );
			break;
		}
	}

	return err;
}

static int apds9130_delete_attr ( struct device_driver *driver )
{
	int idx;
	int err = 0;
	int num = ( int ) ( sizeof ( apds9130_attr_list ) / sizeof ( apds9130_attr_list[0] ) );

	if ( driver == NULL )
	{
		return -EINVAL;
	}

	for ( idx = 0 ; idx < num ; idx++ )
	{
		driver_remove_file ( driver, apds9130_attr_list[idx] );
	}

	return err;
}

//==========================================================
// APDS9130 Service APIs ( based on File I/O )
//==========================================================
static int apds9130_open ( struct inode *inode, struct file *file )
{
	APS_FUN ();
	file->private_data = apds9130_i2c_client;

	if ( !file->private_data )
	{
		APS_ERR ( "Invalid input paramerter\n" );
		return -EINVAL;
	}

	return nonseekable_open ( inode, file );
}

static int apds9130_release ( struct inode *inode, struct file *file )
{
	APS_FUN ();
	file->private_data = NULL;
	return 0;
}

static long apds9130_unlocked_ioctl ( struct file *file, unsigned int cmd, unsigned long arg )
{
	APS_FUN ();
	struct i2c_client *client = ( struct i2c_client * ) file->private_data;
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	long err = 0;
	void __user *ptr = ( void __user * ) arg;
	int dat;
	uint32_t enable;
	uint32_t crosstalk = 0;

	switch ( cmd )
	{
		case ALSPS_SET_PS_MODE:
			APS_LOG ( "CMD = ALSPS_SET_PS_MODE\n" );
			if ( copy_from_user ( &enable, ptr, sizeof ( enable ) ) )
			{
				err = -EFAULT;
				goto err_out;
			}
			if ( enable )
			{
				if ( ( err = apds9130_activate ( obj->client, 1 ) ) )
				{
					APS_ERR ( "failed to activate APDS9130 ( err = %d )\n", err );
					goto err_out;
				}
			}
			else
			{
				if ( ( err = apds9130_activate ( obj->client, 0 ) ) )
				{
					APS_ERR ( "failed to deactivate APDS9130 ( err = %d )\n", err );
					goto err_out;
				}
			}
			break;

		case ALSPS_GET_PS_MODE:
			APS_LOG ( "CMD = ALSPS_GET_PS_MODE\n" );
			enable = obj->activate;
			if ( copy_to_user ( ptr, &enable, sizeof ( enable ) ) )
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_PS_DATA:
			APS_LOG ( "CMD = ALSPS_GET_PS_DATA\n" );
			dat = obj->ps_status;
			if ( copy_to_user ( ptr, &dat, sizeof ( dat ) ) )
			{
				err = -EFAULT;
				goto err_out;
			}
			break;

		case ALSPS_GET_PS_RAW_DATA:
			APS_LOG ( "CMD = ALSPS_GET_PS_RAW_DATA\n" );
			if ( err = apds9130_get_pdata ( obj->client, &dat ) )
			{
				goto err_out;
			}

			if ( copy_to_user ( ptr, &dat, sizeof ( dat ) ) )
			{
				err = -EFAULT;
				goto err_out;
			}
			break;
			
		case ALSPS_GET_CALI:
			APS_LOG ( "CMD = ALSPS_GET_CALI\n" );
			err = apds9130_do_calibration ( obj->client, &dat );
			if ( err == 0 )
			{
				if ( copy_to_user ( ptr, &dat, sizeof ( dat ) ) )
				{
					err = -EFAULT;
					goto err_out;
				}
			}
			break;

		case ALSPS_SET_CALI:
            APS_LOG ( "CMD = ALSPS_SET_CALI\n" );
            if ( copy_from_user ( &crosstalk, ptr, sizeof ( crosstalk ) ) )
            {
                err = -EFAULT;
                goto err_out;
            }

			if ( ( crosstalk == 0x0000FFFF ) || ( crosstalk == 0 ) )
			{
				obj->ps_cross_talk = 600;
			}
			else
			{
				if ( crosstalk > 710 )
				{
					obj->ps_cross_talk = 710;
				}
				else
				{
					obj->ps_cross_talk = crosstalk;
				}
			}
			APS_LOG ( "MISC2 read value=%u, ps_cross_talk set value=%u\n", crosstalk, obj->ps_cross_talk );

			obj->ps_th_far_high = 300 + obj->ps_cross_talk;
		    obj->ps_th_near_low = obj->ps_th_far_high - 50;
            break;

//                                                              
		case ALSPS_GET_DEVICEID:
            APS_LOG ( "CMD = ALSPS_GET_DEVICEID\n" );
			if ( err = apds9130_get_deivceid ( obj->client, &dat ) )
			{
				goto err_out;
			}

			if ( copy_to_user ( ptr, &dat, sizeof ( dat ) ) )
			{
				err = -EFAULT;
				goto err_out;
			}
			break;
//                                                              
		default:
			APS_ERR ( "Invalid Command = 0x%04x\n", cmd );
			err = -ENOIOCTLCMD;
			break;
	}

	err_out : return err;
}

static struct file_operations apds9130_fops = { .owner = THIS_MODULE, .open = apds9130_open, .release = apds9130_release,
												.unlocked_ioctl = apds9130_unlocked_ioctl,  };

static struct miscdevice apds9130_device = { .minor = MISC_DYNAMIC_MINOR, .name = "als_ps", .fops = &apds9130_fops,  };

//==========================================================
// APDS9130 Service APIs ( based on hwmsen Interface )
//==========================================================
static int apds9130_ps_operate ( void *self, uint32_t command, void *buff_in, int size_in, void *buff_out, int size_out, int *actualout )
{
	APS_FUN ();
	int err = 0;
	int value;
	hwm_sensor_data *sensor_data;
	struct apds9130_priv *obj = ( struct apds9130_priv * ) self;

	switch ( command )
	{
		case SENSOR_DELAY:
			APS_LOG ( "CMD = SENSOR_DELAY\n" );
			if ( ( buff_in == NULL ) || ( size_in < sizeof ( int ) ) )
			{
				APS_ERR ( "Invaild input parameter\n" );
				err = -EINVAL;
			}
			// Do nothing
			break;

		case SENSOR_ENABLE:
			if ( ( buff_in == NULL ) || ( size_in < sizeof ( int ) ) )
			{
				APS_ERR ( "Invaild input parameter\n" );
				err = -EINVAL;
			}
			else
			{
				value = *( int * ) buff_in;
				if ( value )
				{
					APS_LOG ( "CMD = SENSOR_ENABLE ( Enable )\n" );
					if ( err = apds9130_activate ( obj->client, 1 ) )
					{
						APS_ERR ( "failed to activate APDS9130 ( err = %d )\n", err );
						return -1;
					}
				}
				else
				{
					APS_LOG ( "CMD = SENSOR_ENABLE ( Disable )\n" );
					if ( err = apds9130_activate ( obj->client, 0 ) )
					{
						APS_ERR ( "failed to deactivate APDS9130 ( err = %d )\n", err );
						return -1;
					}
				}
			}
			break;

		case SENSOR_GET_DATA:
			APS_LOG ( "CMD = SENSOR_GET_DATA\n" );
			if ( ( buff_out == NULL ) || ( size_out < sizeof ( hwm_sensor_data ) ) )
			{
				APS_ERR ( "Invaild input parameter\n" );
				err = -EINVAL;
			}
			else
			{
				sensor_data->values[0] = obj->ps_status;
				sensor_data->value_divide = 1;
				sensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
			}
			break;

		default:
			APS_ERR ( "Invalid Command = %d\n", command );
			err = -1;
			break;
	}

	return err;
}

//==========================================================
// APDS9130 Initialization related Routines
//==========================================================
static int apds9130_init_client ( struct i2c_client *client )
{
	APS_FUN ();
	struct apds9130_priv *obj = i2c_get_clientdata ( client );
	int err = 0;

	err = apds9130_initialize ( client );
	if ( err != APDS9130_SUCCESS )
	{
		APS_ERR ( "failed to init APDS9130\n" );
	}

	return err;
}

/****************************************************************************
* I2C BUS Related Functions
****************************************************************************/

#if defined(CONFIG_HAS_EARLYSUSPEND)
static void apds9130_early_suspend ( struct early_suspend *h )
{
	APS_FUN ();
}

static void apds9130_late_resume ( struct early_suspend *h )
{
	APS_FUN ();
}
#endif

static int apds9130_i2c_probe ( struct i2c_client *client, const struct i2c_device_id *id )
{
	APS_FUN ();
	struct apds9130_priv *obj;
	struct hwmsen_object obj_ps;
	struct alsps_hw *hw = get_cust_alsps_hw ();
	int err = 0;

	if ( !( obj = kzalloc ( sizeof ( *obj ), GFP_KERNEL ) ) )
	{
		err = -ENOMEM;
		goto exit;
	}
	memset ( obj, 0, sizeof ( *obj ) );

	obj->client = client;
	i2c_set_clientdata ( client, obj );

	g_apds9130_ptr = obj;
	apds9130_i2c_client = client;

	INIT_WORK ( &obj->eint_work, apds9130_eint_work );

	#if defined(CONFIG_HAS_EARLYSUSPEND)
	obj->early_drv.level = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	obj->early_drv.suspend = apds9130_early_suspend,
	obj->early_drv.resume = apds9130_late_resume,
	register_early_suspend ( &obj->early_drv );
	#endif

	/* Initialize APDS9130 */
	if ( err = apds9130_init_client ( client ) )
	{
		APS_ERR ( "failed to init APDS9130 ( err = %d )\n", err );
		goto exit_init_failed;
	}

	/* Register APDS9130 as a misc device for general I/O interface */
	if ( err = misc_register ( &apds9130_device ) )
	{
		APS_ERR ( "failed to register misc device ( err = %d )\n", err );
		goto exit_misc_device_register_failed;
	}

	if ( err = apds9130_create_attr ( &apds9130_alsps_driver.driver ) )
	{
		APS_ERR ( "create attribute err = %d\n", err );
		goto exit_create_attr_failed;
	}

	/* Register APDS9130 as a member device of hwmsen */
	obj_ps.self = obj;
	obj_ps.polling = hw->polling_mode_ps;
	obj_ps.sensor_operate = apds9130_ps_operate;
	if ( err = hwmsen_attach ( ID_PROXIMITY, &obj_ps ) )
	{
		APS_ERR ( "failed to attach to hwmsen ( err = %d )\n", err );
		goto exit_create_attr_failed;
	}

	return 0;

	exit_create_attr_failed:
	misc_deregister ( &apds9130_device );
	exit_misc_device_register_failed:
	exit_init_failed:
	unregister_early_suspend ( &obj->early_drv );
	kfree ( obj );
	exit:
	apds9130_i2c_client = NULL;
	APS_ERR ( "Err = %d\n", err );
	return err;
}

static int apds9130_i2c_remove ( struct i2c_client *client )
{
	APS_FUN ();
	int err;

	if ( err = apds9130_delete_attr ( &apds9130_alsps_driver.driver ) )
	{
		APS_ERR ( "apds9130_delete_attr fail: %d\n", err );
	}

	if ( err = misc_deregister ( &apds9130_device ) )
	{
		APS_ERR ( "failed to deregister misc driver : %d\n", err );
	}

	apds9130_i2c_client = NULL;
	i2c_unregister_device ( client );
	kfree ( i2c_get_clientdata ( client ) );

	return 0;
}

static int apds9130_i2c_suspend ( struct i2c_client *client, pm_message_t msg )
{	
	APS_FUN();
	
	return 0;
}

static int apds9130_i2c_resume ( struct i2c_client *client )
{
	APS_FUN();	
	
	return 0;
}

static int apds9130_i2c_detect ( struct i2c_client *client, struct i2c_board_info *info )
{
	APS_FUN ();
	strcpy ( info->type, APDS9130_DEV_NAME );
	return 0;
}

static const struct i2c_device_id apds9130_i2c_id[] = { { APDS9130_DEV_NAME, 0 }, {} };

static struct i2c_driver apds9130_i2c_driver = { .probe = apds9130_i2c_probe, .remove = apds9130_i2c_remove, .suspend = apds9130_i2c_suspend,
												 .resume = apds9130_i2c_resume, .detect = apds9130_i2c_detect, .id_table = apds9130_i2c_id,
												 .driver = { .name = APDS9130_DEV_NAME, }, };

/****************************************************************************
* Linux Device Driver Related Functions
****************************************************************************/
static int apds9130_probe ( struct platform_device *pdev )
{
	APS_FUN ();
	struct alsps_hw *hw = get_cust_alsps_hw ();

	/* Configure external ( GPIO ) interrupt */
	apds9130_setup_eint ();

	/* Turn on the power for APDS9130 */
	apds9130_main_power ( hw, 1 );
	msleep ( 9 );

	/* Add APDS9130 as I2C driver */
	if ( i2c_add_driver ( &apds9130_i2c_driver ) )
	{
		APS_ERR ( "failed to add i2c driver\n" );
		return -1;
	}
	return 0;
}

static int apds9130_remove ( struct platform_device *pdev )
{
	APS_FUN ();
	struct alsps_hw *hw = get_cust_alsps_hw ();

	/* Turn off the power for APDS9130 */
	apds9130_main_power ( hw, 0 );

	i2c_del_driver ( &apds9130_i2c_driver );

	return 0;
}

static struct i2c_board_info __initdata i2c_APDS9130 = { I2C_BOARD_INFO ( "APDS9130", 0x39 ) };

static struct platform_driver apds9130_alsps_driver = { .probe = apds9130_probe, .remove = apds9130_remove, .driver = { .name = "als_ps", } };

static int __init apds9130_init ( void )
{
	APS_FUN ();
	i2c_register_board_info ( 0, &i2c_APDS9130, 1 );
	if ( platform_driver_register ( &apds9130_alsps_driver ) )
	{
		APS_ERR ( "failed to register platform driver\n" );
		return -ENODEV;
	}
	return 0;
}

static void __exit apds9130_exit ( void )
{
	APS_FUN ();
	platform_driver_unregister ( &apds9130_alsps_driver );
}


module_init ( apds9130_init );
module_exit ( apds9130_exit );

MODULE_AUTHOR ( "Kang Jun Mo" );
MODULE_DESCRIPTION ( "apds9130 driver" );
MODULE_LICENSE ( "GPL" );

/* End Of File */

