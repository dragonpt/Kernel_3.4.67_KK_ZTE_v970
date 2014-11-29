/*******************************************************************************************/


/*******************************************************************************************/

/* SENSOR FULL SIZE */
#ifndef __SENSOR_H
#define __SENSOR_H

typedef enum group_enum {
    PRE_GAIN=0,
    CMMCLK_CURRENT,
    FRAME_RATE_LIMITATION,
    REGISTER_EDITOR,
    GROUP_TOTAL_NUMS
} FACTORY_GROUP_ENUM;


#define ENGINEER_START_ADDR 10
#define FACTORY_START_ADDR 0

typedef enum engineer_index
{
    CMMCLK_CURRENT_INDEX=ENGINEER_START_ADDR,
    ENGINEER_END
} FACTORY_ENGINEER_INDEX;

typedef enum register_index
{
	SENSOR_BASEGAIN=FACTORY_START_ADDR,
	PRE_GAIN_R_INDEX,
	PRE_GAIN_Gr_INDEX,
	PRE_GAIN_Gb_INDEX,
	PRE_GAIN_B_INDEX,
	FACTORY_END_ADDR
} FACTORY_REGISTER_INDEX;

typedef struct
{
    SENSOR_REG_STRUCT	Reg[ENGINEER_END];
    SENSOR_REG_STRUCT	CCT[FACTORY_END_ADDR];
} SENSOR_DATA_STRUCT, *PSENSOR_DATA_STRUCT;

typedef enum {
    SENSOR_MODE_INIT = 0,
    SENSOR_MODE_PREVIEW,
    SENSOR_MODE_CAPTURE
} S5K4E1FX_SENSOR_MODE;

