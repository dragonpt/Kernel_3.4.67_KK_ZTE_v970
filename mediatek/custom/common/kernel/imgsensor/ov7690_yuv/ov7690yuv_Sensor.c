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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
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
 *   Sensor.c
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Image sensor driver function
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/io.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"

#include "ov7690yuv_Sensor.h"
#include "ov7690yuv_Camera_Sensor_para.h"
#include "ov7690yuv_CameraCustomized.h"

#define Sleep(ms) mdelay(ms)

MSDK_SENSOR_CONFIG_STRUCT OV7690SensorConfigData;

#define OV7690YUV_DEBUG
#ifdef OV7690YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif







#if 0
extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);
static int sensor_id_fail = 0; 
#define OV7690_write_cmos_sensor(addr,para) iWriteReg((u16) addr , (u32) para ,1,OV7690_WRITE_ID)
//#define OV7690_write_cmos_sensor_2(addr, para, bytes) iWriteReg((u16) addr , (u32) para ,bytes,OV7690_WRITE_ID)
kal_uint16 OV7690_read_cmos_sensor(kal_uint32 addr)
{
kal_uint16 get_byte=0;
    iReadReg((u16) addr ,(u8*)&get_byte,OV7690_WRITE_ID);
    return get_byte;
}
#endif

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
static void OV7690_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
kal_uint8 out_buff[2];

    out_buff[0] = addr;
    out_buff[1] = para;

    if (0 != iWriteRegI2C((u8*)out_buff , (u16)sizeof(out_buff), OV7690_WRITE_ID)) {
        SENSORDB("ERROR: OV7690_write_cmos_sensor \n");
    }
}

static kal_uint8 OV7690_read_cmos_sensor(kal_uint8 addr)
{
  kal_uint8 in_buff[1] = {0xFF};
  kal_uint8 out_buff[1];
  
  out_buff[0] = addr;

    if (0 != iReadRegI2C((u8*)out_buff , (u16) sizeof(out_buff), (u8*)in_buff, (u16) sizeof(in_buff), OV7690_WRITE_ID)) {
        SENSORDB("ERROR: OV7690_read_cmos_sensor \n");
    }

  return in_buff[0];
}


static struct
{
  kal_uint16 Effect;
  kal_uint16 Exposure;
  kal_uint16 Wb;
  kal_uint8 Mirror;
  kal_bool BypassAe;
  kal_bool BypassAwb;
  kal_bool VideoMode;
  kal_uint8 Ctrl3A;
  kal_uint8 SdeCtrl;
  kal_uint8 Reg15;
  kal_uint16 Fps;
  kal_uint32 IntClk; /* internal clock = half of pclk */
  kal_uint16 FrameHeight;
  kal_uint16 LineLength;
  //sensor_data_struct *NvramData;
} OV7690Sensor;
/*************************************************************************
* FUNCTION
*    OV7690HalfAdjust
*
* DESCRIPTION
*    This function dividend / divisor and use round-up.
*
* PARAMETERS
*    DividEnd
*    Divisor
*
* RETURNS
*    [DividEnd / Divisor]
*
* LOCAL AFFECTED
*
*************************************************************************/
__inline static kal_uint32 OV7690HalfAdjust(kal_uint32 DividEnd, kal_uint32 Divisor)
{
  return (DividEnd + (Divisor >> 1)) / Divisor; /* that is [DividEnd / Divisor + 0.5]*/
}



/*************************************************************************
* FUNCTION
*    OV7690SetMirror
*
* DESCRIPTION
*    This function set the Mirror to the CMOS sensor
*
* PARAMETERS
*    Mirror
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV7690SetMirror(kal_uint8 Mirror)
{
  kal_uint8 Reg = 0x16;
  printk("\r\n zhaoshaopeng Mirror =%d \r\n", Mirror);
  if (OV7690Sensor.Mirror == Mirror)
  {
    return;
  }
  OV7690Sensor.Mirror = Mirror;
  switch (OV7690Sensor.Mirror)
  {
  case IMAGE_H_MIRROR:
    Reg |= 0x40;
    break;
  case IMAGE_V_MIRROR:
    Reg |= 0x80;
    break;
  case IMAGE_HV_MIRROR:
    Reg |= 0xC0;
    break;
    //zhaoshaopeng add for k504 i8
  case IMAGE_NORMAL:
    Reg |= 0x00;//0x80;
    break;
    //zhaoshaopeng end
  }
  OV7690_write_cmos_sensor(0x0C, Reg);
}

/*************************************************************************
* FUNCTION
*    OV7690SetClock
*
* DESCRIPTION
*    This function set sensor vt clock and op clock
*
* PARAMETERS
*    Clk: vt clock
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV7690SetClock(kal_uint32 Clk)
{
  static const kal_uint8 ClkSetting[][1] =
  {
    {0x00}, /* MCLK: 26M, INTERNAL CLK: 13M, PCLK: 26M */
    {0x01}, /* MCLK: 26M, INTERNAL CLK: 6.5M, PCLK: 13M */
  };
  kal_uint8 i = 0;
  
  if (OV7690Sensor.IntClk == Clk)
  {
    return;
  }
  OV7690Sensor.IntClk = Clk;
  switch (OV7690Sensor.IntClk)
  {
  case 13000000: i = 0; break;
  case 6500000:  i = 1; break;
  default: ASSERT(0);
  }
/*
  PLL Control:
  PLL clock(f1) = MCLK x N / M, where
    reg29[7:6]: PLL divider
      00: /1, 01: /2, 10: /3, 11: /4
    reg29[5:4]: PLL output control
      00: Bypass PLL, 01: 4x, 10: 6x, 11: 8x
  Int. clock(f2) = f1 / (reg0x11[5:0] + 1)
  PCLK = f2 / 2 / L, where L = 1 if 0x3E[4] = 1, otherwise L = 2
*/
  OV7690_write_cmos_sensor(0x11, ClkSetting[i][0]);
}

