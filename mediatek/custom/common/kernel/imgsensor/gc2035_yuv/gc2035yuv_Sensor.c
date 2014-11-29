/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.c
 *
 * Project:
 * --------
 *  
 *
 * Description:
 * ------------
 *   Source code of Sensor driver
 *
 *
 * Author:
 * -------
 *   Leo Lee
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * [GC2035YUV V1.0.0]
 * 8.17.2012 Leo.Lee
 * .First Release
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by GalaxyCoreinc. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
//#include <windows.h>
//#include <memory.h>
//#include <nkintr.h>
//#include <ceddk.h>
//#include <ceddk_exp.h>

//#include "kal_release.h"
//#include "i2c_exp.h"
//#include "gpio_exp.h"
//#include "msdk_exp.h"
//#include "msdk_sensor_exp.h"
//#include "msdk_isp_exp.h"
//#include "base_regs.h"
//#include "Sensor.h"
//#include "camera_sensor_para.h"
//#include "CameraCustomized.h"

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
//#include <mach/mt6516_pll.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"

#include "gc2035yuv_Sensor.h"
#include "gc2035yuv_Camera_Sensor_para.h"
#include "gc2035yuv_CameraCustomized.h"

#define GC2035YUV_DEBUG
#ifdef GC2035YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

#define  GC2035_SET_PAGE0    GC2035_write_cmos_sensor(0xfe,0x00)
#define  GC2035_SET_PAGE1    GC2035_write_cmos_sensor(0xfe,0x01)
#define  GC2035_SET_PAGE2    GC2035_write_cmos_sensor(0xfe,0x02)
#define  GC2035_SET_PAGE3    GC2035_write_cmos_sensor(0xfe,0x03)

//#define GC2035_MIPI

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
/*************************************************************************
* FUNCTION
*    GC2035_write_cmos_sensor
*
* DESCRIPTION
*    This function wirte data to CMOS sensor through I2C
*
* PARAMETERS
*    addr: the 16bit address of register
*    para: the 8bit value of register
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void GC2035_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
kal_uint8 out_buff[2];

    out_buff[0] = addr;
    out_buff[1] = para;

    iWriteRegI2C((u8*)out_buff , (u16)sizeof(out_buff), GC2035_WRITE_ID); 

#if (defined(__GC2035_DEBUG_TRACE__))
  if (sizeof(out_buff) != rt) printk("I2C write %x, %x error\n", addr, para);
#endif
}

/*************************************************************************
* FUNCTION
*    GC2035_read_cmos_sensor
*
* DESCRIPTION
*    This function read data from CMOS sensor through I2C.
*
* PARAMETERS
*    addr: the 16bit address of register
*
* RETURNS
*    8bit data read through I2C
*
* LOCAL AFFECTED
*
*************************************************************************/
static kal_uint8 GC2035_read_cmos_sensor(kal_uint8 addr)
{
  kal_uint8 in_buff[1] = {0xFF};
  kal_uint8 out_buff[1];
  
  out_buff[0] = addr;

    if (0 != iReadRegI2C((u8*)out_buff , (u16) sizeof(out_buff), (u8*)in_buff, (u16) sizeof(in_buff), GC2035_WRITE_ID)) {
        SENSORDB("ERROR: GC2035_read_cmos_sensor \n");
    }

#if (defined(__GC2035_DEBUG_TRACE__))
  if (size != rt) printk("I2C read %x error\n", addr);
#endif

  return in_buff[0];
}


/*******************************************************************************
* // Adapter for Winmo typedef 
********************************************************************************/
#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT


/*******************************************************************************
* // End Adapter for Winmo typedef 
********************************************************************************/
/* Global Valuable */

static kal_uint32 zoom_factor = 0; 

static kal_bool GC2035_VEDIO_encode_mode = KAL_FALSE; //Picture(Jpeg) or Video(Mpeg4)
static kal_bool GC2035_sensor_cap_state = KAL_FALSE; //Preview or Capture

static kal_uint16 GC2035_exposure_lines=0, GC2035_extra_exposure_lines = 0;

static kal_uint16 GC2035_Capture_Shutter=0;
static kal_uint16 GC2035_Capture_Extra_Lines=0;

kal_uint32 GC2035_capture_pclk_in_M=520,GC2035_preview_pclk_in_M=390,GC2035_PV_dummy_pixels=0,GC2035_PV_dummy_lines=0,GC2035_isp_master_clock=0;

static kal_uint32  GC2035_sensor_pclk=390;

static kal_uint32 Preview_Shutter = 0;
static kal_uint32 Capture_Shutter = 0;

MSDK_SENSOR_CONFIG_STRUCT GC2035SensorConfigData;

static void GC2035_write_shutter(kal_uint16 shutter)
{
	SENSORDB("GC2035_write_shutter \r\n");

	if(shutter<1) shutter = 1;
	GC2035_write_cmos_sensor(0x04, shutter & 0xff);
	GC2035_write_cmos_sensor(0x03, shutter <<8);
}    /* GC2035_write_shutter */

kal_uint16 GC2035_read_shutter(void)
{
    	kal_uint8 temp_reg1, temp_reg2;
	kal_uint16 shutter;
	SENSORDB("GC2035_read_shutter \r\n");

	temp_reg1 = GC2035_read_cmos_sensor(0x04);
	temp_reg2 = GC2035_read_cmos_sensor(0x03);

	shutter = (temp_reg1 & 0xFF) | (temp_reg2 << 8);
	
	return shutter;
} /* GC2035 read_shutter */


static void GC2035_set_mirror_flip(kal_uint8 image_mirror)
{
	kal_uint8 GC2035_HV_Mirror;
	SENSORDB("GC2035_set_mirror_flip \r\n");

//	switch (image_mirror=IMAGE_HV_MIRROR) 
	switch (image_mirror) 
	{
		case IMAGE_NORMAL:
			GC2035_HV_Mirror = 0x14; 
		    break;
		case IMAGE_H_MIRROR:
			GC2035_HV_Mirror = 0x15;
		    break;
		case IMAGE_V_MIRROR:
			GC2035_HV_Mirror = 0x16; 
		    break;
		case IMAGE_HV_MIRROR:
			GC2035_HV_Mirror = 0x17;
		    break;
		default:
		    break;
	}
	GC2035_write_cmos_sensor(0x17, GC2035_HV_Mirror);
}

static void GC2035_set_AE_mode(kal_bool AE_enable)
{
	kal_uint8 temp_AE_reg = 0;
	SENSORDB("GC2035_set_AE_mode \r\n");

	GC2035_write_cmos_sensor(0xfe, 0x00);
	if (AE_enable == KAL_TRUE)
	{
		// turn on AEC/AGC
		GC2035_write_cmos_sensor(0xb6, 0x03);
	}
	else
	{
		// turn off AEC/AGC
		GC2035_write_cmos_sensor(0xb6, 0x00);
	}
}


static void GC2035_set_AWB_mode(kal_bool AWB_enable)
{
	kal_uint8 temp_AWB_reg = 0;
	SENSORDB("GC2035_set_AWB_mode \r\n");

	GC2035_write_cmos_sensor(0xfe, 0x00);
	if (AWB_enable == KAL_TRUE)
	{
		//enable Auto WB
		GC2035_write_cmos_sensor(0x82, 0xfe);
	}
	else
	{
		//turn off AWB
		GC2035_write_cmos_sensor(0x82, 0xfc);
	}
}


