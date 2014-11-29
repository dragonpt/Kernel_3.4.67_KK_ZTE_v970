/*******************************************************************************
* mt_pwm_hal.c PWM Drvier                                                     
*                                                                                             
* Copyright (c) 2012, Media Teck.inc                                           
*                                                                             
* This program is free software; you can redistribute it and/or modify it     
* under the terms and conditions of the GNU General Public Licence,            
* version 2, as publish by the Free Software Foundation.                       
*                                                                              
* This program is distributed and in hope it will be useful, but WITHOUT       
* ANY WARRNTY; without even the implied warranty of MERCHANTABITLITY or        
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for     
* more details.                                                                
*                                                                              
*                                                                              
********************************************************************************
* Author : How wang (how.wang@mediatek.com)                              
********************************************************************************
*/

#include <linux/kernel.h>
#include <generated/autoconf.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include <mach/mt_pwm_hal_pub.h>
#include <mach/mt_pwm_hal.h>
#include <mach/mt_pwm_prv.h>

enum {                                                                                     
	PWM_CON,                                                                              
	PWM_HDURATION,                                                                        
	PWM_LDURATION,                                                                        
	PWM_GDURATION,                                                                        
	PWM_BUF0_BASE_ADDR,                                                                   
	PWM_BUF0_SIZE,                                                                        
	PWM_BUF1_BASE_ADDR,                                                                   
	PWM_BUF1_SIZE,                                                                        
	PWM_SEND_DATA0,                                                                       
	PWM_SEND_DATA1,                                                                       
	PWM_WAVE_NUM,                                                                         
	PWM_DATA_WIDTH,      //only PWM1, PWM2, PWM3, PWM7 have such a register for old mode  
	PWM_THRESH,          //only PWM1, PWM2, PWM3, PWM7 have such a register for old mode  
	PWM_SEND_WAVENUM,                                                                     
	PWM_VALID                                                                             
}PWM_REG_OFF;

U32 PWM_register[PWM_NUM]={                                                                                                                                                     
	(PWM_BASE+0x0010),     //PWM1 REGISTER BASE,   15 registers  
	(PWM_BASE+0x0050),     //PWM2 register base    15 registers                
	(PWM_BASE+0x0090),     //PWM3 register base    15 registers                
	(PWM_BASE+0x00d0),     //PWM4 register base    13 registers                
	(PWM_BASE+0x0110),     //PWM5 register base    13 registers                
	(PWM_BASE+0x0150),     //PWM6 register base    13 registers                
	(PWM_BASE+0x0190)      //PWM7 register base    15 registers                
}; 

void mt_pwm_power_on_hal(U32 pwm_no, BOOL pmic_pad, unsigned long* power_flag)
{
	if ( pwm_no == PWM0) {
		enable_clock( MT65XX_PDN_PERI_PWM1, "PWM");  //enable clock 
		PWMDBG("enable_clock PWM1\n");
	}
	if ( pwm_no == PWM1) {
		enable_clock( MT65XX_PDN_PERI_PWM2, "PWM");  //enable clock 
		PWMDBG("enable_clock PWM2\n");
	}
	if ( pwm_no == PWM2) {
		enable_clock( MT65XX_PDN_PERI_PWM3, "PWM");  //enable clock 
		PWMDBG("enable_clock PWM3\n");
	}
	if ( pwm_no == PWM3 || pwm_no == PWM4 || pwm_no == PWM5) {
		enable_clock( MT65XX_PDN_PERI_PWM456, "PWM");  //enable clock 
		PWMDBG("enable_clock PWM456\n");
	}
	if ( pwm_no == PWM6) {
		enable_clock( MT65XX_PDN_PERI_PWM7, "PWM");  //enable clock 
		PWMDBG("enable_clock PWM7\n");
	}
}

void mt_pwm_power_off_hal(U32 pwm_no, BOOL pmic_pad, unsigned long* power_flag)
{
	if ( pwm_no == PWM0) {
		disable_clock( MT65XX_PDN_PERI_PWM1, "PWM");  //disable clock 
		PWMDBG("disable_clock PWM1\n");
	}
	if ( pwm_no == PWM1) {
		disable_clock( MT65XX_PDN_PERI_PWM2, "PWM");  //disable clock 
		PWMDBG("disable_clock PWM2\n");
	}
	if ( pwm_no == PWM2) {
		disable_clock( MT65XX_PDN_PERI_PWM3, "PWM");  //disable clock 
		PWMDBG("disable_clock PWM3\n");
	}
	if ( pwm_no == PWM3 || pwm_no == PWM4 || pwm_no == PWM5) {
		disable_clock( MT65XX_PDN_PERI_PWM456, "PWM");  //disable clock 
		PWMDBG("disable_clock PWM456\n");
	}
	if ( pwm_no == PWM6) {
		disable_clock( MT65XX_PDN_PERI_PWM7, "PWM");  //disable clock 
		PWMDBG("disable_clock PWM7\n");
	}
}

