/* BMA255 motion sensor driver
 *
 *
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html

 * (C) Copyright 2011 Bosch Sensortec GmbH
 * All Rights Reserved
 */

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
#include <linux/module.h>

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

#define KNOCK_ON
#ifdef KNOCK_ON
#include <mach/eint.h>
#include "cust_eint.h"
#include "cust_gpio_usage.h"
#endif
#include <cust_acc.h>
#include <linux/hwmsensor.h>
#include <linux/hwmsen_dev.h>
#include <linux/sensors_io.h>
#include "bma255.h"
#include <linux/hwmsen_helper.h>

/*----------------------------------------------------------------------------*/
#define I2C_DRIVERID_BMA255 255
/*----------------------------------------------------------------------------*/
#define DEBUG 1
/*----------------------------------------------------------------------------*/
//#define CONFIG_BMA255_LOWPASS   /*apply low pass filter on output*/
#define SW_CALIBRATION

/*----------------------------------------------------------------------------*/
#define BMA255_AXIS_X          0
#define BMA255_AXIS_Y          1
#define BMA255_AXIS_Z          2
#define BMA255_AXES_NUM        3
#define BMA255_DATA_LEN        6
#define BMA255_DEV_NAME        "BMA255"

#define BMA255_MODE_NORMAL      0
#define BMA255_MODE_LOWPOWER    1
#define BMA255_MODE_SUSPEND     2

#define BMA255_ACC_X_LSB__POS           4
#define BMA255_ACC_X_LSB__LEN           4
#define BMA255_ACC_X_LSB__MSK           0xF0
#define BMA255_ACC_X_LSB__REG           BMA255_X_AXIS_LSB_REG

#define BMA255_ACC_X_MSB__POS           0
#define BMA255_ACC_X_MSB__LEN           8
#define BMA255_ACC_X_MSB__MSK           0xFF
#define BMA255_ACC_X_MSB__REG           BMA255_X_AXIS_MSB_REG

#define BMA255_ACC_Y_LSB__POS           4
#define BMA255_ACC_Y_LSB__LEN           4
#define BMA255_ACC_Y_LSB__MSK           0xF0
#define BMA255_ACC_Y_LSB__REG           BMA255_Y_AXIS_LSB_REG

#define BMA255_ACC_Y_MSB__POS           0
#define BMA255_ACC_Y_MSB__LEN           8
#define BMA255_ACC_Y_MSB__MSK           0xFF
#define BMA255_ACC_Y_MSB__REG           BMA255_Y_AXIS_MSB_REG

#define BMA255_ACC_Z_LSB__POS           4
#define BMA255_ACC_Z_LSB__LEN           4
#define BMA255_ACC_Z_LSB__MSK           0xF0
#define BMA255_ACC_Z_LSB__REG           BMA255_Z_AXIS_LSB_REG

#define BMA255_ACC_Z_MSB__POS           0
#define BMA255_ACC_Z_MSB__LEN           8
#define BMA255_ACC_Z_MSB__MSK           0xFF
#define BMA255_ACC_Z_MSB__REG           BMA255_Z_AXIS_MSB_REG

#define BMA255_EN_LOW_POWER__POS          6
#define BMA255_EN_LOW_POWER__LEN          1
#define BMA255_EN_LOW_POWER__MSK          0x40
#define BMA255_EN_LOW_POWER__REG          BMA255_REG_POWER_CTL

#define BMA255_EN_SUSPEND__POS            7
#define BMA255_EN_SUSPEND__LEN            1
#define BMA255_EN_SUSPEND__MSK            0x80
#define BMA255_EN_SUSPEND__REG            BMA255_REG_POWER_CTL

#define BMA255_RANGE_SEL__POS             0
#define BMA255_RANGE_SEL__LEN             4
#define BMA255_RANGE_SEL__MSK             0x0F
#define BMA255_RANGE_SEL__REG             BMA255_REG_DATA_FORMAT

#define BMA255_BANDWIDTH__POS             0
#define BMA255_BANDWIDTH__LEN             5
#define BMA255_BANDWIDTH__MSK             0x1F
#define BMA255_BANDWIDTH__REG             BMA255_REG_BW_RATE

#define BMA255_EN_SOFT_RESET__POS         0
#define BMA255_EN_SOFT_RESET__LEN         8
#define BMA255_EN_SOFT_RESET__MSK         0xFF
#define BMA255_EN_SOFT_RESET__REG         BMA255_RESET_REG

#define BMA255_EN_SOFT_RESET_VALUE        0xB6

#define BMA255_CUT_OFF					0
#define BMA255_OFFSET_TRIGGER_X         1
#define BMA255_OFFSET_TRIGGER_Y         2
#define BMA255_OFFSET_TRIGGER_Z         3

#define BMA255_FAST_CAL_RDY_S__POS             4
#define BMA255_FAST_CAL_RDY_S__LEN             1
#define BMA255_FAST_CAL_RDY_S__MSK             0x10
#define BMA255_FAST_CAL_RDY_S__REG             BMA255_OFFSET_CTRL_REG

#define BMA255_CAL_TRIGGER__POS                5
#define BMA255_CAL_TRIGGER__LEN                2
#define BMA255_CAL_TRIGGER__MSK                0x60
#define BMA255_CAL_TRIGGER__REG                BMA255_OFFSET_CTRL_REG

#define BMA255_COMP_CUTOFF__POS                 0
#define BMA255_COMP_CUTOFF__LEN                 1
#define BMA255_COMP_CUTOFF__MSK                 0x01
#define BMA255_COMP_CUTOFF__REG                 BMA255_OFFSET_PARAMS_REG

#define BMA255_COMP_TARGET_OFFSET_X__POS        1
#define BMA255_COMP_TARGET_OFFSET_X__LEN        2
#define BMA255_COMP_TARGET_OFFSET_X__MSK        0x06
#define BMA255_COMP_TARGET_OFFSET_X__REG        BMA255_OFFSET_PARAMS_REG

#define BMA255_COMP_TARGET_OFFSET_Y__POS        3
#define BMA255_COMP_TARGET_OFFSET_Y__LEN        2
#define BMA255_COMP_TARGET_OFFSET_Y__MSK        0x18
#define BMA255_COMP_TARGET_OFFSET_Y__REG        BMA255_OFFSET_PARAMS_REG

#define BMA255_COMP_TARGET_OFFSET_Z__POS        5
#define BMA255_COMP_TARGET_OFFSET_Z__LEN        2
#define BMA255_COMP_TARGET_OFFSET_Z__MSK        0x60
#define BMA255_COMP_TARGET_OFFSET_Z__REG        BMA255_OFFSET_PARAMS_REG

#ifdef KNOCK_ON
#define BMA255_UNFILT_INT_SRC_TAP__POS         4
#define BMA255_UNFILT_INT_SRC_TAP__LEN         1
#define BMA255_UNFILT_INT_SRC_TAP__MSK         0x10
#define BMA255_UNFILT_INT_SRC_TAP__REG         BMA255_INT_SRC_REG

#define BMA255_INT_MODE_SEL__POS                0
#define BMA255_INT_MODE_SEL__LEN                4
#define BMA255_INT_MODE_SEL__MSK                0x0F
#define BMA255_INT_MODE_SEL__REG                BMA255_INT_CTRL_REG

#define BMA255_EN_INT1_PAD_DB_TAP__POS      4
#define BMA255_EN_INT1_PAD_DB_TAP__LEN      1
#define BMA255_EN_INT1_PAD_DB_TAP__MSK      0x10
#define BMA255_EN_INT1_PAD_DB_TAP__REG      BMA255_INT1_PAD_SEL_REG

#define BMA255_EN_INT1_PAD_SNG_TAP__POS     5
#define BMA255_EN_INT1_PAD_SNG_TAP__LEN     1
#define BMA255_EN_INT1_PAD_SNG_TAP__MSK     0x20
#define BMA255_EN_INT1_PAD_SNG_TAP__REG     BMA255_INT1_PAD_SEL_REG

#define BMA255_EN_INT1_PAD_FLAT__POS        7
#define BMA255_EN_INT1_PAD_FLAT__LEN        1
#define BMA255_EN_INT1_PAD_FLAT__MSK        0x80
#define BMA255_EN_INT1_PAD_FLAT__REG        BMA255_INT1_PAD_SEL_REG

#define BMA255_EN_DOUBLE_TAP_INT__POS      4
#define BMA255_EN_DOUBLE_TAP_INT__LEN      1
#define BMA255_EN_DOUBLE_TAP_INT__MSK      0x10
#define BMA255_EN_DOUBLE_TAP_INT__REG      BMA255_INT_ENABLE0

#define BMA255_EN_SINGLE_TAP_INT__POS      5
#define BMA255_EN_SINGLE_TAP_INT__LEN      1
#define BMA255_EN_SINGLE_TAP_INT__MSK      0x20
#define BMA255_EN_SINGLE_TAP_INT__REG      BMA255_INT_ENABLE0

#define BMA255_EN_FLAT_INT__POS            7
#define BMA255_EN_FLAT_INT__LEN            1
#define BMA255_EN_FLAT_INT__MSK            0x80
#define BMA255_EN_FLAT_INT__REG            BMA255_INT_ENABLE0

#define BMA255_FLAT_STATUS__POS               7
#define BMA255_FLAT_STATUS__LEN               1
#define BMA255_FLAT_STATUS__MSK               0x80
#define BMA255_FLAT_STATUS__REG               BMA255_INT_STATUS3_REG

#define BMA255_TAP_THRES__POS                  0
#define BMA255_TAP_THRES__LEN                  5
#define BMA255_TAP_THRES__MSK                  0x1F
#define BMA255_TAP_THRES__REG                  BMA255_TAP_THRESHOLD

#define BMA255_TAP_QUIET_DURN__POS             7
#define BMA255_TAP_QUIET_DURN__LEN             1
#define BMA255_TAP_QUIET_DURN__MSK             0x80
#define BMA255_TAP_QUIET_DURN__REG             BMA255_TAP_TIMING

#define BMA255_TAP_DUR__POS                    0
#define BMA255_TAP_DUR__LEN                    3
#define BMA255_TAP_DUR__MSK                    0x07
#define BMA255_TAP_DUR__REG                    BMA255_TAP_TIMING

#define BMA255_TAP_SHOCK_DURN__POS             6
#define BMA255_TAP_SHOCK_DURN__LEN             1
#define BMA255_TAP_SHOCK_DURN__MSK             0x40
#define BMA255_TAP_SHOCK_DURN__REG             BMA255_TAP_TIMING

#define BMA255_FLAT_ANGLE__POS                  0
#define BMA255_FLAT_ANGLE__LEN                  6
#define BMA255_FLAT_ANGLE__MSK                  0x3F
#define BMA255_FLAT_ANGLE__REG                  BMA255_FLAT_THRESHOLD_ANGLE

#define LGE_GSENSOR_NAME	 "lge_gsensor"

#define PAD_FLAT					6
#define PAD_DOUBLE_TAP				7
#define PAD_SINGLE_TAP				8

struct foo_obj{
	struct kobject kobj;
	int intterupt;
};
char *gsensor_wakeup_gesture[2] = { "TOUCH_GESTURE_WAKEUP=WAKEUP", NULL };
static struct foo_obj *foo_obj;
#endif

#define BMA255_ACCEL_CALIBRATION

#ifdef BMA255_ACCEL_CALIBRATION
#define BMA255_SHAKING_DETECT_THRESHOLD	(300)	 //clubsh cal2 50 -> 200
#endif

struct bma255acc{
	s16	x,
		y,
		z;
};

