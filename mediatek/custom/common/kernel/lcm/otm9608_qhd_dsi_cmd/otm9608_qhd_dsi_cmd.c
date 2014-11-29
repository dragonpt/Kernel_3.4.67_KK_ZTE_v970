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
#ifdef BUILD_LK
#define LCM_PRINT printf
#else
#include <linux/string.h>

#if defined(BUILD_UBOOT)
	#include <asm/arch/mt6577_gpio.h>
	#define LCM_PRINT printf
	#ifndef KERN_INFO
		#define KERN_INFO
	#endif
#else
#include <linux/kernel.h>
	#include <mach/mt6577_gpio.h>
	#define LCM_PRINT printk
#endif
#endif

#if 0
#define LCM_DBG(fmt, arg...) \
	LCM_PRINT("[OTM9608A] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)
#else
#define LCM_DBG(fmt, arg...) do {} while (0)
#endif

#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (540)
#define FRAME_HEIGHT (960)
#define LCM_DSI_CMD_MODE

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

static unsigned char lcm_initialization_setting[LCM_INIT_TABLE_SIZE_MAX] = {
/*	cmd,		count,	params*/
	0x00,	1,		0x00,
	0xFF,	3,		0x96,0x08,0x01,
	0x00,	1,		0x80,
	0xFF,	2,		0x96,0x08,
	0x00,	1,		0x00,
	0x00,	1,		0x80,
	0xD6,	1,		0x00,
	0x00,	1,		0x00,

	0xD4,	30,		0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
	0xD4,	30,		0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
	0xD4,	30,		0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
	0xD4,	30,		0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
	0xD4,	30,		0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
	0xD4,	30,		0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
	0xD4,	30,		0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
	0xD4,	30,		0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
	0xD4,	30,		0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
	0xD4,	30,		0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
	0xD4,	30,		0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
	0xD4,	30,		0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
					0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,
	0x00,	1,		0x00,
	
	0xD5,	30,		0X0,0X60,0X0,0X60,0X0,0X5F,0X0,0X5F,0X0,0X5E,
					0X0,0X5E,0X0,0X5D,0X0,0X5D,0X0,0X5D,0X0,0X5C,
					0X0,0X5C,0X0,0X5B,0X0,0X5B,0X0,0X5A,0X0,0X5A,
	0xD5,	30,		0X0,0X5A,0X0,0X5B,0X0,0X5C,0X0,0X5D,0X0,0X5D,
					0X0,0X5E,0X0,0X5F,0X0,0X60,0X0,0X61,0X0,0X62,
					0X0,0X63,0X0,0X63,0X0,0X64,0X0,0X65,0X0,0X66,
	0xD5,	30,		0X0,0X67,0X0,0X68,0X0,0X69,0X0,0X69,0X0,0X6A,
					0X0,0X6B,0X0,0X6C,0X0,0X6D,0X0,0X6E,0X0,0X6F,
					0X0,0X6F,0X0,0X70,0X0,0X71,0X0,0X72,0X0,0X73,
	0xD5,	30,		0X0,0X74,0X0,0X74,0X0,0X75,0X0,0X76,0X0,0X77,
					0X0,0X78,0X0,0X78,0X0,0X79,0X0,0X7A,0X0,0X7B,
					0X0,0X7C,0X0,0X7D,0X0,0X7D,0X0,0X7E,0X0,0X7F,
	0xD5,	30,		0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
					0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
					0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
	0xD5,	30,		0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
					0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
					0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
	0xD5,	30,		0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
					0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
					0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
	0xD5,	30,		0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
					0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
					0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
	0xD5,	30,		0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
					0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
					0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,0X0,0X7F,
	0xD5,	30,		0X0,0X7F,0X0,0X7F,0X0,0X7E,0X0,0X7D,0X0,0X7C,
					0X0,0X7B,0X0,0X7A,0X0,0X7A,0X0,0X79,0X0,0X78,
					0X0,0X77,0X0,0X76,0X0,0X76,0X0,0X75,0X0,0X74,
	0xD5,	30,		0X0,0X73,0X0,0X72,0X0,0X71,0X0,0X71,0X0,0X70,
					0X0,0X6F,0X0,0X6E,0X0,0X6D,0X0,0X6C,0X0,0X6C,
					0X0,0X6B,0X0,0X6A,0X0,0X69,0X0,0X68,0X0,0X67,
	0xD5,	30,		0X0,0X66,0X0,0X66,0X0,0X66,0X0,0X65,0X0,0X65,
					0X0,0X64,0X0,0X64,0X0,0X63,0X0,0X63,0X0,0X63,
					0X0,0X62,0X0,0X62,0X0,0X61,0X0,0X61,0X0,0X60,
					
	0x00,	1,		0x00,
	0xA0,	1,		0x80,
	0x00,	1,		0x80,
	0xB3,	5,		0x00,0x00,0x00,0x21,0x00,
	0x00,	1,		0x92,
	0xB3,	1,		0x01,
	0x00,	1,		0xC0,
	0xB3,	1,		0x11,
	0x00,	1,		0x80,
	0xC0,	9,		0x00,0x48,0x00,0x06,0x06,0x00,0x48,0x10,0x10,
	0x00,	1,		0x92,
	0xC0,	4,		0x00,0x17,0x00,0x1A,
	0x00,	1,		0xA2,
	0xC0,	3,		0x01,0x10,0x00,
	0x00,	1,		0xB3,
	0xC0,	2,		0x00,0x50,
	0x00,	1,		0x88,
	0xC4,	1,		0x40,
	0x00,	1,		0x81,
	0xC1,	1,		0x55,/*Frame rate 55h--60Hz, 33h--50Hz*/
	0x00,	1,		0x80,
	0xC4,	3,		0x00,0x84,0xFA,
	0x00,	1,		0xA0,
	0xC4,	8,		0x33,0x09,0x90,0x2B,0x33,0x09,0x90,0x54,
	0x00,	1,		0x80,
	0xC5,	4,		0x08,0x00,0x90,0x11,
	0x00,	1,		0x90,
	0xC5,	7,		0x96,0x76,0x06,0x76,0x33,0x33,0x34,
	0x00,	1,		0xA0,
	0xC5,	7,		0x96,0x77,0x00,0x72,0x33,0x33,0x34,
	0x00,	1,		0xB0,
	0xC5,	2,		0x04,0xF8,
	0x00,	1,		0x80,
	0xC6,	1,		0x64,
	0x00,	1,		0xB0,
	0xC6,	5,		0x03,0x10,0x00,0x1F,0x12,
	0x00,	1,		0xE1,
	0xC0,	1,		0x9F,
	0x00,	1,		0x00,
	0xD0,	1,		0x01,
	0x00,	1,		0x00,
	0xD1,	2,		0x01,0x01,
	0x00,	1,		0xB7,
	0xB0,	1,		0x10,
	0x00,	1,		0xC0,
	0xB0,	1,		0x55,
	0x00,	1,		0xB1,
	0xB0,	2,		0x03,0x06,
	0x00,	1,		0x80,
	0xCB,	10,		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	1,		0x90,
	0xCB,	15,		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	1,		0xA0,
	0xCB,	15,		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	1,		0xB0,
	0xCB,	10,		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	1,		0xC0,
	0xCB,	15,		0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	1,		0xD0,
	0xCB,	15,		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,
	0x00,	1,		0xE0,
	0xCB,	10,		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	1,		0xF0,
	0xCB,	10,		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
       	    	
	0x00,	1,		0x80,
	0xCC,	10,		0x00,0x00,0x0B,0x09,0x01,0x25,0x26,0x00,0x00,0x00,
	0x00,	1,		0x90,
	0xCC,	15,		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x0A,0x02,
	0x00,	1,		0xA0,
	0xCC,	15,		0x25,0x26,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,	1,		0xB0,
	0xCC,	10,		0x00,0x00,0x0A,0x0C,0x02,0x26,0x25,0x00,0x00,0x00,
	0x00,	1,		0xC0,
	0xCC,	15,		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x09,0x0B,0x01,
	0x00,	1,		0xD0,
	0xCC,	15,		0x26,0x25,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
       	    	
	0x00,	1,		0x80,
	0xCE,	12,		0x86,0x01,0x00,0x85,0x01,0x00,0x02,0x00,0x00,0x0F,0x00,0x00,
	0x00,	1,		0x90,
	0xCE,	14,		0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0x00,0x00,
	0x00,	1,		0xA0,
	0xCE,	14,		0x18,0x02,0x03,0xC4,0x00,0x80,0x14,0x18,0x01,0x03,0xC4,0x00,0x80,0x14,
	0x00,	1,		0xB0,
	0xCE,	14,		0x18,0x04,0x03,0xC2,0x00,0x80,0x14,0x18,0x03,0x03,0xC2,0x00,0x80,0x14,
	0x00,	1,		0xC0,
	0xCE,	14,		0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,
	0x00,	1,		0xD0,
	0xCE,	14,		0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,
       	    	
	0x00,	1,		0x80,
	0xCF,	14,		0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,
	0x00,	1,		0x90,
	0xCF,	14,		0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,
	0x00,	1,		0xA0,
	0xCF,	14,		0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,
	0x00,	1,		0xB0,
	0xCF,	14,		0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00,
	0x00,	1,		0xC0,
	0xCF,	10,		0x02,0x02,0x20,0x20,0x00,0x00,0x01,0x00,0x10,0x00,
	0x00,	1,		0x00,
	0xD7,	1,		0x00,
	0x00,	1,		0x00,
	0xD8,	2,		0x89,0x89,
	0x00,	1,		0x00,
	0xD9,	1,		0x60,
	0x00,	1,		0x00,
	0xE1,	8,		0X0,0X4,0X8,0XB,0X4,0XC,0XB,0XA,
	0xE1,	8,		0X3,0X6,0XD,0X4,0X9,0X13,0XA,0X2,
	0x00,	1,		0x00,
	0xE1,	8,		0X0,0X4,0X8,0XB,0X4,0XC,0XB,0XA,
	0xE1,	8,		0X3,0X6,0XD,0X4,0X9,0X13,0XA,0X2,
       	  		
	0x00,	1,		0x00,
	0xFF,	3,		0xFF,0xFF,0xFF,
       	  		
	0x00,	1,		0x00,
	0x3A,	1,		0x77,
	0x00,	1,		0x00,
//0x36,	1,		0xD0, /*LCM layout rotate 180 degree on PCB*/
	0x35,	1,		0x00,/*TE ON, mode=0*/
	0x44,	2,		0x01, 0xE0,/* TE start line = 960/2 */
		
	0x36,	1,		0x00,
		
	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.
	REGFLAG_DELAY, 1,

	// Setting ending by predefined flag
	REGFLAG_END_OF_TABLE
};

static unsigned char lcm_sleep_out_setting[] = {
	// Sleep Out
	0x11, 0, 
	REGFLAG_DELAY, 120,

	// Display ON
	0x29, 0,
	REGFLAG_DELAY, 20,
	
	REGFLAG_END_OF_TABLE
};

static unsigned char lcm_sleep_mode_in_setting[] = {
	// Display off sequence
	0x28, 0,
	REGFLAG_DELAY, 100,

	// Sleep Mode On
	0x10, 0, 
	REGFLAG_DELAY, 200,
	REGFLAG_END_OF_TABLE
};

static unsigned char lcm_compare_id_setting[] = {
	// Display off sequence
	0xF0,	5,	0x55, 0xaa, 0x52,0x08,0x00,
	
	REGFLAG_DELAY, 10,

	REGFLAG_END_OF_TABLE
};

static int push_table(unsigned char table[])
{
	unsigned int i, bExit = 0;
	unsigned char *p = (unsigned char *)table;
	LCM_SETTING_ITEM *pSetting_item;

	while(!bExit) {
		pSetting_item = (LCM_SETTING_ITEM *)p;

		switch (pSetting_item->cmd) {
			
		case REGFLAG_DELAY :
			MDELAY(pSetting_item->count);
			p += 2;
		break;

		case REGFLAG_END_OF_TABLE :
			p += 2;
			bExit = 1;
		break;

		default:
			dsi_set_cmdq_V2(pSetting_item->cmd, 
							pSetting_item->count, pSetting_item->params, 1);
			MDELAY(2);
			p += pSetting_item->count + 2;
		break;
		}
	}
	return p - table; //return the size of  settings array.
}

#if !defined(BUILD_UBOOT) && !defined(BUILD_LK)
static int get_initialization_settings(unsigned char table[])
{
	memcpy(table, lcm_initialization_setting, sizeof(lcm_initialization_setting));
	return sizeof(lcm_initialization_setting);
}

static void lcm_init(void);
static void lcm_reset(void);

static int set_initialization_settings(const unsigned char table[], const int count)
{
	if ( count > LCM_INIT_TABLE_SIZE_MAX ){
		return -EIO;
	}
	memset(lcm_initialization_setting, REGFLAG_END_OF_TABLE, sizeof(lcm_initialization_setting));
	memcpy(lcm_initialization_setting, table, count);

	lcm_reset();
	lcm_init();
	push_table(lcm_sleep_out_setting);

	return count;
}
#endif
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	// enable tearing-free
	params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if defined(LCM_DSI_CMD_MODE)
	params->dsi.mode   = CMD_MODE;
#else
	params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	// Not support in MT6573

		params->dsi.DSI_WMEM_CONTI=0x3C; 
		params->dsi.DSI_RMEM_CONTI=0x3E; 


	params->dsi.packet_size=256;

	// Video mode setting		
	params->dsi.intermediat_buffer_num = 2;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active				= 3;
	params->dsi.vertical_backporch					= 6;
	params->dsi.vertical_frontporch					= 6;
	params->dsi.vertical_active_line				= FRAME_HEIGHT;

	params->dsi.horizontal_sync_active				= 10;
	params->dsi.horizontal_backporch				= 20;
	params->dsi.horizontal_frontporch				= 40;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	// Bit rate calculation
	params->dsi.pll_div1=20;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
	params->dsi.pll_div2=0;			// div2=0~15: fout=fvo/(2*div2)
	params->dsi.LPX      = 13; //read =7-12, write=13

}

