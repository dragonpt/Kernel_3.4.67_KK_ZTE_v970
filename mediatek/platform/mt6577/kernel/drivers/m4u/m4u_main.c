#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
//#include <linux/gfp.h>
#include <linux/aee.h>
#include <linux/timer.h>
#include <linux/disp_assert_layer.h>
#include <linux/xlog.h>
#include <linux/fs.h>

//Arch dependent files
#include <asm/mach/map.h>
#include <mach/sync_write.h>
#include <mach/mt_irq.h>
#include <mach/mt_clock_manager.h>
#include <mach/irqs.h>
#include <mach/mt_boot.h>
// #include <asm/tcm.h>
#include <asm/cacheflush.h>
#include <asm/system.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/proc_fs.h>

#include <mach/m4u.h>
#include "m4u_reg.h"
#include "m4u_dbg.h"

#define MTK_RESOLUTION_CHECK 
#ifdef MTK_RESOLUTION_CHECK
#include <linux/pm.h>
#endif

#include <linux/m4u_profile.h>

//--------------------------------------------Define-----------------------------------------------//

#ifdef MTK_RESOLUTION_CHECK
#define RES_HVGA                             (0x01E00140)
#define RANDOM_CRASH_CNT                     (20)
#define RANDOM_RESET_CNT                     (8000)

static unsigned int lcd_check_cnt = 0;
#endif


//-------------------------------------------------------------------------------------------
//  debug  macros
//-------------------------------------------------------------------------------------------
// for debug msg, low priority
#define M4U_ASSERT(x) if(!(x)){xlog_printk(ANDROID_LOG_ERROR, "M4U_K", "assert fail, file:%s, line:%d", __FILE__, __LINE__);}

//#define MTK_M4U_DBG
#ifdef MTK_M4U_DBG
#define M4UDBG(string, args...)	xlog_printk(ANDROID_LOG_INFO, "M4U_K", "[pid=%d]"string,current->tgid,##args);

bool gM4uLogFlag = false;
#define M4ULOG(string, args...) xlog_printk(ANDROID_LOG_INFO, "M4U_K", "[pid=%d] "string,current->tgid,##args)

#else

#define M4UDBG(string, args...)

bool gM4uLogFlag = false;
#define M4ULOG(string, args...) do { \
	if(gM4uLogFlag){ \
	xlog_printk(ANDROID_LOG_INFO, "M4U_K", "[pid=%d] "string,current->tgid,##args); } \
}while(0)
#endif

// for error msg, high priority
#define MTK_M4U_MSG
#ifdef MTK_M4U_MSG
#define M4UMSG(string, args...)	xlog_printk(ANDROID_LOG_ERROR, "M4U_K", "[pid=%d]"string,current->tgid,##args);

#else

#define M4UMSG(string, args...)
#endif

// serious error, will cause red screen and save log in sdcard/aee_exp/db.xx
unsigned int print_detail_en = 0;

//#ifndef CONFIG_MTK_LDVT  // LDVT load dose not support DAL_xxx func
#if 0
#define M4UERR(string, args...) do {                           \
	printk("[M4U_K] error_assert_fail: "string,##args);          \
  aee_kernel_exception("M4U", "[M4U_K] error:"string,##args);  \
  if(print_detail_en==1)                                       \
  {                                                            \
       DAL_Printf(string,##args);                              \
  		 print_detail_en = 0;                                    \
  }                                                            \
}while(0)

#define M4UWARN(string, args...) do {                           \
	printk("[M4U_K] warning: "string,##args);          \
    aee_kernel_warning("M4U", "[M4U_K] error:"string,##args);  \
    if(print_detail_en==1)                                       \
    {                                                            \
       DAL_Printf(string,##args);                              \
    		 print_detail_en = 0;                                    \
    }                                                            \
}while(0)

#else

#define M4UERR(string, args...) do {\
	xlog_printk(ANDROID_LOG_ERROR, "M4U_K", "[pid=%d]error_assert_fail: "string,current->tgid,##args);  \
	aee_kernel_exception("M4U", "[M4U_K] error:"string,##args);  \
}while(0)

#define M4UWARN(string, args...) do {\
	xlog_printk(ANDROID_LOG_ERROR, "M4U_K", "[pid=%d]error_assert_fail: "string,current->tgid,##args);  \
	aee_kernel_warning("M4U", "[M4U_K] error:"string,##args);  \
}while(0)



#endif

char name[100];
#define M4UERR_WITH_OWNER(string, who, args...) do {                           \
	xlog_printk(ANDROID_LOG_ERROR, "M4U_K", "[pid=%d]error_assert_fail: "string,current->tgid,##args);  \
	sprintf(name, "[M4U] owner:%s",who); \
  aee_kernel_warning(name, "[M4U_K] error:"string,##args);  \
}while(0)


// for performance test
//#define MTK_M4U_INF
#ifdef MTK_M4U_INF
#define M4UINF(string, args...)	xlog_printk(ANDROID_LOG_VERBOSE, "M4U_K", "[pid=%d]"string,current->tgid,##args);
#else
#define M4UINF(string, args...)
#endif


#define PFNMAP_FLAG_SET 0x00555555

//------------------------------------Defines & Data for alloc mva-------------
//----------------------------------------------------------------------
/// macros to handle M4u Page Table processing
// #define M4U_ENABLE_MVA_REF_COUNT

#define M4U_MVA_MAX 0x3fffffff   // 1G 
#define M4U_PAGE_MASK 0xfff
#define M4U_PAGE_SIZE   0x1000 //4KB
#define DEFAULT_PAGE_SIZE   0x1000 //4KB
#define M4U_PTE_MAX (M4U_GET_PTE_OFST_TO_PT_SA(TOTAL_MVA_RANGE-1))
#define mva_pteAddr(mva) ((unsigned int *)pPageTableVA_nonCache+((mva) >> 12))
#define mva_pageOffset(mva) ((mva)&0xfff)

#define MVA_BLOCK_SIZE_ORDER     18     //256K
#define MVA_MAX_BLOCK_NR        4095    //1GB

#define MVA_BLOCK_SIZE      (1<<MVA_BLOCK_SIZE_ORDER)  //0x40000 
#define MVA_BLOCK_ALIGN_MASK (MVA_BLOCK_SIZE-1)        //0x3ffff
#define MVA_BLOCK_NR_MASK   (MVA_MAX_BLOCK_NR)      //0xfff
#define MVA_BUSY_MASK       (1<<15)                 //0x8000

#define MVA_IS_BUSY(index) ((mvaGraph[index]&MVA_BUSY_MASK)!=0)
#define MVA_SET_BUSY(index) (mvaGraph[index] |= MVA_BUSY_MASK)
#define MVA_SET_FREE(index) (mvaGraph[index] & (~MVA_BUSY_MASK))
#define MVA_GET_NR(index)   (mvaGraph[index] & MVA_BLOCK_NR_MASK)

#define MVAGRAPH_INDEX(mva) (mva>>MVA_BLOCK_SIZE_ORDER)



static short mvaGraph[MVA_MAX_BLOCK_NR+1];
static unsigned char moduleGraph[MVA_MAX_BLOCK_NR+1];
static DEFINE_SPINLOCK(gMvaGraph_lock);

static void m4u_mvaGraph_init(void);
void m4u_mvaGraph_dump_raw(void);
void m4u_mvaGraph_dump(void);

int m4u_alloc_mva_tsk(M4U_MODULE_ID_ENUM eModuleID,
                                  struct task_struct *tsk,
								  const unsigned int BufAddr, 
								  const unsigned int BufSize, 
								  unsigned int *pRetMVABuf);

static int m4u_alloc_mva_dynamic(M4U_MODULE_ID_ENUM eModuleID,
                                  struct task_struct * tsk,
								  const unsigned int BufAddr, 
								  const unsigned int BufSize, 
								  unsigned int *pRetMVABuf);
static int m4u_dealloc_mva_dynamic(M4U_MODULE_ID_ENUM eModuleID, 
									const unsigned int BufAddr, 
									const unsigned int BufSize,
									unsigned int mvaRegionAddr);
static unsigned int m4u_do_mva_alloc(M4U_MODULE_ID_ENUM eModuleID, 
								  const unsigned int BufAddr, 
								  const unsigned int BufSize);
static int m4u_do_mva_free(M4U_MODULE_ID_ENUM eModuleID, 
                                const unsigned int BufAddr,
								const unsigned int BufSize,
								unsigned int mvaRegionStart) ;
static M4U_MODULE_ID_ENUM mva2module(unsigned int mva);
int m4u_invalid_tlb_range_by_mva(int m4u_index, unsigned int MVAStart, unsigned int MVAEnd);
int m4u_dump_pagetable(M4U_MODULE_ID_ENUM eModuleID);
int m4u_confirm_range_invalidated(int m4u_index, unsigned int MVAStart, unsigned int MVAEnd);

static bool m4u_struct_init(void);
static bool m4u_hw_init(void);
static bool m4u_hw_reinit(void);                            
                                    
static int m4u_get_pages(M4U_MODULE_ID_ENUM eModuleID,
                            struct task_struct * tsk,
                            unsigned int BufAddr, unsigned int BufSize, 
                            unsigned int* pPageTableAddr);
                               
static int m4u_release_pages(M4U_MODULE_ID_ENUM eModuleID,
                    unsigned int BufAddr, 
                    unsigned int BufSize,
                    unsigned int MVA);

static M4U_DMA_DIR_ENUM m4u_get_dir_by_module(M4U_MODULE_ID_ENUM eModuleID);
static void m4u_clear_intr(unsigned int m4u_base);
static int m4u_get_index_by_module(M4U_MODULE_ID_ENUM moduleID);
static int m4u_get_index_by_port(M4U_PORT_ID_ENUM portID);
static M4U_MODULE_ID_ENUM m4u_get_module_by_index(unsigned int index);
static M4U_PORT_ID_ENUM m4u_get_port_by_index(unsigned int index);
static int m4u_name_init(void);
static void m4u_memory_usage(bool bPrintAll);
static void m4u_print_active_port(unsigned int m4u_index);
static M4U_MODULE_ID_ENUM m4u_get_module_by_MVA(unsigned int MVA);
static M4U_MODULE_ID_ENUM m4u_get_module_by_port(M4U_PORT_ID_ENUM portID);
static char* m4u_get_port_name(M4U_PORT_ID_ENUM portID);
char* m4u_get_module_name(M4U_MODULE_ID_ENUM moduleID);
static void m4u_get_performance(unsigned long data);
int m4u_log_on(void);
int m4u_log_off(void);
void m4u_get_power_status(void);
unsigned int m4u_get_pa_by_mva(unsigned int mva);
int m4u_dump_user_addr_register(unsigned int m4u_index);
int m4u_debug_command(unsigned int command);
static int m4u_add_to_garbage_list(struct file * a_pstFile,
                                          struct task_struct *tsk,
                                          unsigned int mvaStart, 
                                          unsigned int bufSize,
                                          M4U_MODULE_ID_ENUM eModuleID,
                                          unsigned int va,
                                          unsigned int flags);
static int m4u_delete_from_garbage_list(M4U_MOUDLE_STRUCT* p_m4u_module, struct file * a_pstFile, struct task_struct **ptsk);
M4U_PORT_ID_ENUM m4u_get_error_port(unsigned int m4u_index, unsigned int mva);
int m4u_dump_mva_info(void);
int m4u_get_write_mode_by_module(M4U_MODULE_ID_ENUM moduleID);
void m4u_dump_pagetable_range(unsigned int vaStart, unsigned int nr);
int m4u_get_descriptor(unsigned int m4u_base, M4U_DESC_TLB_SELECT_ENUM tlbSelect, unsigned int tlbIndex);
int m4u_dump_main_tlb(int index);
void m4u_print_mva_list(struct file *filep, const char *pMsg);
int m4u_dma_cache_flush_all(void);
void mlock_vma_page(struct page *page);
void munlock_vma_page(struct page *page);
void m4u_profile_dump_reg(void);
int m4u_dump_mva_global_list_nolock(void);
int m4u_dump_mva_global_list_lock(void);



//#define MT6573M4U_MVA_ALLOC_DEBUG
#ifdef MT6573M4U_MVA_ALLOC_DEBUG
#define M4U_MVA_DBUG(string, args...) printk("[M4U_K][MVA]"string,##args)
#define M4U_mvaGraph_dump_DBG() m4u_mvaGraph_dump()
#else
#define M4U_MVA_DBUG(string, args...)
#define M4U_mvaGraph_dump_DBG() 
#endif

//-------------------------------------Global variables------------------------------------------------//
#if defined(MTK_G2D_ENABLE_M4U)
extern unsigned int _m4u_g2d_init(void);
#endif

#define MAX_BUF_SIZE_TO_GET_USER_PAGE (200*1024*1024)  //200MB at most for single time alloc
#define TEMP_BUF_SIZE_FOR_GET_USER_PAGE (MAX_BUF_SIZE_TO_GET_USER_PAGE/4096*sizeof(unsigned int))


extern unsigned char *pMlock_cnt;
extern unsigned int mlock_cnt_size;
// record memory usage
int* pmodule_max_size=NULL;
int* pmodule_current_size=NULL;
int* pmodule_locked_pages=NULL;
int power_module_cnt[M4U_CLIENT_MODULE_NUM];
int power_total;

unsigned int gM4UBaseAddr[TOTAL_M4U_NUM] = {M4UMACRO0001, M4UMACRO0002, M4UMACRO0003, M4UMACRO0004};
unsigned int g4M4UTagCount[TOTAL_M4U_NUM] = {32, 32, 16, 16};
unsigned int g4M4UWrapCount[TOTAL_M4U_NUM] = {4, 4, 3, 3};
unsigned int g4M4UPortCount[TOTAL_M4U_NUM] = {14, 13, 6, 3};
unsigned int g4M4UWrapOffset[TOTAL_M4U_NUM] = {0, 4, 8, 11};



//unsigned int *pPageTableVA;  //Page Table virtual Address
unsigned int PageTablePA;    //Page Table Physical Address, 64K align
unsigned int *pPageTableVA_nonCache;


#define TF_PROTECT_BUFFER_SIZE 128
//unsigned int *pProtectVA = NULL;  
unsigned int ProtectPA = 0;
unsigned int *pProtectVA_nonCache = NULL; 

#define M4U_REG_SIZE 0x400
#define BACKUP_REG_SIZE (M4U_REG_SIZE*TOTAL_M4U_NUM)
unsigned int* pM4URegBackUp = 0;

static M4U_RANGE_DES_T *pRangeDes = NULL;
static M4U_WRAP_DES_T *pWrapDes = 0;
#define RANGE_DES_ADDR 0x11

static volatile int FreeRTRegs[TOTAL_M4U_NUM] = {4, 4, 4, 4};
static volatile int FreeSEQRegs[TOTAL_M4U_NUM] = {4, 4, 4, 4};
static volatile int FreeWrapRegs[TOTAL_M4U_NUM] = {4, 4, 3, 3};

spinlock_t gM4uLock;
static DEFINE_MUTEX(gM4uMutex);
static DEFINE_MUTEX(gM4uMutexPower);
//mutex_lock(&gM4uMutex);
//mutex_unlock(&gM4uMutex);

#define MTK_M4U_DEV_MAJOR_NUMBER 188
static struct cdev * g_pMTKM4U_CharDrv = NULL;
static dev_t g_MTKM4Udevno = MKDEV(MTK_M4U_DEV_MAJOR_NUMBER,0);
#define M4U_DEVNAME "M4U_device"

// memory layout related API, defined in source/kernel
extern unsigned long m4u_virt_to_phys(const void *v);
extern struct page* m4u_pfn_to_page(unsigned int pfn);
extern unsigned int m4u_page_to_phys(struct page* pPage);
extern struct page* m4u_phys_to_page(unsigned int phys);
extern unsigned int m4u_page_to_pfn(struct page* pPage);
extern void init_mlock_cnt(void);

extern unsigned int m4u_user_v2p(unsigned int va);

extern int is_pmem_range(unsigned long* base, unsigned long size);
extern int m4u_get_user_pages(int eModuleID, struct task_struct *tsk, struct mm_struct *mm, unsigned long start, int nr_pages, int write, int force, struct page **pages, struct vm_area_struct **vmas);
//extern void inner_dcache_flush_all(void);
extern void smp_inner_dcache_flush_all(void);


struct timer_list perf_timer;

bool gIsSuspend = false;

// garbage collect related
#define MVA_REGION_FLAG_NONE 0x0
#define MVA_REGION_HAS_TLB_RANGE 0x1
#define MVA_REGION_REGISTER    0x2

// list element, each element record mva's size, start addr info
// if user process dose not call mva_alloc() and mva_dealloc() in pair
// we will help to call mva_dealloc() according to elements' info
typedef struct
{
    struct list_head link;
    struct list_head global_link;
    unsigned int bufAddr;
    unsigned int mvaStart;
    unsigned int size;
    M4U_MODULE_ID_ENUM eModuleId;
    unsigned int flags;    
    struct task_struct *tsk; 
    int tgid;
} garbage_list_t;


// per-file-handler structure, allocated in M4U_Open, used to 
// record calling of mva_alloc() and mva_dealloc()    
typedef struct
{
    struct mutex dataMutex;
    pid_t open_pid;
    pid_t open_tgid;
    unsigned int OwnResource;
    struct list_head mvaList;
    int isM4uDrvConstruct;
    int isM4uDrvDeconstruct;
} garbage_node_t;

static garbage_node_t mva_global_link;


static int m4u_delete_from_global_mva_list(garbage_list_t *pList);

/**********
** mtk80347, 20110625
** seldom issue, LCD_UI's pagetable will be corrupted (from EMI_MPU, corrupted by CPU, but
** we can not locate the caller by CodeViser)
** Corruption only happen once, so we save the pagetable content and 
** restore pagetable after find corruption happened
**********/

unsigned int gModuleMaxMVASize[M4U_CLIENT_MODULE_NUM] = {
    M4U_CLNTMOD_SZ_DEFECT  ,
    M4U_CLNTMOD_SZ_CAM     ,
    M4U_CLNTMOD_SZ_PCA     ,
    M4U_CLNTMOD_SZ_JPEG_DEC,
    M4U_CLNTMOD_SZ_JPEG_ENC,
    M4U_CLNTMOD_SZ_VDO_ROT0    ,
    M4U_CLNTMOD_SZ_RGB_ROT0    ,
    M4U_CLNTMOD_SZ_TVROT   ,
    M4U_CLNTMOD_SZ_RDMA0   ,
    M4U_CLNTMOD_SZ_FD      ,
    M4U_CLNTMOD_SZ_FD_INPUT,
    M4U_CLNTMOD_SZ_RGB_ROT1    ,
    M4U_CLNTMOD_SZ_VDO_ROT1    ,
    M4U_CLNTMOD_SZ_RGB_ROT2    ,
    M4U_CLNTMOD_SZ_OVL     ,
    M4U_CLNTMOD_SZ_LCDC    ,
    M4U_CLNTMOD_SZ_LCDC_UI ,
    M4U_CLNTMOD_SZ_RDMA1   ,
    M4U_CLNTMOD_SZ_TVC     ,
    M4U_CLNTMOD_SZ_SPI     ,
    M4U_CLNTMOD_SZ_DPI     ,
    M4U_CLNTMOD_SZ_VDEC_DMA,
    M4U_CLNTMOD_SZ_VENC_DMA,
    M4U_CLNTMOD_SZ_G2D     ,
    M4U_CLNTMOD_SZ_AUDIO   ,
    M4U_CLNTMOD_SZ_RDMA_GENERAL,
    M4U_CLNTMOD_SZ_ROTDMA_GENERAL,    
    M4U_CLNTMOD_SZ_RESERVED };


typedef enum
{
	M4U_TEST_LEVEL_USER = 0,  // performance best, least verification
	M4U_TEST_LEVEL_ENG = 1,   // SQC used, more M4UMSG and M4UERR
	M4U_TEST_LEVEL_STRESS= 2  // stricker verification ,may use M4UERR instead M4UMSG sometimes, used for our own internal test
} M4U_TEST_LEVEL_ENUM;
M4U_TEST_LEVEL_ENUM gTestLevel = M4U_TEST_LEVEL_ENG;    

                                
// call power_on(), power_off() frequently will cause performance drop
// so we have to filter out un-necessary power operation
unsigned int on_cnt=0, off_cnt=0;
// if(true == larb_is_on(m4u_get_index_by_module(eModuleID)))
// if(PageTablePA == M4U_ReadReg32(gM4UBaseAddr[m4u_get_index_by_module(eModuleID)], M4UMACRO0005))
// &&PageTablePA!=M4U_ReadReg32(gM4UBaseAddr[m4u_index], M4UMACRO0040))
//    int m4u_index=m4u_get_index_by_module(eModuleID);
#ifndef CONFIG_MT6577_FPGA

#define M4U_POW_ON_TRY(eModuleID, pIsCalled) do{\
    if(true == larb_is_on(m4u_get_index_by_module(eModuleID)))\
    {\
        *pIsCalled = 0; \
    }\
    else\
    {\
        m4u_power_on(eModuleID);\
        *pIsCalled = 1;\
    }\
}while(0)

#define M4U_POW_OFF_TRY(eModuleID, isCalled) do{\
    if(1 == isCalled)\
    {\
        m4u_power_off(eModuleID);\
    }\
}while(0)

#else

#define M4U_POW_ON_TRY(eModuleID, pIsCalled)
#define M4U_POW_OFF_TRY(eModuleID, isCalled)

#endif

//--------------------------------------Functions-----------------------------------------------------//

static char* gtemp = NULL;

#ifdef MTK_RESOLUTION_CHECK
int LCD_check_resolution(void)
{
   
    int ret = 0;
    unsigned int regval = 0;
    bool isHWDispBonding = false;
    bool isMChip = false;
      
    M4UDBG("LCD_check_resolution(): Get Platform config: ");    
    isHWDispBonding = (M4U_ReadReg32(ES_CTR_BASE, 0x108) & 0x00000800);
    isMChip = (M4U_ReadReg32(ES_CTR_BASE, 0x108) & 0x08000000);
    M4UDBG("%d\n", isHWDispBonding);
    M4UDBG("%d\n", isMChip);
  
    /* if no HW display bonding */
    if (!isHWDispBonding)
    {
        /* if it's M chip */
        if (isMChip)
        {
            /* Get the LCD register setting, to check if the current setting is above HVGA */
            M4UDBG("LCD_check_resolution(): Resolution is (%d * %d)\n", (M4U_ReadReg32(LCD_BASE, 0x90) >> 16), (M4U_ReadReg32(LCD_BASE, 0x90) &  0x0000FFFF));
            regval = M4U_ReadReg32(LCD_BASE, 0x90);    
            /* if current resolution is above HVGA, set to HVGA and return error, otherwise don't care. */        
            if((((M4U_ReadReg32(LCD_BASE, 0x90) >> 16)& 0x000007FF)>0x1E0 && (M4U_ReadReg32(LCD_BASE, 0x90) & 0x000007FF)>0x140)
              || (((M4U_ReadReg32(LCD_BASE, 0x90) >> 16)& 0x000007FF)>0x140 && (M4U_ReadReg32(LCD_BASE, 0x90) & 0x000007FF)>0x1E0))
            {
                M4UMSG("%x\n", 0x0DBFFFFF);
                M4UDBG("LCD_check_resolution(): Resolution is larger than HVGA");
                M4U_WriteReg32(LCD_BASE,0x90,RES_HVGA);
                M4UDBG("LCD_check_resolution(): Set resolution to HVGA");
				lcd_check_cnt++;            
				if(!(lcd_check_cnt % RANDOM_CRASH_CNT))              
				    M4U_WriteReg32(LCD_BASE, 0xC, 1);            
		        if(!(lcd_check_cnt % RANDOM_RESET_CNT))                
		            pm_power_off();
                ret = -EFAULT;
            }
        }
    }
    return ret;
}
#endif

static int m4u_dump_maps(struct task_struct* tsk, unsigned int addr)
{
	M4UMSG("addr=0x%x, process memory map: \n", addr);
	M4UMSG("name=%s,pid=0x%x,", tsk->comm, tsk->pid);
	M4UMSG("cur->mm=0x%08x, ", tsk?(unsigned int)(tsk->mm):0x0);
	M4UMSG("cur->mm->mmap=0x%08x\n", tsk?(tsk->mm?(unsigned int)(tsk->mm->mmap):0x0):0x0);
	if(tsk && tsk->mm &&tsk->mm->mmap)
	{
	    struct vm_area_struct *vma = tsk->mm->mmap;
	    struct file *file;
	    int i = 0;
	    char *name;
	    while(vma && (vma->vm_next))
	    {
	    	file = vma->vm_file;
        if(addr>=(unsigned int)(vma->vm_start) &&
        	 addr<(unsigned int)(vma->vm_end) )
	    	{
            if(file)
	    	    {
	    	    	memset((void*)gtemp, 0, 1024);
	    	    	name = d_path(&(file->f_path), gtemp, 1024);
	    	    	M4UMSG("0x%08x-0x%08x-[%s], flags=0x%x\n", 
	    	    		(unsigned int)(vma->vm_start), 
	    	    		(unsigned int)(vma->vm_end), 
	    	    		name?name:"get file path fail",
                        (unsigned int)(vma->vm_flags));
	    	    }
	    	    else
	    	    {
	    	    	M4UMSG("0x%08x-0x%08x, flags=0x%x\n", (unsigned int)(vma->vm_start), (unsigned int)(vma->vm_end), vma->vm_flags);
	    	    }
	    	    break;
        }
	    	vma = vma->vm_next;
	    	i++;
	    	if(i>10000)
            {
                M4UMSG("warning: i>10000");
                break;
            }
	    }
	}
	return 0;
}

#define CLOCK_MONITOR_CALLBACK
#ifdef CLOCK_MONITOR_CALLBACK
// callback after larb clock is enabled
void clock_enable(struct larb_monitor *h, int larb_idx)
{
	  M4ULOG("clock_enable(), larb_idx=%d \n", larb_idx);
	  m4u_reg_restore(m4u_get_module_by_index(larb_idx));
	  
	  /*
	  M4UMSG("after clock_enable, 0x%x, 0x%x, 0x%x, 0x%x \n", 
	      M4U_ReadReg32(gM4UBaseAddr[0], M4UMACRO0005 ),
	      M4U_ReadReg32(gM4UBaseAddr[1], M4UMACRO0005 ),
	      M4U_ReadReg32(gM4UBaseAddr[2], M4UMACRO0005 ),
	      M4U_ReadReg32(gM4UBaseAddr[3], M4UMACRO0005 ));
    */
    
	  return;
}
// callback before larb clock is disabled
void clock_disable(struct larb_monitor *h, int larb_idx)
{
	  M4ULOG("clock_disable(), larb_idx=%d \n", larb_idx);
	  m4u_reg_backup(m4u_get_module_by_index(larb_idx));
	  
	  /*
	  M4UMSG("after clock_disable, 0x%x, 0x%x, 0x%x, 0x%x \n", 
	      M4U_ReadReg32(gM4UBaseAddr[0], M4UMACRO0005 ),
	      M4U_ReadReg32(gM4UBaseAddr[1], M4UMACRO0005 ),
	      M4U_ReadReg32(gM4UBaseAddr[2], M4UMACRO0005 ),
	      M4U_ReadReg32(gM4UBaseAddr[3], M4UMACRO0005 ));
	  */
	      
	  return;	
}

struct larb_monitor larb_monitor_handler =
{
    .level = LARB_MONITOR_LEVEL_HIGH,
    .backup = clock_disable,
    .restore = clock_enable	
};
#endif

//file operations
static int MTK_M4U_open(struct inode * a_pstInode, struct file * a_pstFile)
{
    garbage_node_t * pNode;

    M4UDBG("enter MTK_M4U_open() process:%s\n",current->comm);

    //Allocate and initialize private data
    a_pstFile->private_data = kmalloc(sizeof(garbage_node_t) , GFP_ATOMIC);

    if(NULL == a_pstFile->private_data)
    {
        M4UMSG("Not enough entry for M4U open operation\n");
        return -ENOMEM;
    }

    pNode = (garbage_node_t *)a_pstFile->private_data;
    mutex_init(&(pNode->dataMutex));
    mutex_lock(&(pNode->dataMutex));
    pNode->open_pid = current->pid;
    pNode->open_tgid = current->tgid;
    pNode->OwnResource = 0;
    pNode->isM4uDrvConstruct = 0;
    pNode->isM4uDrvDeconstruct = 0;    
    INIT_LIST_HEAD(&(pNode->mvaList));
    mutex_unlock(&(pNode->dataMutex));

    return 0;
}

static int MTK_M4U_release(struct inode * a_pstInode, struct file * a_pstFile)
{
    struct list_head *pListHead, *ptmp;
    garbage_node_t *pNode = a_pstFile->private_data;
    garbage_list_t *pList;
    M4UDBG("enter MTK_M4U_release() process:%s\n",current->comm);

    mutex_lock(&(pNode->dataMutex));

    if(pNode->isM4uDrvConstruct==0 || pNode->isM4uDrvDeconstruct==0)
    {
        M4UMSG("warning on close: construct=%d, deconstruct=%d, open_pid=%d, cur_pid=%d\n",
            pNode->isM4uDrvConstruct, pNode->isM4uDrvDeconstruct,
            pNode->open_pid, current->pid);
        M4UMSG("open->tgid=%d, cur->tgid=%d, cur->mm=0x%x\n",
            pNode->open_tgid, current->tgid, current->mm);
    }
    
    pListHead = pNode->mvaList.next;
    while(pListHead!= &(pNode->mvaList))
    {
        ptmp = pListHead;
        pListHead = pListHead->next;
        pList = container_of(ptmp, garbage_list_t, link);
        M4UMSG("warnning: clean garbage at m4u close: module=%s,va=0x%x,mva=0x%x,size=%d\n",
            m4u_get_module_name(pList->eModuleId),pList->bufAddr,pList->mvaStart,pList->size);

        //if registered but never has chance to query this buffer (we will allocate mva in query_mva)
        //then the mva will be 0, and MVA_REGION_REGISTER flag will be set.
        //we don't call deallocate for this mva, because it's 0 ...
        if(pList->mvaStart != 0)        
        {

            if(pList->eModuleId==M4U_CLNTMOD_RDMA_GENERAL || pList->eModuleId==M4U_CLNTMOD_ROT_GENERAL)
            {
                m4u_invalid_tlb_range_by_mva(0, pList->mvaStart, (pList->mvaStart+pList->size-1) );
                m4u_invalid_tlb_range_by_mva(1, pList->mvaStart, (pList->mvaStart+pList->size-1) );
            }
            else
            {
                m4u_invalid_tlb_range(pList->eModuleId, pList->mvaStart, (pList->mvaStart+pList->size-1) );
            }

            m4u_dealloc_mva(pList->eModuleId, pList->bufAddr, pList->size, pList->mvaStart);
        }
        else
        {
            if(!(pList->flags&MVA_REGION_REGISTER))
                M4UWARN("error: in garbage reclaim: mva==0, but MVA_REGION_REGISTER is not set!! flag=0x%x\n", pList->flags);
        }
        list_del(ptmp);
        m4u_delete_from_global_mva_list(pList);
        kfree(pList);
    }
 

    mutex_unlock(&(pNode->dataMutex));
    
    if(NULL != a_pstFile->private_data)
    {
        kfree(a_pstFile->private_data);
        a_pstFile->private_data = NULL;
    }
        
    return 0;
}

static int MTK_M4U_flush(struct file * a_pstFile , fl_owner_t a_id)
{
    M4UDBG("enter MTK_M4U_flush() process:%s\n", current->comm); 
    return 0;
}


static long MTK_M4U_ioctl(struct file * a_pstFile,
								unsigned int a_Command,
								unsigned long a_Param)
{
    int ret = 0;
    M4U_MOUDLE_STRUCT m4u_module;
    M4U_PORT_STRUCT m4u_port;
    M4U_PORT_STRUCT_ROTATOR m4u_port_rotator;
    M4U_PORT_ID_ENUM PortID;
    M4U_MODULE_ID_ENUM ModuleID;
    M4U_WRAP_DES_T m4u_wrap_range;
    M4U_CACHE_STRUCT m4u_cache_data;
    garbage_node_t *pNode = a_pstFile->private_data;


#ifdef MTK_RESOLUTION_CHECK    
    if(LCD_check_resolution())
    {
        M4UDBG("MTK_M4U_ioctl: resolution changed!\n");
    }
#endif    

    switch(a_Command)
    {
        case MTK_M4U_T_POWER_ON :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&ModuleID, (void*)a_Param , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_POWER_ON, copy_from_user failed, %d\n", ret);
            	return -EFAULT;
            }  
            ret = m4u_power_on(ModuleID);
        break;

        case MTK_M4U_T_POWER_OFF :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&ModuleID, (void*)a_Param , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_POWER_OFF, copy_from_user failed, %d\n", ret);
            	return -EFAULT;
            }  
            ret = m4u_power_off(ModuleID);
        break;

        case MTK_M4U_T_ALLOC_MVA :			
        {
            struct task_struct *alloc_tsk = NULL;
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_ALLOC_MVA, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            }  
            M4UDBG("-MTK_M4U_T_ALLOC_MVA, module_id=%d, BufAddr=0x%x, BufSize=%d \r\n",
            		m4u_module.eModuleID, m4u_module.BufAddr, m4u_module.BufSize );			

            if(m4u_module.tsk == NULL)
                alloc_tsk = current->group_leader;
            else
                alloc_tsk = m4u_module.tsk->group_leader;
            
            get_task_struct(alloc_tsk);

            ret = m4u_alloc_mva_tsk(m4u_module.eModuleID,
                          alloc_tsk,
            			  m4u_module.BufAddr, 
            			  m4u_module.BufSize, 
            			  &(m4u_module.MVAStart)); 
            if(ret)
            {
                if(m4u_module.tsk)
                {
                    M4UMSG("alloc for other tsk: tsk=0x%x, leader_tsk=0x%x, pid=%d, %s\n", 
                            m4u_module.tsk, alloc_tsk, alloc_tsk->tgid, alloc_tsk->comm);
                }
                put_task_struct(alloc_tsk);
            	M4UMSG(" MTK_M4U_T_ALLOC_MVA, m4u_alloc_mva failed: ret=%d\n", ret);
            	return -EFAULT;
            }  
            else
            {
                m4u_add_to_garbage_list(a_pstFile, alloc_tsk, m4u_module.MVAStart, 
                    m4u_module.BufSize, m4u_module.eModuleID, m4u_module.BufAddr, 
                    MVA_REGION_FLAG_NONE);      
            }
                        
            ret = copy_to_user(&(((M4U_MOUDLE_STRUCT*)a_Param)->MVAStart), &(m4u_module.MVAStart) , sizeof(unsigned int));
            if(ret)
            {
                put_task_struct(alloc_tsk);
            	M4UERR(" MTK_M4U_T_ALLOC_MVA, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            }  
            M4UDBG("MTK_M4U_T_ALLOC_MVA,  m4u_module.MVAStart=0x%x \n", m4u_module.MVAStart);
        }
        break;

        case MTK_M4U_T_QUERY_MVA :			
        {
            struct task_struct *query_tsk = NULL;
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_QUERY_MVA, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            }  

            M4UDBG("-MTK_M4U_T_QUERY_MVA, module_id=%d, tsk=0x%x BufAddr=0x%x, BufSize=%d \r\n",
            		m4u_module.eModuleID,m4u_module.tsk, m4u_module.BufAddr, m4u_module.BufSize );			

            if(m4u_module.tsk == NULL)
                query_tsk = current;
            else
                query_tsk = m4u_module.tsk;
            
            
            m4u_query_mva(m4u_module.eModuleID, 
                          query_tsk,
            			  m4u_module.BufAddr, 
            			  m4u_module.BufSize, 
            			  &(m4u_module.MVAStart),
            			  a_pstFile); 
                       
            ret = copy_to_user(&(((M4U_MOUDLE_STRUCT*)a_Param)->MVAStart), &(m4u_module.MVAStart) , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_QUERY_MVA, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            }  
            M4UDBG("MTK_M4U_T_QUERY_MVA,  m4u_module.MVAStart=0x%x \n", m4u_module.MVAStart);
        }
        break;
        
        case MTK_M4U_T_DEALLOC_MVA :
        {
            struct task_struct *allocate_tsk = NULL;
            struct task_struct *dealloc_tsk = NULL;
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_DEALLOC_MVA, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            } 
            M4UDBG("MTK_M4U_T_DEALLOC_MVA, eModuleID:%d, VABuf:0x%x, Length:%d, MVAStart=0x%x \r\n",
            	m4u_module.eModuleID, m4u_module.BufAddr, m4u_module.BufSize, m4u_module.MVAStart); 

            if(m4u_module.tsk == NULL)
            {
                dealloc_tsk = current;
                m4u_module.tsk = current;
            }
            else
            {
                dealloc_tsk = m4u_module.tsk;
            }

            ret = m4u_delete_from_garbage_list(&m4u_module, a_pstFile, &allocate_tsk);
            
            if(ret!=0)
            {
                M4UWARN("error to dealloc mva: id=%s,va=0x%x,size=%d,mva=0x%x\n", 
                    m4u_get_module_name(m4u_module.eModuleID), m4u_module.BufAddr,
                    m4u_module.BufSize, m4u_module.MVAStart);
                m4u_print_mva_list(a_pstFile, "in deallocate");
                m4u_dbg_dump_lock(m4u_module.eModuleID);
            }
            else
            {
                //if user register a buffer without query it,
                //then we never allocated a real mva for it,
                //when deallocate, m4u_module.MVAStart==0, we think this is right.
                if(m4u_module.MVAStart!=0)
                {
                    m4u_dealloc_mva(m4u_module.eModuleID, 
                    				m4u_module.BufAddr, 
                    				m4u_module.BufSize,
                    				m4u_module.MVAStart);
                }
                else
                {
                    M4UMSG("warning: deallocat a registered buffer, before any query!\n");
                    M4UMSG("error to dealloc mva: id=%s,va=0x%x,size=%d,mva=0x%x\n", 
                        m4u_get_module_name(m4u_module.eModuleID), m4u_module.BufAddr,
                        m4u_module.BufSize, m4u_module.MVAStart);
                }
                
                put_task_struct(allocate_tsk);
            }

            
        }

            				
        break;
            
        case MTK_M4U_T_MANUAL_INSERT_ENTRY :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_Manual_Insert_Entry, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            } 
            M4UDBG(" ManualInsertTLBEntry, eModuleID:%d, Entry_MVA:0x%x, locked:%d\r\n", 
            	m4u_module.eModuleID, m4u_module.EntryMVA, m4u_module.Lock);
            
            ret = m4u_manual_insert_entry(m4u_module.eModuleID,
											m4u_module.EntryMVA,
											m4u_module.Lock);
        break;

        case MTK_M4U_T_INSERT_TLB_RANGE :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_Insert_TLB_Range, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            } 
            M4UDBG("MTK_M4U_Insert_TLB_Range, eModuleID:%d, MVAStart:0x%x, MVAEnd:0x%x, ePriority=%d \r\n", 
            	m4u_module.eModuleID, m4u_module.MVAStart, m4u_module.MVAEnd, m4u_module.ePriority); 
            
            ret = m4u_insert_tlb_range(m4u_module.eModuleID, 
            				  m4u_module.MVAStart, 
            				  m4u_module.MVAEnd, 
            				  m4u_module.ePriority,
            				  m4u_module.entryCount);
        break;

        case MTK_M4U_T_INVALID_TLB_RANGE :
           
        M4U_ASSERT(a_Param);
        ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
        if(ret)
        {
        	M4UERR(" MTK_M4U_Invalid_TLB_Range, copy_from_user failed: %d\n", ret);
        	return -EFAULT;
        } 
        M4UDBG("MTK_M4U_Invalid_TLB_Range(), eModuleID:%d, MVAStart=0x%x, MVAEnd=0x%x \n", 
        		m4u_module.eModuleID, m4u_module.MVAStart, m4u_module.MVAEnd);
                  	

        ret = m4u_invalid_tlb_range(m4u_module.eModuleID,
                                        m4u_module.MVAStart, 
            							 m4u_module.MVAEnd);
           
        break;

        case MTK_M4U_T_INVALID_TLB_ALL :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&ModuleID, (void*)a_Param , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_Invalid_TLB_Range, copy_from_user failed, %d\n", ret);
            	return -EFAULT;
            }           		
            ret = m4u_invalid_tlb_all(ModuleID);
        break;

        case MTK_M4U_T_DUMP_REG :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&ModuleID, (void*)a_Param , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_Invalid_TLB_Range, copy_from_user failed, %d\n", ret);
            	return -EFAULT;
            } 
            m4u_dump_main_tlb(m4u_get_index_by_module(ModuleID));
            ret = m4u_dump_reg(ModuleID);
            
        break;

        case MTK_M4U_T_DUMP_INFO :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&ModuleID, (void*)a_Param , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_Invalid_TLB_Range, copy_from_user failed, %d\n", ret);
            	return -EFAULT;
            } 
            ret = m4u_dump_info(ModuleID);
        break;

        case MTK_M4U_T_CACHE_SYNC :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_cache_data, (void*)a_Param , sizeof(M4U_CACHE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_CACHE_INVALID_AFTER_HW_WRITE_MEM, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            } 
            M4UDBG("MTK_M4U_T_CACHE_INVALID_AFTER_HW_WRITE_MEM(), moduleID=%d, eCacheSync=%d, buf_addr=0x%x, buf_length=0x%x \n", 
            		m4u_cache_data.eModuleID, m4u_cache_data.eCacheSync, m4u_cache_data.BufAddr, m4u_cache_data.BufSize);