#define BMA255_GET_BITSLICE(regvar, bitname)\
	((regvar & bitname##__MSK) >> bitname##__POS)

#define BMA255_SET_BITSLICE(regvar, bitname, val)\
	((regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK))

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static const struct i2c_device_id bma255_i2c_id[] = {{BMA255_DEV_NAME,0},{}};
static struct i2c_board_info __initdata bma255_i2c_info ={ I2C_BOARD_INFO(BMA255_DEV_NAME, BMA255_I2C_ADDR)};

/*----------------------------------------------------------------------------*/
static int bma255_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int bma255_i2c_remove(struct i2c_client *client);
static int bma255_set_tap_threshold(struct i2c_client *client, unsigned char threshold);
static int bma255_set_tap_threshold_flat(struct i2c_client *client, unsigned char threshold);
/*----------------------------------------------------------------------------*/
typedef enum {
    BMA_TRC_FILTER  = 0x01,
    BMA_TRC_RAWDATA = 0x02,
    BMA_TRC_IOCTL   = 0x04,
    BMA_TRC_CALI	= 0X08,
    BMA_TRC_INFO	= 0X10,
} BMA_TRC;
/*----------------------------------------------------------------------------*/
struct scale_factor{
    u8  whole;
    u8  fraction;
};
/*----------------------------------------------------------------------------*/
struct data_resolution {
    struct scale_factor scalefactor;
    int                 sensitivity;
};
/*----------------------------------------------------------------------------*/
#define C_MAX_FIR_LENGTH (32)
/*----------------------------------------------------------------------------*/
struct data_filter {
    s16 raw[C_MAX_FIR_LENGTH][BMA255_AXES_NUM];
    int sum[BMA255_AXES_NUM];
    int num;
    int idx;
};
/*----------------------------------------------------------------------------*/
struct bma255_i2c_data {
    struct i2c_client *client;
    struct acc_hw *hw;
    struct hwmsen_convert   cvt;

    /*misc*/
    struct data_resolution *reso;
    atomic_t                trace;
    atomic_t                suspend;
    atomic_t                selftest;
	atomic_t				filter;
    s16                     cali_sw[BMA255_AXES_NUM+1];

    /*data*/
    s8                      offset[BMA255_AXES_NUM+1];  /*+1: for 4-byte alignment*/
    s16                     data[BMA255_AXES_NUM+1];

#if defined(CONFIG_BMA255_LOWPASS)
    atomic_t                firlen;
    atomic_t                fir_en;
    struct data_filter      fir;
#endif
    /*early suspend*/
#if defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend    early_drv;
#endif
//                                                                                
	atomic_t fast_calib_x_rslt;
	atomic_t fast_calib_y_rslt;
	atomic_t fast_calib_z_rslt;
	atomic_t fast_calib_rslt;
//                                                                               
};
/*----------------------------------------------------------------------------*/
static struct i2c_driver bma255_i2c_driver = {
    .driver = {
        .name           = BMA255_DEV_NAME,
    },
	.probe      		= bma255_i2c_probe,
	.remove    			= bma255_i2c_remove,
#if !defined(CONFIG_HAS_EARLYSUSPEND)
    .suspend            = bma255_suspend,
    .resume             = bma255_resume,
#endif
	.id_table = bma255_i2c_id,
};

/*----------------------------------------------------------------------------*/
static struct i2c_client *bma255_i2c_client = NULL;
static struct platform_driver bma255_gsensor_driver;
static struct bma255_i2c_data *obj_i2c_data = NULL;
static bool sensor_power = true;
static GSENSOR_VECTOR3D gsensor_gain;
static char selftestRes[8]= {0};

#ifdef KNOCK_ON
static bool sensor_suspend = false;
bool doubletap_enable = true;
static unsigned char tap_threshold_flat = 0x01;
static unsigned char tap_threshold = 0x04;
static unsigned char tap_bandwidth = 0x0c;
EXPORT_SYMBOL(doubletap_enable);

static struct delayed_work bma255_work;
static struct workqueue_struct *bma255_wq;
//extern void kpd_send_pwrkey(void);
#endif
/*----------------------------------------------------------------------------*/
#define GSE_TAG                  "[Gsensor] "
#define GSE_FUN(f)               printk(KERN_ERR GSE_TAG"%s()\n", __FUNCTION__)
#define GSE_ERR(fmt, args...)    printk(KERN_ERR GSE_TAG"[ERROR] %s() line=%d : "fmt, __FUNCTION__, __LINE__, ##args)
#define GSE_LOG(fmt, args...)    printk(KERN_ERR GSE_TAG fmt, ##args)
/*----------------------------------------------------------------------------*/
static struct data_resolution bma255_data_resolution[1] = {
 /* combination by {FULL_RES,RANGE}*/
    {{ 1, 0}, 1024},   // dataformat +/-2g  in 12-bit resolution;  { 1, 0} = 1.0= (2*2*1000)/(2^12);  1024 = (2^12)/(2*2)
};
/*----------------------------------------------------------------------------*/
static struct data_resolution bma255_offset_resolution = {{1, 0}, 1024};
/*----------------------------------------------------------------------------*/

static int bma255_smbus_read_byte(struct i2c_client *client,	unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_read_byte_data(client, reg_addr);
	if (dummy < 0)
		return -1;
	*data = dummy & 0x000000ff;

	return 0;
}

static int bma255_smbus_write_byte(struct i2c_client *client, unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_write_byte_data(client, reg_addr, *data);
	if (dummy < 0)
		return -1;
	return 0;
}

static int bma255_smbus_read_byte_block(struct i2c_client *client, unsigned char reg_addr, unsigned char *data, unsigned char len)
{
	s32 dummy;
	dummy = i2c_smbus_read_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0)
		return -1;
	return 0;
}

#ifdef KNOCK_ON
static int bma255_get_orient_flat_status(struct i2c_client *client, unsigned char *intstatus)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_FLAT_STATUS__REG, &data);
	data = BMA255_GET_BITSLICE(data, BMA255_FLAT_STATUS);

	*intstatus = data;

	return comres;
}

static int bma255_set_Int_Mode(struct i2c_client *client, unsigned char Mode)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_INT_MODE_SEL__REG, &data);
	data = BMA255_SET_BITSLICE(data, BMA255_INT_MODE_SEL, Mode);
	comres = bma255_smbus_write_byte(client, BMA255_INT_MODE_SEL__REG, &data);

	return comres;
}

static int bma255_set_int1_pad_sel(struct i2c_client *client, unsigned char int1sel)
{
	int comres = 0;
	unsigned char data;
	unsigned char state;
	state = 0x01;

	switch (int1sel)
	{
		case PAD_DOUBLE_TAP:
			comres = bma255_smbus_read_byte(client, BMA255_EN_INT1_PAD_DB_TAP__REG, &data);
			data = BMA255_SET_BITSLICE(data, BMA255_EN_INT1_PAD_DB_TAP, state);
			comres = bma255_smbus_write_byte(client, BMA255_EN_INT1_PAD_DB_TAP__REG, &data);

			comres = bma255_smbus_read_byte(client,	BMA255_UNFILT_INT_SRC_TAP__REG, &data);
			data |= 0x10;
			comres = bma255_smbus_write_byte(client, BMA255_UNFILT_INT_SRC_TAP__REG, &data);
			break;
		case PAD_SINGLE_TAP:
			comres = bma255_smbus_read_byte(client, BMA255_EN_INT1_PAD_SNG_TAP__REG, &data);
			data = BMA255_SET_BITSLICE(data, BMA255_EN_INT1_PAD_SNG_TAP, state);
			comres = bma255_smbus_write_byte(client, BMA255_EN_INT1_PAD_SNG_TAP__REG, &data);
			break;
		case PAD_FLAT:
			comres = bma255_smbus_read_byte(client,BMA255_EN_INT1_PAD_FLAT__REG, &data);
			data = BMA255_SET_BITSLICE(data, BMA255_EN_INT1_PAD_FLAT,state);
			comres = bma255_smbus_write_byte(client,BMA255_EN_INT1_PAD_FLAT__REG, &data);
			break;
		default:
			break;
	}

	return comres;
}

static int bma255_get_interruptstatus1(struct i2c_client *client, unsigned char *intstatus)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_INT_STATUS0_REG, &data);
	*intstatus = data;

	return comres;
}

static int bma255_set_Int_Enable(struct i2c_client *client, unsigned char InterruptType , unsigned char value)
{
	int comres = 0;
	unsigned char data1;

	comres = bma255_smbus_read_byte(client, BMA255_INT_ENABLE0, &data1);
	value = value & 1;

	switch (InterruptType)
	{
		case 8:
			/* Single Tap Interrupt */
			data1 = BMA255_SET_BITSLICE(data1, BMA255_EN_SINGLE_TAP_INT, value);
			break;
		case 9:
			/* Double Tap Interrupt */
			data1 = BMA255_SET_BITSLICE(data1, BMA255_EN_DOUBLE_TAP_INT, value);
			break;
		case 11:
			/* Flat Interrupt */
			data1 = BMA255_SET_BITSLICE(data1, BMA255_EN_FLAT_INT, value);
			break;
		default:
			break;
	}

	comres = bma255_smbus_write_byte(client, BMA255_INT_ENABLE0, &data1);

	return comres;
}

static void bma255_set_int1(struct i2c_client *client)
{
	bma255_set_Int_Mode(client, 1); /*latch interrupt 250ms*/

	//bma255_set_Int_Enable(client, 8, 1);
	bma255_set_Int_Enable(client, 9, 1);
	bma255_set_Int_Enable(client, 11, 1);
	/* maps interrupt to INT1 pin */
	//bma255_set_int1_pad_sel(client, PAD_SINGLE_TAP);
	bma255_set_int1_pad_sel(client, PAD_DOUBLE_TAP);

	bma255_set_int1_pad_sel(client, PAD_FLAT);
}

static void bma255_work_func(struct work_struct *work)
{
	unsigned char status = 0;
	unsigned char trigger_data;
	unsigned char sign_value = 0;
	struct i2c_client *client = bma255_i2c_client;

	bma255_get_interruptstatus1(client, &status);

	switch (status)
	{
		case 0x10:
			GSE_LOG("double tap interrupt happened\n");
			if(sensor_suspend == true)
			{
				bma255_smbus_read_byte(client, BMA255_INT_STATUS2_REG, &trigger_data);
				GSE_LOG("double tap trigger_data = %d\n", trigger_data);
				if(trigger_data == 192 || trigger_data == 64)
				{
					GSE_LOG("double tap interrupt happened by Z\n");
					//kpd_send_pwrkey();
					kobject_uevent_env(&foo_obj->kobj, KOBJ_CHANGE, gsensor_wakeup_gesture);
				}
			}
			break;
		case 0x20:
			GSE_LOG("single tap interrupt happened\n");
//			if(sensor_suspend==true)kpd_send_pwrkey();
			break;
		case 0x80:
			bma255_get_orient_flat_status(client, &sign_value);
			GSE_LOG("flat detection interrupt happened\n");
			if (sign_value == 1)
			{
				GSE_LOG("flat position\n");
				bma255_set_tap_threshold_flat(client, tap_threshold_flat);
			}
			else
			{
				GSE_LOG("non flat position\n");
				bma255_set_tap_threshold(client, tap_threshold);
			}
			break;
		default:
			GSE_LOG("Unknown interrupt: %x\n", status);
			break;
	}
	
	mt_eint_unmask(CUST_EINT_GSE_1_NUM);
}

static void bma255_eint_func(void)
{
	mt_eint_mask(CUST_EINT_GSE_1_NUM);
	queue_delayed_work(bma255_wq, &bma255_work, 0);
}

