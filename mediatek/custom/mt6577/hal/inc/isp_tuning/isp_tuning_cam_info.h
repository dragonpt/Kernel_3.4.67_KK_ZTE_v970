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
#ifndef _ISP_TUNING_CAM_INFO_H_
#define _ISP_TUNING_CAM_INFO_H_

#include <cutils/xlog.h>
namespace NSIspTuning
{


/*******************************************************************************
* 
*******************************************************************************/

//  Scene index
typedef Fid2Type<FID_SCENE_MODE>::Type      EIndex_Scene_T;
enum { eNUM_OF_SCENE_IDX = Fid2Type<FID_SCENE_MODE>::Num };


//  Color Effect Index
typedef Fid2Type<FID_COLOR_EFFECT>::Type    EIndex_Effect_T;


//  ISP End-User-Define Tuning Index: 
//  Edge, Hue, Saturation, Brightness, Contrast
typedef Fid2Type<FID_ISP_EDGE    >::Type    EIndex_Isp_Edge_T;
typedef Fid2Type<FID_ISP_HUE     >::Type    EIndex_Isp_Hue_T;
typedef Fid2Type<FID_ISP_SAT     >::Type    EIndex_Isp_Saturation_T;
typedef Fid2Type<FID_ISP_BRIGHT  >::Type    EIndex_Isp_Brightness_T;
typedef Fid2Type<FID_ISP_CONTRAST>::Type    EIndex_Isp_Contrast_T;
typedef struct IspUsrSelectLevel
{
    EIndex_Isp_Edge_T           eIdx_Edge;
    EIndex_Isp_Hue_T            eIdx_Hue;
    EIndex_Isp_Saturation_T     eIdx_Sat;
    EIndex_Isp_Brightness_T     eIdx_Bright;
    EIndex_Isp_Contrast_T       eIdx_Contrast;

    IspUsrSelectLevel()
        : eIdx_Edge     (ISP_EDGE_MIDDLE)
        , eIdx_Hue      (ISP_HUE_MIDDLE)
        , eIdx_Sat      (ISP_SAT_MIDDLE)
        , eIdx_Bright   (ISP_BRIGHT_MIDDLE)
        , eIdx_Contrast (ISP_CONTRAST_MIDDLE)
    {}
} IspUsrSelectLevel_T;


//  ISO index.
typedef enum EIndex_ISO
{
    eIDX_ISO_100 = 0, 
    eIDX_ISO_200, 
    eIDX_ISO_400, 
    eIDX_ISO_800, 
    eIDX_ISO_1600, 
    eNUM_OF_ISO_IDX
} EIndex_ISO_T;


//  Correlated color temperature index for CCM.
typedef enum EIndex_CCM_CCT
{
    eIDX_CCM_CCT_BEGIN  = 0, 
    eIDX_CCM_CCT_TL84   = eIDX_CCM_CCT_BEGIN, 
    eIDX_CCM_CCT_CWF, 
    eIDX_CCM_CCT_D65
} EIndex_CCM_CCT_T;


//  Correlated color temperature index for shading.
typedef enum EIndex_Shading_CCT
{
    eIDX_Shading_CCT_BEGIN  = 0, 
    eIDX_Shading_CCT_ALight   = eIDX_Shading_CCT_BEGIN, 
    eIDX_Shading_CCT_CWF, 
    eIDX_Shading_CCT_D65
} EIndex_Shading_CCT_T;

// CCT estimation
typedef struct
{
	MINT32 i4CCT; // CCT
	MINT32 i4FluorescentIndex; // Fluorescent index
	MINT32 i4DaylightFluorescentIndex; // Daylight fluorescent index
	MINT32 i4DaylightProb; // Daylight probability
	MINT32 i4DaylightFluorescentProb; // Daylight fluorescent probability
	MINT32 i4SceneLV; // Scene LV
} ISP_CCT_T;


/*******************************************************************************
* 
*******************************************************************************/
struct IspCamInfo
{
public:
    ECamMode_T          eCamMode;   //  camera mode.

    EIndex_Scene_T      eIdx_Scene; //  scene mode.

public:
    IspCamInfo()
        : eCamMode(ECamMode_Online_Preview)
        , eIdx_Scene(SCENE_MODE_OFF)
    {}

public:
    void dump() const
    {
        XLOGD("[IspCamInfo][dump](eCamMode, eIdx_Scene)=(%d, %d)", eCamMode, eIdx_Scene);
    }
};


/*******************************************************************************
* 
*******************************************************************************/
struct RAWIspCamInfo : public IspCamInfo
{
public:
    MUINT32             u4ISOValue;         //  iso value
    EIndex_ISO_T        eIdx_ISO;           //  iso index

    MINT32              i4CCT;              //  color temperature.
    EIndex_CCM_CCT_T    eIdx_CCM_CCT;       //  CT index for CCM.
    EIndex_Shading_CCT_T    eIdx_Shading_CCT;       //  CT index for Shading.

    /*
        x100 Zoom Ratio
    */
    MUINT32             u4ZoomRatio_x100;

    /*
        x10 "light value" or ¡§light level¡¨
    
        In photography, light value has been used to refer to a ¡§light level¡¨ 
        for either incident or reflected light, often on a base-2 logarithmic scale.
        The term does not derive from a published standard, and has had several 
        different meanings:
    */
    MINT32              i4LightValue_x10;

public:
    RAWIspCamInfo()
        : IspCamInfo()
        , u4ISOValue(0)
        , eIdx_ISO(eIDX_ISO_100)
        , i4CCT(0)
        , eIdx_CCM_CCT(eIDX_CCM_CCT_BEGIN)
        , eIdx_Shading_CCT(eIDX_Shading_CCT_CWF)
        , u4ZoomRatio_x100(0)
        , i4LightValue_x10(0)
    {}

public:
    void dump() const
    {
        IspCamInfo::dump();
        XLOGD(
            "[RAWIspCamInfo][dump]"
            "(eIdx_ISO, u4ISOValue, i4CCT, eIdx_CCM_CCT, u4ZoomRatio_x100, i4LightValue_x10)"
            "=(%d, %d, %d, %d, %d, %d)"
            , eIdx_ISO, u4ISOValue, i4CCT, eIdx_CCM_CCT, u4ZoomRatio_x100, i4LightValue_x10
        );
    }
};


/*******************************************************************************
* 
*******************************************************************************/
struct YUVIspCamInfo : public IspCamInfo
{
public:
    YUVIspCamInfo()
        : IspCamInfo()
    {}
};


/*******************************************************************************
* 
*******************************************************************************/
};  //  NSIspTuning
#endif //  _ISP_TUNING_CAM_INFO_H_

