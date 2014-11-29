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
#ifndef _ISP_TUNING_CUSTOM_MT6575_H_
#define _ISP_TUNING_CUSTOM_MT6575_H_


#include "camera_custom_nvram.h"


namespace NSIspTuning
{


/*******************************************************************************
*
*******************************************************************************/
class IspTuningCustom
{
protected:  ////    Ctor/Dtor.
    IspTuningCustom() {}
    virtual ~IspTuningCustom() {}

public:
    static IspTuningCustom* createInstance(ESensorRole_T const eSensorRole);
    virtual void destroyInstance() = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Attributes
    virtual ESensorRole_T   getSensorRole() const = 0;
    virtual INDEX_T const*  getDefaultIndex(
                                ECamMode_T const eCamMode,
                                EIndex_Scene_T const eIdx_Scene,
                                EIndex_ISO_T const eIdx_ISO
                            ) const = 0;

public:     ////    Operations.

    virtual
    MBOOL
    is_to_invoke_offline_capture(
        RAWIspCamInfo const& rCamInfo
    ) const;

public:     ////    NVRAM

    virtual
    MVOID
    evaluate_nvram_index(
        RAWIspCamInfo const& rCamInfo, IndexMgr& rIdxMgr
    );

    virtual
    MVOID
    refine_NR1(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_NR1_T& rNR1
    );

    virtual
    MVOID
    refine_DP(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_DP_T& rDP
    );    

    virtual
    MVOID
    refine_NR2(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_NR2_T& rNR2
    );

    virtual
    MVOID
    refine_DM(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_DEMOSAIC_T& rDM
    );

    virtual
    MVOID
    refine_EE(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_EE_T& rEE
    );

    virtual
    MVOID
    refine_Saturation(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_SATURATION_T& rSaturation
    );

    virtual
    MVOID
    refine_Contrast(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_CONTRAST_T& rContrast
    );

    virtual
    MVOID
    refine_Hue(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_HUE_T& rHue
    );

    virtual
    MVOID
    refine_CCM(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_CCM_T& rCCM
    );

    virtual
    MVOID
    refine_OB(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_OB_T& rOB
    );

    virtual
    MVOID
    refine_GammaECO(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_GAMMA_ECO_T& rGammaECO
    );

    virtual
    MVOID
    refine_RGB2YCC_YOfst(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_RGB2YCC_YOFST_T& rRGB2YCC_YOfst
    );

    virtual
    MVOID
    refine_ColorClip(
        RAWIspCamInfo const& rCamInfo, ISP_NVRAM_CCLIP_T& rColorClip
    );

    virtual
    MVOID
    prepare_edge_gamma(ISP_NVRAM_EDGE_GAMMA_T& rEGamma);

public:     ////    Color Temperature Index: CCM, Shading

    virtual
    EIndex_CCM_CCT_T
    evaluate_CCM_CCT_index  (
        EIndex_CCM_CCT_T const eIdx_CCM_CCT_old,
        MINT32 const i4CCT,
        MINT32 const i4FluorescentIndex
    ) const;

    virtual
    EIndex_Shading_CCT_T
    evaluate_Shading_CCT_index  (
        EIndex_Shading_CCT_T const eIdx_Shading_CCT_old,
        MINT32 const i4CCT,
        MINT32 const i4DaylightFluorescentIndex,
        MINT32 const i4DaylightProb,
        MINT32 const i4DaylightFluorescentProb,
        MINT32 const i4SceneLV
    ) const;

public:     ////    ISO

    virtual
    EIndex_ISO_T
    map_ISO_value_to_index(
        MUINT32 const u4Iso
    ) const;

public:     ////    Effect

    template <EIndex_Effect_T eEffect>
    MVOID
    prepare_effect(ISP_EFFECT_T& rEffect);

public:     ////    End-User Setting Level.

    template <class ISP_NVRAM_xxx_T>
    MUINT32
    map_user_setting_to_nvram_index(
        MUINT8 const u8Idx_nvram_current,
        IspUsrSelectLevel_T const& rUsr
    );

};


};  //  NSIspTuning
#endif //  _ISP_TUNING_CUSTOM_H_

