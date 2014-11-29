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
#ifndef _ISP_TUNING_H_
#define _ISP_TUNING_H_


namespace NSIspTuning
{


/*******************************************************************************
* 
*******************************************************************************/
typedef enum MERROR_ENUM
{
    MERR_OK         = 0, 
    MERR_UNKNOWN    = 0x80000000, // Unknown error
    MERR_UNSUPPORT, 
    MERR_BAD_PARAM, 
    MERR_BAD_CTRL_CODE, 
    MERR_BAD_FORMAT, 
    MERR_BAD_ISP_DRV, 
    MERR_BAD_NVRAM_DRV, 
    MERR_BAD_SENSOR_DRV, 
    MERR_BAD_SYSRAM_DRV, 
    MERR_SET_ISP_REG, 
    MERR_NO_MEM, 
    MERR_NO_SYSRAM_MEM, 
    MERR_NO_RESOURCE, 
    MERR_CUSTOM_DEFAULT_INDEX_NOT_FOUND, 
    MERR_CUSTOM_NOT_READY, 
    MERR_PREPARE_HW, 
    MERR_APPLY_TO_HW
} MERROR_ENUM_T;


/*******************************************************************************
* Operation Mode
*******************************************************************************/
typedef enum
{
    EOperMode_Normal    = 0, 
    EOperMode_PureRaw, 
    EOperMode_Meta, 
} EOperMode_T;


/*******************************************************************************
* 
*******************************************************************************/
typedef enum
{
    //  NORMAL
    ECamMode_Video              = 0, 
    ECamMode_Online_Preview,
    ECamMode_Online_Capture, 
    ECamMode_Online_Capture_ZSD,
    ECamMode_Offline_Capture_Pass1, 
    ECamMode_Offline_Capture_Pass2, 
    //  HDR
    ECamMode_HDR_Cap_Pass1_SF,  //  Pass1: Single Frame
    ECamMode_HDR_Cap_Pass1_MF1, //  Pass1: Multi Frame Stage1
    ECamMode_HDR_Cap_Pass1_MF2, //  Pass1: Multi Frame Stage2
    ECamMode_HDR_Cap_Pass2,     //  Pass2
    // YUV2JPG
    ECamMode_YUV2JPG_Scalado,   // YUV -> JPG (Scalado): disable EE, NR2, PCA, YCCGO
    ECamMode_YUV2JPG_ZSD,       // YUV -> JPG (ZSD): enable NR2, disable EE, PCA, YCCGO
    // Face Beautifier
    ECamMode_FB_PostProcess_NR2_Only, // do NR2, disable EE, PCA, and YCCGO
    ECamMode_FB_PostProcess_PCA_Only,  // do PCA, disable EE, NR2, and YCCGO
#ifdef MTK_ZSD_AF_ENHANCE
    ECamMode_Online_Preview_ZSD
#endif
} ECamMode_T;


/*******************************************************************************
* 
*******************************************************************************/
typedef enum
{
    ESensorRole_Main = 0,   //   Main Sensor
    ESensorRole_Sub,        //   Sub Sensor
    ESensorRole_ATV, 
}   ESensorRole_T;


typedef enum
{
    ESensorType_RAW = 0,    //  RAW Sensor
    ESensorType_YUV,        //  YUV Sensor
}   ESensorType_T;


};  //  NSIspTuning


/*******************************************************************************
* 
*******************************************************************************/
#ifdef  USE_CUSTOM_ISP_TUNING

    #include "camera_feature.h"
    using namespace NSFeature;
    #include "isp_tuning_debug.h"
    #include "isp_tuning_cam_info.h"
    #include "isp_tuning_idx.h"
    #include "isp_tuning_custom.h"

#endif  //  USE_CUSTOM_ISP_TUNING
/*******************************************************************************
* 
*******************************************************************************/


#endif //  _ISP_TUNING_H_