typedef struct
{
	kal_uint16 DummyPixels;
	kal_uint16 DummyLines;
	
	kal_uint16 pvPclk;  // x10 480 for 48MHZ
	kal_uint16 capPclk; // x10
	
	kal_uint16 shutter;
	kal_uint16 maxExposureLines;

	kal_uint16 sensorGlobalGain;
	kal_uint16 pvSensorGlobalGain;
	kal_uint16 sensorBaseGain;
	kal_uint16 ispBaseGain;

	kal_int16 imgMirror;

	S5K4E1FX_SENSOR_MODE sensorMode;
	
}S5K4E1FX_PARA_STRUCT,*PS5K4E1FX_PARA_STRUCT;


	#define S5K4E1FX_IMAGE_SENSOR_FULL_WIDTH					(2592-24)	
	#define S5K4E1FX_IMAGE_SENSOR_FULL_HEIGHT					(1944-18)

	/* SENSOR PV SIZE */
	#define S5K4E1FX_IMAGE_SENSOR_PV_WIDTH						(1280)//(1280-4)
	#define S5K4E1FX_IMAGE_SENSOR_PV_HEIGHT						(960)//(960-4)

	/* SENSOR SCALER FACTOR */
	#define S5K4E1FX_PV_SCALER_FACTOR					    	3
	#define S5K4E1FX_FULL_SCALER_FACTOR					    	1
	                                        	
	/* SENSOR START/EDE POSITION */         	
	#define S5K4E1FX_FULL_X_START						    		(2)
	#define S5K4E1FX_FULL_Y_START						    		(2)
	#define S5K4E1FX_FULL_X_END						        		2607 //2601
	#define S5K4E1FX_FULL_Y_END						        		1959 //1953
	#define S5K4E1FX_PV_X_START						    			(2)
	#define S5K4E1FX_PV_Y_START						    			(2)
	#define S5K4E1FX_PV_X_END						    			2607 //2597//2597
	#define S5K4E1FX_PV_Y_END						    			1959 //1953//1949
	
	/* SENSOR HORIZONTAL/VERTICAL BLANKING */	
	//#define S5K4E1FX_IMAGE_SENSOR_FULL_HBLANKING				(30)
	//#define S5K4E1FX_IMAGE_SENSOR_FULL_VBLANKING				(126)			
	//#define S5K4E1FX_IMAGE_SENSOR_PV_HBLANKING					(160)	
	//#define S5K4E1FX_IMAGE_SENSOR_PV_VBLANKING					(130)			

	/* SENSOR PIXEL/LINE NUMBERS IN ONE PERIOD */
	#define S5K4E1FX_PV_ACTIVE_PIXEL_NUMS						(S5K4E1FX_FULL_X_END-S5K4E1FX_FULL_X_START+1-2)//2598
	#define S5K4E1FX_PV_ACTIVE_LINE_NUMS						(S5K4E1FX_FULL_Y_END-S5K4E1FX_FULL_Y_START+1-2)//1950
	#define S5K4E1FX_FULL_ACTIVE_PIXEL_NUMS						(((S5K4E1FX_PV_X_END-S5K4E1FX_PV_X_START+1)/((S5K4E1FX_PV_SCALER_FACTOR+1)/2)))//1300
	#define S5K4E1FX_FULL_ACTIVE_LINE_NUMS						(((S5K4E1FX_PV_Y_END-S5K4E1FX_PV_Y_START+1)/((S5K4E1FX_PV_SCALER_FACTOR+1)/2)))//1952

	#define S5K4E1FX_MAX_ANALOG_GAIN					(16)
	#define S5K4E1FX_MIN_ANALOG_GAIN					(1)
	#define S5K4E1FX_ANALOG_GAIN_1X						(0x0020)

	//#define S5K4E1FX_MAX_DIGITAL_GAIN					(8)
	//#define S5K4E1FX_MIN_DIGITAL_GAIN					(1)
	//#define S5K4E1FX_DIGITAL_GAIN_1X					(0x0100)


	#define S5K4E1FX_FULL_PERIOD_PIXEL_NUMS					0x0AB2  //2738
	#define S5K4E1FX_FULL_PERIOD_LINE_NUMS					0x07B4  //1972
	
	#define S5K4E1FX_PV_PERIOD_PIXEL_NUMS					0x0AB2	//2738
	#define S5K4E1FX_PV_PERIOD_LINE_NUMS					0x03E0	//992

	#define S5K4E1FX_MIN_LINE_LENGTH						0x0AA4  //2724
	#define S5K4E1FX_MIN_FRAME_LENGTH						0x0214  //532
	
	#define S5K4E1FX_MAX_LINE_LENGTH						0xCCCC
	#define S5K4E1FX_MAX_FRAME_LENGTH						0xFFFF

	/* DUMMY NEEDS TO BE INSERTED */
	/* SETUP TIME NEED TO BE INSERTED */
	#define S5K4E1FX_IMAGE_SENSOR_PV_INSERTED_PIXELS			2	// Sync, Nosync=2
	#define S5K4E1FX_IMAGE_SENSOR_PV_INSERTED_LINES			2

	#define S5K4E1FX_IMAGE_SENSOR_FULL_INSERTED_PIXELS		4
	#define S5K4E1FX_IMAGE_SENSOR_FULL_INSERTED_LINES		4
#if 0
// Important Note:
//     1. Make sure horizontal PV sensor output is larger than S5K4E1FXMIPI_REAL_PV_WIDTH  + 2 * S5K4E1FXMIPI_IMAGE_SENSOR_PV_STARTX + 4.
//     2. Make sure vertical   PV sensor output is larger than S5K4E1FXMIPI_REAL_PV_HEIGHT + 2 * S5K4E1FXMIPI_IMAGE_SENSOR_PV_STARTY + 6.
//     3. Make sure horizontal CAP sensor output is larger than S5K4E1FXMIPI_REAL_CAP_WIDTH  + 2 * S5K4E1FXMIPI_IMAGE_SENSOR_CAP_STARTX + IMAGE_SENSOR_H_DECIMATION*4.
//     4. Make sure vertical   CAP sensor output is larger than S5K4E1FXMIPI_REAL_CAP_HEIGHT + 2 * S5K4E1FXMIPI_IMAGE_SENSOR_CAP_STARTY + IMAGE_SENSOR_V_DECIMATION*6.
// Note:
//     1. The reason why we choose REAL_PV_WIDTH/HEIGHT as tuning starting point is
//        that if we choose REAL_CAP_WIDTH/HEIGHT as starting point, then:
//            REAL_PV_WIDTH  = REAL_CAP_WIDTH  / IMAGE_SENSOR_H_DECIMATION
//            REAL_PV_HEIGHT = REAL_CAP_HEIGHT / IMAGE_SENSOR_V_DECIMATION
//        There might be some truncation error when dividing, which may cause a little view angle difference.
//Macro for Resolution
#define IMAGE_SENSOR_H_DECIMATION				2	// For current PV mode, take 1 line for every 2 lines in horizontal direction.
#define IMAGE_SENSOR_V_DECIMATION				2	// For current PV mode, take 1 line for every 2 lines in vertical direction.