/*************************************************************************
* FUNCTION
*	GC2035_night_mode
*
* DESCRIPTION
*	This function night mode of GC2035.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void GC2035_night_mode(kal_bool enable)
{
	SENSORDB("GC2035_night_mode \r\n");
	
		/* ==Video Preview, Auto Mode, use 39MHz PCLK, 30fps; Night Mode use 39M, 15fps */
		if (GC2035_sensor_cap_state == KAL_FALSE) 
		{
			if (enable) 
			{
				if (GC2035_VEDIO_encode_mode == KAL_TRUE) 
				{
					GC2035_write_cmos_sensor(0xfe, 0x01);
					GC2035_write_cmos_sensor(0x3e, 0x60);
					GC2035_write_cmos_sensor(0xfe, 0x00);
				}
				else 
				{
					GC2035_write_cmos_sensor(0xfe, 0x01);
					GC2035_write_cmos_sensor(0x3e, 0x60);
					GC2035_write_cmos_sensor(0xfe, 0x00);
				}
			}
			else 
			{
				/* when enter normal mode (disable night mode) without light, the AE vibrate */
				if (GC2035_VEDIO_encode_mode == KAL_TRUE) 
				{
					GC2035_write_cmos_sensor(0xfe, 0x01);
					GC2035_write_cmos_sensor(0x3e, 0x40);
					GC2035_write_cmos_sensor(0xfe, 0x00);
				}
				else 
				{
					GC2035_write_cmos_sensor(0xfe, 0x01);
					GC2035_write_cmos_sensor(0x3e, 0x40);
					GC2035_write_cmos_sensor(0xfe, 0x00);
				}
		}
	}
}	/* GC2035_night_mode */



/*************************************************************************
* FUNCTION
*	GC2035_GetSensorID
*
* DESCRIPTION
*	This function get the sensor ID
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 GC2035_GetSensorID(kal_uint32 *sensorID)
{
	SENSORDB("GC2035_GetSensorID \r\n");

    int  retry = 3; 
    // check if sensor ID correct
    do {
        *sensorID=((GC2035_read_cmos_sensor(0xf0)<< 8)|GC2035_read_cmos_sensor(0xf1));
	printk("GC2035 Sensor id = %x\n", *sensorID);

        if (*sensorID == GC2035_SENSOR_ID)
            break; 
        SENSORDB("Read Sensor ID Fail = 0x%04x\n", *sensorID); 
        retry--; 
    } while (retry > 0);

    if (*sensorID != GC2035_SENSOR_ID) {
        *sensorID = 0xFFFFFFFF; 
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    return ERROR_NONE;    
}   /* GC2035Open  */