static void bma255_setup_eint(void)
{
	mt_set_gpio_dir(GPIO_GSE_1_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_mode(GPIO_GSE_1_EINT_PIN, GPIO_GSE_1_EINT_PIN_M_EINT);
	mt_set_gpio_pull_enable(GPIO_GSE_1_EINT_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_GSE_1_EINT_PIN, GPIO_PULL_DOWN);

	mt_eint_set_sens(CUST_EINT_GSE_1_NUM, CUST_EINT_EDGE_SENSITIVE);
	mt_eint_set_polarity(CUST_EINT_GSE_1_NUM, CUST_EINT_POLARITY_HIGH);
	mt_eint_set_hw_debounce(CUST_EINT_GSE_1_NUM, CUST_EINT_GSE_1_DEBOUNCE_CN);
	mt_eint_registration(CUST_EINT_GSE_1_NUM, CUST_EINT_POLARITY_HIGH, bma255_eint_func, 0);

	mt_eint_unmask(CUST_EINT_GSE_1_NUM);
}
#endif

/*--------------------BMA255 power control function----------------------------------*/
static void BMA255_power(struct acc_hw *hw, unsigned int on)
{
	static unsigned int power_on = 0;

	if(hw->power_id != POWER_NONE_MACRO)		// have externel LDO
	{
		GSE_LOG("power %s\n", on ? "on" : "off");
		if(power_on == on)	// power status not change
		{
			GSE_LOG("ignore power control: %d\n", on);
		}
		else if(on)	// power on
		{
			if(!hwPowerOn(hw->power_id, hw->power_vol, "BMA255"))
			{
				GSE_ERR("power on fails!!\n");
			}
		}
		else	// power off
		{
#ifdef KNOCK_ON
			if(doubletap_enable == false)
			{
				GSE_LOG("POWER OFF\n");
#endif
				if (!hwPowerDown(hw->power_id, "BMA255"))
				{
					GSE_ERR("power off fail!!\n");
				}
#ifdef KNOCK_ON				
			}
			else
			{
				GSE_LOG("POWER On\n");
			}
#endif
		}
	}
	power_on = on;
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
static int BMA255_SetDataResolution(struct bma255_i2c_data *obj)
{

/*set g sensor dataresolution here*/

/*BMA255 only can set to 10-bit dataresolution, so do nothing in bma255 driver here*/

/*end of set dataresolution*/

 /*we set measure range from -2g to +2g in BMA255_SetDataFormat(client, BMA255_RANGE_2G),
                                                    and set 10-bit dataresolution BMA255_SetDataResolution()*/

 /*so bma255_data_resolution[0] set value as {{ 3, 9}, 256} when declaration, and assign the value to obj->reso here*/

	obj->reso = &bma255_data_resolution[0];
	return 0;

/*if you changed the measure range, for example call: BMA255_SetDataFormat(client, BMA255_RANGE_4G),
you must set the right value to bma255_data_resolution*/

}
/*----------------------------------------------------------------------------*/
static int BMA255_ReadData(struct i2c_client *client, s16 data[BMA255_AXES_NUM])
{
	struct bma255_i2c_data *priv = i2c_get_clientdata(client);
	u8 addr = BMA255_REG_DATAXLOW;
	u8 buf[BMA255_DATA_LEN] = {0};
	int err = 0;

	if(NULL == client)
	{
		err = -EINVAL;
	}
	else if(err = hwmsen_read_block(client, addr, buf, BMA255_DATA_LEN))
	{
		GSE_ERR("error: %d\n", err);
	}
	else
	{
		/* Convert sensor raw data to 16-bit integer */
		data[BMA255_AXIS_X] = BMA255_GET_BITSLICE(buf[0], BMA255_ACC_X_LSB) | (BMA255_GET_BITSLICE(buf[1], BMA255_ACC_X_MSB)<<BMA255_ACC_X_LSB__LEN);
		data[BMA255_AXIS_X] = data[BMA255_AXIS_X] << (sizeof(short)*8-(BMA255_ACC_X_LSB__LEN + BMA255_ACC_X_MSB__LEN));
		data[BMA255_AXIS_X] = data[BMA255_AXIS_X] >> (sizeof(short)*8-(BMA255_ACC_X_LSB__LEN + BMA255_ACC_X_MSB__LEN));

		data[BMA255_AXIS_Y] = BMA255_GET_BITSLICE(buf[2], BMA255_ACC_Y_LSB)	| (BMA255_GET_BITSLICE(buf[3], BMA255_ACC_Y_MSB)<<BMA255_ACC_Y_LSB__LEN);
		data[BMA255_AXIS_Y] = data[BMA255_AXIS_Y] << (sizeof(short)*8-(BMA255_ACC_Y_LSB__LEN + BMA255_ACC_Y_MSB__LEN));
		data[BMA255_AXIS_Y] = data[BMA255_AXIS_Y] >> (sizeof(short)*8-(BMA255_ACC_Y_LSB__LEN + BMA255_ACC_Y_MSB__LEN));

		data[BMA255_AXIS_Z] = BMA255_GET_BITSLICE(buf[4], BMA255_ACC_Z_LSB)	| (BMA255_GET_BITSLICE(buf[5], BMA255_ACC_Z_MSB)<<BMA255_ACC_Z_LSB__LEN);
		data[BMA255_AXIS_Z] = data[BMA255_AXIS_Z] << (sizeof(short)*8-(BMA255_ACC_Z_LSB__LEN + BMA255_ACC_Z_MSB__LEN));
		data[BMA255_AXIS_Z] = data[BMA255_AXIS_Z] >> (sizeof(short)*8-(BMA255_ACC_Z_LSB__LEN + BMA255_ACC_Z_MSB__LEN));

#ifdef CONFIG_BMA255_LOWPASS
		if(atomic_read(&priv->filter))
		{
			if(atomic_read(&priv->fir_en) && !atomic_read(&priv->suspend))
			{
				int idx, firlen = atomic_read(&priv->firlen);
				if(priv->fir.num < firlen)
				{
					priv->fir.raw[priv->fir.num][BMA255_AXIS_X] = data[BMA255_AXIS_X];
					priv->fir.raw[priv->fir.num][BMA255_AXIS_Y] = data[BMA255_AXIS_Y];
					priv->fir.raw[priv->fir.num][BMA255_AXIS_Z] = data[BMA255_AXIS_Z];
					priv->fir.sum[BMA255_AXIS_X] += data[BMA255_AXIS_X];
					priv->fir.sum[BMA255_AXIS_Y] += data[BMA255_AXIS_Y];
					priv->fir.sum[BMA255_AXIS_Z] += data[BMA255_AXIS_Z];
					if(atomic_read(&priv->trace) & BMA_TRC_FILTER)
					{
						GSE_LOG("add [%2d] [%5d %5d %5d] => [%5d %5d %5d]\n", priv->fir.num,
							priv->fir.raw[priv->fir.num][BMA255_AXIS_X], priv->fir.raw[priv->fir.num][BMA255_AXIS_Y], priv->fir.raw[priv->fir.num][BMA255_AXIS_Z],
							priv->fir.sum[BMA255_AXIS_X], priv->fir.sum[BMA255_AXIS_Y], priv->fir.sum[BMA255_AXIS_Z]);
					}
					priv->fir.num++;
					priv->fir.idx++;
				}
				else
				{
					idx = priv->fir.idx % firlen;
					priv->fir.sum[BMA255_AXIS_X] -= priv->fir.raw[idx][BMA255_AXIS_X];
					priv->fir.sum[BMA255_AXIS_Y] -= priv->fir.raw[idx][BMA255_AXIS_Y];
					priv->fir.sum[BMA255_AXIS_Z] -= priv->fir.raw[idx][BMA255_AXIS_Z];
					priv->fir.raw[idx][BMA255_AXIS_X] = data[BMA255_AXIS_X];
					priv->fir.raw[idx][BMA255_AXIS_Y] = data[BMA255_AXIS_Y];
					priv->fir.raw[idx][BMA255_AXIS_Z] = data[BMA255_AXIS_Z];
					priv->fir.sum[BMA255_AXIS_X] += data[BMA255_AXIS_X];
					priv->fir.sum[BMA255_AXIS_Y] += data[BMA255_AXIS_Y];
					priv->fir.sum[BMA255_AXIS_Z] += data[BMA255_AXIS_Z];
					priv->fir.idx++;
					data[BMA255_AXIS_X] = priv->fir.sum[BMA255_AXIS_X]/firlen;
					data[BMA255_AXIS_Y] = priv->fir.sum[BMA255_AXIS_Y]/firlen;
					data[BMA255_AXIS_Z] = priv->fir.sum[BMA255_AXIS_Z]/firlen;
					if(atomic_read(&priv->trace) & BMA_TRC_FILTER)
					{
						GSE_LOG("add [%2d] [%5d %5d %5d] => [%5d %5d %5d] : [%5d %5d %5d]\n", idx,
						priv->fir.raw[idx][BMA255_AXIS_X], priv->fir.raw[idx][BMA255_AXIS_Y], priv->fir.raw[idx][BMA255_AXIS_Z],
						priv->fir.sum[BMA255_AXIS_X], priv->fir.sum[BMA255_AXIS_Y], priv->fir.sum[BMA255_AXIS_Z],
						data[BMA255_AXIS_X], data[BMA255_AXIS_Y], data[BMA255_AXIS_Z]);
					}
				}
			}
		}
#endif
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int BMA255_ResetCalibration(struct i2c_client *client)
{
	struct bma255_i2c_data *obj = i2c_get_clientdata(client);
	u8 ofs[4]={0,0,0,0};
	int err;

	#ifdef SW_CALIBRATION

	#else
		if(err = hwmsen_write_block(client, BMA255_REG_OFSX, ofs, 4))
		{
			GSE_ERR("error: %d\n", err);
		}
	#endif

	memset(obj->cali_sw, 0x00, sizeof(obj->cali_sw));
	memset(obj->offset, 0x00, sizeof(obj->offset));
	return err;
}
/*----------------------------------------------------------------------------*/
static int BMA255_CheckDeviceID(struct i2c_client *client)
{
	u8 databuf[2];
	int res = 0;

	memset(databuf, 0, sizeof(u8)*2);
	databuf[0] = BMA255_REG_DEVID;

	res = i2c_master_send(client, databuf, 0x1);
	if(res <= 0)
	{
		res = i2c_master_send(client, databuf, 0x1);
		if(res <= 0)
		{
			goto exit_BMA255_CheckDeviceID;
		}
	}

	udelay(500);

	databuf[0] = 0x0;
	res = i2c_master_recv(client, databuf, 0x01);
	if(res <= 0)
	{
		goto exit_BMA255_CheckDeviceID;
	}

	if(databuf[0]!=BMA255_FIXED_DEVID)
	{
		GSE_ERR("BMA255_CheckDeviceID %d failt!\n ", databuf[0]);
		return BMA255_ERR_IDENTIFICATION;
	}
	else
	{
		GSE_LOG("BMA255_CheckDeviceID %d pass!\n ", databuf[0]);
	}

	exit_BMA255_CheckDeviceID:
	if (res <= 0)
	{
		return BMA255_ERR_I2C;
	}

	return BMA255_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int BMA255_SetPowerMode(struct i2c_client *client, bool enable)
{
	GSE_FUN();
	u8 databuf[2] = {0};
	int res = 0;
	struct bma255_i2c_data *obj = i2c_get_clientdata(client);

	if(enable == sensor_power )
	{
		GSE_LOG("Sensor power status is newest!\n");
		return BMA255_SUCCESS;
	}

	if(enable == TRUE)
	{
		databuf[1] = 0;
	}
	else
	{
#ifdef KNOCK_ON
		databuf[1] = 0;
		if(doubletap_enable)
		{
			GSE_LOG("POWER On -2\n");
		}
		else
		{
			GSE_LOG("POWER Off -2\n");
		}
#else
		databuf[1] = BMA255_MEASURE_MODE;
#endif
	}

	databuf[0] = BMA255_REG_POWER_CTL;

	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		GSE_LOG("set power mode failed!\n");
		return BMA255_ERR_I2C;
	}
	else
	{
		GSE_LOG("set power mode ok: 0x%02x\n", databuf[1]);
	}

	sensor_power = enable;

	mdelay(20);

	return BMA255_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int BMA255_SetDataFormat(struct i2c_client *client, u8 dataformat)
{
	struct bma255_i2c_data *obj = i2c_get_clientdata(client);
	u8 databuf[2] = {0};
	int res = 0;

	databuf[1] = dataformat;
	databuf[0] = BMA255_REG_DATA_FORMAT;

	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		return BMA255_ERR_I2C;
	}
	else
	{
		GSE_LOG("Set DataFormat: 0x%02x\n", dataformat);
	}

	return BMA255_SetDataResolution(obj);
}
/*----------------------------------------------------------------------------*/
static int BMA255_SetBWRate(struct i2c_client *client, u8 bwrate)
{
	u8 databuf[2] = {0};
	int res = 0;

	databuf[1] = bwrate;
	databuf[0] = BMA255_REG_BW_RATE;

	res = i2c_master_send(client, databuf, 0x2);
	if(res <= 0)
	{
		return BMA255_ERR_I2C;
	}
	else
	{
		GSE_LOG("Set BWrate: 0x%02x\n", bwrate);
	}

	return BMA255_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int BMA255_SetIntEnable(struct i2c_client *client, u8 intenable)
{
	int res = 0;

	res = hwmsen_write_byte(client, BMA255_INT_ENABLE0, intenable);
	if(res != BMA255_SUCCESS)
	{
		return res;
	}
	res = hwmsen_write_byte(client, BMA255_INT_ENABLE1, intenable);
	if(res != BMA255_SUCCESS)
	{
		return res;
	}
	GSE_LOG("BMA255 interrupt was disabled\n");

	/*for disable interrupt function*/

	return BMA255_SUCCESS;
}

/*----------------------------------------------------------------------------*/
static int bma255_init_client(struct i2c_client *client, int reset_cali)
{
	GSE_FUN();
	struct bma255_i2c_data *obj = i2c_get_clientdata(client);
	int res = 0;

	res = BMA255_CheckDeviceID(client);
	if(res != BMA255_SUCCESS)
	{
		return res;
	}

	res = BMA255_SetBWRate(client, BMA255_BW_100HZ);
	if(res != BMA255_SUCCESS )
	{
		return res;
	}

	res = BMA255_SetDataFormat(client, BMA255_RANGE_2G);
	if(res != BMA255_SUCCESS)
	{
		return res;
	}

	gsensor_gain.x = gsensor_gain.y = gsensor_gain.z = obj->reso->sensitivity;

#ifdef KNOCK_ON
	bma255_set_int1(client);
#else
	res = BMA255_SetIntEnable(client, 0x00);
	if(res != BMA255_SUCCESS)
	{
		return res;
	}
#endif

	res = BMA255_SetPowerMode(client, false);
	if(res != BMA255_SUCCESS)
	{
		return res;
	}

	if(0 != reset_cali)
	{
		/*reset calibration only in power on*/
		res = BMA255_ResetCalibration(client);
		if(res != BMA255_SUCCESS)
		{
			return res;
		}
	}

#ifdef CONFIG_BMA255_LOWPASS
	memset(&obj->fir, 0x00, sizeof(obj->fir));
#endif

	mdelay(20);

	return BMA255_SUCCESS;
}
/*----------------------------------------------------------------------------*/
static int BMA255_ReadChipInfo(struct i2c_client *client, char *buf, int bufsize)
{
	u8 databuf[10];

	memset(databuf, 0, sizeof(u8)*10);

	if((NULL == buf)||(bufsize<=30))
	{
		return -1;
	}

	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	sprintf(buf, "BMA255 Chip");
	return 0;
}
/*----------------------------------------------------------------------------*/
static int BMA255_CompassReadData(struct i2c_client *client, char *buf, int bufsize)
{
	struct bma255_i2c_data *obj = (struct bma255_i2c_data*)i2c_get_clientdata(client);
	u8 databuf[20];
	int acc[BMA255_AXES_NUM];
	int res = 0;
	memset(databuf, 0, sizeof(u8)*10);

	if(NULL == buf)
	{
		return -1;
	}
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	if(sensor_power == FALSE)
	{
		res = BMA255_SetPowerMode(client, true);
		if(res)
		{
			GSE_ERR("Power on bma255 error %d!\n", res);
		}
	}

	if(res = BMA255_ReadData(client, obj->data))
	{
		GSE_ERR("I2C error: ret value=%d", res);
		return -3;
	}
	else
	{
		/*remap coordinate*/
		acc[obj->cvt.map[BMA255_AXIS_X]] = obj->cvt.sign[BMA255_AXIS_X]*obj->data[BMA255_AXIS_X];
		acc[obj->cvt.map[BMA255_AXIS_Y]] = obj->cvt.sign[BMA255_AXIS_Y]*obj->data[BMA255_AXIS_Y];
		acc[obj->cvt.map[BMA255_AXIS_Z]] = obj->cvt.sign[BMA255_AXIS_Z]*obj->data[BMA255_AXIS_Z];
		//printk("cvt x=%d, y=%d, z=%d \n",obj->cvt.sign[BMA255_AXIS_X],obj->cvt.sign[BMA255_AXIS_Y],obj->cvt.sign[BMA255_AXIS_Z]);

		//GSE_LOG("Mapped gsensor data: %d, %d, %d!\n", acc[BMA255_AXIS_X], acc[BMA255_AXIS_Y], acc[BMA255_AXIS_Z]);

		sprintf(buf, "%d %d %d", (s16)acc[BMA255_AXIS_X], (s16)acc[BMA255_AXIS_Y], (s16)acc[BMA255_AXIS_Z]);
		if(atomic_read(&obj->trace) & BMA_TRC_IOCTL)
		{
			GSE_LOG("gsensor data for compass: %s!\n", buf);
		}
	}

	return 0;
}
/*----------------------------------------------------------------------------*/
static int BMA255_ReadSensorData(struct i2c_client *client, char *buf, int bufsize)
{
	struct bma255_i2c_data *obj = (struct bma255_i2c_data*)i2c_get_clientdata(client);
	u8 databuf[20];
	int acc[BMA255_AXES_NUM];
	int res = 0;
	memset(databuf, 0, sizeof(u8)*10);

	if(NULL == buf)
	{
		return -1;
	}
	if(NULL == client)
	{
		*buf = 0;
		return -2;
	}

	if(sensor_power == FALSE)
	{
		res = BMA255_SetPowerMode(client, true);
		if(res)
		{
			GSE_ERR("Power on bma255 error %d!\n", res);
		}
	}

	if(res = BMA255_ReadData(client, obj->data))
	{
		GSE_ERR("I2C error: ret value=%d", res);
		return -3;
	}
	else
	{
		//printk("raw data x=%d, y=%d, z=%d \n",obj->data[BMA255_AXIS_X],obj->data[BMA255_AXIS_Y],obj->data[BMA255_AXIS_Z]);
		obj->data[BMA255_AXIS_X] += obj->cali_sw[BMA255_AXIS_X];
		obj->data[BMA255_AXIS_Y] += obj->cali_sw[BMA255_AXIS_Y];
		obj->data[BMA255_AXIS_Z] += obj->cali_sw[BMA255_AXIS_Z];

		//printk("cali_sw x=%d, y=%d, z=%d \n",obj->cali_sw[BMA255_AXIS_X],obj->cali_sw[BMA255_AXIS_Y],obj->cali_sw[BMA255_AXIS_Z]);

		/*remap coordinate*/
		acc[obj->cvt.map[BMA255_AXIS_X]] = obj->cvt.sign[BMA255_AXIS_X]*obj->data[BMA255_AXIS_X];
		acc[obj->cvt.map[BMA255_AXIS_Y]] = obj->cvt.sign[BMA255_AXIS_Y]*obj->data[BMA255_AXIS_Y];
		acc[obj->cvt.map[BMA255_AXIS_Z]] = obj->cvt.sign[BMA255_AXIS_Z]*obj->data[BMA255_AXIS_Z];
		//printk("cvt x=%d, y=%d, z=%d \n",obj->cvt.sign[BMA255_AXIS_X],obj->cvt.sign[BMA255_AXIS_Y],obj->cvt.sign[BMA255_AXIS_Z]);

		//GSE_LOG("Mapped gsensor data: %d, %d, %d!\n", acc[BMA255_AXIS_X], acc[BMA255_AXIS_Y], acc[BMA255_AXIS_Z]);

		//Out put the mg
		//printk("mg acc=%d, GRAVITY=%d, sensityvity=%d \n",acc[BMA255_AXIS_X],GRAVITY_EARTH_1000,obj->reso->sensitivity);
		acc[BMA255_AXIS_X] = acc[BMA255_AXIS_X] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		acc[BMA255_AXIS_Y] = acc[BMA255_AXIS_Y] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;
		acc[BMA255_AXIS_Z] = acc[BMA255_AXIS_Z] * GRAVITY_EARTH_1000 / obj->reso->sensitivity;

		sprintf(buf, "%04x %04x %04x", acc[BMA255_AXIS_X], acc[BMA255_AXIS_Y], acc[BMA255_AXIS_Z]);
		if(atomic_read(&obj->trace) & BMA_TRC_IOCTL)
		{
			GSE_LOG("gsensor data: %s!\n", buf);
		}
	}

	return 0;
}
/*----------------------------------------------------------------------------*/
static int BMA255_ReadRawData(struct i2c_client *client, char *buf)
{
	struct bma255_i2c_data *obj = (struct bma255_i2c_data*)i2c_get_clientdata(client);
	int res = 0;

	if (!buf || !client)
	{
		return EINVAL;
	}

	if(res = BMA255_ReadData(client, obj->data))
	{
		GSE_ERR("I2C error: ret value=%d", res);
		return EIO;
	}
	else
	{
		sprintf(buf, "BMA255_ReadRawData %04x %04x %04x", obj->data[BMA255_AXIS_X],
			obj->data[BMA255_AXIS_Y], obj->data[BMA255_AXIS_Z]);
	}

	return 0;
}
/*----------------------------------------------------------------------------*/
static int bma255_set_mode(struct i2c_client *client, unsigned char mode)
{
	int comres = 0;
	unsigned char data[2] = {BMA255_EN_LOW_POWER__REG};

	if ((client == NULL) || (mode >= 3))
	{
		return -1;
	}

	comres = hwmsen_read_block(client, BMA255_EN_LOW_POWER__REG, data+1, 1);
	switch (mode)
	{
		case BMA255_MODE_NORMAL:
			data[1] = BMA255_SET_BITSLICE(data[1], BMA255_EN_LOW_POWER, 0);
			data[1] = BMA255_SET_BITSLICE(data[1], BMA255_EN_SUSPEND, 0);
			break;

		case BMA255_MODE_LOWPOWER:
#ifdef KNOCK_ON
			data[1] = BMA255_SET_BITSLICE(data[1], BMA255_EN_LOW_POWER, 0);
#else
			data[1] = BMA255_SET_BITSLICE(data[1], BMA255_EN_LOW_POWER, 1);
#endif
			data[1] = BMA255_SET_BITSLICE(data[1], BMA255_EN_SUSPEND, 0);
			break;

		case BMA255_MODE_SUSPEND:
			data[1] = BMA255_SET_BITSLICE(data[1], BMA255_EN_LOW_POWER, 0);
#ifdef KNOCK_ON
			data[1] = BMA255_SET_BITSLICE(data[1], BMA255_EN_SUSPEND, 0);
#else
			data[1] = BMA255_SET_BITSLICE(data[1], BMA255_EN_SUSPEND, 1);
#endif
			break;

		default:
			break;
	}

	comres = i2c_master_send(client, data, 2);
	if(comres <= 0)
	{
		return BMA255_ERR_I2C;
	}
	else
	{
		return comres;
	}
}
/*----------------------------------------------------------------------------*/
static int bma255_get_mode(struct i2c_client *client, unsigned char *mode)
{
	int comres = 0;

	if (client == NULL)
	{
		return -1;
	}
	comres = hwmsen_read_block(client, BMA255_EN_LOW_POWER__REG, mode, 1);
	*mode = (*mode) >> 6;

	return comres;
}
/*----------------------------------------------------------------------------*/
static int bma255_set_range(struct i2c_client *client, unsigned char range)
{
	int comres = 0;
	unsigned char data[2] = {BMA255_RANGE_SEL__REG};

	if (client == NULL)
	{
		return -1;
	}

	comres = hwmsen_read_block(client, BMA255_RANGE_SEL__REG, data+1, 1);
	data[1] = BMA255_SET_BITSLICE(data[1], BMA255_RANGE_SEL, range);

	comres = i2c_master_send(client, data, 2);
	if(comres <= 0)
	{
		return BMA255_ERR_I2C;
	}
	else
	{
		return comres;
	}
}
/*----------------------------------------------------------------------------*/
static int bma255_get_range(struct i2c_client *client, unsigned char *range)
{
	int comres = 0;
	unsigned char data;

	if (client == NULL)
	{
		return -1;
	}

	comres = hwmsen_read_block(client, BMA255_RANGE_SEL__REG, &data, 1);
	*range = BMA255_GET_BITSLICE(data, BMA255_RANGE_SEL);

	return comres;
}
/*----------------------------------------------------------------------------*/
static int bma255_set_bandwidth(struct i2c_client *client, unsigned char bandwidth)
{
	int comres = 0;
	unsigned char data[2] = {BMA255_BANDWIDTH__REG};

	if (client == NULL)
	{
		return -1;
	}

	comres = hwmsen_read_block(client, BMA255_BANDWIDTH__REG, data+1, 1);
	data[1] = BMA255_SET_BITSLICE(data[1], BMA255_BANDWIDTH, bandwidth);

	comres = i2c_master_send(client, data, 2);
	if(comres <= 0)
	{
		return BMA255_ERR_I2C;
	}
	else
	{
		return comres;
	}
}
/*----------------------------------------------------------------------------*/
static int bma255_get_bandwidth(struct i2c_client *client, unsigned char *bandwidth)
{
	int comres = 0;
	unsigned char data;

	if (client == NULL)
	{
		return -1;
	}

	comres = hwmsen_read_block(client, BMA255_BANDWIDTH__REG, &data, 1);
	data = BMA255_GET_BITSLICE(data, BMA255_BANDWIDTH);

	if (data < 0x08) //7.81Hz
	{
		*bandwidth = 0x08;
	}
	else if (data > 0x0f)	// 1000Hz
	{
		*bandwidth = 0x0f;
	}
	else
	{
		*bandwidth = data;
	}
	return comres;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_chipinfo_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = bma255_i2c_client;
	char strbuf[BMA255_BUFSIZE];
	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}

	BMA255_ReadChipInfo(client, strbuf, BMA255_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);
}

static ssize_t gsensor_init(struct device_driver *ddri, char *buf, size_t count)
{
	struct i2c_client *client = bma255_i2c_client;
	char strbuf[BMA255_BUFSIZE];

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	bma255_init_client(client, 1);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);
}

/*----------------------------------------------------------------------------*/
/*
g sensor opmode for compass tilt compensation
*/
static ssize_t show_cpsopmode_value(struct device_driver *ddri, char *buf)
{
	unsigned char data;

	if (bma255_get_mode(bma255_i2c_client, &data) < 0)
	{
		return sprintf(buf, "Read error\n");
	}
	else
	{
		return sprintf(buf, "%d\n", data);
	}
}

/*----------------------------------------------------------------------------*/
/*
g sensor opmode for compass tilt compensation
*/
static ssize_t store_cpsopmode_value(struct device_driver *ddri, char *buf, size_t count)
{
	unsigned long data;
	int error;

	if (error = strict_strtoul(buf, 10, &data))
	{
		return error;
	}
	if (data == BMA255_MODE_NORMAL)
	{
		BMA255_SetPowerMode(bma255_i2c_client, true);
	}
	else if (data == BMA255_MODE_SUSPEND)
	{
		BMA255_SetPowerMode(bma255_i2c_client, false);
	}
	else if (bma255_set_mode(bma255_i2c_client, (unsigned char) data) < 0)
	{
		GSE_ERR("invalid content: '%s', length = %d\n", buf, count);
	}

	return count;
}

/*----------------------------------------------------------------------------*/
/*
g sensor range for compass tilt compensation
*/
static ssize_t show_cpsrange_value(struct device_driver *ddri, char *buf)
{
	unsigned char data;

	if (bma255_get_range(bma255_i2c_client, &data) < 0)
	{
		return sprintf(buf, "Read error\n");
	}
	else
	{
		return sprintf(buf, "%d\n", data);
	}
}

/*----------------------------------------------------------------------------*/
/*
g sensor range for compass tilt compensation
*/
static ssize_t store_cpsrange_value(struct device_driver *ddri, char *buf, size_t count)
{
	unsigned long data;
	int error;

	if (error = strict_strtoul(buf, 10, &data))
	{
		return error;
	}
	if (bma255_set_range(bma255_i2c_client, (unsigned char) data) < 0)
	{
		GSE_ERR("invalid content: '%s', length = %d\n", buf, count);
	}

	return count;
}
/*----------------------------------------------------------------------------*/
/*
g sensor bandwidth for compass tilt compensation
*/
static ssize_t show_cpsbandwidth_value(struct device_driver *ddri, char *buf)
{
	unsigned char data;

	if (bma255_get_bandwidth(bma255_i2c_client, &data) < 0)
	{
		return sprintf(buf, "Read error\n");
	}
	else
	{
		return sprintf(buf, "%d\n", data);
	}
}

/*----------------------------------------------------------------------------*/
/*
g sensor bandwidth for compass tilt compensation
*/
static ssize_t store_cpsbandwidth_value(struct device_driver *ddri, char *buf, size_t count)
{
	unsigned long data;
	int error;

	if (error = strict_strtoul(buf, 10, &data))
	{
		return error;
	}
	if (bma255_set_bandwidth(bma255_i2c_client, (unsigned char) data) < 0)
	{
		GSE_ERR("invalid content: '%s', length = %d\n", buf, count);
	}

	return count;
}

/*----------------------------------------------------------------------------*/
/*
g sensor data for compass tilt compensation
*/
static ssize_t show_cpsdata_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = bma255_i2c_client;
	char strbuf[BMA255_BUFSIZE];

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	BMA255_CompassReadData(client, strbuf, BMA255_BUFSIZE);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);
}

/*----------------------------------------------------------------------------*/
static ssize_t show_sensordata_value(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = bma255_i2c_client;
	char strbuf[BMA255_BUFSIZE];

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	BMA255_ReadSensorData(client, strbuf, BMA255_BUFSIZE);
	//BMA255_ReadRawData(client, strbuf);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);
}

static ssize_t show_sensorrawdata_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct i2c_client *client = bma255_i2c_client;
	char strbuf[BMA255_BUFSIZE];

	if(NULL == client)
	{
		GSE_ERR("i2c client is null!!\n");
		return 0;
	}
	//BMA255_ReadSensorData(client, strbuf, BMA255_BUFSIZE);
	BMA255_ReadRawData(client, strbuf);
	return snprintf(buf, PAGE_SIZE, "%s\n", strbuf);
}
/*----------------------------------------------------------------------------*/
static ssize_t show_firlen_value(struct device_driver *ddri, char *buf)
{
#ifdef CONFIG_BMA255_LOWPASS
	struct i2c_client *client = bma255_i2c_client;
	struct bma255_i2c_data *obj = i2c_get_clientdata(client);
	if(atomic_read(&obj->firlen))
	{
		int idx, len = atomic_read(&obj->firlen);
		GSE_LOG("len = %2d, idx = %2d\n", obj->fir.num, obj->fir.idx);

		for(idx = 0; idx < len; idx++)
		{
			GSE_LOG("[%5d %5d %5d]\n", obj->fir.raw[idx][BMA255_AXIS_X], obj->fir.raw[idx][BMA255_AXIS_Y], obj->fir.raw[idx][BMA255_AXIS_Z]);
		}

		GSE_LOG("sum = [%5d %5d %5d]\n", obj->fir.sum[BMA255_AXIS_X], obj->fir.sum[BMA255_AXIS_Y], obj->fir.sum[BMA255_AXIS_Z]);
		GSE_LOG("avg = [%5d %5d %5d]\n", obj->fir.sum[BMA255_AXIS_X]/len, obj->fir.sum[BMA255_AXIS_Y]/len, obj->fir.sum[BMA255_AXIS_Z]/len);
	}
	return snprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&obj->firlen));
