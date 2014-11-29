#ifndef __MTK_CCCI_HELPER_H
#define __MTK_CCCI_HELPER_H

#include <mach/eint.h>
#include <mach/mt_gpio.h>
#include <mach/mt_reg_base.h>

//-------------other configure-------------------------//
#define MAX_KERN_API	  (20)
#define MAX_SLEEP_API	  (20)
#define MAX_FILTER_MEMBER (4)


//-------------error code define-----------------------//
#define E_NO_EXIST		  (-1)
#define E_PARAM			  (-2)



//-------------enum define---------------------------//
/*modem image version definitions*/
typedef enum{
	AP_IMG_INVALID = 0,
	AP_IMG_2G,
	AP_IMG_3G
}AP_IMG_TYPE;


typedef enum {
	ID_GET_MD_WAKEUP_SRC = 0,
	ID_CCCI_DORMANCY = 1,
    ID_LOCK_MD_SLEEP = 2,
	ID_ACK_MD_SLEEP = 3,
	ID_SSW_SWITCH_MODE = 4,
	ID_SET_MD_TX_LEVEL = 5,
	ID_GET_TXPOWER = 6,			// For thermal
	ID_IPO_H_RESTORE_CB = 7,
	ID_FORCE_MD_ASSERT,
}KERN_FUNC_ID;

// System channel, AP -->(/ <-->) MD message start from 0x100
enum {
	MD_DORMANT_NOTIFY = 0x100,
	MD_SLP_REQUEST = 0x101,
	MD_TX_POWER = 0x102,
	MD_RF_TEMPERATURE = 0x103,
	MD_RF_TEMPERATURE_3G = 0x104,
	MD_GET_BATTERY_INFO = 0x105,
};

typedef enum {
	 RSM_ID_RESUME_WDT_IRQ = 0,
	 RSM_ID_MD_LOCK_DORMANT = 1,
	 RSM_ID_WAKE_UP_MD = 2,
	 RSM_ID_MAX
}RESUME_ID;
 
typedef enum {
	 SLP_ID_MD_FAST_DROMANT = 0,
	 SLP_ID_MD_UNLOCK_DORMANT = 1,
	 SLP_ID_MAX
}SLEEP_ID;

typedef enum {
	ID_GET_FDD_THERMAL_DATA = 0,
	ID_GET_TDD_THERMAL_DATA,
}SYS_CB_ID;


typedef enum {
	md_type_invalid = 0,
	modem_2g = 1,
	modem_3g,
	modem_wg,
	modem_tg,
}modem_type_t;

//-------------structure define------------------------//
typedef int (*ccci_kern_cb_func_t)(char *buf, unsigned int len);
typedef struct{
	KERN_FUNC_ID id;
	ccci_kern_cb_func_t func;
}ccci_kern_func_info;


typedef int (*ccci_sys_cb_func_t)(int, int);
typedef struct{
	SYS_CB_ID			id;
	ccci_sys_cb_func_t	func;
}ccci_sys_cb_func_info_t;


typedef size_t (*ccci_filter_cb_func_t)(char *, size_t);
typedef struct _cmd_op_map{
	char cmd[8];
	int  cmd_len;
	ccci_filter_cb_func_t store;
	ccci_filter_cb_func_t show;
}cmd_op_map_t;



//-----------------export function declaration----------------------------//
void get_md_post_fix(int md_id, char buf[], char buf_ex[]);
extern modem_type_t get_modem_support(int md_id);
extern int IMM_get_adc_channel_num(char *channel_name, int len);
extern int get_td_eint_info(char * eint_name, unsigned int len);
extern int get_md_gpio_info(char *gpio_name, unsigned int len);
extern int get_md_adc_info(char *adc_name, unsigned int len);

extern void AudSys_Power_On(bool on);

extern int register_ccci_kern_func(unsigned int id, ccci_kern_cb_func_t func);
extern int exec_ccci_kern_func(unsigned int id, char *buf, unsigned int len);
extern int exec_ccci_kern_func_by_md_id(int md_id, unsigned int id, char *buf, unsigned int len);

extern int get_dram_info(int *clk, int *type);
extern int get_dram_type_clk(int *clk, int *type);

extern void register_suspend_notify(unsigned int id, void (*func)(void));
extern void register_resume_notify(unsigned int id, void (*func)(void));

int register_ccci_sys_call_back(int md_id, unsigned int id, ccci_sys_cb_func_t func);
void exec_ccci_sys_call_back(int md_id, int cb_id, int data);


#endif
