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
#ifndef _CAMERA_CUSTOM_HDR_H_
#define _CAMERA_CUSTOM_HDR_H_

#include "camera_custom_types.h"	// For MUINT*/MINT*/MVOID/MBOOL type definitions.

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
// For HDR Customer Parameters

// [Core Number]
//     - Value Range: 1(For single-core)/2(For multi-core).
#define CUST_HDR_CORE_NUMBER	2

// [Prolonged VD]
//     - Value Range: 1(default)~ (depend on sensor characteristics).
#define CUST_HDR_PROLONGED_VD	1

// [Blurring Ratio]
//     - Higher value:
//         - Decrease the degree of artifact (halo-like around object boundary).
//         - Avoid non-continued edge in fusion result.
//         - Decrease little dynamic range.
//         - Decrease sharpness.
//     - Value Range: 1 (non-blur) ~ 160. (Default: 40).
#define CUST_HDR_BLUR_RATIO		40

// [Gain]
//     - Higher value increase sharpness, but also increase noise.
//     - Value Range: 256 ~ 512. (Default: 256 (1x gain)) (384/256 = 1.5x gain)
#define CUST_HDR_GAIN_00	384
#define CUST_HDR_GAIN_01	384
#define CUST_HDR_GAIN_02	384
#define CUST_HDR_GAIN_03	256
#define CUST_HDR_GAIN_04	256
#define CUST_HDR_GAIN_05	256
#define CUST_HDR_GAIN_06	256
#define CUST_HDR_GAIN_07	256
#define CUST_HDR_GAIN_08	256
#define CUST_HDR_GAIN_09	256
#define CUST_HDR_GAIN_10	256

// [Flare Control]
//     -Higher value decreases the degree of flare, but also decrease parts of the image details.
//     - Value Range: 0 ~ 50. (Default: 10)
#define CUST_HDR_BOTTOM_FLARE_RATIO		10
//     - Value Range: 0 ~ 50. (Default: 10)
#define CUST_HDR_TOP_FLARE_RATIO		10
//     - Value Range: 0 ~ 24. (Default: 16)
#define CUST_HDR_BOTTOM_FLARE_BOUND		16
//     - Value Range: 0 ~ 24. (Default: 16)
#define CUST_HDR_TOP_FLARE_BOUND		16

// [De-halo Control]
//     - Higher value reduce more halo for sky, but also reduce more dynamic range in some parts of the image.
//     - Value Range: 0 (off) ~ 255. (Default: 245)
#define CUST_HDR_TH_HIGH				245

// [Noise Control]
//     - Higher value reduce more noise, but also reduce dynamic range in low light region.
//     - Value Range: 0 (off) ~ 255. (Default: 25)
#define CUST_HDR_TH_LOW					0

// [Level Subtract]
//     - Value Range: 0 (more low-frequency halo, more dynamic range) or 1 (less low-frequency halo, less dynamic range). (Default: 0)
#define CUST_HDR_TARGET_LEVEL_SUB		0


/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/
MUINT32 CustomHdrCoreNumberGet(void);
MUINT32 CustomHdrProlongedVdGet(void);
MUINT32 CustomHdrBlurRatioGet(void);
MUINT32 CustomHdrGainArrayGet(MUINT32 u4ArrayIndex);
double CustomHdrBottomFlareRatioGet(void);
double CustomHdrTopFlareRatioGet(void);
MUINT32 CustomHdrBottomFlareBoundGet(void);
MUINT32 CustomHdrTopFlareBoundGet(void);
MINT32 CustomHdrThHighGet(void);
MINT32 CustomHdrThLowGet(void);
MUINT32 CustomHdrTargetLevelSubGet(void);

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/




#endif	// _CAMERA_CUSTOM_HDR_H_