static void GC2035_Sensor_Init(void)
{
	zoom_factor = 0; 
	SENSORDB("GC2035_Sensor_Init");
	GC2035_write_cmos_sensor(0xfe, 0x80);  
	GC2035_write_cmos_sensor(0xfe, 0x80);  
	GC2035_write_cmos_sensor(0xfe, 0x80);  
	GC2035_write_cmos_sensor(0xfc , 0x06);
	GC2035_write_cmos_sensor(0xf2 , 0x00);
	GC2035_write_cmos_sensor(0xf3 , 0x00);
	GC2035_write_cmos_sensor(0xf4 , 0x00);
	GC2035_write_cmos_sensor(0xf5 , 0x00);
	GC2035_write_cmos_sensor(0xf9, 0xfe);  
	GC2035_write_cmos_sensor(0xfa, 0x00);  
	GC2035_write_cmos_sensor(0xf6, 0x00);  
	GC2035_write_cmos_sensor(0xf7, 0x05);  
	GC2035_write_cmos_sensor(0xf8, 0x85);  
	GC2035_write_cmos_sensor(0xfe, 0x00);  
	GC2035_write_cmos_sensor(0x82, 0x00);  
	GC2035_write_cmos_sensor(0xb3, 0x60);  
	GC2035_write_cmos_sensor(0xb4 , 0x40);
	GC2035_write_cmos_sensor(0xb5 , 0x60);
	GC2035_write_cmos_sensor(0x03 , 0x05);
	GC2035_write_cmos_sensor(0x04 , 0x2e);
	GC2035_write_cmos_sensor(0xfe, 0x00);  
	GC2035_write_cmos_sensor(0xec, 0x04);  
	GC2035_write_cmos_sensor(0xed, 0x04);  
	GC2035_write_cmos_sensor(0xee, 0x60);  
	GC2035_write_cmos_sensor(0xef, 0x90);  
	GC2035_write_cmos_sensor(0x0a, 0x00);  
	GC2035_write_cmos_sensor(0x0c, 0x00);  
	GC2035_write_cmos_sensor(0x0d, 0x04);  
	GC2035_write_cmos_sensor(0x0e, 0xc0);  
	GC2035_write_cmos_sensor(0x0f, 0x06);  
	GC2035_write_cmos_sensor(0x10, 0x58);  
	GC2035_write_cmos_sensor(0x17, 0x14); //14 
	GC2035_write_cmos_sensor(0x18, 0x0a);  
	GC2035_write_cmos_sensor(0x19, 0x0c);  
	GC2035_write_cmos_sensor(0x1a , 0x01); //CISCTL mode4
	GC2035_write_cmos_sensor(0x1b , 0x48);
	GC2035_write_cmos_sensor(0x1e , 0x88); //
	GC2035_write_cmos_sensor(0x1f , 0x08); //[3] tx-low en
	GC2035_write_cmos_sensor(0x20, 0x05);  
	GC2035_write_cmos_sensor(0x21, 0x0f);  
	GC2035_write_cmos_sensor(0x22, 0xf0);  
	GC2035_write_cmos_sensor(0x23, 0xc3);  
	GC2035_write_cmos_sensor(0x24, 0x16);  
	GC2035_write_cmos_sensor(0xfe , 0x01);
	GC2035_write_cmos_sensor(0x11 , 0x20);//
	GC2035_write_cmos_sensor(0x1f , 0xc0);//
	GC2035_write_cmos_sensor(0x20 , 0x60);//
	GC2035_write_cmos_sensor(0x47 , 0x30);//
	GC2035_write_cmos_sensor(0x0b , 0x10);//
	GC2035_write_cmos_sensor(0x13 , 0x75);//y_target
	GC2035_write_cmos_sensor(0xfe , 0x00);
	GC2035_write_cmos_sensor(0x05 , 0x00);//hb                           
	GC2035_write_cmos_sensor(0x06 , 0xea);                               
	GC2035_write_cmos_sensor(0x07 , 0x00);//vb                           
	GC2035_write_cmos_sensor(0x08 , 0x7e);                               
	GC2035_write_cmos_sensor(0xfe, 0x01);  
	GC2035_write_cmos_sensor(0x27, 0x00);//
	GC2035_write_cmos_sensor(0x28, 0xb9);  
	GC2035_write_cmos_sensor(0x29, 0x05);//
	GC2035_write_cmos_sensor(0x2a, 0x0f);  
	GC2035_write_cmos_sensor(0x2b, 0x05);//
	GC2035_write_cmos_sensor(0x2c, 0xc8);  
	GC2035_write_cmos_sensor(0x2d, 0x07);//
	GC2035_write_cmos_sensor(0x2e, 0x3a);  
	GC2035_write_cmos_sensor(0x2f, 0x0e);//
	GC2035_write_cmos_sensor(0x30, 0x74);  
	GC2035_write_cmos_sensor(0x3e, 0x40);//
	GC2035_write_cmos_sensor(0xfe, 0x00);  
	GC2035_write_cmos_sensor(0xb6, 0x03);  
	GC2035_write_cmos_sensor(0xfe, 0x00);  
	GC2035_write_cmos_sensor(0x3f, 0x00);  
	GC2035_write_cmos_sensor(0x40, 0x77);  
	GC2035_write_cmos_sensor(0x42, 0x7f);  
	GC2035_write_cmos_sensor(0x43, 0x30);  
	GC2035_write_cmos_sensor(0x5c, 0x08);  
	GC2035_write_cmos_sensor(0x5e, 0x20);  
	GC2035_write_cmos_sensor(0x5f, 0x20);  
	GC2035_write_cmos_sensor(0x60, 0x20);  
	GC2035_write_cmos_sensor(0x61, 0x20);  
	GC2035_write_cmos_sensor(0x62, 0x20);  
	GC2035_write_cmos_sensor(0x63, 0x20);  
	GC2035_write_cmos_sensor(0x64, 0x20);  
	GC2035_write_cmos_sensor(0x65, 0x20);  
	GC2035_write_cmos_sensor(0x66, 0x20);  
	GC2035_write_cmos_sensor(0x67, 0x20);  
	GC2035_write_cmos_sensor(0x68, 0x20);  
	GC2035_write_cmos_sensor(0x69, 0x20);  
	GC2035_write_cmos_sensor(0x90, 0x01);  
	GC2035_write_cmos_sensor(0x95, 0x04);  
	GC2035_write_cmos_sensor(0x96, 0xb0);  
	GC2035_write_cmos_sensor(0x97, 0x06);  
	GC2035_write_cmos_sensor(0x98, 0x40);  
	GC2035_write_cmos_sensor(0xfe, 0x03);  
	GC2035_write_cmos_sensor(0x42, 0x80);  
	GC2035_write_cmos_sensor(0x43, 0x06);  
	GC2035_write_cmos_sensor(0x41, 0x00);  
	GC2035_write_cmos_sensor(0x40, 0x00);  
	GC2035_write_cmos_sensor(0x17, 0x01);  
	GC2035_write_cmos_sensor(0xfe, 0x00);  
	GC2035_write_cmos_sensor(0x80, 0xff);  
	GC2035_write_cmos_sensor(0x81, 0x26);  
	GC2035_write_cmos_sensor(0x03, 0x05);  
	GC2035_write_cmos_sensor(0x04, 0x2e);  
	GC2035_write_cmos_sensor(0x84, 0x01);  
	GC2035_write_cmos_sensor(0x86, 0x02);  
	GC2035_write_cmos_sensor(0x87, 0x90);  
	GC2035_write_cmos_sensor(0x8b, 0xbc);  
	GC2035_write_cmos_sensor(0xa7, 0x80);  
	GC2035_write_cmos_sensor(0xa8, 0x80);  
	GC2035_write_cmos_sensor(0xb0, 0x80);  
	GC2035_write_cmos_sensor(0xc0, 0x40);  
	GC2035_write_cmos_sensor(0xfe , 0x01);
	GC2035_write_cmos_sensor(0xc2 , 0x2a);
	GC2035_write_cmos_sensor(0xc3 , 0x20);
	GC2035_write_cmos_sensor(0xc4 , 0x16);
	GC2035_write_cmos_sensor(0xc8 , 0x21);
	GC2035_write_cmos_sensor(0xc9 , 0x22);
	GC2035_write_cmos_sensor(0xca , 0x18);
	GC2035_write_cmos_sensor(0xbc , 0x46);
	GC2035_write_cmos_sensor(0xbd , 0x2e);
	GC2035_write_cmos_sensor(0xbe , 0x26);
	GC2035_write_cmos_sensor(0xb6 , 0x35);
	GC2035_write_cmos_sensor(0xb7 , 0x2e);
	GC2035_write_cmos_sensor(0xb8, 0x1b);  
	GC2035_write_cmos_sensor(0xc5, 0x00);  
	GC2035_write_cmos_sensor(0xc6, 0x00);  
	GC2035_write_cmos_sensor(0xc7, 0x00);  
	GC2035_write_cmos_sensor(0xcb, 0x00);  
	GC2035_write_cmos_sensor(0xcc, 0x00);  
	GC2035_write_cmos_sensor(0xcd, 0x00);  
	GC2035_write_cmos_sensor(0xbf, 0x0c);  
	GC2035_write_cmos_sensor(0xc0, 0x12);  
	GC2035_write_cmos_sensor(0xc1 , 0x17);
	GC2035_write_cmos_sensor(0xb9 , 0x00);
	GC2035_write_cmos_sensor(0xba , 0x08);
	GC2035_write_cmos_sensor(0xbb , 0x07);
	GC2035_write_cmos_sensor(0xaa, 0x1b);  
	GC2035_write_cmos_sensor(0xab, 0x20);  
	GC2035_write_cmos_sensor(0xac, 0x20);  
	GC2035_write_cmos_sensor(0xad, 0x24);  
	GC2035_write_cmos_sensor(0xae, 0x1f);  
	GC2035_write_cmos_sensor(0xaf, 0x23);  
	GC2035_write_cmos_sensor(0xb0, 0x20);  
	GC2035_write_cmos_sensor(0xb1, 0x20);  
	GC2035_write_cmos_sensor(0xb2, 0x20);  
	GC2035_write_cmos_sensor(0xb3, 0x16);  
	GC2035_write_cmos_sensor(0xb4, 0x1c);  
	GC2035_write_cmos_sensor(0xb5, 0x16);  
	GC2035_write_cmos_sensor(0xd0, 0x00);  
	GC2035_write_cmos_sensor(0xd2, 0x00);  
	GC2035_write_cmos_sensor(0xd3, 0x00);  
	GC2035_write_cmos_sensor(0xd8, 0x00);  
	GC2035_write_cmos_sensor(0xda, 0x00);  
	GC2035_write_cmos_sensor(0xdb, 0x00);  
	GC2035_write_cmos_sensor(0xdc, 0x00);  
	GC2035_write_cmos_sensor(0xde, 0x00);  
	GC2035_write_cmos_sensor(0xdf, 0x00);  
	GC2035_write_cmos_sensor(0xd4, 0x00);  
	GC2035_write_cmos_sensor(0xd6, 0x00);  
	GC2035_write_cmos_sensor(0xd7, 0x0c);  
	GC2035_write_cmos_sensor(0xa4, 0x00);  
	GC2035_write_cmos_sensor(0xa5, 0x00);  
	GC2035_write_cmos_sensor(0xa6, 0x00);  
	GC2035_write_cmos_sensor(0xa7, 0x00);  
	GC2035_write_cmos_sensor(0xa8, 0x00);  
	GC2035_write_cmos_sensor(0xa9, 0x00);  
	GC2035_write_cmos_sensor(0xa1, 0x80);  
	GC2035_write_cmos_sensor(0xa2, 0x80);  
	GC2035_write_cmos_sensor(0xfe, 0x02);  
	GC2035_write_cmos_sensor(0xa4, 0x00);  
	GC2035_write_cmos_sensor(0xfe, 0x00);  
	GC2035_write_cmos_sensor(0xfe, 0x02);  
	GC2035_write_cmos_sensor(0xc0, 0x01);  
	GC2035_write_cmos_sensor(0xc1, 0x40);  
	GC2035_write_cmos_sensor(0xc2, 0xfc);  
	GC2035_write_cmos_sensor(0xc3, 0x05);  
	GC2035_write_cmos_sensor(0xc4, 0xec);  
	GC2035_write_cmos_sensor(0xc5, 0x42);  
	GC2035_write_cmos_sensor(0xc6, 0xf8);  
	GC2035_write_cmos_sensor(0xc7, 0x40);  
	GC2035_write_cmos_sensor(0xc8, 0xf8);  
	GC2035_write_cmos_sensor(0xc9, 0x06);  
	GC2035_write_cmos_sensor(0xca, 0xfd);  
	GC2035_write_cmos_sensor(0xcb, 0x3e);  
	GC2035_write_cmos_sensor(0xcc, 0xf3);  
	GC2035_write_cmos_sensor(0xcd, 0x36);  
	GC2035_write_cmos_sensor(0xce, 0xf6);  
	GC2035_write_cmos_sensor(0xcf, 0x04);  
	GC2035_write_cmos_sensor(0xe3, 0x0c);  
	GC2035_write_cmos_sensor(0xe4, 0x44);  
	GC2035_write_cmos_sensor(0xe5, 0xe5); 
	GC2035_write_cmos_sensor(0xfe, 0x00);  
	GC2035_write_cmos_sensor(0xfe, 0x01);  
	GC2035_write_cmos_sensor(0x4f, 0x00);  
	GC2035_write_cmos_sensor(0x4d, 0x00);//
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4d, 0x10);//10
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4d, 0x20);//20
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4d, 0x30);//30
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);
	GC2035_write_cmos_sensor(0x4e, 0x00);
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4d, 0x40);//40
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4d, 0x50);//50
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4d, 0x60);//60
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00); 
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4d, 0x70);//70
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00); 
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4d, 0x80);//80
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4d, 0x90);//90
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4d, 0xa0);//a0
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4d, 0xb0);//b0
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4d, 0xc0);//c0
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4d, 0xd0);//d0
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4e, 0x00);  
	GC2035_write_cmos_sensor(0x4f, 0x01);  
	GC2035_write_cmos_sensor(0xfe , 0x01);
	GC2035_write_cmos_sensor(0x50 , 0x80);
	GC2035_write_cmos_sensor(0x4f , 0x00);
	GC2035_write_cmos_sensor(0x4d , 0x20);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x80);
	GC2035_write_cmos_sensor(0x4e , 0x80);
	GC2035_write_cmos_sensor(0x4d , 0x30);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x80);
	GC2035_write_cmos_sensor(0x4e , 0x80);
	GC2035_write_cmos_sensor(0x4d , 0x42);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x04);//d50 04
	GC2035_write_cmos_sensor(0x4e , 0x04);//d50
	GC2035_write_cmos_sensor(0x4e , 0x00);//d50
	GC2035_write_cmos_sensor(0x4e , 0x00);//d65
	GC2035_write_cmos_sensor(0x4e , 0x00);//d65
	GC2035_write_cmos_sensor(0x4d , 0x52);
	GC2035_write_cmos_sensor(0x4e , 0x00);//cwf
	GC2035_write_cmos_sensor(0x4e , 0x00);//cwf 10
	GC2035_write_cmos_sensor(0x4e , 0x00);//cwf/D50
	GC2035_write_cmos_sensor(0x4e , 0x02);//d65
	GC2035_write_cmos_sensor(0x4e , 0x02);//d65
	GC2035_write_cmos_sensor(0x4d , 0x60);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x00);
	GC2035_write_cmos_sensor(0x4e , 0x02);
	GC2035_write_cmos_sensor(0x4e , 0x00);//02
	GC2035_write_cmos_sensor(0x4d , 0x92);
	GC2035_write_cmos_sensor(0x4e , 0x40);//a
	GC2035_write_cmos_sensor(0x4f , 0x01);
	/////awb//////
	GC2035_write_cmos_sensor(0xfe , 0x01);
	GC2035_write_cmos_sensor(0x50 , 0x88);//c0//[6]green mode
	GC2035_write_cmos_sensor(0x52 , 0x40);
	GC2035_write_cmos_sensor(0x54 , 0x60);
	GC2035_write_cmos_sensor(0x56 , 0x00);
	GC2035_write_cmos_sensor(0x57, 0x20);  
	GC2035_write_cmos_sensor(0x58, 0x01);  
	GC2035_write_cmos_sensor(0x5b, 0x02);  
	GC2035_write_cmos_sensor(0x61, 0xaa);  
	GC2035_write_cmos_sensor(0x62, 0xaa);  
	GC2035_write_cmos_sensor(0x71, 0x00);  
	GC2035_write_cmos_sensor(0x74, 0x10);  
	GC2035_write_cmos_sensor(0x77 , 0x08);  
	GC2035_write_cmos_sensor(0x78 , 0xfd); 
	GC2035_write_cmos_sensor(0x86 , 0x30);
	GC2035_write_cmos_sensor(0x87 , 0x00);
	GC2035_write_cmos_sensor(0x88 , 0x04);
	GC2035_write_cmos_sensor(0x8a, 0xc0);  
	GC2035_write_cmos_sensor(0x89, 0x75);  
	GC2035_write_cmos_sensor(0x84, 0x08);  
	GC2035_write_cmos_sensor(0x8b, 0x00);  
	GC2035_write_cmos_sensor(0x8d, 0x70);  
	GC2035_write_cmos_sensor(0x8e, 0x70);  
	GC2035_write_cmos_sensor(0x8f, 0xf4);  
	GC2035_write_cmos_sensor(0xfe, 0x00);  
	GC2035_write_cmos_sensor(0x82, 0x02);  
	GC2035_write_cmos_sensor(0xfe, 0x01);  
	GC2035_write_cmos_sensor(0x21, 0xbf);  
	GC2035_write_cmos_sensor(0xfe , 0x02);
	GC2035_write_cmos_sensor(0xa4 , 0x00);//
	GC2035_write_cmos_sensor(0xa5 , 0x40); //lsc_th
	GC2035_write_cmos_sensor(0xa2 , 0xa0); 
	GC2035_write_cmos_sensor(0xa6, 0x80); //50 
	GC2035_write_cmos_sensor(0xa7, 0x80); //30 
	GC2035_write_cmos_sensor(0xab , 0x31); //
	GC2035_write_cmos_sensor(0xa9 , 0x6f); //
		
	GC2035_write_cmos_sensor(0xb0 , 0x99);  
	GC2035_write_cmos_sensor(0xb1 , 0x34);
	GC2035_write_cmos_sensor(0xb3 , 0x80); 
	GC2035_write_cmos_sensor(0xde , 0xb9); //0xb6
	GC2035_write_cmos_sensor(0x38 , 0x0a); //0x0f
	GC2035_write_cmos_sensor(0x39 , 0x60); 
	GC2035_write_cmos_sensor(0xfe , 0x00);
	GC2035_write_cmos_sensor(0x81 , 0x26);

	GC2035_write_cmos_sensor(0xfe , 0x02);
	GC2035_write_cmos_sensor(0x83 , 0x00);
	GC2035_write_cmos_sensor(0x84 , 0x45);
	///====YCP
	GC2035_write_cmos_sensor(0xd1 , 0x3c);  //saturation 0x38
	GC2035_write_cmos_sensor(0xd2 , 0x3c);  //saturation 0x38
	GC2035_write_cmos_sensor(0xdc , 0x30);
	GC2035_write_cmos_sensor(0xdd , 0xb8);  //edge_sa_g,b
	GC2035_write_cmos_sensor(0xfe , 0x00);
	////===dndd
	GC2035_write_cmos_sensor(0xfe , 0x02);
	GC2035_write_cmos_sensor(0x88 , 0x15);
	GC2035_write_cmos_sensor(0x8c , 0xf6); 
	GC2035_write_cmos_sensor(0x89 , 0x03); 
       GC2035_write_cmos_sensor(0x97 , 0x85); //ee th
	//y gamma
	GC2035_write_cmos_sensor(0xfe , 0x02);
	GC2035_write_cmos_sensor(0x2b , 0x00);
	GC2035_write_cmos_sensor(0x2c, 0x04);  
	GC2035_write_cmos_sensor(0x2d, 0x09);  
	GC2035_write_cmos_sensor(0x2e, 0x18);  
	GC2035_write_cmos_sensor(0x2f, 0x27);  
	GC2035_write_cmos_sensor(0x30, 0x37);  
	GC2035_write_cmos_sensor(0x31, 0x49);  
	GC2035_write_cmos_sensor(0x32, 0x5c);  
	GC2035_write_cmos_sensor(0x33, 0x7e);  
	GC2035_write_cmos_sensor(0x34, 0xa0);  
	GC2035_write_cmos_sensor(0x35, 0xc0);  
	GC2035_write_cmos_sensor(0x36, 0xe0);  
	GC2035_write_cmos_sensor(0x37, 0xff);  
	////dark sun
	GC2035_write_cmos_sensor(0xfe , 0x02);
	GC2035_write_cmos_sensor(0x40 , 0x06);
	GC2035_write_cmos_sensor(0x41 , 0x23);
	GC2035_write_cmos_sensor(0x42 , 0x3f);
	GC2035_write_cmos_sensor(0x43 , 0x06);
	GC2035_write_cmos_sensor(0x44 , 0x80);
	GC2035_write_cmos_sensor(0x45 , 0x00);
	GC2035_write_cmos_sensor(0x46 , 0x14);
	GC2035_write_cmos_sensor(0x47 , 0x09);

	/////1600x1200size//
	GC2035_write_cmos_sensor(0xfe , 0x00);
	GC2035_write_cmos_sensor(0x90 , 0x01);  //crop enable
	GC2035_write_cmos_sensor(0x95 , 0x04);  //1600x1200
	GC2035_write_cmos_sensor(0x96 , 0xb0);
	GC2035_write_cmos_sensor(0x97 , 0x06);
	GC2035_write_cmos_sensor(0x98 , 0x40);
	GC2035_write_cmos_sensor(0x99 , 0x11);
	GC2035_write_cmos_sensor(0xfe , 0x03);
	GC2035_write_cmos_sensor(0x42 , 0x80); 
	GC2035_write_cmos_sensor(0x43 , 0x06); 
	GC2035_write_cmos_sensor(0x41 , 0x00); // 
	GC2035_write_cmos_sensor(0x40 , 0x00); 
	GC2035_write_cmos_sensor(0x17 , 0x01); 
	GC2035_write_cmos_sensor(0xfe , 0x00);
	GC2035_write_cmos_sensor(0xfe, 0x00);  
	GC2035_write_cmos_sensor(0x82, 0xfe);
	Sleep(400);
