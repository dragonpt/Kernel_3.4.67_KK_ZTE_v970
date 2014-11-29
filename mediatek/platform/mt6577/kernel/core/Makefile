obj-y := core.o irq.o system.o timer.o mt_devs.o mt_board.o mt_pm_init.o mt_emi_mpu.o mt_dma.o mt_dbg.o
obj-y += mt_boot.o mt_gpt.o mt_eint.o mt_outercache.o mt_ca9_power.o sbchk_base.o mt_freqhopping.o
#obj-y += mt_rfkill.o

obj-$(CONFIG_SMP) += mt-smp.o hotplug.o mt-headsmp.o mtk_hotplug_mechanism.o
#obj-$(CONFIG_SMP) += fiq_smp_call.o

obj-y += mt_emi_bwl.o
obj-y += mt_clock_manager.o mt_dcm.o mt_pm_log.o
obj-y += ca9_power.o
obj-y += mt_smi.o

obj-$(CONFIG_MT65XX_TRACER) += pmu_v7.o mon_interface.o mt_emi_bm.o mt_mon.o
obj-y += innercache.o
obj-y += ple.o
obj-y += dram_overclock.o
obj-y += mt_dramc.o

obj-y += mt_pm_ldo.o mt_pmic_feature_api.o
obj-y += mt_cpufreq.o mt_cpe.o
#obj-y += sec_osal.o
obj-y += mt_sleep.o mt_sc.o
obj-y += mt_dormant.o cpu_dormant.o
#+++ Camera related driver +++
obj-y += camera_isp.o
obj-y += camera_sysram.o
obj-y += camera_eis.o
obj-y += camera_fdvt.o
obj-y += camera_res_mgr.o
#+++ Camera related driver ---
obj-y += mt_pqparam.o
obj-y += mt_mdp.o
obj-y += mt_mdp_kernel_trap.o
obj-y += mtk_ccci_helper.o

obj-y += ca9_speed.o ca9_speed_indicative.o ca9_speed_test.o ca9_speed_neon.o ca9_neon.o ca9_neon_indicative.o ca9_neon_test.o slt_ca9_helpers.o slt_ca9_helpers_c.o


ifneq ($(CONFIG_MTK_CCCI_DEVICES),y)
ccflags-y  += -D__USING_DUMMY_CCCI_API__
endif

ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/power \
             -I$(MTK_PATH_SOURCE)/drivers/wdk

# Hibernation
AFLAGS_swsusp.o	:= -DTEXT_OFFSET=$(TEXT_OFFSET)
obj-$(CONFIG_HIBERNATION)	+= cpu_hibernate.o swsusp.o

ifeq ($(CONFIG_ARCH_MT6577),y)
obj-y += mtk_cpu_management.o
ifeq ($(CONFIG_THERMAL),y)
	obj-y += mtk_ts_cpu.o
	obj-y += mtk_ts_battery.o
	obj-y += mtk_ts_pa.o
	obj-y += mtk_ts_abb.o
	obj-y += mtk_ts_pmic.o
endif
endif

KBUILD_AFLAGS	:=$(KBUILD_AFLAGS:-msoft-float=-Wa,-mfpu=neon)