#if 0 /* not referenced currently */
/*************************************************************************
* FUNCTION
*    OV7690WriteShutter
*
* DESCRIPTION
*    This function apply Shutter to sensor
*
* PARAMETERS
*    Shutter: integration time
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV7690WriteShutter(kal_uint32 Shutter)
{
  OV7690_write_cmos_sensor(0x10, Shutter);
  OV7690_write_cmos_sensor(0x0F, Shutter >> 8);
}

/*************************************************************************
* FUNCTION
*    OV7690ReadShutter
*
* DESCRIPTION
*    This function get shutter from sensor
*
* PARAMETERS
*    None
*
* RETURNS
*    Shutter: integration time
*
* LOCAL AFFECTED
*
*************************************************************************/
static kal_uint16 OV7690ReadShutter(void)
{
  return (OV7690_read_cmos_sensor(0x0F) << 8)|OV7690_read_cmos_sensor(0x10);
}
#endif
/*************************************************************************
* FUNCTION
*    OV7690LSCSetting
*
* DESCRIPTION
*    This function set Lens Shading Correction.
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV7690LSCSetting(void)
{
  static const kal_uint8 Data[] =
  {
#if 0 /* Sunny F28 */
    0x90,0x18,0xb0,0xA0,0x24,0x1f,0x21,
#elif 0 /* Sunny F24 */
    0x90,0x18,0x10,0x00,0x32,0x2c,0x30,
#elif 0 /* Phoenix F28 */
    0x90,0x18,0xb0,0xA0,0x32,0x2c,0x30,
#elif 1 /* Phoenix F24 */
    0x90,0x00,0xa0,0x80,0x18,0x14,0x15,
#elif 0 /* Dongya F24 */
    0x90,0x18,0x10,0x00,0x32,0x2c,0x30,
#endif
  };
  
  OV7690_write_cmos_sensor(0x85, Data[0]);
  OV7690_write_cmos_sensor(0x86, Data[1]);
  OV7690_write_cmos_sensor(0x87, Data[2]);
  OV7690_write_cmos_sensor(0x88, Data[3]);
  OV7690_write_cmos_sensor(0x89, Data[4]);
  OV7690_write_cmos_sensor(0x8a, Data[5]);
  OV7690_write_cmos_sensor(0x8b, Data[6]);
}

/*************************************************************************
* FUNCTION
*    OV7690AeEnable
*
* DESCRIPTION
*    disable/enable AE
*
* PARAMETERS
*    Enable
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV7690AeEnable(kal_bool Enable)
{
  const kal_bool AeEnable = (OV7690Sensor.Ctrl3A&0x05) ? KAL_TRUE : KAL_FALSE;
  
  if (OV7690Sensor.BypassAe)
  {
    Enable = KAL_FALSE;
  }
  if (AeEnable == Enable)
  {
    return;
  }
  if (Enable)
  {
    OV7690Sensor.Ctrl3A |= 0x05;
    OV7690Sensor.Reg15 |= 0x80;
  }
  else
  {
    OV7690Sensor.Ctrl3A &= 0xFA;//f8
    /* extra line can not be set if not fix frame rate!!! */
    OV7690Sensor.Reg15 &= 0x7F;
  }
  OV7690_write_cmos_sensor(0x13, OV7690Sensor.Ctrl3A);
  OV7690_write_cmos_sensor(0x15, OV7690Sensor.Reg15);
}



/*************************************************************************
* FUNCTION
*    OV7690AwbEnable
*
* DESCRIPTION
*    disable/enable awb
*
* PARAMETERS
*    Enable
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV7690AwbEnable(kal_bool Enable)
{
  const kal_bool AwbEnable = (OV7690Sensor.Ctrl3A&0x02) ? KAL_TRUE : KAL_FALSE;
  
  if (OV7690Sensor.BypassAwb)
  {
    Enable = KAL_FALSE;
  }
  if (AwbEnable == Enable)
  {
    return;
  }
  if (Enable && AWB_MODE_AUTO == OV7690Sensor.Wb)
  {
    OV7690Sensor.Ctrl3A |= 0x02;
  }
  else
  {
    OV7690Sensor.Ctrl3A &= 0xFD;
  }
  OV7690_write_cmos_sensor(0x13, OV7690Sensor.Ctrl3A);
}

/*************************************************************************
* FUNCTION
*    OV7690SetDummy
*
* DESCRIPTION
*    This function add DummyPixel and DummyLine.
*
* PARAMETERS
*    DummyPixel
*    DummyLine
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV7690SetDummy(kal_uint16 DummyPixel, kal_uint16 DummyLine)
{
  kal_bool update = KAL_FALSE; /* need config banding */
  const kal_uint16 LineLength = DummyPixel + OV7690_PERIOD_PIXEL_NUMS;
  const kal_uint16 FrameHeight = DummyLine + OV7690_PERIOD_LINE_NUMS;
  
  /* config line length */
  if (LineLength != OV7690Sensor.LineLength)
  {
    if (LineLength > OV7690_MAX_LINELENGTH)
    {
      ASSERT(0);
    }
    update = KAL_TRUE;
    OV7690Sensor.LineLength = LineLength;
    OV7690_write_cmos_sensor(0x2B, LineLength); /* Actually, the 0x2a,0x2b is not DummyPixel, it's the total line length */
  }
  /* config frame height */
  if (FrameHeight != OV7690Sensor.FrameHeight)
  {
    if (DummyLine > OV7690_MAX_FRAMEHEIGHT)
    {
      ASSERT(0);
    }
    update = KAL_TRUE;
    OV7690Sensor.FrameHeight = FrameHeight;
    OV7690_write_cmos_sensor(0x2C, DummyLine);
  }
  /* config banding */
  if (update)
  {
    kal_uint8 BandStep50, BandStep60, MaxBand50, MaxBand60;
    
    OV7690_write_cmos_sensor(0x2A, ((LineLength >> 4)&0x70)|((DummyLine >> 8)&0x0F)|(OV7690_read_cmos_sensor(0x2A)&0x80));
    BandStep50 = OV7690HalfAdjust(OV7690Sensor.IntClk, LineLength * OV7690_NUM_50HZ * 2);
    BandStep60 = OV7690HalfAdjust(OV7690Sensor.IntClk, LineLength * OV7690_NUM_60HZ * 2);
    
    OV7690_write_cmos_sensor(0x50, BandStep50); /* 50Hz banding step */
    OV7690_write_cmos_sensor(0x51, BandStep60); /* 60Hz banding step */
    
    /* max banding in a frame */
    MaxBand50 = FrameHeight / BandStep50;
    MaxBand60 = FrameHeight / BandStep60;
    OV7690_write_cmos_sensor(0x20, ((MaxBand50&0x10) << 3)|((MaxBand60&0x10) << 2));
    OV7690_write_cmos_sensor(0x21, ((MaxBand50&0x0F) << 4)|(MaxBand60&0x0F));
  }
}