#else
	return snprintf(buf, PAGE_SIZE, "not support\n");
#endif
}
/*----------------------------------------------------------------------------*/
static ssize_t store_firlen_value(struct device_driver *ddri, char *buf, size_t count)
{
#ifdef CONFIG_BMA255_LOWPASS
	struct i2c_client *client = bma255_i2c_client;
	struct bma255_i2c_data *obj = i2c_get_clientdata(client);
	int firlen;

	if(1 != sscanf(buf, "%d", &firlen))
	{
		GSE_ERR("invallid format\n");
	}
	else if(firlen > C_MAX_FIR_LENGTH)
	{
		GSE_ERR("exceeds maximum filter length\n");
	}
	else
	{
		atomic_set(&obj->firlen, firlen);
		if(NULL == firlen)
		{
			atomic_set(&obj->fir_en, 0);
		}
		else
		{
			memset(&obj->fir, 0x00, sizeof(obj->fir));
			atomic_set(&obj->fir_en, 1);
		}
	}
#endif
	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_trace_value(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	struct bma255_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}

	res = snprintf(buf, PAGE_SIZE, "0x%04X\n", atomic_read(&obj->trace));
	return res;
}
/*----------------------------------------------------------------------------*/
static ssize_t store_trace_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct bma255_i2c_data *obj = obj_i2c_data;
	int trace;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}

	if(1 == sscanf(buf, "0x%x", &trace))
	{
		atomic_set(&obj->trace, trace);
	}
	else
	{
		GSE_ERR("invalid content: '%s', length = %d\n", buf, count);
	}

	return count;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_status_value(struct device_driver *ddri, char *buf)
{
	ssize_t len = 0;
	struct bma255_i2c_data *obj = obj_i2c_data;
	if (obj == NULL)
	{
		GSE_ERR("i2c_data obj is null!!\n");
		return 0;
	}

	if(obj->hw)
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: %d %d (%d %d)\n", obj->hw->i2c_num, obj->hw->direction, obj->hw->power_id, obj->hw->power_vol);
	}
	else
	{
		len += snprintf(buf+len, PAGE_SIZE-len, "CUST: NULL\n");
	}
	return len;
}
/*----------------------------------------------------------------------------*/
static ssize_t show_power_status_value(struct device_driver *ddri, char *buf)
{
	if(sensor_power)
		GSE_LOG("G sensor is in work mode, sensor_power = %d\n", sensor_power);
	else
		GSE_LOG("G sensor is in standby mode, sensor_power = %d\n", sensor_power);

	return 0;
}

