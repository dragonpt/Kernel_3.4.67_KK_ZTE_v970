ccflags-y +=-I$(MTK_PATH_PLATFORM)/drivers/power/

ifeq ($(MTK_NCP1851_SUPPORT),yes) 
    obj-$(CONFIG_MTK_SMART_BATTERY) += mt_battery_tbl.o mt_fuel_gauge_tbl.o ncp1851.o
endif