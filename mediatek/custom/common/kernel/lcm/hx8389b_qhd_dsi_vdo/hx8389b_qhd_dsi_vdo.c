#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <platform/mt_pwm.h>
	#include <printf.h>
	#ifdef LCD_DEBUG
		#define LCM_DEBUG(format, ...)   printf("uboot ssd2825" format "\n", ## __VA_ARGS__)
	#else
		#define LCM_DEBUG(format, ...)
	#endif

#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt6577_gpio.h>
	#include <asm/arch/mt6577_pwm.h>

	#ifdef LCD_DEBUG
		#define LCM_DEBUG(format, ...)   printf("uboot ssd2825" format "\n", ## __VA_ARGS__)
	#else
		#define LCM_DEBUG(format, ...)
	#endif
#else
	#include <mach/mt_gpio.h>
	#include <mach/mt_pwm.h>
	#include <mach/mt_pm_ldo.h>
	#ifdef LCD_DEBUG
		#define LCM_DEBUG(format, ...)   printk("kernel ssd2825" format "\n", ## __VA_ARGS__)
	#else
		#define LCM_DEBUG(format, ...)
	#endif
#endif

#include "lcm_drv.h"

#ifdef BUILD_UBOOT
#elif defined(BUILD_LK) 
#else
#include "lcd_reg.h"
#include "lcd_drv.h"

#include "dpi_reg.h"
#include "dpi_drv.h"

#include "dsi_reg.h"
#include "dsi_drv.h"

#include "disp_drv.h"
#include "disp_drv_log.h"

extern void DSI_wait_2frame_end_L90(void);
#endif
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(540)
#define FRAME_HEIGHT 										(960)

#define REGFLAG_DELAY             							0XFF
#define REGFLAG_END_OF_TABLE      							0xFE   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

#define LCM_ID_HX8389B 0x89

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V3(ppara, size, force_update)			lcm_util.dsi_set_cmdq_V3(ppara, size, force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    
       
static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[128];
};