void mt_pwm_init_power_flag(unsigned long* power_flag)
{
}

S32 mt_pwm_sel_pmic_hal(U32 pwm_no)
{
	return RSUCCESS;
}

S32 mt_pwm_sel_ap_hal(U32 pwm_no)
{
	return RSUCCESS;
}

void mt_set_pwm_enable_hal(U32 pwm_no)
{
	SETREG32(PWM_ENABLE, 1 << pwm_no);
}
void mt_set_pwm_disable_hal(U32 pwm_no)
{
	CLRREG32 ( PWM_ENABLE, 1 << pwm_no );
	mdelay(1);
}

void mt_set_pwm_enable_seqmode_hal(void)
{
	SETREG32 ( PWM_ENABLE, 1 << PWM_ENABLE_SEQ_OFFSET );
}

void mt_set_pwm_disable_seqmode_hal(void)
{
	CLRREG32 ( PWM_ENABLE,1 << PWM_ENABLE_SEQ_OFFSET );
}

S32 mt_set_pwm_test_sel_hal(U32 val)   //val as 0 or 1
{
	if (val == TEST_SEL_TRUE)
		SETREG32 ( PWM_ENABLE, 1 << PWM_ENABLE_TEST_SEL_OFFSET );
	else if ( val == TEST_SEL_FALSE )
		CLRREG32 ( PWM_ENABLE, 1 << PWM_ENABLE_TEST_SEL_OFFSET );
	else
		return 1;
	return 0;
}

void mt_set_pwm_clk_hal (U32 pwm_no, U32 clksrc, U32 div )
{
	U32 reg_con;

	reg_con = PWM_register [pwm_no] + 4* PWM_CON;
	MASKREG32 ( reg_con, PWM_CON_CLKDIV_MASK, div );
	if (clksrc == CLK_BLOCK)
		CLRREG32 ( reg_con, 1 << PWM_CON_CLKSEL_OFFSET );
	else if (clksrc == CLK_BLOCK_BY_1625_OR_32K)
		SETREG32 ( reg_con, 1 << PWM_CON_CLKSEL_OFFSET );
}

S32 mt_get_pwm_clk_hal(U32 pwm_no)
{
	S32 clk, clksrc, clkdiv;
	U32 reg_con, reg_val,reg_en;

	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	reg_val = INREG32 (reg_con);
	reg_en = INREG32 (PWM_ENABLE);

	if ( ( ( reg_val & PWM_CON_CLKSEL_MASK ) >> PWM_CON_CLKSEL_OFFSET ) == 1 )
		if ( ((reg_en &PWM_CON_OLD_MODE_MASK) >> PWM_CON_OLD_MODE_OFFSET ) == 1)
			clksrc = 32*1024;
		else clksrc = BLOCK_CLK;
	else
		clksrc = BLOCK_CLK/1625;

	clkdiv = 2 << ( reg_val & PWM_CON_CLKDIV_MASK);
	if ( clkdiv <= 0 ) {
		PWMDBG ( "clkdiv less zero, not valid \n" );
		return -ERROR;
	}

	clk = clksrc/clkdiv;
	PWMDBG ( "CLK is :%d\n", clk );

	return clk;
}

S32 mt_set_pwm_con_datasrc_hal ( U32 pwm_no, U32 val )
{
	U32 reg_con;
	
	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if ( val == PWM_FIFO )
		CLRREG32 ( reg_con, 1 << PWM_CON_SRCSEL_OFFSET );
	else if ( val == MEMORY )
		SETREG32 ( reg_con, 1 << PWM_CON_SRCSEL_OFFSET );
	else 
		return 1;
	return 0;
}

S32 mt_set_pwm_con_mode_hal( U32 pwm_no, U32 val )
{
	U32 reg_con;
	
	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if ( val == PERIOD )
		CLRREG32 ( reg_con, 1 << PWM_CON_MODE_OFFSET );
	else if (val == RAND)
		SETREG32 ( reg_con, 1 << PWM_CON_MODE_OFFSET );
	else
		return 1;
	return 0;
}

S32 mt_set_pwm_con_idleval_hal(U32 pwm_no, U16 val)
{
	U32 reg_con;
	
	reg_con = PWM_register[pwm_no] + 4*PWM_CON;

	if ( val == IDLE_TRUE )
		SETREG32 ( reg_con,1 << PWM_CON_IDLE_VALUE_OFFSET );
	else if ( val == IDLE_FALSE )
		CLRREG32 ( reg_con, 1 << PWM_CON_IDLE_VALUE_OFFSET );
	else 
		return 1;
	return 0;
}