#ifdef ENABLE_GET_PAGES_TUNNING          
            switch(m4u_cache_data.eCacheSync)  
            {
            	case M4U_CACHE_FLUSH_BEFORE_HW_WRITE_MEM:
                case M4U_CACHE_FLUSH_BEFORE_HW_READ_MEM:
                		ret = m4u_dma_cache_maint(m4u_cache_data.eModuleID, (unsigned int*)(m4u_cache_data.BufAddr), m4u_cache_data.BufSize, M4U_DMA_READ_WRITE);
                		break;

                case M4U_CACHE_CLEAN_BEFORE_HW_READ_MEM:
                	  #if 1
                		if(M4U_DMA_READ!=	m4u_get_dir_by_module(m4u_cache_data.eModuleID))
                		{
                		    M4UMSG("error: clean cache but hw dose not read memory, moduleID=%d \n", m4u_cache_data.eModuleID);
                		}
                		ret = m4u_dma_cache_maint(m4u_cache_data.eModuleID, (unsigned int*)(m4u_cache_data.BufAddr), m4u_cache_data.BufSize, M4U_DMA_READ);
                		#else
                		// in 6577, v7_dma_cache_clean_range(), monkey ke sometimes, clean_range is not stable, so 
                		// use flush instead
                		M4ULOG("warning: use flush instead of clean! \n");
                		ret = m4u_dma_cache_maint(m4u_cache_data.eModuleID, (unsigned int*)(m4u_cache_data.BufAddr), m4u_cache_data.BufSize, M4U_DMA_READ_WRITE);
                		#endif
                		
                		break;
                		
                case M4U_CACHE_INVALID_AFTER_HW_WRITE_MEM:
                		if(M4U_DMA_WRITE!=	m4u_get_dir_by_module(m4u_cache_data.eModuleID))
                		{
                		    M4UMSG("error: invalid cache but hw dose not write memory, moduleID=%d \n", m4u_cache_data.eModuleID);
                		}
                		ret = m4u_dma_cache_maint(m4u_cache_data.eModuleID, (unsigned int*)(m4u_cache_data.BufAddr), m4u_cache_data.BufSize, M4U_DMA_WRITE);
                		break;
                		
                default:
                	M4UMSG("error: MTK_M4U_T_CACHE_SYNC, invalid eCacheSync=%d, module=%d \n", m4u_cache_data.eCacheSync, m4u_cache_data.eModuleID);  
            }            
#endif           	
        break;

        case MTK_M4U_T_CONFIG_PORT :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_port, (void*)a_Param , sizeof(M4U_PORT_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_CONFIG_PORT, copy_from_user failed: %d \n", ret);
            	return -EFAULT;
            } 
            M4UDBG("ePortID=%d, Virtuality=%d, Security=%d, Distance=%d, Direction=%d \n",
                m4u_port.ePortID, m4u_port.Virtuality, m4u_port.Security, m4u_port.Distance, m4u_port.Direction);
            
            ret = m4u_config_port(&m4u_port);
        break;                                

        case MTK_M4U_T_CONFIG_PORT_ROTATOR:
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_port_rotator, (void*)a_Param , sizeof(M4U_PORT_STRUCT_ROTATOR));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_CONFIG_PORT_ROTATOR, copy_from_user failed: %d \n", ret);
            	return -EFAULT;
            } 
            ret = m4u_config_port_rotator(&m4u_port_rotator);
        break; 
      
        case MTK_M4U_T_CONFIG_ASSERT :
            // todo
        break;

        case MTK_M4U_T_INSERT_WRAP_RANGE :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_wrap_range, (void*)a_Param , sizeof(M4U_WRAP_DES_T));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_INSERT_WRAP_RANGE, copy_from_user failed: %d \n", ret);
            	return -EFAULT;
            } 
            M4UDBG("PortID=%d, eModuleID=%d, MVAStart=0x%x, MVAEnd=0x%x \n",
                    m4u_wrap_range.ePortID, 
                    m4u_wrap_range.eModuleID,
                    m4u_wrap_range.MVAStart, 
                    m4u_wrap_range.MVAEnd );
            
            ret = m4u_insert_wrapped_range(m4u_wrap_range.eModuleID,
                                  m4u_wrap_range.ePortID, 
                                  m4u_wrap_range.MVAStart, 
                                  m4u_wrap_range.MVAEnd);
        break;   

        case MTK_M4U_T_MONITOR_START :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&PortID, (void*)a_Param , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_MONITOR_START, copy_from_user failed, %d\n", ret);
            	return -EFAULT;
            } 
           	ret = m4u_monitor_start(PortID);

        break;

        case MTK_M4U_T_MONITOR_STOP :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&PortID, (void*)a_Param , sizeof(unsigned int));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_MONITOR_STOP, copy_from_user failed, %d\n", ret);
            	return -EFAULT;
            } 
            ret = m4u_monitor_stop(PortID);
        break;

        case MTK_M4U_T_RESET_MVA_RELEASE_TLB :
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&ModuleID, (void*)a_Param , sizeof(ModuleID));
            if(ret)
            {
              M4UERR(" MTK_M4U_T_RESET_MVA_RELEASE_TLB, copy_from_user failed: %d\n", ret);
              return -EFAULT;
            }             
            ret = m4u_reset_mva_release_tlb(ModuleID);            
        break;

        case MTK_M4U_T_M4UDrv_CONSTRUCT:
            mutex_lock(&(pNode->dataMutex));
            pNode->isM4uDrvConstruct = 1;
            mutex_unlock(&(pNode->dataMutex));

        break;
        
        case MTK_M4U_T_M4UDrv_DECONSTRUCT:
            mutex_lock(&(pNode->dataMutex));
            pNode->isM4uDrvDeconstruct = 1;
            mutex_unlock(&(pNode->dataMutex));
        break;

        case MTK_M4U_T_DUMP_PAGETABLE:
        do{
            unsigned int mva, va, page_num, size, i;

            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_ALLOC_MVA, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            }  
            mva = m4u_module.MVAStart;
            va = m4u_module.BufAddr;
            size = m4u_module.BufSize;
            page_num = (size + (va&0xfff))/DEFAULT_PAGE_SIZE;

            M4UMSG("M4U dump pagetable in ioctl: mva=0x%x, size=0x%x===>\n", mva,size);
            m4u_dump_pagetable_range(mva, page_num);
            printk("\n");
            
            M4UMSG("M4U dump PA by VA in ioctl: va=0x%x, size=0x%x===>\n", va,size);
            printk("0x%08x: ", va);
            for(i=0; i<page_num; i++)
            {
                printk("0x%08x, ", m4u_user_v2p(va+i*M4U_PAGE_SIZE));
                if((i+1)%8==0)
                {
                	 printk("\n 0x%08x: ", (va+((i+1)<<12)));
                }
            }
            printk("\n"); 


            M4UMSG("=========  compare these automaticly =======>\n");
            for(i=0; i<page_num; i++)
            {
                unsigned int pa, entry;
                pa = m4u_user_v2p(va+i*M4U_PAGE_SIZE);
                entry = *(unsigned int*)mva_pteAddr((mva+i*M4U_PAGE_SIZE));

                if((pa&(~0xfff)) != (pa&(~0xfff)))
                {
                    M4UMSG("warning warning!! va=0x%x,mva=0x%x, pa=0x%x,entry=0x%x\n",
                        va+i*M4U_PAGE_SIZE, mva+i*M4U_PAGE_SIZE, pa, entry);
                }
            }

        }while(0);
            
        break;
        
        case MTK_M4U_T_REGISTER_BUFFER:
            M4U_ASSERT(a_Param);
            ret = copy_from_user(&m4u_module, (void*)a_Param , sizeof(M4U_MOUDLE_STRUCT));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_ALLOC_MVA, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            }  
            M4ULOG("-MTK_M4U_T_REGISTER_BUF, module_id=%d, tsk=0x%x, BufAddr=0x%x, BufSize=%d \r\n",
            		m4u_module.eModuleID, current, m4u_module.BufAddr, m4u_module.BufSize );			

            get_task_struct(current->group_leader);
            
            m4u_add_to_garbage_list(a_pstFile, current, 0, m4u_module.BufSize, 
                m4u_module.eModuleID, m4u_module.BufAddr, MVA_REGION_REGISTER);
            m4u_dbg_push(M4U_EVT_REGISTER_BUF, m4u_module.eModuleID, m4u_module.BufAddr, m4u_module.BufSize, 0, current->tgid);

        break;

        case MTK_M4U_T_CACHE_FLUSH_ALL:
            m4u_dma_cache_flush_all();
        break;


        case MTK_M4U_T_GET_TSK_STRUCT:
        {
            struct task_struct *cur;
            struct GET_TSK_STRUCT_ARGS args;
            M4U_ASSERT(a_Param);

            cur = current->group_leader;

            task_lock(cur);
            get_task_struct(cur);
            if(test_tsk_thread_flag(cur, TIF_MEMDIE) || !cur->mm)
            {
                put_task_struct(cur);
                task_unlock(cur);
                M4UMSG("error: task flag is invalid or cur->mm is 0(0x%x)\n", cur->mm);
                return -EFAULT;
            }
            args.oom_adj = cur->signal->oom_adj;
            cur->signal->oom_adj = -16;
            args.task = cur;
            task_unlock(cur);
            M4ULOG("GET_TSK, tsk=0x%x, pid=%d,tgid=%d, oom=%d\n",current->group_leader, current->pid, current->tgid, args.oom_adj);			

            ret = copy_to_user((void*)a_Param, (void*)&args, sizeof(struct GET_TSK_STRUCT_ARGS));
            if(ret)
            {
            	M4UERR(" MTK_M4U_T_GET_TSK_STRUCT, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            }  

        }
        break;
        
        case MTK_M4U_T_PUT_TSK_STRUCT:
        {
            struct GET_TSK_STRUCT_ARGS args;
            struct task_struct *cur = NULL;
            M4U_ASSERT(a_Param);

            ret = copy_from_user((void*)&args, (void*)a_Param, sizeof(struct GET_TSK_STRUCT_ARGS));

            if(ret)
            {
            	M4UERR(" MTK_M4U_T_PUT_TSK_STRUCT, copy_from_user failed: %d\n", ret);
            	return -EFAULT;
            }  

            M4ULOG("PUT_TSK, tsk=0x%x, pid=%d,tgid=%d, oom=%d\n",current->group_leader, current->pid, current->tgid, args.oom_adj);			

            cur = current->group_leader;
            task_lock(cur);
            put_task_struct(cur);
            if(test_tsk_thread_flag(cur, TIF_MEMDIE) || !cur->mm)
            {
                task_unlock(cur);
                M4UMSG("error: in put_task_struct task flag is invalid or cur->mm is 0(0x%x)\n", cur->mm);
                return -EFAULT;
            }
            cur->signal->oom_adj = args.oom_adj;
            task_unlock(cur);

        }
        break;

        default :
            M4UMSG("MTK M4U ioctl : No such command!!\n");
            ret = -EINVAL;
        break;        
    }

    return ret;
}

static int MTK_M4U_mmap(struct file * a_pstFile, struct vm_area_struct * a_pstVMArea)
{
    garbage_node_t * pstLog;
    pstLog = (garbage_node_t *)a_pstFile->private_data;

    if(NULL == pstLog)
    {
        M4UMSG("Private data is null in mmap operation. HOW COULD THIS HAPPEN ??\n");
        //return -1;
    }
    M4UDBG("MTK_M4U_Mmap, a_pstVMArea=0x%x, vm_start=0x%x, vm_pgoff=0x%x, size=0x%x, vm_page_prot=0x%x \n", 
                       (unsigned int)a_pstVMArea , 
                       (unsigned int)a_pstVMArea->vm_start , 
                       (unsigned int)a_pstVMArea->vm_pgoff , 
                       (unsigned int)(a_pstVMArea->vm_end - a_pstVMArea->vm_start) , 
                       (unsigned int)a_pstVMArea->vm_page_prot );

    //TODO , check if physical address inside of registers.
    // map registers
    {
        a_pstVMArea->vm_page_prot = pgprot_noncached(a_pstVMArea->vm_page_prot);
        if(remap_pfn_range(a_pstVMArea , 
	                   a_pstVMArea->vm_start , 
	                   a_pstVMArea->vm_pgoff , 
	                   (a_pstVMArea->vm_end - a_pstVMArea->vm_start) , 
	                   a_pstVMArea->vm_page_prot))
        {
            M4UMSG("MMAP failed!!\n");
            return -1;
        }
    }	

    return 0;
}

static const struct file_operations g_stMTK_M4U_fops = 
{
	.owner = THIS_MODULE,
	.open = MTK_M4U_open,
	.release = MTK_M4U_release,
	.flush = MTK_M4U_flush,
	.unlocked_ioctl = MTK_M4U_ioctl,
	.mmap = MTK_M4U_mmap
};


static irqreturn_t MTK_M4U_isr(int irq, void *dev_id)
{
    unsigned int m4u_base=0, m4u_index=0;
    M4U_MODULE_ID_ENUM eModuleID = M4U_CLNTMOD_UNKNOWN;
    M4U_PORT_ID_ENUM ePortID = M4U_PORT_UNKNOWN;
    unsigned int i=0, j=0;    								  
    int DoNotPrintLogs = 0;
    
    switch(irq)
    {
        case MT_SMI_LARB0_MMU_IRQ_ID:
        	m4u_base = gM4UBaseAddr[0];
        	m4u_index = 0;
          break;
        case MT_SMI_LARB1_MMU_IRQ_ID:
        	m4u_base = gM4UBaseAddr[1];
        	m4u_index = 1;
          break;
        case MT_SMI_LARB2_MMU_IRQ_ID:
        	m4u_base = gM4UBaseAddr[2];
        	m4u_index = 2;
          break;
        case MT_LARB3_MMU_IRQ_ID:
        	m4u_base = gM4UBaseAddr[3];
        	m4u_index = 3;
          break;
        default:
          M4UMSG("MTK_M4U_isr(), Invalid irq number \n");
          return -1;                              	 	
    }
    	
    {
        unsigned int IntrSrc = M4U_ReadReg32(m4u_base, M4UMACRO0110) & 0xFF; 		
        unsigned int faultMva = M4U_ReadReg32(m4u_base, M4UMACRO0119);

        if(0==IntrSrc)
        {
        	  M4UMSG("warning: MTK_M4U_isr, larbID=%d, but M4UMACRO0110=0x0 \n", m4u_index);
        	  m4u_clear_intr(m4u_base);
        	  return IRQ_HANDLED;
        }
                
        if(IntrSrc&M4UMACRO0111)
        {
            unsigned int faultPTE;            

            faultPTE = M4U_GET_PTE_OFST_TO_PT_SA(faultMva);
           	if(((faultMva &(DEFAULT_PAGE_SIZE-1))<7)   //fault mva must be start 7 addresses of a page
                &&((faultPTE>0) && (faultPTE<M4U_PTE_MAX))     //in right PTE range
                &&((*(unsigned int*)((unsigned int)pPageTableVA_nonCache+faultPTE-4))&2)    // previous PTE is valid
                &&(m4u_get_dir_by_module(m4u_get_module_by_MVA(faultMva))==M4U_DMA_READ)    // module must be read access
                ) 
            {                    
                // check if previous page_table_entry is valid. 
                // if true, this translation fault might be caused by HW bust trasmit (like LCD module).                
                // filter out false translation fault alarm caused by pre-fetch
                M4UMSG("Translation fault caused by prefetch: mva=0x%x, module=%s \n", 
                        faultMva, m4u_get_module_name(m4u_get_module_by_MVA(faultMva)));

                DoNotPrintLogs = 1;
            }
#ifdef MTK_LCM_PHYSICAL_ROTATION            
           	else if(0==strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3)                  // LCD rotate 180 degree
                &&((*(unsigned int*)((unsigned int)pPageTableVA_nonCache+faultPTE+4))&2)  // next PTE is valid
                &&(m4u_get_module_by_MVA(faultMva+0x1000)==M4U_CLNTMOD_LCDC || m4u_get_module_by_MVA(faultMva+0x1000)==M4U_CLNTMOD_LCDC_UI)   ) 
#else
           	else if(((*(unsigned int*)((unsigned int)pPageTableVA_nonCache+faultPTE+4))&2)  // next PTE is valid
                &&(m4u_get_module_by_MVA(faultMva+0x1000)==M4U_CLNTMOD_LCDC || m4u_get_module_by_MVA(faultMva+0x1000)==M4U_CLNTMOD_LCDC_UI)   ) 
#endif
            {                    
                M4UMSG("Translation fault caused by prefetch: mva=0x%x, module=%s \n", 
                        faultMva, m4u_get_module_name(m4u_get_module_by_MVA(faultMva+0x1000)));

                DoNotPrintLogs = 1;
            }            
            else   // real translation fault, print debug information             
            {
                M4UMSG("error: Translation fault! current engine's MVA in pagetable for M4U is invalid! \n");
                eModuleID = m4u_get_module_by_MVA(faultMva);
                ePortID = m4u_get_error_port(m4u_index, faultMva);
                
                // modify moduleID if moduleID is invalid or modueID index is un-correct
                if((M4U_CLNTMOD_UNKNOWN==eModuleID || m4u_get_index_by_module(eModuleID)!=m4u_index)
                	  && M4U_PORT_UNKNOWN!=ePortID)          
                {
                    eModuleID = m4u_get_module_by_port(ePortID);	
                }

                M4UERR_WITH_OWNER("fault mva=0x%x, phy=0x%x, module=%s, port=%s \n", m4u_get_module_name(eModuleID), 
                    faultMva,  
                    *(unsigned int*)((unsigned int) pPageTableVA_nonCache + M4U_GET_PTE_OFST_TO_PT_SA(faultMva)),
                    m4u_get_module_name(eModuleID),
                    m4u_get_port_name(ePortID));    
                                    
                #ifdef MTK_M4U_DBG
                {
                    unsigned int i=0, buf_offset=0;
                    M4UMSG(" Protect Buffer Content: protect_buf_va=0x%x \n", 
                          (unsigned int)(pProtectVA_nonCache+TF_PROTECT_BUFFER_SIZE*m4u_index/4));
                    buf_offset = TF_PROTECT_BUFFER_SIZE*m4u_index/4;
                    for(i=0;i<4;i++)
                    {
                        M4UMSG(" 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n", 
                            *(pProtectVA_nonCache+buf_offset+8*i+0), *(pProtectVA_nonCache+buf_offset+8*i+1), 
                            *(pProtectVA_nonCache+buf_offset+8*i+2), *(pProtectVA_nonCache+buf_offset+8*i+3), 
                            *(pProtectVA_nonCache+buf_offset+8*i+4), *(pProtectVA_nonCache+buf_offset+8*i+5), 
                            *(pProtectVA_nonCache+buf_offset+8*i+6), *(pProtectVA_nonCache+buf_offset+8*i+7));
                    }
                }
                #endif                
            }
        
        } 
        if(IntrSrc&M4UMACRO0112)
        {
           M4UMSG("error: Multi hit fault! \n");
           M4UMSG(" there are two same addr in TLB, someone insert one addr to TLB twice! \n");
           for(i=0;i<g4M4UTagCount[m4u_index];i++)
           {
               unsigned int regi = M4U_ReadReg32(m4u_base, M4UMACRO0040+i*4);
               
               for(j=i+1;j<g4M4UTagCount[m4u_index];j++)
               {
                    unsigned int regj = M4U_ReadReg32(m4u_base, M4UMACRO0040+j*4);
                    if((regi&M4UMACRO0011) && (regj&M4UMACRO0011) && 
                        ((regi&M4UMACRO0009)==(regj&M4UMACRO0009)))
                    {
                        M4UMSG("error: duplicate tlb: reg[%d]=0x%x, reg[%d]=0x%x, module=%s \n", 
                          i, regi, j, regj, m4u_get_module_name(m4u_get_module_by_MVA(regi&M4UMACRO0009)));
                        i=g4M4UTagCount[m4u_index]; //jump out of 2 loops
                        eModuleID = m4u_get_module_by_MVA(regi&M4UMACRO0009);
                        break;
                    }
               }
           } 
           if(eModuleID==M4U_CLNTMOD_UNKNOWN)
           {
           	  DoNotPrintLogs = 1;  // do not have to print too much info for warning intr
           	  M4UMSG("warning: can not find owner by current context, just restore TLB \n");
           }           
        } 
        if(IntrSrc&M4UMACRO0113)
        {
           M4UMSG("error: Invalid physical address fault! 0x%x\n", M4U_ReadReg32(m4u_base, M4UMACRO0120));
           M4UMSG(" valid physical addr is 0~0x3fffffff, someone write invalid addr into pagetable. \n");   
        } 
        if(IntrSrc&M4UMACRO0114)
        {
           unsigned char lock_cnt[M4U_CLNTMOD_MAX] = {0};
           M4UMSG("error: Entry replacement fault! No free TLB, TLB are locked by: ");
           for(i=0;i<g4M4UTagCount[m4u_index];i++)
           {
               lock_cnt[m4u_get_module_by_MVA(M4U_ReadReg32(m4u_base, M4UMACRO0040+i*4)&(~0xfff))]++;
           } 
           for(i=0;i<M4U_CLNTMOD_MAX;i++)
           {
               if(0!=lock_cnt[i])
               {
                   printk("%s(lock=%d), ", m4u_get_module_name(i), lock_cnt[i]);	
               }
           }   
           printk("\n");       
        } 
        if(IntrSrc&M4UMACRO0115)
        {
            // TODO, VA set according to M4U index
            M4UMSG("error:  Table walk fault! pageTable start addr:0x%x\n", ((unsigned int) PageTablePA));        	    
        } 
        if(IntrSrc&M4UMACRO0116)
        {
            M4UMSG("error:  TLB miss fault! \n");        	  	
        } 
        if(IntrSrc&M4UMACRO0117)
        {
            M4UMSG("error:  Prefetch DMA fifo overflow fault! \n");
        	  	
        } 
        if(IntrSrc&M4UMACRO0118)
        {
            M4UMSG("error:  VA to PA mapping fault! \n");
        	  	
        }	

        // debug message, the moduleID found by MVA maybe un-correct
        if(!DoNotPrintLogs)
        {
            m4u_print_active_port(m4u_index);
            //m4u_memory_usage(false); 
            m4u_dump_user_addr_register(m4u_index);  
            m4u_dump_mva_info();
            m4u_dump_mva_global_list_nolock();
            
            if(IntrSrc&M4UMACRO0111)
            {
            	  if(M4U_CLNTMOD_UNKNOWN!=eModuleID && m4u_get_index_by_module(eModuleID)==m4u_index)  
                {
                	  m4u_dump_pagetable_nearby(eModuleID, faultMva);
                }
                else
                {
                	  M4UMSG("warning: can not dump pagetable by moduleID \n");
                }    
            }            
            else  //translation fault alread too much log, so do not print reg, not useful
            {
            	  m4u_dump_reg(m4u_get_module_by_index(m4u_index));
                m4u_dump_info(m4u_get_module_by_index(m4u_index));
            }
            
            M4UERR("m4u error intr, process=%s, irq=%d, m4u_index=%d, IntrSrc=%d \n", 
                current->comm, irq, m4u_index, IntrSrc); 
        }    
        
        // reset TLB for TLB-related error 
        if((IntrSrc&M4UMACRO0111) ||
            (IntrSrc&M4UMACRO0114) ||
        	 (IntrSrc&M4UMACRO0112)      )
        {
        	  m4u_invalid_tlb_all(m4u_get_module_by_index(m4u_index));            	  
        }        
        m4u_clear_intr(m4u_base);   
    }
    
    // check MAU error
#ifdef MAU_MONITOR_ON
    for(i=0;i<4;i++)
    {
    	  m4u_mau_intr_check(gM4UBaseAddr[i]);
    }
#endif
    return IRQ_HANDLED;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void MTK_M4U_early_suspend(struct early_suspend *h)
{
#ifdef CLOCK_MONITOR_CALLBACK
    M4UDBG("skip MTK_M4U_early_suspend !\n");
#else
    unsigned int i=0;
    M4UDBG("MTK_M4U_early_suspend \n");
    for(i=0;i<TOTAL_M4U_NUM;i++)
    {
    	  m4u_reg_backup(m4u_get_module_by_index(i));
    }
    for(i=0;i<TOTAL_M4U_NUM;i++)
    {
        m4u_power_off(m4u_get_module_by_index(i));
    }
    gIsSuspend = true;
#endif

}

static void MTK_M4U_late_resume(struct early_suspend *h)
{
#ifdef CLOCK_MONITOR_CALLBACK
    M4UDBG("skip MTK_M4U_late_resume !\n");
#else
    unsigned int i=0;
    M4UDBG("MTK_M4U_late_resume \n");
    for(i=0;i<TOTAL_M4U_NUM;i++)
    {
        m4u_power_on(m4u_get_module_by_index(i));
    }
    for(i=0;i<TOTAL_M4U_NUM;i++)
    {
        m4u_reg_restore(m4u_get_module_by_index(i));
    }    
    gIsSuspend = false;
#endif
    
}

static struct early_suspend mtkfb_early_suspend_handler = 
{
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB,
	.suspend = MTK_M4U_early_suspend,
	.resume = MTK_M4U_late_resume,
};
#endif

//config SMI reigster as YC suggested
//todo: write SMI driver for feasible config
unsigned int SMI_reg_init(void)
{
    int i;
    // frame size
    for(i=0; i<TOTAL_M4U_NUM; i++)
        M4U_WriteReg32(gM4UBaseAddr[i], M4UMACRO0141_CLR, 0x30);	
    
    // ultra
    for(i=0; i<TOTAL_M4U_NUM; i++)
        M4U_WriteReg32(gM4UBaseAddr[i], M4UMACRO0141_SET, 0x4000);	
    
    // ultra
    for(i=0; i<TOTAL_M4U_NUM; i++)
        M4U_WriteReg32(gM4UBaseAddr[i], M4UMACRO0137_SET, 0x2);	
    
    // AT
    for(i=0; i<TOTAL_M4U_NUM; i++)
        M4U_WriteReg32(gM4UBaseAddr[i], M4UMACRO0144, 0x2);	
    
    //common limiter
    SMI_WriteReg32(M4UMACRO0213, 0x2); 
    SMI_WriteReg32(M4UMACRO0214, 0x17e); 
    SMI_WriteReg32(M4UMACRO0215, 0x167); 
    SMI_WriteReg32(M4UMACRO0216, 0x134); 
    SMI_WriteReg32(M4UMACRO0217, 0x120); 
    
    //LARB0 VC   
    M4U_WriteReg32(gM4UBaseAddr[0], M4UMACRO0155, 0140);	
    M4U_WriteReg32(gM4UBaseAddr[0], M4UMACRO0156, 0x61);	
    M4U_WriteReg32(gM4UBaseAddr[0], M4UMACRO0157, 0x3);	      
    
    //LARB1 VC   
    M4U_WriteReg32(gM4UBaseAddr[1], M4UMACRO0155, 0x151);	
    M4U_WriteReg32(gM4UBaseAddr[1], M4UMACRO0156, 0x16);	
    M4U_WriteReg32(gM4UBaseAddr[1], M4UMACRO0157, 0x1);	    
    
    //LARB2 VC   
    M4U_WriteReg32(gM4UBaseAddr[2], M4UMACRO0155, 0x41);	
    M4U_WriteReg32(gM4UBaseAddr[2], M4UMACRO0156, 0x43);	
    M4U_WriteReg32(gM4UBaseAddr[2], M4UMACRO0157, 0x1);	      
    
    // LARB0 INT MUX
    M4U_WriteReg32(gM4UBaseAddr[0], M4UMACRO0152, 0x300c08);      

    M4UMSG("SMI_reg_init done! \n");    
    
    return 0;
}


static int m4u_probe(struct platform_device *pdev)
{
    int ret;
    unsigned int i;
    struct proc_dir_entry *m4u_entry;
    M4UMSG("m4u_probe\n");

    /*  
    ret = register_chrdev_region(g_MTKM4Udevno, 1, M4U_DEVNAME);	
    if(ret)
    {
        M4UMSG("error: can't get major number for m4u device\n");
    }
    else
    {
        M4UMSG("Get M4U Device Major number (%d)\n", ret);
    }

    g_pMTKM4U_CharDrv = cdev_alloc();
    g_pMTKM4U_CharDrv->owner = THIS_MODULE;
    g_pMTKM4U_CharDrv->ops = &g_stMTK_M4U_fops;
    ret = cdev_add(g_pMTKM4U_CharDrv, g_MTKM4Udevno, 1);	

     */

    m4u_entry = create_proc_entry("M4U_device", 0, NULL);
    if(m4u_entry)
    {
        m4u_entry -> proc_fops = &g_stMTK_M4U_fops;
    }


    m4u_name_init();
    
    // power on four larb to initialize registers
    for(i=0;i<M4U_CLIENT_MODULE_NUM;i++)
    {
    	  power_module_cnt[i] = 0;
    }
    power_total = 0;

    for(i=0;i<TOTAL_M4U_NUM;i++)
    {
	if(i==1)
		continue;
        m4u_power_on(m4u_get_module_by_index(i));
    }
            
    m4u_struct_init(); //init related structures

    m4u_mvaGraph_init();
        
    // add SMI reg init here
    SMI_reg_init();
    
    for(i=0;i<TOTAL_M4U_NUM;i++)
    {
    	  // m4u_dump_reg(m4u_get_module_by_index(i));
        // m4u_print_active_port(i);
        // m4u_memory_usage(true); 
    }
      
    spin_lock_init(&gM4uLock);

    m4u_dbg_fifo_init();
    
  
#ifdef CONFIG_HAS_EARLYSUSPEND
    register_early_suspend(&mtkfb_early_suspend_handler);
#endif

    init_timer(&perf_timer);  //for performance monitor

    //Set IRQ   
    if(request_irq(MT_SMI_LARB0_MMU_IRQ_ID , MTK_M4U_isr, IRQF_TRIGGER_HIGH, M4U_DEVNAME , NULL))
    {
        M4UERR("request M4U0 IRQ line failed\n");
        return -ENODEV;
    }


    //mt_irq_set_sens(MT_SMI_LARB0_MMU_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
    //mt_irq_set_polarity(MT_SMI_LARB0_MMU_IRQ_ID, MT65xx_POLARITY_HIGH);
//    mt6577_irq_unmask(MT_SMI_LARB0_MMU_IRQ_ID);
//    enable_irq(MT_SMI_LARB0_MMU_IRQ_ID);
    
    if(request_irq(MT_SMI_LARB1_MMU_IRQ_ID , MTK_M4U_isr, IRQF_TRIGGER_HIGH, M4U_DEVNAME , NULL))
    {
        M4UERR("request M4U1 IRQ line failed\n");
        return -ENODEV;
    }
    //mt_irq_set_sens(MT_SMI_LARB1_MMU_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
    //mt_irq_set_polarity(MT_SMI_LARB1_MMU_IRQ_ID, MT65xx_POLARITY_HIGH);
//    mt6577_irq_unmask(MT_SMI_LARB1_MMU_IRQ_ID);
//    enable_irq(MT_SMI_LARB1_MMU_IRQ_ID);
    
    if(request_irq(MT_SMI_LARB2_MMU_IRQ_ID , MTK_M4U_isr, IRQF_TRIGGER_HIGH, M4U_DEVNAME , NULL))
    {
        M4UERR("request M4U2 IRQ line failed\n");
        return -ENODEV;
    }
    //mt_irq_set_sens(MT_SMI_LARB2_MMU_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
    //mt_irq_set_polarity(MT_SMI_LARB2_MMU_IRQ_ID, MT65xx_POLARITY_HIGH);
//    mt6577_irq_unmask(MT_SMI_LARB2_MMU_IRQ_ID);
//    enable_irq(MT_SMI_LARB2_MMU_IRQ_ID);
    

#ifndef CONFIG_MT6577_FPGA
    if(request_irq(MT_LARB3_MMU_IRQ_ID , MTK_M4U_isr, IRQF_TRIGGER_HIGH, M4U_DEVNAME , NULL))
    {
        M4UERR("request M4U3 IRQ line failed\n");
        return -ENODEV;
    }
    //mt_irq_set_sens(MT_LARB3_MMU_IRQ_ID, MT65xx_LEVEL_SENSITIVE);
    //mt_irq_set_polarity(MT_LARB3_MMU_IRQ_ID, MT65xx_POLARITY_HIGH);
    //mt6577_irq_unmask(MT_LARB3_MMU_IRQ_ID);
//    enable_irq(MT_LARB3_MMU_IRQ_ID);
#endif


#ifdef MAU_MONITOR_ON
    {
        M4U_MAU_STRUCT M4uMau;
        for(i=0;i<TOTAL_M4U_NUM;i++)
        {
        	M4uMau.LarbID = i;
            M4uMau.TagID = 0;				
            M4uMau.InvalidMasterIDLow = 0xffffffff; 
            M4uMau.InvalidMasterIDHigh = 0xffffffff; 
            M4uMau.ReadEn = 1;
            M4uMau.WriteEn = 1;
            M4uMau.StartAddr = PageTablePA; 
            M4uMau.EndAddr = PageTablePA + PT_TOTAL_ENTRY_NUM*sizeof(unsigned int);
            M4uMau.VirtualEn = 0;    
            m4u_config_mau(&M4uMau); 
       }   	
    }    
#endif

    _m4u_g2d_init();


    // clock monitor enabled, so power off and wait for 
    // power manager's callback
#ifndef CONFIG_MT6577_FPGA
#ifdef CLOCK_MONITOR_CALLBACK
    for(i=0;i<TOTAL_M4U_NUM;i++)  // init backup registers
    {
        m4u_reg_backup(m4u_get_module_by_index(i));
    } 
    for(i=0;i<TOTAL_M4U_NUM;i++)
    {
	if(i==1)
		continue;
        m4u_power_off(m4u_get_module_by_index(i));
    }   
    M4UMSG("register_larb_monitor done* ! \n");
    register_larb_monitor(&larb_monitor_handler);
#endif    
#endif
    M4UMSG("init done\n");

    return 0;


}