static LCM_setting_table_V3 lcm_initialization_setting_V3[] = {

	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/
	{0X39, 0XB9, 3, {0XFF,0X83,0X89}},
	{0X39, 0XBA, 2, {0X01,0X92}}, 
	{0X39, 0XDE, 2, {0X05,0X58}},
#if 0//                                                                       
	{0X39, 0XB1, 19, {0X00,0X00,0X04,0XE4,
				0X9B,0X10,0X11,0XD1,
				0XF0,0X36,0X3E,0X23,
				0X23,0X43,0X01,0X2A,
		  		0XFB,0X20,0X00}},
#else // fix
	{0X39, 0XB1, 19, {0X00,0X00,0X04,0XE4,
				0X9B,0X10,0X11,0XD1,
				0XF0,0X36,0X3E,0X3F,
				0X3F,0X43,0X01,0X2A,
		  		0XFB,0X20,0X00}},
#endif //                     
	{0X39, 0XB2, 7,   {0X00,0X00,0X78,0X08,
				 0X03,0X00,0XF0}},
#if 0//                                                                      
	{0X39, 0XB6, 2,   {0X00,0X96}},
#else // fix
	{0X39, 0XB6, 2,   {0X00,0X98}},
#endif //                     
#if 0//                                                                      
	{0X39, 0XCB, 2,   {0X07,0X07}},
	{0X00, REGFLAG_DELAY, 10, {}},
#endif //                     
	{0X15, 0XCC, 1,   {0X0A}},
#if 0//                                                                       
	{0X39, 0XB4, 23, {0X00,0X08,0X00,0X32,
				0X10,0X00,0X00,0X00,
				0X00,0X00,0X00,0X00,
				0X37,0X0A,0X40,0X04,
				0X37,0X0A,0X40,0X14,
				0X50,0X55,0X0A}}, 
#else // fix
	{0X39, 0XB4, 23, {0X00,0X08,0X00,0X32,
				0X10,0X00,0X00,0X00,
				0X00,0X00,0X00,0X00,
				0X37,0X0A,0X40,0X04,
				0X37,0X0A,0X40,0X14,
				0X65,0X65,0X0A}}, 
#endif //               

#if 0//                                                                      
	{0X39, 0XBB, 4, {0x00,0x00,0xFF,0x80}}, 
	{0X00, REGFLAG_DELAY, 10, {}},
#endif //               

	{0X39, 0XD5, 50, {0x00,0x00,0x4C,0x00,
				0x01,0x00,0x00,0x00,
				0x60,0x00,0x99,0x88,
				0xAA,0xBB,0x88,0x23,
				0x88,0x01,0x88,0x67,
				0x88,0x45,0x01,0x23,
				0x88,0x88,0x88,0x88,
				0x88,0x88,0x99,0xAA,
				0xBB,0x88,0x54,0x88,
				0x76,0x88,0x10,0x88,
				0x32,0x32,0x10,0x88,
				0x88,0x88,0x88,0x88,
				0x01,0x02}}, 

	// SET_LUT 2013-08-06 
	{0X39, 0XC1, 127, {0x01,0x00,0x14,0x20,
				0x28,0x32,0x3C,0x47,
				0x50,0x59,0x61,0x68,
				0x6F,0x78,0x7F,0x87,
				0x8D,0x95,0x9C,0xA3,
				0xA9,0xB0,0xB7,0xBF,
				0xC7,0xCB,0xD0,0xD8,
				0xE2,0xE5,0xED,0xF4,
				0xF9,0xFE,0x00,0x13,
				0x8B,0xE3,0x81,0xB3,
				0x3E,0xE0,0x40,0x00,
				0x14,0x20,0x28,0x32,
				0x3C,0x47,0x50,0x59,
				0x61,0x68,0x6F,0x78,
				0x7F,0x87,0x8D,0x95,
				0x9C,0xA3,0xA9,0xB0,
				0xB7,0xBF,0xC7,0xCB,
				0xD0,0xD8,0xE2,0xE5,
				0xED,0xF4,0xF9,0xFE,
				0x00,0x13,0x8B,0xE3,
				0x81,0xB3,0x3E,0xE0,
				0x40,0x00,0x14,0x20,
				0x28,0x32,0x3C,0x47,
				0x50,0x59,0x61,0x68,
				0x6F,0x78,0x7F,0x87,
				0x8D,0x95,0x9C,0xA3,
				0xA9,0xB0,0xB7,0xBF,
				0xC7,0xCB,0xD0,0xD8,
				0xE2,0xE5,0xED,0xF4,
				0xF9,0xFE,0x00,0x13,
				0x8B,0xE3,0x81,0xB3,
				0x3E,0xE0,0x40}},

// SET GAMMA 2013-08-06 
	{0X39, 0XE0, 34, {0x05,0x11,0x18,0x33,
				0x3F,0x3F,0x25,0x4F,
				0x05,0x0E,0x0D,0x13,
				0x15,0x11,0x16,0x14,
				0x18,0x05,0x11,0x18,
				0x33,0x3F,0x3F,0x25,
				0x4F,0x05,0x0E,0x0D,
				0x13,0x15,0x11,0x16,
				0x14,0x18}},
};
// Ver 3.0
static LCM_setting_table_V3 lcm_resume_setting_V3_first[] ={
		//External Power
		{0X39, 0XB0, 4, {0X00,0X01,0X00,0X04}},
		{0X39, 0XCF, 3, {0X00,0X1D,0X03}},

		{0X39, 0XB0, 4, {0X00,0X01,0X08,0X04}},

		// 10ms delay
		{0X00, REGFLAG_DELAY, 10, {}},

		{0X39, 0XB0, 4, {0X00,0X01,0X18,0X04}},

		// 25ms delay
		{0X00, REGFLAG_DELAY, 25, {}},

		{0X39, 0XB0, 4, {0X00,0X01,0X1C,0X04}},

		// 5ms delay
		{0X00, REGFLAG_DELAY, 5, {}},

		{0X39, 0XCF, 3, {0X00,0X15,0X03}},

		// 5ms delay
		{0X00, REGFLAG_DELAY, 5, {}},

		{0X39, 0XCF, 3, {0X00,0X01,0X03}},
		{0X39, 0XB0, 4, {0X00,0X01,0X3C,0X04}},
		{0X39, 0XB0, 4, {0X00,0X01,0X7C,0X04}},
};

static LCM_setting_table_V3 lcm_resume_setting_V3_second[] ={
		// 5ms delay
		{0X00, REGFLAG_DELAY, 5, {}},

		{0X39, 0XCF, 3, {0X00,0X00,0X03}},
		{0X39, 0XB0, 4, {0X00,0X01,0X7C,0X0E}},
};

static LCM_setting_table_V3 lcm_resume_setting_V3_last[] ={
		{0X39, 0XB0, 4, {0X00,0X01,0X7C,0X0F}},
};