/*************************************************************************
* FUNCTION
*    OV7690CalFps
*
* DESCRIPTION
*    This function calculate & set frame rate and fix frame rate when video mode
*    MUST BE INVOKED AFTER OV7690_preview() !!!
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV7690CalFps(void)
{
  /* camera preview also fix frame rate for auto frame rate will cause AE peak */
  if (OV7690Sensor.VideoMode) /* fix frame rate when video mode */
  {
    kal_uint32 LineLength, FrameHeight;
    
    /* get max line length */
    LineLength = OV7690Sensor.IntClk * OV7690_FPS(1) / (OV7690Sensor.Fps * OV7690Sensor.FrameHeight);
    if (LineLength > OV7690_PERIOD_PIXEL_NUMS * 2) /* overflow check */
    {
      OV7690SetClock(OV7690_VIDEO_LOW_CLK); /* slow down clock */
      LineLength = OV7690Sensor.IntClk * OV7690_FPS(1) / (OV7690Sensor.Fps * OV7690Sensor.FrameHeight);
      if (LineLength > OV7690_MAX_LINELENGTH) /* overflow check */
      {
        LineLength = OV7690_MAX_LINELENGTH;
      }
    }
    if (LineLength < OV7690Sensor.LineLength)
    {
      LineLength = OV7690Sensor.LineLength;
    }
    /* get frame height */
    FrameHeight = OV7690Sensor.IntClk * OV7690_FPS(1) / (OV7690Sensor.Fps * LineLength);
    if (FrameHeight < OV7690Sensor.FrameHeight)
    {
      FrameHeight = OV7690Sensor.FrameHeight;
    }
    OV7690SetDummy(LineLength - OV7690_PERIOD_PIXEL_NUMS, FrameHeight - OV7690_PERIOD_LINE_NUMS);
    
    /* fix frame rate */
    OV7690Sensor.Reg15 &= 0x7F;
    OV7690_write_cmos_sensor(0x15, OV7690Sensor.Reg15);
  }
  else
  {
    kal_uint16 CurFps;
    
    CurFps = OV7690HalfAdjust(OV7690Sensor.IntClk * OV7690_FPS(1),
                              OV7690Sensor.Fps * OV7690Sensor.LineLength * OV7690Sensor.FrameHeight);
    /* to force frame rate change when change night mode to normal mode */
    OV7690Sensor.Reg15 &= 0x0F;
    switch (CurFps)
    {
    case 0:
    case 1: break;
    case 2: OV7690Sensor.Reg15 |= 0x10; break;
    case 3: OV7690Sensor.Reg15 |= 0x20; break;
    case 4: OV7690Sensor.Reg15 |= 0x30; break;
    default: OV7690Sensor.Reg15 |= 0x40; break;
    }
    OV7690_write_cmos_sensor(0x15, OV7690Sensor.Reg15);
    OV7690Sensor.Reg15 |= 0x80;
    OV7690_write_cmos_sensor(0x15, OV7690Sensor.Reg15);
  }
}

/*************************************************************************
* FUNCTION
*    OV7690InitialSetting
*
* DESCRIPTION
*    This function initialize the registers of CMOS sensor
*
* PARAMETERS
*    None
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV7690InitialSetting(void)
{
  OV7690_write_cmos_sensor(0x12, 0x80); /* [7]: software reset */
  Sleep(5);
  OV7690_write_cmos_sensor(0x0C, 0x16);//16,56,96,d6