static int m4u_remove(struct platform_device *pdev)
{
    M4UDBG("MT6577_M4U_Exit() \n");
    m4u_hw_reinit();
    
    cdev_del(g_pMTKM4U_CharDrv);
    unregister_chrdev_region(g_MTKM4Udevno, 1);

    //Release IRQ
    free_irq(MT_SMI_LARB0_MMU_IRQ_ID , NULL);
    free_irq(MT_SMI_LARB1_MMU_IRQ_ID , NULL);
    free_irq(MT_SMI_LARB2_MMU_IRQ_ID , NULL);
    free_irq(MT_LARB3_MMU_IRQ_ID , NULL);
        
#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&mtkfb_early_suspend_handler);
#endif    

    return 0;
}

static int m4u_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

static int m4u_resume(struct platform_device *pdev)
{
    return 0;
}

static struct platform_driver m4uDrv = {
    .probe	= m4u_probe,
    .remove	= m4u_remove,
    .suspend = m4u_suspend,
    .resume	= m4u_resume,
    .driver	= {
    .name	= M4U_DEVNAME,
    .owner	= THIS_MODULE,
}
};



static int __init MTK_M4U_Init(void)
{
    if(platform_driver_register(&m4uDrv)){
        M4UMSG("failed to register MAU driver");
        return -ENODEV;
    }

	return 0;
}



static void __exit MTK_M4U_Exit(void)
{
    platform_driver_unregister(&m4uDrv);
}

///> for kernel driver call directly
///> or user driver call through M4U library's ioctl
int m4u_dump_reg(M4U_MODULE_ID_ENUM eModuleID) 
{
    // M4U related
    unsigned int i=0;
    unsigned int m4u_base = gM4UBaseAddr[m4u_get_index_by_module(eModuleID)];
    unsigned int isCalled=0;
    
    M4U_POW_ON_TRY(eModuleID, &isCalled);
    M4UMSG(" M4U Register Start ======= index=%d \n", m4u_get_index_by_module(eModuleID));
    for(i=0;i<M4U_REG_SIZE/8;i+=4)
    {
    	M4UMSG("+0x%03x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x \n", 0x800+8*i, 
    	M4U_ReadReg32(m4u_base, M4UMACRO0005 + 8*i + 4*0), M4U_ReadReg32(m4u_base, M4UMACRO0005 + 8*i + 4*1),
    	M4U_ReadReg32(m4u_base, M4UMACRO0005 + 8*i + 4*2), M4U_ReadReg32(m4u_base, M4UMACRO0005 + 8*i + 4*3),
    	M4U_ReadReg32(m4u_base, M4UMACRO0005 + 8*i + 4*4), M4U_ReadReg32(m4u_base, M4UMACRO0005 + 8*i + 4*5),
    	M4U_ReadReg32(m4u_base, M4UMACRO0005 + 8*i + 4*6), M4U_ReadReg32(m4u_base, M4UMACRO0005 + 8*i + 4*7));
    }
    M4UMSG(" M4U Register End ========== \n");
    
    //GMC related
    M4UMSG(" SMI Register Start ======= \n"); 	  
    M4UMSG("+0x804:0x%08x, 0x808:0x%08x, 0x80c:0x%08x, +0x2c:0x%08x, 0x200:0x%08x, 0x204:0x%08x \n", 
    	SMI_ReadReg32(M4UMACRO0194 ), SMI_ReadReg32(M4UMACRO0197 ),
    	SMI_ReadReg32(M4UMACRO0198 ), SMI_ReadReg32(M4UMACRO0199 ),
    	M4U_ReadReg32(m4u_base, M4UMACRO0159 ), M4U_ReadReg32(m4u_base, M4UMACRO0160 ));
    
    SMI_WriteReg32(M4UMACRO0199, 0xff);
    M4UMSG("after write: +0x004:0x%08x, 0x008:0x%08x, 0x00c:0x%08x, 0x200:0x%08x, 0x204:0x%08x \n", 
    	SMI_ReadReg32(M4UMACRO0194 ), SMI_ReadReg32(M4UMACRO0197 ),
    	SMI_ReadReg32(M4UMACRO0198 ),
    	M4U_ReadReg32(m4u_base, M4UMACRO0159 ), M4U_ReadReg32(m4u_base, M4UMACRO0160 ));    
    
    M4U_POW_OFF_TRY(eModuleID, isCalled);
    M4UMSG(" SMI Register End====== \n"); 	
    
      return 0;
}

int m4u_confirm_range_invalidated(int m4u_index, unsigned int MVAStart, unsigned int MVAEnd)
{
    unsigned int i;
    unsigned int RegAddr, Temp;
    volatile unsigned int RegValue;
    unsigned int m4u_base = gM4UBaseAddr[m4u_index];
    int result = 0;

    M4ULOG("m4u_confirm_range_invalidated, idx=0x%x, MVAStart=0x%x, MVAEnd=0x%x \n",
        m4u_index, MVAStart, MVAEnd);    

    if(gTestLevel==M4U_TEST_LEVEL_USER)
    {
    	  return 0;    	  
    }    
    
    ///> check Main TLB part
    RegAddr = M4UMACRO0040;
    for(i=0;i<g4M4UTagCount[m4u_index];i++)
    {
        RegValue = M4U_ReadReg32(m4u_base, RegAddr);
        Temp = RegValue & M4UMACRO0011; //bit 10
        if(Temp != 0)  ///> a valid Main TLB entry
        {
            unsigned int mva = RegValue & M4UMACRO0009;
            if(mva<=MVAEnd && mva>=MVAStart)
            {
            	  if(gTestLevel==M4U_TEST_LEVEL_STRESS)
            	  {
            	  	M4UWARN("main: i=%d, idx=0x%x, MVAStart=0x%x, MVAEnd=0x%x, RegValue=0x%x, larb_on=%d \n",
                        i, m4u_index, MVAStart, MVAEnd, RegValue, larb_is_on(m4u_index));
                    //m4u_dump_reg(m4u_get_module_by_index(m4u_index)); 
                }
                else if(gTestLevel==M4U_TEST_LEVEL_ENG)
            	  {
            	  	M4UMSG("main: i=%d, idx=0x%x, MVAStart=0x%x, MVAEnd=0x%x, RegValue=0x%x, larb_on=%d  \n",
                        i, m4u_index, MVAStart, MVAEnd, RegValue, larb_is_on(m4u_index));
                }
                result = -1;
            }
        }
        RegAddr += 4;
    }

    if(result < 0)
        return result;

    ///> check Prefetch TLB part
    RegAddr = M4UMACRO0072;
    for(i=0;i<g4M4UTagCount[m4u_index];i++)
    {
        RegValue = M4U_ReadReg32(m4u_base, RegAddr);
        Temp = RegValue & (3<<11);  //bit 11~12
        if(Temp != 0)  ///> a valid Prefetch TLB entry
        {
            unsigned int mva = RegValue & 0xffffe000;
            if(mva<=(MVAEnd&0xffffe000) && mva>=(MVAStart&0xffffe000))
            {
            	  if(gTestLevel==M4U_TEST_LEVEL_STRESS)
            	  {
            	  	M4UWARN("prefetch: i=%d, idx=0x%x, MVAStart=0x%x, MVAEnd=0x%x, RegValue=0x%x, larb_on=%d \n",
                        i, m4u_index, MVAStart, MVAEnd, RegValue, larb_is_on(m4u_index));
                    //m4u_dump_reg(m4u_get_module_by_index(m4u_index)); 
                }
                else if(gTestLevel==M4U_TEST_LEVEL_ENG)
            	  {
            	  	M4UMSG("prefetch: i=%d, idx=0x%x, MVAStart=0x%x, MVAEnd=0x%x, RegValue=0x%x, larb_on=%d  \n",
                        i, m4u_index, MVAStart, MVAEnd, RegValue, larb_is_on(m4u_index));
                }
                result = -1;
            }
        }
        RegAddr += 4;
    }
    
    return result;
}

int m4u_dump_main_tlb(int index) 
{
    // M4U related
    unsigned int i=0;
    unsigned int m4u_base = gM4UBaseAddr[index];
    M4UMSG("dump main tlb=======>");
    for(i=0;i<32;i++)
    {
        printk("%d : 0x%x : 0x%x  ", i,
            M4U_ReadReg32(m4u_base, M4UMACRO0040+4*i),
            m4u_get_descriptor(m4u_base, M4U_DESC_MAIN_TLB, i));
        
        if((i+1)%4==0)
            printk("\n");
    }
    
    return 0;
}

int m4u_dump_info(M4U_MODULE_ID_ENUM eModuleID) 
{
    unsigned int i=0;

#ifdef MTK_M4U_DBG 
    unsigned int j=0;
    unsigned int Desc[6];
    unsigned int m4u_base = gM4UBaseAddr[m4u_get_index_by_module(eModuleID)];
  
    M4UMSG(" Main Desc: \n");
    for(i=0;i<4;i++)
    {
        for(j=0;j<6;j++)
        {
        	Desc[j] = m4u_get_descriptor(m4u_base, M4U_DESC_MAIN_TLB, 6*i+j); //0:means main tlb
        }
        M4UMSG(" 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n", 
        Desc[0], Desc[1], Desc[2], Desc[3], Desc[4], Desc[5]);
    }

    M4UMSG(" Prefetch Desc LSB: \n");
    for(i=0;i<4;i++)
    {
        for(j=0;j<6;j++)
        {
        	Desc[j] = m4u_get_descriptor(m4u_base, M4U_DESC_PRE_TLB_LSB, 6*i+j);  //1:means pre-fetch tlb
        }
        M4UMSG(" 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n", 
        Desc[0], Desc[1], Desc[2], Desc[3], Desc[4], Desc[5]);
    }

    M4UMSG(" Prefetch Desc MSB: \n");
    for(i=0;i<4;i++)
    {
        for(j=0;j<6;j++)
        {
        	Desc[j] = m4u_get_descriptor(m4u_base, M4U_DESC_PRE_TLB_MSB, 6*i+j);  //1:means pre-fetch tlb
        }
        M4UMSG(" 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n", 
        Desc[0], Desc[1], Desc[2], Desc[3], Desc[4], Desc[5]);
    }
#endif


    M4UMSG(" MVA Range Info: \n");
    for(i=0;i<TOTAL_RANGE_NUM;i++)
    {
        if(1==pRangeDes[i].Enabled)
        {
            M4UMSG("pRangeDes[%d]: Enabled=%d, module=%s, MVAStart=0x%x, MVAEnd=0x%x \n", 
                i, pRangeDes[i].Enabled, m4u_get_module_name(pRangeDes[i].eModuleID), 
                pRangeDes[i].MVAStart, pRangeDes[i].MVAEnd);
        }
    }    

    M4UMSG(" Wrap Range Info: \n");
    for(i=0;i<TOTAL_WRAP_NUM;i++)
    {
        if(1==pWrapDes[i].Enabled)
        {
            M4UMSG("pWrapDes[%d]: Enabled=%d, module=%s, MVAStart=0x%x, MVAEnd=0x%x \n", 
                i, pWrapDes[i].Enabled, m4u_get_module_name(pWrapDes[i].eModuleID), 
                pWrapDes[i].MVAStart, pWrapDes[i].MVAEnd);
        }
    } 

    M4UMSG("dump pagetable for this module\n");
    m4u_dump_pagetable(eModuleID);
        
    return 0;
}

void m4u_dump_pagetable_range(unsigned int vaStart, unsigned int nr)
{
    unsigned int *pteStart;
    int i;

    pteStart = mva_pteAddr(vaStart);
    vaStart &= ~0xfff;

    // printk("m4u dump pagetable by range: start=0x%x, nr=%d ==============>\n", vaStart, nr);
    // printk("index : mva : PTE\n");
    printk("\n 0x%08x: ", vaStart);
    for(i=0; i<nr; i++)
    {
        printk("0x%08x, ", pteStart[i]);
        if((i+1)%8==0)
        {
        	 printk("\n 0x%08x: ", (vaStart+((i+1)<<12)));
        }
    }
    printk("\n");
    // printk("m4u dump pagetable done : start=0x%x, nr=%d ==============<\n", vaStart, nr);
}



int m4u_dump_pagetable(M4U_MODULE_ID_ENUM eModuleID)
{
    unsigned int addr=0;
    short index=1, nr=0;

    
    printk("[M4U_K] dump pagetable by module: %s, page_num=%d ========>\n", 
        m4u_get_module_name(eModuleID), pmodule_locked_pages[eModuleID]);

//  this function may be called in ISR
//  spin_lock(&gMvaGraph_lock);
    for(index=1; index<MVA_MAX_BLOCK_NR+1; index += nr)
    {
        addr = index << MVA_BLOCK_SIZE_ORDER;
        nr = MVA_GET_NR(index);
        if(MVA_IS_BUSY(index) && moduleGraph[index] == eModuleID)
        {
            // printk("start a mva region for module %d===>\n", eModuleID);
            m4u_dump_pagetable_range(addr, ((nr<<MVA_BLOCK_SIZE_ORDER)>>12));
        }
    }

//  spin_unlock(&gMvaGraph_lock);
    printk("[M4U_K]  dump pagetable by module done =========================<\n");

    return 0;
}


int m4u_dump_pagetable_nearby(M4U_MODULE_ID_ENUM eModuleID, unsigned int mva_addr)
{
    unsigned int addr=0;
    short index=1, nr=0;

    
    printk("[M4U_K] dump pagetable by module: %s, page_num=%d ========>\n", 
        m4u_get_module_name(eModuleID), pmodule_locked_pages[eModuleID]);

//  this function may be called in ISR
//  spin_lock(&gMvaGraph_lock);
    for(index=1; index<MVA_MAX_BLOCK_NR+1; index += nr)
    {
        addr = index << MVA_BLOCK_SIZE_ORDER;
        nr = MVA_GET_NR(index);
        if(MVA_IS_BUSY(index) && 
		   moduleGraph[index] == eModuleID && 
		  ((mva_addr>=addr && mva_addr-addr<=2*DEFAULT_PAGE_SIZE)||
		   (mva_addr<=addr && addr-mva_addr<=2*DEFAULT_PAGE_SIZE))   )
        {
            // printk("start a mva region for module %d===>\n", eModuleID);
            m4u_dump_pagetable_range(addr, ((nr<<MVA_BLOCK_SIZE_ORDER)>>12));
        }
    }

//  spin_unlock(&gMvaGraph_lock);
    printk("[M4U_K]  dump pagetable by module done =========================<\n");

    return 0;
}



int m4u_dump_mva_info()
{
    short index=1, nr=0;
    unsigned int addr=0;
    
    M4UMSG(" dump mva allocated info ========>\n");
    M4UMSG("module   mva_start  mva_end  page_num  \n");
    for(index=1; index<MVA_MAX_BLOCK_NR+1; index += nr)
    {
        addr = index << MVA_BLOCK_SIZE_ORDER;
        nr = MVA_GET_NR(index);
        if(MVA_IS_BUSY(index))
        {            
            M4UMSG("%s, 0x%-8x, 0x%-8x, %d  \n", 
                m4u_get_module_name(moduleGraph[index]), addr, addr+nr*MVA_BLOCK_SIZE, nr);
        }
    }
    M4UMSG(" dump mva allocated info done ========>\n");
    
    return 0;
}

int _do_m4u_power_on(int index, const char *module_name)
{
  char name[30];
  sprintf(name, "m4u+%s", module_name);  
  
  M4ULOG("m4u_power_on() module=%s, index=%d \n", name, index);

#ifndef CONFIG_MT6577_FPGA
  switch(index)
  {
      case 0: 
      	   enable_clock(MT65XX_PDN_MM_SMI_LARB0, name);
      	   enable_clock(MT65XX_PDN_MM_SMI_LARB0_EMI, name);
      	   break;
      case 1:
      	   enable_clock(MT65XX_PDN_MM_SMI_LARB1, name);
      	   enable_clock(MT65XX_PDN_MM_SMI_LARB1_EMI, name);
      	   break;
      case 2: 
      	   enable_clock(MT65XX_PDN_MM_SMI_LARB2_ACP_BUS, name);
      	   enable_clock(MT65XX_PDN_MM_SMI_LARB2_260MHZ, name);
      	   enable_clock(MT65XX_PDN_MM_SMI_LARB2_EMI, name);
      	   enable_clock(MT65XX_PDN_MM_SMI_LARB2_ACP_BUS_EMI, name);
      	   enable_clock(MT65XX_PDN_MM_SMI_LARB2, name);
      	   break;
      case 3: 
      	   enable_clock(MT65XX_PDN_MM_SMI_LARB3_FULL, name);
      	   enable_clock(MT65XX_PDN_MM_SMI_LARB3_HALF, name);
      	   break;
      default: 
      	M4UMSG("m4u_power_on(), invalid index=%d \n", index);
  }
#endif 
  
  return 0;
}


int m4u_power_on(M4U_MODULE_ID_ENUM eModuleID) 
{
    int index = m4u_get_index_by_module(eModuleID);

    _do_m4u_power_on(index, m4u_get_module_name(eModuleID));

    mutex_lock(&gM4uMutexPower);
    power_module_cnt[eModuleID]++;
    power_total++;
    mutex_unlock(&gM4uMutexPower);

    return 0;
}

int _do_m4u_power_off(int index, const char *module_name)
{
  char name[30];
  sprintf(name, "m4u+%s", module_name);
  
  M4ULOG("m4u_power_off() module=%s, index=%d \n", name, index);

#ifndef CONFIG_MT6577_FPGA

  switch(index)
  {
      case 0: 
      	   disable_clock(MT65XX_PDN_MM_SMI_LARB0, name);
      	   disable_clock(MT65XX_PDN_MM_SMI_LARB0_EMI, name);
      	   break;
      case 1:
      	   disable_clock(MT65XX_PDN_MM_SMI_LARB1, name);
      	   disable_clock(MT65XX_PDN_MM_SMI_LARB1_EMI, name);
      	   break;
      case 2: 
      	   disable_clock(MT65XX_PDN_MM_SMI_LARB2_ACP_BUS, name);
      	   disable_clock(MT65XX_PDN_MM_SMI_LARB2_260MHZ, name);
      	   disable_clock(MT65XX_PDN_MM_SMI_LARB2_EMI, name);
      	   disable_clock(MT65XX_PDN_MM_SMI_LARB2_ACP_BUS_EMI, name);
      	   disable_clock(MT65XX_PDN_MM_SMI_LARB2, name);
      	   break;
      case 3: 
      	   disable_clock(MT65XX_PDN_MM_SMI_LARB3_FULL, name);
      	   disable_clock(MT65XX_PDN_MM_SMI_LARB3_HALF, name);
      	   break;
      default: 
      	M4UMSG("m4u_power_off(), invalid index=%d \n", index);
  }
#endif
  
  return 0;

}

int m4u_power_off(M4U_MODULE_ID_ENUM eModuleID) 
{
    int index = m4u_get_index_by_module(eModuleID);

    _do_m4u_power_off(index, m4u_get_module_name(eModuleID));

    mutex_lock(&gM4uMutexPower);
    power_module_cnt[eModuleID]--;
    if(power_module_cnt[eModuleID]<0)
    {
    	M4UMSG("error: module %s power off M4U, but not power on before \n", name);
    }
    power_total--;
    mutex_unlock(&gM4uMutexPower);	

    return 0;
}


static void m4u_mvaGraph_init(void)
{
    spin_lock(&gMvaGraph_lock);
    memset(mvaGraph, 0, sizeof(short)*(MVA_MAX_BLOCK_NR+1));
    memset(moduleGraph, 0, sizeof(unsigned char)*(MVA_MAX_BLOCK_NR+1));
    mvaGraph[0] = 1|MVA_BUSY_MASK;
    moduleGraph[0] = M4U_CLNTMOD_UNKNOWN;
    mvaGraph[1] = MVA_MAX_BLOCK_NR;
    moduleGraph[1] = M4U_CLNTMOD_UNKNOWN;
    mvaGraph[MVA_MAX_BLOCK_NR] = MVA_MAX_BLOCK_NR;
    moduleGraph[MVA_MAX_BLOCK_NR] = M4U_CLNTMOD_UNKNOWN;
    
    spin_unlock(&gMvaGraph_lock);
}

void m4u_mvaGraph_dump_raw(void)
{
    int i;
    spin_lock(&gMvaGraph_lock);
    printk("[M4U_K] dump raw data of mvaGraph:============>\n");
    for(i=0; i<MVA_MAX_BLOCK_NR+1; i++)
        printk("0x%4x: 0x%08x   ID:%d\n", i, mvaGraph[i], moduleGraph[i]); 
    spin_unlock(&gMvaGraph_lock);
}


void m4u_mvaGraph_dump(void)
{
    unsigned int addr=0, size=0;
    short index=1, nr=0;
    M4U_MODULE_ID_ENUM moduleID;
    char *pMvaFree = "FREE";
    char *pErrorId = "ERROR";
    char *pOwner = NULL;
    int i,max_bit;    
    short frag[12] = {0};
    short nr_free=0, nr_alloc=0;
    
    printk("[M4U_K] mva allocation info dump:====================>\n");
    printk("start      size     blocknum    owner       \n");

    spin_lock(&gMvaGraph_lock);
    for(index=1; index<MVA_MAX_BLOCK_NR+1; index += nr)
    {
        addr = index << MVA_BLOCK_SIZE_ORDER;
        nr = MVA_GET_NR(index);
        size = nr << MVA_BLOCK_SIZE_ORDER;
        if(MVA_IS_BUSY(index))
        {
            moduleID = (M4U_MODULE_ID_ENUM)moduleGraph[index];
            if(moduleID > M4U_CLIENT_MODULE_NUM-1)
                pOwner = pErrorId;
            else
                pOwner = m4u_get_module_name(moduleID);
            nr_alloc += nr;
        }
        else    // mva region is free
        {
            pOwner = pMvaFree;
            nr_free += nr;

            max_bit=0;
            for(i=0; i<12; i++)
            {
                if(nr & (1<<i))
                    max_bit = i;
            }
            frag[max_bit]++; 
        }

        printk("0x%08x  0x%08x  %4d    %s\n", addr, size, nr, pOwner);

     }

    spin_unlock(&gMvaGraph_lock);

    printk("\n");
    printk("[M4U_K] mva alloc summary: (unit: blocks)========================>\n");
    printk("free: %d , alloc: %d, total: %d \n", nr_free, nr_alloc, nr_free+nr_alloc);
    printk("[M4U_K] free region fragments in 2^x blocks unit:===============\n");
    printk("  0     1     2     3     4     5     6     7     8     9     10    11    \n");
    printk("%4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d  %4d  \n",
        frag[0],frag[1],frag[2],frag[3],frag[4],frag[5],frag[6],frag[7],frag[8],frag[9],frag[10],frag[11]);
    printk("[M4U_K] mva alloc dump done=========================<\n");
    
}

static M4U_MODULE_ID_ENUM mva2module(unsigned int mva)
{

    M4U_MODULE_ID_ENUM eModuleId = M4U_CLNTMOD_UNKNOWN;
    int index;

    index = MVAGRAPH_INDEX(mva);
    if(index==0 || index>MVA_MAX_BLOCK_NR)
    {
        M4UMSG("mvaGraph index is 0. mva=0x%x\n", mva);
        return M4U_CLNTMOD_UNKNOWN;
    }
    
    spin_lock(&gMvaGraph_lock);

    //find prev head/tail of this region
    while(mvaGraph[index]==0)
        index--;

    if(MVA_IS_BUSY(index))
    {
        eModuleId = moduleGraph[index];
        goto out;
    }
    else
    {
        eModuleId = M4U_CLNTMOD_UNKNOWN;
        goto out;
    }                    

out:    
    spin_unlock(&gMvaGraph_lock);
    return eModuleId;
    
}


static unsigned int m4u_do_mva_alloc(M4U_MODULE_ID_ENUM eModuleID, 
								  const unsigned int BufAddr, 
								  const unsigned int BufSize)
{
    short s,end;
    short new_start, new_end;
    short nr = 0;
    unsigned int mvaRegionStart;
    unsigned int startRequire, endRequire, sizeRequire; 

    if(BufSize == 0) return 0;

    ///-----------------------------------------------------
    ///calculate mva block number
    startRequire = BufAddr & (~M4U_PAGE_MASK);
    endRequire = (BufAddr+BufSize-1)| M4U_PAGE_MASK;
    sizeRequire = endRequire-startRequire+1;
    nr = (sizeRequire+MVA_BLOCK_ALIGN_MASK)>>MVA_BLOCK_SIZE_ORDER;//(sizeRequire>>MVA_BLOCK_SIZE_ORDER) + ((sizeRequire&MVA_BLOCK_ALIGN_MASK)!=0);

    spin_lock(&gMvaGraph_lock);

    ///-----------------------------------------------
    ///find first match free region
    for(s=1; (s<(MVA_MAX_BLOCK_NR+1))&&(mvaGraph[s]<nr); s+=(mvaGraph[s]&MVA_BLOCK_NR_MASK))
        ;
    if(s > MVA_MAX_BLOCK_NR)
    {
        spin_unlock(&gMvaGraph_lock);
        M4UERR("mva_alloc error: no available MVA region for %d blocks!\n", nr);
        return 0;
    }

    ///-----------------------------------------------
    ///alloc a mva region
    end = s + mvaGraph[s] - 1;

    if(unlikely(nr == mvaGraph[s]))
    {
        MVA_SET_BUSY(s);
        MVA_SET_BUSY(end);
        moduleGraph[s] = eModuleID;
        moduleGraph[end] = eModuleID;
    }
    else
    {
        new_end = s + nr - 1;
        new_start = new_end + 1;
        //note: new_start may equals to end
        mvaGraph[new_start] = (mvaGraph[s]-nr);
        mvaGraph[new_end] = nr | MVA_BUSY_MASK;
        mvaGraph[s] = mvaGraph[new_end];
        mvaGraph[end] = mvaGraph[new_start];

        moduleGraph[s] = eModuleID;
        moduleGraph[new_end] = eModuleID;
        //moduleGraph[new_start] = M4U_CLNTMOD_UNKNOWN;
        //moduleGraph[end] = M4U_CLNTMOD_UNKNOWN;
    }

    spin_unlock(&gMvaGraph_lock);

    mvaRegionStart = (unsigned int)s;
    
    return (mvaRegionStart<<MVA_BLOCK_SIZE_ORDER) + mva_pageOffset(BufAddr);

}



#define RightWrong(x) ( (x) ? "correct" : "error")
static int m4u_do_mva_free(M4U_MODULE_ID_ENUM eModuleID, 
                                const unsigned int BufAddr,
								const unsigned int BufSize,
								unsigned int mvaRegionStart) 
{
    short startIdx = mvaRegionStart >> MVA_BLOCK_SIZE_ORDER;
    short nr = mvaGraph[startIdx] & MVA_BLOCK_NR_MASK;
    short endIdx = startIdx + nr - 1;
    unsigned int startRequire, endRequire, sizeRequire;
    short nrRequire;


    spin_lock(&gMvaGraph_lock);
    ///--------------------------------
    ///check the input arguments
    ///right condition: startIdx is not NULL && region is busy && right module && right size 
    startRequire = BufAddr & (~M4U_PAGE_MASK);
    endRequire = (BufAddr+BufSize-1)| M4U_PAGE_MASK;
    sizeRequire = endRequire-startRequire+1;
    nrRequire = (sizeRequire+MVA_BLOCK_ALIGN_MASK)>>MVA_BLOCK_SIZE_ORDER;//(sizeRequire>>MVA_BLOCK_SIZE_ORDER) + ((sizeRequire&MVA_BLOCK_ALIGN_MASK)!=0);
    if(!(   startIdx != 0           //startIdx is not NULL
            && MVA_IS_BUSY(startIdx)               // region is busy
            && (moduleGraph[startIdx]==eModuleID) //right module
            && (nr==nrRequire)       //right size
          )
       )
    {

        spin_unlock(&gMvaGraph_lock);
        M4UERR("error to free mva========================>\n");
        M4UMSG("ModuleID=%s (expect %s) [%s]\n", 
                m4u_get_module_name(eModuleID), m4u_get_module_name(moduleGraph[startIdx]),RightWrong(eModuleID==moduleGraph[startIdx]));
        M4UMSG("BufSize=%d(unit:0x%xBytes) (expect %d) [%s]\n", 
                nrRequire, MVA_BLOCK_SIZE, nr, RightWrong(nrRequire==nr));
        M4UMSG("mva=0x%x, (IsBusy?)=%d (expect %d) [%s]\n",
                mvaRegionStart, MVA_IS_BUSY(startIdx),1, RightWrong(MVA_IS_BUSY(startIdx)));
        m4u_mvaGraph_dump();
        //m4u_mvaGraph_dump_raw();
        return -1;
    }


    ///--------------------------------
    ///merge with followed region
    if( (endIdx+1 <= MVA_MAX_BLOCK_NR)&&(!MVA_IS_BUSY(endIdx+1)))
    {
        nr += mvaGraph[endIdx+1];
        mvaGraph[endIdx] = 0;
        mvaGraph[endIdx+1] = 0;
    }

    ///--------------------------------
    ///merge with previous region
    if( (startIdx-1>0)&&(!MVA_IS_BUSY(startIdx-1)) )
    {
        int pre_nr = mvaGraph[startIdx-1];
        mvaGraph[startIdx] = 0;
        mvaGraph[startIdx-1] = 0;
        startIdx -= pre_nr;
        nr += pre_nr;
    }
    ///--------------------------------
    ///set region flags
    mvaGraph[startIdx] = nr;
    mvaGraph[startIdx+nr-1] = nr;

    spin_unlock(&gMvaGraph_lock);
    return 0;    

}


// set the size to 0, because TVC owner has find the error root cause and resolved
// left the code for debug use in the feature
void m4u_profile_init(void)
{
    MMP_Event M4U_Event;
    M4U_Event = MMProfileRegisterEvent(MMP_RootEvent, "M4U");
    M4U_MMP_Events[PROFILE_ALLOC_MVA] = MMProfileRegisterEvent(M4U_Event, "Alloc MVA");
    M4U_MMP_Events[PROFILE_ALLOC_MVA_REGION] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_ALLOC_MVA], "Alloc MVA Region");
    M4U_MMP_Events[PROFILE_GET_PAGES] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_ALLOC_MVA], "Get Pages");
    M4U_MMP_Events[PROFILE_FOLLOW_PAGE] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_GET_PAGES], "Follow Page");
    M4U_MMP_Events[PROFILE_FORCE_PAGING] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_GET_PAGES], "Force Paging");
    M4U_MMP_Events[PROFILE_MLOCK] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_GET_PAGES], "MLock");
    M4U_MMP_Events[PROFILE_ALLOC_FLUSH_TLB] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_ALLOC_MVA], "Alloc Flush TLB");
    M4U_MMP_Events[PROFILE_DEALLOC_MVA] = MMProfileRegisterEvent(M4U_Event, "DeAlloc MVA");
    M4U_MMP_Events[PROFILE_RELEASE_PAGES] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DEALLOC_MVA], "Release Pages");
    M4U_MMP_Events[PROFILE_MUNLOCK] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_RELEASE_PAGES], "MUnLock");
    M4U_MMP_Events[PROFILE_PUT_PAGE] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_RELEASE_PAGES], "Put Page");
    M4U_MMP_Events[PROFILE_RELEASE_MVA_REGION] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DEALLOC_MVA], "Release MVA Region");
    M4U_MMP_Events[PROFILE_QUERY] = MMProfileRegisterEvent(M4U_Event, "Query MVA");
    M4U_MMP_Events[PROFILE_INSERT_TLB] = MMProfileRegisterEvent(M4U_Event, "Insert TLB");
    M4U_MMP_Events[PROFILE_DMA_MAINT_ALL] = MMProfileRegisterEvent(M4U_Event, "Cache Maintain");
    M4U_MMP_Events[PROFILE_DMA_CLEAN_RANGE] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], "Clean Range");
    M4U_MMP_Events[PROFILE_DMA_CLEAN_ALL] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], "Clean All");
    M4U_MMP_Events[PROFILE_DMA_INVALID_RANGE] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], "Invalid Range");
    M4U_MMP_Events[PROFILE_DMA_INVALID_ALL] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], "Invalid All");
    M4U_MMP_Events[PROFILE_DMA_FLUSH_RANGE] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], "Flush Range");
    M4U_MMP_Events[PROFILE_DMA_FLUSH_ALL] = MMProfileRegisterEvent(M4U_MMP_Events[PROFILE_DMA_MAINT_ALL], "Flush All");
    M4U_MMP_Events[PROFILE_CACHE_FLUSH_ALL] = MMProfileRegisterEvent(M4U_Event, "Cache Flush All");
    M4U_MMP_Events[PROFILE_CONFIG_PORT] = MMProfileRegisterEvent(M4U_Event, "Config Port");
    M4U_MMP_Events[PROFILE_MAIN_TLB_MON] = MMProfileRegisterEvent(M4U_Event, "Main TLB Monitor");
    M4U_MMP_Events[PROFILE_PREF_TLB_MON] = MMProfileRegisterEvent(M4U_Event, "PreFetch TLB Monitor");
    M4U_MMP_Events[PROFILE_M4U_REG] = MMProfileRegisterEvent(M4U_Event, "M4U Registers");
}

void m4u_profile_dump_reg(void)
{
    unsigned int i;
    int isCalled[4];
    MMP_MetaData_t MetaData;
    char* RegBuf;
    char* pM4UReg;
    int m4u_index;
    if (MMProfileQueryEnable(M4U_MMP_Events[PROFILE_M4U_REG]) == 0)
        return;
    if (in_interrupt())
        return;
    RegBuf = vmalloc(0x4000);
    memset(RegBuf, 0, 0x4000);
    for (i=0; i<4; i++)
        M4U_POW_ON_TRY(m4u_get_module_by_index(i), &isCalled[i]);
    for (m4u_index=0; m4u_index<4; m4u_index++)
    {
        pM4UReg = RegBuf + m4u_index * 0x1000;
        for (i=0; i<0xC04; i+=4)
            *(unsigned int*)(pM4UReg+i) = M4U_ReadReg32(gM4UBaseAddr[m4u_index], i);
    }
    MetaData.data_type = MMProfileMetaUserM4UReg;
    MetaData.size = 0x4000;
    MetaData.pData = RegBuf;
    MMProfileLogMeta(M4U_MMP_Events[PROFILE_M4U_REG], MMProfileFlagPulse, &MetaData);
    vfree(RegBuf);
    for (i=0; i<4; i++)
        M4U_POW_OFF_TRY(m4u_get_module_by_index(i), isCalled[i]);
    M4UMSG("[M4U_TEMP]: m4u_profile_dump_reg is executed!\n");
}

// query mva by va
int m4u_query_mva(M4U_MODULE_ID_ENUM eModuleID, 
                                struct task_struct *tsk,
								  const unsigned int BufAddr, 
								  const unsigned int BufSize, 
								  unsigned int *pRetMVABuf,
								  struct file * a_pstFile) 
{
    struct list_head *pListHead;
    garbage_list_t *pList = NULL;
    garbage_node_t *pNode = (garbage_node_t*)(a_pstFile->private_data);
    unsigned int query_start = BufAddr;
    unsigned int query_end = BufAddr + BufSize - 1;
    unsigned int s,e;
    int ret, err = 0;
    m4u_dbg_event_t event = M4U_EVT_UNKOWN;

    *pRetMVABuf = 0;                 
    
