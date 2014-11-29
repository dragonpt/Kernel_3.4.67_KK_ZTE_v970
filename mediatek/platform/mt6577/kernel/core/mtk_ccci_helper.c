#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/kfifo.h>
#include <linux/firmware.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>

#include <mach/mtk_ccci_helper.h>

#include <mach/dfo_boot.h>
#include <mach/dfo_boot_default.h>

#ifndef __USING_DUMMY_CCCI_API__

//#define android_bring_up_prepare 1 //when porting ccci drver for android bring up, enable the macro
ccci_kern_func_info ccci_func_table[MAX_KERN_API];
#if 0
#define MAX_MD_NUM 1
ccci_sys_cb_func_info_t	ccci_sys_cb_table_1000[MAX_MD_NUM][MAX_KERN_API];
ccci_sys_cb_func_info_t	ccci_sys_cb_table_100[MAX_MD_NUM][MAX_KERN_API];
static unsigned char		kern_func_err_num[MAX_MD_NUM][MAX_KERN_API];
#endif
/***************************************************************************
 * Make device node helper function section
 ***************************************************************************/
 #if 0
static void* create_dev_class(struct module *owner, const char *name)
{
	int err = 0;
	struct class *dev_class = class_create(owner, name);
	if(IS_ERR(dev_class)){
		err = PTR_ERR(dev_class);
		printk("create %s class fail, %d\n", name, err);
		return NULL;
	}

	return dev_class;
}

static void release_dev_class(void *dev_class)
{
	if(NULL != dev_class)
		class_destroy(dev_class);
}

static int registry_dev_node(void *dev_class, const char *name, int major_id, int minor_start_id, int index)
{
	int ret=0;
	dev_t dev;
	struct device *devices;

	if(index>=0){
		dev = MKDEV(major_id, minor_start_id) + index;
		devices = device_create( (struct class *)dev_class, NULL, dev, NULL, "%s%d", name, index );
	}else{
		dev = MKDEV(major_id, minor_start_id);
		devices = device_create( (struct class *)dev_class, NULL, dev, NULL, "%s", name );
	}

	if(IS_ERR(devices)){
		ret = PTR_ERR(devices);
		printk("create %s device fail, %d\n", name, ret);
	}
	return ret;
}


static void release_dev_node(void *dev_class, int major_id, int minor_start_id, int index)
{
	device_destroy(dev_class,MKDEV(major_id, minor_start_id) + index);
}

#endif

void get_md_post_fix(int md_id,char buf[], char buf_ex[])
{
	// modem_X_YY_K_[Ex].img
	int		X;
	char	YY_K[8];
	int		Ex = 0;	
	unsigned int	feature_val = 0;

	// X
	X = 1;

	// YY_K
	YY_K[0] = '\0';
	feature_val = get_modem_support(md_id);
	
	switch(feature_val)
	{
		case modem_2g:
			snprintf(YY_K, 8, "_2g_n");
			break;
			
		case modem_3g:
			snprintf(YY_K, 8, "_3g_n");
			break;
			
		case modem_wg:
			snprintf(YY_K, 8, "_wg_n");
			break;
			
		case modem_tg:
			snprintf(YY_K, 8, "_tg_n");
			break;			
			
		default:
			break;
	}

	// [_Ex] Get chip version
	//if(get_chip_version() == CHIP_SW_VER_01)
		Ex = 1;
	//else if(get_chip_version() == CHIP_SW_VER_02)
		//Ex = 2;

	// Gen post fix
	if(buf)
		snprintf(buf, 12, "%d%s", X, YY_K);

	if(buf_ex)
		snprintf(buf_ex, 12, "%d%s_E%d", X, YY_K, Ex);
}
EXPORT_SYMBOL(get_md_post_fix);
static int ccci_get_dfo_setting(void *key, int* value)
{
	int i;
	for (i=0; i<DFO_BOOT_COUNT; i++) 
	{
		if(key && !strcmp(key, dfo_boot_default.name[i]))
		{
			*value = dfo_boot_default.value[i];
			printk("[ccci/ctl] (0)dfo boot default:%s:0x%08X\n", key, *value);
			return 1;
		}
	}
	return 0;
}