/*
  Please update reg0x48 from 40 to 42. Also, please apply the rule for OV7690.
  If DOVDD = 1.7 - 2.0V, then reg0x49 = 0x0C
  IF DOVDD = 2.1 - 2.5V, then reg0x49 = 0x04
  If DOVDD = 2.6 - 3.0V, then reg0x49 = 0x0D
  
  That is, for example, if your DOVDD = 2.8v, please insert setting as below.
  reg0x48 = 0x42
  reg0x49 = 0x0D
*/
  OV7690_write_cmos_sensor(0x48, 0x42); /* 1.75x pre-gain */
  OV7690_write_cmos_sensor(0x49, 0x0D); /* releated IO voltage */
  OV7690_write_cmos_sensor(0x41, 0x43);
  OV7690_write_cmos_sensor(0x81, 0xFF); /* do not disable SDE!!! */
  OV7690_write_cmos_sensor(0x16, 0x03);
  OV7690_write_cmos_sensor(0x39, 0x80);
  
  /* Clock */ 
  OV7690_write_cmos_sensor(0x11, 0x00);
  OV7690_write_cmos_sensor(0x3E, 0x30);
  
  /* Isp common */
  OV7690_write_cmos_sensor(0x13, 0xF7);
  OV7690_write_cmos_sensor(0x14, 0x21); /* 8x gain ceiling, PPChrg off */
  OV7690_write_cmos_sensor(0x15, 0x04);
  OV7690_write_cmos_sensor(0x1E, 0x33);
  OV7690_write_cmos_sensor(0x0E, 0x03); /* driving capability: 0x00:1x, 0x01:2x, 0x02:3x, 0x03:4x */
  OV7690_write_cmos_sensor(0xD2, 0x04); /* SDE ctrl */
  
  SENSORDB("OV7690Open_start33333 \n");
  /* Format */
  OV7690_write_cmos_sensor(0x12, 0x00);
  OV7690_write_cmos_sensor(0x82, 0x03);
  OV7690_write_cmos_sensor(0xd0, 0x48);
  OV7690_write_cmos_sensor(0x80, 0x7f);
  OV7690_write_cmos_sensor(0x22, 0x00); /* Disable Optical Black Output Selection bit[7] */
  
  /* Resolution */
  OV7690_write_cmos_sensor(0x2A, 0x30); /* linelength */
  OV7690_write_cmos_sensor(0x2B, 0x4E);
  OV7690_write_cmos_sensor(0x2C, 0x00); /* dummy line */
  OV7690_write_cmos_sensor(0x17, OV7690_IMAGE_SENSOR_HSTART); /* H window start line */
  OV7690_write_cmos_sensor(0x18, (OV7690_IMAGE_SENSOR_HACTIVE + 0x10) >> 2); /* H sensor size */
  OV7690_write_cmos_sensor(0x19, OV7690_IMAGE_SENSOR_VSTART); /* V window start line */
  OV7690_write_cmos_sensor(0x1A, (OV7690_IMAGE_SENSOR_VACTIVE + OV7690_IMAGE_SENSOR_VSTART) >> 1); /* V sensor size */
  OV7690_write_cmos_sensor(0xC8, OV7690_IMAGE_SENSOR_HACTIVE >> 8);
  OV7690_write_cmos_sensor(0xC9, OV7690_IMAGE_SENSOR_HACTIVE);/* ISP input hsize */
  OV7690_write_cmos_sensor(0xCA, OV7690_IMAGE_SENSOR_VACTIVE >> 8);
  OV7690_write_cmos_sensor(0xCB, OV7690_IMAGE_SENSOR_VACTIVE);/* ISP input vsize */
  OV7690_write_cmos_sensor(0xCC, OV7690_IMAGE_SENSOR_HACTIVE >> 8);
  OV7690_write_cmos_sensor(0xCD, OV7690_IMAGE_SENSOR_HACTIVE);/* ISP output hsize */
  OV7690_write_cmos_sensor(0xCE, OV7690_IMAGE_SENSOR_VACTIVE >> 8);
  OV7690_write_cmos_sensor(0xCF, OV7690_IMAGE_SENSOR_VACTIVE);/* ISP output vsize */
  
      /*OV7690_LSC_setting();*/
            OV7690_write_cmos_sensor(0x80, 0x7f);
            OV7690_write_cmos_sensor(0x85, 0x90);
            OV7690_write_cmos_sensor(0x86, 0x00);
            OV7690_write_cmos_sensor(0x87, 0x5);
            OV7690_write_cmos_sensor(0x88, 0x2);
            OV7690_write_cmos_sensor(0x89, 0x41);
            OV7690_write_cmos_sensor(0x8a, 0x36);
            OV7690_write_cmos_sensor(0x8b, 0x33);
  
  /* Color Matrix */
            OV7690_write_cmos_sensor(0xBB, 0x7A);
            OV7690_write_cmos_sensor(0xBC, 0x69);
            OV7690_write_cmos_sensor(0xBD, 0x11);
            OV7690_write_cmos_sensor(0xBE, 0x13);
            OV7690_write_cmos_sensor(0xBF, 0x81);
            OV7690_write_cmos_sensor(0xC0, 0x96);
            OV7690_write_cmos_sensor(0xC1, 0x1E);
  
  /* Edge + Denoise */
            OV7690_write_cmos_sensor(0xb4, 0x20);
            OV7690_write_cmos_sensor(0xB6, 0x08);
            OV7690_write_cmos_sensor(0xb5, 0x05);
  OV7690_write_cmos_sensor(0xb8, 0x06);
            OV7690_write_cmos_sensor(0xb9, 0x02);
            OV7690_write_cmos_sensor(0xba, 0x08);
  
  /* AEC/AGC target */
  /*This only in fixed frame change!*/
            OV7690_write_cmos_sensor(0x24, 0x88);
            OV7690_write_cmos_sensor(0x25, 0x78);
  OV7690_write_cmos_sensor(0x26, 0xb4);
  
  /* UV adjust */
      OV7690_write_cmos_sensor(0x81, 0xff);
      OV7690_write_cmos_sensor(0x5A, 0x14);
      OV7690_write_cmos_sensor(0x5B, 0xa2);
      OV7690_write_cmos_sensor(0x5C, 0x70);
      OV7690_write_cmos_sensor(0x5d, 0x20);
  
      OV7690_write_cmos_sensor(0x81, 0xff);//21
      OV7690_write_cmos_sensor(0xd5, 0x20);
      OV7690_write_cmos_sensor(0xd4, 0x20);
      OV7690_write_cmos_sensor(0xd3, 0x00);
      OV7690_write_cmos_sensor(0xdc, 0x00);
      OV7690_write_cmos_sensor(0xD2, 0x04);
      OV7690_write_cmos_sensor(0xD8, 0x55);
      OV7690_write_cmos_sensor(0xD9, 0x55);
  /* Gamma */
  OV7690_write_cmos_sensor(0xa3, 0x05);
  OV7690_write_cmos_sensor(0xa4, 0x10);
  OV7690_write_cmos_sensor(0xa5, 0x25);
  OV7690_write_cmos_sensor(0xa6, 0x4f);
  OV7690_write_cmos_sensor(0xa7, 0x5f);
  OV7690_write_cmos_sensor(0xa8, 0x6c);
  OV7690_write_cmos_sensor(0xa9, 0x78);
  OV7690_write_cmos_sensor(0xaa, 0x82);
  OV7690_write_cmos_sensor(0xab, 0x8b);
  OV7690_write_cmos_sensor(0xac, 0x92);
  OV7690_write_cmos_sensor(0xad, 0x9f);
  OV7690_write_cmos_sensor(0xae, 0xAc);
  OV7690_write_cmos_sensor(0xaf, 0xC1);
  OV7690_write_cmos_sensor(0xb0, 0xD5);
  OV7690_write_cmos_sensor(0xb1, 0xE7);
  OV7690_write_cmos_sensor(0xb2, 0x21);
  
  /* Advance AWB */
      OV7690_write_cmos_sensor(0x8c, 0x52);
  OV7690_write_cmos_sensor(0x8d, 0x11);
  OV7690_write_cmos_sensor(0x8e, 0x12);
  OV7690_write_cmos_sensor(0x8f, 0x19);
  OV7690_write_cmos_sensor(0x90, 0x50);
  OV7690_write_cmos_sensor(0x91, 0x20);
      OV7690_write_cmos_sensor(0x92, 0xb1);
      OV7690_write_cmos_sensor(0x93, 0x9a);
      OV7690_write_cmos_sensor(0x94, 0x0c);
      OV7690_write_cmos_sensor(0x95, 0x0c);
  OV7690_write_cmos_sensor(0x96, 0xf0);
  OV7690_write_cmos_sensor(0x97, 0x10);
      OV7690_write_cmos_sensor(0x98, 0x61);
      OV7690_write_cmos_sensor(0x99, 0x63);
      OV7690_write_cmos_sensor(0x9a, 0x71);
      OV7690_write_cmos_sensor(0x9b, 0x78);
  OV7690_write_cmos_sensor(0x9c, 0xf0);
  OV7690_write_cmos_sensor(0x9d, 0xf0);
  OV7690_write_cmos_sensor(0x9e, 0xf0);
  OV7690_write_cmos_sensor(0x9f, 0xff);
      OV7690_write_cmos_sensor(0xa0, 0xa8);
      OV7690_write_cmos_sensor(0xa1, 0xa8);
      OV7690_write_cmos_sensor(0xa2, 0x0f);

  /* test pattern */