    if(pNode==NULL)
    {
        M4UMSG("error: m4u_query_mva, pNode is NULL, va=0x%x, module=%s! \n", BufAddr, m4u_get_module_name(eModuleID));
        return -1;
    }  
    MMProfileLogEx(M4U_MMP_Events[PROFILE_QUERY], MMProfileFlagStart, eModuleID, BufAddr);
    
    mutex_lock(&(pNode->dataMutex));              
    list_for_each(pListHead, &(pNode->mvaList))
    {
        pList = container_of(pListHead, garbage_list_t, link);
        s = pList->bufAddr;
        e = s + pList->size - 1;
        M4UDBG("pList-tsk=0x%x, tsk=0x%x,pid=%d,%d  ppid=%d,%d tgid=%d,%d, name=%s,%s va=0x%x\n", 
            pList->tsk, tsk, pList->tsk->pid, tsk->pid,
            pList->tsk->parent->pid, tsk->parent->pid,
            pList->tsk->tgid, tsk->tgid,
            pList->tsk->comm, tsk->comm,
            pList->bufAddr);
			
			
        if( ((pList->tgid == tsk->tgid))&&
            (pList->eModuleId==eModuleID) &&
        	 (query_start>=s && query_end<=e))
        {
            if(pList->mvaStart > 0) //here we have allocated mva for this buffer
            {
                *pRetMVABuf = pList->mvaStart + (query_start-s);
                event = M4U_EVT_QUERY_MVA;
            }
            else    // here we have not allocated mva (this buffer is registered, and query for first time)
            {
                M4U_ASSERT(pList->flags&MVA_REGION_REGISTER);
                //we should allocate mva for this buffer

                ret = m4u_alloc_mva_tsk(pList->eModuleId, pList->tsk, pList->bufAddr, 
                          pList->size, pRetMVABuf);
                if(ret)
                {
                	M4UMSG("m4u_alloc_mva failed when query for it: %d\n", ret);
                	err = -EFAULT;
                } 
                else
                {
                    pList->flags &= ~(MVA_REGION_REGISTER);
                    pList->mvaStart = *pRetMVABuf;
                    *pRetMVABuf = pList->mvaStart + (query_start-s);
                }
                event = M4U_EVT_QUERY_ALLOC;
                M4ULOG("allocate for first query: id=%s,tsk=0x%x, addr=0x%08x, size=%d, mva=0x%x \n", 
                      m4u_get_module_name(eModuleID), tsk, BufAddr,  BufSize, *pRetMVABuf);
            }
    		break;
        }
    }
    mutex_unlock(&(pNode->dataMutex));

    m4u_dbg_push(event, eModuleID, BufAddr, BufSize, *pRetMVABuf, tsk->tgid);

    MMProfileLogEx(M4U_MMP_Events[PROFILE_QUERY], MMProfileFlagEnd, eModuleID, BufSize);
    
    M4ULOG("m4u_query_mva: id=%s, tsk=0x%x, tsk.tgid=%d, addr=0x%08x, size=%d, mva=0x%x \n", 
                m4u_get_module_name(eModuleID), tsk, tsk->tgid, BufAddr,  BufSize, *pRetMVABuf);



    return err;

}




#define TVC_MVA_SAFE_MARGIN 0 //(4*1024*50)
static int m4u_invalidate_and_check(unsigned int m4u_index, unsigned int start, unsigned int end)
{
    unsigned int m4u_base = gM4UBaseAddr[m4u_index];
    unsigned int RegAddr = M4UMACRO0040;
    int i;
    unsigned int main_tlb[32], pfh_tlb[32];
    //save main/pfh tlb just for debug
    for(i=0;i<g4M4UTagCount[m4u_index];i++)
    {
        main_tlb[i] = M4U_ReadReg32(m4u_base, RegAddr);
        RegAddr += 4;
    }
    RegAddr = M4UMACRO0072;
    for(i=0;i<g4M4UTagCount[m4u_index];i++)
    {
        pfh_tlb[i] = M4U_ReadReg32(m4u_base, RegAddr);
        RegAddr += 4;
    }
 

    SMI_WriteReg32(M4UMACRO0197, (start)|(1<<m4u_index));
    SMI_WriteReg32(M4UMACRO0198, end);
    SMI_WriteReg32(M4UMACRO0194, M4UMACRO0196); 
    if(0!=m4u_confirm_range_invalidated(m4u_index, start, end)) // first time fail, invalidate range again
    {
        M4UMSG("invalidate range first fail! tlb saved is: \n");

        //dump main/pfh tlb saved for debug
        for(i=0; i<32; i++)
        {
            printk("%x,",main_tlb[i]);
        }
        printk("\n");
        
        for(i=0; i<32; i++)
        {
            printk("%x,",pfh_tlb[i]);
        }
        printk("\n");
        
        SMI_WriteReg32(M4UMACRO0197, (start)|(1<<m4u_index)); 
        SMI_WriteReg32(M4UMACRO0198, end);
        SMI_WriteReg32(M4UMACRO0194, M4UMACRO0196);            	
    	if(0!=m4u_confirm_range_invalidated(m4u_index, start, end)) // again failed, invalidate all
    	{
    		M4UMSG("invalidate range twice, also fail! \n");

            // last chance -- invalidate all
            _do_m4u_power_on(m4u_index, "inv_tlb");
    		SMI_WriteReg32(M4UMACRO0197, 1<<m4u_index);
            SMI_WriteReg32(M4UMACRO0194, M4UMACRO0195);
            if(0!=m4u_confirm_range_invalidated(m4u_index, start, end)) // invalidate all failed, die
            {
            	  M4UERR("invalidate all fail! \n");
            }
            _do_m4u_power_off(m4u_index, "inv_tlb");
    	}
    }
    return 0;	
}


#define TVC_MVA_SAFE_MARGIN 0 //(4*1024*50)
static int m4u_alloc_mva_dynamic(M4U_MODULE_ID_ENUM eModuleID,
                                  struct task_struct * tsk,
								  const unsigned int BufAddr, 
								  const unsigned int BufSize, 
								  unsigned int *pRetMVABuf) 
{

    unsigned int* pPageTableAddr = NULL;
    int page_num;
    unsigned int mvaStart;
    unsigned int actualMvaStart, actrualSize;
    
    MMProfileLogEx(M4U_MMP_Events[PROFILE_ALLOC_MVA_REGION], MMProfileFlagStart, eModuleID, BufAddr);
    ///------------------------------------------------------
    ///alloc a mva region for nrBlocks size
    // TVC HW issue workaround, TVC will prefetch up to 50 pages more than HW really should
    if(M4U_CLNTMOD_TVC==eModuleID)
    {
        actrualSize = BufSize+TVC_MVA_SAFE_MARGIN;
        actualMvaStart= m4u_do_mva_alloc(eModuleID, BufAddr, actrualSize); 
        mvaStart = actualMvaStart;
    }
    else if(M4U_CLNTMOD_LCDC_UI==eModuleID)  //LCD_UI workaround, allocate 2 pages more at front and end
    {
        actrualSize = BufSize + M4U_PAGE_SIZE*4;
        actualMvaStart = m4u_do_mva_alloc(eModuleID, BufAddr, actrualSize);
        mvaStart = actualMvaStart + M4U_PAGE_SIZE;
    }
    else if(M4U_CLNTMOD_LCDC==eModuleID)  //LCD_UI workaround, allocate 2 pages more at front and end
    {
        actrualSize = BufSize + M4U_PAGE_SIZE*2;
        actualMvaStart = m4u_do_mva_alloc(eModuleID, BufAddr, actrualSize);
        mvaStart = actualMvaStart + M4U_PAGE_SIZE;
    }
    else if(M4U_CLNTMOD_G2D==eModuleID)  //LCD_UI workaround, allocate 2 pages more at front and end
    {
        actrualSize = BufSize + M4U_PAGE_SIZE*2;
        actualMvaStart = m4u_do_mva_alloc(eModuleID, BufAddr, actrualSize);
        mvaStart = actualMvaStart + M4U_PAGE_SIZE;
    }    
    else if(M4U_CLNTMOD_CAM==eModuleID)  //LCD_UI workaround, allocate 2 pages more at front and end
    {
        actrualSize = BufSize + M4U_PAGE_SIZE*20;
        actualMvaStart = m4u_do_mva_alloc(eModuleID, BufAddr, actrualSize);
        mvaStart = actualMvaStart + M4U_PAGE_SIZE*10;
    }    

    else 
    {
        actrualSize = BufSize;
        actualMvaStart= m4u_do_mva_alloc(eModuleID, BufAddr, actrualSize); 
        mvaStart = actualMvaStart;
    }
    MMProfileLogEx(M4U_MMP_Events[PROFILE_ALLOC_MVA_REGION], MMProfileFlagEnd, mvaStart, BufSize);

    
    if(actualMvaStart == 0)
    {
        M4UERR("mva_alloc error: no available MVA region for %d bytes!\n", BufSize);
        m4u_mvaGraph_dump();
        *pRetMVABuf = 0;
        return -1;
    }

    M4U_mvaGraph_dump_DBG();

    mutex_lock(&gM4uMutex);    
    ///-------------------------------------------------------
    ///fill m4u page table of this mva Region
    pPageTableAddr = mva_pteAddr(mvaStart); // get offset in the page table
    MMProfileLogEx(M4U_MMP_Events[PROFILE_GET_PAGES], MMProfileFlagStart, eModuleID, BufAddr);
    page_num = m4u_get_pages(eModuleID, tsk, BufAddr, BufSize, pPageTableAddr);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_GET_PAGES], MMProfileFlagEnd, eModuleID, BufSize);
    if(page_num<=0)
    {
        M4UDBG("Error: m4u_get_pages failed \n");
        goto error_out;
    }
    

    if(M4U_CLNTMOD_TVC==eModuleID)
    {
       unsigned int j;
       unsigned int page_num = (BufSize + (BufAddr&0xfff))/DEFAULT_PAGE_SIZE;
       if((BufAddr+BufSize)&0xfff)
       {
           page_num++;
       }    
       
       for(j=0;j<TVC_MVA_SAFE_MARGIN/4096;j++)
       {
       	   *(pPageTableAddr + page_num + j) = *(pPageTableAddr);
       } 	  
       //m4u_dump_pagetable(M4U_CLNTMOD_TVC);       
    }
    if(M4U_CLNTMOD_LCDC_UI==eModuleID || M4U_CLNTMOD_LCDC==eModuleID || M4U_CLNTMOD_G2D==eModuleID)
    {    
        // copy the last PTE to next one, to avoid translation fault! 
        // WARNING: only LCDC_UI can use this function
        *(pPageTableAddr - 1) = *pPageTableAddr;
        *(pPageTableAddr+page_num) = *(pPageTableAddr+page_num-1);
    }
    if(M4U_CLNTMOD_CAM==eModuleID)
    {    
        int i;
        for(i=1; i<=0; i++)
        {
            *(pPageTableAddr - i) = *pPageTableAddr;
            *(pPageTableAddr+page_num-1+i) = *(pPageTableAddr+page_num-1);
        }
    }
        
    MMProfileLogEx(M4U_MMP_Events[PROFILE_ALLOC_FLUSH_TLB], MMProfileFlagStart, eModuleID, BufAddr);
    ///-------------------------------------------------------
    ///flush tlb entries in this mva range
    {
        unsigned int isCalled = 0;
        unsigned int start = actualMvaStart&(~(M4U_PAGE_SIZE-1));
        unsigned int end = (actualMvaStart+actrualSize-1+M4U_PAGE_SIZE)&(~(M4U_PAGE_SIZE-1));
        if(eModuleID!=M4U_CLNTMOD_RDMA_GENERAL && eModuleID!=M4U_CLNTMOD_ROT_GENERAL)
        {
            M4U_POW_ON_TRY(eModuleID, &isCalled);
            m4u_invalidate_and_check(m4u_get_index_by_module(eModuleID), start, end);            
            M4U_POW_OFF_TRY(eModuleID, isCalled);
        }
        else
        {
            //MDP general may in larb0 or larb1
            M4U_POW_ON_TRY(m4u_get_module_by_index(0), &isCalled);
            m4u_invalidate_and_check(0, start, end);            
            M4U_POW_OFF_TRY(m4u_get_module_by_index(0), isCalled);

            M4U_POW_ON_TRY(m4u_get_module_by_index(1), &isCalled);
            m4u_invalidate_and_check(1, start, end);
            M4U_POW_OFF_TRY(m4u_get_module_by_index(1), isCalled);
            
        }
    }
    MMProfileLogEx(M4U_MMP_Events[PROFILE_ALLOC_FLUSH_TLB], MMProfileFlagEnd, eModuleID, BufSize);

    mutex_unlock(&gM4uMutex);

    ///------------------------------------------------------
    *pRetMVABuf = mvaStart;

     return 0;

error_out:
    mutex_unlock(&gM4uMutex);
    m4u_do_mva_free(eModuleID, BufAddr, BufSize, mvaStart);
    *pRetMVABuf = 0;
    return -1;
    

}

static int m4u_dealloc_mva_dynamic(M4U_MODULE_ID_ENUM eModuleID, 
									const unsigned int BufAddr, 
									const unsigned int BufSize,
									unsigned int mvaRegionAddr) 
{			
    int ret;
    unsigned int pteStart, pteNr;

    M4ULOG("mva dealloc: ID=%s, VA=0x%x, size=%d, mva=0x%x\n", m4u_get_module_name(eModuleID), BufAddr, BufSize, mvaRegionAddr);

    mutex_lock(&gM4uMutex);

    MMProfileLogEx(M4U_MMP_Events[PROFILE_RELEASE_PAGES], MMProfileFlagStart, eModuleID, BufAddr);
    m4u_release_pages(eModuleID,BufAddr,BufSize,mvaRegionAddr);

    //==================================
    // fill pagetable with 0
    {
        pteStart= (unsigned int)mva_pteAddr(mvaRegionAddr); // get offset in the page table  
        pteNr = ((BufSize+(BufAddr&0xfff))/DEFAULT_PAGE_SIZE) + (((BufAddr+BufSize)&0xfff)!=0);

        if(M4U_CLNTMOD_LCDC==eModuleID)
        {
            pteStart -= 1;
            pteNr += 2;
        }
        memset((void*)pteStart, 0, pteNr<<2);
    }
    MMProfileLogEx(M4U_MMP_Events[PROFILE_RELEASE_PAGES], MMProfileFlagEnd, eModuleID, BufSize);

  
    
    mutex_unlock(&gM4uMutex);

    MMProfileLogEx(M4U_MMP_Events[PROFILE_RELEASE_MVA_REGION], MMProfileFlagStart, eModuleID, BufAddr);

    if(M4U_CLNTMOD_TVC==eModuleID)
    {    
    	ret = m4u_do_mva_free(eModuleID, BufAddr, BufSize+TVC_MVA_SAFE_MARGIN, mvaRegionAddr);
    }
    else if(M4U_CLNTMOD_LCDC==eModuleID)  //LCD_UI workaround, allocate 2 pages more at front and end
    {
    	ret = m4u_do_mva_free(eModuleID, BufAddr, BufSize+M4U_PAGE_SIZE*2, mvaRegionAddr-M4U_PAGE_SIZE);
    }
    else if(M4U_CLNTMOD_G2D==eModuleID)  //LCD_UI workaround, allocate 2 pages more at front and end
    {
    	ret = m4u_do_mva_free(eModuleID, BufAddr, BufSize+M4U_PAGE_SIZE*2, mvaRegionAddr-M4U_PAGE_SIZE);
    }
    else if(M4U_CLNTMOD_CAM==eModuleID)  //LCD_UI workaround, allocate 2 pages more at front and end
    {
    	ret = m4u_do_mva_free(eModuleID, BufAddr, BufSize+M4U_PAGE_SIZE*20, mvaRegionAddr-M4U_PAGE_SIZE*10);
    }
    else
    {
    	ret = m4u_do_mva_free(eModuleID, BufAddr, BufSize, mvaRegionAddr);
    }
    
    MMProfileLogEx(M4U_MMP_Events[PROFILE_RELEASE_MVA_REGION], MMProfileFlagEnd, mvaRegionAddr, BufSize);
    M4U_mvaGraph_dump_DBG();

    return ret;
}

int m4u_alloc_mva_tsk(M4U_MODULE_ID_ENUM eModuleID,
                                  struct task_struct *tsk,
								  const unsigned int BufAddr, 
								  const unsigned int BufSize, 
								  unsigned int *pRetMVABuf) 
{
    int ret;

    //task_lock(tsk);

    MMProfileLogEx(M4U_MMP_Events[PROFILE_ALLOC_MVA], MMProfileFlagStart, eModuleID, BufAddr);
    ret = m4u_alloc_mva_dynamic(eModuleID, tsk, BufAddr, BufSize, pRetMVABuf);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_ALLOC_MVA], MMProfileFlagEnd, eModuleID, BufSize);
    if(ret < 0)
    {
        M4UMSG(" m4u_alloc_mva failed: id=%s,addr=0x%x,size=%d\n",
            m4u_get_module_name(eModuleID), BufAddr, BufSize);
    }

    //task_unlock(tsk);

    m4u_dbg_push(M4U_EVT_ALLOC_MVA, eModuleID, BufAddr, BufSize, *pRetMVABuf, tsk->tgid);
    M4ULOG("alloc_mva_tsk: id=%s, tsk=0x%x, tsk.tgid=%d, addr=0x%08x, size=%d, mva=0x%x, mva_end=0x%x\n", 
                    m4u_get_module_name(eModuleID), tsk, tsk->tgid, BufAddr,  BufSize, *pRetMVABuf, *pRetMVABuf+BufSize-1);
    return ret;
}

int m4u_alloc_mva(M4U_MODULE_ID_ENUM eModuleID, 
								  const unsigned int BufAddr, 
								  const unsigned int BufSize, 
								  unsigned int *pRetMVABuf) 
{
    return m4u_alloc_mva_tsk(eModuleID, current, BufAddr, BufSize, pRetMVABuf);
}


#define MVA_PROTECT_BUFFER_SIZE 1024*1024
int m4u_dealloc_mva(M4U_MODULE_ID_ENUM eModuleID, 
									const unsigned int BufAddr, 
									const unsigned int BufSize, 
									const unsigned int MVA) 
{									

    int ret;
    

    MMProfileLogEx(M4U_MMP_Events[PROFILE_DEALLOC_MVA], MMProfileFlagStart, eModuleID, BufAddr);

#if 1
    if(eModuleID!=M4U_CLNTMOD_RDMA_GENERAL && eModuleID!=M4U_CLNTMOD_ROT_GENERAL)
    {
        if(m4u_invalid_tlb_range_by_mva(m4u_get_index_by_module(eModuleID), MVA, MVA+BufSize-1)==0)
        {
            M4UMSG("warning: dealloc mva without invalid tlb range!! id=%s,add=0x%x,size=0x%x,mva=0x%x\n",
                m4u_get_module_name(eModuleID), BufAddr, BufSize, MVA);
        }
    }
#endif
    
    ret = m4u_dealloc_mva_dynamic(eModuleID, BufAddr, BufSize, MVA);

    m4u_dbg_push(M4U_EVT_DEALLOC_MVA, eModuleID, BufAddr, BufSize, MVA, 0);

    M4ULOG("m4u_dealloc_mva, module=%s, addr=0x%x, size=0x%x, MVA=0x%x, mva_end=0x%x\n",
        m4u_get_module_name(eModuleID), BufAddr, BufSize, MVA, MVA+BufSize-1 );


    MMProfileLogEx(M4U_MMP_Events[PROFILE_DEALLOC_MVA], MMProfileFlagEnd, eModuleID, BufSize);

    return ret;

}


int m4u_invalid_tlb_all(M4U_MODULE_ID_ENUM eModuleID) 
{
    unsigned int i;
    unsigned int Temp;
    unsigned int m4u_base = gM4UBaseAddr[m4u_get_index_by_module(eModuleID)];
    unsigned int m4u_index = m4u_get_index_by_module(eModuleID);
    unsigned int m4u_index_offset = (RT_RANGE_NUM+SEQ_RANGE_NUM)*m4u_index;
    unsigned int isCalled=0;
    
  
    M4ULOG("m4u_invalid_tlb_all, module:%s \n", m4u_get_module_name(eModuleID)); 
    M4U_POW_ON_TRY(eModuleID, &isCalled);

    spin_lock(&gM4uLock);
    if(FreeRTRegs[m4u_index] + FreeSEQRegs[m4u_index] < TOTAL_RANGE_NUM)
    {
        if(FreeRTRegs[m4u_index] < RT_RANGE_NUM)
        {
            for(i=m4u_index_offset;i<m4u_index_offset+RT_RANGE_NUM;i++)
            {
                if(pRangeDes[i].Enabled == 1)
                {
                    pRangeDes[i].Enabled = 0;
                    Temp = ((i-m4u_index_offset)<<3) + M4UMACRO0013;
                    M4U_WriteReg32(m4u_base, Temp, 0);
                    Temp = ((i-m4u_index_offset)<<3) + M4UMACRO0015;
                    M4U_WriteReg32(m4u_base, Temp, 0);
                    FreeRTRegs[m4u_index]++;
                }
            }
        }

        if(FreeSEQRegs[m4u_index] < SEQ_RANGE_NUM)
        {
            for(i=m4u_index_offset+RT_RANGE_NUM;i<m4u_index_offset+RT_RANGE_NUM+SEQ_RANGE_NUM;i++)
            {
                if(pRangeDes[i].Enabled == 1)
                {
                     pRangeDes[i].Enabled = 0;
                     Temp = ((i-m4u_index_offset)<<3) + M4UMACRO0013;
                     M4U_WriteReg32(m4u_base, Temp, 0);
                     Temp = ((i-m4u_index_offset)<<3) + M4UMACRO0015;
                     M4U_WriteReg32(m4u_base, Temp, 0);
                     FreeSEQRegs[m4u_index]++;
                }
            }
        }
    }

    SMI_WriteReg32(M4UMACRO0197, 1<<m4u_index);
    SMI_WriteReg32(M4UMACRO0194, M4UMACRO0195);
    M4U_POW_OFF_TRY(eModuleID, isCalled);
    spin_unlock(&gM4uLock);
    
    return 0;

}

static inline int mva_owner_match(M4U_MODULE_ID_ENUM id, M4U_MODULE_ID_ENUM owner)
{
    if(owner == id)
        return 1;
    if(owner==M4U_CLNTMOD_RDMA_GENERAL &&
       (id==M4U_CLNTMOD_RDMA0||id==M4U_CLNTMOD_RDMA1) 
       )
    {
        return 1;
    }
    if(owner==M4U_CLNTMOD_ROT_GENERAL &&
        (id==M4U_CLNTMOD_VDO_ROT0||
        id==M4U_CLNTMOD_RGB_ROT0||
        id==M4U_CLNTMOD_RGB_ROT1||
        id==M4U_CLNTMOD_VDO_ROT1||
        id==M4U_CLNTMOD_RGB_ROT2)
        )
    {
        return 1;
    }

    return 0;
}



int m4u_manual_insert_entry(M4U_MODULE_ID_ENUM eModuleID,
									unsigned int EntryMVA, 
									bool Lock) 
{ 
    unsigned int i,regval;
    unsigned int *pPageAddr = NULL;
    unsigned int EntryPA;
    unsigned int m4u_base = gM4UBaseAddr[m4u_get_index_by_module(eModuleID)];
    unsigned int m4u_index = m4u_get_index_by_module(eModuleID);
    unsigned int isCalled = 0;       

    M4ULOG("m4u_manual_insert_entry, module:%s, EntryMVA:0x%x, Lock:%d \r\n", 
            m4u_get_module_name(eModuleID), EntryMVA, Lock);

    if(!mva_owner_match(eModuleID, mva2module(EntryMVA)))
    {
        M4UERR_WITH_OWNER("m4u_manual_insert_entry module=%s, EntryMVA=0x%x is %s \n", m4u_get_module_name(eModuleID), 
    	    m4u_get_module_name(eModuleID), EntryMVA, m4u_get_module_name(mva2module(EntryMVA)));
        m4u_mvaGraph_dump();
        return -1;
    }

#if 1
    if(2==m4u_index)
    {
        for(i=0;i<g4M4UTagCount[m4u_index];i++)
        {
            regval = M4U_ReadReg32(m4u_base, M4UMACRO0040+i*4);
        
            if( ((regval&M4UMACRO0011)!=0) &&
                ((regval&M4UMACRO0009) == (EntryMVA&M4UMACRO0009))
              )
              {
                  M4UMSG("error: manual insert tlb error (duplicated): id=%s, va=0x%x, tlb_tag=0x%x\n",
                        m4u_get_module_name(eModuleID), EntryMVA, regval);
              	  goto error_out;
              }
        } 
    }

#endif
                	  
	  pPageAddr = (unsigned int *)((unsigned int) pPageTableVA_nonCache + (EntryMVA/DEFAULT_PAGE_SIZE*4));
	  EntryPA = *pPageAddr;  
	  EntryPA &= 0xFFFFF000;
	
	  if(Lock)
	  {
		  EntryMVA |= M4UMACRO0010; 
	  }
	  else
	  {
		  EntryMVA &= ~M4UMACRO0010; 
	  }
	  EntryMVA &= 0xFFFFF800;
	
	  M4UDBG("m4u_manual_insert_entry, Entry_MVA=0x%x, Entry_PA=0x%x \n", 
	          EntryMVA, EntryPA);

	  spin_lock(&gM4uLock);
    M4U_POW_ON_TRY(eModuleID, &isCalled);	  
	  M4U_WriteReg32(m4u_base, M4UMACRO0008, EntryMVA);
	  M4U_WriteReg32(m4u_base, M4UMACRO0012, EntryPA);
	  M4U_WriteReg32(m4u_base, M4UMACRO0006, M4UMACRO0007);
    M4U_POW_OFF_TRY(eModuleID, isCalled);	  
	  spin_unlock(&gM4uLock);


error_out:
	  
	  return 0;
}


#define M4U_INVALID_ID 0x5555
int m4u_do_insert_tlb_range(M4U_MODULE_ID_ENUM eModuleID, 
                             unsigned int MVAStart, 
                             unsigned int MVAEnd, 
                             M4U_RANGE_PRIORITY_ENUM ePriority,
                             unsigned int entryCount)
{
    unsigned int i;
    unsigned int RangeReg_ID = M4U_INVALID_ID;
    unsigned int RegAddr;
    unsigned int m4u_base = gM4UBaseAddr[m4u_get_index_by_module(eModuleID)];
    unsigned int m4u_index = m4u_get_index_by_module(eModuleID);
    unsigned int m4u_index_offset = (RT_RANGE_NUM+SEQ_RANGE_NUM)*m4u_index;

    if(entryCount!=1 && entryCount!=2 && entryCount!=4 && entryCount!=8 && entryCount!=16)
        entryCount = 1;

    if(entryCount!=1 &&
    	 eModuleID!=M4U_CLNTMOD_VDO_ROT0 &&
    	 eModuleID!=M4U_CLNTMOD_RGB_ROT0 && 
    	 eModuleID!=M4U_CLNTMOD_RGB_ROT1 && 
    	 eModuleID!=M4U_CLNTMOD_VDO_ROT1 && 
    	 eModuleID!=M4U_CLNTMOD_RGB_ROT2 && 
    	 eModuleID!=M4U_CLNTMOD_TVROT && 
    	 eModuleID!=M4U_CLNTMOD_G2D )
    {
    	 M4UDBG("warning: m4u_insert_tlb_range() entryCount=%d, but module=%s should set entryCount to 0 \n", 
    	         entryCount, m4u_get_module_name(eModuleID));
    	 entryCount = 1;
    }

    if(!mva_owner_match(eModuleID, mva2module(MVAStart)))
    {
        
        M4UERR_WITH_OWNER("m4u_insert_tlb_range module=%s, MVAStart=0x%x is %s, MVAEnd=0x%x is %s\n", m4u_get_module_name(eModuleID), 
    	    m4u_get_module_name(eModuleID), MVAStart, m4u_get_module_name(mva2module(MVAStart)),
    	    MVAEnd, m4u_get_module_name(mva2module(MVAEnd)));
        m4u_mvaGraph_dump();
    }


    
    
    M4ULOG("m4u_insert_tlb_range, module:%s, MVAStart:0x%x, MVAEnd:0x%x, ePriority=%d, entryCount=%d \r\n", 
            m4u_get_module_name(eModuleID), MVAStart, MVAEnd, ePriority, entryCount);
            
    if(FreeRTRegs[m4u_index] + FreeSEQRegs[m4u_index] == 0)
    {
        M4ULOG("No seq range found. module=%s \n", m4u_get_module_name(eModuleID));
#ifdef M4U_PRINT_RANGE_DETAIL
        M4UMSG("m4u_insert_tlb_range, module:%s, MVAStart:0x%x, MVAEnd:0x%x, ePriority=%d, entryCount=%d \r\n", 
                m4u_get_module_name(eModuleID), MVAStart, MVAEnd, ePriority, entryCount);
        M4UMSG(" Curent Range Info: \n");
        for(i=0;i<TOTAL_RANGE_NUM;i++)
        {
            if(1==pRangeDes[i].Enabled)
            {
                M4UMSG("pRangeDes[%d]: Enabled=%d, module=%s, MVAStart=0x%x, MVAEnd=0x%x \n", 
                    i, pRangeDes[i].Enabled, m4u_get_module_name(pRangeDes[i].eModuleID), 
                    pRangeDes[i].MVAStart, pRangeDes[i].MVAEnd);
            }
        } 
#endif        
        return 0;
    }


    
    for(i=m4u_index_offset;i<m4u_index_offset+RT_RANGE_NUM+SEQ_RANGE_NUM;i++)
    {
        if(1==pRangeDes[i].Enabled)
        {
            if(MVAEnd<pRangeDes[i].MVAStart || MVAStart>pRangeDes[i].MVAEnd) //no overlap
            {
            	  continue;
            }
            else
            {
                M4UERR_WITH_OWNER("error: insert tlb range is overlapped with previous ranges, current process=%s,!\n", m4u_get_module_name(eModuleID),  current->comm);	
                M4UMSG("module=%s, mva_start=0x%x, mva_end=0x%x \n", m4u_get_module_name(eModuleID), MVAStart, MVAEnd);
                M4UMSG("overlapped range id=%d, module=%s, mva_start=0x%x, mva_end=0x%x \n", 
                        i, m4u_get_module_name(pRangeDes[i].eModuleID), pRangeDes[i].MVAStart, pRangeDes[i].MVAEnd);
                return -1;
            }
        }
    }

    for(i=m4u_index_offset;i<m4u_index_offset+RT_RANGE_NUM+SEQ_RANGE_NUM;i++)
    {
        if(1==pRangeDes[i].Enabled)
        {

            if(pRangeDes[i].MVAStart - MVAEnd >0 &&
               pRangeDes[i].MVAStart - MVAEnd < 2*DEFAULT_PAGE_SIZE &&
               (MVAEnd & 0x1000)==0 )
            {
                if(MVAEnd - DEFAULT_PAGE_SIZE <= MVAStart)
                {
                    M4UMSG("warning: skip un-needed tlb insert! \n"); 
                    M4UMSG("module=%s, mva_start=0x%x, mva_end=0x%x \n", m4u_get_module_name(eModuleID), MVAStart, MVAEnd);
                    M4UMSG("overlapped range id=%d, mva_start=0x%x, mva_end=0x%x \n",i, pRangeDes[i].MVAStart, pRangeDes[i].MVAEnd);
                    return 0;
                }
                M4UDBG("warning: shrink range by MVAEnd -= DEFAULT_PAGE_SIZE \n"); 
                MVAEnd -= DEFAULT_PAGE_SIZE;
            }
            

            if(MVAStart - pRangeDes[i].MVAEnd > 0 &&
               MVAStart - pRangeDes[i].MVAEnd < 2*DEFAULT_PAGE_SIZE &&
               (pRangeDes[i].MVAEnd & 0x1000)==0 )
            {
                if(MVAStart + DEFAULT_PAGE_SIZE >= MVAEnd)
                {
                    M4UMSG("warning: skip un-needed tlb insert! \n");  
                    M4UMSG("module=%s, mva_start=0x%x, mva_end=0x%x \n", m4u_get_module_name(eModuleID), MVAStart, MVAEnd);
                    M4UMSG("overlapped range id=%d, mva_start=0x%x, mva_end=0x%x \n",i, pRangeDes[i].MVAStart, pRangeDes[i].MVAEnd);
                    return 0;
                }
                M4UDBG("warning: shrink range by  MVAStart += DEFAULT_PAGE_SIZE \n"); 
                MVAStart += DEFAULT_PAGE_SIZE;                
            }             

        }
    }
        
    spin_lock(&gM4uLock);
    if(ePriority == RT_RANGE_HIGH_PRIORITY)
    {
        if(FreeRTRegs[m4u_index]>0)
        {
            for(i=m4u_index_offset;i<m4u_index_offset+RT_RANGE_NUM;i++)
            {
                if(pRangeDes[i].Enabled == 0)
                {
                    M4UDBG(", Real-Time range found.\n");
                    RangeReg_ID = i;
                    FreeRTRegs[m4u_index]--;
                    break;
                }
            }
        }
        else if(FreeSEQRegs[m4u_index]>0)
        {
            for(i=m4u_index_offset+RT_RANGE_NUM;i<m4u_index_offset+RT_RANGE_NUM+SEQ_RANGE_NUM;i++)
            {
                if(pRangeDes[i].Enabled == 0)
                {
                    M4UDBG(", Real-Time range request is downgraded to Sequential range.\n");
                    RangeReg_ID = i;
                    FreeSEQRegs[m4u_index]--;
                    break;
                }
            }
        }
    }
    else 
    {
        if(FreeSEQRegs[m4u_index]>0)
        {
            for(i=m4u_index_offset+RT_RANGE_NUM;i<m4u_index_offset+RT_RANGE_NUM+SEQ_RANGE_NUM;i++)
            {
                if(pRangeDes[i].Enabled == 0)
                {
                    M4UDBG(", Sequential range found.\n");
                    RangeReg_ID = i;
                    FreeSEQRegs[m4u_index]--;
                    break;
                }
            }
        }
        else if(FreeRTRegs[m4u_index]>0)
        {
            for(i=m4u_index_offset;i<m4u_index_offset+RT_RANGE_NUM;i++)
            {
                if(pRangeDes[i].Enabled == 0)
                {
                    M4UDBG(", Sequential range request is downgraded to Real-Time range.\n");
                    RangeReg_ID = i;
                    FreeRTRegs[m4u_index]--;
                    break;
                }
            }
        }
    }


    if(RangeReg_ID == M4U_INVALID_ID)
    {
        M4ULOG("error: can not find available range \n");
        spin_unlock(&gM4uLock);
        return 0; 
    }
    
    pRangeDes[RangeReg_ID].Enabled = 1;
    pRangeDes[RangeReg_ID].eModuleID = eModuleID;
    pRangeDes[RangeReg_ID].MVAStart = MVAStart;
    pRangeDes[RangeReg_ID].MVAEnd = MVAEnd;
    pRangeDes[RangeReg_ID].entryCount = entryCount;
    RegAddr = ((RangeReg_ID-m4u_index_offset)<<3) + M4UMACRO0013;
    
    MVAStart &= ~(0x0f<<7);
    MVAStart |= ((entryCount-1)<<7);
    MVAStart |= M4UMACRO0014;
    
    {
        unsigned int isCalled=0;
        M4U_POW_ON_TRY(eModuleID, &isCalled);
        M4U_WriteReg32(m4u_base, RegAddr, MVAStart);
        RegAddr += 4;
        M4U_WriteReg32(m4u_base, RegAddr, MVAEnd);
        M4U_POW_OFF_TRY(eModuleID, isCalled);
    }
    
    spin_unlock(&gM4uLock);

    
    return 0;
} 


int m4u_insert_tlb_range(M4U_MODULE_ID_ENUM eModuleID, 
                             unsigned int MVAStart, 
                             unsigned int MVAEnd, 
                             M4U_RANGE_PRIORITY_ENUM ePriority,
                             unsigned int entryCount) //0:disable multi-entry, 1,2,4,8,16: enable multi-entry
{

    int ret;
    MMProfileLogEx(M4U_MMP_Events[PROFILE_INSERT_TLB], MMProfileFlagStart, eModuleID, MVAStart);
    ret = m4u_do_insert_tlb_range(eModuleID, MVAStart, MVAEnd, ePriority,entryCount);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_INSERT_TLB], MMProfileFlagEnd, eModuleID, MVAEnd-MVAStart+1);
    return ret;
    
}

