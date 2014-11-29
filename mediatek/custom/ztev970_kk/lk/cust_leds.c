//#include <platform/cust_leds.h>
#include <cust_leds.h>
#include <platform/mt_pwm.h>
#include <platform/mt_gpio.h>

extern int DISP_SetBacklight(int level);

#define BACKLIGHT_ENABLE_PIN GPIO68
static unsigned int Cust_SetBacklight(int level)
{
	if(level) {
		mt_set_gpio_mode(BACKLIGHT_ENABLE_PIN, GPIO_MODE_GPIO);
		mt_set_gpio_dir(BACKLIGHT_ENABLE_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(BACKLIGHT_ENABLE_PIN, 1);
	}
	else {
		mt_set_gpio_mode(BACKLIGHT_ENABLE_PIN, GPIO_MODE_GPIO);
		mt_set_gpio_dir(BACKLIGHT_ENABLE_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(BACKLIGHT_ENABLE_PIN, 0);
	}
	return 0;
}


static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK4,{0}},
	{"green",             MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK5,{0}},
	{"blue",              MT65XX_LED_MODE_NONE, -1,{0}},
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1,{0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1,{0}},
	{"button-backlight",  MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_BUTTON,{0}},
	{"lcd-backlight",     MT65XX_LED_MODE_CUST, (int)Cust_SetBacklight,{0}},
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
	return cust_led_list;
}