S32 mt_set_pwm_con_guardval_hal(U32 pwm_no, U16 val)
{
	U32 reg_con;
	
	reg_con = PWM_register[pwm_no] + 4*PWM_CON;
	if ( val == GUARD_TRUE )
		SETREG32 ( reg_con, 1 << PWM_CON_GUARD_VALUE_OFFSET );
	else if ( val == GUARD_FALSE )
		CLRREG32 ( reg_con, 1 << PWM_CON_GUARD_VALUE_OFFSET );
	else 
		return 1;
	return 0;
}

void mt_set_pwm_con_stpbit_hal(U32 pwm_no, U32 stpbit, U32 srcsel )
{
	U32 reg_con;
	
	reg_con = PWM_register[pwm_no] + 4*PWM_CON;
	if ( srcsel == PWM_FIFO )
		MASKREG32 ( reg_con, PWM_CON_STOP_BITS_MASK, stpbit << PWM_CON_STOP_BITS_OFFSET);
	if ( srcsel == MEMORY )
		MASKREG32 ( reg_con, PWM_CON_STOP_BITS_MASK & (0x1f << PWM_CON_STOP_BITS_OFFSET), stpbit << PWM_CON_STOP_BITS_OFFSET);
}

S32 mt_set_pwm_con_oldmode_hal ( U32 pwm_no, U32 val )
{
	U32 reg_con;
	
	reg_con = PWM_register[pwm_no] + 4*PWM_CON;
	if ( val == OLDMODE_DISABLE )
		CLRREG32 ( reg_con, 1 << PWM_CON_OLD_MODE_OFFSET );
	else if ( val == OLDMODE_ENABLE )
		SETREG32 ( reg_con, 1 << PWM_CON_OLD_MODE_OFFSET );
	else 
		return 1;
	return 0;
}

void mt_set_pwm_HiDur_hal(U32 pwm_no, U16 DurVal)  //only low 16 bits are valid
{
	U32 reg_HiDur;
	
	reg_HiDur = PWM_register[pwm_no]+4*PWM_HDURATION;
	OUTREG32 ( reg_HiDur, DurVal);	
}

void mt_set_pwm_LowDur_hal (U32 pwm_no, U16 DurVal)
{
	U32 reg_LowDur;
	reg_LowDur = PWM_register[pwm_no] + 4*PWM_LDURATION;
	OUTREG32 ( reg_LowDur, DurVal );
}

void mt_set_pwm_GuardDur_hal (U32 pwm_no, U16 DurVal)
{
	U32 reg_GuardDur;
	reg_GuardDur = PWM_register[pwm_no] + 4*PWM_GDURATION;
	OUTREG32 ( reg_GuardDur, DurVal );
}

void mt_set_pwm_send_data0_hal ( U32 pwm_no, U32 data )
{
	U32 reg_data0;
	reg_data0 = PWM_register[pwm_no] + 4 * PWM_SEND_DATA0;
	OUTREG32 ( reg_data0, data );
}

void mt_set_pwm_send_data1_hal ( U32 pwm_no, U32 data )
{
	U32 reg_data1;
	reg_data1 = PWM_register[pwm_no] + 4 * PWM_SEND_DATA1;
	OUTREG32 ( reg_data1, data );
}

void mt_set_pwm_wave_num_hal ( U32 pwm_no, U16 num )
{
	U32 reg_wave_num;
	reg_wave_num = PWM_register[pwm_no] + 4 * PWM_WAVE_NUM;
	OUTREG32 ( reg_wave_num, num );
}

void mt_set_pwm_data_width_hal ( U32 pwm_no, U16 width )
{
	U32 reg_data_width;
	reg_data_width = PWM_register[pwm_no] + 4 * PWM_DATA_WIDTH;
	OUTREG32 ( reg_data_width, width );
}

void mt_set_pwm_thresh_hal ( U32 pwm_no, U16 thresh )
{
	U32 reg_thresh;

	reg_thresh = PWM_register[pwm_no] + 4 * PWM_THRESH;
	OUTREG32 ( reg_thresh, thresh );
}

S32 mt_get_pwm_send_wavenum_hal ( U32 pwm_no )
{
	U32 reg_send_wavenum;

	if ( (pwm_no <=PWM2)||(pwm_no == PWM6) )
		reg_send_wavenum = PWM_register[pwm_no] + 4 * PWM_SEND_WAVENUM;
	else
		reg_send_wavenum = PWM_register[pwm_no] + 4 * (PWM_SEND_WAVENUM - 2); 

	return INREG32 ( reg_send_wavenum );
}

void mt_set_intr_enable_hal(U32 pwm_intr_enable_bit)
{
	SETREG32 ( PWM_INT_ENABLE, 1 << pwm_intr_enable_bit );
}