int m4u_invalid_tlb_range_by_mva(int m4u_index, unsigned int MVAStart, unsigned int MVAEnd)
{
    unsigned int i;
    unsigned int Temp;
    unsigned int m4u_base = gM4UBaseAddr[m4u_index];
    unsigned int m4u_index_offset = (RT_RANGE_NUM+SEQ_RANGE_NUM)*m4u_index;
	unsigned int isCalled = 0;
    int ret=-1;

    M4UDBG("m4u_invalid_tlb_range_by_mva,  MVAStart:0x%x, MVAEnd:0x%x \r\n", MVAStart, MVAEnd);
	      
    spin_lock(&gM4uLock); 
    M4U_POW_ON_TRY(m4u_get_module_by_index(m4u_index), &isCalled);
    
    if(FreeRTRegs[m4u_index]+FreeSEQRegs[m4u_index] < RT_RANGE_NUM+SEQ_RANGE_NUM)
    {
        if(FreeRTRegs[m4u_index] < RT_RANGE_NUM)
        {
            for(i=m4u_index_offset;i<m4u_index_offset+RT_RANGE_NUM;i++)
            {
                if(pRangeDes[i].Enabled == 1 &&
                    pRangeDes[i].MVAStart>=MVAStart && 
                    pRangeDes[i].MVAEnd<=MVAEnd)
                {
                    pRangeDes[i].Enabled = 0;
                    Temp = ((i-m4u_index_offset)<<3) + M4UMACRO0013;
                    M4U_WriteReg32(m4u_base, Temp, 0);
                    Temp = ((i-m4u_index_offset)<<3) + M4UMACRO0015;
                    M4U_WriteReg32(m4u_base, Temp, 0);
                    FreeRTRegs[m4u_index]++;
                    ret = 0;
                }
            }
        }

        if(ret!=0 && FreeSEQRegs[m4u_index]<SEQ_RANGE_NUM)
        {
            for(i=m4u_index_offset+RT_RANGE_NUM;i<m4u_index_offset+RT_RANGE_NUM+SEQ_RANGE_NUM;i++)
            {
                if(pRangeDes[i].Enabled == 1 &&
                    pRangeDes[i].MVAStart>=MVAStart && 
                    pRangeDes[i].MVAEnd<=MVAEnd)
                {
                     pRangeDes[i].Enabled = 0;
                     Temp = ((i-m4u_index_offset)<<3) + M4UMACRO0013;
                     M4U_WriteReg32(m4u_base, Temp, 0);
                     Temp = ((i-m4u_index_offset)<<3) + M4UMACRO0015;
                     M4U_WriteReg32(m4u_base, Temp, 0);
                     FreeSEQRegs[m4u_index]++;
                     ret = 0;
                }
            }
        }
    }
    
    MVAStart &= 0xfffff000;
    MVAStart |= (1<< m4u_index);
    SMI_WriteReg32(M4UMACRO0197, MVAStart);
    SMI_WriteReg32(M4UMACRO0198, MVAEnd);
    SMI_WriteReg32(M4UMACRO0194, M4UMACRO0196);    
    
    M4U_POW_OFF_TRY(m4u_get_module_by_index(m4u_index), isCalled);                  
    spin_unlock(&gM4uLock);

    return ret;

}



int m4u_invalid_tlb_range(M4U_MODULE_ID_ENUM eModuleID, unsigned int MVAStart, unsigned int MVAEnd)
{
	  
    M4ULOG("m4u_invalid_tlb_range, module:%s, MVAStart:0x%x, MVAEnd:0x%x \r\n", m4u_get_module_name(eModuleID), MVAStart, MVAEnd);
	      
    if(!mva_owner_match(eModuleID, mva2module(MVAStart)))
    {
        M4UERR_WITH_OWNER("m4u_invalid_tlb_range module=%s, MVAStart=0x%x is %s, MVAEnd=0x%x is %s\n", m4u_get_module_name(eModuleID), 
    	    m4u_get_module_name(eModuleID), MVAStart, m4u_get_module_name(mva2module(MVAStart)),
    	    MVAEnd, m4u_get_module_name(mva2module(MVAEnd)));
        m4u_mvaGraph_dump();
    }
    
    m4u_invalid_tlb_range_by_mva(m4u_get_index_by_module(eModuleID), MVAStart, MVAEnd);

    return 0;

}



int m4u_insert_wrapped_range(M4U_MODULE_ID_ENUM eModuleID, 
                             M4U_PORT_ID_ENUM portID, 
                             unsigned int MVAStart, 
                             unsigned int MVAEnd)
{
    unsigned int i;
    unsigned int WrapRangeID = M4U_INVALID_ID;
    unsigned int RegAddr, RegVal;
    unsigned int m4u_base = gM4UBaseAddr[m4u_get_index_by_port(portID)];
    unsigned int m4u_index = m4u_get_index_by_port(portID);
    unsigned int m4u_index_offset = g4M4UWrapOffset[m4u_index];
    unsigned int isCalled=0;
    
    M4ULOG("m4u_insert_wrapped_range, module:%s, port:%s, MVAStart:0x%x, MVAEnd:0x%x \r\n", 
            m4u_get_module_name(eModuleID), m4u_get_port_name(portID), MVAStart, MVAEnd);
	  
            
    if(FreeWrapRegs[m4u_index] == 0)
    {
        M4UMSG("warning: m4u_insert_wrapped_range, no available wrap range found.\n");
        return -1;
    }    	      
    

    if(mva2module(MVAStart)!=eModuleID || mva2module(MVAEnd)!=eModuleID)
    {
        M4UERR_WITH_OWNER("m4u_insert_wrapped_range module=%s, MVAStart=0x%x is %s, MVAEnd=0x%x is %s\n", m4u_get_module_name(eModuleID), 
    	    m4u_get_module_name(eModuleID), MVAStart, m4u_get_module_name(mva2module(MVAStart)),
    	    MVAEnd, m4u_get_module_name(mva2module(MVAEnd)));
        m4u_mvaGraph_dump();
        return -1;
    }
    
        
    for(i=m4u_index_offset;i<m4u_index_offset+g4M4UWrapCount[m4u_index];i++)
    {
        if(1==pWrapDes[i].Enabled)
        {
            if(MVAEnd<pWrapDes[i].MVAStart || MVAStart>pWrapDes[i].MVAEnd) //no overlap
            {
            	  continue;
            }
            else
            {
                M4UMSG("error: insert tlb range is overlapped with previous ranges, current!\n");	
                M4UMSG("module=%s, mva_start=0x%x, mva_end=0x%x \n", m4u_get_module_name(eModuleID), MVAStart, MVAEnd);
                M4UMSG("overlapped range id=%d, module=%s, mva_start=0x%x, mva_end=0x%x \n", 
                        i, m4u_get_module_name(pWrapDes[i].eModuleID), pWrapDes[i].MVAStart, pWrapDes[i].MVAEnd);
                return -1;        
            }
        }
    }

    for(i=m4u_index_offset;i<m4u_index_offset+g4M4UWrapCount[m4u_index];i++)
    {
        if(1==pWrapDes[i].Enabled)
        {

            if(pWrapDes[i].MVAStart - MVAEnd >0 &&
               pWrapDes[i].MVAStart - MVAEnd < 2*DEFAULT_PAGE_SIZE &&
               (MVAEnd & 0x1000)==0 )
            {
                if(MVAEnd - DEFAULT_PAGE_SIZE <= MVAStart)
                {
                    M4UMSG("warning: skip un-needed tlb insert! \n"); 
                    M4UMSG("module=%s, mva_start=0x%x, mva_end=0x%x \n", m4u_get_module_name(eModuleID), MVAStart, MVAEnd);
                    M4UMSG("overlapped range id=%d, mva_start=0x%x, mva_end=0x%x \n",i, pWrapDes[i].MVAStart, pWrapDes[i].MVAEnd);
                    return 0;
                }
                M4UMSG("warning: shrink range by MVAEnd -= DEFAULT_PAGE_SIZE \n"); 
                MVAEnd -= DEFAULT_PAGE_SIZE;
            }
            

            if(MVAStart - pWrapDes[i].MVAEnd > 0 &&
               MVAStart - pWrapDes[i].MVAEnd < 2*DEFAULT_PAGE_SIZE &&
               (pWrapDes[i].MVAEnd & 0x1000)==0 )
            {
                if(MVAStart + DEFAULT_PAGE_SIZE >= MVAEnd)
                {
                    M4UMSG("warning: skip un-needed tlb insert! \n");  
                    M4UMSG("module=%s, mva_start=0x%x, mva_end=0x%x \n", m4u_get_module_name(eModuleID), MVAStart, MVAEnd);
                    M4UMSG("overlapped range id=%d, mva_start=0x%x, mva_end=0x%x \n",i, pWrapDes[i].MVAStart, pWrapDes[i].MVAEnd);
                    return 0;
                }
                M4UMSG("warning: shrink range by  MVAStart += DEFAULT_PAGE_SIZE \n"); 
                MVAStart += DEFAULT_PAGE_SIZE;                
            }             

        }
    }

    spin_lock(&gM4uLock);        
    for(i=m4u_index_offset;i<m4u_index_offset+g4M4UWrapCount[m4u_index];i++)
    {
        if(pWrapDes[i].Enabled == 0)
        {
            M4UDBG("wrap range found. rangeID=%d \n", i);
            WrapRangeID = i;
            FreeWrapRegs[m4u_index]--;
            break;
        }
    }
            
    if(WrapRangeID == M4U_INVALID_ID)
    {
        M4U_ASSERT(0);
        M4UDBG("can not find available wrap range \n");
        spin_unlock(&gM4uLock);
        return -1;
    }
    
    pWrapDes[WrapRangeID].Enabled = 1;
    pWrapDes[WrapRangeID].eModuleID = eModuleID;
    pWrapDes[WrapRangeID].ePortID = portID;
    pWrapDes[WrapRangeID].MVAStart = MVAStart;
    pWrapDes[WrapRangeID].MVAEnd = MVAEnd;

    M4U_POW_ON_TRY(eModuleID, &isCalled);
    M4U_WriteReg32(m4u_base, M4UMACRO0125 + (WrapRangeID-m4u_index_offset)*8, MVAStart&(~0xfff));
    M4U_WriteReg32(m4u_base, M4UMACRO0126 + (WrapRangeID-m4u_index_offset)*8, MVAEnd&(~0xfff));

    RegAddr = M4UMACRO0133 + (portID%SEGMENT_SIZE)/8*4;
    RegVal = M4U_ReadReg32(m4u_base, RegAddr);
    RegVal &= ~(0x0f <<((portID%SEGMENT_SIZE%8)*4));
    RegVal |= ((WrapRangeID-m4u_index_offset+1) <<((portID%SEGMENT_SIZE%8)*4));
    M4U_WriteReg32(m4u_base, RegAddr, RegVal);

    M4U_POW_OFF_TRY(eModuleID, isCalled);
  
    spin_unlock(&gM4uLock);
    
    return 0;
}

int m4u_invalid_wrapped_range(M4U_MODULE_ID_ENUM eModuleID, 
                              M4U_PORT_ID_ENUM portID,
                              unsigned int MVAStart, 
                              unsigned int MVAEnd)
{
    unsigned int i;
    unsigned int WrapRangeID = M4U_INVALID_ID;
    unsigned int RegAddr, RegVal;
    unsigned int m4u_base = gM4UBaseAddr[m4u_get_index_by_module(eModuleID)];
    unsigned int m4u_index = m4u_get_index_by_module(eModuleID);
    unsigned int m4u_index_offset = g4M4UWrapOffset[m4u_index];
	  unsigned int isCalled=0;
	  
    M4ULOG("m4u_invalid_wrapped_range, module:%s, MVAStart:0x%x, MVAEnd:0x%x \r\n", m4u_get_module_name(eModuleID), MVAStart, MVAEnd);
	      

    if(!mva_owner_match(eModuleID, mva2module(MVAStart)))
    {
        M4UERR_WITH_OWNER("m4u_invalid_wrapped_range module=%s, MVAStart=0x%x is %s, MVAEnd=0x%x is %s\n", m4u_get_module_name(eModuleID), 
    	    m4u_get_module_name(eModuleID), MVAStart, m4u_get_module_name(mva2module(MVAStart)),
    	    MVAEnd, m4u_get_module_name(mva2module(MVAEnd)));
        m4u_mvaGraph_dump();
        return -1;
    }
        
    spin_lock(&gM4uLock);
    
    if(FreeWrapRegs[m4u_index] < g4M4UWrapCount[m4u_index])
    {
        for(i=m4u_index_offset;i<m4u_index_offset+g4M4UWrapCount[m4u_index];i++)
        {
            if(pWrapDes[i].Enabled == 1 &&
                pWrapDes[i].MVAStart>=MVAStart && 
                pWrapDes[i].MVAEnd<=MVAEnd)
            {
                pWrapDes[i].Enabled = 0;
                WrapRangeID = i;
                FreeWrapRegs[m4u_index]++;
            }
        }
    }

    if(WrapRangeID == M4U_INVALID_ID)
    {
        M4UDBG("warning: m4u_invalid_wrapped_range(), does not find wrap range \n");
        spin_unlock(&gM4uLock);
        return 0;
    }
    
    pWrapDes[WrapRangeID].Enabled = 0;
    pWrapDes[WrapRangeID].eModuleID = 0;
    pWrapDes[WrapRangeID].ePortID = 0;
    pWrapDes[WrapRangeID].MVAStart = 0;
    pWrapDes[WrapRangeID].MVAEnd = 0;

    M4U_POW_ON_TRY(eModuleID, &isCalled);
    M4U_WriteReg32(m4u_base, M4UMACRO0125 + (WrapRangeID-m4u_index_offset)*8, 0);
    M4U_WriteReg32(m4u_base, M4UMACRO0126 + (WrapRangeID-m4u_index_offset)*8, 0);
    RegAddr = M4UMACRO0133 + (portID%SEGMENT_SIZE)/8*4;
    RegVal = M4U_ReadReg32(m4u_base, RegAddr);
    RegVal &= ~(0x0f <<((portID%SEGMENT_SIZE%8)*4));
    M4U_WriteReg32(m4u_base, RegAddr, RegVal);

    M4U_POW_OFF_TRY(eModuleID, isCalled);
	  
	  spin_unlock(&gM4uLock);
	  
    return 0;

}



int m4u_config_port(M4U_PORT_STRUCT* pM4uPort)
{
    unsigned int TempPort = 0;
    unsigned int TempRegPtr = 0;
    unsigned int m4u_base = gM4UBaseAddr[m4u_get_index_by_port(pM4uPort->ePortID)];
    M4U_MODULE_ID_ENUM eModuleID = m4u_get_module_by_port(pM4uPort->ePortID);
    unsigned char PortID = (pM4uPort->ePortID)%SEGMENT_SIZE;
    unsigned int Addr,BitOffset,Mask;
    unsigned int TempDistance;
    unsigned int isCalled=0;
    
    M4ULOG("m4u_config_port(), port=%s, Virtuality=%d, Security=%d, Distance=%d, Direction=%d \n", 
        m4u_get_port_name(pM4uPort->ePortID), pM4uPort->Virtuality, pM4uPort->Security, pM4uPort->Distance, pM4uPort->Direction);


    MMProfileLogEx(M4U_MMP_Events[PROFILE_CONFIG_PORT], MMProfileFlagStart, eModuleID, pM4uPort->ePortID);

    M4U_POW_ON_TRY(eModuleID, &isCalled);
    spin_lock(&gM4uLock);
        
    if(PortID <= g4M4UPortCount[m4u_get_index_by_port(pM4uPort->ePortID)])
    {
        TempRegPtr = M4UMACRO0159;
        TempPort = 1 << PortID;
    }
    else
    {
        M4UERR("portID is bigger than expected! PortID=%d\n", PortID);
    }

    if(pM4uPort->Virtuality)
    {       
        M4U_WriteReg32(m4u_base, TempRegPtr, (M4U_ReadReg32(m4u_base, TempRegPtr)) |TempPort);
    }
    else
    {
        M4U_WriteReg32(m4u_base, TempRegPtr, (M4U_ReadReg32(m4u_base, TempRegPtr)) & (~TempPort));
        goto config_port_out;
    }
    M4UDBG("after config port: EN0=0x%x, EN1=0x%x \n", 
        M4U_ReadReg32(m4u_base, M4UMACRO0159), 
        M4U_ReadReg32(m4u_base, M4UMACRO0160));

    if(pM4uPort->Direction)
    {
        TempPort = M4U_ReadReg32(m4u_base, M4UMACRO0038) | TempPort;
        M4U_WriteReg32(m4u_base, M4UMACRO0038, TempPort);
    }
    else
    {
        TempPort = M4U_ReadReg32(m4u_base, M4UMACRO0038) & (~TempPort);
        M4U_WriteReg32(m4u_base, M4UMACRO0038, TempPort);
    }

    if(M4U_PORT_G2D_W == pM4uPort->ePortID)
    {
        Addr = M4UMACRO0036;
        TempDistance = (M4U_ReadReg32(m4u_base, Addr)&0x0000ffff) |(pM4uPort->Distance<<16) ;
        M4U_WriteReg32(m4u_base, Addr, TempDistance);
    }
    else if(M4U_PORT_G2D_R == pM4uPort->ePortID)
    {
        Addr = M4UMACRO0036;
        TempDistance = (M4U_ReadReg32(m4u_base, Addr)&0xffff0000) |(pM4uPort->Distance) ;
        M4U_WriteReg32(m4u_base, Addr, TempDistance);
    }
    else
    {
        unsigned char ClippingPortID = PortID > MODULE_WITH_INDEPENDENT_PORT_ID ? MODULE_WITH_INDEPENDENT_PORT_ID : PortID;
        M4U_ASSERT(pM4uPort->Distance>=0 &&  pM4uPort->Distance<16);
        Addr = M4UMACRO0030 + (ClippingPortID>>3)*4;
        BitOffset = (ClippingPortID&0x07)*4;
        TempDistance = M4U_ReadReg32(m4u_base, Addr);
        Mask = ~(0x0F<<BitOffset);
        TempDistance = (TempDistance & Mask) | (pM4uPort->Distance<<BitOffset);
        M4U_WriteReg32(m4u_base, Addr, TempDistance);
    }

config_port_out:

    spin_unlock(&gM4uLock);
    M4U_POW_OFF_TRY(eModuleID, isCalled); 

    MMProfileLogEx(M4U_MMP_Events[PROFILE_CONFIG_PORT], MMProfileFlagEnd, pM4uPort->Virtuality, pM4uPort->ePortID);

    return 0;

}

#define MULTI_ENTRY_VALUE 4
int m4u_config_port_rotator(M4U_PORT_STRUCT_ROTATOR *pM4uPort)
{    
    M4U_PORT_STRUCT portStruct;
    unsigned int direction = 0;
    unsigned int distance = 0;
    unsigned int page_num;
    M4U_MODULE_ID_ENUM eModuleID = m4u_get_module_by_port(pM4uPort->ePortID);
                
    M4ULOG("m4u_config_port_rotator(), module=%s, port=%s, Virtuality=%d, Security=%d, MVAStart=0x%x, BufAddr=0x%x, BufSize=0x%x, angle=%d \n", 
            m4u_get_module_name(eModuleID),
            m4u_get_port_name(pM4uPort->ePortID), 
            pM4uPort->Virtuality, 
            pM4uPort->Security, 
            pM4uPort->MVAStart, 
            pM4uPort->BufAddr,
            pM4uPort->BufSize,
            pM4uPort->angle);    

    if(pM4uPort->ePortID!=M4U_PORT_VDO_ROT0_OUT0 &&
    	 pM4uPort->ePortID!=M4U_PORT_RGB_ROT0_OUT0 &&
    	 pM4uPort->ePortID!=M4U_PORT_RGB_ROT1_OUT0 &&
    	 pM4uPort->ePortID!=M4U_PORT_VDO_ROT1_OUT0 &&
    	 pM4uPort->ePortID!=M4U_PORT_RGB_ROT2_OUT0 &&
    	 pM4uPort->ePortID!=M4U_PORT_TV_ROT_OUT0 &&
    	 pM4uPort->ePortID!=M4U_PORT_G2D_W)
    {
    	  M4UERR("error, only rotator port should call m4u_config_port_rotator(), port=%s \n", m4u_get_port_name(pM4uPort->ePortID));
    	  return -1;
    }
    
    page_num = (pM4uPort->BufSize + (pM4uPort->BufAddr&0xfff))/DEFAULT_PAGE_SIZE;
    if((pM4uPort->BufAddr+pM4uPort->BufSize)&0xfff)
    {
        page_num++;
    } 
        
    if(M4U_PORT_G2D_W==pM4uPort->ePortID)
    {
        if(pM4uPort->angle==ROTATE_0 || 
           pM4uPort->angle==ROTATE_90 ||
           pM4uPort->angle==ROTATE_HFLIP_90||
           pM4uPort->angle==ROTATE_HFLIP_0)
        {
            direction = 0;
        }
        else if(pM4uPort->angle==ROTATE_HFLIP_270 || 
                pM4uPort->angle==ROTATE_270||
                pM4uPort->angle==ROTATE_HFLIP_180||
                pM4uPort->angle==ROTATE_180)
        {
            direction = 1;        
        }
        else
        {
            M4UERR("G2D rotate angel is invalid:%d \n", pM4uPort->angle);
            return -1;
        }
    }
    else
    {
        if(pM4uPort->angle==ROTATE_0 || pM4uPort->angle==ROTATE_90)
        {
            direction = 0;
        }
        else if(pM4uPort->angle==ROTATE_180 || pM4uPort->angle==ROTATE_270)
        {
            direction = 1;
        }
        else
        {
            M4UERR("rotate angel is invalid:%d \n", pM4uPort->angle);
            return -1;
        }
    }
    
    if(pM4uPort->angle==ROTATE_90 || pM4uPort->angle==ROTATE_270)
    {
        distance = MULTI_ENTRY_VALUE;
    }
    else
    {
        distance = 1;
    }
    
    portStruct.ePortID    =pM4uPort->ePortID;
    portStruct.Virtuality =pM4uPort->Virtuality;	
    portStruct.Security   =pM4uPort->Security;   
    portStruct.Distance   =distance;   
    portStruct.Direction  =direction;  
    m4u_config_port(&portStruct);    

#ifdef M4U_ENABLE_AUTO_SET_WRAPPED_RANGE_MULTI_ENTRY
    if(1==portStruct.Virtuality)    
    {
        if(page_num%16==0 || page_num%16>8)
        {
            m4u_insert_wrapped_range(eModuleID, pM4uPort->ePortID, pM4uPort->MVAStart, pM4uPort->MVAStart+pM4uPort->BufSize-1);
        }
        else
        {
            for(i=0;i<page_num%16;i++)
            {
                m4u_manual_insert_entry(eModuleID, pM4uPort->MVAStart+DEFAULT_PAGE_SIZE*i, true);            	
            }
            m4u_insert_wrapped_range(eModuleID, pM4uPort->ePortID, pM4uPort->MVAStart+(page_num%16)*DEFAULT_PAGE_SIZE, pM4uPort->MVAStart+pM4uPort->BufSize-1);
        }
        
        if(pM4uPort->angle==ROTATE_90 || pM4uPort->angle==ROTATE_270)
        {
            entryCount = MULTI_ENTRY_VALUE;
        }
        else
        {
            entryCount = 1;
        }
        m4u_insert_tlb_range(eModuleID, pM4uPort->MVAStart, pM4uPort->MVAStart+pM4uPort->BufSize-1, RT_RANGE_HIGH_PRIORITY, entryCount);    
    }
    else
    {
        if(page_num%16==0 || page_num%16>8)
        {
            m4u_invalid_wrapped_range(eModuleID, pM4uPort->ePortID, pM4uPort->MVAStart, pM4uPort->MVAStart+pM4uPort->BufSize-1);
        }
        else
        {
            m4u_invalid_wrapped_range(eModuleID, pM4uPort->ePortID, pM4uPort->MVAStart+(page_num%16)*DEFAULT_PAGE_SIZE, pM4uPort->MVAStart+pM4uPort->BufSize-1);
        }
        m4u_invalid_tlb_range(eModuleID, pM4uPort->MVAStart, pM4uPort->MVAStart+pM4uPort->BufSize-1);       	
    }
#endif
    
    return 0;
}

int m4u_monitor_start(M4U_PORT_ID_ENUM PortID)
{
    int Temp = 0;
    unsigned int m4u_base = gM4UBaseAddr[m4u_get_index_by_port(PortID)];
    unsigned int isCalled=0;
    
    M4ULOG("start monitor, port=%s \n", m4u_get_port_name(PortID));
    
    M4U_POW_ON_TRY(m4u_get_module_by_port(PortID), &isCalled);
    //clear GMC performance counter
    Temp = M4U_ReadReg32(m4u_base, M4UMACRO0106) | 0x8;
    M4U_WriteReg32(m4u_base, M4UMACRO0106, Temp);
    // Sleep(1);
    Temp = M4U_ReadReg32(m4u_base, M4UMACRO0106) & (~0x8);
    M4U_WriteReg32(m4u_base, M4UMACRO0106, Temp);
    //enable GMC performance monitor
    Temp = M4U_ReadReg32(m4u_base, M4UMACRO0106) | 0x4;
    M4U_WriteReg32(m4u_base, M4UMACRO0106, Temp);
    M4U_POW_OFF_TRY(m4u_get_module_by_port(PortID), isCalled);


    return 0;
}

/**
 * @brief ,             
 * @param 
 * @return 
 */
int m4u_monitor_stop(M4U_PORT_ID_ENUM PortID)
{
    int Temp = 0;
    int m4u_index = m4u_get_index_by_port(PortID);
    unsigned int m4u_base = gM4UBaseAddr[m4u_index];
    unsigned int isCalled=0;
    unsigned int main_total;
    unsigned int main_miss;
    unsigned int pref_total;
    unsigned int pref_miss;
    
    M4ULOG("stop monitor, port=%s \n", m4u_get_port_name(PortID));

    M4U_POW_ON_TRY(m4u_get_module_by_port(PortID), &isCalled);
    //disable GMC performance monitor
    Temp = M4U_ReadReg32(m4u_base, M4UMACRO0106) & (~0x4);
    M4U_WriteReg32(m4u_base, M4UMACRO0106, Temp);
    main_total = M4U_ReadReg32(m4u_base, M4UMACRO0121);
    main_miss = M4U_ReadReg32(m4u_base, M4UMACRO0122);
    pref_total = M4U_ReadReg32(m4u_base, M4UMACRO0124);
    pref_miss = M4U_ReadReg32(m4u_base, M4UMACRO0123);

    MMProfileLogEx(M4U_MMP_Events[PROFILE_MAIN_TLB_MON], MMProfileFlagStart, (unsigned int) m4u_index, main_total);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_PREF_TLB_MON], MMProfileFlagStart, (unsigned int) m4u_index, pref_total);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_MAIN_TLB_MON], MMProfileFlagEnd, (unsigned int) m4u_index, main_miss);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_PREF_TLB_MON], MMProfileFlagEnd, (unsigned int) m4u_index, pref_miss);

    //read register get the count
    if(0!=main_total)
    {
        M4UMSG("[M4U] index=%d, port=%s, main tlb miss rate: %d%%, total:%d, miss:%d\r\n", 
          m4u_get_index_by_port(PortID),
          m4u_get_port_name(PortID),
          100*main_miss/main_total, 
          main_total, 
          main_miss);
    }
    else
    {
        M4UMSG("[M4U] no main tlb translate! \r\n");
    }
	
    if(0!=pref_total)
    {
        M4UMSG("[M4U] index=%d, prefetch tlb miss rate: %d%%, total:%d, miss:%d\r\n", 
          m4u_get_index_by_port(PortID),
          100*pref_miss/pref_total, 
          pref_total, 
          pref_miss);
    }
	else
    {
        M4UDBG("[M4U] index=%d, no prefetch tlb translate! \r\n", m4u_get_index_by_port(PortID));
	}
    M4U_POW_OFF_TRY(m4u_get_module_by_port(PortID), isCalled);
    
    return 0;
}


#define M4U_ERR_PAGE_UNLOCKED -101


int m4u_put_unlock_page(struct page* page)
{
    unsigned int pfn;
    int ret = 0;
    pfn = m4u_page_to_pfn(page);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_MUNLOCK], MMProfileFlagStart, 0, (unsigned int)(pfn<<12));

    if(pMlock_cnt[pfn])
    {
        if(!PageMlocked(page))
        {
            ret = M4U_ERR_PAGE_UNLOCKED;
        }
        
        pMlock_cnt[pfn]--;
        if(pMlock_cnt[pfn] == 0)
        {
			int i;

			for(i=0; i<3000; i++) {
				if (trylock_page(page)) {
					munlock_vma_page(page);
					unlock_page(page);
					break;
				}
			}
            if(PageMlocked(page)==1)
            {
                M4UERR(" Can't munlock page: %d\n", i);
                dump_page(page);
            }
        }
    }
    else
    {
        M4UMSG("warning pMlock_cnt[%d]==0 !! \n", pfn);
        ret = M4U_ERR_PAGE_UNLOCKED;
    }
    MMProfileLogEx(M4U_MMP_Events[PROFILE_MUNLOCK], MMProfileFlagEnd, 0, 0x1000);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_PUT_PAGE], MMProfileFlagStart, 0, pfn<<12);
    put_page(page);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_PUT_PAGE], MMProfileFlagEnd, 0, 0x1000);

    return ret;
    
}

///> m4u driver internal use function
///> should not be called outside m4u kernel driver
static int m4u_get_pages(M4U_MODULE_ID_ENUM eModuleID, struct task_struct * tsk,
                            unsigned int BufAddr, unsigned int BufSize, 
                            unsigned int* pPageTableAddr)
{
    int ret,i;
    int page_num;
    unsigned int start_pa;    
    unsigned int write_mode = 0;
    struct vm_area_struct *vma = NULL;
    
#ifdef GET_USER_PROF    
    unsigned long long get_user_time1, get_user_time2;
#endif
    
    M4ULOG("^ m4u_get_pages: module=%s, BufAddr=0x%x, BufSize=%d \n", m4u_get_module_name(eModuleID), BufAddr, BufSize);
    
    // caculate page number
    page_num = (BufSize + (BufAddr&0xfff))/DEFAULT_PAGE_SIZE;
    if((BufAddr+BufSize)&0xfff)
    {
        page_num++;
    }        

    if(M4U_CLNTMOD_LCDC_UI==eModuleID)
    {
        for(i=0;i<page_num;i++)
        {
            *(pPageTableAddr+i) = (BufAddr&0xfffff000) + i*DEFAULT_PAGE_SIZE;
        } 
       
    }  
    else if(BufAddr<PAGE_OFFSET)  // from user space
    {
        start_pa = m4u_user_v2p(BufAddr);
        if(0==start_pa)
        {
        	  M4UDBG("m4u_user_v2p=0 in m4u_get_pages() \n");
        }
        if(is_pmem_range((unsigned long*)start_pa, BufSize))
        {
            M4UERR("error: m4u_get_pages virtual addr from pmem! start_pa=0x%x\n", start_pa);
            for(i=0;i<page_num;i++)
            {
                *(pPageTableAddr+i) = m4u_user_v2p((BufAddr&0xfffff000) + i*DEFAULT_PAGE_SIZE);
            }   
        }    
        else 
        {
            struct mm_struct *tskmm = NULL;
            if(BufSize>MAX_BUF_SIZE_TO_GET_USER_PAGE)
            {
            	  //M4UMSG("error: m4u_get_pages(), size is bigger than 32MB size=%d \n", BufSize);
            	  M4UWARN(": m4u_get_pages(), single time alloc size=0x%x, bigger than limit=0x%x \n", BufSize, MAX_BUF_SIZE_TO_GET_USER_PAGE);
            	  return -EFAULT;
            } 

            tskmm = get_task_mm(tsk);
            if(!tskmm)
            {
                M4UMSG("error to get_task_mm! mm=0x%x, module=%s,\n", tskmm);
                M4UMSG("module=%s, va=0x%x, size=0x%x\n", m4u_get_module_name(eModuleID), BufAddr, BufSize);
                return -EFAULT;                
            }
            {   // here we hold reference count of tsk->mm

                down_read(&tskmm->mmap_sem);
                
                vma = find_vma(tskmm, BufAddr);
                if(vma == NULL)
                {
                    M4UWARN("m4u_get_pages: cannot find vma: module=%s, va=0x%x, size=0x%x\n", 
                        m4u_get_module_name(eModuleID), BufAddr, BufSize);
                    m4u_dump_maps(tsk, BufAddr);
                    
                    up_read(&tskmm->mmap_sem);
                    mmput(tskmm);
                    return -EFAULT;
                }
                write_mode = (vma->vm_flags&VM_WRITE)?1:0;

                if( (m4u_get_write_mode_by_module(eModuleID)==1) &&
                    (!(vma->vm_flags&VM_WRITE) || vma->vm_flags&VM_EXECUTABLE) )
                {
                   M4UMSG("error: write port alloc read-only or executable mem! m=%s, va=0x%x, size=0x%x \n", 
                       m4u_get_module_name(eModuleID), BufAddr, BufSize);
                }

                
        #ifdef GET_USER_PROF
                get_user_time1 = sched_clock();
        #endif
                ret = m4u_get_user_pages(
                    eModuleID,
                    tsk,
                    tskmm,
                    BufAddr,
                    page_num,
                    write_mode, //m4u_get_write_mode_by_module(eModuleID),	// 1 /* write */
                    0,	/* force */
                    (struct page **)pPageTableAddr,
                    NULL);
        
        #ifdef GET_USER_PROF
                get_user_time2 = sched_clock();
                up_read(&tskmm->mmap_sem);
                //M4UINF("[Get_page] get_user_pages: %llu ns\n", get_user_time2-get_user_time1);
        #else
                up_read(&tskmm->mmap_sem);
        #endif
            }   // we hold reference count of tsk->mm here
            mmput(tskmm);
            
            if(ret<page_num)
            {
            	  // release pages first
            	for(i=0;i<ret;i++)
                {
                    m4u_put_unlock_page(*((struct page **)pPageTableAddr+i));
                }
                
                if(unlikely(fatal_signal_pending(current))) 
                {
                    // get_user_pages() fail because the process is killed by signal_kill
                    // in such case, just return error, do not KE, because in such case, M4U user 
                    // can not resolve such issue either
                    M4UMSG("error: receive sigkill during get_user_pages(),  page_num=%d, return=%d, module=%s, current_process:%s \n", 
                        page_num, ret, m4u_get_module_name(eModuleID), current->comm);
                }
                else
                {
                    if(ret>0) //return value bigger than 0 but smaller than expected, trigger red screen
                    {
                        M4UMSG("error: page_num=%d, get_user_pages return=%d, module=%s, current_process:%s \n", 
                            page_num, ret, m4u_get_module_name(eModuleID), tsk->comm);                    	
                        M4UMSG("error hint: maybe the allocated VA size is smaller than the size configured to m4u_alloc_mva()!");
                    }
                    else  // return vaule is smaller than 0, maybe the buffer is not exist, just return error to up-layer
                    {                    	                    
                        M4UMSG("error: page_num=%d, get_user_pages return=%d, module=%s, current_process:%s \n", 
                            page_num, ret, m4u_get_module_name(eModuleID), tsk->comm);                    	
                        M4UMSG("error hint: maybe the VA is deallocated before call m4u_alloc_mva(), or no VA has be ever allocated!");
                    }
                    m4u_dump_maps(tsk, BufAddr);
                }
            
                return -EFAULT;                
            }

            // add locked pages count, used for debug whether there is memory leakage
            pmodule_locked_pages[eModuleID] += page_num;
                    
            for(i=0;i<page_num;i++)
            {
                *(pPageTableAddr+i) = m4u_page_to_phys(*((struct page **)pPageTableAddr+i));
            }		
    
            M4UDBG("\n [user verify] BufAddr_sv=0x%x, BufAddr_sp=0x%x, BufAddr_ev=0x%x, BufAddr_ep=0x%x \n",
                        BufAddr, 
                        m4u_user_v2p(BufAddr), 
                        BufAddr+BufSize-1, 
                        m4u_user_v2p(BufAddr+BufSize-4));                    
        }
    }
    else // from kernel space
    {
        if(BufAddr>=VMALLOC_START && BufAddr<=VMALLOC_END) // vmalloc
        {
            struct page * ppage;
            for(i=0;i<page_num;i++)
            {          	
                ppage=vmalloc_to_page((unsigned int *)(BufAddr + i*DEFAULT_PAGE_SIZE));            
                *(pPageTableAddr+i) = m4u_page_to_phys(ppage) & 0xfffff000 ;
            }
        }
        else // kmalloc
        {
            for(i=0;i<page_num;i++)
            {
                *(pPageTableAddr+i) = m4u_virt_to_phys((void*)((BufAddr&0xfffff000) + i*DEFAULT_PAGE_SIZE));
            }        	
        }
        
        M4UDBG("\n [kernel verify] BufAddr_sv=0x%x, BufAddr_sp=0x%x, BufAddr_ev=0x%x, BufAddr_ep=0x%x \n",
                    BufAddr, 
                    m4u_virt_to_phys((void*)BufAddr), 
                    BufAddr+BufSize-1, 
                    m4u_virt_to_phys(BufAddr+BufSize-4));

    }

    // invalid page entries in the page table
    for(i=0; i<page_num; i++)
    {
        *(pPageTableAddr+i) |= ACCESS_TYPE_4K_PAGE;
    }
	  
	  // flush write-buffer after CPU fill pagetable
	  // because the pagetable buffer is mapped to SHARED_DEVICE, ARM on-chip write-buffer is
	  // enabled for such kind of memory
	  mb();
	  
  
          
    // record memory usage
    pmodule_current_size[eModuleID] += BufSize;
    if(pmodule_current_size[eModuleID]>gModuleMaxMVASize[eModuleID])
    {    	 
    	 M4UERR_WITH_OWNER("error: MVA overflow, module=%s, Current alloc MVA=0x%x, Max MVA size=0x%x \n",  m4u_get_module_name(eModuleID), 
    	     m4u_get_module_name(eModuleID), pmodule_current_size[eModuleID], gModuleMaxMVASize[eModuleID]);
       m4u_memory_usage(false);
       m4u_dump_mva_info();
    }
    if(pmodule_current_size[eModuleID]> pmodule_max_size[eModuleID])
    {
        pmodule_max_size[eModuleID] = pmodule_current_size[eModuleID];
    }

    //m4u_print_active_port();
    //m4u_memory_usage(true);  
    //m4u_dump_maps(BufAddr);
    
    return page_num;
}

