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
#ifndef _ISP_TUNING_IDX_MT6575_H_
#define _ISP_TUNING_IDX_MT6575_H_


namespace NSIspTuning
{


/*******************************************************************************
* Index
*******************************************************************************/
typedef struct CUSTOM_NVRAM_REG_INDEX
{
    MUINT8  DM;
    MUINT8  DP;
    MUINT8  NR1;
    MUINT8  NR2;
    MUINT8  Saturation;
    MUINT8  Contrast;
    MUINT8  Hue;
    MUINT8  Gamma;
    MUINT8  EE;
} CUSTOM_NVRAM_REG_INDEX_T;

typedef CUSTOM_NVRAM_REG_INDEX    INDEX_T;


//==============================================================================
struct IndexMgr : protected INDEX_T
{
public:
    IndexMgr()
    {
        ::memset(static_cast<INDEX_T*>(this), 0, sizeof(INDEX_T));
    }

    IndexMgr(INDEX_T const& rIndex)
    {
        (*this) = rIndex;
    }

    IndexMgr& operator=(INDEX_T const& rIndex)
    {
        *static_cast<INDEX_T*>(this) = rIndex;
        return  (*this);
    }

public:
    void dump() const;

public:     ////    Set Index
    MBOOL   setIdx_DM           (MUINT8 const idx);
    MBOOL   setIdx_DP           (MUINT8 const idx);
    MBOOL   setIdx_NR1          (MUINT8 const idx);
    MBOOL   setIdx_NR2          (MUINT8 const idx);
    MBOOL   setIdx_Saturation   (MUINT8 const idx);
    MBOOL   setIdx_Contrast     (MUINT8 const idx);
    MBOOL   setIdx_Hue          (MUINT8 const idx);
    MBOOL   setIdx_Gamma        (MUINT8 const idx);
    MBOOL   setIdx_EE           (MUINT8 const idx);
public:     ////    Get Index
    inline  MUINT8 getIdx_DM()          const { return DM; }
    inline  MUINT8 getIdx_DP()          const { return DP; }
    inline  MUINT8 getIdx_NR1()         const { return NR1; }
    inline  MUINT8 getIdx_NR2()         const { return NR2; }
    inline  MUINT8 getIdx_Saturation()  const { return Saturation; }
    inline  MUINT8 getIdx_Contrast()    const { return Contrast; }
    inline  MUINT8 getIdx_Hue()         const { return Hue; }
    inline  MUINT8 getIdx_Gamma()       const { return Gamma; }
    inline  MUINT8 getIdx_EE()          const { return EE; }
};


/*******************************************************************************
* Index Set Template
*******************************************************************************/
template <ESensorRole_T role, ECamMode_T mode, MUINT32 m = -1, MUINT32 n = -1>
struct IdxSet
{
    static INDEX_T const idx;
};


/*******************************************************************************
* Index Set Manager Base
*******************************************************************************/
class IdxSetMgrBase
{
public:

    template <ESensorRole_T eSensorRole> static IdxSetMgrBase& getInstance();