#ifdef GC2035_MIPI
	GC2035_write_cmos_sensor(0xf2, 0x00);
	GC2035_write_cmos_sensor(0xf3, 0x00);
	GC2035_write_cmos_sensor(0xf4, 0x00);
	GC2035_write_cmos_sensor(0xf5, 0x00);
	GC2035_write_cmos_sensor(0xfe, 0x01);
	GC2035_write_cmos_sensor(0x0b, 0x90);
	GC2035_write_cmos_sensor(0x87, 0x10);
	GC2035_write_cmos_sensor(0xfe, 0x00);

	GC2035_write_cmos_sensor(0xfe, 0x03);
	GC2035_write_cmos_sensor(0x01, 0x07);
	GC2035_write_cmos_sensor(0x02, 0x07);//00
	GC2035_write_cmos_sensor(0x03, 0x10);
	GC2035_write_cmos_sensor(0x06, 0x80);
	GC2035_write_cmos_sensor(0x11, 0x1E);
	GC2035_write_cmos_sensor(0x12, 0x80);
	GC2035_write_cmos_sensor(0x13, 0x0c);
	GC2035_write_cmos_sensor(0x15, 0x12);
	GC2035_write_cmos_sensor(0x04, 0x20);
	GC2035_write_cmos_sensor(0x05, 0x00);
	GC2035_write_cmos_sensor(0x17, 0x00);

	GC2035_write_cmos_sensor(0x21, 0x02);
	GC2035_write_cmos_sensor(0x29, 0x02);
	GC2035_write_cmos_sensor(0x2a, 0x03);
	GC2035_write_cmos_sensor(0x2b, 0x08);

	GC2035_write_cmos_sensor(0x10, 0x95);
	GC2035_write_cmos_sensor(0xfe, 0x00);
