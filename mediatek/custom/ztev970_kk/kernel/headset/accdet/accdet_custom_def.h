// use accdet + EINT solution
#define ACCDET_EINT
#ifdef ACCDET_EINT
#define ACCDET_DELAY_ENABLE_TIME 500 //delay 500ms to enable accdet after EINT 
//#define ACCDET_EINT_HIGH_ACTIVE //defaule low active if not define ACCDET_EINT_HIGH_ACTIVE
#endif

struct tv_mode_settings{
    int start_line0;	//range: 1~19
    int end_line0;	//range: 1~19
    int start_line1;	//range: 263~285(NTSC), 310~336(PAL)
    int end_line1;	//range: 263~285(NTSC), 310~336(PAL)
    int pre_line;
    int start_pixel;	//range: 112~850
    int end_pixel;	//range: 112~850
    int fall_delay;
    int rise_delay;
    int div_rate;	//pwm div in tv-out mode 
    int debounce;	//tv-out debounce
};

#ifdef MTK_TVOUT_SUPPORT
#define TV_OUT_SUPPORT
#endif