static LCM_setting_table_V3 lcm_sleep_out_setting_V3_first[] = {
    // Sleep Out
	{0x05, 0x11, 0, {0x00}},
};

static LCM_setting_table_V3 lcm_sleep_out_setting_V3_second[] = {
    // Display ON
	{0x05, 0x29, 0, {0x00}},
};

static LCM_setting_table_V3 lcm_sleep_in_setting_V3_first[] = {
	// Display off sequence
    // Sleep Mode On - first
	{0x05, 0x28, 0, {0x00}},
};

static LCM_setting_table_V3 lcm_sleep_in_setting_V3_second[] = {
	// Display off sequence
    // Sleep Mode On - second
	{0x05, 0x10, 0, {0x00}},
};


static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/


	//must use 0x39 for init setting for all register.

	{0XB9, 3, {0XFF,0X83,0X89}},
	{REGFLAG_DELAY, 10, {}},

#if 0//CLOSE IC ESD Protect enhance
	{0XBA, 17, {0X41,0X83,0X00,0X16,
				0XA4,0X00,0X18,0XFF,
				0X0F,0X21,0X03,0X21,
				0X23,0X25,0X20,0X02, 
		  		0X31}}, 
	{REGFLAG_DELAY, 10, {}},
#else //open IC ESD  Protect enhance 
{0XBA, 17, {0X41,0X83,0X00,0X16,
			0XA4,0X00,0X18,0XFF,
			0X0F,0X21,0X03,0X21,
			0X23,0X25,0X20,0X02, 
			0X35}}, 
{REGFLAG_DELAY, 10, {}},

#endif

	{0XDE, 2, {0X05,0X58}},
	{REGFLAG_DELAY, 10, {}},

	{0XB1, 19, {0X00,0X00,0X04,0XD9,
				0XCF,0X10,0X11,0XAC,
				0X0C,0X1D,0X25,0X1D,
				0X1D,0X42,0X01,0X58,
		  		0XF7,0X20,0X80}},
	{REGFLAG_DELAY, 10, {}},


	{0XB2, 5,   {0X00,0X00,0X78,0X03,
				 0X02}},
	{REGFLAG_DELAY, 10, {}},
	
	{0XB4, 31, {0X82,0X04,0X00,0X32,
				0X10,0X00,0X32,0X10,
				0X00,0X00,0X00,0X00,
				0X17,0X0A,0X40,0X01,
				0X13,0X0A,0X40,0X14,
				0X46,0X50,0X0A,0X0A,
				0X3C,0X0A,0X3C,0X14,
		  		0X46,0X50,0X0A}}, 
	{REGFLAG_DELAY, 10, {}},
	
	{0XD5, 48, {0X00,0X00,0X00,0X00,
				0X01,0X00,0X00,0X00,
				0X20,0X00,0X99,0X88,
				0X88,0X88,0X88,0X88,
				0X88,0X88,0X88,0X01,
				0X88,0X23,0X01,0X88,
				0X88,0X88,0X88,0X88,
				0X88,0X88,0X99,0X88,
				0X88,0X88,0X88,0X88,
				0X88,0X88,0X32,0X88,
				0X10,0X10,0X88,0X88,
				0X88,0X88,0X88,0X88}}, 
	{REGFLAG_DELAY, 10, {}},
	

	{0XB6, 4,   {0X00,0X8A,0X00,0X8A}},
	{REGFLAG_DELAY, 10, {}},

	{0XCC, 1,   {0X02}},
	{REGFLAG_DELAY, 10, {}},


	{0X35, 1,   {0X00}},//TE on
	{REGFLAG_DELAY, 10, {}},

	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.

	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
	{REGFLAG_DELAY, 10, {}},
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_in_setting[] = {
	// Display off sequence
	{0x28, 0, {0x00}},

    // Sleep Mode On
	{0x10, 0, {0x00}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }
	
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;//SYNC_EVENT_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 2;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.vertical_sync_active				= 5;
		params->dsi.vertical_backporch					= 7;//11;
		params->dsi.vertical_frontporch					= 5;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 40;
		params->dsi.horizontal_backporch				= 40;
		params->dsi.horizontal_frontporch				= 40;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		// Bit rate calculation
		params->dsi.pll_div1=35;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)

		/* ESD or noise interference recovery For video mode LCM only. */
		// Send TE packet to LCM in a period of n frames and check the response.
		params->dsi.lcm_int_te_monitor = FALSE;
		params->dsi.lcm_int_te_period = 1;		// Unit : frames

		// Need longer FP for more opportunity to do int. TE monitor applicably.
		if(params->dsi.lcm_int_te_monitor)
			params->dsi.vertical_frontporch *= 2;
		
		// Monitor external TE (or named VSYNC) from LCM once per 2 sec. (LCM VSYNC must be wired to baseband TE pin.)
		params->dsi.lcm_ext_te_monitor = FALSE;
		// Non-continuous clock
		params->dsi.noncont_clock = TRUE;
		params->dsi.noncont_clock_period = 2;	// Unit : frames

		// DSI MIPI Spec parameters setting
		params->dsi.HS_TRAIL = 6;
		params->dsi.HS_ZERO = 9;
		params->dsi.HS_PRPR = 5;
		params->dsi.LPX = 4;
		params->dsi.TA_SACK = 1;
		params->dsi.TA_GET = 20;
		params->dsi.TA_SURE = 6;
		params->dsi.TA_GO = 16;
		params->dsi.CLK_TRAIL = 5;
		params->dsi.CLK_ZERO = 18;
		params->dsi.LPX_WAIT = 1;
		params->dsi.CONT_DET = 0;
		params->dsi.CLK_HS_PRPR = 4;
}