/* Real PV Size, i.e. the size after all ISP processing (so already -4/-6), before MDP. */
#define S5K4E1FXMIPI_REAL_PV_WIDTH				(1280)
#define S5K4E1FXMIPI_REAL_PV_HEIGHT				(960)

/* Real CAP Size, i.e. the size after all ISP processing (so already -4/-6), before MDP. */
#define S5K4E1FXMIPI_REAL_CAP_WIDTH				(S5K4E1FXMIPI_REAL_PV_WIDTH  * IMAGE_SENSOR_H_DECIMATION)
#define S5K4E1FXMIPI_REAL_CAP_HEIGHT			(S5K4E1FXMIPI_REAL_PV_HEIGHT * IMAGE_SENSOR_V_DECIMATION)

/* X/Y Starting point */
#define S5K4E1FXMIPI_IMAGE_SENSOR_PV_STARTX       2
#define S5K4E1FXMIPI_IMAGE_SENSOR_PV_STARTY       2	// The value must bigger or equal than 1.
#define S5K4E1FXMIPI_IMAGE_SENSOR_CAP_STARTX		(S5K4E1FXMIPI_IMAGE_SENSOR_PV_STARTX * IMAGE_SENSOR_H_DECIMATION)
#define S5K4E1FXMIPI_IMAGE_SENSOR_CAP_STARTY		(S5K4E1FXMIPI_IMAGE_SENSOR_PV_STARTY * IMAGE_SENSOR_V_DECIMATION)		// The value must bigger or equal than 1.

/* SENSOR 720P SIZE */
#define S5K4E1FXMIPI_IMAGE_SENSOR_PV_WIDTH		(S5K4E1FXMIPI_REAL_PV_WIDTH  + 2 * S5K4E1FXMIPI_IMAGE_SENSOR_PV_STARTX)	// 2*: Leave PV_STARTX unused space at both left side and right side of REAL_PV_WIDTH.	//(1620) //(820) //(1600)//(3272-8)
#define S5K4E1FXMIPI_IMAGE_SENSOR_PV_HEIGHT		(S5K4E1FXMIPI_REAL_PV_HEIGHT + 2 * S5K4E1FXMIPI_IMAGE_SENSOR_PV_STARTY)	// 2*: Leave PV_STARTY unused space at both top side and bottom side of REAL_PV_HEIGHT.	//(1220) //(612) //(1200)//(612)

/* SENSOR 8M SIZE */
#define S5K4E1FXMIPI_IMAGE_SENSOR_FULL_WIDTH		(S5K4E1FXMIPI_REAL_CAP_WIDTH  + 2 * S5K4E1FXMIPI_IMAGE_SENSOR_CAP_STARTX)	// 2*: Leave CAP_STARTX unused space at both left side and right side of REAL_CAP_WIDTH.	//3284
#define S5K4E1FXMIPI_IMAGE_SENSOR_FULL_HEIGHT		(S5K4E1FXMIPI_REAL_CAP_HEIGHT + 2 * S5K4E1FXMIPI_IMAGE_SENSOR_CAP_STARTY)	// 2*: Leave CAP_STARTY unused space at both top side and bottom side of REAL_CAP_HEIGHT.	//2462