#ifdef BMA255_ACCEL_CALIBRATION
static int bma255_set_offset_target(struct i2c_client *client, unsigned char channel, unsigned char offset)
{
	unsigned char data;
	int comres = 0;

	switch (channel)
	{
		case BMA255_CUT_OFF:
			comres = bma255_smbus_read_byte(client,	BMA255_COMP_CUTOFF__REG, &data);
			data = BMA255_SET_BITSLICE(data, BMA255_COMP_CUTOFF, offset);
			comres = bma255_smbus_write_byte(client, BMA255_COMP_CUTOFF__REG, &data);
			break;

		case BMA255_OFFSET_TRIGGER_X:
			comres = bma255_smbus_read_byte(client,	BMA255_COMP_TARGET_OFFSET_X__REG, &data);
			data = BMA255_SET_BITSLICE(data, BMA255_COMP_TARGET_OFFSET_X, offset);
			comres = bma255_smbus_write_byte(client, BMA255_COMP_TARGET_OFFSET_X__REG, &data);
			break;

		case BMA255_OFFSET_TRIGGER_Y:
			comres = bma255_smbus_read_byte(client,	BMA255_COMP_TARGET_OFFSET_Y__REG, &data);
			data = BMA255_SET_BITSLICE(data, BMA255_COMP_TARGET_OFFSET_Y, offset);
			comres = bma255_smbus_write_byte(client, BMA255_COMP_TARGET_OFFSET_Y__REG, &data);
			break;

		case BMA255_OFFSET_TRIGGER_Z:
			comres = bma255_smbus_read_byte(client,	BMA255_COMP_TARGET_OFFSET_Z__REG, &data);
			data = BMA255_SET_BITSLICE(data, BMA255_COMP_TARGET_OFFSET_Z, offset);
			comres = bma255_smbus_write_byte(client, BMA255_COMP_TARGET_OFFSET_Z__REG, &data);
			break;

		default:
			comres = -1;
			break;
	}

	return comres;
}

static int bma255_get_cal_ready(struct i2c_client *client, unsigned char *calrdy)
{
	int comres = 0 ;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_FAST_CAL_RDY_S__REG,	&data);
	data = BMA255_GET_BITSLICE(data, BMA255_FAST_CAL_RDY_S);
	*calrdy = data;

	return comres;
}

static int bma255_set_cal_trigger(struct i2c_client *client, unsigned char caltrigger)
{
	int comres = 0;
	unsigned char data;
	struct bma255_i2c_data *bma255 = i2c_get_clientdata(client);

	if(atomic_read(&bma255->fast_calib_rslt) != 0)
	{
		atomic_set(&bma255->fast_calib_rslt, 0);
		GSE_LOG(KERN_INFO "[set] bma2X2->fast_calib_rslt:%d\n",atomic_read(&bma255->fast_calib_rslt));
	}

	comres = bma255_smbus_read_byte(client, BMA255_CAL_TRIGGER__REG, &data);
	data = BMA255_SET_BITSLICE(data, BMA255_CAL_TRIGGER, caltrigger);
	comres = bma255_smbus_write_byte(client, BMA255_CAL_TRIGGER__REG, &data);

	return comres;
}

static int bma255_set_offset_x(struct i2c_client *client, unsigned char	offsetfilt)
{
	int comres = 0;
	unsigned char data;

	data =  offsetfilt;
	comres = bma255_smbus_write_byte(client, BMA255_OFFSET_X_AXIS_REG, &data);

	return comres;
}

static int bma255_get_offset_x(struct i2c_client *client, unsigned char *offsetfilt)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_OFFSET_X_AXIS_REG, &data);
	*offsetfilt = data;

	return comres;
}

static int bma255_set_offset_y(struct i2c_client *client, unsigned char	offsetfilt)
{
	int comres = 0;
	unsigned char data;

	data =  offsetfilt;
	comres = bma255_smbus_write_byte(client, BMA255_OFFSET_Y_AXIS_REG, &data);

	return comres;
}

static int bma255_get_offset_y(struct i2c_client *client, unsigned char *offsetfilt)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_OFFSET_Y_AXIS_REG, &data);
	*offsetfilt = data;

	return comres;
}

static int bma255_set_offset_z(struct i2c_client *client, unsigned char	offsetfilt)
{
	int comres = 0;
	unsigned char data;

	data =  offsetfilt;
	comres = bma255_smbus_write_byte(client, BMA255_OFFSET_Z_AXIS_REG, &data);

	return comres;
}

static int bma255_get_offset_z(struct i2c_client *client, unsigned char *offsetfilt)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_OFFSET_Z_AXIS_REG,
						&data);
	*offsetfilt = data;

	return comres;
}

static int bma255_read_accel_xyz(struct i2c_client *client, struct bma255acc *acc)
{
	int comres = 0;
	unsigned char data[6];

	comres = bma255_smbus_read_byte_block(client, BMA255_ACC_X_LSB__REG, data, 6);

	acc->x = BMA255_GET_BITSLICE(data[0], BMA255_ACC_X_LSB) | (BMA255_GET_BITSLICE(data[1], BMA255_ACC_X_MSB)<<(BMA255_ACC_X_LSB__LEN));
	acc->x = acc->x << (sizeof(short)*8-(BMA255_ACC_X_LSB__LEN + BMA255_ACC_X_MSB__LEN));
	acc->x = acc->x >> (sizeof(short)*8-(BMA255_ACC_X_LSB__LEN + BMA255_ACC_X_MSB__LEN));

	acc->y = BMA255_GET_BITSLICE(data[2], BMA255_ACC_Y_LSB) | (BMA255_GET_BITSLICE(data[3],	BMA255_ACC_Y_MSB)<<(BMA255_ACC_Y_LSB__LEN));
	acc->y = acc->y << (sizeof(short)*8-(BMA255_ACC_Y_LSB__LEN + BMA255_ACC_Y_MSB__LEN));
	acc->y = acc->y >> (sizeof(short)*8-(BMA255_ACC_Y_LSB__LEN + BMA255_ACC_Y_MSB__LEN));

	acc->z = BMA255_GET_BITSLICE(data[4], BMA255_ACC_Z_LSB) | (BMA255_GET_BITSLICE(data[5],	BMA255_ACC_Z_MSB)<<(BMA255_ACC_Z_LSB__LEN));
	acc->z = acc->z << (sizeof(short)*8-(BMA255_ACC_Z_LSB__LEN + BMA255_ACC_Z_MSB__LEN));
	acc->z = acc->z >> (sizeof(short)*8-(BMA255_ACC_Z_LSB__LEN + BMA255_ACC_Z_MSB__LEN));

	return comres;
}

static ssize_t show_cali_value(struct device_driver *ddri, char *buf)
{
	GSE_FUN();
	struct i2c_client *client = bma255_i2c_client;
    struct bma255_i2c_data *bma255 = i2c_get_clientdata(client);
	unsigned char offset_x,offset_y,offset_z;
	unsigned int offsets[3];

	if(bma255_get_offset_x(bma255->client, &offset_x) < 0)
		return -EINVAL;
	if(bma255_get_offset_y(bma255->client, &offset_y) < 0)
		return -EINVAL;
	if(bma255_get_offset_z(bma255->client, &offset_z) < 0)
		return -EINVAL;

    GSE_LOG("offset_x: %c, offset_y: %c, offset_z: %c\n",offset_x,offset_y,offset_z);

	return snprintf(buf, PAGE_SIZE, "%d %d %d \n", offset_x, offset_y, offset_z);
}

static ssize_t store_cali_value(struct device_driver *ddri, char *buf, size_t count)
{
	struct i2c_client *client = bma255_i2c_client;
	struct bma255_i2c_data *bma255 = i2c_get_clientdata(client);
	int err;
	unsigned int offset_x,offset_y,offset_z;
	int dat[BMA255_AXES_NUM];

	if(!strncmp(buf, "rst", 3))
	{
		GSE_FUN();
		if(err = BMA255_ResetCalibration(client))
		{
			GSE_ERR("reset offset err = %d\n", err);
		}
	}
	else if(3 == sscanf(buf, "%d %d %d", &offset_x, &offset_y, &offset_z))
	{
		GSE_LOG("store_cali_value: x=%d, y=%d, z=%d\n", offset_x, offset_y, offset_z);

		if(bma255_set_offset_x(bma255->client, (unsigned char)offset_x) < 0)
			return -EINVAL;
		if(bma255_set_offset_y(bma255->client, (unsigned char)offset_y) < 0)
			return -EINVAL;
		if(bma255_set_offset_z(bma255->client, (unsigned char)offset_z) < 0)
			return -EINVAL;
	}
	else
	{
		GSE_ERR("invalid format\n");
	}

	return count;
}

static ssize_t bma255_fast_calibration_x_show(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = bma255_i2c_client;
	struct bma255_i2c_data *bma255 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma255->fast_calib_x_rslt));
}