#if defined(__OV7690_TEST_PATTERN__)
  OV7690_write_cmos_sensor(0x82, 0x0F);
#endif
}

/*************************************************************************
* FUNCTION
*	OV7690Open
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
kal_uint32 OV7690Open(void)

{
	kal_uint16 sensor_id=0; 

    SENSORDB("OV7690Open_start \n");


	sensor_id=((OV7690_read_cmos_sensor(0x0A)<< 8)|OV7690_read_cmos_sensor(0x0B));
    SENSORDB("OV7690Open_startzhijie  sensor_id=%x \n",sensor_id);

	if (sensor_id != OV7690_SENSOR_ID) {
                sensor_id = 0xFFFFFFFF; 
                SENSORDB("sensor_id error \n");
        	  return ERROR_SENSOR_CONNECT_FAIL;
	}
	OV7690InitialSetting();
	
    SENSORDB("OV7690Open_start1111 \n");
	memset(&OV7690Sensor, 0xFF, sizeof(OV7690Sensor));
	OV7690Sensor.BypassAe = KAL_FALSE;
	OV7690Sensor.BypassAwb = KAL_FALSE;
	OV7690Sensor.Ctrl3A = OV7690_read_cmos_sensor(0x13);
	OV7690Sensor.SdeCtrl = OV7690_read_cmos_sensor(0xD2);
	OV7690Sensor.Reg15 = OV7690_read_cmos_sensor(0x15);



    SENSORDB("OV7690Open_end \n");
    
	return ERROR_NONE;
}   /* OV7690Open  */

/*************************************************************************
* FUNCTION
*	OV7690Close
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
kal_uint32 OV7690Close(void)
{


	return ERROR_NONE;
}   /* OV7690Close */
/*************************************************************************
* FUNCTION
* OV7690_Preview
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
static kal_uint32 OV7690_Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
	kal_uint16 DummyPixel = 0;
    SENSORDB("[IN] OV7690_Preview SensorImageMirror:%d\n",sensor_config_data->SensorImageMirror);

	OV7690_FPS(30);
	OV7690SetMirror(sensor_config_data->SensorImageMirror);
	OV7690SetClock(OV7690_PREVIEW_CLK);
	OV7690SetDummy(DummyPixel, 0);
	OV7690CalFps(); /* need cal new fps */
	OV7690_write_cmos_sensor(0x14,0x21 );//for banding 50HZ
	OV7690AeEnable(KAL_TRUE);
	OV7690AwbEnable(KAL_TRUE);
        OV7690_write_cmos_sensor(0x13,0xf7 );


	image_window->GrabStartX= OV7690_X_START;
	image_window->GrabStartY= OV7690_Y_START;
	image_window->ExposureWindowWidth = OV7690_IMAGE_SENSOR_WIDTH;
	image_window->ExposureWindowHeight =OV7690_IMAGE_SENSOR_HEIGHT;
    SENSORDB("[OUT] OV7690_Preview \n");

	return ERROR_NONE;

}   /*  OV7690_Preview   */

/*************************************************************************
* FUNCTION
*	OV7690_Capture
*
* DESCRIPTION
*	This function setup the CMOS sensor in capture MY_OUTPUT mode
*
* PARAMETERS
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
static kal_uint32 OV7690_Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
						  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    SENSORDB("[IN] OV7690_Capture SensorImageMirror:%d\n",sensor_config_data->SensorImageMirror);
	//OV7690SetMirror(IMAGE_H_MIRROR);
	OV7690SetMirror(sensor_config_data->SensorImageMirror);
	image_window->GrabStartX= OV7690_X_START;
	image_window->GrabStartY= OV7690_Y_START;
	image_window->ExposureWindowWidth = OV7690_IMAGE_SENSOR_WIDTH;
	image_window->ExposureWindowHeight =OV7690_IMAGE_SENSOR_HEIGHT; 
    SENSORDB("[OUT] OV7690_Capture \n");

	return ERROR_NONE;
}   /* OV7576_Capture() */