/* SENSOR PIXEL/LINE NUMBERS IN ONE PERIOD */
#define S5K4E1FXMIPI_PV_PERIOD_PIXEL_NUMS			S5K4E1FXMIPI_IMAGE_SENSOR_PV_WIDTH	// 720P mode's pixel # in one HSYNC w/o dummy pixels
#define S5K4E1FXMIPI_PV_PERIOD_LINE_NUMS			S5K4E1FXMIPI_IMAGE_SENSOR_PV_HEIGHT	//720P mode's HSYNC # in one HSYNC w/o dummy lines
#define S5K4E1FXMIPI_FULL_PERIOD_PIXEL_NUMS		S5K4E1FXMIPI_IMAGE_SENSOR_FULL_WIDTH	//8M mode's pixel # in one HSYNC w/o dummy pixels
#define S5K4E1FXMIPI_FULL_PERIOD_LINE_NUMS		S5K4E1FXMIPI_IMAGE_SENSOR_FULL_HEIGHT	//8M mode's HSYNC # in one HSYNC w/o dummy lines

#define S5K4E1FXMIPI_PV_PERIOD_EXTRA_PIXEL_NUMS	18 // 4 //32
#define S5K4E1FXMIPI_PV_PERIOD_EXTRA_LINE_NUMS	34 // 17 //44 //500
#define S5K4E1FXMIPI_FULL_PERIOD_EXTRA_PIXEL_NUMS	36
#define S5K4E1FXMIPI_FULL_PERIOD_EXTRA_LINE_NUMS	36

#define S5K4E1FXMIPI_IMAGE_SENSOR_8M_PIXELS_LINE         (3292 + 168)
#define S5K4E1FXMIPI_IMAGE_SENSOR_720P_PIXELS_LINE      1690 //1716 // 820 //1600 // 616

#define MAX_FRAME_RATE	(15)
#define MIN_FRAME_RATE  (12)

/* SENSOR EXPOSURE LINE LIMITATION */
#define S5K4E1FXMIPI_FULL_EXPOSURE_LIMITATION   (2496) // 8M mode
#define S5K4E1FXMIPI_PV_EXPOSURE_LIMITATION (1222) // (612 + 490) // (1254)  // # of lines in one 720P frame
// SENSOR VGA SIZE

#define S5K4E1FXMIPI_IMAGE_SENSOR_CCT_WIDTH		(3284)
#define S5K4E1FXMIPI_IMAGE_SENSOR_CCT_HEIGHT		(2462)

//For 2x Platform camera_para.c used
#define IMAGE_SENSOR_PV_WIDTH		S5K4E1FXMIPI_IMAGE_SENSOR_PV_WIDTH
#define IMAGE_SENSOR_PV_HEIGHT		S5K4E1FXMIPI_IMAGE_SENSOR_PV_HEIGHT

#define IMAGE_SENSOR_FULL_WIDTH		S5K4E1FXMIPI_IMAGE_SENSOR_FULL_WIDTH
#define IMAGE_SENSOR_FULL_HEIGHT	S5K4E1FXMIPI_IMAGE_SENSOR_FULL_HEIGHT

#define S5K4E1FXMIPI_SHUTTER_LINES_GAP	  3
#endif

#define S5K4E1FXMIPI_WRITE_ID 	0x20//(0x6E)
#define S5K4E1FXMIPI_READ_ID	0x21//(0x6F)

// SENSOR CHIP VERSION

#define S5K4E1FXMIPI_SENSOR_ID            S5K4E1FX_SENSOR_ID

#define S5K4E1FXMIPI_PAGE_SETTING_REG    (0xFF)

//s_add for porting
//s_add for porting
//s_add for porting

//export functions
UINT32 S5K4E1FXMIPIOpen(void);
UINT32 S5K4E1FXMIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 S5K4E1FXMIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 S5K4E1FXMIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 S5K4E1FXMIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 S5K4E1FXMIPIClose(void);

//#define Sleep(ms) mdelay(ms)
//#define RETAILMSG(x,...)
//#define TEXT

//e_add for porting
//e_add for porting
//e_add for porting

#endif /* __SENSOR_H */