static void lcm_init(void)
{
	unsigned int data_array[40];
#ifdef BUILD_UBOOT 
#elif defined(BUILD_LK) 
	mt_set_gpio_mode(GPIO_LCD_RESET, GPIO_LCD_RESET_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_LCD_RESET, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_LCD_RESET, GPIO_DIR_OUT);

	mt_set_gpio_out(GPIO_LCD_RESET, GPIO_OUT_ONE);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RESET, GPIO_OUT_ZERO);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RESET, GPIO_OUT_ONE);
	MDELAY(50);

#else
	hwPowerOn(MT65XX_POWER_LDO_VCAM_IO, VOL_1800, "1V8_LCD_VIO_MTK_S");
	MDELAY(1);
	hwPowerOn(MT65XX_POWER_LDO_VCAMD, VOL_3000, "3V0_LCD_VCC_MTK_S");
	MDELAY(1);

	mt_set_gpio_mode(GPIO_LCD_RESET, GPIO_LCD_RESET_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_LCD_RESET, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_LCD_RESET, GPIO_DIR_OUT);
	
	mt_set_gpio_out(GPIO_LCD_RESET, GPIO_OUT_ONE);
	MDELAY(1);
	mt_set_gpio_out(GPIO_LCD_RESET, GPIO_OUT_ZERO);
	MDELAY(1);
	mt_set_gpio_out(GPIO_LCD_RESET, GPIO_OUT_ONE);
	MDELAY(50);	

#endif
	dsi_set_cmdq_V3(lcm_initialization_setting_V3, sizeof(lcm_initialization_setting_V3) / sizeof(LCM_setting_table_V3), 1);
	MDELAY(70);	
}


static void lcm_suspend(void)
{
	unsigned int data_array[1];

	MDELAY(10);
	
	data_array[0] = 0x00280500; //Display OFF
	dsi_set_cmdq(&data_array, 1, 1);

#if 1	// 0731 - TEST

#ifdef BUILD_UBOOT
#elif defined(BUILD_LK)
#else
#if 1
	// 1Frame or more :DSI_wait_2frame_end_L90();
	MDELAY(60);	
#else
	// 1Frame or more :DSI_wait_2frame_end_L90();
	MDELAY(30);	

	// MIPI Video Packet Disable

	DPI_CHECK_RET(DPI_DisableClk());
	DSI_Reset();
	DSI_clk_HS_mode(0);
	DSI_SetMode(CMD_MODE);

	MDELAY(30);	
#endif

	mt_set_gpio_mode(GPIO_DSV_EN, GPIO_DSV_EN_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_DSV_EN, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_DSV_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_DSV_EN, GPIO_OUT_ZERO);

//	MDELAY(1);	

	mt_set_gpio_mode(GPIO_LCD_RESET, GPIO_LCD_RESET_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_LCD_RESET, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_LCD_RESET, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_RESET, GPIO_OUT_ZERO);

	MDELAY(20);	

	MDELAY(1);	
	hwPowerDown(MT65XX_POWER_LDO_VCAM_IO, "1V8_LCD_VIO_MTK_S");
	MDELAY(1);
	hwPowerDown(MT65XX_POWER_LDO_VCAMD, "3V0_LCD_VCC_MTK_S");
	MDELAY(1);
	
#endif	
#else

#ifdef BUILD_UBOOT 
#elif defined(BUILD_LK) 
#else
	// 2Frame or more :DSI_wait_2frame_end_L90();
	MDELAY(40);	
#endif	

	mt_set_gpio_mode(GPIO_DSV_EN, GPIO_DSV_EN_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_DSV_EN, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_DSV_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_DSV_EN, GPIO_OUT_ZERO);
	MDELAY(10);	

	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(&data_array, 1, 1);	
	MDELAY(1);

#ifdef BUILD_UBOOT 
#elif defined(BUILD_LK) 
#else

	// 2Frame or more :DSI_wait_2frame_end_L90();
	MDELAY(40);	

	// MIPI Video Packet Disable
	DPI_CHECK_RET(DPI_DisableClk());
	DSI_Reset();
	DSI_clk_HS_mode(0);
	DSI_SetMode(CMD_MODE);

	MDELAY(20);	

	mt_set_gpio_mode(GPIO_LCD_RESET, GPIO_LCD_RESET_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_LCD_RESET, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_LCD_RESET, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_RESET, GPIO_OUT_ZERO);

	MDELAY(1);	
	hwPowerDown(MT65XX_POWER_LDO_VCAM_IO, "1V8_LCD_VIO_MTK_S");
	MDELAY(1);
	hwPowerDown(MT65XX_POWER_LDO_VCAMD, "3V0_LCD_VCC_MTK_S");
	MDELAY(1);
#endif

#endif
}