#else
	GC2035_write_cmos_sensor(0xf2, 0x70);
	GC2035_write_cmos_sensor(0xf3, 0xff);
	GC2035_write_cmos_sensor(0xf4, 0x00);
	GC2035_write_cmos_sensor(0xf5, 0x30);
	GC2035_write_cmos_sensor(0xfe, 0x01);
	GC2035_write_cmos_sensor(0x0b, 0x90);
	GC2035_write_cmos_sensor(0x87, 0x10);
	GC2035_write_cmos_sensor(0xfe, 0x00);

#endif
}
  
static void GC2035_Sensor_SVGA(void)
{
	SENSORDB("GC2035_Sensor_SVGA");
#ifdef GC2035_MIPI
	GC2035_write_cmos_sensor(0xf7,0x05);
	GC2035_write_cmos_sensor(0xf8,0x85);
	GC2035_write_cmos_sensor(0xc8,0x14);
#else
	GC2035_write_cmos_sensor(0xf7,0x15);
	GC2035_write_cmos_sensor(0xf8,0x85);
	GC2035_write_cmos_sensor(0xc8,0x54);
#endif
	GC2035_SET_PAGE3;
	GC2035_write_cmos_sensor(0x40,0x40);
	GC2035_write_cmos_sensor(0x41,0x02);
	GC2035_write_cmos_sensor(0x42,0x40);
	GC2035_write_cmos_sensor(0x43,0x06);
	GC2035_write_cmos_sensor(0x17,0x00);
	GC2035_SET_PAGE0;
	
	GC2035_write_cmos_sensor(0x90,0x01);
	GC2035_write_cmos_sensor(0x95,0x02);
	GC2035_write_cmos_sensor(0x96,0x58);
	GC2035_write_cmos_sensor(0x97,0x03);
	GC2035_write_cmos_sensor(0x98,0x20);
#ifdef GC2035_MIPI
	GC2035_write_cmos_sensor(0xfe, 0x03);
	GC2035_write_cmos_sensor(0x12, 0x40);
	GC2035_write_cmos_sensor(0x13, 0x06);
	GC2035_write_cmos_sensor(0x04, 0x90);
	GC2035_write_cmos_sensor(0x05, 0x01);
	GC2035_write_cmos_sensor(0xfe, 0x00);
#endif
}

static void GC2035_Sensor_2M(void)
{
	SENSORDB("GC2035_Sensor_2M");
	GC2035_write_cmos_sensor(0xf7,0x05);
	GC2035_write_cmos_sensor(0xf8,0x85);
	GC2035_write_cmos_sensor(0xc8,0x00);
	
	GC2035_write_cmos_sensor(0x90,0x01);
	GC2035_write_cmos_sensor(0x95,0x04);
	GC2035_write_cmos_sensor(0x96,0xb0);
	GC2035_write_cmos_sensor(0x97,0x06);
	GC2035_write_cmos_sensor(0x98,0x40);
#ifdef GC2035_MIPI
	GC2035_write_cmos_sensor(0xfe, 0x03);
	GC2035_write_cmos_sensor(0x12, 0x80);
	GC2035_write_cmos_sensor(0x13, 0x0c);
	GC2035_write_cmos_sensor(0x04, 0x20);
	GC2035_write_cmos_sensor(0x05, 0x00);
	GC2035_write_cmos_sensor(0xfe, 0x00);
#endif
}