S32 mt_get_intr_status_hal(U32 pwm_intr_status_bit)
{
	int ret;
	ret = INREG32 ( PWM_INT_STATUS );
	ret = ( ret >> pwm_intr_status_bit ) & 0x01;
	return ret;
}
void mt_set_intr_ack_hal ( U32 pwm_intr_ack_bit )
{
	SETREG32 ( PWM_INT_ACK, 1 << pwm_intr_ack_bit );
}

void mt_pwm_dump_regs_hal(void)
{
	int i;
	U32 reg_val;
	reg_val = INREG32(PWM_ENABLE);
	PWMMSG("\r\n[PWM_ENABLE is:%x]\n\r ", reg_val );

	for ( i = PWM0;  i <= PWM6; i ++ ) 
	{
		reg_val = INREG32(PWM_register[i] + 4* PWM_CON);
		PWMMSG("\r\n[PWM%d_CON is:%x]\r\n", i+1, reg_val);
		reg_val=INREG32(PWM_register[i]+4* PWM_HDURATION);
		PWMMSG("\r\n[PWM%d_HDURATION is:%x]\r\n",i+1,reg_val);
		reg_val = INREG32(PWM_register[i] + 4* PWM_LDURATION);
		PWMMSG("\r\n[PWM%d_LDURATION is:%x]\r\n", i+1, reg_val);
		reg_val=INREG32(PWM_register[i]+4* PWM_GDURATION);
		PWMMSG("\r\n[PWM%d_HDURATION is:%x]\r\n",i+1,reg_val);
		reg_val = INREG32(PWM_register[i] + 4* PWM_BUF0_BASE_ADDR);
		PWMMSG("\r\n[PWM%d_BUF0_BASE_ADDR is:%x]\r\n", i+1, reg_val);
		reg_val=INREG32(PWM_register[i]+4* PWM_BUF0_SIZE);
		PWMMSG("\r\n[PWM%d_BUF0_SIZE is:%x]\r\n",i+1,reg_val);
		reg_val = INREG32(PWM_register[i] + 4* PWM_BUF1_BASE_ADDR);
		PWMMSG("\r\n[PWM%d_BUF1_BASE_ADDR is:%x]\r\n", i+1, reg_val);
		reg_val=INREG32(PWM_register[i]+4* PWM_BUF1_SIZE);
		PWMMSG("\r\n[PWM%d_BUF1_SIZE is:%x]\r\n",i+1,reg_val);
		reg_val = INREG32(PWM_register[i] + 4* PWM_SEND_DATA0);
		PWMMSG("\r\n[PWM%d_SEND_DATA0 is:%x]\r\n", i+1, reg_val);
		reg_val=INREG32(PWM_register[i]+4* PWM_SEND_DATA1);
		PWMMSG("\r\n[PWM%d_PWM_SEND_DATA1 is:%x]\r\n",i+1,reg_val);
		reg_val = INREG32(PWM_register[i] + 4* PWM_WAVE_NUM);
		PWMMSG("\r\n[PWM%d_WAVE_NUM is:%x]\r\n", i+1, reg_val);
		if ( i <= PWM2||i==PWM6) {
			reg_val=INREG32(PWM_register[i]+4* PWM_DATA_WIDTH);
			PWMMSG("\r\n[PWM%d_WIDTH is:%x]\r\n",i+1,reg_val);
			reg_val=INREG32(PWM_register[i]+4* PWM_THRESH);
			PWMMSG("\r\n[PWM%d_THRESH is:%x]\r\n",i+1,reg_val);
			reg_val=INREG32(PWM_register[i]+4* PWM_SEND_WAVENUM);
			PWMMSG("\r\n[PWM%d_SEND_WAVENUM is:%x]\r\n",i+1,reg_val);
						reg_val=INREG32(PWM_register[i]+4* PWM_VALID);
			PWMMSG("\r\n[PWM%d_SEND_VALID is:%x]\r\n",i+1,reg_val);
		}else {
			reg_val=INREG32(PWM_register[i]+4* (PWM_SEND_WAVENUM-2));
			PWMMSG("\r\n[PWM%d_SEND_WAVENUM is:%x]\r\n",i+1,reg_val);
			reg_val=INREG32(PWM_register[i]+4* (PWM_VALID-2));
			PWMMSG("\r\n[PWM%d_SEND_VALID is:%x]\r\n",i+1,reg_val);
		}
	}		
}

void pwm_debug_store_hal()
{
}

void pwm_debug_show_hal()
{
}

void mt_set_pwm_3dlcm_enable_hal(BOOL enable)
{
}

void mt_set_pwm_3dlcm_inv_hal(U32 pwm_no, BOOL inv)
{
}

void mt_set_pwm_3dlcm_base_hal(U32 pwm_no)
{
}

void mt_pwm_26M_clk_enable_hal (U32 enable)
{
}