void OV7690_NightMode(kal_bool bEnable)
{
	kal_uint16 dummy = 0;
	
    SENSORDB("[IN]OV7690_NightMode \n");
	if(bEnable == KAL_FALSE)
	    {
            OV7690Sensor.Fps = OV7690_FPS(30);
           
            }
	else if(bEnable == KAL_TRUE)
            { 
	    OV7690Sensor.Fps = OV7690_FPS(15);           
            }
	else
		printk("Wrong mode setting \n");

	OV7690SetDummy(dummy, 0);
  	OV7690CalFps(); /* need cal new fps */
	 OV7690_write_cmos_sensor(0x15,0xa4);//94
}

kal_uint32 OV7690GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{

	pSensorResolution->SensorFullWidth=OV7690_IMAGE_SENSOR_WIDTH_DRV;
	pSensorResolution->SensorFullHeight=OV7690_IMAGE_SENSOR_HEIGHT_DRV;
    pSensorResolution->SensorPreviewWidth=OV7690_IMAGE_SENSOR_WIDTH_DRV-2;
	pSensorResolution->SensorPreviewHeight=OV7690_IMAGE_SENSOR_HEIGHT_DRV-2;
	return ERROR_NONE;
}	/* OV7690GetResolution() */

kal_uint32 OV7690GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
					  MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{

	pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_UYVY;
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;

	 pSensorInfo->CaptureDelayFrame = 4; 
  pSensorInfo->PreviewDelayFrame = 2;  
	pSensorInfo->SensorMasterClockSwitch = 0; 
      pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_2MA;   		


	//CAMERA_CONTROL_FLOW(ScenarioId,ScenarioId);

	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:		
			pSensorInfo->SensorClockFreq=26;
			pSensorInfo->SensorClockDividCount=	3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;
			pSensorInfo->SensorDataLatchCount= 2;
			
			pSensorInfo->SensorGrabStartX = OV7690_X_START; 
			pSensorInfo->SensorGrabStartY = OV7690_Y_START;			   
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:		
			pSensorInfo->SensorClockFreq=26;
			pSensorInfo->SensorClockDividCount= 3;
			pSensorInfo->SensorClockRisingCount=0;
			pSensorInfo->SensorClockFallingCount=2;
			pSensorInfo->SensorPixelClockCount=3;
			pSensorInfo->SensorDataLatchCount=2;	
			
			pSensorInfo->SensorGrabStartX = OV7690_X_START; 
			pSensorInfo->SensorGrabStartY = OV7690_Y_START;			   
		break;
		default:
			pSensorInfo->SensorClockFreq=26;
			pSensorInfo->SensorClockDividCount= 3;
			pSensorInfo->SensorClockRisingCount=0;
			pSensorInfo->SensorClockFallingCount=2;
			pSensorInfo->SensorPixelClockCount=3;
			pSensorInfo->SensorDataLatchCount=2;
			
			pSensorInfo->SensorGrabStartX = OV7690_X_START; 
			pSensorInfo->SensorGrabStartY = OV7690_Y_START;			   
		break;
	}

	memcpy(pSensorConfigData, &OV7690SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	
	return ERROR_NONE;
}	/* OV7690GetInfo() */


kal_uint32 OV7690Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    SENSORDB("[IN] OV7690Control ScenarioId=%d \n",ScenarioId);

	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			OV7690_Preview(pImageWindow, pSensorConfigData);
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
			OV7690_Capture(pImageWindow, pSensorConfigData);
		break;
		default:
			return ERROR_INVALID_SCENARIO_ID;
	}
	return TRUE;
}	/* MT9P012Control() */



static BOOL OV7690_set_param_wb(UINT16 para)
{
	kal_uint8  i;

	const static kal_uint8 AwbGain[5][2]=
    { /* R gain, B gain. base: 0x40 */
     {0x56,0x5c}, /* cloud */
     {0x56,0x58}, /* daylight */
     {0x40,0x67}, /* INCANDESCENCE */
     {0x60,0x58}, /* FLUORESCENT */
     {0x40,0xA0}, /* TUNGSTEN */
    };
	if (OV7690Sensor.Wb == para)
    {
      return TRUE;
    }
    OV7690Sensor.Wb = para;

	switch (para)
	{
		case AWB_MODE_AUTO:
			OV7690AwbEnable(KAL_TRUE);	
			OV7690_write_cmos_sensor(0x8c,0x52);//52
			break;
		case AWB_MODE_INCANDESCENT: //office
			i = 1;
		OV7690AwbEnable(KAL_FALSE);
			OV7690_write_cmos_sensor(0x01,0x56);
			OV7690_write_cmos_sensor(0x02,0x58);
			OV7690_write_cmos_sensor(0x03,0x40);
			OV7690_write_cmos_sensor(0x8c,0x56);
			break;
		case AWB_MODE_TUNGSTEN: //home 
			i = 0;
		OV7690AwbEnable(KAL_FALSE);
			OV7690_write_cmos_sensor(0x01,0x56);
			OV7690_write_cmos_sensor(0x02,0x5c);
			OV7690_write_cmos_sensor(0x03,0x40);			
			OV7690_write_cmos_sensor(0x8c,0x56);
			break;
		case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
			i = 2;			
		OV7690AwbEnable(KAL_FALSE);
			OV7690_write_cmos_sensor(0x01,0x88);
			OV7690_write_cmos_sensor(0x02,0xa3);
			OV7690_write_cmos_sensor(0x03,0x80);			
			OV7690_write_cmos_sensor(0x8c,0x56);
			break;
		case AWB_MODE_FLUORESCENT: 
			i = 4;
		OV7690AwbEnable(KAL_FALSE);
			OV7690_write_cmos_sensor(0x01,0x57);
			OV7690_write_cmos_sensor(0x02,0x56);
			OV7690_write_cmos_sensor(0x03,0x40);			
			OV7690_write_cmos_sensor(0x8c,0x56);
			break;
		case AWB_MODE_DAYLIGHT: //sunny
			i = 3;
			OV7690AwbEnable(KAL_FALSE);
			OV7690_write_cmos_sensor(0x01,0x50);
			OV7690_write_cmos_sensor(0x02,0x52);
			OV7690_write_cmos_sensor(0x03,0x40);			
			OV7690_write_cmos_sensor(0x8c,0x56);
			break; 
		default:
			return FALSE;
	}
	 //OV7690AwbEnable(KAL_FALSE);
   //OV7690_write_cmos_sensor(0x02, AwbGain[i][0]); /* AWb R gain */
   //OV7690_write_cmos_sensor(0x01, AwbGain[i][1]); /* AWb B gain */

	return TRUE;
} /* OV7690_set_param_wb */