static ssize_t bma255_fast_calibration_x_store(struct device_driver *ddri, char *buf, size_t count)
{
	unsigned long data;
	signed char tmp;
	unsigned char timeout = 0;
	unsigned int timeout_shaking = 0;
	int error;
	struct i2c_client *client =bma255_i2c_client;
	struct bma255_i2c_data *bma255 = i2c_get_clientdata(client);
	struct bma255acc acc_cal;
	struct bma255acc acc_cal_pre;

	if (error = strict_strtoul(buf, 10, &data))
		return error;

	atomic_set(&bma255->fast_calib_x_rslt, 0);
	bma255_read_accel_xyz(bma255->client, &acc_cal_pre);

	do{
		mdelay(20);
		bma255_read_accel_xyz(bma255->client, &acc_cal);

		GSE_LOG(KERN_INFO "===============moved x=============== timeout = %d\n",timeout_shaking);
		GSE_LOG(KERN_INFO "(%d, %d, %d) (%d, %d, %d)\n", acc_cal_pre.x,	acc_cal_pre.y,	acc_cal_pre.z, acc_cal.x,acc_cal.y,acc_cal.z );

		if((abs(acc_cal.x - acc_cal_pre.x) > BMA255_SHAKING_DETECT_THRESHOLD)
			|| (abs((acc_cal.y - acc_cal_pre.y)) > BMA255_SHAKING_DETECT_THRESHOLD)
			|| (abs((acc_cal.z - acc_cal_pre.z)) > BMA255_SHAKING_DETECT_THRESHOLD))
		{
			atomic_set(&bma255->fast_calib_rslt, 0);
			GSE_LOG(KERN_INFO "===============shaking x===============\n");
			return -EINVAL;
		}
		else
		{
			acc_cal_pre.x = acc_cal.x;
			acc_cal_pre.y = acc_cal.y;
			acc_cal_pre.z = acc_cal.z;
		}
		timeout_shaking++;
		GSE_LOG(KERN_INFO "===============timeout_shaking: %d=============== \n",timeout_shaking);
	} while(timeout_shaking < 10);

	GSE_LOG(KERN_INFO "===============complete shaking x check===============\n");

	if (bma255_set_offset_target(bma255->client, 1, (unsigned char)data) < 0)
		return -EINVAL;

	if (bma255_set_cal_trigger(bma255->client, 1) < 0)
		return -EINVAL;

	do{
		mdelay(2);
		bma255_get_cal_ready(bma255->client, &tmp);

		GSE_LOG(KERN_INFO "x wait 2ms cal ready flag is %d\n", tmp);

		timeout++;
		if (timeout == 50)
		{
			GSE_LOG(KERN_INFO "get fast calibration ready error\n");
			return -EINVAL;
		}
	} while (tmp == 0);

	atomic_set(&bma255->fast_calib_x_rslt, 1);
	GSE_LOG(KERN_INFO "x axis fast calibration finished\n");

	return count;
}

static ssize_t bma255_fast_calibration_y_show(struct device_driver *ddri, char *buf)
{

	struct i2c_client *client = bma255_i2c_client;
	struct bma255_i2c_data *bma255 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma255->fast_calib_y_rslt));
}

static ssize_t bma255_fast_calibration_y_store(struct device_driver *ddri, char *buf, size_t count)
{
	unsigned long data;
	signed char tmp;
	unsigned char timeout = 0;
	unsigned int timeout_shaking = 0;
	int error;
	struct i2c_client *client = bma255_i2c_client;
	struct bma255_i2c_data *bma255 = i2c_get_clientdata(client);
	struct bma255acc acc_cal;
	struct bma255acc acc_cal_pre;

	if (error = strict_strtoul(buf, 10, &data))
		return error;

	atomic_set(&bma255->fast_calib_y_rslt, 0);
	bma255_read_accel_xyz(bma255->client, &acc_cal_pre);

	do{
		mdelay(20);
		bma255_read_accel_xyz(bma255->client, &acc_cal);

		GSE_LOG(KERN_INFO "===============moved y=============== timeout = %d\n",timeout_shaking);
		GSE_LOG(KERN_INFO "(%d, %d, %d) (%d, %d, %d)\n", acc_cal_pre.x,	acc_cal_pre.y,	acc_cal_pre.z, acc_cal.x,acc_cal.y,acc_cal.z );

		if((abs(acc_cal.x - acc_cal_pre.x) > BMA255_SHAKING_DETECT_THRESHOLD)
			|| (abs((acc_cal.y - acc_cal_pre.y)) > BMA255_SHAKING_DETECT_THRESHOLD)
			|| (abs((acc_cal.z - acc_cal_pre.z)) > BMA255_SHAKING_DETECT_THRESHOLD))
		{
			atomic_set(&bma255->fast_calib_rslt, 0);
			GSE_LOG(KERN_INFO "===============shaking y===============\n");
			return -EINVAL;
		}
		else
		{
			acc_cal_pre.x = acc_cal.x;
			acc_cal_pre.y = acc_cal.y;
			acc_cal_pre.z = acc_cal.z;
		}
		timeout_shaking++;
	} while(timeout_shaking < 10);

	GSE_LOG(KERN_INFO "===============complete shaking y check===============\n");

	if (bma255_set_offset_target(bma255->client, 2, (unsigned char)data) < 0)
		return -EINVAL;

	if (bma255_set_cal_trigger(bma255->client, 2) < 0)
		return -EINVAL;

	do {
		mdelay(2);
		bma255_get_cal_ready(bma255->client, &tmp);

		GSE_LOG(KERN_INFO "y wait 2ms cal ready flag is %d\n", tmp);

		timeout++;
		if (timeout == 50)
		{
			GSE_LOG(KERN_INFO "get fast calibration ready error\n");
			return -EINVAL;
		}
	} while (tmp == 0);

	atomic_set(&bma255->fast_calib_y_rslt, 1);
	GSE_LOG(KERN_INFO "y axis fast calibration finished\n");

	return count;
}

static ssize_t bma255_fast_calibration_z_show(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = bma255_i2c_client;
	struct bma255_i2c_data *bma255 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma255->fast_calib_z_rslt));
}

static ssize_t bma255_fast_calibration_z_store(struct device_driver *ddri, char *buf, size_t count)
{
	unsigned long data;
	signed char tmp;
	unsigned char timeout = 0;
	unsigned int timeout_shaking = 0;
	int error;
	struct i2c_client *client = bma255_i2c_client;
	struct bma255_i2c_data *bma255 = i2c_get_clientdata(client);
	struct bma255acc acc_cal;
	struct bma255acc acc_cal_pre;

	if (error = strict_strtoul(buf, 10, &data))
		return error;

	atomic_set(&bma255->fast_calib_z_rslt, 0);
	bma255_read_accel_xyz(bma255->client, &acc_cal_pre);

	do{
		mdelay(20);
		bma255_read_accel_xyz(bma255->client, &acc_cal);

		GSE_LOG(KERN_INFO "===============moved z=============== timeout = %d\n",timeout_shaking);
		GSE_LOG(KERN_INFO "(%d, %d, %d) (%d, %d, %d)\n", acc_cal_pre.x,	acc_cal_pre.y,	acc_cal_pre.z, acc_cal.x,acc_cal.y,acc_cal.z );

		if((abs(acc_cal.x - acc_cal_pre.x) > BMA255_SHAKING_DETECT_THRESHOLD)
			|| (abs((acc_cal.y - acc_cal_pre.y)) > BMA255_SHAKING_DETECT_THRESHOLD)
			|| (abs((acc_cal.z - acc_cal_pre.z)) > BMA255_SHAKING_DETECT_THRESHOLD))
		{
			atomic_set(&bma255->fast_calib_rslt, 0);
			GSE_LOG(KERN_INFO "===============shaking z===============\n");
			return -EINVAL;
		}
		else
		{
			acc_cal_pre.x = acc_cal.x;
			acc_cal_pre.y = acc_cal.y;
			acc_cal_pre.z = acc_cal.z;
		}
		timeout_shaking++;
	} while(timeout_shaking < 10);

	GSE_LOG(KERN_INFO "===============complete shaking z check===============\n");

	if (bma255_set_offset_target(bma255->client, 3, (unsigned char)data) < 0)
		return -EINVAL;

	if (bma255_set_cal_trigger(bma255->client, 3) < 0)
		return -EINVAL;

	do {
		mdelay(2);
		bma255_get_cal_ready(bma255->client, &tmp);

		GSE_LOG(KERN_INFO " z wait 2ms cal ready flag is %d\n", tmp);

		timeout++;
		if (timeout == 50)
		{
			GSE_LOG(KERN_INFO "get fast calibration ready error\n");
			return -EINVAL;
		}
	} while (tmp == 0);

	atomic_set(&bma255->fast_calib_z_rslt, 1);
	GSE_LOG(KERN_INFO "z axis fast calibration finished\n");

	return count;
}

static ssize_t bma255_bandwidth_show(struct device_driver *ddri, char *buf)
{
	unsigned char data;
	struct i2c_client *client = bma255_i2c_client;
	struct bma255_i2c_data *bma255 = i2c_get_clientdata(client);

	if (bma255_get_bandwidth(bma255->client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma255_bandwidth_store(struct device_driver *ddri, char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = bma255_i2c_client;
	struct bma255_i2c_data *bma255 = i2c_get_clientdata(client);

	if (error = strict_strtoul(buf, 10, &data))
		return error;

	tap_bandwidth = data;

	if (bma255_set_bandwidth(bma255->client, (unsigned char) data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma255_eeprom_writing_show(struct device_driver *ddri, char *buf)
{
	struct i2c_client *client = bma255_i2c_client;
	struct bma255_i2c_data *bma255 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma255->fast_calib_rslt));
}

static ssize_t bma255_eeprom_writing_store(struct device_driver *ddri, char *buf, size_t count)
{
	unsigned char offset_x,offset_y,offset_z;
	struct i2c_client *client = bma255_i2c_client;
	struct bma255_i2c_data *bma255 = i2c_get_clientdata(client);

	if(bma255_get_offset_x(bma255->client, &offset_x) < 0)
		return -EINVAL;
	if(bma255_get_offset_y(bma255->client, &offset_y) < 0)
		return -EINVAL;
	if(bma255_get_offset_z(bma255->client, &offset_z) < 0)
		return -EINVAL;

	atomic_set(&bma255->fast_calib_rslt, 1);

	return count;
}

static int bma255_soft_reset(struct i2c_client *client)
{
	int comres = 0;
	unsigned char data = BMA255_EN_SOFT_RESET_VALUE ;

	comres = bma255_smbus_write_byte(client, BMA255_EN_SOFT_RESET__REG, &data);

	return comres;
}

static ssize_t bma255_softreset_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = bma255_i2c_client;
	struct bma255_i2c_data *bma255 = i2c_get_clientdata(client);

	if (bma255_soft_reset(bma255->client) < 0)
		return -EINVAL;

	return count;
}
#endif

#ifdef KNOCK_ON
static int bma255_set_tap_threshold(struct i2c_client *client, unsigned char threshold)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_TAP_THRES__REG, &data);
	data = BMA255_SET_BITSLICE(data, BMA255_TAP_THRES, threshold);
	comres = bma255_smbus_write_byte(client, BMA255_TAP_THRES__REG, &data);

	tap_threshold = threshold;

	return comres;
}

static int bma255_get_tap_threshold(struct i2c_client *client, unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_TAP_THRES__REG, &data);
	data = BMA255_GET_BITSLICE(data, BMA255_TAP_THRES);

	*status = tap_threshold;

	return comres;
}