/*****************************************************************************/
/* Windows Mobile Sensor Interface */
/*****************************************************************************/
/*************************************************************************
* FUNCTION
*	GC2035Open
*
* DESCRIPTION
*	This function initialize the registers of CMOS sensor
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 GC2035Open(void)
{
	volatile signed char i;
	kal_uint16 sensor_id=0;
	SENSORDB("GC2035Open \r\n");

	zoom_factor = 0; 
	Sleep(10);


	//  Read sensor ID to adjust I2C is OK?
	for(i=0;i<3;i++)
	{
		sensor_id = (GC2035_read_cmos_sensor(0xf0) << 8) | GC2035_read_cmos_sensor(0xf1);
		if(sensor_id != GC2035_SENSOR_ID)
		{
			return ERROR_SENSOR_CONNECT_FAIL;
		}
	}
	
	SENSORDB("GC2035 Sensor Read ID OK \r\n");
	GC2035_Sensor_Init();
	
	return ERROR_NONE;
}	/* GC2035Open() */

/*************************************************************************
* FUNCTION
*	GC2035Close
*
* DESCRIPTION
*	This function is to turn off sensor module power.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 GC2035Close(void)
{
//	CISModulePowerOn(FALSE);
	return ERROR_NONE;
}	/* GC2035Close() */

/*************************************************************************
* FUNCTION
*	GC2035Preview
*
* DESCRIPTION
*	This function start the sensor preview.
*
* PARAMETERS
*	*image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 GC2035Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	kal_uint8 iTemp, temp_AE_reg, temp_AWB_reg;
	kal_uint16 iDummyPixels = 0, iDummyLines = 0, iStartX = 0, iStartY = 0;

	SENSORDB("GC2035Previe\n");

	GC2035_sensor_cap_state = KAL_FALSE;
	//===preview setting===
	// set shutter
        //GC2035_write_shutter(Preview_Shutter);
	//GC2035_write_cmos_sensor(0xfa, 0x00);

	GC2035_Sensor_SVGA();
	
	// turn on AEC/AGC
	GC2035_set_AE_mode(KAL_TRUE); 

	//enable Auto WB
	//GC2035_set_AWB_mode(KAL_TRUE); 


	// copy sensor_config_data
	memcpy(&GC2035SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	return ERROR_NONE;
}	/* GC2035Preview() */

UINT32 GC2035Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    volatile kal_uint32 shutter = GC2035_exposure_lines, temp_reg;
    kal_uint8 temp_AE_reg, temp;
    kal_uint16 AE_setting_delay = 0;

    SENSORDB("GC2035Capture\n");
    GC2035_sensor_cap_state = KAL_TRUE;
 
    // turn off AEC/AGC
    GC2035_set_AE_mode(KAL_FALSE);

    //GC2035_set_AWB_mode(KAL_FALSE); 

    shutter = GC2035_read_shutter();
    Preview_Shutter = shutter;

    printk("GC2035 Read Shutter = %d\n", shutter); 
    if ((image_window->ImageTargetWidth<=GC2035_IMAGE_SENSOR_PV_WIDTH)&&
        (image_window->ImageTargetHeight<=GC2035_IMAGE_SENSOR_PV_HEIGHT))
    {    /* Less than PV Mode */
	 SENSORDB("GC2035Capture SVGA\n");
        GC2035_capture_pclk_in_M = GC2035_preview_pclk_in_M;   //Don't change the clk

        shutter = (shutter*GC2035_capture_pclk_in_M)/GC2035_preview_pclk_in_M;
        if (shutter < 1) 
        {
            shutter = 1;
        }
        // set shutter
        //GC2035_write_shutter(shutter);
        
        image_window->GrabStartX = 1;
        image_window->GrabStartY = 1;
        image_window->ExposureWindowWidth= GC2035_IMAGE_SENSOR_PV_WIDTH - 3;
        image_window->ExposureWindowHeight = GC2035_IMAGE_SENSOR_PV_HEIGHT - 3;

    }
    else 
    {    
    	 /* 2M FULL Mode */
	 SENSORDB("GC2035Capture 2M\n");
        image_window->GrabStartX=1;
        image_window->GrabStartY=1;
        image_window->ExposureWindowWidth=GC2035_IMAGE_SENSOR_FULL_WIDTH - image_window->GrabStartX - 2;
        image_window->ExposureWindowHeight=GC2035_IMAGE_SENSOR_FULL_HEIGHT -image_window->GrabStartY - 2;    	 
        //calculator auto uv	
        if (shutter < 1) 
        {
            shutter = 1;
        }
        Capture_Shutter = shutter / 2; 
        
        // set shutter
        //GC2035_write_shutter(Capture_Shutter);
	//GC2035_write_cmos_sensor(0xfa, 0x11);
	GC2035_Sensor_2M();
	//Sleep(600);
    }

    printk("GC2035 Capture Shutter = %d\n", Capture_Shutter); 
    // AEC/AGC/AWB will be enable in preview and param_wb function
    /* total delay 4 frame for AE stable */
	// copy sensor_config_data
    memcpy(&GC2035SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	return ERROR_NONE;
}	/* GC2035Capture() */

UINT32 GC2035GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	SENSORDB("GC2035GetResolution \r\n");

	pSensorResolution->SensorFullWidth=GC2035_IMAGE_SENSOR_FULL_WIDTH - 2 * IMAGE_SENSOR_START_GRAB_X;
	pSensorResolution->SensorFullHeight=GC2035_IMAGE_SENSOR_FULL_HEIGHT - 2 * IMAGE_SENSOR_START_GRAB_Y;
	pSensorResolution->SensorPreviewWidth=GC2035_IMAGE_SENSOR_PV_WIDTH - 2 * IMAGE_SENSOR_START_GRAB_X;
	pSensorResolution->SensorPreviewHeight=GC2035_IMAGE_SENSOR_PV_HEIGHT - 2 * IMAGE_SENSOR_START_GRAB_Y;

	return ERROR_NONE;
}	/* GC2035GetResolution() */

UINT32 GC2035GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
					  MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	SENSORDB("GC2035GetInfo \r\n");

	pSensorInfo->SensorPreviewResolutionX=GC2035_IMAGE_SENSOR_PV_WIDTH;
	pSensorInfo->SensorPreviewResolutionY=GC2035_IMAGE_SENSOR_PV_HEIGHT;
	pSensorInfo->SensorFullResolutionX=GC2035_IMAGE_SENSOR_FULL_WIDTH;
	pSensorInfo->SensorFullResolutionY=GC2035_IMAGE_SENSOR_FULL_HEIGHT;

	pSensorInfo->SensorCameraPreviewFrameRate=30;
	pSensorInfo->SensorVideoFrameRate=30;
	pSensorInfo->SensorStillCaptureFrameRate=10;
	pSensorInfo->SensorWebCamCaptureFrameRate=15;
	pSensorInfo->SensorResetActiveHigh=FALSE;
	pSensorInfo->SensorResetDelayCount=1;
	pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;	/*??? */
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorInterruptDelayLines = 1;
	pSensorInfo->CaptureDelayFrame = 4; 
	pSensorInfo->PreviewDelayFrame = 2; 
	pSensorInfo->VideoDelayFrame = 0; 
	pSensorInfo->SensorMasterClockSwitch = 0; 
	pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_6MA;

#ifdef GC2035_MIPI
	pSensorInfo->SensroInterfaceType = SENSOR_INTERFACE_TYPE_MIPI;
#else
	pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_PARALLEL;