static BOOL OV7690_set_param_effect(UINT16 para)
{
	
	static const kal_uint8 Data[6][3]=
    {
      {0x06,0x80,0x80}, /* NORMAL */
      {0x1e,0x80,0x80}, //{0x26,0x80,0x80}, /* GRAYSCALE */
      {0x1e,0x40,0xA0}, /* SEPIA */
      {0x1e,0x60,0x60}, /* SEPIAGREEN */
      {0x1e,0xA0,0x40}, /* SEPIABLUE */
      {0x46,0x80,0x80}, /* COLORINV */
    };
    kal_uint8 i;
	BOOL ret = TRUE;
    
    if (OV7690Sensor.Effect == para)
    {
      return TRUE;
    }
    OV7690Sensor.Effect = para;
	switch (para)
	{
		case MEFFECT_OFF:
			i = 0;		
			break;
		case MEFFECT_SEPIA:
			i = 2;	
			break;
		case MEFFECT_NEGATIVE:
			i = 5;
			break;
		case MEFFECT_SEPIAGREEN:
			i = 3;
			break;
		case MEFFECT_SEPIABLUE:
			i = 4;
			break;
		case MEFFECT_MONO://CAM_EFFECT_ENC_GRAYSCALE: 
			i = 1;
			break;
		default:
			ret = FALSE;
	}
	OV7690Sensor.SdeCtrl &= 0xA7; /* disable bit3/4/6 */
	OV7690Sensor.SdeCtrl |= Data[i][0];
	OV7690_write_cmos_sensor(0xD2, OV7690Sensor.SdeCtrl);
	OV7690_write_cmos_sensor(0xDA, Data[i][1]);
	OV7690_write_cmos_sensor(0xDB, Data[i][2]);

	return ret;

} /* OV7690_set_param_effect */

static BOOL OV7690_set_param_banding(UINT16 para)
{

	
	
	switch (para)
	{
		case AE_FLICKER_MODE_50HZ:
			OV7690_write_cmos_sensor(0x14, 0x21);
			break;
		case AE_FLICKER_MODE_60HZ:
			OV7690_write_cmos_sensor(0x14, 0x20);
			break;
		default:
			return FALSE;
	}

	return TRUE;
} /* OV7690_set_param_banding */

static BOOL OV7690_set_param_exposure(UINT16 para)
{
	static const kal_uint8 Data[9][2]=
    {
      {0x09,0x40}, /* EV -4 */
      {0x09,0x30}, /* EV -3 */
      {0x09,0x20}, /* EV -2 */
      {0x09,0x10}, /* EV -1 */
      {0x01,0x00}, /* EV 0 */
      {0x01,0x10}, /* EV +1 */
      {0x01,0x20}, /* EV +2 */
      {0x01,0x30}, /* EV +3 */
      {0x01,0x40}, /* EV +4 */
    };
    kal_uint8 i;
	
	if (OV7690Sensor.Exposure == para)
	{
	  return TRUE;
	}
	OV7690Sensor.Exposure = para;

	switch (para)
	{
		case AE_EV_COMP_00:	// Disable EV compenate
			i = 4;	
			break;
		case AE_EV_COMP_05:// EV compensate 0.5
			i = 5;
			break;
		case AE_EV_COMP_10:// EV compensate 1.0
			i = 6;
			break;
		case AE_EV_COMP_15:// EV compensate 1.5
			i = 7;
			break;
		case AE_EV_COMP_20:// EV compensate 2.0
			i = 8;
			break;
		case AE_EV_COMP_n05:// EV compensate -0.5
			i = 3;
			break;
		case AE_EV_COMP_n10:// EV compensate -1.0
			i = 2;
			break;
		case AE_EV_COMP_n15:// EV compensate -1.5
			i = 1;
			break;
		case AE_EV_COMP_n20:// EV compensate -2.0
			i = 0;
			break;
		default:
			return FALSE;
		}
	OV7690_write_cmos_sensor(0xDC, Data[i][0]); /* SGNSET */
   OV7690_write_cmos_sensor(0xD3, Data[i][1]); /* YBright */

	return TRUE;
} /* OV7690_set_param_exposure */

static kal_uint32 OV7690_YUVSensorSetting(FEATURE_ID iCmd, UINT16 iPara)
{
	
    SENSORDB("[IN] OV7690_YUVSensorSetting iCmd =%d, iPara =%d\n",iCmd, iPara);
	
	switch (iCmd) 
	{
		case FID_SCENE_MODE:
            if (iPara == SCENE_MODE_OFF)
			{

			//SENSORDB("OV7690_NightMode = FALSE \n");
			    OV7690_NightMode(FALSE); 
			}
			else if (iPara == SCENE_MODE_NIGHTSCENE)
			{
				//SENSORDB("OV7690_NightMode = TRUE \n");
			    OV7690_NightMode(TRUE); 
			}	    
		    break; 
		case FID_AWB_MODE:
			OV7690_set_param_wb(iPara);
		break;
		case FID_COLOR_EFFECT:
			OV7690_set_param_effect(iPara);
		break;
		case FID_AE_EV:	
			OV7690_set_param_exposure(iPara);
		break;
		case FID_AE_FLICKER:
			OV7690_set_param_banding(iPara);
		break;
		
		case FID_ZOOM_FACTOR:
		default:
		break;
	}
	
	return TRUE;
}   /* OV7690_YUVSensorSetting */