static ssize_t bma255_tap_threshold_show(struct device *dev,	struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct bma255_i2c_data *bma255 = obj_i2c_data;

	if (bma255_get_tap_threshold(bma255->client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma255_tap_threshold_store(struct device *dev,	const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct bma255_i2c_data *bma255 = obj_i2c_data;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma255_set_tap_threshold(bma255->client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static int bma255_set_tap_threshold_flat(struct i2c_client *client, unsigned char threshold)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_TAP_THRES__REG, &data);
	data = BMA255_SET_BITSLICE(data, BMA255_TAP_THRES, threshold);
	comres = bma255_smbus_write_byte(client, BMA255_TAP_THRES__REG, &data);

	tap_threshold_flat = threshold;

	return comres;
}

static int bma255_get_tap_threshold_flat(struct i2c_client *client, unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_TAP_THRES__REG, &data);
	data = BMA255_GET_BITSLICE(data, BMA255_TAP_THRES);

	*status = tap_threshold_flat;

	return comres;
}

static ssize_t bma255_tap_threshold_flat_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct bma255_i2c_data *bma255 = obj_i2c_data;

	if (bma255_get_tap_threshold_flat(bma255->client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma255_tap_threshold_flat_store(struct device *dev, const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct bma255_i2c_data *bma255 = obj_i2c_data;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma255_set_tap_threshold_flat(bma255->client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static int bma255_set_tap_quiet(struct i2c_client *client, unsigned char duration)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_TAP_QUIET_DURN__REG, &data);
	data = BMA255_SET_BITSLICE(data, BMA255_TAP_QUIET_DURN, duration);
	comres = bma255_smbus_write_byte(client, BMA255_TAP_QUIET_DURN__REG, &data);

	return comres;
}

static int bma255_get_tap_quiet(struct i2c_client *client, unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_TAP_TIMING, &data);
	data = BMA255_GET_BITSLICE(data, BMA255_TAP_QUIET_DURN);

	*status = data;

	return comres;
}

static ssize_t bma255_tap_quiet_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct bma255_i2c_data *bma255 = obj_i2c_data;

	if (bma255_get_tap_quiet(bma255->client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma255_tap_quiet_store(struct device *dev, const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct bma255_i2c_data *bma255 = obj_i2c_data;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma255_set_tap_quiet(bma255->client, (unsigned char)data) <	0)
		return -EINVAL;

	return count;
}

static int bma255_set_tap_duration(struct i2c_client *client, unsigned char duration)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_TAP_DUR__REG, &data);
	data = BMA255_SET_BITSLICE(data, BMA255_TAP_DUR, duration);
	comres = bma255_smbus_write_byte(client, BMA255_TAP_DUR__REG, &data);

	return comres;
}

static int bma255_get_tap_duration(struct i2c_client *client, unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_TAP_TIMING, &data);
	data = BMA255_GET_BITSLICE(data, BMA255_TAP_DUR);

	*status = data;

	return comres;
}

static ssize_t bma255_tap_duration_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct bma255_i2c_data *bma255 = obj_i2c_data;

	if (bma255_get_tap_duration(bma255->client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma255_tap_duration_store(struct device *dev, const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct bma255_i2c_data *bma2x2 =obj_i2c_data;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma255_set_tap_duration(bma2x2->client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static int bma255_set_tap_shock(struct i2c_client *client, unsigned char setval)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_TAP_SHOCK_DURN__REG,	&data);
	data = BMA255_SET_BITSLICE(data, BMA255_TAP_SHOCK_DURN, setval);
	comres = bma255_smbus_write_byte(client, BMA255_TAP_SHOCK_DURN__REG, &data);

	return comres;
}

static int bma255_get_tap_shock(struct i2c_client *client, unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_TAP_TIMING, &data);
	data = BMA255_GET_BITSLICE(data, BMA255_TAP_SHOCK_DURN);

	*status = data;

	return comres;
}

static ssize_t bma255_tap_shock_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct bma255_i2c_data *bma255 = obj_i2c_data;

	if (bma255_get_tap_shock(bma255->client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma255_tap_shock_store(struct device *dev, const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct bma255_i2c_data *bma255 = obj_i2c_data;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma255_set_tap_shock(bma255->client, (unsigned char)data) <	0)
		return -EINVAL;

	return count;
}

static int bma255_set_theta_flat(struct i2c_client *client, unsigned char thetaflat)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_FLAT_ANGLE__REG, &data);
	data = BMA255_SET_BITSLICE(data, BMA255_FLAT_ANGLE, thetaflat);
	comres = bma255_smbus_write_byte(client, BMA255_FLAT_ANGLE__REG, &data);

	return comres;
}

static int bma255_get_theta_flat(struct i2c_client *client, unsigned char *status)
{
	int comres = 0;
	unsigned char data;

	comres = bma255_smbus_read_byte(client, BMA255_FLAT_ANGLE__REG, &data);
	data = BMA255_GET_BITSLICE(data, BMA255_FLAT_ANGLE);

	*status = data;

	return comres;
}

static ssize_t bma255_flat_theta_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct bma255_i2c_data *bma255 = obj_i2c_data;

	if (bma255_get_theta_flat(bma255->client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma255_flat_theta_store(struct device *dev, const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct bma255_i2c_data *bma255 = obj_i2c_data;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;

	if (bma255_set_theta_flat(bma255->client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma255_tap_enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", doubletap_enable);
}

static ssize_t bma255_tap_enable_store(struct device *dev, const char *buf, size_t count)
{
	struct i2c_client *client = bma255_i2c_client;
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	GSE_LOG("Double Tap Enable value = %d \n", data);
	if (error)
		return error;

	if (data == 1)
	{
		bma255_set_Int_Mode(client, 1);
		bma255_set_Int_Enable(client, 9, 1);
		bma255_set_int1_pad_sel(client, PAD_DOUBLE_TAP);
		doubletap_enable = true;
	}
	else
	{
		bma255_set_Int_Mode(client, 0);
		bma255_set_Int_Enable(client, 9, 0);
		doubletap_enable = false;
	}

	return count;
}
#endif
/*----------------------------------------------------------------------------*/
static DRIVER_ATTR(chipinfo, S_IWUSR|S_IRUGO, show_chipinfo_value, NULL);
static DRIVER_ATTR(cpsdata, S_IWUSR|S_IRUGO, show_cpsdata_value, NULL);
static DRIVER_ATTR(cpsopmode, S_IWUSR|S_IRUGO|S_IWGRP, show_cpsopmode_value, store_cpsopmode_value);
static DRIVER_ATTR(cpsrange, S_IWUSR|S_IRUGO|S_IWGRP, show_cpsrange_value, store_cpsrange_value);
static DRIVER_ATTR(cpsbandwidth, S_IWUSR|S_IRUGO|S_IWGRP, show_cpsbandwidth_value, store_cpsbandwidth_value);
static DRIVER_ATTR(sensordata, S_IWUSR|S_IRUGO, show_sensordata_value, NULL);
static DRIVER_ATTR(cali, S_IWUSR|S_IRUGO|S_IWGRP, show_cali_value, store_cali_value);
static DRIVER_ATTR(firlen, S_IWUSR|S_IRUGO|S_IWGRP, show_firlen_value, store_firlen_value);
static DRIVER_ATTR(trace, S_IWUSR|S_IRUGO|S_IWGRP, show_trace_value, store_trace_value);
static DRIVER_ATTR(status, S_IRUGO, show_status_value, NULL);
static DRIVER_ATTR(powerstatus, S_IRUGO, show_power_status_value, NULL);
static DRIVER_ATTR(fast_calibration_x, S_IRUGO|S_IWUSR|S_IWGRP, bma255_fast_calibration_x_show, bma255_fast_calibration_x_store);
static DRIVER_ATTR(fast_calibration_y, S_IRUGO|S_IWUSR|S_IWGRP, bma255_fast_calibration_y_show, bma255_fast_calibration_y_store);
static DRIVER_ATTR(fast_calibration_z, S_IRUGO|S_IWUSR|S_IWGRP, bma255_fast_calibration_z_show, bma255_fast_calibration_z_store);
static DRIVER_ATTR(eeprom_writing, S_IRUGO|S_IWUSR|S_IWGRP, bma255_eeprom_writing_show, bma255_eeprom_writing_store);
static DRIVER_ATTR(bandwidth, S_IRUGO|S_IWUSR|S_IWGRP, bma255_bandwidth_show, bma255_bandwidth_store);
static DRIVER_ATTR(softreset, S_IWUSR|S_IWGRP, NULL, bma255_softreset_store);
#ifdef KNOCK_ON
static DRIVER_ATTR(tap_threshold, S_IRUGO|S_IWUSR|S_IWGRP, bma255_tap_threshold_show, bma255_tap_threshold_store);
static DRIVER_ATTR(tap_threshold_flat, S_IRUGO|S_IWUSR|S_IWGRP, bma255_tap_threshold_flat_show, bma255_tap_threshold_flat_store);
static DRIVER_ATTR(tap_quiet, S_IRUGO|S_IWUSR|S_IWGRP, bma255_tap_quiet_show, bma255_tap_quiet_store);
static DRIVER_ATTR(tap_duration, S_IRUGO|S_IWUSR|S_IWGRP, bma255_tap_duration_show, bma255_tap_duration_store);
static DRIVER_ATTR(tap_shock, S_IRUGO|S_IWUSR|S_IWGRP, bma255_tap_shock_show, bma255_tap_shock_store);
static DRIVER_ATTR(flat_theta, S_IRUGO|S_IWUSR|S_IWGRP, bma255_flat_theta_show, bma255_flat_theta_store);
static DRIVER_ATTR(tap_enable, S_IRUGO|S_IWUSR|S_IWGRP, bma255_tap_enable_show, bma255_tap_enable_store);
#endif
/*----------------------------------------------------------------------------*/
static struct driver_attribute *bma255_attr_list[] = {
	&driver_attr_chipinfo,     /*chip information*/
	&driver_attr_sensordata,   /*dump sensor data*/
	&driver_attr_cali,         /*show calibration data*/
	&driver_attr_firlen,       /*filter length: 0: disable, others: enable*/
	&driver_attr_trace,        /*trace log*/
	&driver_attr_status,
	&driver_attr_powerstatus,
	&driver_attr_cpsdata,	/*g sensor data for compass tilt compensation*/
	&driver_attr_cpsopmode,	/*g sensor opmode for compass tilt compensation*/
	&driver_attr_cpsrange,	/*g sensor range for compass tilt compensation*/
	&driver_attr_cpsbandwidth,	/*g sensor bandwidth for compass tilt compensation*/
    &driver_attr_fast_calibration_x,
    &driver_attr_fast_calibration_y,
    &driver_attr_fast_calibration_z,
    &driver_attr_eeprom_writing,
    &driver_attr_bandwidth,
    &driver_attr_softreset,
#ifdef KNOCK_ON
	&driver_attr_tap_threshold,
	&driver_attr_tap_threshold_flat,
	&driver_attr_tap_quiet,
	&driver_attr_tap_duration,
	&driver_attr_tap_shock,
	&driver_attr_flat_theta,
	&driver_attr_tap_enable,
#endif
};
/*----------------------------------------------------------------------------*/
static int bma255_create_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = (int)(sizeof(bma255_attr_list)/sizeof(bma255_attr_list[0]));
	if (driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		if(err = driver_create_file(driver, bma255_attr_list[idx]))
		{
			GSE_ERR("driver_create_file (%s) = %d\n", bma255_attr_list[idx]->attr.name, err);
			break;
		}
	}
	return err;
}
/*----------------------------------------------------------------------------*/
static int bma255_delete_attr(struct device_driver *driver)
{
	int idx ,err = 0;
	int num = (int)(sizeof(bma255_attr_list)/sizeof(bma255_attr_list[0]));

	if(driver == NULL)
	{
		return -EINVAL;
	}

	for(idx = 0; idx < num; idx++)
	{
		driver_remove_file(driver, bma255_attr_list[idx]);
	}

	return err;
}

/*----------------------------------------------------------------------------*/
int gsensor_operate(void* self, uint32_t command, void* buff_in, int size_in, void* buff_out, int size_out, int* actualout)
{
	int err = 0;
	int value, sample_delay;
	struct bma255_i2c_data *priv = (struct bma255_i2c_data*)self;
	hwm_sensor_data* gsensor_data;
	char buff[BMA255_BUFSIZE];

	//GSE_FUN(f);
	switch (command)
	{
		case SENSOR_DELAY:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Set delay parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;
				if(value <= 5)
				{
					sample_delay = BMA255_BW_200HZ;
				}
				else if(value <= 10)
				{
					sample_delay = BMA255_BW_100HZ;
				}
				else
				{
					sample_delay = BMA255_BW_50HZ;
				}

				err = BMA255_SetBWRate(priv->client, sample_delay);
				if(err != BMA255_SUCCESS ) //0x2C->BW=100Hz
				{
					GSE_ERR("Set delay parameter error!\n");
				}

				if(value >= 50)
				{
					atomic_set(&priv->filter, 0);
				}
				else
				{
				#if defined(CONFIG_BMA255_LOWPASS)
					priv->fir.num = 0;
					priv->fir.idx = 0;
					priv->fir.sum[BMA255_AXIS_X] = 0;
					priv->fir.sum[BMA255_AXIS_Y] = 0;
					priv->fir.sum[BMA255_AXIS_Z] = 0;
					atomic_set(&priv->filter, 1);
				#endif
				}
			}
			break;

		case SENSOR_ENABLE:
			if((buff_in == NULL) || (size_in < sizeof(int)))
			{
				GSE_ERR("Enable sensor parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				value = *(int *)buff_in;
				if(((value == 0) && (sensor_power == false)) || ((value == 1) && (sensor_power == true)))
				{
					GSE_LOG("Gsensor device have updated!, power: %d\n", sensor_power);
				}
				else
				{
					err = BMA255_SetPowerMode( priv->client, !sensor_power);
				}
			}
			break;

		case SENSOR_GET_DATA:
			if((buff_out == NULL) || (size_out< sizeof(hwm_sensor_data)))
			{
				GSE_ERR("get sensor data parameter error!\n");
				err = -EINVAL;
			}
			else
			{
				gsensor_data = (hwm_sensor_data *)buff_out;
				BMA255_ReadSensorData(priv->client, buff, BMA255_BUFSIZE);
				sscanf(buff, "%x %x %x", &gsensor_data->values[0], &gsensor_data->values[1], &gsensor_data->values[2]);
				gsensor_data->status = SENSOR_STATUS_ACCURACY_MEDIUM;
				gsensor_data->value_divide = 1000;
			}
			break;

		default:
			GSE_ERR("gsensor operate function no this parameter %d!\n", command);
			err = -1;
			break;
	}

	return err;
}

/******************************************************************************
 * Function Configuration
******************************************************************************/
static int bma255_open(struct inode *inode, struct file *file)
{
	file->private_data = bma255_i2c_client;

	if(file->private_data == NULL)
	{
		GSE_ERR("null pointer!!\n");
		return -EINVAL;
	}
	return nonseekable_open(inode, file);
}
/*----------------------------------------------------------------------------*/
static int bma255_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}
/*----------------------------------------------------------------------------*/
static long bma255_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct i2c_client *client = (struct i2c_client*)file->private_data;
	struct bma255_i2c_data *obj = (struct bma255_i2c_data*)i2c_get_clientdata(client);
	char strbuf[BMA255_BUFSIZE];
	void __user *data;
	SENSOR_DATA sensor_data;
	long err = 0;
	int cali[3];

	//GSE_FUN(f);
	if(_IOC_DIR(cmd) & _IOC_READ)
	{
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	}
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
	{
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	}

	if(err)
	{
		GSE_ERR("access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
		return -EFAULT;
	}

	switch(cmd)
	{
		case GSENSOR_IOCTL_INIT:
			bma255_init_client(client, 0);
			break;

		case GSENSOR_IOCTL_READ_CHIPINFO:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}

			BMA255_ReadChipInfo(client, strbuf, BMA255_BUFSIZE);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;
			}
			break;

		case GSENSOR_IOCTL_READ_SENSORDATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}

			BMA255_ReadSensorData(client, strbuf, BMA255_BUFSIZE);
			if(copy_to_user(data, strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;
			}
			break;

		case GSENSOR_IOCTL_READ_GAIN:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}

			if(copy_to_user(data, &gsensor_gain, sizeof(GSENSOR_VECTOR3D)))
			{
				err = -EFAULT;
				break;
			}
			break;

		case GSENSOR_IOCTL_READ_RAW_DATA:
			data = (void __user *) arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}
			BMA255_ReadRawData(client, strbuf);
			if(copy_to_user(data, &strbuf, strlen(strbuf)+1))
			{
				err = -EFAULT;
				break;
			}
			break;

		case GSENSOR_IOCTL_SET_CALI:
			data = (void __user*)arg;
			if(data == NULL)
			{
				err = -EINVAL;
				break;
			}
			if(copy_from_user(&sensor_data, data, sizeof(sensor_data)))
			{
				err = -EFAULT;
				break;
			}

			//                                                            
			if(bma255_set_offset_x(obj->client, (unsigned char)sensor_data.x) < 0)
				return -EINVAL;
			if(bma255_set_offset_y(obj->client, (unsigned char)sensor_data.y) < 0)
				return -EINVAL;
			if(bma255_set_offset_z(obj->client, (unsigned char)sensor_data.z) < 0)
				return -EINVAL;
			//                                                            
			break;

		case GSENSOR_IOCTL_CLR_CALI:
			err = BMA255_ResetCalibration(client);
			break;

		case GSENSOR_IOCTL_GET_CALI:
			GSE_LOG("GSENSOR_IOCTL_GET_CALI\n");
			break;

		default:
			GSE_ERR("unknown IOCTL: 0x%08x\n", cmd);
			err = -ENOIOCTLCMD;
			break;
	}

	return err;
}