static void lcm_reset(void)
{
	SET_RESET_PIN(0);
	MDELAY(2);
	SET_RESET_PIN(1);
	MDELAY(128);
}

static void lcm_init(void)
{
	LCM_DBG();
    push_table(lcm_initialization_setting);
}

static void lcm_suspend(void)
{
	unsigned int data_array[2];
	LCM_DBG();

	data_array[0] = 0x00280500; // Display Off
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(10); 
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(100);
}

static void lcm_resume(void)
{
	LCM_DBG();
#if defined(BUILD_UBOOT) || defined(BUILD_LK)
	lcm_reset();
	lcm_init();
#endif
	push_table(lcm_sleep_out_setting);
}

static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];
	
	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(data_array, 7, 0);
	
}

#if defined(BUILD_UBOOT) || defined(BUILD_LK)
#include "cust_adc.h"
#define LCM_MAX_VOLTAGE 1600 
#define LCM_MIN_VOLTAGE 1200 

extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);

static unsigned int lcm_adc_read_chip_id()
{
	int data[4] = {0, 0, 0, 0};
	int tmp = 0, rc = 0, iVoltage = 0;
	rc = IMM_GetOneChannelValue(AUXADC_LCD_ID_CHANNEL, data, &tmp);
	if(rc < 0) {
		printf("read LCD_ID vol error--Liu\n");
		return 0;
	}
	else {
		iVoltage = (data[0]*1000) + (data[1]*10) + (data[2]);
		printf("read LCD_ID success, data[0]=%d, data[1]=%d, data[2]=%d, data[3]=%d, iVoltage=%d\n", 
			data[0], data[1], data[2], data[3], iVoltage);
		if(	LCM_MIN_VOLTAGE < iVoltage &&
			iVoltage < LCM_MAX_VOLTAGE)
			return 1;
		else
			return 0;
	}
	return 0;
}
#endif	