    virtual ~IdxSetMgrBase() {}

public:
    virtual INDEX_T const*
    get(
        MUINT32 const mode, MUINT32 const i=-1, MUINT32 const j=-1
    ) const = 0;
};


/*******************************************************************************
* ISP Tuning Index Manager for MT6575
*******************************************************************************/
class IdxSetMgr_MT6575 : public IdxSetMgrBase
{
    friend class IdxSetMgrBase;

private:
    INDEX_T const* m_pOnlinePreview      [eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pOnlineCapture      [eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pOfflineCapturePass1[eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pOfflineCapturePass2[eNUM_OF_SCENE_IDX][eNUM_OF_ISO_IDX];
    INDEX_T const* m_pHdrCapPass1SF      [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pHdrCapPass1MF1     [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pHdrCapPass1MF2     [eNUM_OF_ISO_IDX];
    INDEX_T const* m_pHdrCapPass2        [eNUM_OF_ISO_IDX];

private:
    template <ESensorRole_T eSensorRole> MVOID linkIndexSet();

private:    ////    Normal
    inline MBOOL isInvalid(MUINT32 const scene, MUINT32 const iso) const
    {
        return  ( scene >= eNUM_OF_SCENE_IDX || iso >= eNUM_OF_ISO_IDX );
    }
    inline INDEX_T const* get_OnlinePreview(MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(scene, iso) ? NULL : m_pOnlinePreview[scene][iso];
    }
    inline INDEX_T const* get_OnlineCapture(MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(scene, iso) ? NULL : m_pOnlineCapture[scene][iso];
    }
    inline INDEX_T const* get_OfflineCapturePass1(MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(scene, iso) ? NULL : m_pOfflineCapturePass1[scene][iso];
    }
    inline INDEX_T const* get_OfflineCapturePass2(MUINT32 const scene, MUINT32 const iso) const
    {
        return  isInvalid(scene, iso) ? NULL : m_pOfflineCapturePass2[scene][iso];
    }

private:    ////    HDR
    inline INDEX_T const* get_HdrCapPass1SF(MUINT32 const scene, MUINT32 const iso) const
    {   //  Scene-Indep.
        return  isInvalid(0, iso) ? NULL : m_pHdrCapPass1SF[iso];
    }
    inline INDEX_T const* get_HdrCapPass1MF1(MUINT32 const scene, MUINT32 const iso) const
    {   //  Scene-Indep.
        return  isInvalid(0, iso) ? NULL : m_pHdrCapPass1MF1[iso];
    }
    inline INDEX_T const* get_HdrCapPass1MF2(MUINT32 const scene, MUINT32 const iso) const
    {   //  Scene-Indep.
        return  isInvalid(0, iso) ? NULL : m_pHdrCapPass1MF2[iso];
    }
    inline INDEX_T const* get_HdrCapPass2(MUINT32 const scene, MUINT32 const iso) const
    {   //  Scene-Indep.
        return  isInvalid(0, iso) ? NULL : m_pHdrCapPass2[iso];
    }

public:
    virtual
    INDEX_T const*
    get(MUINT32 const mode, MUINT32 const i/*=-1*/, MUINT32 const j/*=-1*/) const;

};  //  class IdxSetMgr_MT6575


/*******************************************************************************
* 
*******************************************************************************/
#define IDX_SET(dm, dp, nr1, nr2, sat, contrast, hue, gamma, ee)\
    {\
        dm, dp, nr1, nr2, sat, contrast, hue, gamma, ee\
    }


/*******************************************************************************
* Normal
*******************************************************************************/
#define IDX_MODE_Online_Preview(scene, iso)\
    template <> INDEX_T const IdxSet<Define_ESensorRole, ECamMode_Online_Preview, scene, iso>::idx = 

#define IDX_MODE_Online_Capture(scene, iso)\
    template <> INDEX_T const IdxSet<Define_ESensorRole, ECamMode_Online_Capture, scene, iso>::idx = 

#define IDX_MODE_Offline_Capture_Pass1(scene, iso)\
    template <> INDEX_T const IdxSet<Define_ESensorRole, ECamMode_Offline_Capture_Pass1, scene, iso>::idx = 

#define IDX_MODE_Offline_Capture_Pass2(scene, iso)\
    template <> INDEX_T const IdxSet<Define_ESensorRole, ECamMode_Offline_Capture_Pass2, scene, iso>::idx = 

/*******************************************************************************
* HDR
*******************************************************************************/
#define IDX_MODE_HDR_Cap_Pass1_SF(iso)\
    template <> INDEX_T const IdxSet<Define_ESensorRole, ECamMode_HDR_Cap_Pass1_SF, iso>::idx = 

#define IDX_MODE_HDR_Cap_Pass1_MF1(iso)\
    template <> INDEX_T const IdxSet<Define_ESensorRole, ECamMode_HDR_Cap_Pass1_MF1, iso>::idx = 

#define IDX_MODE_HDR_Cap_Pass2(iso)\
    template <> INDEX_T const IdxSet<Define_ESensorRole, ECamMode_HDR_Cap_Pass2, iso>::idx = 

/*******************************************************************************
* 
*******************************************************************************/


};  //  NSIspTuning
#endif //  _ISP_TUNING_IDX_MT6575_H_