modem_type_t get_modem_support(int md_id)
{
	int val=0;
	ccci_get_dfo_setting("MTK_MD1_SUPPORT",&val);
	switch(val)
	{
		case 1:
    return modem_2g;
		case 2:
	return modem_3g;
		case 3:
	return modem_wg;
		case 4:
	return modem_tg;
		default:
	return md_type_invalid;
	}
}
EXPORT_SYMBOL(get_modem_support);


int get_td_eint_info(char * eint_name, unsigned int len)
{
	#if !defined (CONFIG_MT6577_FPGA) && !defined (android_bring_up_prepare)
	return get_td_eint_num(eint_name, len);
	
	#else
	return 0;
	#endif
}
EXPORT_SYMBOL(get_td_eint_info);
	
	
int get_md_gpio_info(char *gpio_name, unsigned int len)
{
	#if !defined (CONFIG_MT6577_FPGA) && !defined (android_bring_up_prepare)
	return mt_get_md_gpio(gpio_name, len);
	
	#else
	return 0;
	#endif
}
EXPORT_SYMBOL(get_md_gpio_info);
	

int get_md_adc_info(char *adc_name, unsigned int len)
{
	#if !defined (CONFIG_MT6577_FPGA) && !defined (android_bring_up_prepare)
	
	return IMM_get_adc_channel_num(adc_name, len);
	
	#else
	return 0;
	#endif
}
EXPORT_SYMBOL(get_md_adc_info);
	
	
	
void AudSys_Power_On(bool on)
{
	#if !defined(CONFIG_MTK_LDVT) && !defined(CONFIG_MT6577_FPGA)
	
	#ifndef android_bring_up_prepare
		extern void CCCI_AudDrv_Clk_On(void);
		extern void CCCI_AudDrv_Clk_Off(void);
	
	if(on)
		return CCCI_AudDrv_Clk_On();
	else
		return CCCI_AudDrv_Clk_Off();
	
	#endif
	#endif

	return;
	
}
EXPORT_SYMBOL(AudSys_Power_On);


int get_dram_type_clk(int *clk, int *type)
{
	return get_dram_info(clk, type);
}
EXPORT_SYMBOL(get_dram_type_clk);


/***************************************************************************
 * Make sysfs node helper function section
 ***************************************************************************/
ssize_t mtk_ccci_attr_show(struct kobject *kobj, struct attribute *attr, char *buffer);
ssize_t mtk_ccci_attr_store(struct kobject *kobj, struct attribute *attr, const char *buffer, size_t size);
ssize_t mtk_ccci_filter_show(struct kobject *kobj, char *page);
ssize_t mtk_ccci_filter_store(struct kobject *kobj, const char *page, size_t size);

struct sysfs_ops mtk_ccci_sysfs_ops = {
	.show   = mtk_ccci_attr_show,
	.store  = mtk_ccci_attr_store,
};

struct mtk_ccci_sys_entry {
	struct attribute attr;
	ssize_t (*show)(struct kobject *kobj, char *page);
	ssize_t (*store)(struct kobject *kobj, const char *page, size_t size);
};

static struct mtk_ccci_sys_entry filter_setting_entry = {
	{ .name = "filter",  .mode = S_IRUGO | S_IWUSR },
	mtk_ccci_filter_show,
	mtk_ccci_filter_store,
};

struct attribute *mtk_ccci_attributes[] = {
	&filter_setting_entry.attr,
	NULL,
};

struct kobj_type mtk_ccci_ktype = {
	.sysfs_ops = &mtk_ccci_sysfs_ops,
	.default_attrs = mtk_ccci_attributes,
};

static struct mtk_ccci_sysobj {
	struct kobject kobj;
} ccci_sysobj;


static int mtk_ccci_sysfs(void) 
{
	struct mtk_ccci_sysobj *obj = &ccci_sysobj;

	memset(&obj->kobj, 0x00, sizeof(obj->kobj));

	obj->kobj.parent = kernel_kobj;
	if (kobject_init_and_add(&obj->kobj, &mtk_ccci_ktype, NULL, "ccci")) {
		kobject_put(&obj->kobj);
		return -ENOMEM;
	}
	kobject_uevent(&obj->kobj, KOBJ_ADD);

	return 0;
}