static unsigned int lcm_compare_id(void)
{

	int   array[4];
	char  buffer[3];
	char  id0=0;
	char  id1=0;
	char  id2=0;


	lcm_reset();//must be ahead of this function.
		
#if defined(BUILD_UBOOT) || defined(BUILD_LK)
	if(lcm_adc_read_chip_id())
		return 1;
#endif
		
	array[0] = 0x00033700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xDA,buffer, 1);

	
	array[0] = 0x00033700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xDB,buffer+1, 1);

	
	array[0] = 0x00033700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xDC,buffer+2, 1);
	
	id0 = buffer[0]; //should be 0x00
	id1 = buffer[1];//should be 0xaa
	id2 = buffer[2];//should be 0x55
#if defined(BUILD_UBOOT) || defined(BUILD_LK)
	printf("%s, id0 = 0x%08x\n", __func__, id0);//should be 0x00
	printf("%s, id1 = 0x%08x\n", __func__, id1);//should be 0xaa
	printf("%s, id2 = 0x%08x\n", __func__, id2);//should be 0x55
#endif
	
	return 1;
}

LCM_DRIVER otm9608a_dsi_lcm_drv = 
{
	.name			= "otm9608a_dsi",
	.set_util_funcs = lcm_set_util_funcs,
	.compare_id     = lcm_compare_id,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
#if defined(LCM_DSI_CMD_MODE)
	.update         = lcm_update,
#endif
#if !defined(BUILD_UBOOT) && !defined(BUILD_LK)
    .get_initialization_settings = get_initialization_settings,
    .set_initialization_settings = set_initialization_settings,
#endif
    };
