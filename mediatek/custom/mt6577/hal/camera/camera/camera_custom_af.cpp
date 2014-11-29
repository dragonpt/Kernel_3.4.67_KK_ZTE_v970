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

#include "camera_custom_af.h"

AF_COEF_T get_AF_Coef()
{
    AF_COEF_T sAFcoef;

#ifdef MTK_ZSD_AF_ENHANCE

    MUINT32 RES = 16;           //  5M:16, 8M:26
    MUINT32 NOISE_LEVEL = 16;   //  Normal:16, High noise level: > 16, low noise level: <16,    NOISE_LEVEL=[8, 24]

    sAFcoef.i4AFS_STEP_MIN_ENABLE = 1;
    sAFcoef.i4AFS_STEP_MIN_MACRO  = 4;
    sAFcoef.i4AFS_STEP_MIN_NORMAL = 3;

    sAFcoef.i4FRAME_TIME = 33;
    sAFcoef.i4ZSD_FRAME_TIME = 66;

    sAFcoef.i4AFS_MAX_STEP          = 60;    // DAC, max lens step with no damping
    sAFcoef.i4ZSD_AFS_MAX_STEP      = 120;   // DAC, max lens step with no damping

    sAFcoef.i4AFS_FAIL_POS = 0;
    sAFcoef.i4VAFC_FAIL_POS         = 0;
    sAFcoef.i4VAFC_FAIL_CNT         = 2;

    sAFcoef.i4AFC_THRES_MAIN        = 30;
    sAFcoef.i4AFC_THRES_SUB         = 20;
    
    sAFcoef.i4VAFC_THRES_MAIN       = 30;
    sAFcoef.i4VAFC_THRES_SUB        = 20;
    
    sAFcoef.i4ZSD_AFS_THRES_MAIN    = 25;
    sAFcoef.i4ZSD_AFS_THRES_SUB     = 20;
    sAFcoef.i4ZSD_AFS_MONO_THRES    = 40;    
    sAFcoef.i4ZSD_AFS_MONO_OFFSET   = 60000;
    sAFcoef.i4ZSD_AFC_THRES_MAIN    = 25;
    sAFcoef.i4ZSD_AFC_THRES_SUB     = 20;    

    sAFcoef.i4AFS_FRAME_WAIT        = 0;    
    sAFcoef.i4ZSD_AFS_FRAME_WAIT    = 0;        
    sAFcoef.i4VAFC_FRAME_WAIT       = 0;
    
    sAFcoef.i4AFS_INIT_WAIT         = 1;
    sAFcoef.i4ZSD_AFS_INIT_WAIT     = 1;
    
    sAFcoef.i4AFC_DONE_WAIT         = 0;
    sAFcoef.i4ZSD_AFC_DONE_WAIT     = 0;
    sAFcoef.i4VAFC_DONE_WAIT        = 0;    

    sAFcoef.i4CHANGE_CNT_DELTA      = 0;

    // -------------------------------
    sAFcoef.i4FIRST_FV_WAIT         = 10;
    sAFcoef.i4ZSD_FIRST_FV_WAIT     = 5;
    sAFcoef.i4VAFC_FIRST_FV_WAIT   = 10;

    // -------------------------------
    sAFcoef.i4VAFC_GS_CHANGE_THRES       = 40;    
    sAFcoef.i4VAFC_GS_CHANGE_OFFSET      = 10000;    
    sAFcoef.i4VAFC_GS_CHANGE_CNT         = 12;

    sAFcoef.i4VAFC_FV_CHANGE_THRES       = 40;    
    sAFcoef.i4VAFC_FV_CHANGE_OFFSET      = 10000;    
    sAFcoef.i4VAFC_FV_CHANGE_CNT         = 12;

    sAFcoef.i4VAFC_FV_STABLE1_THRES      = 15;     // percentage -> 0 more stable  
    sAFcoef.i4VAFC_FV_STABLE1_OFFSET     = 10000;  // value -> 0 more stable
    sAFcoef.i4VAFC_FV_STABLE1_NUM        = 4;      // max = 9 (more stable), reset = 0
    sAFcoef.i4VAFC_FV_STABLE1_CNT        = 4;      // max = 9                      

    // -------------------------------
    sAFcoef.i4GS_CHANGE_THRES       = 40;    
    sAFcoef.i4GS_CHANGE_OFFSET      = 10000;    
    sAFcoef.i4GS_CHANGE_CNT         = 12;

    sAFcoef.i4FV_CHANGE_OFFSET      = 10000;        
    
    sAFcoef.i4FV_STABLE1_THRES      = 15;     // percentage -> 0 more stable  
    sAFcoef.i4FV_STABLE1_OFFSET     = 10000;  // value -> 0 more stable
    sAFcoef.i4FV_STABLE1_NUM        = 4;      // max = 9 (more stable), reset = 0
    sAFcoef.i4FV_STABLE1_CNT        = 4;      // max = 9                   
    
    sAFcoef.i4FV_STABLE2_THRES      = 12;        
    sAFcoef.i4FV_STABLE2_OFFSET     = 10000;
    sAFcoef.i4FV_STABLE2_NUM        = 6;                        
    sAFcoef.i4FV_STABLE2_CNT        = 6;                        
    
    // -------------------------------    
    sAFcoef.i4ZSD_GS_CHANGE_THRES   = 40;    
    sAFcoef.i4ZSD_GS_CHANGE_OFFSET  = 10000;    
    sAFcoef.i4ZSD_GS_CHANGE_CNT     = 6;

    sAFcoef.i4ZSD_FV_CHANGE_THRES   = 45;
    sAFcoef.i4ZSD_FV_CHANGE_OFFSET  = 10000;    
    sAFcoef.i4ZSD_FV_CHANGE_CNT     = 6;    

    sAFcoef.i4ZSD_FV_STABLE1_THRES  = 15;     // after FV disturb   
    sAFcoef.i4ZSD_FV_STABLE1_OFFSET = 10000;
    sAFcoef.i4ZSD_FV_STABLE1_NUM    = 3;                        
    sAFcoef.i4ZSD_FV_STABLE1_CNT    = 3;                        

    sAFcoef.i4ZSD_FV_STABLE2_THRES  = 12;     // before AFC start   
    sAFcoef.i4ZSD_FV_STABLE2_OFFSET = 10000;
    sAFcoef.i4ZSD_FV_STABLE2_NUM    = 4;                        
    sAFcoef.i4ZSD_FV_STABLE2_CNT    = 4;                        

    // -------------------------------
    sAFcoef.i4VIDEO_AFC_SPEED_ENABLE = 0;
    sAFcoef.i4VIDEO_AFC_NormalNum = 20;
    sAFcoef.i4VIDEO_AFC_TABLE[0] = 0;
    sAFcoef.i4VIDEO_AFC_TABLE[1] = 20;
    sAFcoef.i4VIDEO_AFC_TABLE[2] = 40;
    sAFcoef.i4VIDEO_AFC_TABLE[3] = 60;
    sAFcoef.i4VIDEO_AFC_TABLE[4] = 80;
    sAFcoef.i4VIDEO_AFC_TABLE[5] = 100;
    sAFcoef.i4VIDEO_AFC_TABLE[6] = 120;
    sAFcoef.i4VIDEO_AFC_TABLE[7] = 140;
    sAFcoef.i4VIDEO_AFC_TABLE[8] = 160;
    sAFcoef.i4VIDEO_AFC_TABLE[9] = 180;
    sAFcoef.i4VIDEO_AFC_TABLE[10] = 200;
    sAFcoef.i4VIDEO_AFC_TABLE[11] = 220;
    sAFcoef.i4VIDEO_AFC_TABLE[12] = 240;
    sAFcoef.i4VIDEO_AFC_TABLE[13] = 260;
    sAFcoef.i4VIDEO_AFC_TABLE[14] = 280;
    sAFcoef.i4VIDEO_AFC_TABLE[15] = 300;    
    sAFcoef.i4VIDEO_AFC_TABLE[16] = 320;    
    sAFcoef.i4VIDEO_AFC_TABLE[17] = 340;    
    sAFcoef.i4VIDEO_AFC_TABLE[18] = 360;        
    sAFcoef.i4VIDEO_AFC_TABLE[19] = 390;    // last pos should be the same as last pos in normal or macro search table            

    // Non-ZSD -------------------------------------------------------------------
    sAFcoef.sNormalTH.i4ISONum = 8;
    sAFcoef.sNormalTH.i4ISO[0] = 100;
    sAFcoef.sNormalTH.i4ISO[1] = 150;
    sAFcoef.sNormalTH.i4ISO[2] = 200;
    sAFcoef.sNormalTH.i4ISO[3] = 300;
    sAFcoef.sNormalTH.i4ISO[4] = 400;
    sAFcoef.sNormalTH.i4ISO[5] = 600;
    sAFcoef.sNormalTH.i4ISO[6] = 800;
    sAFcoef.sNormalTH.i4ISO[7] = 1600;    

    sAFcoef.sNormalTH.i4GMeanNum = 6;
    sAFcoef.sNormalTH.i4GMean[0] = 20;
    sAFcoef.sNormalTH.i4GMean[1] = 55;
    sAFcoef.sNormalTH.i4GMean[2] = 105;
    sAFcoef.sNormalTH.i4GMean[3] = 150;
    sAFcoef.sNormalTH.i4GMean[4] = 180;
    sAFcoef.sNormalTH.i4GMean[5] = 205;

    for (MINT32 i=0; i<sAFcoef.sNormalTH.i4GMeanNum; i++)
    {
        for (MINT32 j=0; j<sAFcoef.sNormalTH.i4ISONum; j++)
        {
            sAFcoef.sNormalTH.i4FV_DC[i][j] = 0;
            sAFcoef.sNormalTH.i4MIN_TH[i][j] = 50000;
            sAFcoef.sNormalTH.i4HW_TH[i][j] = 2;            
        }        
    }  

//ZSD -------------------------------------------------------------------
    sAFcoef.sZSDTH.i4ISONum = 8;
    sAFcoef.sZSDTH.i4ISO[0] = 100;
    sAFcoef.sZSDTH.i4ISO[1] = 150;
    sAFcoef.sZSDTH.i4ISO[2] = 200;
    sAFcoef.sZSDTH.i4ISO[3] = 300;
    sAFcoef.sZSDTH.i4ISO[4] = 400;
    sAFcoef.sZSDTH.i4ISO[5] = 600;
    sAFcoef.sZSDTH.i4ISO[6] = 800;
    sAFcoef.sZSDTH.i4ISO[7] = 1600;    

    sAFcoef.sZSDTH.i4GMeanNum = 6;
    sAFcoef.sZSDTH.i4GMean[0] = 20;
    sAFcoef.sZSDTH.i4GMean[1] = 55;
    sAFcoef.sZSDTH.i4GMean[2] = 105;
    sAFcoef.sZSDTH.i4GMean[3] = 150;
    sAFcoef.sZSDTH.i4GMean[4] = 180;
    sAFcoef.sZSDTH.i4GMean[5] = 205;

    // GMean = 20
    sAFcoef.sZSDTH.i4FV_DC[0][0] = (27783*RES)>>4;     // ISO100  = Xsize * Ysize * 0.001 * HW_TH ^2
    sAFcoef.sZSDTH.i4FV_DC[0][1] = (49686*RES)>>4;     // ISO150    
    sAFcoef.sZSDTH.i4FV_DC[0][2] = (38416*RES)>>4;     // ISO200
    sAFcoef.sZSDTH.i4FV_DC[0][3] = (37632*RES)>>4;     // ISO300
    sAFcoef.sZSDTH.i4FV_DC[0][4] = (56644*RES)>>4;     // ISO400
    sAFcoef.sZSDTH.i4FV_DC[0][5] = (178605*RES)>>4;    // ISO600
    sAFcoef.sZSDTH.i4FV_DC[0][6] = (94864*RES)>>4;     // ISO800
    sAFcoef.sZSDTH.i4FV_DC[0][7] = (91875*RES)>>4;     // ISO1600

    // GMean = 55
    sAFcoef.sZSDTH.i4FV_DC[1][0] = (41405*RES)>>4;      // ISO100  = Xsize * Ysize * 0.001 * HW_TH ^2
    sAFcoef.sZSDTH.i4FV_DC[1][1] = (56644*RES)>>4;      // ISO150    
    sAFcoef.sZSDTH.i4FV_DC[1][2] = (98000*RES)>>4;      // ISO200
    sAFcoef.sZSDTH.i4FV_DC[1][3] = (142296*RES)>>4;     // ISO300
    sAFcoef.sZSDTH.i4FV_DC[1][4] = (142296*RES)>>4;     // ISO400
    sAFcoef.sZSDTH.i4FV_DC[1][5] = (226576*RES)>>4;     // ISO600
    sAFcoef.sZSDTH.i4FV_DC[1][6] = (200704*RES)>>4;     // ISO800
    sAFcoef.sZSDTH.i4FV_DC[1][7] = (345744*RES)>>4;     // ISO1600

    // GMean = 105
    sAFcoef.sZSDTH.i4FV_DC[2][0] = (42336*RES)>>4;      // ISO100  = Xsize * Ysize * 0.001 * HW_TH ^2
    sAFcoef.sZSDTH.i4FV_DC[2][1] = (57967*RES)>>4;      // ISO150    
    sAFcoef.sZSDTH.i4FV_DC[2][2] = (66150*RES)>>4;      // ISO200
    sAFcoef.sZSDTH.i4FV_DC[2][3] = (79380*RES)>>4;      // ISO300
    sAFcoef.sZSDTH.i4FV_DC[2][4] = (70756*RES)>>4;      // ISO400
    sAFcoef.sZSDTH.i4FV_DC[2][5] = (206045*RES)>>4;     // ISO600
    sAFcoef.sZSDTH.i4FV_DC[2][6] = (188356*RES)>>4;     // ISO800
    sAFcoef.sZSDTH.i4FV_DC[2][7] = (345744*RES)>>4;     // ISO1600

    // GMean = 150
    sAFcoef.sZSDTH.i4FV_DC[3][0] = (11907*RES)>>4;      // ISO100  = Xsize * Ysize * 0.001 * HW_TH ^2
    sAFcoef.sZSDTH.i4FV_DC[3][1] = (17787*RES)>>4;      // ISO150    
    sAFcoef.sZSDTH.i4FV_DC[3][2] = (24500*RES)>>4;      // ISO200
    sAFcoef.sZSDTH.i4FV_DC[3][3] = (48020*RES)>>4;      // ISO300
    sAFcoef.sZSDTH.i4FV_DC[3][4] = (44100*RES)>>4;      // ISO400
    sAFcoef.sZSDTH.i4FV_DC[3][5] = (70805*RES)>>4;      // ISO600
    sAFcoef.sZSDTH.i4FV_DC[3][6] = (88445*RES)>>4;      // ISO800
    sAFcoef.sZSDTH.i4FV_DC[3][7] = (132496*RES)>>4;     // ISO1600

    // GMean = 180
    sAFcoef.sZSDTH.i4FV_DC[4][0] = (12005*RES)>>4;      // ISO100  = Xsize * Ysize * 0.001 * HW_TH ^2
    sAFcoef.sZSDTH.i4FV_DC[4][1] = (19208*RES)>>4;      // ISO150    
    sAFcoef.sZSDTH.i4FV_DC[4][2] = (18816*RES)>>4;      // ISO200
    sAFcoef.sZSDTH.i4FV_DC[4][3] = (29400*RES)>>4;      // ISO300
    sAFcoef.sZSDTH.i4FV_DC[4][4] = (29645*RES)>>4;      // ISO400
    sAFcoef.sZSDTH.i4FV_DC[4][5] = (21168*RES)>>4;      // ISO600
    sAFcoef.sZSDTH.i4FV_DC[4][6] = (41405*RES)>>4;      // ISO800
    sAFcoef.sZSDTH.i4FV_DC[4][7] = (38416*RES)>>4;      // ISO1600

    // GMean = 205
    sAFcoef.sZSDTH.i4FV_DC[5][0] = (6125*RES)>>4;       // ISO100  = Xsize * Ysize * 0.001 * HW_TH ^2
    sAFcoef.sZSDTH.i4FV_DC[5][1] = (7056*RES)>>4;       // ISO150    
    sAFcoef.sZSDTH.i4FV_DC[5][2] = (12348*RES)>>4;      // ISO200
    sAFcoef.sZSDTH.i4FV_DC[5][3] = (12544*RES)>>4;      // ISO300
    sAFcoef.sZSDTH.i4FV_DC[5][4] = (12544*RES)>>4;      // ISO400
    sAFcoef.sZSDTH.i4FV_DC[5][5] = (19600*RES)>>4;      // ISO600
    sAFcoef.sZSDTH.i4FV_DC[5][6] = (29400*RES)>>4;      // ISO800
    sAFcoef.sZSDTH.i4FV_DC[5][7] = (19600*RES)>>4;      // ISO1600

    for (MINT32 i=0; i<sAFcoef.sZSDTH.i4GMeanNum; i++)
    {
        for (MINT32 j=0; j<sAFcoef.sZSDTH.i4ISONum; j++)
        {
            sAFcoef.sZSDTH.i4MIN_TH[i][j] = 30000;
        }        
    }  

    // GMean = 20
    sAFcoef.sZSDTH.i4HW_TH[0][0] = (9*NOISE_LEVEL)>>4;      // ISO100
    sAFcoef.sZSDTH.i4HW_TH[0][1] = (13*NOISE_LEVEL)>>4;     // ISO150   
    sAFcoef.sZSDTH.i4HW_TH[0][2] = (14*NOISE_LEVEL)>>4;     // ISO200
    sAFcoef.sZSDTH.i4HW_TH[0][3] = (16*NOISE_LEVEL)>>4;     // ISO300
    sAFcoef.sZSDTH.i4HW_TH[0][4] = (17*NOISE_LEVEL)>>4;     // ISO400
    sAFcoef.sZSDTH.i4HW_TH[0][5] = (22*NOISE_LEVEL)>>4;     // ISO600
    sAFcoef.sZSDTH.i4HW_TH[0][6] = (22*NOISE_LEVEL)>>4;     // ISO800
    sAFcoef.sZSDTH.i4HW_TH[0][7] = (25*NOISE_LEVEL)>>4;     // ISO1600

    // GMean = 55
    sAFcoef.sZSDTH.i4HW_TH[1][0] = (13*NOISE_LEVEL)>>4;     // ISO100
    sAFcoef.sZSDTH.i4HW_TH[1][1] = (17*NOISE_LEVEL)>>4;     // ISO150   
    sAFcoef.sZSDTH.i4HW_TH[1][2] = (20*NOISE_LEVEL)>>4;     // ISO200
    sAFcoef.sZSDTH.i4HW_TH[1][3] = (22*NOISE_LEVEL)>>4;     // ISO300
    sAFcoef.sZSDTH.i4HW_TH[1][4] = (22*NOISE_LEVEL)>>4;     // ISO400
    sAFcoef.sZSDTH.i4HW_TH[1][5] = (30*NOISE_LEVEL)>>4;     // ISO600
    sAFcoef.sZSDTH.i4HW_TH[1][6] = (30*NOISE_LEVEL)>>4;     // ISO800
    sAFcoef.sZSDTH.i4HW_TH[1][7] = (42*NOISE_LEVEL)>>4;     // ISO1600

    // GMean = 105
    sAFcoef.sZSDTH.i4HW_TH[2][0] = (12*NOISE_LEVEL)>>4;     // ISO100
    sAFcoef.sZSDTH.i4HW_TH[2][1] = (13*NOISE_LEVEL)>>4;     // ISO150   
    sAFcoef.sZSDTH.i4HW_TH[2][2] = (15*NOISE_LEVEL)>>4;     // ISO200
    sAFcoef.sZSDTH.i4HW_TH[2][3] = (16*NOISE_LEVEL)>>4;     // ISO300
    sAFcoef.sZSDTH.i4HW_TH[2][4] = (17*NOISE_LEVEL)>>4;     // ISO400
    sAFcoef.sZSDTH.i4HW_TH[2][5] = (24*NOISE_LEVEL)>>4;     // ISO600
    sAFcoef.sZSDTH.i4HW_TH[2][6] = (26*NOISE_LEVEL)>>4;     // ISO800
    sAFcoef.sZSDTH.i4HW_TH[2][7] = (40*NOISE_LEVEL)>>4;     // ISO1600

    // GMean = 150
    sAFcoef.sZSDTH.i4HW_TH[3][0] = (11*NOISE_LEVEL)>>4;      // ISO100
    sAFcoef.sZSDTH.i4HW_TH[3][1] = (11*NOISE_LEVEL)>>4;     // ISO150   
    sAFcoef.sZSDTH.i4HW_TH[3][2] = (12*NOISE_LEVEL)>>4;     // ISO200
    sAFcoef.sZSDTH.i4HW_TH[3][3] = (12*NOISE_LEVEL)>>4;     // ISO300
    sAFcoef.sZSDTH.i4HW_TH[3][4] = (13*NOISE_LEVEL)>>4;     // ISO400
    sAFcoef.sZSDTH.i4HW_TH[3][5] = (14*NOISE_LEVEL)>>4;     // ISO600
    sAFcoef.sZSDTH.i4HW_TH[3][6] = (17*NOISE_LEVEL)>>4;     // ISO800
    sAFcoef.sZSDTH.i4HW_TH[3][7] = (24*NOISE_LEVEL)>>4;     // ISO1600

    // GMean = 180
    sAFcoef.sZSDTH.i4HW_TH[4][0] = (11*NOISE_LEVEL)>>4;      // ISO100
    sAFcoef.sZSDTH.i4HW_TH[4][1] = (11*NOISE_LEVEL)>>4;      // ISO150   
    sAFcoef.sZSDTH.i4HW_TH[4][2] = (11*NOISE_LEVEL)>>4;      // ISO200
    sAFcoef.sZSDTH.i4HW_TH[4][3] = (11*NOISE_LEVEL)>>4;      // ISO300
    sAFcoef.sZSDTH.i4HW_TH[4][4] = (11*NOISE_LEVEL)>>4;     // ISO400
    sAFcoef.sZSDTH.i4HW_TH[4][5] = (11*NOISE_LEVEL)>>4;     // ISO600
    sAFcoef.sZSDTH.i4HW_TH[4][6] = (12*NOISE_LEVEL)>>4;     // ISO800
    sAFcoef.sZSDTH.i4HW_TH[4][7] = (13*NOISE_LEVEL)>>4;     // ISO1600

    // GMean = 205
    sAFcoef.sZSDTH.i4HW_TH[5][0] = (5*NOISE_LEVEL)>>4;      // ISO100
    sAFcoef.sZSDTH.i4HW_TH[5][1] = (6*NOISE_LEVEL)>>4;      // ISO150   
    sAFcoef.sZSDTH.i4HW_TH[5][2] = (6*NOISE_LEVEL)>>4;      // ISO200
    sAFcoef.sZSDTH.i4HW_TH[5][3] = (8*NOISE_LEVEL)>>4;      // ISO300
    sAFcoef.sZSDTH.i4HW_TH[5][4] = (8*NOISE_LEVEL)>>4;      // ISO400
    sAFcoef.sZSDTH.i4HW_TH[5][5] = (10*NOISE_LEVEL)>>4;     // ISO600
    sAFcoef.sZSDTH.i4HW_TH[5][6] = (10*NOISE_LEVEL)>>4;     // ISO800
    sAFcoef.sZSDTH.i4HW_TH[5][7] = (10*NOISE_LEVEL)>>4;     // ISO1600

#else

    sAFcoef.i4AFS_STEP_MIN_ENABLE = 1;
    sAFcoef.i4AFS_STEP_MIN_MACRO  = 4;
    sAFcoef.i4AFS_STEP_MIN_NORMAL = 4;

    sAFcoef.i4AFC_THRES_MAIN        = 30;
    sAFcoef.i4AFC_THRES_SUB         = 20;
    
    sAFcoef.i4ZSD_HW_TH             = 12;
    sAFcoef.i4ZSD_AFS_THRES_MAIN    = 30;
    sAFcoef.i4ZSD_AFS_THRES_SUB     = 20;
    sAFcoef.i4ZSD_AFS_THRES_OFFSET  = 10000;
    sAFcoef.i4ZSD_AFC_THRES_MAIN    = 30;
    sAFcoef.i4ZSD_AFC_THRES_SUB     = 20;    
    sAFcoef.i4ZSD_AFC_THRES_OFFSET  = 10000;

    sAFcoef.i4ZSD_AFS_MONO_THRES    = 60;    
    sAFcoef.i4ZSD_AFS_MONO_OFFSET   = 100000;

    sAFcoef.i4AFS_WAIT              = 1;
    sAFcoef.i4AFC_WAIT              = 10;

    sAFcoef.i4FV_CHANGE_OFFSET      = 1000;        
    sAFcoef.i4FV_STABLE_THRES       = 30;        
    sAFcoef.i4FV_STABLE_OFFSET      = 0;
    sAFcoef.i4FV_STABLE_CNT         = 5;                        

    sAFcoef.i4VIDEO_AFC_SPEED_ENABLE = 0;
    sAFcoef.i4VIDEO_AFC_NormalNum = 10;
    sAFcoef.i4VIDEO_AFC_TABLE[0] = 0;
    sAFcoef.i4VIDEO_AFC_TABLE[1] = 25;
    sAFcoef.i4VIDEO_AFC_TABLE[2] = 55;
    sAFcoef.i4VIDEO_AFC_TABLE[3] = 80;
    sAFcoef.i4VIDEO_AFC_TABLE[4] = 110;
    sAFcoef.i4VIDEO_AFC_TABLE[5] = 160;
    sAFcoef.i4VIDEO_AFC_TABLE[6] = 215;
    sAFcoef.i4VIDEO_AFC_TABLE[7] = 270;
    sAFcoef.i4VIDEO_AFC_TABLE[8] = 325;
    sAFcoef.i4VIDEO_AFC_TABLE[9] = 390;
    sAFcoef.i4VIDEO_AFC_TABLE[10] = 455;
    sAFcoef.i4VIDEO_AFC_TABLE[11] = 520;
    sAFcoef.i4VIDEO_AFC_TABLE[12] = 585;
    sAFcoef.i4VIDEO_AFC_TABLE[13] = 650;
    sAFcoef.i4VIDEO_AFC_TABLE[14] = 715;
#endif
    
	return sAFcoef;
}

