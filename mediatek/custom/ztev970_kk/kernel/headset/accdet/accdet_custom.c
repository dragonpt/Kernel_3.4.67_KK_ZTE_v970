#include "accdet_custom_def.h"
#include <accdet_custom.h>

//key press customization: long press time
struct headset_key_custom headset_key_custom_setting = {
	2000
};

struct headset_key_custom* get_headset_key_custom_setting(void)
{
	return &headset_key_custom_setting;
}

#ifdef  ACCDET_MULTI_KEY_FEATURE
static struct headset_mode_settings cust_headset_settings = {
	0x900, 0x200, 1, 0x1f0, 0x800, 0x800, 0x20
};
#else
//headset mode register settings(for MT6575)
static struct headset_mode_settings cust_headset_settings = {
//	0x1900, 0x140, 1, 0x12c, 0x3000, 0x3000, 0x400
0x900, 0x140, 1, 0x12c, 0x3000, 0x3000, 0x400
};
#endif
#ifdef TV_OUT_SUPPORT
//tv mode register settings (for MT6575)
struct tv_mode_settings cust_tv_settings[2]={
        {9, 19, 273, 284, 5, 112, 850, 2, 0x160, 0x20, 0x6}, //NTSC mode
        {9, 19, 320, 334, 5, 112, 850, 2, 0x160, 0x20, 0x6}, //PAL mode
};
struct tv_mode_settings* get_tv_mode_settings(void)
{
	return &cust_tv_settings;
}
#endif
struct headset_mode_settings* get_cust_headset_settings(void)
{
	return &cust_headset_settings;
}

