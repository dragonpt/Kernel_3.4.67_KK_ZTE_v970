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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _AF_PARAM_MT6573_H
#define _AF_PARAM_MT6573_H

#define AF_WIN_NUM 9
#define AF_TH_NUM  5

#define AF_WIN_NUM_SPOT     1
#define AF_WIN_NUM_MATRIX   5

#define AF_WIN_NUM_FD       1

#define PATH_LENGTH         100

// --- 6573 AF Config ---
typedef struct
{
	struct
	{
    	MUINT8 uLeft;
	    MUINT8 uRight;
    	MUINT8 uUp;
	    MUINT8 uBottom;

	} sWin[AF_WIN_NUM];

} AF_WIN_T;

typedef struct
{
    MBOOL     bEn;
    MBOOL     bSel;
    AF_WIN_T sAFWin;
    MUINT8    uTH[AF_TH_NUM];

} AF_STAT_CONFIG_T;

// --- 6573 AF Statistic ---

typedef struct
{
	struct
	{
		MUINT32 u4FV[AF_TH_NUM];

	} sWin[AF_WIN_NUM];

	MUINT32 u4Mean[AF_WIN_NUM];

} AF_STAT_T;

typedef struct
{
	MINT32 i4CurrentPos;				//current position
	MINT32 i4MacroPos;				//macro position
	MINT32 i4InfPos;					//Infiniti position
	MBOOL  bIsMotorMoving;			//Motor Status
	MBOOL  bIsMotorOpen;				//Motor Open?

} FOCUS_INFO_T;

typedef struct
{
	MINT32       i4SceneLV;
	AF_STAT_T   sAFStat;
	FOCUS_INFO_T sFocusInfo;
	FD_INFO_T   sFDInfo;
    EZOOM_WIN_T sEZoom;

} AF_INPUT_T;

typedef struct
{
	MINT32 i4FocusPos;
	MBOOL  bUpdateAFStatConfig;
	AF_STAT_CONFIG_T sAFStatConfig;

} AF_OUTPUT_T;

// --- AF algorithm parameter ---

typedef struct
{
    MINT32  i4Num;
    MINT32  i4Pos[PATH_LENGTH];

} AF_STEP_T;

typedef struct
{
    MINT32  i4MATRIX_AF_DOF1;

} AF_PARAM_T;

typedef enum
{
	AF_MARK_NORMAL = 0,
	AF_MARK_OK,
	AF_MARK_FAIL,
	AF_MARK_NONE

} AF_MARK_T;

typedef struct
{
    MINT32 i4Count;
    MINT32 i4Width;
    MINT32 i4Height;

    struct
    {
        AF_MARK_T eMarkStatus;
        MINT32 i4Left;
        MINT32 i4Right;
        MINT32 i4Up;
        MINT32 i4Bottom;

    } sRect[AF_WIN_NUM_MATRIX];

} AF_WIN_RESULT_T;


// --- AF debug info ---
#define AF_DEBUG_TAG_SIZE 1030

typedef struct
{
    AAA_DEBUG_TAG_T Tag[AF_DEBUG_TAG_SIZE];

} AF_DEBUG_INFO_T;

typedef enum
{
    IDX = 0,	// search idx
    POS,		// lens position
    VLU,		// focus value
    MINL,		// idx for min FV in inf side
    MAX,		// idx for max FV
    MINR,		// idx for min FV in macro side

    FIN_3P, 	// finish: Peak found from 3 points
    FIN_ICL,	// finish: Incline case
    FIN_BND,	// finish: Full search case

    BEST_POS,   // lens target position from curve fitting
    FOCUS_POS,  // focused lens position

    LV,         // light value
    FAIL,       // can not find peak
    THRES,      // main threshold value
    STATE,      // AF state
    AFMODE,     // 1: AFS, 2: AFC, 3: Macro, 4: Inf, 5: MF, 6: Cal, 7: Fullscan
    AFMETER,    // 1: Spot, 2: Matrix
    AFLOCAT,    // AF window location
    AFTIME,     // AF process time

    FD_STATUS,  // 0: no face, 1: face detected

    SCAN_START, // fullscan start position
    SCAN_STEP,  // fullscan step interval
    SCAN_NUM,
    
    BOUND,
    PERCENT,
    FOCUSED_IDX,
    DOF,
    
    OVER_PATH_LENGTH

} AF_DEBUG_TAG_T;


#endif