#endif
	
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].BinningEnable=FALSE;	

	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			pSensorInfo->SensorClockFreq=26;
			pSensorInfo->SensorClockDividCount=3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;
			pSensorInfo->SensorDataLatchCount= 2;
                     pSensorInfo->SensorGrabStartX = 2; 
                     pSensorInfo->SensorGrabStartY = 2;
#ifdef GC2035_MIPI
			pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;		
			pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
			pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
			pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
			pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
			pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
			pSensorInfo->SensorPacketECCOrder = 1;
#endif
	
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
			pSensorInfo->SensorClockFreq=26;
			pSensorInfo->SensorClockDividCount=3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;
			pSensorInfo->SensorDataLatchCount= 2;
                     pSensorInfo->SensorGrabStartX = 2; 
                     pSensorInfo->SensorGrabStartY = 2;
#ifdef GC2035_MIPI
			pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_2_LANE;		
			pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
			pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;
			pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
			pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
			pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
			pSensorInfo->SensorPacketECCOrder = 1;
#endif
			
		break;
		default:
			pSensorInfo->SensorClockFreq=26;
			pSensorInfo->SensorClockDividCount=3;
			pSensorInfo->SensorClockRisingCount=0;
			pSensorInfo->SensorClockFallingCount=2;
			pSensorInfo->SensorPixelClockCount=3;
			pSensorInfo->SensorDataLatchCount=2;
                     pSensorInfo->SensorGrabStartX = 2; 
                     pSensorInfo->SensorGrabStartY = 2;             
			
		break;
	}
	memcpy(pSensorConfigData, &GC2035SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	return ERROR_NONE;
}	/* GC2035GetInfo() */


UINT32 GC2035Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			GC2035Preview(pImageWindow, pSensorConfigData);
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
			GC2035Capture(pImageWindow, pSensorConfigData);
		break;
		default:
		    break; 
	}
	return TRUE;
}	/* GC2035Control() */

BOOL GC2035_set_param_wb(UINT16 para)
{
	SENSORDB("GC2035_set_param_wb \r\n");

	switch (para)
	{
		case AWB_MODE_AUTO:
			GC2035_write_cmos_sensor(0xb3, 0x61);
			GC2035_write_cmos_sensor(0xb4, 0x40);
			GC2035_write_cmos_sensor(0xb5, 0x61);
			GC2035_set_AWB_mode(KAL_TRUE);
		break;
		case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
			GC2035_set_AWB_mode(KAL_FALSE);
			GC2035_write_cmos_sensor(0xb3, 0x58);
			GC2035_write_cmos_sensor(0xb4, 0x40);
			GC2035_write_cmos_sensor(0xb5, 0x50);
		break;
		case AWB_MODE_DAYLIGHT: //sunny
			GC2035_set_AWB_mode(KAL_FALSE);
			GC2035_write_cmos_sensor(0xb3, 0x58);
			GC2035_write_cmos_sensor(0xb4, 0x40);
			GC2035_write_cmos_sensor(0xb5, 0x50);
		break;
		case AWB_MODE_INCANDESCENT: //office
			GC2035_set_AWB_mode(KAL_FALSE);
			GC2035_write_cmos_sensor(0xb3, 0x50);
			GC2035_write_cmos_sensor(0xb4, 0x40);
			GC2035_write_cmos_sensor(0xb5, 0xa8);
		break;
		case AWB_MODE_TUNGSTEN: //home
			GC2035_set_AWB_mode(KAL_FALSE);
			GC2035_write_cmos_sensor(0xb3, 0xa0);
			GC2035_write_cmos_sensor(0xb4, 0x45);
			GC2035_write_cmos_sensor(0xb5, 0x40);
		break;
		case AWB_MODE_FLUORESCENT:
			GC2035_set_AWB_mode(KAL_FALSE);
			GC2035_write_cmos_sensor(0xb3, 0x72);
			GC2035_write_cmos_sensor(0xb4, 0x40);
			GC2035_write_cmos_sensor(0xb5, 0x5b);
		break;	
		default:
		return FALSE;
	}
	return TRUE;
} /* GC2035_set_param_wb */

BOOL GC2035_set_param_effect(UINT16 para)
{
	kal_uint32 ret = KAL_TRUE;
	SENSORDB("GC2035_set_param_effect \r\n");
	
	switch (para)
	{
		case MEFFECT_OFF:
			GC2035_write_cmos_sensor(0xfe, 0x00);
			GC2035_write_cmos_sensor(0x83, 0xe0);
		break;

		case MEFFECT_SEPIA:
			GC2035_write_cmos_sensor(0xfe, 0x00);
			GC2035_write_cmos_sensor(0x83, 0x82);
		break;  

		case MEFFECT_NEGATIVE:		
			GC2035_write_cmos_sensor(0xfe, 0x00);
			GC2035_write_cmos_sensor(0x83, 0x01);
		break; 

		case MEFFECT_SEPIAGREEN:		
			GC2035_write_cmos_sensor(0xfe, 0x00);
			GC2035_write_cmos_sensor(0x83, 0x52);
		break;

		case MEFFECT_SEPIABLUE:	
			GC2035_write_cmos_sensor(0xfe, 0x00);
			GC2035_write_cmos_sensor(0x83, 0x62);
		break;

		case MEFFECT_MONO:				
			GC2035_write_cmos_sensor(0xfe, 0x00);
			GC2035_write_cmos_sensor(0x83, 0x12);
		break;

		default:
		return FALSE;
	}

	return ret;
} /* GC2035_set_param_effect */

BOOL GC2035_set_param_banding(UINT16 para)
{
	SENSORDB("GC2035_set_param_banding \r\n");

    switch (para)
    {
        case AE_FLICKER_MODE_50HZ:
		GC2035_write_cmos_sensor(0x05,0x00);//hb
		GC2035_write_cmos_sensor(0x06,0xca);
		GC2035_write_cmos_sensor(0x07,0x00);//vb
		GC2035_write_cmos_sensor(0x08,0x5f);
		GC2035_SET_PAGE1;
		GC2035_write_cmos_sensor(0x27,0x00);//step
		GC2035_write_cmos_sensor(0x28,0xb9);
		GC2035_write_cmos_sensor(0x29,0x05);//level1
		GC2035_write_cmos_sensor(0x2a,0x0f);
		GC2035_write_cmos_sensor(0x2b,0x05);//level2
		GC2035_write_cmos_sensor(0x2c,0xc8);
		GC2035_write_cmos_sensor(0x2d,0x07);//6e8//level3  073a->>05c8
		GC2035_write_cmos_sensor(0x2e,0x3a);
		GC2035_write_cmos_sensor(0x2f,0x0e);//level4
		GC2035_write_cmos_sensor(0x30,0x74);
		GC2035_SET_PAGE0;
            break;

        case AE_FLICKER_MODE_60HZ:
		GC2035_write_cmos_sensor(0x05,0x00);//hb
		GC2035_write_cmos_sensor(0x06,0xd2);
		GC2035_write_cmos_sensor(0x07,0x00);//vb
		GC2035_write_cmos_sensor(0x08,0x18);
		GC2035_SET_PAGE1;
		GC2035_write_cmos_sensor(0x27,0x00);//step
		GC2035_write_cmos_sensor(0x28,0x99);
		GC2035_write_cmos_sensor(0x29,0x04);//level1
		GC2035_write_cmos_sensor(0x2a,0xc8);
		GC2035_write_cmos_sensor(0x2b,0x05);//level2
		GC2035_write_cmos_sensor(0x2c,0xfa);
		GC2035_write_cmos_sensor(0x2d,0x07);//level3  072c->05fa
		GC2035_write_cmos_sensor(0x2e,0x2c);
		GC2035_write_cmos_sensor(0x2f,0x0e);//level4
		GC2035_write_cmos_sensor(0x30,0x58);
		GC2035_SET_PAGE0;
            break;

          default:
              return FALSE;
    }

    return TRUE;
} /* GC2035_set_param_banding */