int m4u_release_pages(M4U_MODULE_ID_ENUM eModuleID, unsigned int BufAddr, unsigned int BufSize, unsigned int MVA)
{
    unsigned int page_num=0, i=0;
    unsigned int start_pa;
    struct page *page;
    int put_page_err = 0, tmp;
    M4ULOG("m4u_release_pages(),  module=%s, BufAddr=0x%x, BufSize=0x%x\n", m4u_get_module_name(eModuleID), BufAddr, BufSize);

    if(!mva_owner_match(eModuleID, mva2module(MVA)))
    {
        M4UERR_WITH_OWNER("m4u_release_pages module=%s, MVA=0x%x, expect module is %s \n", m4u_get_module_name(eModuleID), 
    	    m4u_get_module_name(eModuleID), MVA, m4u_get_module_name(mva2module(MVA)));
        m4u_mvaGraph_dump();
    }

    if(M4U_CLNTMOD_LCDC_UI==eModuleID)
    {
        goto RELEASE_FINISH;	
    }

    if(BufAddr<PAGE_OFFSET)  // from user space
    {	

        // put page by finding PA in pagetable
        unsigned int* pPageTableAddr = (unsigned int*)((unsigned int) pPageTableVA_nonCache + M4U_GET_PTE_OFST_TO_PT_SA(MVA)); 
        page_num = (BufSize + (BufAddr&0xfff))/DEFAULT_PAGE_SIZE;
        if((BufAddr+BufSize)&0xfff)
        {
            page_num++;
        } 

        for(i=0;i<page_num;i++)
        {
            start_pa = *(pPageTableAddr+i);
            if((start_pa&0x02)==0)
            {
                continue;
            }
            page = m4u_pfn_to_page(__phys_to_pfn(start_pa));
    	  
            //we should check page count before call put_page, because m4u_release_pages() may fail in the middle of buffer
            //that is to say, first several pages may be put successfully in m4u_release_pages()
            if(page_count(page)>0) 
            {
                //to avoid too much log, we only save tha last err here.
                if((tmp=m4u_put_unlock_page(page)))
                    put_page_err = tmp;
            }         
            pmodule_locked_pages[eModuleID]--;   
            *(pPageTableAddr+i) &= (~0x2); 
        }
    

        if(put_page_err == M4U_ERR_PAGE_UNLOCKED)
        {
            M4UMSG("warning: in m4u_release_page: module=%s, va=0x%x, size=0x%x,mva=0x%x (page is unlocked before put page)\n", 
                m4u_get_module_name(eModuleID), BufAddr, BufSize, MVA);
        }
    } //end of "if(BufAddr<PAGE_OFFSET)"

RELEASE_FINISH:
    // record memory usage
    if(pmodule_current_size[eModuleID]<BufSize)
    {
        pmodule_current_size[eModuleID] = 0;
        M4UMSG("error pmodule_current_size is less than BufSize, module=%s, current_size=%d, BufSize=%d \n", 
           m4u_get_module_name(eModuleID), pmodule_current_size[eModuleID], BufSize);
    }
    else
    {
        pmodule_current_size[eModuleID] -= BufSize;
    }

    return 0;
}


// Refer to dma_cache_maint().
// The function works for user virtual addr 
#define BUFFER_SIZE_FOR_FLUSH_ALL (864*480*2)
int L1_CACHE_SYNC_BY_RANGE_ONLY = 0;



int m4u_dma_cache_maint(M4U_MODULE_ID_ENUM eModuleID, const void *start, size_t size, int direction)
{
    void (*outer_op)(phys_addr_t start, phys_addr_t end);
//	void (*outer_op)(unsigned long, unsigned long);
	void (*outer_op_all)(void);
	unsigned int page_start, page_num;
    unsigned int *pPhy = NULL;
    int i, ret=0;
    PROFILE_TYPE ptype=PROFILE_DMA_MAINT_ALL;
    switch (direction) {
	case DMA_FROM_DEVICE:
        if(size < BUFFER_SIZE_FOR_FLUSH_ALL)
            ptype = PROFILE_DMA_INVALID_RANGE;
        else
            ptype = PROFILE_DMA_INVALID_ALL;
		break;
	case DMA_TO_DEVICE:
        if(size < BUFFER_SIZE_FOR_FLUSH_ALL)
            ptype = PROFILE_DMA_CLEAN_RANGE;
        else
            ptype = PROFILE_DMA_CLEAN_ALL;
        break;
	case DMA_BIDIRECTIONAL:
        if(size < BUFFER_SIZE_FOR_FLUSH_ALL)
            ptype = PROFILE_DMA_FLUSH_RANGE;
        else
            ptype = PROFILE_DMA_FLUSH_ALL;
		break;
	default:
        break;
	}
    MMProfileLogEx(M4U_MMP_Events[ptype], MMProfileFlagStart, eModuleID, (unsigned int)start);

    M4ULOG(" m4u_dma_cache_maint():  module=%s, start=0x%x, size=%d, direction=%d \n",
          m4u_get_module_name(eModuleID), (unsigned int)start, size, direction);

  if(0==start)
  {
      M4UERR_WITH_OWNER(" m4u_dma_cache_maint():  module=%s, start=0x%x, size=%d, direction=%d \n", m4u_get_module_name(eModuleID), 
          m4u_get_module_name(eModuleID), (unsigned int)start, size, direction);
  	  return -1;
  }         

    mutex_lock(&gM4uMutex);
   
  //To avoid non-cache line align cache corruption, user should make sure
  //cache start addr and size both cache-line-bytes align
  //we check start addr here but size should be checked in memory allocator
  //Rotdma memory is allocated by surfacefligner, address is not easy to modify
  //so do not check them now, should followup after MP
    if( m4u_get_dir_by_module(eModuleID)== M4U_DMA_WRITE &&
        (((unsigned int)start%L1_CACHE_BYTES!=0) || (size%L1_CACHE_BYTES)!=0)&&
        M4U_CLNTMOD_ROT_GENERAL != eModuleID &&
        M4U_CLNTMOD_VDO_ROT0 != eModuleID && 
        M4U_CLNTMOD_RGB_ROT0 != eModuleID && 
        M4U_CLNTMOD_RGB_ROT1 != eModuleID &&
        M4U_CLNTMOD_VDO_ROT1 != eModuleID &&
        M4U_CLNTMOD_RGB_ROT2 != eModuleID
       )
    {
        if(1) //screen red in debug mode
        {
      		M4UERR_WITH_OWNER("error: addr un-align, module=%s, addr=0x%x, size=0x%x, process=%s, align=0x%x\n",  m4u_get_module_name(eModuleID), 
      	        m4u_get_module_name(eModuleID), (unsigned int)start, size, current->comm, L1_CACHE_BYTES);
      	}
      	else
      	{
      		M4UMSG("error: addr un-align, module=%s, addr=0x%x, size=0x%x, process=%s, align=0x%x\n", 
      	        m4u_get_module_name(eModuleID), (unsigned int)start, size, current->comm, L1_CACHE_BYTES);
      	}
    }
          
	switch (direction) {
	case DMA_FROM_DEVICE:		/* invalidate only, HW write to memory */
        M4UMSG("error: someone call cache maint with DMA_FROM_DEVICE, module=%s\n",m4u_get_module_name(eModuleID));
		outer_op = outer_inv_range;
		outer_op_all = outer_inv_all;  
		break;
	case DMA_TO_DEVICE:		/* writeback only, HW read from memory */
		outer_op = outer_clean_range;
		outer_op_all = outer_flush_all;
		break;
	case DMA_BIDIRECTIONAL:		/* writeback and invalidate */
		outer_op = outer_flush_range;
		outer_op_all = outer_flush_all;
		break;
	default:
		M4UERR("m4u_dma_cache_maint, direction=%d is invalid \n", direction);
        return -1;
	}


//<===========================================================================
//< check wether input buffer is valid (has physical pages allocated)
	page_start = (unsigned int)start & 0xfffff000;
	page_num = (size + ((unsigned int)start & 0xfff)) / DEFAULT_PAGE_SIZE;
	if(((unsigned int)start + size) & 0xfff)
		page_num++;

    if(size < BUFFER_SIZE_FOR_FLUSH_ALL)
    {
        pPhy = kmalloc(sizeof(int)*page_num, GFP_KERNEL);
        if(pPhy == NULL)
        {
            M4UMSG("error to kmalloc in m4u_cache_maint: module=%s, start=0x%x, size=%d, direction=%d \n", 
                m4u_get_module_name(eModuleID), (unsigned int)start, size, direction);
            goto out;
        }

        if((unsigned int)start<PAGE_OFFSET)  // from user space
        {
            for(i=0; i<page_num; i++,page_start+=DEFAULT_PAGE_SIZE)
            {
                struct page* page;
                pPhy[i] = m4u_user_v2p(page_start);
                page = m4u_phys_to_page(pPhy[i]);
                
                if((pPhy[i]==0) || (!PageMlocked(page))) 
                {
                    ret=-1;
                    M4UMSG("error: cache_maint() fail, module=%s, start=0x%x, page_start=0x%x, size=%d, pPhy[i]=0x%x\n", 
                            m4u_get_module_name(eModuleID), (unsigned int)start, (unsigned int)page_start, size, pPhy[i]);
                    dump_page(page);
                    m4u_dump_maps(current, (unsigned int)start);
                    goto out;
                }
            }
        }
        else if((unsigned int)start>=VMALLOC_START && (unsigned int)start<=VMALLOC_END) // vmalloc
        {

            struct page * ppage;

            for(i=0; i<page_num; i++,page_start+=DEFAULT_PAGE_SIZE)
            {
                ppage=vmalloc_to_page((void *)page_start); 
                if(ppage == NULL) 
                {
                    ret=-1;
                    M4UMSG("error: ppage is 0 in cache_maint of vmalloc!, module=%s, start=0x%x, pagestart=0x%x\n", 
                            m4u_get_module_name(eModuleID), (unsigned int)start,page_start);
                    goto out;
                }
                pPhy[i] = m4u_page_to_phys(ppage);
            }
        }
        else // kmalloc
        {
            for(i=0; i<page_num; i++,page_start+=DEFAULT_PAGE_SIZE)
            {
                pPhy[i] = m4u_virt_to_phys((void*)page_start);
            }        	
        }
        
    }

//=====================================================================================
// L1 cache clean before hw read
    if(L1_CACHE_SYNC_BY_RANGE_ONLY)
    {
    	if (direction == DMA_TO_DEVICE) 
    	{
            dmac_map_area(start, size, direction);
    	}

    	if (direction == DMA_BIDIRECTIONAL) 
    	{
            dmac_flush_range(start, start+size-1);
    	}

    }
    else
    {
        smp_inner_dcache_flush_all();
    }

//=============================================================================================
	// L2 cache maintenance by physical pages
    if(size<BUFFER_SIZE_FOR_FLUSH_ALL)
    {
        for (i=0; i<page_num; i++) 
        {
    		outer_op(pPhy[i], pPhy[i]+ DEFAULT_PAGE_SIZE);
    	}
    }
    else 
    {
        outer_op_all();
    }
//=========================================================================================      
	// L1 cache invalidate after hw write to memory
    if(L1_CACHE_SYNC_BY_RANGE_ONLY)
    {
    	if (direction == DMA_FROM_DEVICE) 
        {
    	    dmac_unmap_area(start, size, direction);
        }
    }
  
out:
    if(pPhy != NULL)
        kfree(pPhy);

    MMProfileLogEx(M4U_MMP_Events[ptype], MMProfileFlagEnd, eModuleID, size);

    mutex_unlock(&gM4uMutex);
        
    return ret;
}



int m4u_dma_cache_flush_all()
{

   // M4UMSG("cache flush all!!\n")
    mutex_lock(&gM4uMutex);
    MMProfileLogEx(M4U_MMP_Events[PROFILE_CACHE_FLUSH_ALL], MMProfileFlagStart, 0, 0);

    // L1 cache clean before hw read
    smp_inner_dcache_flush_all();
     
	// L2 cache maintenance by physical pages
    outer_flush_all();
    
	MMProfileLogEx(M4U_MMP_Events[PROFILE_CACHE_FLUSH_ALL], MMProfileFlagEnd, 0, 0);
    mutex_unlock(&gM4uMutex);
   
    return 0;
}



#if 0
static M4U_DMA_DIR_ENUM m4u_get_dir_by_port(M4U_PORT_ID_ENUM portID)
{
    
    M4U_DMA_DIR_ENUM dir;
    switch(portID)  // from user space
    {
        case M4U_PORT_UNKNOWN      : 
        case M4U_PORT_DEFECT       :
        case M4U_PORT_CAM          :
        case M4U_PORT_PCA          :
        	  dir = M4U_DMA_READ_WRITE;   // bi-direction ports
        	  break;
        	  
        case M4U_PORT_FD2          :
        case M4U_PORT_JPEG_DEC     :
        case M4U_PORT_R_DMA0_OUT0  :
        case M4U_PORT_R_DMA0_OUT1  :
        case M4U_PORT_R_DMA0_OUT2  :
        case M4U_PORT_OVL_MASK     :
        case M4U_PORT_OVL_DCP      :
        case M4U_PORT_DPI          :
        case M4U_PORT_TVC          :
        case M4U_PORT_LCD_R        :
        case M4U_PORT_R_DMA1_OUT0  :
        case M4U_PORT_R_DMA1_OUT1  :
        case M4U_PORT_R_DMA1_OUT2  :
        case M4U_PORT_VENC_MC      :
        case M4U_PORT_VENC_MVQP    :
        case M4U_PORT_VDEC_DMA     :
        case M4U_PORT_VDEC_POST0   :
        case M4U_PORT_G2D_R        :
            dir = M4U_DMA_READ;
            break;

        case M4U_PORT_AUDIO        :
        case M4U_PORT_G2D_W        :
        case M4U_PORT_VDEC_REC     :
        case M4U_PORT_VENC_BSDMA   :
        case M4U_PORT_SPI          :
        case M4U_PORT_FD1          :
        case M4U_PORT_JPEG_ENC     :
        case M4U_PORT_VDO_ROT0_OUT0:
        case M4U_PORT_RGB_ROT0_OUT0:
        case M4U_PORT_TV_ROT_OUT0  :
        case M4U_PORT_FD0          :
        case M4U_PORT_RGB_ROT1_OUT0:
        case M4U_PORT_VDO_ROT1_OUT0:
        case M4U_PORT_RGB_ROT2_OUT0:
        case M4U_PORT_LCD_W        :        	
            dir = M4U_DMA_WRITE;
            break;

        default:
            M4UDBG("error, can not get port's direction \n");
            dir = M4U_DMA_READ_WRITE;
            break;
    }

    return dir;
}

#endif

static M4U_DMA_DIR_ENUM m4u_get_dir_by_module(M4U_MODULE_ID_ENUM eModuleID)
{
    
    M4U_DMA_DIR_ENUM dir;
    switch(eModuleID)  // from user space
    {
        case M4U_CLNTMOD_DEFECT:
        case M4U_CLNTMOD_CAM:
        case M4U_CLNTMOD_PCA: 
        case M4U_CLNTMOD_FD_INPUT:
        case M4U_CLNTMOD_VDEC_DMA:
        case M4U_CLNTMOD_VENC_DMA: 
        case M4U_CLNTMOD_G2D:
          dir = M4U_DMA_READ_WRITE;
          break;                   

        case M4U_CLNTMOD_DPI: 
        case M4U_CLNTMOD_AUDIO:
        case M4U_CLNTMOD_RDMA_GENERAL:
        case M4U_CLNTMOD_RDMA0:
        case M4U_CLNTMOD_OVL:        
        case M4U_CLNTMOD_LCDC:
        case M4U_CLNTMOD_LCDC_UI:
        case M4U_CLNTMOD_RDMA1: 
        case M4U_CLNTMOD_TVC:        
        case M4U_CLNTMOD_JPEG_DEC: 
        case M4U_CLNTMOD_FD:        	
          dir = M4U_DMA_READ;
          break;                

        case M4U_CLNTMOD_SPI:
        case M4U_CLNTMOD_ROT_GENERAL:
        case M4U_CLNTMOD_RGB_ROT1:    
        case M4U_CLNTMOD_VDO_ROT1:
        case M4U_CLNTMOD_RGB_ROT2:        
        case M4U_CLNTMOD_VDO_ROT0:     
        case M4U_CLNTMOD_RGB_ROT0:    
        case M4U_CLNTMOD_TVROT: 
        case M4U_CLNTMOD_JPEG_ENC: 
          dir = M4U_DMA_WRITE;
          break;

        default:
            M4UMSG("error: can not get port's direction, module=%s \n", m4u_get_module_name(eModuleID));
            dir = M4U_DMA_READ_WRITE;
            break;
    }

    return dir;
}



#if 1
/**
 * @brief ,             
 * @param , tlbSelect 0:main tlb, 1:pre-fetch tlb LSB, 2:pre-fetch tlb MSB
 * @return 
 */
int m4u_get_descriptor(unsigned int m4u_base, M4U_DESC_TLB_SELECT_ENUM tlbSelect, unsigned int tlbIndex)
{
    unsigned regValue=0;
    if(M4U_DESC_MAIN_TLB==tlbSelect)
    {
        regValue &= ~(1<<12);
    }
    else if(M4U_DESC_PRE_TLB_LSB==tlbSelect)
    {
        regValue |= (1<<12);
        regValue &= ~(1<<3);
    }
    else if(M4U_DESC_PRE_TLB_MSB==tlbSelect)
    {
        regValue |= (1<<12);
        regValue |= (1<<3);
    }
    else
    {
        M4UDBG("m4u_get_descriptor failed,  tlbSelect=%d \n", tlbSelect);
        return 0;
    }
    regValue |= tlbIndex<<4;
    regValue |= 1;
    M4U_WriteReg32(m4u_base, M4UMACRO0104, regValue);  // config read entry control register
    regValue = M4U_ReadReg32(m4u_base, M4UMACRO0105);   // entry descriptor read data

    return regValue;
}



#if 0
int m4u_dma_cache_clean_kernel(const void *start, size_t size)
{

    M4ULOG(" m4u_dma_cache_clean_kernel(): start=0x%x, size=%d \n",
          (unsigned int)start, size);

    if(0==start)
    {
        M4UERR(" m4u_dma_cache_clean_kernel() \n");
    	return -1;
    }         

          
//=====================================================================================
// L1 cache maintenance when going to devices
	if(size<BUFFER_SIZE_FOR_FLUSH_ALL)
	{
        dmac_map_area(start, size, DMA_TO_DEVICE);
	}
	else
	{
        //inner_dcache_flush_all();
	}
//=============================================================================================
	// L2 cache maintenance by physical pages
    if(size<BUFFER_SIZE_FOR_FLUSH_ALL)
    {
        outer_clean_range(m4u_virt_to_phys(start), m4u_virt_to_phys(start)+size-1);
    }
    else 
    {
        outer_clean_all();
    }
	
    return 0;
}

#endif

#endif



#define M4U_PAGE_TABLE_OFFSET (64*1024-1) // page table addr should (2^16)x align
#define M4U_PROTECT_BUF_OFFSET (128-1)    // protect buffer should be 128x align
static bool m4u_struct_init(void)
{
    //unsigned int Temp=0, Size=0;
    //int i;
    unsigned int *pPageTableVA=NULL, *pProtectVA=NULL;  //Page Table virtual Address


    //======= alloc pagetable=======================
    pPageTableVA_nonCache = dma_alloc_coherent(NULL, PT_TOTAL_ENTRY_NUM * sizeof(unsigned int), &PageTablePA, GFP_KERNEL);
    if(!pPageTableVA_nonCache)
    {
        M4UMSG("dma_alloc_coherent error!  dma memory not available.\n");
        return false;
    }
    if((PageTablePA&0x0000ffff)!=0)
    {
        M4UMSG("dma_alloc_coherent memory not align. PageTablePA=0x%x we will try again \n", PageTablePA);
        dma_free_coherent(NULL, PT_TOTAL_ENTRY_NUM * sizeof(unsigned int), pPageTableVA, PageTablePA);
        pPageTableVA = dma_alloc_coherent(NULL, PT_TOTAL_ENTRY_NUM * sizeof(unsigned int)+M4U_PAGE_TABLE_OFFSET, &PageTablePA, GFP_KERNEL);
        if(!pPageTableVA)
        {
            M4UMSG("dma_alloc_coherent error!  dma memory not available.\n");
            return false;
        }
        pPageTableVA_nonCache = (unsigned int*)(((unsigned int)pPageTableVA+M4U_PAGE_TABLE_OFFSET)&(~M4U_PAGE_TABLE_OFFSET));
        PageTablePA += (unsigned int)pPageTableVA_nonCache - (unsigned int)pPageTableVA;
    }
    
    M4UMSG("dma_alloc_coherent success! pagetable_va=0x%x, pagetable_pa=0x%x.\n", (unsigned int)pPageTableVA_nonCache, (unsigned int)PageTablePA);
    memset((void*)pPageTableVA_nonCache, 0, PT_TOTAL_ENTRY_NUM * sizeof(unsigned int));
    //======= alloc pagetable done=======================

    init_mlock_cnt();
    if(NULL==pMlock_cnt)
        return false;
          
    // allocate 128 byte for translation fault protection
    // when TF occurs, M4U will translate the physical address to ProtectPA
    pProtectVA = (unsigned int*) kmalloc(TF_PROTECT_BUFFER_SIZE*TOTAL_M4U_NUM+M4U_PROTECT_BUF_OFFSET, GFP_KERNEL|__GFP_ZERO);
    if(NULL==pProtectVA)
    {
        
        M4UMSG("Physical memory not available.\n");
        return false;
    }
    pProtectVA = (unsigned int*)(((unsigned int)pProtectVA+M4U_PROTECT_BUF_OFFSET)&(~M4U_PROTECT_BUF_OFFSET));
    ProtectPA = m4u_virt_to_phys(pProtectVA);
    if((ProtectPA&0x7f)!=0)
    {        
        M4UERR("Physical memory not align. ProtectPA=0x%x \n", ProtectPA);
    }  
    pProtectVA_nonCache = pProtectVA;
    memset((unsigned char*)pProtectVA_nonCache, 0x55, TF_PROTECT_BUFFER_SIZE*TOTAL_M4U_NUM);

    //clean all cache after ioremap, because after kmalloc, some data may in cache.
//    m4u_dma_cache_clean_all();

    M4UDBG("ProtectTablePA:0x%x, ProtectTableVA:0x%x, pProtectVA_nonCache:0x%x \n", 
        ProtectPA, (unsigned int)pProtectVA, (unsigned int)pProtectVA_nonCache);
           
    //initialize global variables
    pRangeDes = kmalloc(sizeof(M4U_RANGE_DES_T) * TOTAL_RANGE_NUM, GFP_KERNEL|__GFP_ZERO);
    if(NULL==pRangeDes)
    {
        
        M4UMSG("Physical memory not available.\n");
        return false;
    }
    
    pWrapDes = kmalloc(sizeof(M4U_WRAP_DES_T) * TOTAL_WRAP_NUM, GFP_KERNEL|__GFP_ZERO);
    if(NULL==pWrapDes)
    {
        
        M4UMSG("Physical memory not available.\n");
        return false;
    }

    pM4URegBackUp = (unsigned int*)kmalloc(BACKUP_REG_SIZE, GFP_KERNEL|__GFP_ZERO);
    if(pM4URegBackUp==NULL)
    {
    	  M4UERR("pM4URegBackUp kmalloc fail \n");
    }    


    mutex_init(&(mva_global_link.dataMutex));
    mutex_lock(&(mva_global_link.dataMutex));
    INIT_LIST_HEAD(&(mva_global_link.mvaList));
    mutex_unlock(&(mva_global_link.dataMutex));
            
         
    //TODO, move hw init into  power on func
    m4u_hw_init();
	 
    gM4uLogFlag = false; 
    gIsSuspend = false;

    gtemp = (char*)vmalloc(1024);
    ASSERT(gtemp!=NULL);
    return 0;
}


/**
 * @brief ,     system power on / return from power resume        
 * @param 
 * @return 
 */
static bool m4u_hw_init(void)
{
    unsigned int i;
    M4UDBG("m4u_hw_init() \n");

    for(i=0;i<TOTAL_M4U_NUM;i++)
    {
        M4UDBG("gM4UBaseAddr[%d]=0x%x \n",i, gM4UBaseAddr[i]);
    }

    for(i=0;i<TOTAL_M4U_NUM;i++)
    {
    	  //set page table base address
        M4U_WriteReg32(gM4UBaseAddr[i], M4UMACRO0005, PageTablePA);
		
        //enable interrupt control except "Same VA-to-PA test"
        M4U_WriteReg32(gM4UBaseAddr[i], M4UMACRO0108, 0x7F);
        
        //invalidate all TLB entry
        SMI_WriteReg32(M4UMACRO0194, M4UMACRO0195);
        
        //enable "Table Walk Logic" and "PF TLB" and disable performance monitor
        //set the translation fault replace address to a valid physical address to avoid invalid 
        //physical address interrupt occurs together with translation fault
        M4U_WriteReg32(gM4UBaseAddr[i], M4UMACRO0106, 0x40);
        M4U_WriteReg32(gM4UBaseAddr[i], M4UMACRO0107, ProtectPA+TF_PROTECT_BUFFER_SIZE*i);

        M4UDBG("init hw OK: %d \n",i);
    }

    return true;
}


/**
 * @brief , 
 * @param 
 * @return 
 */
static bool m4u_hw_reinit(void)
{
    unsigned int Temp;
    unsigned int i, j;
    M4UDBG("m4u_hw_reinit() \n");
    for(j=0;j<TOTAL_M4U_NUM;j++)
    {    	
        ///> clear all RT, SQ SA&EA registers
        Temp = M4UMACRO0013;
        i = TOTAL_RANGE_NUM*2;
        while(i--)
        {
            M4U_WriteReg32(gM4UBaseAddr[j], Temp, 0);
            Temp += 4;
        }
        
        ///> invalidate all tags
        //eM4UInvldTLBAll();
        
        ///> zero-init all tags to ease FPGA debugging
        Temp = M4UMACRO0040;
        i = 64;
        while(i--)
        {
            M4U_WriteReg32(gM4UBaseAddr[j], Temp, 0); 
            Temp += 4;
        }
        
        ///> reset distance to default sequential distance (distance = 1) for all ports
        Temp = M4UMACRO0030;
        i = 6;
        while(i--)
        {
            M4U_WriteReg32(gM4UBaseAddr[j], Temp, 0x11111111);
            Temp += 4;
        }
        
        ///> reset MMU control register to enable "Table Walk Logic" and "PF TLB" and disable monitor
        M4U_WriteReg32(gM4UBaseAddr[j], M4UMACRO0106, 0x40);
    }
    return true;
}

static void m4u_clear_intr(unsigned int m4u_base)
{
    unsigned int Temp;
    Temp = M4U_ReadReg32(m4u_base, M4UMACRO0108) | M4UMACRO0109;
    M4U_WriteReg32(m4u_base, M4UMACRO0108, Temp);   
}

int m4u_reg_backup(M4U_MODULE_ID_ENUM eModuleID)
{
    unsigned int m4u_base = gM4UBaseAddr[m4u_get_index_by_module(eModuleID)];
    unsigned int* pReg = pM4URegBackUp + (M4U_REG_SIZE*m4u_get_index_by_module(eModuleID))/sizeof(unsigned int);

    // open all clock bits before read/write registers
    // to make sure all registers can be backup successfuly
    M4ULOG("m4u_reg_backup() index=%d, PTBase=0x%x \n", 
    m4u_get_index_by_module(eModuleID), M4U_ReadReg32(m4u_base, M4UMACRO0005));

    // M4UMSG("before m4u_reg_backup");
    // m4u_dump_reg(eModuleID);   
            
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0005);
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0006	   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0008	   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0012   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0013);
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0015  );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0016);
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0017  );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0018);
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0019  );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0020);
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0021  );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0022);
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0023  );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0024);
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0025  );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0026);
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0027  );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0028);
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0029  );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0030  );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0031  );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0032  );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0033  );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0034  );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0035  );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0036 );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0037 );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0038   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0039   );    
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0106   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0107 );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0108);
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0125   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0126   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0127   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0128   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0129   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0130   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0131   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0132   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0133   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0134   );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0135   );   
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0137   );   
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0141   );   
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0144   );   
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0152   );   
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0155   );   
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0156   );   
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0157   );   
    
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0159     );
    *(pReg++) = M4U_ReadReg32(m4u_base, M4UMACRO0160     );
    *(pReg++) = SMI_ReadReg32(M4UMACRO0213);
    *(pReg++) = SMI_ReadReg32(M4UMACRO0214);
    *(pReg++) = SMI_ReadReg32(M4UMACRO0215);
    *(pReg++) = SMI_ReadReg32(M4UMACRO0216);
    *(pReg++) = SMI_ReadReg32(M4UMACRO0217);

    if(NULL==(pM4URegBackUp + (M4U_REG_SIZE*m4u_get_index_by_module(eModuleID))/sizeof(unsigned int)) || 
    	 PageTablePA!=*(pM4URegBackUp + (M4U_REG_SIZE*m4u_get_index_by_module(eModuleID))/sizeof(unsigned int)))
    {
    	  M4UERR("PT_BASE in memory is error after backup! expect PTPA=0x%x, backupReg=0x%x, larb=%d \n", 
            PageTablePA, *(pM4URegBackUp + (M4U_REG_SIZE*m4u_get_index_by_module(eModuleID))/sizeof(unsigned int)),
            m4u_get_index_by_module(eModuleID));
    }
    
    return 0;
}

int m4u_reg_restore(M4U_MODULE_ID_ENUM eModuleID)
{
    unsigned int m4u_base = gM4UBaseAddr[m4u_get_index_by_module(eModuleID)];
    unsigned int* pReg = pM4URegBackUp + M4U_REG_SIZE*m4u_get_index_by_module(eModuleID)/sizeof(unsigned int);

    // open all clock bits before read/write registers
    // to make sure all registers can be backup successfuly

    M4ULOG("m4u_reg_restore() index=%d, PTBase=0x%x \n", 
        m4u_get_index_by_module(eModuleID), M4U_ReadReg32(m4u_base, M4UMACRO0005));


    if(NULL==pReg || 0==*pReg)
    {
    	  M4UERR("PT_BASE in memory is 0 before restore! larb=%d \n", m4u_get_index_by_module(eModuleID));
    }
        
    M4U_WriteReg32(m4u_base, M4UMACRO0005, *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0006	   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0008	   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0012    , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0013 , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0015   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0016 , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0017   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0018 , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0019   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0020 , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0021   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0022 , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0023   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0024 , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0025   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0026 , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0027   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0028 , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0029   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0030   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0031   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0032   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0033   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0034   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0035   , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0036  , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0037  , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0038    , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0039    , *(pReg++));    
    M4U_WriteReg32(m4u_base, M4UMACRO0106    , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0107  , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0108 , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0125    , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0126    , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0127    , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0128    , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0129    , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0130    , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0131    , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0132    , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0133    , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0134    , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0135    , *(pReg++)); 
    M4U_WriteReg32(m4u_base, M4UMACRO0137_RESET    , 0xFFFFFFFF); 
    M4U_WriteReg32(m4u_base, M4UMACRO0137_SET    , *(pReg++)); 
    M4U_WriteReg32(m4u_base, M4UMACRO0141_CLR    , 0xFFFFFFFF); 
    M4U_WriteReg32(m4u_base, M4UMACRO0141_SET    , *(pReg++)); 
    M4U_WriteReg32(m4u_base, M4UMACRO0144    , *(pReg++)); 
    M4U_WriteReg32(m4u_base, M4UMACRO0152    , *(pReg++)); 
    M4U_WriteReg32(m4u_base, M4UMACRO0155    , *(pReg++)); 
    M4U_WriteReg32(m4u_base, M4UMACRO0156    , *(pReg++)); 
    M4U_WriteReg32(m4u_base, M4UMACRO0157    , *(pReg++)); 
    
    M4U_WriteReg32(m4u_base, M4UMACRO0159      , *(pReg++));
    M4U_WriteReg32(m4u_base, M4UMACRO0160      , *(pReg++));   

    SMI_WriteReg32(M4UMACRO0213, *(pReg++));   
    SMI_WriteReg32(M4UMACRO0214, *(pReg++));   
    SMI_WriteReg32(M4UMACRO0215, *(pReg++));   
    SMI_WriteReg32(M4UMACRO0216, *(pReg++));   
    SMI_WriteReg32(M4UMACRO0217, *(pReg++));   
    
    // M4UMSG("after m4u_reg_restore");
    // m4u_dump_reg(eModuleID);  

  
    if(M4U_ReadReg32(m4u_base, M4UMACRO0005) != PageTablePA)
    {
    	  M4UERR("PT_BASE is error after restore! 0x%x != 0x%x  larb=%d \n",
            M4U_ReadReg32(m4u_base, M4UMACRO0005), PageTablePA, 
            m4u_get_index_by_module(eModuleID));
    }


    
    return 0;
}

static M4U_MODULE_ID_ENUM m4u_get_module_by_index(unsigned int index)
{
    switch(index)
    {
        case 0: return M4U_CLNTMOD_DEFECT;
        case 1: return M4U_CLNTMOD_RGB_ROT1;
        case 2: return M4U_CLNTMOD_VDEC_DMA;
        case 3: return M4U_CLNTMOD_G2D;
        default: 
            M4UMSG("m4u_get_module_by_index invalid index=%d \n", index);
            return M4U_CLNTMOD_DEFECT;
    } 
}

static M4U_PORT_ID_ENUM m4u_get_port_by_index(unsigned int index)
{
    switch(index)
    {
        case 0: return M4U_PORT_DEFECT;
        case 1: return M4U_PORT_OVL_MASK;
        case 2: return M4U_PORT_VENC_MC;
        case 3: return M4U_PORT_G2D_W;
        default: 
            M4UMSG("m4u_get_port_by_index invalid index=%d \n", index);
            return M4U_PORT_DEFECT;
    } 
}

static int m4u_get_index_by_module(M4U_MODULE_ID_ENUM moduleID)
{
    unsigned int m4u_index = 0;
    switch(moduleID)
    {
        case M4U_CLNTMOD_DEFECT:
        case M4U_CLNTMOD_CAM:
        case M4U_CLNTMOD_PCA: 
        case M4U_CLNTMOD_JPEG_DEC:        
        case M4U_CLNTMOD_JPEG_ENC:  
        case M4U_CLNTMOD_VDO_ROT0:        
        case M4U_CLNTMOD_RGB_ROT0:    
        case M4U_CLNTMOD_TVROT:        
        case M4U_CLNTMOD_RDMA0:
        case M4U_CLNTMOD_FD:
        case M4U_CLNTMOD_FD_INPUT:
            m4u_index = 0;
            break;
            
        case M4U_CLNTMOD_RGB_ROT1:
        case M4U_CLNTMOD_VDO_ROT1:
        case M4U_CLNTMOD_RGB_ROT2:
        case M4U_CLNTMOD_OVL:        
        case M4U_CLNTMOD_LCDC:
        case M4U_CLNTMOD_LCDC_UI:
        case M4U_CLNTMOD_RDMA1:
        case M4U_CLNTMOD_TVC: 
        case M4U_CLNTMOD_SPI:
        case M4U_CLNTMOD_DPI:
            m4u_index = 1;
            break;

        case M4U_CLNTMOD_VDEC_DMA:
        case M4U_CLNTMOD_VENC_DMA: 
            m4u_index = 2;
            break;
            
        case M4U_CLNTMOD_G2D:
        case M4U_CLNTMOD_AUDIO:
            m4u_index = 3;
            break;
            
        default:
            M4UMSG("m4u_get_index_by_module() fail, invalid moduleID=%d", moduleID);
    }
    // M4UDBG("get_index_by_module() moduleID=%d, index=%d \n", moduleID, m4u_index);
    return m4u_index;
}

