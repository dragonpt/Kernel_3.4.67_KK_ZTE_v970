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
#ifndef MT6516
#ifndef _ISP_TUNING_CUSTOM_INSTANCE_H_
#define _ISP_TUNING_CUSTOM_INSTANCE_H_


namespace NSIspTuning
{


/*******************************************************************************
* 
*******************************************************************************/
template <ESensorRole_T eSensorRole>
class CTIspTuningCustom : public IspTuningCustom
{
public:
    static
    IspTuningCustom*
    getInstance()
    {
        static CTIspTuningCustom<eSensorRole> singleton;
        return &singleton;
    }
    virtual void destroyInstance() {}

    CTIspTuningCustom()
        : IspTuningCustom()
        , m_rIdxSetMgr(IdxSetMgrBase::getInstance<eSensorRole>())
    {}

public:     ////    Attributes
    virtual ESensorRole_T   getSensorRole() const { return eSensorRole; }
    virtual
    INDEX_T const*
    getDefaultIndex(
        ECamMode_T const eCamMode, 
        EIndex_Scene_T const eIdx_Scene, 
        EIndex_ISO_T const eIdx_ISO
    ) const
    {
        return m_rIdxSetMgr.get(eCamMode, eIdx_Scene, eIdx_ISO);
    }

protected:  ////    Data Members.
    IdxSetMgrBase&  m_rIdxSetMgr;

};


/*******************************************************************************
* Customers can specialize CTIspTuningCustom<xxx> 
* and then override default behaviors if needed.
*******************************************************************************/
#if 0
/*
    ps: where ESensorRole_xxx = ESensorRole_Main/ESensorRole_Sub
*/
template <>
class CTIspTuningCustom< ESensorRole_Main > : public IspTuningCustom
{
public:
    static
    IspTuningCustom*
    getInstance()
    {
        static CTIspTuningCustom< ESensorRole_Main > singleton;
        return &singleton;
    }
    virtual void destroyInstance() {}

public:     ////    Overrided Interfaces.
    //......
};

#endif


/*******************************************************************************
* 
*******************************************************************************/
#define INSTANTIATE(_role_id) \
    case _role_id: return  CTIspTuningCustom<_role_id>::getInstance()

IspTuningCustom*
IspTuningCustom::
createInstance(ESensorRole_T const eSensorRole)
{
    switch  (eSensorRole)
    {
    INSTANTIATE(ESensorRole_Main);  //  Main Sensor
    INSTANTIATE(ESensorRole_Sub);   //  Sub Sensor
    default:
        break;
    }

    return  NULL;
}


};  //  NSIspTuning
#endif  //  _ISP_TUNING_CUSTOM_INSTANCE_H_
#endif  //  ! MT6516