/*----------------------------------------------------------------------------*/
static struct file_operations bma255_fops = {
	//.owner = THIS_MODULE,
	.open = bma255_open,
	.release = bma255_release,
	.unlocked_ioctl = bma255_unlocked_ioctl,
};
/*----------------------------------------------------------------------------*/
static struct miscdevice bma255_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gsensor",
	.fops = &bma255_fops,
};
/*----------------------------------------------------------------------------*/
#ifndef CONFIG_HAS_EARLYSUSPEND
/*----------------------------------------------------------------------------*/
static int bma255_suspend(struct i2c_client *client, pm_message_t msg)
{
	GSE_FUN();

	return 0;
}
/*----------------------------------------------------------------------------*/
static int bma255_resume(struct i2c_client *client)
{
	GSE_FUN();

	return 0;
}
/*----------------------------------------------------------------------------*/
#else /*CONFIG_HAS_EARLY_SUSPEND is defined*/
/*----------------------------------------------------------------------------*/
static void bma255_early_suspend(struct early_suspend *h)
{
	GSE_FUN();
#ifdef KNOCK_ON
	int err;
	struct i2c_client *client = bma255_i2c_client;

	err = BMA255_SetBWRate(client, tap_bandwidth);
	if(err != BMA255_SUCCESS)
	{
		GSE_ERR("Set delay parameter error!\n");
	}

	sensor_suspend = true;
#endif
}
/*----------------------------------------------------------------------------*/
static void bma255_late_resume(struct early_suspend *h)
{
	GSE_FUN();
#ifdef KNOCK_ON
	struct i2c_client *client = bma255_i2c_client;

	sensor_suspend = false;

	bma255_set_Int_Mode(client, 0);
	bma255_set_Int_Enable(client, 9, 0);
#endif
}
/*----------------------------------------------------------------------------*/
#ifdef KNOCK_ON
#define to_foo_obj(x) container_of(x, struct foo_obj, kobj)
struct foo_attribute {
	struct attribute attr;
	ssize_t (*show)(struct foo_obj *foo, struct foo_attribute *attr, char *buf);
	ssize_t (*store)(struct foo_obj *foo, struct foo_attribute *attr, const char *buf, size_t count);
};
#define to_foo_attr(x) container_of(x, struct foo_attribute, attr)
static ssize_t foo_attr_show(struct kobject *kobj,
			     struct attribute *attr,
			     char *buf){
		struct foo_attribute *attribute;
	struct foo_obj *foo;

	attribute = to_foo_attr(attr);
	foo = to_foo_obj(kobj);

	if (!attribute->show)
		return -EIO;

	return attribute->show(foo, attribute, buf);
}
static ssize_t foo_attr_store(struct kobject *kobj,
			      struct attribute *attr,
			      const char *buf, size_t len)
{
	struct foo_attribute *attribute;
	struct foo_obj *foo;

	attribute = to_foo_attr(attr);
	foo = to_foo_obj(kobj);

	if (!attribute->store)
		return -EIO;

	return attribute->store(foo, attribute, buf, len);
}
static const struct sysfs_ops foo_sysfs_ops = {
	.show = foo_attr_show,
	.store = foo_attr_store,
};
static void foo_release(struct kobject *kobj)
{
	struct foo_obj *foo;

	foo = to_foo_obj(kobj);
	kfree(foo);
}

static ssize_t foo_show(struct foo_obj *foo_obj, struct foo_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", foo_obj->intterupt);
}
static ssize_t foo_store(struct foo_obj *foo_obj, struct foo_attribute *attr,
			 const char *buf, size_t count)
{
	sscanf(buf, "%du", &foo_obj->intterupt);
	return count;
}
static struct foo_attribute foo_attribute =__ATTR(interrupt, 0664, foo_show, foo_store);
static struct attribute *foo_default_attrs[] = {
	&foo_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};
static struct kobj_type foo_ktype = {
	.sysfs_ops = &foo_sysfs_ops,
	.release = foo_release,
	.default_attrs = foo_default_attrs,
};
static struct kset *example_kset;

static struct foo_obj *create_foo_obj(const char *name){
	struct foo_obj *foo;
	int retval;
	foo = kzalloc(sizeof(*foo), GFP_KERNEL);
	if (!foo)
		return NULL;
	foo->kobj.kset = example_kset;
	retval = kobject_init_and_add(&foo->kobj, &foo_ktype, NULL, "%s", name);
	if (retval) {
		kobject_put(&foo->kobj);
		return NULL;
	}
	kobject_uevent(&foo->kobj, KOBJ_ADD);
	return foo;
}
#endif

/*----------------------------------------------------------------------------*/
#endif /*CONFIG_HAS_EARLYSUSPEND*/
/*----------------------------------------------------------------------------*/
static int bma255_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct i2c_client *new_client;
	struct bma255_i2c_data *obj;
	struct hwmsen_object sobj;
	int err = 0;
#ifdef KNOCK_ON
	struct foo_obj *foo;
	unsigned char data;
#endif
	GSE_FUN();

	if(!(obj = kzalloc(sizeof(*obj), GFP_KERNEL)))
	{
		err = -ENOMEM;
		goto exit;
	}
#ifdef KNOCK_ON
	example_kset = kset_create_and_add("lge", NULL, kernel_kobj);
	foo_obj = create_foo_obj(LGE_GSENSOR_NAME);
#endif
	
	memset(obj, 0, sizeof(struct bma255_i2c_data));

	obj->hw = get_cust_acc_hw();

	if(err = hwmsen_get_convert(obj->hw->direction, &obj->cvt))
	{
		GSE_ERR("invalid direction: %d\n", obj->hw->direction);
		goto exit;
	}

	obj_i2c_data = obj;
	obj->client = client;
	new_client = obj->client;
	i2c_set_clientdata(new_client,obj);

	atomic_set(&obj->trace, 0);
	atomic_set(&obj->suspend, 0);

#ifdef CONFIG_BMA255_LOWPASS
	if(obj->hw->firlen > C_MAX_FIR_LENGTH)
	{
		atomic_set(&obj->firlen, C_MAX_FIR_LENGTH);
	}
	else
	{
		atomic_set(&obj->firlen, obj->hw->firlen);
	}

	if(atomic_read(&obj->firlen) > 0)
	{
		atomic_set(&obj->fir_en, 1);
	}

#endif

	bma255_i2c_client = new_client;

	if(err = bma255_init_client(new_client, 1))
	{
		GSE_ERR ( "failed to init BMA255 ( err = %d )\n", err );
		goto exit_init_failed;
	}

	if(err = misc_register(&bma255_device))
	{
		GSE_ERR("bma255_device register failed\n");
		goto exit_misc_device_register_failed;
	}

	if(err = bma255_create_attr(&bma255_gsensor_driver.driver))
	{
		GSE_ERR("create attribute err = %d\n", err);
		goto exit_create_attr_failed;
	}

	sobj.self = obj;
    sobj.polling = 1;
    sobj.sensor_operate = gsensor_operate;
	if(err = hwmsen_attach(ID_ACCELEROMETER, &sobj))
	{
		GSE_ERR("attach fail = %d\n", err);
		goto exit_kfree;
	}
#ifdef KNOCK_ON
	if(err < 0)
	{
		GSE_ERR("kobject_init_and_add fail = %d\n", err);
	}	
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
	obj->early_drv.level    = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1,
	obj->early_drv.suspend  = bma255_early_suspend,
	obj->early_drv.resume   = bma255_late_resume,
	register_early_suspend(&obj->early_drv);
#endif

#ifdef KNOCK_ON
	bma255_wq = create_singlethread_workqueue("bma255_wq");
	INIT_DELAYED_WORK(&bma255_work, bma255_work_func);

	bma255_setup_eint();

	data = 0x06;
	bma255_smbus_write_byte(client, BMA255_TAP_TIMING, &data);
	data = 0xC1;
	bma255_smbus_write_byte(client, BMA255_TAP_THRES__REG, &data);
	data = 0x09; //15 degree
	bma255_smbus_write_byte(client, BMA255_FLAT_ANGLE__REG, &data);
#endif

	GSE_LOG("%s: OK\n", __func__);
	return 0;

	exit_create_attr_failed:
	misc_deregister(&bma255_device);
	exit_misc_device_register_failed:
	exit_init_failed:
	//i2c_detach_client(new_client);
	exit_kfree:
	kfree(obj);
	exit:
	GSE_ERR("%s: err = %d\n", __func__, err);
	return err;
}

/*----------------------------------------------------------------------------*/
static int bma255_i2c_remove(struct i2c_client *client)
{
	int err = 0;

	if(err = bma255_delete_attr(&bma255_gsensor_driver.driver))
	{
		GSE_ERR("bma150_delete_attr fail: %d\n", err);
	}

	if(err = misc_deregister(&bma255_device))
	{
		GSE_ERR("misc_deregister fail: %d\n", err);
	}

	if(err = hwmsen_detach(ID_ACCELEROMETER))


	bma255_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(i2c_get_clientdata(client));
	return 0;
}
/*----------------------------------------------------------------------------*/
static int bma255_probe(struct platform_device *pdev)
{
	struct acc_hw *hw = get_cust_acc_hw();

	GSE_FUN();

	BMA255_power(hw, 1);
	msleep ( 10 );
	if(i2c_add_driver(&bma255_i2c_driver))
	{
		GSE_ERR("add driver error\n");
		return -1;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
#ifdef KNOCK_ON
static void bma255_shutdown(struct platform_device *dev)
{
	GSE_ERR("############## BMA255 SHUTDOWN###############\n");
	if (!hwPowerDown(MT65XX_POWER_LDO_VCAM_AF, "BMA255"))
	{
		GSE_ERR("power off fail!!\n");
	}
}
#endif
/*----------------------------------------------------------------------------*/
static int bma255_remove(struct platform_device *pdev)
{
    struct acc_hw *hw = get_cust_acc_hw();

    GSE_FUN();
    BMA255_power(hw, 0);
    i2c_del_driver(&bma255_i2c_driver);
    return 0;
}
/*----------------------------------------------------------------------------*/
static struct platform_driver bma255_gsensor_driver = {
	.probe      = bma255_probe,
	.remove     = bma255_remove,
#ifdef KNOCK_ON
	.shutdown 	= bma255_shutdown,
#endif
	.driver     = {
		.name  = "gsensor",
	}
};

/*----------------------------------------------------------------------------*/
static int __init bma255_init(void)
{
	struct acc_hw *hw = get_cust_acc_hw();

	GSE_FUN();
	i2c_register_board_info(hw->i2c_num, &bma255_i2c_info, 1);
	if(platform_driver_register(&bma255_gsensor_driver))
	{
		GSE_ERR("failed to register driver");
		return -ENODEV;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
static void __exit bma255_exit(void)
{
	GSE_FUN();
	platform_driver_unregister(&bma255_gsensor_driver);
}
/*----------------------------------------------------------------------------*/
module_init(bma255_init);
module_exit(bma255_exit);
/*----------------------------------------------------------------------------*/
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("BMA255 I2C driver");
MODULE_AUTHOR("hongji.zhou@bosch-sensortec.com");