static int m4u_get_index_by_port(M4U_PORT_ID_ENUM portID)
{
    unsigned int m4u_index = 0;
    switch(portID)
    {
        case M4U_PORT_DEFECT       :
        case M4U_PORT_JPEG_ENC     :
        case M4U_PORT_VDO_ROT0_OUT0:
        case M4U_PORT_RGB_ROT0_OUT0:
        case M4U_PORT_TV_ROT_OUT0  :
        case M4U_PORT_CAM          :
        case M4U_PORT_FD0          :
        case M4U_PORT_FD2          :
        case M4U_PORT_JPEG_DEC     :
        case M4U_PORT_R_DMA0_OUT0  :
        case M4U_PORT_R_DMA0_OUT1  :
        case M4U_PORT_R_DMA0_OUT2  :
        case M4U_PORT_FD1          :
        case M4U_PORT_PCA          :
        	m4u_index = 0;
        	break;

        case M4U_PORT_OVL_MASK     :
        case M4U_PORT_OVL_DCP      :
        case M4U_PORT_DPI          :
        case M4U_PORT_RGB_ROT1_OUT0:
        case M4U_PORT_VDO_ROT1_OUT0:
        case M4U_PORT_RGB_ROT2_OUT0:
        case M4U_PORT_TVC          :
        case M4U_PORT_LCD_R        :
        case M4U_PORT_LCD_W        :
        case M4U_PORT_R_DMA1_OUT0  :
        case M4U_PORT_R_DMA1_OUT1  :
        case M4U_PORT_R_DMA1_OUT2  :
        case M4U_PORT_SPI          :                   
        	m4u_index = 1;
        	break;
        	
        case M4U_PORT_VENC_MC      :
        case M4U_PORT_VENC_BSDMA   :
        case M4U_PORT_VENC_MVQP    :
        case M4U_PORT_VDEC_DMA     :
        case M4U_PORT_VDEC_REC     :
        case M4U_PORT_VDEC_POST0   :
        	m4u_index = 2;
        	break;
        	
        case M4U_PORT_G2D_W        :
        case M4U_PORT_G2D_R        :
        case M4U_PORT_AUDIO        :
        	m4u_index = 3;
        	break;
        	                	
        default:
        	M4UERR("m4u_get_index_by_port() fail, invalid portID=%d", portID);
    }	
  
    //M4UDBG("get_index_by_port() portID=%d, index=%d \n", portID, m4u_index);
    return m4u_index;    
}

static M4U_MODULE_ID_ENUM m4u_get_module_by_port(M4U_PORT_ID_ENUM portID)
{
    M4U_MODULE_ID_ENUM moduleID = M4U_PORT_DEFECT;
    switch(portID)
    {
        case M4U_PORT_DEFECT       :
        	moduleID = M4U_CLNTMOD_DEFECT;
        	break;
        	
        case M4U_PORT_JPEG_ENC     :
        	moduleID = M4U_CLNTMOD_JPEG_ENC;
        	break;
        	
        case M4U_PORT_VDO_ROT0_OUT0:
        	moduleID = M4U_CLNTMOD_VDO_ROT0;
        	break;
        	        
        case M4U_PORT_RGB_ROT0_OUT0:
        	moduleID = M4U_CLNTMOD_RGB_ROT0;
        	break;
        	
        case M4U_PORT_TV_ROT_OUT0  :
        	moduleID = M4U_CLNTMOD_TVROT;
        	break;
        
        case M4U_PORT_CAM          :
        	moduleID = M4U_CLNTMOD_CAM;
        	break;
        	
        case M4U_PORT_FD0          :
        case M4U_PORT_FD1          :
        case M4U_PORT_FD2          :
        	moduleID = M4U_CLNTMOD_FD;
        	break;
        	
        case M4U_PORT_JPEG_DEC     :
        	moduleID = M4U_CLNTMOD_JPEG_DEC;
        	break;
        	        
        case M4U_PORT_R_DMA0_OUT0  :
        case M4U_PORT_R_DMA0_OUT1  :
        case M4U_PORT_R_DMA0_OUT2  :
         	moduleID = M4U_CLNTMOD_RDMA0;
        	break;
        	       
        case M4U_PORT_PCA          :
        	moduleID = M4U_CLNTMOD_PCA;
        	break;

        case M4U_PORT_OVL_MASK     :
        case M4U_PORT_OVL_DCP      :
        	moduleID = M4U_CLNTMOD_OVL;
        	break;
        	        	
        case M4U_PORT_DPI          :
        	moduleID = M4U_CLNTMOD_DPI;
        	break;
        	
        case M4U_PORT_RGB_ROT1_OUT0:
        	moduleID = M4U_CLNTMOD_RGB_ROT1;
        	break;
        	        	
        case M4U_PORT_VDO_ROT1_OUT0:
        	moduleID = M4U_CLNTMOD_VDO_ROT1;
        	break;
        	        
        case M4U_PORT_RGB_ROT2_OUT0:
        	moduleID = M4U_CLNTMOD_RGB_ROT2;
        	break;
        	        
        case M4U_PORT_TVC          :
        	moduleID = M4U_CLNTMOD_TVC;
        	break;
        	        	
        case M4U_PORT_LCD_R        :
        case M4U_PORT_LCD_W        :
        	moduleID = M4U_CLNTMOD_LCDC;
        	break;
        	        
        case M4U_PORT_R_DMA1_OUT0  :
        case M4U_PORT_R_DMA1_OUT1  :
        case M4U_PORT_R_DMA1_OUT2  :
        	moduleID = M4U_CLNTMOD_RDMA1;
        	break;
        	        	
        case M4U_PORT_SPI          :                   
        	moduleID = M4U_CLNTMOD_SPI;
        	break;
        	
        case M4U_PORT_VENC_MC      :
        case M4U_PORT_VENC_BSDMA   :
        case M4U_PORT_VENC_MVQP    :
        	moduleID = M4U_CLNTMOD_VENC_DMA;
        	break;        	
        	
        case M4U_PORT_VDEC_DMA     :
        case M4U_PORT_VDEC_REC     :
        case M4U_PORT_VDEC_POST0   :
        	moduleID = M4U_CLNTMOD_VDEC_DMA;
        	break;
        	
        case M4U_PORT_G2D_W        :
        case M4U_PORT_G2D_R        :
        	moduleID = M4U_CLNTMOD_G2D;
        	break;
        	        	
        case M4U_PORT_AUDIO        :
        	moduleID = M4U_CLNTMOD_AUDIO;
        	break;
        	                	
        default:
        	M4UERR("m4u_get_module_by_port() fail, invalid portID=%d", portID);
    }	
  
    M4UDBG("m4u_get_module_by_port() portID=%d(%s), moduleID=%d(%s)\n", 
            portID, m4u_get_port_name(portID), moduleID, m4u_get_module_name(moduleID));
            
    return moduleID;    
}

static int m4u_name_init(void)
{
    pmodule_current_size = (int*)kmalloc(M4U_CLIENT_MODULE_NUM*4, GFP_KERNEL|__GFP_ZERO);
    pmodule_max_size = (int*)kmalloc(M4U_CLIENT_MODULE_NUM*4, GFP_KERNEL|__GFP_ZERO);
    pmodule_locked_pages = (int*)kmalloc(M4U_CLIENT_MODULE_NUM*4, GFP_KERNEL|__GFP_ZERO);

    return 0;                       
}

char* m4u_get_module_name(M4U_MODULE_ID_ENUM moduleID)
{
    switch(moduleID)
    {
        case M4U_CLNTMOD_DEFECT:   return "CAM_DEFECT";  
        case M4U_CLNTMOD_CAM:      return "CAM";      
        case M4U_CLNTMOD_PCA:      return "CAM_PCA";      
        case M4U_CLNTMOD_JPEG_DEC: return "JPEG_DEC"; 
        case M4U_CLNTMOD_JPEG_ENC: return "JPEG_ENC"; 
        case M4U_CLNTMOD_ROT_GENERAL:     return "MDP_GENERAL";
        case M4U_CLNTMOD_VDO_ROT0:     return "MDP_VDO_ROT0";     
        case M4U_CLNTMOD_RGB_ROT0:     return "MDP_RGB_ROT0";     
        case M4U_CLNTMOD_TVROT:    return "TVOUT_TVROT";    
        case M4U_CLNTMOD_RDMA_GENERAL:    return "MDP_RDMA_GENERAL";
        case M4U_CLNTMOD_RDMA0:    return "MDP_RDMA0";    
        case M4U_CLNTMOD_FD:       return "CAM_FD";       
        case M4U_CLNTMOD_FD_INPUT: return "CAM_FD_INPUT"; 
        case M4U_CLNTMOD_RGB_ROT1:     return "MDP_RGB_ROT1";     
        case M4U_CLNTMOD_VDO_ROT1:     return "MDP_VDO_ROT1";     
        case M4U_CLNTMOD_RGB_ROT2:     return "MDP_RGB_ROT2";     
        case M4U_CLNTMOD_OVL:      return "MDP_OVL";      
        case M4U_CLNTMOD_LCDC:     return "LCDC";     
        case M4U_CLNTMOD_LCDC_UI:  return "LCDC_UI";  
        case M4U_CLNTMOD_RDMA1:    return "MDP_RDMA1";    
        case M4U_CLNTMOD_TVC:      return "TVOUT_TVC";      
        case M4U_CLNTMOD_SPI:      return "SPI";      
        case M4U_CLNTMOD_DPI:      return "LCD_DPI";      
        case M4U_CLNTMOD_VDEC_DMA: return "VDEC_DMA"; 
        case M4U_CLNTMOD_VENC_DMA: return "VENC_DMA"; 
        case M4U_CLNTMOD_G2D:      return "G2D";      
        case M4U_CLNTMOD_AUDIO:    return "AUDIO";    
        case M4U_CLNTMOD_UNKNOWN: return "UNKNOWN"; 
        default:
             M4UMSG("invalid module id=%d", moduleID);
             return "UN-KNOWN";    	
    	
    }
}
                  
static char* m4u_get_port_name(M4U_PORT_ID_ENUM portID)
{
    unsigned int m4u_index = portID/SEGMENT_SIZE;
    unsigned int port_index = portID%SEGMENT_SIZE;

    if(0==m4u_index)
    {
        switch(port_index)
        {
            case 0: return "DEFECT";      
            case 1: return "JPEG_ENC";    
            case 2: return "MDP_VDO_ROT0";
            case 3: return "MDP_RGB_ROT0";
            case 4: return "TV_ROT_OUT0"; 
            case 5: return "CAM";         
            case 6: return "FD0";         
            case 7: return "FD2";         
            case 8: return "JPEG_DEC";    
            case 9: return "MDP_RDMA0_OUT0";  
            case 10:return "MDP_RDMA0_OUT1";  
            case 11:return "MDP_RDMA0_OUT2";  
            case 12:return "FD1";         
            case 13:return "PCA";         
            default:
            	  M4UMSG("invalid port id=%d", portID);
            	  return "UNKNOWN";
            	   
        }  	
    }
    else if(1==m4u_index)
    {
        switch(port_index)
        {
            case 0: return "OVL_MASK";    
            case 1: return "OVL_DCP";     
            case 2: return "DPI";         
            case 3: return "MDP_RGB_OUT1";
            case 4: return "MDP_VDO_OUT1";
            case 5: return "MDP_RGB_OUT2";
            case 6: return "TVC";         
            case 7: return "LCD_R";       
            case 8: return "LCD_W";       
            case 9: return "MDP_RDMA1_OUT0";  
            case 10:return "MDP_RDMA1_OUT1";  
            case 11:return "MDP_RDMA1_OUT2";  
            case 12:return "SPI";         
            default:
            	  M4UMSG("invalid port id=%d", portID);
            	  return "UNKNOWN";            	    
         } 	
    }
    else if(2==m4u_index)
    {
        switch(port_index)
        {
             case 0: return "VENC_MC";   
             case 1: return "VENC_BSDMA";
             case 2: return "VENC_MVQP"; 
             case 3: return "VDEC_DMA";  
             case 4: return "VDEC_REC";  
             case 5: return "VDEC_POST0";
             default:
             	  M4UMSG("invalid port id=%d", portID); 
             	  return "UNKNOWN";             	   
        } 	
    }
    else if(3==m4u_index)
    {
        switch(port_index)
        {
        	  case 0: return "G2D_W";
            case 1: return "G2D_R";
            case 2: return "AUDIO";
            default:
            	  M4UMSG("invalid port id=%d", portID);
            	  return "UNKNOWN";            	     
        }	
    }
    else
    {
        M4UMSG("invalid port id=%d", portID); 
        return "UNKNOWN";
    }            
}

unsigned int m4u_get_pa_by_mva(unsigned int mva)
{
    unsigned int * pPageTableAddr = 0;
    pPageTableAddr = (unsigned int*)((unsigned int) pPageTableVA_nonCache + M4U_GET_PTE_OFST_TO_PT_SA(mva)); 
    if( (*pPageTableAddr & 0x2) !=0)
    {
        return *pPageTableAddr;
    }
    else
    {
        M4UMSG("error: pa is invalid, mva=0x%x, pa=0x%x \n", mva, *pPageTableAddr);
        return 0;
    }
}

static void m4u_memory_usage(bool bPrintAll)
{
    unsigned int i=0;
    for(i=0;i<M4U_CLIENT_MODULE_NUM;i++)
    {
        M4UMSG("id=%-2d, name=%-10s, max=%-5dKB, current=%-5dKB, locked_page=%-3d \n",
            i, m4u_get_module_name(i), pmodule_max_size[i]/1024, pmodule_current_size[i]/1024, 
            pmodule_locked_pages[i]);
    }    	
}    

#if 0
static void m4u_print_locked_pages(void)
{
	  unsigned int i;
	  
	  M4UMSG("m4u_print_locked_pages() ! \n");
    for(i=0;i<M4U_CLIENT_MODULE_NUM;i++)
    {
        M4UMSG("module=%s, locked_pages=%d \n", m4u_get_module_name(i), pmodule_locked_pages[i]);
    }    	
}  
#endif

static void m4u_print_active_port(unsigned int m4u_index)
{
    unsigned int i=0;
    unsigned int temp;
    if(m4u_index>=TOTAL_M4U_NUM)
    {
    	  M4UERR("m4u_print_active_port, invalid m4u_index=%d \n", m4u_index);
    	  return;
    }
    
    temp = M4U_ReadReg32(gM4UBaseAddr[m4u_index], M4UMACRO0159 );
    M4UMSG("active ports in larb%d(0x%x): ", m4u_index, temp);
    if(0==(temp&0xffff))
    {
    	  printk("no active ports! \n");
    }
    else
    {
        for(i=0;i<g4M4UPortCount[m4u_index];i++)
        {
            if(temp & (1<<i))
            {
                printk("%s(id=%d), ", m4u_get_port_name(m4u_index*SEGMENT_SIZE+i),i);
            }
        }
        printk("\n");
    }
} 

#if 0
int m4u_release_pages_no_va(M4U_MODULE_ID_ENUM eModuleID, unsigned int page_num, unsigned int* pPageTableAddr)
{
    struct page *page;
    unsigned int pa;
    unsigned int i;
    
    M4ULOG(" m4u_release_pages_no_va(), module=%s, page_num=0x%x, pagetableAddr=0x%x \n",
        m4u_get_module_name(eModuleID), page_num, (unsigned int)pPageTableAddr);   
            
    for(i=0;i<page_num;i++)
    {
    	  pa = *(pPageTableAddr+i);
    	  if((pa&0x02)==0)
    	  {
    	      continue;  
    	  }
    	  page = m4u_pfn_to_page(__phys_to_pfn(pa));
    	  //we should check page count before call put_page, because m4u_release_pages() may fail in the middle of buffer
    	  //that is to say, first several pages may be put successfully in m4u_release_pages()
    	  if(page_count(page)>0) 
    	  {
    	      put_page(page);          
    	  }  
    	  pmodule_locked_pages[eModuleID]--;
        *(pPageTableAddr+i) &= ~(0x2);       
    }
    
    return 0;
}
#endif

// used to clear all TLB resource occupied by the module
int m4u_reset_mva_release_tlb(M4U_MODULE_ID_ENUM eModuleID) 
{	
    //todo: implement this func in dynamic alloc mode
    M4ULOG("Have not implemented m4u_reset_mva_release_tlb() in dynamic mva alloc mode! \n");
    
    return 0;
}


int m4u_get_write_mode_by_module(M4U_MODULE_ID_ENUM moduleID)
{
    
    unsigned int write;
    switch(moduleID)  // from user space
    {
        case M4U_CLNTMOD_DEFECT:    // these 8 module has both read and write ports 
        case M4U_CLNTMOD_CAM:       // set as writeable now, 
        case M4U_CLNTMOD_PCA:       // may get write mode by port in the future(need user config port ID)
        case M4U_CLNTMOD_FD:        // or set as readable, then set (get_user_page->force)=true
        case M4U_CLNTMOD_FD_INPUT:        	
        case M4U_CLNTMOD_VDEC_DMA:
        case M4U_CLNTMOD_VENC_DMA:
        case M4U_CLNTMOD_G2D: 
            write = 1;
            break;
                    	        	      	     
        case M4U_CLNTMOD_JPEG_ENC:  
        case M4U_CLNTMOD_ROT_GENERAL:
        case M4U_CLNTMOD_VDO_ROT0:
        case M4U_CLNTMOD_RGB_ROT0:    
        case M4U_CLNTMOD_TVROT:
        case M4U_CLNTMOD_RGB_ROT1:
        case M4U_CLNTMOD_VDO_ROT1:
        case M4U_CLNTMOD_RGB_ROT2:          
        case M4U_CLNTMOD_SPI:    	
            write = 1;
            break;

        case M4U_CLNTMOD_AUDIO: 
        case M4U_CLNTMOD_DPI:
        case M4U_CLNTMOD_OVL:        
        case M4U_CLNTMOD_LCDC:
        case M4U_CLNTMOD_LCDC_UI:
        case M4U_CLNTMOD_RDMA1:
        case M4U_CLNTMOD_TVC:            
        case M4U_CLNTMOD_RDMA_GENERAL:
        case M4U_CLNTMOD_RDMA0:
        case M4U_CLNTMOD_JPEG_DEC:        	
            write = 0;
            break;

        default:
            M4UMSG("error: can not get module's write mode, module=%s \n", m4u_get_module_name(moduleID));
            write = 1;
            break;
    }

    return write;
}


static M4U_MODULE_ID_ENUM m4u_get_module_by_MVA(unsigned int MVA)
{    
    return mva2module(MVA);
}

#if 0
static int m4u_print_range_info(void)
{
    unsigned int i=0;

    M4UMSG(" MVA Range Info: \n");
    for(i=0;i<TOTAL_RANGE_NUM;i++)
    {
        if(1==pRangeDes[i].Enabled)
        {
            M4UMSG("pRangeDes[%d]: Enabled=%d, module=%s, MVAStart=0x%x, MVAEnd=0x%x \n", 
                i, pRangeDes[i].Enabled, m4u_get_module_name(pRangeDes[i].eModuleID), 
                pRangeDes[i].MVAStart, pRangeDes[i].MVAEnd);
        }
    }    
    
    return 0;	
}

#endif

static void m4u_get_performance(unsigned long data)
{
    unsigned int i=0;

    for(i=0;i<TOTAL_M4U_NUM;i++)
    {    
        m4u_monitor_stop(m4u_get_port_by_index(i));   // print performance in last 1 second    
    }
    
    mod_timer(&perf_timer, jiffies+HZ);
    
    for(i=0;i<TOTAL_M4U_NUM;i++)
    {
        m4u_monitor_start(m4u_get_port_by_index(i));  // start to count performance for next 1 second  
    }
  
}

void m4u_get_power_status(void)
{
	  unsigned int i=0;
    M4UMSG("power_total=%d \n", power_total);
    for(i=0;i<M4U_CLIENT_MODULE_NUM;i++)
    {
    	 M4UMSG("power_on_cnt=%d, module=%s\n", power_module_cnt[i], m4u_get_module_name(i));
    }
}

int m4u_perf_timer_on(void)
{
    M4UMSG("m4u_perf_timer_on is called! \n");
    perf_timer.expires = jiffies + HZ; //every 1 second
    perf_timer.data = 0;
    perf_timer.function = m4u_get_performance;
    add_timer(&perf_timer);    	
    
    return 0;
}

int m4u_perf_timer_off(void)
{
    M4UMSG("m4u_perf_timer_off is called! \n"); 
    del_timer(&perf_timer);  
    return 0;  	
}

int m4u_log_on(void)
{
	  unsigned int i=0;
	  
    M4UMSG("m4u_log_on is called! \n");  
    // m4u_print_locked_pages();
    gM4uLogFlag = true;
    m4u_get_power_status();
    m4u_dump_mva_info();
    m4u_dump_mva_global_list_lock();
    m4u_memory_usage(true);
    for(i=0;i<TOTAL_M4U_NUM;i++)
    {
    	// m4u_dump_reg(i);
      m4u_dump_info(i);
      m4u_print_active_port(i);
    }
    
    M4UMSG("m4u pagetable info: \n");
    m4u_mvaGraph_dump();
    

          
    return 0;
}

int m4u_log_off(void)
{
    M4UMSG("m4u_log_off is called! \n");  
    gM4uLogFlag = false;
    return 0;  	
}


int m4u_enable_prefetch(M4U_PORT_ID_ENUM PortID)
{
    unsigned int m4u_base = gM4UBaseAddr[m4u_get_index_by_port(PortID)];
    unsigned int temp = M4U_ReadReg32(m4u_base,M4UMACRO0106);
    M4U_WriteReg32(m4u_base, M4UMACRO0106, temp&(~0x01));

    return 0;
}

int m4u_disable_prefetch(M4U_PORT_ID_ENUM PortID)
{
    unsigned int isCalled0=0;
    unsigned int isCalled1=0;
    unsigned int isCalled2=0;
    unsigned int isCalled3=0;
    unsigned int m4u_base, temp;
    M4U_POW_ON_TRY(m4u_get_module_by_index(0), &isCalled0);
    M4U_POW_ON_TRY(m4u_get_module_by_index(1), &isCalled1);
    M4U_POW_ON_TRY(m4u_get_module_by_index(2), &isCalled2);
    M4U_POW_ON_TRY(m4u_get_module_by_index(3), &isCalled3);

    m4u_base = gM4UBaseAddr[m4u_get_index_by_port(PortID)];
    temp = M4U_ReadReg32(m4u_base,M4UMACRO0106);
    M4U_WriteReg32(m4u_base, M4UMACRO0106, temp|0x01);

    M4U_POW_OFF_TRY(m4u_get_module_by_index(0), isCalled0);
    M4U_POW_OFF_TRY(m4u_get_module_by_index(1), isCalled1);
    M4U_POW_OFF_TRY(m4u_get_module_by_index(2), isCalled2);
    M4U_POW_OFF_TRY(m4u_get_module_by_index(3), isCalled3);
    return 0;
}

int m4u_enable_error_hang(M4U_PORT_ID_ENUM PortID)
{
    unsigned int m4u_base = gM4UBaseAddr[m4u_get_index_by_port(PortID)];
    unsigned int temp = M4U_ReadReg32(m4u_base,M4UMACRO0106);
    M4U_WriteReg32(m4u_base, M4UMACRO0106, temp|(1<<7));

    return 0;
}

int m4u_disable_error_hang(M4U_PORT_ID_ENUM PortID)
{
    unsigned int m4u_base = gM4UBaseAddr[m4u_get_index_by_port(PortID)];
    unsigned int temp = M4U_ReadReg32(m4u_base,M4UMACRO0106);
    M4U_WriteReg32(m4u_base, M4UMACRO0106, temp&(~(1<<7)));

    return 0;
}



//======================================== for MAU IT
// find all active M4U ports' MVA range to check whether the pagetable contain PA inside [start_addr, end_addr]
int m4u_mau_check_pagetable(unsigned int start_addr, unsigned int end_addr)
{
    unsigned int mva;
    int found=0;
    M4UMSG("m4u_check_pagetable, start_addr=0x%x, end_addr=0x%x \n", 
        start_addr, end_addr);

    start_addr &= ~(M4U_PAGE_SIZE-1);
    end_addr &= ~(M4U_PAGE_SIZE-1);

    
    for(mva=0; mva<=M4U_MVA_MAX; mva+=M4U_PAGE_SIZE)
    {
        unsigned int pa = *(unsigned int*)(mva_pteAddr(mva));
        if(pa & 0x2)
        {
            pa &= ~(M4U_PAGE_SIZE-1);
            if((pa>=start_addr) && (pa<=end_addr))
            {
                M4UMSG("pa found in pagetable: pa=0x%x, module=%s\n",pa, m4u_get_module_name(mva2module(mva)));
                found=1;
            }
        }
    }
    
    if(found)
    {
        M4UMSG("m4u_check_pagetable found pa! \n");
    }
    else
    {
        M4UMSG("m4u_check_pagetable cannot found pa\n");
    }

    return 0;	
}

// return port ID that use physical address currently
// parameter: an unsigned int array with 4 element
int m4u_mau_get_physical_port(unsigned int* engineMask)
{
    ASSERT(engineMask!=NULL);
    *(engineMask + 0) = ~(M4U_ReadReg32(gM4UBaseAddr[0], M4UMACRO0159 ));
    *(engineMask + 1) = ~(M4U_ReadReg32(gM4UBaseAddr[1], M4UMACRO0159 )); 
    *(engineMask + 2) = ~(M4U_ReadReg32(gM4UBaseAddr[2], M4UMACRO0159 )); 
    *(engineMask + 3) = ~(M4U_ReadReg32(gM4UBaseAddr[3], M4UMACRO0159 ));  

    return 0;
}

static int __dump_reg_range(unsigned int start, unsigned int end)
{
    int i;
    
    for(i=0; i<=end-start; i+=4)
    {
        printk("0x%x,", ioread32(start+i));
        if((i+4)%32==0)
            printk("\n");
    }
    if(i%32!=0)
        printk("\n");
    return 0;
}

int __dump_memory_range(unsigned int start, unsigned int end)
{
    int i;
    
    for(i=0; i<=end-start; i+=4)
    {
        printk("0x%x,", *(volatile unsigned int *)(start+i));
        if((i+4)%32==0)
            printk("\n");
    }
    if(i%32!=0)
        printk("\n");
    return 0;
}