static kal_uint32 OV7690_YUVSetVideoMode(UINT16 u2FrameRate)
{
    kal_uint16 dummy;
    if (u2FrameRate == 30)
    {
	    OV7690_FPS(30);
    }
    else if (u2FrameRate == 15)       
    {
		OV7690_FPS(15);
    }
    else 
    {
        printk("Wrong frame rate setting \n");
    }   

	OV7690SetDummy(dummy, 0);
  	OV7690CalFps(); /* need cal new fps */
    
	printk("\n OV7690_YUVSetVideoMode:u2FrameRate=%d\n\n",u2FrameRate);
    return TRUE;
}


/*************************************************************************
* FUNCTION
*    OV7690_get_size
*
* DESCRIPTION
*    This function return the image width and height of image sensor.
*
* PARAMETERS
*    *sensor_width: address pointer of horizontal effect pixels of image sensor
*    *sensor_height: address pointer of vertical effect pixels of image sensor
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV7690_get_size(kal_uint16 *sensor_width, kal_uint16 *sensor_height)
{
  *sensor_width = OV7690_IMAGE_SENSOR_WIDTH_DRV; /* must be 4:3 */
  *sensor_height = OV7690_IMAGE_SENSOR_HEIGHT_DRV;
}

/*************************************************************************
* FUNCTION
*    OV7690_get_period
*
* DESCRIPTION
*    This function return the image width and height of image sensor.
*
* PARAMETERS
*    *pixel_number: address pointer of pixel numbers in one period of HSYNC
*    *line_number: address pointer of line numbers in one period of VSYNC
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
static void OV7690_get_period(kal_uint16 *pixel_number, kal_uint16 *line_number)
{
  *pixel_number = OV7690_PERIOD_PIXEL_NUMS;
  *line_number = OV7690_PERIOD_LINE_NUMS;
}

/*************************************************************************
* FUNCTION
*    OV7690_feature_control
*
* DESCRIPTION
*    This function control sensor mode
*
* PARAMETERS
*    id: scenario id
*    image_window: image grab window
*    cfg_data: config data
*
* RETURNS
*    error code
*
* LOCAL AFFECTED
*
*************************************************************************/
kal_uint32 OV7690FeatureControl(MSDK_SENSOR_FEATURE_ENUM id, kal_uint8 *para, kal_uint32 *len)
{
	UINT32 *pFeatureData32=(UINT32 *) para;
	if((id!=3000)&&(id!=3004)&&(id!=3006)){
	    //CAMERA_CONTROL_FLOW(id,id);
	}

	switch (id)
	{
		case SENSOR_FEATURE_GET_RESOLUTION: /* no use */
			OV7690_get_size((kal_uint16 *)para, (kal_uint16 *)(para + sizeof(kal_uint16)));
			*len = sizeof(kal_uint32);
			break;
		case SENSOR_FEATURE_GET_PERIOD:
			OV7690_get_period((kal_uint16 *)para, (kal_uint16 *)(para + sizeof(kal_uint16)));
			*len = sizeof(kal_uint32);
			break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			*(kal_uint32 *)para = OV7690Sensor.IntClk ;
			*len = sizeof(kal_uint32);
			break;
		case SENSOR_FEATURE_SET_ESHUTTER:
			break;
		case SENSOR_FEATURE_SET_NIGHTMODE: 
			OV7690_NightMode((kal_bool)*(kal_uint16 *)para);
			break;
		case SENSOR_FEATURE_SET_GAIN:
		case SENSOR_FEATURE_SET_FLASHLIGHT:
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			break;
		case SENSOR_FEATURE_SET_REGISTER:
			OV7690_write_cmos_sensor(((MSDK_SENSOR_REG_INFO_STRUCT *)para)->RegAddr, ((MSDK_SENSOR_REG_INFO_STRUCT *)para)->RegData);
			break;
		case SENSOR_FEATURE_GET_REGISTER: /* 10 */
			((MSDK_SENSOR_REG_INFO_STRUCT *)para)->RegData = OV7690_read_cmos_sensor(((MSDK_SENSOR_REG_INFO_STRUCT *)para)->RegAddr);
			break;
		case SENSOR_FEATURE_SET_CCT_REGISTER:
		case SENSOR_FEATURE_GET_CCT_REGISTER:
		case SENSOR_FEATURE_SET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
		case SENSOR_FEATURE_GET_CONFIG_PARA: /* no use */
			break;
		case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
			break;
		case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
			break;
		case SENSOR_FEATURE_GET_GROUP_COUNT:
		case SENSOR_FEATURE_GET_GROUP_INFO: /* 20 */
		case SENSOR_FEATURE_GET_ITEM_INFO:
		case SENSOR_FEATURE_SET_ITEM_INFO:
		case SENSOR_FEATURE_GET_ENG_INFO:
			break;
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
		/*
		* get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
		* if EEPROM does not exist in camera module.
		*/
			*(kal_uint32 *)para = LENS_DRIVER_ID_DO_NOT_CARE;
			*len = sizeof(kal_uint32);
			break;
		case SENSOR_FEATURE_SET_YUV_CMD:
	//		OV7690_YUVSensorSetting((FEATURE_ID)(UINT32 *)para, (UINT32 *)(para+1));
			
			OV7690_YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
			break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:
			
			SENSORDB("[IN]SENSOR_FEATURE_SET_VIDEO_MODE \n");
			OV7690_YUVSetVideoMode(*para);
			break; 		
		default:
			break;
	}
	return ERROR_NONE;
}


SENSOR_FUNCTION_STRUCT	SensorFuncOV7690=
{
	OV7690Open,
	OV7690GetInfo,
	OV7690GetResolution,
	OV7690FeatureControl,
	OV7690Control,
	OV7690Close
};

UINT32 OV7690_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncOV7690;

	return ERROR_NONE;
}	/* SensorInit() */
