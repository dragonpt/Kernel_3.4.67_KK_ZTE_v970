vcodec_kernel_driver-objs :=      videocodec_kernel_driver.o \
                                            codec/common/val/linux/kernel/val.o 


#obj-$(CONFIG_MTK_MFLEXVIDEO_DRIVER) += vcodec_kernel_driver.o
obj-$(CONFIG_MTK_VIDEOCODEC_DRIVER) += vcodec_kernel_driver.o

#obj-m += vcodec_kernel_driver.o


ccflags-y +=                                    -I$(MTK_PATH_PLATFORM)/drivers/videocodec/inc \
                                                -I$(MTK_ROOT)/kernel/include/linux/vcodec \
                                                -I$(MTK_PATH_PLATFORM)/drivers/videocodec/codec/common/val/linux/kernel \
                                                -I$(MTK_PATH_PLATFORM)/drivers/videocodec/codec/common/hal/inc \
                                                -I$(MTK_PATH_PLATFORM)/drivers/videocodec/codec/common/api/inc \
                                                -I$(MTK_PATH_PLATFORM)/core/include/mach  \
-I$(MTK_PATH_PLATFORM)/../../mt6577/kernel/core/include/mach \
-I$(MTK_PATH_PLATFORM)/../../mt6577/kernel/core/include \
-I$(MTK_PATH_PLATFORM)/../../mt6575/kernel/core/include/mach \
-I$(MTK_PATH_PLATFORM)/../../mt6575/kernel/core/include \