ssize_t mtk_ccci_attr_show(struct kobject *kobj, struct attribute *attr, char *buffer) 
{
	struct mtk_ccci_sys_entry *entry = container_of(attr, struct mtk_ccci_sys_entry, attr);
	return entry->show(kobj, buffer);
}

ssize_t mtk_ccci_attr_store(struct kobject *kobj, struct attribute *attr, const char *buffer, size_t size) 
{
	struct mtk_ccci_sys_entry *entry = container_of(attr, struct mtk_ccci_sys_entry, attr);
	return entry->store(kobj, buffer, size);
}

//----------------------------------------------------------//
// Filter table                                                         
//----------------------------------------------------------//
cmd_op_map_t cmd_map_table[MAX_FILTER_MEMBER] = {{"",0}, {"",0}, {"",0}, {"",0}};

ssize_t mtk_ccci_filter_show(struct kobject *kobj, char *buffer) 
{
	int i;
	int remain = PAGE_SIZE;
	unsigned long len;
	char *ptr = buffer;

	for(i=0; i<MAX_FILTER_MEMBER; i++){
		if( cmd_map_table[i].cmd_len !=0 ){
			// Find a mapped cmd
			if(NULL != cmd_map_table[i].show){
				len = cmd_map_table[i].show(ptr, remain);
				ptr += len;
        			remain -= len;
        		}
		}
	}

	return (PAGE_SIZE-remain);
}

ssize_t mtk_ccci_filter_store(struct kobject *kobj, const char *buffer, size_t size) 
{
	int i;

	for(i=0; i<MAX_FILTER_MEMBER; i++){
		if( strncmp(buffer, cmd_map_table[i].cmd, cmd_map_table[i].cmd_len)==0 ){
			// Find a mapped cmd
			if(NULL != cmd_map_table[i].store){
				return cmd_map_table[i].store((char*)buffer, size);
			}
		}
	}
	printk("unsupport cmd\n");
	return size;
}

int register_filter_func(char cmd[], ccci_filter_cb_func_t store, ccci_filter_cb_func_t show)
{
	int i;
	int empty_slot = -1;
	int cmd_len;
	
	for(i=0; i<MAX_FILTER_MEMBER; i++){
		if( 0 == cmd_map_table[i].cmd_len ){
			// Find a empty slot
			if(-1 == empty_slot)
				empty_slot = i;
		}else if( strcmp(cmd, cmd_map_table[i].cmd)==0 ){
			// Find a duplicate cmd
			return -1;
		}
	}
	if( -1 != empty_slot){
		cmd_len = strlen(cmd);
		if(cmd_len > 7){
			return -2;
		}

		cmd_map_table[empty_slot].cmd_len = cmd_len;
		for(i=0; i<cmd_len; i++)
			cmd_map_table[empty_slot].cmd[i] = cmd[i];
		cmd_map_table[empty_slot].cmd[i] = 0; // termio char
		cmd_map_table[empty_slot].store = store;
		cmd_map_table[empty_slot].show = show;
		return 0;
	}
	return -3;
}
EXPORT_SYMBOL(register_filter_func);



/***************************************************************************/
/* Register kernel API for ccci driver invoking                                                           
/ **************************************************************************/
int register_ccci_kern_func(unsigned int id, ccci_kern_cb_func_t func)
{
	int ret = 0;
	
	if((id >= MAX_KERN_API) || (func == NULL)) {
		printk("ccci/ctl: invalid parameter(%d)!", id);
		return E_PARAM;
	}
	
	if(ccci_func_table[id].func == NULL) {
		ccci_func_table[id].id = id;
		ccci_func_table[id].func = func;
	}
	else
		printk("ccci/ctl: func%d has registered!", id);

	return ret;
}


EXPORT_SYMBOL(register_ccci_kern_func);

int exec_ccci_kern_func(unsigned int id, char *buf, unsigned int len)
{
	ccci_kern_cb_func_t func;
	int ret = 0;
	
	if(id >= MAX_KERN_API) {
		printk("ccci/ctl: invalid function id(%d)!", id);
		return E_PARAM;
	}
	
	func = ccci_func_table[id].func;
	if(func != NULL) {
		ret = func(buf, len);
	}
	else {
		ret = E_NO_EXIST;
		printk("ccci/ctl: func%d not register!", id);
	}

	return ret;
}
EXPORT_SYMBOL(exec_ccci_kern_func);
int exec_ccci_kern_func_by_md_id(int md_id, unsigned int id, char *buf, unsigned int len)
{   
    printk("exec_ccci_kern_func_by_md_id:%d!", id);
	return exec_ccci_kern_func(id,buf,len);
}
EXPORT_SYMBOL(exec_ccci_kern_func_by_md_id);

