include $(MTK_PATH_BUILD)/common.mk
$(call all-modules-src-or-makefile,$(obj),CUSTOM_KERNEL_)
ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/video/  \
           -I$(MTK_PATH_SOURCE)/drivers/video/
ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/videox/  \
           -I$(MTK_PATH_SOURCE)/drivers/videox/
obj-n := dummy.o

ifeq ($(MTK_ALPS_BOX_SUPPORT), yes)
ccflags-y += -DMTK_ALPS_BOX_SUPPORT
endif
