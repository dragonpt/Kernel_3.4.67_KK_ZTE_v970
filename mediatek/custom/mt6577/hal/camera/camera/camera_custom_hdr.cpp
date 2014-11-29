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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#include "camera_custom_hdr.h"



/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define MAX_HDR_GAIN_ARRAY_ELEM		11	// Maximun HDR GainArray element number.

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static MUINT32 au4HdrGainArray[MAX_HDR_GAIN_ARRAY_ELEM] =
{
	CUST_HDR_GAIN_00,
	CUST_HDR_GAIN_01,
	CUST_HDR_GAIN_02,
	CUST_HDR_GAIN_03,
	CUST_HDR_GAIN_04,
	CUST_HDR_GAIN_05,
	CUST_HDR_GAIN_06,
	CUST_HDR_GAIN_07,
	CUST_HDR_GAIN_08,
	CUST_HDR_GAIN_09,
	CUST_HDR_GAIN_10,
};

/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for Core Number.
///////////////////////////////////////////////////////////////////////////
MUINT32 CustomHdrCoreNumberGet(void)
{
	return CUST_HDR_CORE_NUMBER;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Get prolonged VD number.
///////////////////////////////////////////////////////////////////////////
MUINT32 CustomHdrProlongedVdGet(void)
{
	return CUST_HDR_PROLONGED_VD;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for BlurRatio.
///////////////////////////////////////////////////////////////////////////
MUINT32 CustomHdrBlurRatioGet(void)
{
	return CUST_HDR_BLUR_RATIO;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for Gain.
///////////////////////////////////////////////////////////////////////////
MUINT32 CustomHdrGainArrayGet(MUINT32 u4ArrayIndex)
{
	if (u4ArrayIndex > MAX_HDR_GAIN_ARRAY_ELEM-1)
		u4ArrayIndex = MAX_HDR_GAIN_ARRAY_ELEM-1;

	return au4HdrGainArray[u4ArrayIndex];
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for BottomFlareRatio.
///////////////////////////////////////////////////////////////////////////
double CustomHdrBottomFlareRatioGet(void)
{
	return CUST_HDR_BOTTOM_FLARE_RATIO;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for TopFlareRatio.
///////////////////////////////////////////////////////////////////////////
double CustomHdrTopFlareRatioGet(void)
{
	return CUST_HDR_TOP_FLARE_RATIO;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for BottomFlareBound.
///////////////////////////////////////////////////////////////////////////
MUINT32 CustomHdrBottomFlareBoundGet(void)
{
	return CUST_HDR_BOTTOM_FLARE_BOUND;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for TopFlareBound.
///////////////////////////////////////////////////////////////////////////
MUINT32 CustomHdrTopFlareBoundGet(void)
{
	return CUST_HDR_TOP_FLARE_BOUND;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for ThHigh.
///////////////////////////////////////////////////////////////////////////
MINT32 CustomHdrThHighGet(void)
{
	return CUST_HDR_TH_HIGH;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for ThLow.
///////////////////////////////////////////////////////////////////////////
MINT32 CustomHdrThLowGet(void)
{
	return CUST_HDR_TH_LOW;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get the customer-set value for TargetLevelSub.
///////////////////////////////////////////////////////////////////////////
MUINT32 CustomHdrTargetLevelSubGet(void)
{
	return CUST_HDR_TARGET_LEVEL_SUB;
}

/*******************************************************************************
*
*******************************************************************************/

