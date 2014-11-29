/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _AF_COEF_H
#define _AF_COEF_H

#include "MediaTypes.h"

#ifdef MTK_ZSD_AF_ENHANCE

#define ISO_MAX_NUM 8
#define GMEAN_MAX_NUM 6

    typedef struct
    {
        MINT32 i4ISONum;
        MINT32 i4ISO[ISO_MAX_NUM];

        MINT32 i4GMeanNum;
        MINT32 i4GMean[GMEAN_MAX_NUM];
        
        MINT32 i4FV_DC[GMEAN_MAX_NUM][ISO_MAX_NUM];
        MINT32 i4MIN_TH[GMEAN_MAX_NUM][ISO_MAX_NUM];        
        MINT32 i4HW_TH[GMEAN_MAX_NUM][ISO_MAX_NUM];       

    }AF_THRES;

    typedef struct
    {
        MINT32 i4AFS_STEP_MIN_ENABLE;
        MINT32 i4AFS_STEP_MIN_NORMAL;
        MINT32 i4AFS_STEP_MIN_MACRO;

        MINT32 i4FRAME_TIME;
        MINT32 i4ZSD_FRAME_TIME;

        MINT32 i4AFS_MAX_STEP;
        MINT32 i4ZSD_AFS_MAX_STEP;

        MINT32 i4AFS_FAIL_POS;
        MINT32 i4VAFC_FAIL_POS;
        MINT32 i4VAFC_FAIL_CNT;

        MINT32 i4AFC_THRES_MAIN;
        MINT32 i4AFC_THRES_SUB;
        MINT32 i4VAFC_THRES_MAIN;
        MINT32 i4VAFC_THRES_SUB;        
        AF_THRES sNormalTH;
        
        MINT32 i4ZSD_AFS_THRES_MAIN;
        MINT32 i4ZSD_AFS_THRES_SUB;
        MINT32 i4ZSD_AFS_MONO_THRES;    
        MINT32 i4ZSD_AFS_MONO_OFFSET;
        MINT32 i4ZSD_AFC_THRES_MAIN;
        MINT32 i4ZSD_AFC_THRES_SUB;    
        AF_THRES sZSDTH;

        MINT32 i4AFS_FRAME_WAIT;
        MINT32 i4ZSD_AFS_FRAME_WAIT;
        MINT32 i4VAFC_FRAME_WAIT;
        
        MINT32 i4AFS_INIT_WAIT;
        MINT32 i4ZSD_AFS_INIT_WAIT;
        
        MINT32 i4AFC_DONE_WAIT;
        MINT32 i4ZSD_AFC_DONE_WAIT;
        MINT32 i4VAFC_DONE_WAIT;

        MINT32 i4CHANGE_CNT_DELTA;

        // -------------------------------
        MINT32 i4FIRST_FV_WAIT;
        MINT32 i4ZSD_FIRST_FV_WAIT;
        MINT32 i4VAFC_FIRST_FV_WAIT;

        // -------------------------------
        MINT32 i4VAFC_GS_CHANGE_THRES;    
        MINT32 i4VAFC_GS_CHANGE_OFFSET;    
        MINT32 i4VAFC_GS_CHANGE_CNT;
        
        MINT32 i4VAFC_FV_CHANGE_THRES;    
        MINT32 i4VAFC_FV_CHANGE_OFFSET;    
        MINT32 i4VAFC_FV_CHANGE_CNT;
        
        MINT32 i4VAFC_FV_STABLE1_THRES;     // percentage -> 0 more stable  
        MINT32 i4VAFC_FV_STABLE1_OFFSET;    // value -> 0 more stable
        MINT32 i4VAFC_FV_STABLE1_NUM;      // max = 9 (more stable), reset = 0
        MINT32 i4VAFC_FV_STABLE1_CNT;      // max = 9                      

        // -------------------------------
        MINT32 i4GS_CHANGE_THRES;    
        MINT32 i4GS_CHANGE_OFFSET;    
        MINT32 i4GS_CHANGE_CNT;    
        
        MINT32 i4FV_CHANGE_OFFSET;        
        
        MINT32 i4FV_STABLE1_THRES;        
        MINT32 i4FV_STABLE1_OFFSET;
        MINT32 i4FV_STABLE1_NUM;                        
        MINT32 i4FV_STABLE1_CNT;                        

        MINT32 i4FV_STABLE2_THRES;        
        MINT32 i4FV_STABLE2_OFFSET;
        MINT32 i4FV_STABLE2_NUM;                        
        MINT32 i4FV_STABLE2_CNT;                        

        // -------------------------------
        MINT32 i4ZSD_GS_CHANGE_THRES;    
        MINT32 i4ZSD_GS_CHANGE_OFFSET;    
        MINT32 i4ZSD_GS_CHANGE_CNT;    

        MINT32 i4ZSD_FV_CHANGE_THRES;    
        MINT32 i4ZSD_FV_CHANGE_OFFSET;    
        MINT32 i4ZSD_FV_CHANGE_CNT;    
        
        MINT32 i4ZSD_FV_STABLE1_THRES;        
        MINT32 i4ZSD_FV_STABLE1_OFFSET;
        MINT32 i4ZSD_FV_STABLE1_NUM;                        
        MINT32 i4ZSD_FV_STABLE1_CNT;                        

        MINT32 i4ZSD_FV_STABLE2_THRES;        
        MINT32 i4ZSD_FV_STABLE2_OFFSET;
        MINT32 i4ZSD_FV_STABLE2_NUM;                        
        MINT32 i4ZSD_FV_STABLE2_CNT;                        
        // -------------------------------

        MINT32 i4VIDEO_AFC_SPEED_ENABLE;
        MINT32 i4VIDEO_AFC_NormalNum;
        MINT32 i4VIDEO_AFC_TABLE[30];
        
    } AF_COEF_T;

#else

    typedef struct
    {
        MINT32 i4AFS_STEP_MIN_ENABLE;
        MINT32 i4AFS_STEP_MIN_NORMAL;
        MINT32 i4AFS_STEP_MIN_MACRO;

        MINT32 i4AFC_THRES_MAIN;
        MINT32 i4AFC_THRES_SUB;
        
        MINT32 i4ZSD_HW_TH;
        MINT32 i4ZSD_AFS_THRES_MAIN;
        MINT32 i4ZSD_AFS_THRES_SUB;
        MINT32 i4ZSD_AFS_THRES_OFFSET;
        MINT32 i4ZSD_AFC_THRES_MAIN;
        MINT32 i4ZSD_AFC_THRES_SUB;    
        MINT32 i4ZSD_AFC_THRES_OFFSET;
        
        MINT32 i4ZSD_AFS_MONO_THRES;    
        MINT32 i4ZSD_AFS_MONO_OFFSET;

        MINT32 i4AFS_WAIT;
        MINT32 i4AFC_WAIT;
        
        MINT32 i4FV_CHANGE_OFFSET;        
        MINT32 i4FV_STABLE_THRES;        
        MINT32 i4FV_STABLE_OFFSET;
        MINT32 i4FV_STABLE_CNT;                        

        MINT32 i4VIDEO_AFC_SPEED_ENABLE;
        MINT32 i4VIDEO_AFC_NormalNum;
        MINT32 i4VIDEO_AFC_TABLE[30];
        
    } AF_COEF_T;
#endif

	AF_COEF_T get_AF_Coef();
	
#endif /* _AF_COEF_H */