static void lcm_resume(void)
{
	unsigned int data_array[1];	

	lcm_init();

#ifdef BUILD_UBOOT
#elif defined(BUILD_LK)
	dsi_set_cmdq_V3(lcm_resume_setting_V3_first, sizeof(lcm_resume_setting_V3_first) / sizeof(LCM_setting_table_V3), 1);

	mt_set_gpio_mode(GPIO_DSV_EN, GPIO_DSV_EN_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_DSV_EN, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_DSV_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_DSV_EN, GPIO_OUT_ONE);

	data_array[0] = 0x00380500;		// IDLE MODE OFF
	dsi_set_cmdq(&data_array, 1, 1);

	dsi_set_cmdq_V3(lcm_resume_setting_V3_second, sizeof(lcm_resume_setting_V3_second) / sizeof(LCM_setting_table_V3), 1);

	MDELAY(50);

	dsi_set_cmdq_V3(lcm_resume_setting_V3_last, sizeof(lcm_resume_setting_V3_last) / sizeof(LCM_setting_table_V3), 1);

	MDELAY(20);
#else
	dsi_set_cmdq_V3(lcm_resume_setting_V3_first, sizeof(lcm_resume_setting_V3_first) / sizeof(LCM_setting_table_V3), 1);

	mt_set_gpio_mode(GPIO_DSV_EN, GPIO_DSV_EN_M_GPIO);
	mt_set_gpio_pull_enable(GPIO_DSV_EN, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO_DSV_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_DSV_EN, GPIO_OUT_ONE);

	data_array[0] = 0x00380500;		// IDLE MODE OFF
	dsi_set_cmdq(&data_array, 1, 1);	

	dsi_set_cmdq_V3(lcm_resume_setting_V3_second, sizeof(lcm_resume_setting_V3_second) / sizeof(LCM_setting_table_V3), 1);

	MDELAY(50);

	dsi_set_cmdq_V3(lcm_resume_setting_V3_last, sizeof(lcm_resume_setting_V3_last) / sizeof(LCM_setting_table_V3), 1);

	MDELAY(20);
#endif
}


static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(&data_array, 7, 0);

}


static unsigned int lcm_compare_id(void)
{
#if 0
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];  

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(10);//Must over 6 ms

	array[0]=0x00043902;
	array[1]=0x8983FFB9;// page enable
	dsi_set_cmdq(&array, 2, 1);
	MDELAY(10);

	array[0] = 0x00023700;// return byte number
	dsi_set_cmdq(&array, 1, 1);
	MDELAY(10);

	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; 
	
#if defined(BUILD_UBOOT)
	printf("%s, id = 0x%08x\n", __func__, id);
#endif

	return (LCM_ID_HX8389B == id)?1:0;
#else
	return 1;
#endif
}




LCM_DRIVER hx8389b_qhd_dsi_vdo_drv = 
{
    .name			= "hx8389b_dsi_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
    .update         = lcm_update,
#endif
};