int m4u_dump_user_addr_register(unsigned int m4u_index)
{
    unsigned int temp;
        
    M4UMSG("dump active modules' addr register: \n");
    if(m4u_index==0)
    {
        temp = M4U_ReadReg32(gM4UBaseAddr[m4u_index], M4UMACRO0159 );
        if(0==(temp&0xffff))
        {
        	  M4UMSG("larb%d: no port use M4U \n", m4u_index);
        }
        else
        {
            M4UMSG("larb%d: \n", m4u_index);
            if(temp&(1<<M4U_PORT_DEFECT))
            {
                M4UMSG("TG RDMA R, CAM_INADDR=0x%x \n", ioread32(CAMINF_BASE+0x0028));
                M4UMSG("Cam WDMA W, CAM_OUTADDR=0x%x \n", ioread32(CAMINF_BASE+0x002C));
                M4UMSG("RawCodec DMA R/W, COM_RESULT_ADDR=0x%x, COM_RESULT_LEN_ADDR=0x%x, COM_BASE_ADDR=0x%x, COM_BASE_LEN_ADDR=0x%x", 
                  ioread32(CAMINF_BASE+0x0644), ioread32(CAMINF_BASE+0x0648), ioread32(CAMINF_BASE+0x064C), ioread32(CAMINF_BASE+0x0650));
            }
            if(temp&(1<<M4U_PORT_JPEG_ENC))
            {
                M4UMSG("JPG_ENC_DST_ADDR0=0x%x \n", ioread32(0xF209B120));
                M4UMSG("Buffer size (JPG_CODEC_BASE+0x10C): 0x%x\n", ioread32(JPG_CODEC_BASE+0x10C));
                M4UMSG("End address (JPG_CODEC_BASE+0x124): 0x%x\n",ioread32(JPG_CODEC_BASE+0x124));
                M4UMSG("Enable status (JPG_CODEC_BASE+0x104): 0x%x\n",ioread32(JPG_CODEC_BASE+0x104));
                
            }
            
            if(temp&(1<<M4U_PORT_VDO_ROT0_OUT0))
            {
                M4UMSG("VDO_ROT0, VDO_ROT0_Y=0x%x, VDO_ROT0_U=0x%x, VDO_ROT0_V=0x%x \n", 
                  ioread32(VDO_ROT0_BASE+0x318), ioread32(VDO_ROT0_BASE+0x320), ioread32(VDO_ROT0_BASE+0x328));
                M4UMSG("Enable status (VDO_ROT0_BASE+0x30): 0x%x\n",ioread32(VDO_ROT0_BASE+0x30));
            }
            if(temp&(1<<M4U_PORT_RGB_ROT0_OUT0))
            {
                M4UMSG("RGB_ROT0, RGB_ROT0_Y=0x%x, RGB_ROT0_U=0x%x, RGB_ROT0_V=0x%x \n", 
                  ioread32(RGB_ROT0_BASE+0x318), ioread32(RGB_ROT0_BASE+0x320), ioread32(RGB_ROT0_BASE+0x328));
                M4UMSG("Enable status (RGB_ROT0_BASE+0x30): 0x%x\n",ioread32(RGB_ROT0_BASE+0x30));
            }   
            if(temp&(1<<M4U_PORT_TV_ROT_OUT0))
            {
                M4UMSG("TV_ROT_Y_DST_STR_ADDR=0x%x \n", ioread32(0xF209F318));
                M4UMSG("TV_ROT_size( (0xF209F330 >> 16)*(0xF209F330 & 0xFFFF)*2 ): 0x%x\n",(ioread32(0xF209F330) >> 16)*(ioread32(0xF209F330)&0xFFFF)*2);
                M4UMSG("TV_ROT enable stauts (0xF209F030:bit[0]): 0x%x\n", ioread32(0xF209F030));
            }
            if(temp&(1<<M4U_PORT_CAM))
            {
                M4UMSG("CAM_FLK_GADDR=0x%x, CAM_FLK_CON(en bit)=0x%x, size=30000 \n", ioread32(CAMINF_BASE+0x00E8), ioread32(CAMINF_BASE+0x00E0));
                M4UMSG("CAM_SDRADDR=0x%x, CAM_SHADING1 (en bit)=0x%x, size=19200+49152 \n", ioread32(CAMINF_BASE+0x021C), ioread32(CAMINF_BASE+0x0214));
            }  
            if(temp&(1<<M4U_PORT_FD0))
            {
                M4UMSG("FDVT_SRC_IMG_BASE_ADDR (R)=0x%x \n", ioread32(FDVT_BASE+0x164)); 
                M4UMSG("FD0 size: 0x%x\n",150*1024);
                M4UMSG("FD enable stauts (FDVT_BASE+0x15C): 0x%x\n", ioread32(FDVT_BASE+0x15C));

            }
            if( (temp&(1<<M4U_PORT_FD2)) ||
                (temp&(1<<M4U_PORT_FD1)) )
            {
                M4UMSG("FDVT_RSCON_ADR(R)=0x%x, FDVT_FD_CON_BASE_ADR(R)=0x%x, FDVT_FD_RLT_BASE_ADR(R,W)=0x%x, \
                FDVT_TC_RLT_BASE_ADR(W)=0x%x, LD_BADR_0(R)=0x%x, LD_BADR_1 (R)=0x%x, LD_BADR_2(R)=0x%x, \
                LD_BADR_3(R)=0x%x, LD_BADR_4(R)=0x%x, LD_BADR_5(R)=0x%x, LD_BADR_6(R)=0x%x, LD_BADR_7(R)=0x%x \n",
                ioread32(FDVT_BASE+0xC ), ioread32(FDVT_BASE+0x80), ioread32(FDVT_BASE+0x9C), 
                ioread32(FDVT_BASE+0xA0), ioread32(FDVT_BASE+0x54), ioread32(FDVT_BASE+0x58), 
                ioread32(FDVT_BASE+0x5C), ioread32(FDVT_BASE+0x60), ioread32(FDVT_BASE+0x64), 
                ioread32(FDVT_BASE+0x68), ioread32(FDVT_BASE+0x6C), ioread32(FDVT_BASE+0x70));
                M4UMSG("FD1/2 size: 0x%x\n",560064);
                M4UMSG("FD enable stauts (FDVT_BASE+0x15C): 0x%x\n", ioread32(FDVT_BASE+0x15C));

            }  
            if(temp&(1<<M4U_PORT_JPEG_DEC))
            {
                M4UMSG("JPG_DEC_FILE_ADDR=0x%x \n", ioread32(0xF209B000));
                M4UMSG("JPG_DEC size(JPG_CODEC_BASE+0x4C): 0x%x\n", ioread32(JPG_CODEC_BASE + 0x4C));
                M4UMSG("JPG_DEC endAddress(JPG_CODEC_BASE+0x00): 0x%x\n", ioread32(JPG_CODEC_BASE + 0x00));
                M4UMSG("JPG_DEC Enable status(JPG_CODEC_BASE+0x40):0x%x\n", ioread32(JPG_CODEC_BASE + 0x40));
                M4UMSG("JPG_DEC other (JPG_CODEC_BASE + 0x78): 0x%x\n", ioread32(JPG_CODEC_BASE + 0x78));
            }
            if(temp&(1<<M4U_PORT_R_DMA0_OUT0))
            {
                M4UMSG("RDMA0_Y=0x%x \n", ioread32(R_DMA0_BASE+0x340));
                M4UMSG("RDMA0 enable status (R_DMA0_BASE+0x30): 0x%x\n", ioread32(R_DMA0_BASE+0x30));

            }  
            if(temp&(1<<M4U_PORT_R_DMA0_OUT1))
            {
                M4UMSG("RDMA0_U=0x%x \n", ioread32(R_DMA0_BASE+0x344));
                M4UMSG("RDMA0 enable status (R_DMA0_BASE+0x30): 0x%x\n", ioread32(R_DMA0_BASE+0x30));
            }
            if(temp&(1<<M4U_PORT_R_DMA0_OUT2))
            {
                M4UMSG("RDMA0_V=0x%x \n", ioread32(R_DMA0_BASE+0x348));
                M4UMSG("RDMA0 enable status (R_DMA0_BASE+0x30): 0x%x\n", ioread32(R_DMA0_BASE+0x30));
            }    
            if(temp&(1<<M4U_PORT_PCA))
            {
                M4UMSG("PCA: STNR_INPUT0 =0x%x, STNR_INPUT1 =0x%x, STNR_INPUT2 =0x%x, \
                STNR_INPUT4 =0x%x, STNR_BASE1  =0x%x, STNR_BASE2  =0x%x, \
                STNR_RESULT1=0x%x, STNR_RESULT2=0x%x \n", 
                ioread32(CAMINF_BASE+0x06C0), ioread32(CAMINF_BASE+0x06C4), ioread32(CAMINF_BASE+0x06C8), 
                ioread32(CAMINF_BASE+0x06D0), ioread32(CAMINF_BASE+0x06B4), ioread32(CAMINF_BASE+0x06B8), 
                ioread32(CAMINF_BASE+0x06D4), ioread32(CAMINF_BASE+0x06D8));
                
            }                                                                     
        }
    }    
    
    if(m4u_index==1)
    {
        temp = M4U_ReadReg32(gM4UBaseAddr[m4u_index], M4UMACRO0159 );
        if(0==(temp&0xffff))
        {
        	  M4UMSG("larb%d: no port use M4U \n", m4u_index);
        }
        else
        {
            M4UMSG("larb%d: \n", m4u_index);
            if(temp&(1<<(M4U_PORT_OVL_MASK%SEGMENT_SIZE)))
            {
                M4UMSG("warning: OVL port dose not enable M4U currently?! \n");
            }
            if(temp&(1<<(M4U_PORT_OVL_DCP%SEGMENT_SIZE)))
            {
                M4UMSG("warning: OVL port dose not enable M4U currently?! \n");
            }            
            if(temp&(1<<(M4U_PORT_DPI%SEGMENT_SIZE)))
            {
                M4UMSG("DPI buf0=0x%x, buf1=0x%x, buf2=0x%x \n", 
                    ioread32(DPI_BASE+0x30), ioread32(DPI_BASE+0x38), ioread32(DPI_BASE+0x40));
                M4UMSG("DPI enable status (0xf208c0a0): 0x%x\n", ioread32(0xf208c0a0));
            }
            if(temp&(1<<(M4U_PORT_RGB_ROT1_OUT0%SEGMENT_SIZE)))
            {
                M4UMSG("RGB_ROT1, RGB_ROT1_Y=0x%x, RGB_ROT1_U=0x%x, RGB_ROT1_V=0x%x \n", 
                  ioread32(RGB_ROT1_BASE+0x318), ioread32(RGB_ROT1_BASE+0x320), ioread32(RGB_ROT1_BASE+0x328));
                M4UMSG("RGB_ROT1 enable status (RGB_ROT1_BASE+0x30): 0x%x\n", ioread32(RGB_ROT1_BASE+0x30));
            }   
            if(temp&(1<<(M4U_PORT_VDO_ROT1_OUT0%SEGMENT_SIZE)))
            {
                M4UMSG("VDO_ROT1, VDO_ROT1_Y=0x%x, VDO_ROT1_U=0x%x, VDO_ROT1_V=0x%x \n", 
                  ioread32(VDO_ROT1_BASE+0x318), ioread32(VDO_ROT1_BASE+0x320), ioread32(VDO_ROT1_BASE+0x328)); 
                M4UMSG("VDO_ROT1 enable status (VDO_ROT1_BASE+0x30): 0x%x\n", ioread32(VDO_ROT1_BASE+0x30));
            }
            if(temp&(1<<(M4U_PORT_RGB_ROT2_OUT0%SEGMENT_SIZE)))
            {
                M4UMSG("RGB_ROT2, RGB_ROT2_Y=0x%x, RGB_ROT2_U=0x%x, RGB_ROT2_V=0x%x \n", 
                  ioread32(RGB_ROT2_BASE+0x318), ioread32(RGB_ROT2_BASE+0x320), ioread32(RGB_ROT2_BASE+0x328));
                M4UMSG("RGB_ROT2 enable status (RGB_ROT2_BASE+0x30): 0x%x\n", ioread32(RGB_ROT2_BASE+0x30));
            }  
            if(temp&(1<<(M4U_PORT_TVC%SEGMENT_SIZE)))
            {
                M4UMSG("TVC: TVC_YADR_TSRC=0x%x, TVC_UADR_TSRC=0x%x, TVC_VADR_TSRC=0x%x \n", 
                  ioread32(0xF209D00C), ioread32(0xF209D010), ioread32(0xF209D014)); 
                M4UMSG("TVC size(0xF209D024*0xF209D028*2): 0x%x\n", ioread32(0xF209D024)*ioread32(0xF209D028)*2);
                M4UMSG("TVC enable status(0xF209D000:[0]): 0x%x\n", ioread32(0xF209D000));
            }
            if(temp&(1<<(M4U_PORT_LCD_R%SEGMENT_SIZE)))
            {
                M4UMSG("LCD_LAYER_0=0x%x, LCD_LAYER_1=0x%x, LCD_LAYER_2=0x%x, LCD_LAYER_3=0x%x, \
                LCD_LAYER_4=0x%x, LCD_LAYER_5=0x%x,  \n",
                ioread32(0xF20A10BC), ioread32(0xF20A10EC), ioread32(0xF20A111C), 
                ioread32(0xF20A114C), ioread32(0xF20A117C), ioread32(0xF20A11AC));
                M4UMSG("LCD_R enable status (0xF20A1080): 0x%x\n", ioread32(0xF20A1080));

            }  
            if(temp&(1<<(M4U_PORT_LCD_W%SEGMENT_SIZE)))
            {
                M4UMSG("LCD_W: lcd_w2m_fb0=0x%x, lcd_w2m_fb1=0x%x, lcd_w2m_fb2=0x%x \n", 
                  ioread32(0xF20A1060), ioread32(0xF20A1064), ioread32(0xF20A1068)); 
                M4UMSG("LCD_R enable status (0xF20A1080): 0x%x\n", ioread32(0xF20A1080));

            }
            if(temp&(1<<(M4U_PORT_R_DMA1_OUT0%SEGMENT_SIZE)))
            {
                M4UMSG("RDMA1_Y=0x%x \n", ioread32(R_DMA1_BASE+0x340));
                M4UMSG("RDMA1 enable status (R_DMA1_BASE+0x30): 0x%x\n", ioread32(R_DMA1_BASE+0x30));
            }  
            if(temp&(1<<(M4U_PORT_R_DMA1_OUT1%SEGMENT_SIZE)))
            {
                M4UMSG("RDMA1_U=0x%x \n", ioread32(R_DMA1_BASE+0x344));
                M4UMSG("RDMA1 enable status (R_DMA1_BASE+0x30): 0x%x\n", ioread32(R_DMA1_BASE+0x30));
            }
            if(temp&(1<<(M4U_PORT_R_DMA1_OUT2%SEGMENT_SIZE)))
            {
                M4UMSG("RDMA1_V=0x%x \n", ioread32(R_DMA1_BASE+0x348));
                M4UMSG("RDMA1 enable status (R_DMA1_BASE+0x30): 0x%x\n", ioread32(R_DMA1_BASE+0x30));
            }    
            if(temp&(1<<(M4U_PORT_SPI%SEGMENT_SIZE)))
            {
                M4UMSG("warning: SPI port dose not enable M4U currently?! \n");
            }                                                                     
        }
    }  
      
    if(m4u_index==2)
    {
        temp = M4U_ReadReg32(gM4UBaseAddr[m4u_index], M4UMACRO0159 );
        if(0==(temp&0xffff))
        {
        	  M4UMSG("larb%d: no port use M4U \n", m4u_index);
        }
        else
        {
            M4UMSG("larb%d: \n", m4u_index);
            if(temp&(1<<(M4U_PORT_VENC_MC %SEGMENT_SIZE)))
            {
                M4UMSG("VENC_MC, REFADDR_Y=0x%x, REFADDR_U=0x%x, REFADDR_V=0x%x \n", 
                  ioread32(VENC_BASE+0x54), ioread32(VENC_BASE+0x58), ioread32(VENC_BASE+0x5c));
                M4UMSG("VENC_MC size: 0x%x\n", 1413632);
                M4UMSG("VENC_MC enable status (VENC_BASE+0x0DC,bit[4:2]==3): 0x%x\n", ioread32(VENC_BASE+0x0DC));
                M4UMSG("VENC_MC other VENC_SIDE_ADDR=0x%x, VENC_MC_CTRL=0x%x\n", ioread32(VENC_BASE+0x38), ioread32(VENC_BASE+0x6C));
            }
            if(temp&(1<<(M4U_PORT_VENC_BSDMA%SEGMENT_SIZE)))
            {
                M4UMSG("VENC_BSDMA, VENC_BITADR=0x%x \n", ioread32(VENC_BASE+0x60));  
                M4UMSG("VENC_BSDMA size: 0x%x\n", 1024*1024);
                M4UMSG("VENC_BSDMA enable status (VENC_BASE+0x0D8, bit[2:0]==4 or 5): 0x%x\n", ioread32(VENC_BASE+0x0D8));
                M4UMSG("VENC_BSDMA other VENC_BITADR=0x%x\n", ioread32(VENC_BASE +0x60));

            }            
            if(temp&(1<<(M4U_PORT_VENC_MVQP%SEGMENT_SIZE)))
            {
                M4UMSG("VENC_MVQP, SIDE_ADDR=0x%x \n", ioread32(VENC_BASE+0x38));  
                M4UMSG("VENC_BSDMA size: 0x%x\n", 28832);
                M4UMSG("VENC_BSDMA enable status (VENC_BASE+0x0E4, bit[2:0]==2): 0x%x\n", ioread32(VENC_BASE+0x0E4));
                M4UMSG("VENC_BSDMA other VENC_SIDE_ADDR=0x%x\n", ioread32(VENC_BASE +0x38));
            }
            if(temp&(1<<(M4U_PORT_VDEC_DMA %SEGMENT_SIZE)))
            {
                M4UMSG("VDEC_DMA, SRCADDR_Y=0x%x, SRCADDR_U=0x%x, SRCADDR_V=0x%x \n", 
                  ioread32(VENC_BASE+0x3c), ioread32(VENC_BASE+0x40), ioread32(VENC_BASE+0x44));
            }
            if(temp&(1<<(M4U_PORT_VDEC_REC%SEGMENT_SIZE)))
            {
                M4UMSG("VDEC_REC, RECADDR_Y=0x%x, RECADDR_U=0x%x, RECADDR_V=0x%x \n", 
                  ioread32(VENC_BASE+0x48), ioread32(VENC_BASE+0x4c), ioread32(VENC_BASE+0x50));           
            }  
            /*
            if(temp&(1<<(M4U_PORT_VDEC_POST0%SEGMENT_SIZE)))
            {
                M4UMSG("dump memory in M4U_PORT_VDEC_POST0: (*(VDEC_IDMA_PAC_ADDR+0) ~ *(VDEC_IDMA_PAC_ADDR+63))\n");
                __dump_memory_range(ioread32(VDEC_BASE+0x84),ioread32(VDEC_BASE+0x84)+63);
            }
            */

            if((temp&(1<<(M4U_PORT_VDEC_DMA %SEGMENT_SIZE)))||
            	 (temp&(1<<(M4U_PORT_VDEC_REC %SEGMENT_SIZE)))||
            	 (temp&(1<<(M4U_PORT_VDEC_POST0%SEGMENT_SIZE))))
            {
                M4UMSG("VDEC_REC, RECADDR_Y=0x%x, RECADDR_U=0x%x, RECADDR_V=0x%x \n", 
                  ioread32(VDEC_BASE+0x30), ioread32(VDEC_BASE+0x34), ioread32(VDEC_BASE+0x38));    
                M4UMSG("VDEC_DB_YADDR=0x%x, VDEC_DB_UADDR=0x%x, VDEC_DB_VADDR=0x%x \n",                 
                  ioread32(VDEC_BASE +0x3C ), ioread32(VDEC_BASE +0x40 ), ioread32(VDEC_BASE +0x44 ));                
                M4UMSG("VDEC_MBIF_ADDR=0x%x, VDEC_IDMA_PAC_ADDR=0x%x, VDEC_POST_DM_MB_BASE=0x%x \n",                 
                  ioread32(VDEC_BASE +0x48 ), ioread32(VDEC_BASE +0x84 ), ioread32(VDEC_BASE +0xFF8 ));
                M4UMSG("VDEC enable status (VDEC_BASE +0x50): 0x%x\n", ioread32(VDEC_BASE +0x50));
                M4UMSG("VDEC enable status 2 (VDEC_BASE +0x8C): 0x%x\n", ioread32(VDEC_BASE +0x8C));
                M4UMSG("VDEC_DMA VDEC_PIC_SIZE=0x%x\n", ioread32(VDEC_BASE+0x18));
                
            } 

            M4UMSG("dump VDEC common registers:===> \n");
            M4UMSG("hw control regs (VDEC_BASE~VDEC_BASE+0xB4)\n");
            __dump_reg_range(VDEC_BASE, VDEC_BASE+0xB4);
            M4UMSG("HW pitch config regs (VENC_BASE+0x10, VENC_BASE+0x14)\n");
            __dump_reg_range(VENC_BASE+0x10, VENC_BASE+0x14);
            M4UMSG("HW post DM registers: (VDEC_BASE+0xff0, VDEC_BASE+0xffc)\n");
            __dump_reg_range(VDEC_BASE+0xff0, VDEC_BASE+0xffc);
            M4UMSG("HW post GR15 registe: (VDEC_BASE+0x43c)\n");
            printk("0x%x\n", ioread32(VDEC_BASE+0x43c));
            
        }
    }      

    if(m4u_index==3)
    {
        temp = M4U_ReadReg32(gM4UBaseAddr[m4u_index], M4UMACRO0159 );
        if(0==(temp&0xffff))
        {
        	  M4UMSG("larb%d: no port use M4U \n", m4u_index);
        }
        else
        {
            M4UMSG("larb%d: \n", m4u_index);
            if(temp&(1<<(M4U_PORT_G2D_W %SEGMENT_SIZE)))
            {
                M4UMSG("G2D_W: W2M_ADDR=0x%x \n", ioread32(0xF20C5044));
                M4UMSG("G2D_W size 0xF20C5000+0x4C=0x%x, 0xF20C5000+0x50=0x%x\n",ioread32(0xF20C5000+0x4C), ioread32(0xF20C5000+0x50));
                M4UMSG("G2D status: 0xF20C5000+0xC=0x%x\n", ioread32(0xF20C5000+0xC));
            }
            if(temp&(1<<(M4U_PORT_G2D_R%SEGMENT_SIZE)))
            {
                M4UMSG("G2D_R: L0_ADDR=0x%x \n", ioread32(0xF20C5084));
                M4UMSG("G2D_R: L1_ADDR=0x%x \n", ioread32(0xF20C50C4));
                M4UMSG("G2D_R size 0xF20C5000+0x90=0x%x\n",ioread32(0xF20C5000+0x90));
                M4UMSG("G2D status: 0xF20C5000+0xC=0x%x\n", ioread32(0xF20C5000+0xC));
            }            
            if(temp&(1<<(M4U_PORT_AUDIO%SEGMENT_SIZE)))
            {
                M4UMSG("warning: ADUIO port dose not enable M4U currently?! \n");
            }                                                                
        }
    }        

    return 0;
}


int m4u_debug_command(unsigned int command)
{

    M4UMSG("m4u_debug_command, command=0x%x \n", command);
    
    switch(command)
    {
    	  case 0:
    	  	M4UMSG("m4u_debug_command 0, crush LCD_UI layer's pagetable! \n");
        
          m4u_dump_pagetable(M4U_CLNTMOD_LCDC_UI);	
    	  	break;
    	  	
    	  case 1:
    	  		print_detail_en = 1;
    	  	break;
    	  
    	  case 2:
    	  	  print_detail_en = 1;
    	  	break;
        
    	  case 5:
                M4UMSG("[M4U VERSION] %s %s\n", __DATE__, __TIME__);
    	  	break;     

          case 7:   //start dynamic profile
                m4u_profile_init();
                M4UMSG("profile initated done\n");
            break;

          case 8:  // get profile report
              {
                  int i;
                  for(i=0;i<TOTAL_M4U_NUM;i++)
                  {
                      m4u_monitor_start(m4u_get_port_by_index(i));  // start to count performance for next 1 second  
                  }
              }
            break;

          case 9: //stop profile and get report
              {
                  int i;
                  for(i=0;i<TOTAL_M4U_NUM;i++)
                  {    
                      m4u_monitor_stop(m4u_get_port_by_index(i));   // print performance in last 1 second    
                  }
              }
            break;

          case 10: 
            M4UMSG("debug 10: disable prefetch\n");
            m4u_disable_prefetch(m4u_get_port_by_index(0));
            m4u_disable_prefetch(m4u_get_port_by_index(1));
            m4u_disable_prefetch(m4u_get_port_by_index(2));
            m4u_disable_prefetch(m4u_get_port_by_index(3));
            break;
          case 11: 
            M4UMSG("debug 11: enable prefetch\n");
            m4u_enable_prefetch(m4u_get_port_by_index(0));
            m4u_enable_prefetch(m4u_get_port_by_index(1));
            m4u_enable_prefetch(m4u_get_port_by_index(2));
            m4u_enable_prefetch(m4u_get_port_by_index(3));
            break;

          case 12: 
            M4UMSG("debug 12: dump mva info\n");
            m4u_dump_mva_info();
            m4u_dump_mva_global_list_lock();
            break;

          case 13: 
            M4UMSG("debug 13: L1 enable cache flush all\n");
            L1_CACHE_SYNC_BY_RANGE_ONLY = 0;
            break;
            
          case 14: 
            M4UMSG("debug 14: L1 cache flush by range only\n");
            L1_CACHE_SYNC_BY_RANGE_ONLY = 1;
            break;
         	  
         case 15:
         	  M4UMSG("debug 15: set level to user \n");
         	  gTestLevel = M4U_TEST_LEVEL_USER;
         	  break;  

         case 16:
         	  M4UMSG("debug 16: set level to eng \n");
         	  gTestLevel = M4U_TEST_LEVEL_ENG;
         	  break;  

         case 17:
         	  M4UMSG("debug 17: set level to stress \n");
         	  gTestLevel = M4U_TEST_LEVEL_STRESS;
         	  break;  

          case 18: 
            M4UMSG("debug 18: dump reg\n");
            m4u_dump_reg(m4u_get_module_by_index(0));
            m4u_dump_reg(m4u_get_module_by_index(1));
            m4u_dump_reg(m4u_get_module_by_index(2));
            // m4u_dump_reg(m4u_get_module_by_index(3));
            break;
          case 19:
              {
                  char* name = "m4u_debug_power_on";
                  enable_clock(MT65XX_PDN_MM_SMI_LARB0, name);
                  enable_clock(MT65XX_PDN_MM_SMI_LARB0_EMI, name);
                  enable_clock(MT65XX_PDN_MM_SMI_LARB1, name);
                  enable_clock(MT65XX_PDN_MM_SMI_LARB1_EMI, name);
                  enable_clock(MT65XX_PDN_MM_SMI_LARB2_ACP_BUS, name);
                  enable_clock(MT65XX_PDN_MM_SMI_LARB2_260MHZ, name);
                  enable_clock(MT65XX_PDN_MM_SMI_LARB2_EMI, name);
                  enable_clock(MT65XX_PDN_MM_SMI_LARB2_ACP_BUS_EMI, name);
                  enable_clock(MT65XX_PDN_MM_SMI_LARB2, name);
                  enable_clock(MT65XX_PDN_MM_SMI_LARB3_FULL, name);
                  enable_clock(MT65XX_PDN_MM_SMI_LARB3_HALF, name);
              }
              break;
          case 20:
              m4u_profile_dump_reg();
              break;
         
    	  default:
    	  	M4UMSG("undefined command! \n");
    }
    
    return 0;	
}

void m4u_print_mva_list(struct file *filep, const char *pMsg)
{
    garbage_node_t *pNode = filep->private_data;
    garbage_list_t *pList;
    struct list_head *pListHead;

    M4UMSG("print mva list [%s] ================================>\n", pMsg);
    mutex_lock(&(pNode->dataMutex));
    list_for_each(pListHead, &(pNode->mvaList))
    {
        pList = container_of(pListHead, garbage_list_t, link);
        M4UMSG("module=%s, va=0x%x, size=0x%x, mva=0x%x, flags=%d, tsk=0x%x, tgid=%d (%d), tsk.state=%d\n", 
            m4u_get_module_name(pList->eModuleId), pList->bufAddr, pList->size, pList->mvaStart, pList->flags, pList->tsk, pList->tgid, pList->tsk->tgid, pList->tsk->state);
    }
    mutex_unlock(&(pNode->dataMutex));

    M4UMSG("print mva list done ==========================>\n");
}

static int m4u_add_to_global_mva_list(garbage_list_t *pList)
{

    mutex_lock(&(mva_global_link.dataMutex));
    list_add(&(pList->global_link), &(mva_global_link.mvaList));
    mutex_unlock(&(mva_global_link.dataMutex));
    
    return 0;	
}

static int m4u_delete_from_global_mva_list(garbage_list_t *pList)
{

    mutex_lock(&(mva_global_link.dataMutex));
    list_del(&(pList->global_link));
    mutex_unlock(&(mva_global_link.dataMutex));
    
    return 0;	
}

int m4u_dump_mva_global_list_nolock()
{

    struct list_head *pListHead;
    garbage_list_t *pList = NULL;

    M4UMSG("dump mva global_list ========>\n");
    M4UMSG("module  va   mva_start  mva_end  size  flag  tgid\n");

    list_for_each(pListHead, &(mva_global_link.mvaList))
    {
        pList = container_of(pListHead, garbage_list_t, global_link);
        M4UMSG("%s 0x%-8x 0x%-8x 0x%-8x %-8d 0x%x %d\n", 
                m4u_get_module_name(pList->eModuleId), pList->bufAddr, pList->mvaStart, 
                pList->mvaStart+pList->size-1, pList->size, pList->flags, pList->tgid);
    }
    
    return 0;	
}
int m4u_dump_mva_global_list_lock()
{
    mutex_lock(&(mva_global_link.dataMutex));
    m4u_dump_mva_global_list_nolock();
    mutex_unlock(&(mva_global_link.dataMutex));
    return 0;
}

static int m4u_add_to_garbage_list(struct file * a_pstFile,
                                          struct task_struct *tsk,
                                          unsigned int mvaStart, 
                                          unsigned int bufSize,
                                          M4U_MODULE_ID_ENUM eModuleID,
                                          unsigned int va,
                                          unsigned int flags)
{
    garbage_list_t *pList = NULL;
    garbage_node_t *pNode = (garbage_node_t*)(a_pstFile->private_data);
    pList = (garbage_list_t*)kmalloc(sizeof(garbage_list_t), GFP_KERNEL);
    if(pList==NULL || pNode==NULL)
    {
        M4UERR("m4u_add_to_garbage_list(), pList=0x%x, pNode=0x%x \n", (unsigned int)pList, (unsigned int)pNode);
        return -1;
    }

    pList->tsk = tsk->group_leader;
    pList->tgid = tsk->tgid;
    pList->mvaStart = mvaStart;
    pList->size = bufSize;
    pList->eModuleId = eModuleID;
    pList->bufAddr = va;
    pList->flags = flags;
    mutex_lock(&(pNode->dataMutex));
    list_add(&(pList->link), &(pNode->mvaList));
    mutex_unlock(&(pNode->dataMutex));

    m4u_add_to_global_mva_list(pList);
    
    return 0;	
}

static int m4u_delete_from_garbage_list(M4U_MOUDLE_STRUCT* p_m4u_module, struct file * a_pstFile, struct task_struct **ptsk)
{
    struct list_head *pListHead;
    garbage_list_t *pList = NULL;
    garbage_node_t *pNode = (garbage_node_t*)(a_pstFile->private_data);
    int ret=0;

    if(pNode==NULL)
    {
        M4UERR("m4u_delete_from_garbage_list(), pNode is NULL! \n");
        return -1;
    }

    mutex_lock(&(pNode->dataMutex));
    list_for_each(pListHead, &(pNode->mvaList))
    {
        pList = container_of(pListHead, garbage_list_t, link);
        if((pList->mvaStart== p_m4u_module->MVAStart))
        {
            if( (pList->tsk->tgid == p_m4u_module->tsk->tgid)
                && (pList->bufAddr== p_m4u_module->BufAddr)
                && (pList->size == p_m4u_module->BufSize)
                && (pList->eModuleId == p_m4u_module->eModuleID) )
            {                    
                *ptsk = pList->tsk;
                list_del(pListHead);
                m4u_delete_from_global_mva_list(pList);
                kfree(pList);
                ret = 0;
                break;
            }
            else
            {
                ret=-1;
            	//M4UMSG("error: input argument isn't valid, can't find the node at garbage list\n");
            }
        }
    }
    if(pListHead == &(pNode->mvaList))
    {
        ret=-1;
        //M4UMSG("error: input argument isn't valid, can't find the node at garbage list\n");
    }
    mutex_unlock(&(pNode->dataMutex));
    
    return ret;	
}



// parameter number can is not decided
M4U_PORT_ID_ENUM port_id = M4U_PORT_UNKNOWN;
unsigned int m4u_get_nearest_port(M4U_PORT_ID_ENUM cur_port,
                                  unsigned int cur_mva,
                                  unsigned int error_mva,
                                  unsigned int reg_list, ...)
{
    va_list arg_ptr;
    unsigned int reg_value = reg_list & (~0xfff);
    unsigned int mva = cur_mva;
    
    va_start(arg_ptr, reg_list);
    while(reg_value<TOTAL_MVA_RANGE)
    {
    	  //M4UMSG("cur_port=%s, error_port=%s, reg_value=0x%x, error_mva=0x%x, cur_mva=0x%x \n", 
    	  //    m4u_get_port_name(cur_port), m4u_get_port_name(port_id), reg_value, error_mva, cur_mva);
    	      
    	  if( error_mva>=reg_value &&
    	  	 (error_mva-reg_value) < (error_mva-cur_mva) )
    	  {
    	  	  mva = reg_value;
    	  	  port_id = cur_port;
    	  }
    	  reg_value = va_arg(arg_ptr, unsigned int) & (~0xfff);
    }
    va_end(arg_ptr);	
    
    //M4UMSG("return mva=0x%x\n", mva);
    return mva;
}

M4U_PORT_ID_ENUM m4u_get_error_port(unsigned int m4u_index, unsigned int mva)
{
    unsigned int cur_mva = TOTAL_MVA_RANGE;
    unsigned int temp;
    
    if(m4u_index==0)
    {
        temp = M4U_ReadReg32(gM4UBaseAddr[m4u_index], M4UMACRO0159 );
        //M4UMSG("m4u_get_error_port(), M4UMACRO0159=0x%x \n", temp);
        if(0==(temp&0xffff))
        {
        	  return port_id;
        }
        else
        {
            if(temp&(1<<M4U_PORT_DEFECT))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_DEFECT, cur_mva, mva, 
                                               ioread32(CAMINF_BASE+0x0028), 
                                               ioread32(CAMINF_BASE+0x002C),
                                               ioread32(CAMINF_BASE+0x0644),
                                               ioread32(CAMINF_BASE+0x0648),
                                               ioread32(CAMINF_BASE+0x064C),
                                               ioread32(CAMINF_BASE+0x0650),TOTAL_MVA_RANGE);
            }
            if(temp&(1<<M4U_PORT_JPEG_ENC))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_JPEG_ENC, cur_mva, mva,
                                               ioread32(0xF209B120),TOTAL_MVA_RANGE);
            }
            
            if(temp&(1<<M4U_PORT_VDO_ROT0_OUT0))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_VDO_ROT0_OUT0, cur_mva, mva, 
                                               ioread32(VDO_ROT0_BASE+0x318), 
                                               ioread32(VDO_ROT0_BASE+0x320), 
                                               ioread32(VDO_ROT0_BASE+0x328),TOTAL_MVA_RANGE);
            }
            if(temp&(1<<M4U_PORT_RGB_ROT0_OUT0))
            {
                //M4UMSG("RGB_ROT0 para: port=0x%x, cm=0x%x, mva=0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n", M4U_PORT_RGB_ROT0_OUT0, cur_mva, mva,  
                //  ioread32(RGB_ROT0_BASE+0x318), ioread32(RGB_ROT0_BASE+0x320), ioread32(RGB_ROT0_BASE+0x328),TOTAL_MVA_RANGE);            	
                cur_mva = m4u_get_nearest_port(M4U_PORT_RGB_ROT0_OUT0, cur_mva, mva,  
                  ioread32(RGB_ROT0_BASE+0x318), ioread32(RGB_ROT0_BASE+0x320), ioread32(RGB_ROT0_BASE+0x328),TOTAL_MVA_RANGE);
            }   
            if(temp&(1<<M4U_PORT_TV_ROT_OUT0))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_TV_ROT_OUT0, cur_mva, mva, ioread32(0xF209F318),TOTAL_MVA_RANGE);                
            }
            if(temp&(1<<M4U_PORT_CAM))
            {

                cur_mva = m4u_get_nearest_port(M4U_PORT_CAM, cur_mva, mva, ioread32(CAMINF_BASE+0x28), ioread32(CAMINF_BASE+0x2C),TOTAL_MVA_RANGE);
            }  
            if(temp&(1<<M4U_PORT_FD0))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_FD0, cur_mva, mva, ioread32(FDVT_BASE+0x164),TOTAL_MVA_RANGE);                
            }
            if( (temp&(1<<M4U_PORT_FD2)) ||
                (temp&(1<<M4U_PORT_FD1)) )
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_FD1, cur_mva, mva, 
                ioread32(FDVT_BASE+0xC ), ioread32(FDVT_BASE+0x80), ioread32(FDVT_BASE+0x9C), 
                ioread32(FDVT_BASE+0xA0), ioread32(FDVT_BASE+0x54), ioread32(FDVT_BASE+0x58), 
                ioread32(FDVT_BASE+0x5C), ioread32(FDVT_BASE+0x60), ioread32(FDVT_BASE+0x64), 
                ioread32(FDVT_BASE+0x68), ioread32(FDVT_BASE+0x6C), ioread32(FDVT_BASE+0x70),TOTAL_MVA_RANGE);
            }  
            if(temp&(1<<M4U_PORT_JPEG_DEC))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_JPEG_DEC, cur_mva, mva,  ioread32(0xF209B000),TOTAL_MVA_RANGE);                
            }
            if(temp&(1<<M4U_PORT_R_DMA0_OUT0))
            {
                // M4UMSG("check RDMA0! \n");
                cur_mva = m4u_get_nearest_port(M4U_PORT_R_DMA0_OUT0, cur_mva, mva,  ioread32(R_DMA0_BASE+0x340),TOTAL_MVA_RANGE);
            }  
            if(temp&(1<<M4U_PORT_R_DMA0_OUT1))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_R_DMA0_OUT1, cur_mva, mva,  ioread32(R_DMA0_BASE+0x344),TOTAL_MVA_RANGE);
            }
            if(temp&(1<<M4U_PORT_R_DMA0_OUT2))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_R_DMA0_OUT2, cur_mva, mva,  ioread32(R_DMA0_BASE+0x348),TOTAL_MVA_RANGE);
            }    
            if(temp&(1<<M4U_PORT_PCA))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_PCA, cur_mva, mva,  
                ioread32(CAMINF_BASE+0x06C0), ioread32(CAMINF_BASE+0x06C4), ioread32(CAMINF_BASE+0x06C8), 
                ioread32(CAMINF_BASE+0x06D0), ioread32(CAMINF_BASE+0x06B4), ioread32(CAMINF_BASE+0x06B8), 
                ioread32(CAMINF_BASE+0x06D4), ioread32(CAMINF_BASE+0x06D8),TOTAL_MVA_RANGE);
                
            }                                                                     
        }
    }    
    
    if(m4u_index==1)
    {
        temp = M4U_ReadReg32(gM4UBaseAddr[m4u_index], M4UMACRO0159 );
        if(0==(temp&0xffff))
        {
        	  return port_id;
        }
        else
        {
            if(temp&(1<<(M4U_PORT_OVL_MASK%SEGMENT_SIZE)))
            {
                M4UMSG("warning: OVL port dose not enable M4U currently?! \n");
            }
            if(temp&(1<<(M4U_PORT_OVL_DCP%SEGMENT_SIZE)))
            {
                M4UMSG("warning: OVL port dose not enable M4U currently?! \n");
            }            
            if(temp&(1<<(M4U_PORT_DPI%SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_DPI, cur_mva, mva, ioread32(R_DMA1_BASE+0x344),TOTAL_MVA_RANGE);
            }
            if(temp&(1<<(M4U_PORT_RGB_ROT1_OUT0%SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_RGB_ROT1_OUT0, cur_mva, mva,  
                  ioread32(RGB_ROT1_BASE+0x318), ioread32(RGB_ROT1_BASE+0x320), ioread32(RGB_ROT1_BASE+0x328),TOTAL_MVA_RANGE);
            }   
            if(temp&(1<<(M4U_PORT_VDO_ROT1_OUT0%SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_VDO_ROT1_OUT0, cur_mva, mva, 
                  ioread32(VDO_ROT1_BASE+0x318), ioread32(VDO_ROT1_BASE+0x320), ioread32(VDO_ROT1_BASE+0x328),TOTAL_MVA_RANGE);             
            }
            if(temp&(1<<(M4U_PORT_RGB_ROT2_OUT0%SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_RGB_ROT2_OUT0, cur_mva, mva,  
                  ioread32(RGB_ROT2_BASE+0x318), ioread32(RGB_ROT2_BASE+0x320), ioread32(RGB_ROT2_BASE+0x328),TOTAL_MVA_RANGE);
            }  
            if(temp&(1<<(M4U_PORT_TVC%SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_TVC, cur_mva, mva, 
                  ioread32(0xF209D00C), ioread32(0xF209D010), ioread32(0xF209D014),TOTAL_MVA_RANGE);                
            }
            if(temp&(1<<(M4U_PORT_LCD_R%SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_LCD_R, cur_mva, mva, 
                ioread32(0xF20A10BC), ioread32(0xF20A10EC), ioread32(0xF20A111C), 
                ioread32(0xF20A114C), ioread32(0xF20A117C), ioread32(0xF20A11AC),TOTAL_MVA_RANGE);
            }  
            if(temp&(1<<(M4U_PORT_LCD_W%SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_LCD_R, cur_mva, mva, 
                ioread32(0xF20A1060), ioread32(0xF20A1064), ioread32(0xF20A1068), TOTAL_MVA_RANGE);
            }
            if(temp&(1<<(M4U_PORT_R_DMA1_OUT0%SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_R_DMA1_OUT0, cur_mva, mva, ioread32(R_DMA1_BASE+0x340),TOTAL_MVA_RANGE);
            }  
            if(temp&(1<<(M4U_PORT_R_DMA1_OUT1%SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_R_DMA1_OUT1, cur_mva, mva, ioread32(R_DMA1_BASE+0x344),TOTAL_MVA_RANGE);
            }
            if(temp&(1<<(M4U_PORT_R_DMA1_OUT2%SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_R_DMA1_OUT2, cur_mva, mva, ioread32(R_DMA1_BASE+0x348),TOTAL_MVA_RANGE);
            }    
            if(temp&(1<<(M4U_PORT_SPI%SEGMENT_SIZE)))
            {
                M4UMSG("warning: SPI port dose not enable M4U currently?! \n");
            }                                                                     
        }
    }  
      
    if(m4u_index==2)
    {
        temp = M4U_ReadReg32(gM4UBaseAddr[m4u_index], M4UMACRO0159 );
        if(0==(temp&0xffff))
        {
        	  return port_id;
        }
        else
        {
            if(temp&(1<<(M4U_PORT_VENC_MC %SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_VENC_MC, cur_mva, mva,  
                  ioread32(VENC_BASE+0x54), ioread32(VENC_BASE+0x58), ioread32(VENC_BASE+0x5c),TOTAL_MVA_RANGE);            
            }
            if(temp&(1<<(M4U_PORT_VENC_BSDMA%SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_VENC_BSDMA, cur_mva, mva, ioread32(VENC_BASE+0x60),TOTAL_MVA_RANGE);                 
            }            
            if(temp&(1<<(M4U_PORT_VENC_MVQP%SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_VENC_MVQP, cur_mva, mva, ioread32(VENC_BASE+0x38),TOTAL_MVA_RANGE);  
            }
            if(temp&(1<<(M4U_PORT_VDEC_DMA %SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_VDEC_DMA, cur_mva, mva, 
                  ioread32(VENC_BASE+0x3c), ioread32(VENC_BASE+0x40), ioread32(VENC_BASE+0x44),TOTAL_MVA_RANGE);                  
            }
            if(temp&(1<<(M4U_PORT_VDEC_REC%SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_VDEC_REC, cur_mva, mva, 
                  ioread32(VENC_BASE+0x48), ioread32(VENC_BASE+0x4c), ioread32(VENC_BASE+0x50),TOTAL_MVA_RANGE);           
            }            
            if((temp&(1<<(M4U_PORT_VDEC_DMA %SEGMENT_SIZE)))||
            	 (temp&(1<<(M4U_PORT_VDEC_REC %SEGMENT_SIZE)))||
            	 (temp&(1<<(M4U_PORT_VDEC_POST0%SEGMENT_SIZE))))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_VDEC_DMA, cur_mva, mva, 
                  ioread32(VDEC_BASE+0x30), ioread32(VDEC_BASE+0x34), ioread32(VDEC_BASE+0x38),TOTAL_MVA_RANGE);    
                cur_mva = m4u_get_nearest_port(M4U_PORT_VDEC_REC, cur_mva, mva,              
                  ioread32(VDEC_BASE +0x3C ), ioread32(VDEC_BASE +0x40 ), ioread32(VDEC_BASE +0x44 ),TOTAL_MVA_RANGE);                
                cur_mva = m4u_get_nearest_port(M4U_PORT_VDEC_POST0, cur_mva, mva,                 
                  ioread32(VDEC_BASE +0x48 ), ioread32(VDEC_BASE +0x84 ), ioread32(VDEC_BASE +0xFF8 ),TOTAL_MVA_RANGE);  
            }                                                                 
        }
    }      

    if(m4u_index==3)
    {
        temp = M4U_ReadReg32(gM4UBaseAddr[m4u_index], M4UMACRO0159 );
        if(0==(temp&0xffff))
        {
        	  return port_id;
        }
        else
        {
            if(temp&(1<<(M4U_PORT_G2D_W %SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_G2D_W, cur_mva, mva, ioread32(0xF20C5044),TOTAL_MVA_RANGE);
            }
            if(temp&(1<<(M4U_PORT_G2D_R%SEGMENT_SIZE)))
            {
                cur_mva = m4u_get_nearest_port(M4U_PORT_G2D_R, cur_mva, mva, ioread32(0xF20C5084),TOTAL_MVA_RANGE);
                cur_mva = m4u_get_nearest_port(M4U_PORT_G2D_R, cur_mva, mva, ioread32(0xF20C50C4),TOTAL_MVA_RANGE);
            }            
            if(temp&(1<<(M4U_PORT_AUDIO%SEGMENT_SIZE)))
            {
                M4UMSG("warning: ADUIO port dose not enable M4U currently?! \n");
            }                                                                
        }
    }      
    
    return port_id; 
    	
}
                      
module_init(MTK_M4U_Init);
module_exit(MTK_M4U_Exit);
                      

MODULE_DESCRIPTION("MTK M4U driver");
MODULE_AUTHOR("MTK80347 <Xiang.Xu@mediatek.com>");
MODULE_LICENSE("GPL");
//MODULE_LICENSE("Proprietary");
