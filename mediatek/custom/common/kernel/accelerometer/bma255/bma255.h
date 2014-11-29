/* BMA255 motion sensor driver
 *
 *
 * This software program is licensed subject to the GNU General Public License
 * (GPL).Version 2,June 1991, available at http://www.fsf.org/copyleft/gpl.html

 * (C) Copyright 2011 Bosch Sensortec GmbH
 * All Rights Reserved
 */

#ifndef BMA255_H
#define BMA255_H

#include <linux/ioctl.h>

/* 7-bit addr: 0x10 (SDO connected to GND); 0x11 (SDO connected to VDDIO) */
#ifdef MT6575
#define BMA255_I2C_ADDR_REV_A		0x10
#else
#define BMA255_I2C_ADDR_EVB			0x10
#define BMA255_I2C_ADDR_REV_A		0x11
#endif
/* chip ID */
#define BMA255_FIXED_DEVID			0xFA

 /* BMA255 Register Map  (Please refer to BMA255 Specifications) */
#define BMA255_REG_DEVID			0x00
#define BMA255_REG_OFSX				0x16
#define BMA255_REG_OFSX_HIGH		0x1A
#define BMA255_REG_BW_RATE			0x10
#define BMA255_BW_MASK				0x1f
#define BMA255_BW_200HZ				0x0d
#define BMA255_BW_100HZ				0x0c
#define BMA255_BW_50HZ				0x0b
#define BMA255_BW_25HZ				0x0a
#define BMA255_REG_POWER_CTL		0x11
#define BMA255_REG_DATA_FORMAT		0x0f
#define BMA255_RANGE_MASK			0x0f
#define BMA255_RANGE_2G				0x03
#define BMA255_RANGE_4G				0x05
#define BMA255_RANGE_8G				0x08
#define BMA255_REG_DATAXLOW			0x02
#define BMA255_REG_DATA_RESOLUTION	0x14
#define BMA255_MEASURE_MODE			0x80
#define BMA255_SELF_TEST           	0x32
#define BMA255_SELF_TEST_AXIS_X		0x01
#define BMA255_SELF_TEST_AXIS_Y		0x02
#define BMA255_SELF_TEST_AXIS_Z		0x03
#define BMA255_SELF_TEST_POSITIVE	0x00
#define BMA255_SELF_TEST_NEGATIVE	0x04
#define BMA255_INT_REG_1           	0x16
#define BMA255_INT_REG_2          	0x17

//                                                      
#define BMA255_X_AXIS_LSB_REG                   0x02
#define BMA255_X_AXIS_MSB_REG                   0x03
#define BMA255_Y_AXIS_LSB_REG                   0x04
#define BMA255_Y_AXIS_MSB_REG                   0x05
#define BMA255_Z_AXIS_LSB_REG                   0x06
#define BMA255_Z_AXIS_MSB_REG                   0x07

#define BMA255_RESET_REG                        0x14
#define BMA255_EEPROM_CTRL_REG                  0x33
#define BMA255_OFFSET_CTRL_REG                  0x36
#define BMA255_OFFSET_PARAMS_REG                0x37
#define BMA255_OFFSET_X_AXIS_REG                0x38
#define BMA255_OFFSET_Y_AXIS_REG                0x39
#define BMA255_OFFSET_Z_AXIS_REG                0x3A
//                                                      

#if 1 // ADD_DOUBLE_TAP
#define BMA255_STATUS1_REG                      0x09
#define BMA255_STATUS2_REG                      0x0A
#define BMA255_INT1_PAD_SEL_REG                 0x19
#define BMA255_INT_CTRL_REG                     0x21

#define PAD_LOWG					0
#define PAD_HIGHG					1
#define PAD_SLOP					2
#define PAD_DOUBLE_TAP					3
#define PAD_SINGLE_TAP					4
#define PAD_ORIENT					5
#define PAD_FLAT					6
#define PAD_SLOW_NO_MOTION				7

static int bma255_smbus_read_byte(struct i2c_client *client, unsigned char reg_addr, unsigned char *data);
static int bma255_smbus_write_byte(struct i2c_client *client, unsigned char reg_addr, unsigned char *data);

#endif

#define BMA255_SUCCESS					0
#define BMA255_ERR_I2C					-1
#define BMA255_ERR_STATUS				-3
#define BMA255_ERR_SETUP_FAILURE		-4
#define BMA255_ERR_GETGSENSORDATA		-5
#define BMA255_ERR_IDENTIFICATION		-6

#define BMA255_BUFSIZE					256

#endif