BOOL GC2035_set_param_exposure(UINT16 para)
{
	SENSORDB("GC2035_set_param_exposure \r\n");

	switch (para)
	{
		case AE_EV_COMP_n13:
			GC2035_SET_PAGE1;
			GC2035_write_cmos_sensor(0x13,0x40);
			GC2035_SET_PAGE0;
		break;
		case AE_EV_COMP_n10:
			GC2035_SET_PAGE1;
			GC2035_write_cmos_sensor(0x13,0x50);
			GC2035_SET_PAGE0;
		break;
		case AE_EV_COMP_n07:
			GC2035_SET_PAGE1;
			GC2035_write_cmos_sensor(0x13,0x60);
			GC2035_SET_PAGE0;
		break;
		case AE_EV_COMP_n03:
			GC2035_SET_PAGE1;
			GC2035_write_cmos_sensor(0x13,0x70);
			GC2035_SET_PAGE0;
		break;
		case AE_EV_COMP_00:
			GC2035_SET_PAGE1;
			GC2035_write_cmos_sensor(0x13,0x88);//0x80->0x88
			GC2035_SET_PAGE0;
		break;
		case AE_EV_COMP_03:
			GC2035_SET_PAGE1;
			GC2035_write_cmos_sensor(0x13,0x90);
			GC2035_SET_PAGE0;
		break;
		case AE_EV_COMP_07:
			GC2035_SET_PAGE1;
			GC2035_write_cmos_sensor(0x13,0xa0);
			GC2035_SET_PAGE0;
		break;
		case AE_EV_COMP_10:
			GC2035_SET_PAGE1;
			GC2035_write_cmos_sensor(0x13,0xb0);
			GC2035_SET_PAGE0;
		break;
		case AE_EV_COMP_13:
			GC2035_SET_PAGE1;
			GC2035_write_cmos_sensor(0x13,0xc0);
			GC2035_SET_PAGE0;
		break;
		default:
		return FALSE;
	}
	return TRUE;
} /* GC2035_set_param_exposure */

UINT32 GC2035YUVSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
//   if( GC2035_sensor_cap_state == KAL_TRUE)
//	   return TRUE;
	SENSORDB("GC2035YUVSensorSetting \r\n");

	switch (iCmd) {
	case FID_SCENE_MODE:	    
//	    printk("Set Scene Mode:%d\n", iPara); 
	    if (iPara == SCENE_MODE_OFF)
	    {
	        GC2035_night_mode(0); 
	    }
	    else if (iPara == SCENE_MODE_NIGHTSCENE)
	    {
               GC2035_night_mode(1); 
	    }	    
	    break; 	    
	case FID_AWB_MODE:
//	    printk("Set AWB Mode:%d\n", iPara); 	    
           GC2035_set_param_wb(iPara);
	break;
	case FID_COLOR_EFFECT:
//	    printk("Set Color Effect:%d\n", iPara); 	    	    
           GC2035_set_param_effect(iPara);
	break;
	case FID_AE_EV:
//           printk("Set EV:%d\n", iPara); 	    	    
           GC2035_set_param_exposure(iPara);
	break;
	case FID_AE_FLICKER:
//           printk("Set Flicker:%d\n", iPara); 	    	    	    
           GC2035_set_param_banding(iPara);
	break;
        case FID_AE_SCENE_MODE: 
            if (iPara == AE_MODE_OFF) {
                GC2035_set_AE_mode(KAL_FALSE);
            }
            else {
                GC2035_set_AE_mode(KAL_TRUE);
	    }
            break; 
	case FID_ZOOM_FACTOR:
	    zoom_factor = iPara; 
        break; 
	default:
	break;
	}
	return TRUE;
}   /* GC2035YUVSensorSetting */

UINT32 GC2035YUVSetVideoMode(UINT16 u2FrameRate)
{
    kal_uint8 iTemp;
    /* to fix VSYNC, to fix frame rate */
    //printk("Set YUV Video Mode \n");  
	SENSORDB("GC2035YUVSetVideoMode \r\n");

    if (u2FrameRate == 30)
    {
    }
    else if (u2FrameRate == 15)       
    {
    }
    else 
    {
        printk("Wrong frame rate setting \n");
    }
    GC2035_VEDIO_encode_mode = KAL_TRUE; 
        
    return TRUE;
}

UINT32 GC2035FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
							 UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
	UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
	UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
	UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
	UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
	MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
	SENSORDB("GC2035FeatureControl \r\n");

	switch (FeatureId)
	{
		case SENSOR_FEATURE_GET_RESOLUTION:
			*pFeatureReturnPara16++=GC2035_IMAGE_SENSOR_FULL_WIDTH;
			*pFeatureReturnPara16=GC2035_IMAGE_SENSOR_FULL_HEIGHT;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_GET_PERIOD:
			*pFeatureReturnPara16++=GC2035_PV_PERIOD_PIXEL_NUMS+GC2035_PV_dummy_pixels;
			*pFeatureReturnPara16=GC2035_PV_PERIOD_LINE_NUMS+GC2035_PV_dummy_lines;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			*pFeatureReturnPara32 = GC2035_sensor_pclk/10;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_SET_ESHUTTER:
		break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			GC2035_night_mode((BOOL) *pFeatureData16);
		break;
		case SENSOR_FEATURE_SET_GAIN:
		case SENSOR_FEATURE_SET_FLASHLIGHT:
		break;
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			GC2035_isp_master_clock=*pFeatureData32;
		break;
		case SENSOR_FEATURE_SET_REGISTER:
			GC2035_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
		break;
		case SENSOR_FEATURE_GET_REGISTER:
			pSensorRegData->RegData = GC2035_read_cmos_sensor(pSensorRegData->RegAddr);
		break;
		case SENSOR_FEATURE_GET_CONFIG_PARA:
			memcpy(pSensorConfigData, &GC2035SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
			*pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
		break;
		case SENSOR_FEATURE_SET_CCT_REGISTER:
		case SENSOR_FEATURE_GET_CCT_REGISTER:
		case SENSOR_FEATURE_SET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_REGISTER_DEFAULT:

		case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
		case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
		case SENSOR_FEATURE_GET_GROUP_INFO:
		case SENSOR_FEATURE_GET_ITEM_INFO:
		case SENSOR_FEATURE_SET_ITEM_INFO:
		case SENSOR_FEATURE_GET_ENG_INFO:
		break;
		case SENSOR_FEATURE_GET_GROUP_COUNT:
                        *pFeatureReturnPara32++=0;
                        *pFeatureParaLen=4;	    
		    break; 
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			// get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
			// if EEPROM does not exist in camera module.
			*pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_CHECK_SENSOR_ID:
			 GC2035_GetSensorID(pFeatureData32);
			 break;
		case SENSOR_FEATURE_SET_YUV_CMD:
		       //printk("GC2035 YUV sensor Setting:%d, %d \n", *pFeatureData32,  *(pFeatureData32+1));
			GC2035YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
		break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:
		       GC2035YUVSetVideoMode(*pFeatureData16);
		       break; 
		default:
			break;			
	}
	return ERROR_NONE;
}	/* GC2035FeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncGC2035=
{
	GC2035Open,
	GC2035GetInfo,
	GC2035GetResolution,
	GC2035FeatureControl,
	GC2035Control,
	GC2035Close
};

UINT32 GC2035_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncGC2035;

	return ERROR_NONE;
}	/* SensorInit() */