int register_ccci_sys_call_back(int md_id, unsigned int id, ccci_sys_cb_func_t func)
{
#if 0
	int ret = 0;
	ccci_sys_cb_func_info_t *info_ptr;
	
	if( md_id >= MAX_MD_NUM ) {
		printk("[ccci/ctl] (0)register_sys_call_back fail: invalid md id(%d)\n", md_id+1);
		return E_PARAM;
	}

	if((id >= 0x100)&&((id-0x100) < MAX_KERN_API)) {
		info_ptr = &(ccci_sys_cb_table_100[md_id][id-0x100]);
	} else if((id >= 0x1000)&&((id-0x1000) < MAX_KERN_API)) {
		info_ptr = &(ccci_sys_cb_table_1000[md_id][id-0x1000]);
	} else {
		printk("[ccci/ctl] (%d)register_sys_call_back fail: invalid func id(0x%x)\n", md_id+1, id);
		return E_PARAM;
	}
	
	if(info_ptr->func == NULL) {
		info_ptr->id = id;
		info_ptr->func = func;
	}
	else
		printk("[ccci/ctl] (%d)register_sys_call_back fail: func(0x%x) registered!\n", md_id+1, id);
	return ret;
#endif
    return 0;
}
EXPORT_SYMBOL(register_ccci_sys_call_back);


void exec_ccci_sys_call_back(int md_id, int cb_id, int data)
{
#if 0
	ccci_sys_cb_func_t func;
	int	id;
	ccci_sys_cb_func_info_t	*curr_table;
	
	if(md_id >= MAX_MD_NUM) {
		printk("[ccci/ctl] (0)exec_sys_cb fail: invalid md id(%d) \n", md_id+1);
		return;
	}

	id = cb_id & 0xFF;
	if(id >= MAX_KERN_API) {
		printk("[ccci/ctl] (%d)exec_sys_cb fail: invalid func id(0x%x)\n",  md_id+1, cb_id);
		return;
	}

	if ((cb_id & (0x1000|0x100))==0x1000) {
		curr_table = ccci_sys_cb_table_1000[md_id];
	} else if ((cb_id & (0x1000|0x100))==0x100) {
		curr_table = ccci_sys_cb_table_100[md_id];
	} else {
		printk("[ccci/ctl] (%d)exec_sys_cb fail: invalid func id(0x%x)\n",  md_id+1, cb_id);
		return;
	}
	
	func = curr_table[id].func;
	if(func != NULL) {
		func(md_id, data);
	} else {
		printk("[ccci/ctl] (%d)exec_sys_cb fail: func id(0x%x) not register!\n", md_id+1, cb_id);
	}
#endif
}
EXPORT_SYMBOL(exec_ccci_sys_call_back);

static void exec_ccci_dormancy(void)
{
	#ifdef MTK_FD_SUPPORT	
	exec_ccci_kern_func(ID_CCCI_DORMANCY, NULL, 0);
	#endif
}

/*-------------------------------------------------------------------------------------------------
 * Device interface for ccci helper
 *-------------------------------------------------------------------------------------------------
 */
typedef void (*suspend_cb_func_t)(void);
typedef void (*resume_cb_func_t)(void);
	
static suspend_cb_func_t suspend_cb_table[MAX_SLEEP_API];
static resume_cb_func_t resume_cb_table[MAX_SLEEP_API];
	
void register_suspend_notify(unsigned int id, void (*func)(void))
{
	if((id >= MAX_SLEEP_API) || (func == NULL)) {
		printk("ccci/ctl: invalid suspend parameter(%d)!", id);
	}
	
	if (suspend_cb_table[id] == NULL)
		suspend_cb_table[id] = func;
}
EXPORT_SYMBOL(register_suspend_notify);
	
void register_resume_notify(unsigned int id, void (*func)(void))
{
	if((id >= MAX_SLEEP_API) || (func == NULL)) {
		printk("ccci/ctl: invalid resume parameter(%d)!", id);
	}
	
	if (resume_cb_table[id] == NULL)
		resume_cb_table[id] = func;
}
EXPORT_SYMBOL(register_resume_notify);


static int ccci_helper_probe(struct platform_device *dev)
{
	
	printk( "\nccci_helper_probe\n" );
	return 0;
}

static int ccci_helper_remove(struct platform_device *dev)
{
	printk( "\nccci_helper_remove\n" );
	return 0;
}

static void ccci_helper_shutdown(struct platform_device *dev)
{
	printk( "\nccci_helper_shutdown\n" );
}

static int ccci_helper_suspend(struct platform_device *dev, pm_message_t state)
{
	int i;
	suspend_cb_func_t func;

	printk( "\nccci_helper_suspend\n" );
	
	for (i = 0; i < SLP_ID_MAX; i++) {
		func = suspend_cb_table[i];
		if(func != NULL)
			func();
	}
	
	return 0;
}

static int ccci_helper_resume(struct platform_device *dev)
{
	int i;
	resume_cb_func_t func;

	printk( "\nccci_helper_resume\n" );
	
	for (i = 0; i < RSM_ID_MAX; i++) {
		func = resume_cb_table[i];
		if(func != NULL)
			func();
	}
	
	return 0;
}

static struct platform_driver ccci_helper_driver =
{
	.driver     = {
		.name	= "ccci-helper",
	},
	.probe		= ccci_helper_probe,
	.remove		= ccci_helper_remove,
	.shutdown	= ccci_helper_shutdown,
	.suspend	= ccci_helper_suspend,
	.resume		= ccci_helper_resume,
};

struct platform_device ccci_helper_device = {
	.name		= "ccci-helper",
	.id		= 0,
	.dev		= {}
};

static int __init ccci_helper_init(void)
{
	int ret;

	// Init ccci helper sys fs
	memset( (void*)cmd_map_table, 0, sizeof(cmd_map_table) );
	mtk_ccci_sysfs();

	// init ccci kernel API register table
	memset((void*)ccci_func_table, 0, sizeof(ccci_func_table));
	
 	// init ccci system channel call back function register table
	//memset((void*)ccci_sys_cb_table_100, 0, sizeof(ccci_sys_cb_table_100));
	//memset((void*)ccci_sys_cb_table_1000, 0, sizeof(ccci_sys_cb_table_1000));
	//memset((void*)kern_func_err_num, 0, sizeof(kern_func_err_num));

	ret = platform_device_register(&ccci_helper_device);
	if (ret) {
		printk("ccci_helper_device register fail(%d)\n", ret);
		return ret;
	}

	ret = platform_driver_register(&ccci_helper_driver);
	if (ret) {
		printk("ccci_helper_driver register fail(%d)\n", ret);
		return ret;
	}

	register_suspend_notify(SLP_ID_MD_FAST_DROMANT, exec_ccci_dormancy);

	return 0;
}

module_init(ccci_helper_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTK");
MODULE_DESCRIPTION("The ccci helper function");

#else
unsigned int modem_size_list[1] = {0};
unsigned int get_nr_modem(void)
{
    return 0;
}

unsigned int *get_modem_size_list(void)
{
    return modem_size_list;
}
int parse_ccci_dfo_setting(void *dfo_tbl, int num)
{
	return 0;
}

int parse_meta_md_setting(unsigned char args[])
{
	return 0;
}

unsigned int get_modem_is_enabled(int md_id)
{
	return 0;
}

unsigned int get_modem_support(int md_id)
{
	return 0;
}

int register_ccci_kern_func(unsigned int id, ccci_kern_cb_func_t func)
{
	return -1;
}

int register_ccci_kern_func_by_md_id(int md_id, unsigned int id, ccci_kern_cb_func_t func)
{
	return -1;
}

int exec_ccci_kern_func(unsigned int id, char *buf, unsigned int len)
{
	return -1;
}

int exec_ccci_kern_func_by_md_id(int md_id, unsigned int id, char *buf, unsigned int len)
{
	return -1;
}

int register_ccci_sys_call_back(int md_id, unsigned int id, ccci_sys_cb_func_t func)
{
	return -1;
}

void exec_ccci_sys_call_back(int md_id, int cb_id, int data)
{
}

void ccci_helper_exit(void)
{
}


#endif

