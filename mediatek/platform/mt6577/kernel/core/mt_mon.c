#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/trace_seq.h>
#include <linux/ftrace_event.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>


#include "asm/hardware/cache-l2x0.h"
#include "mach/mt_reg_base.h"
#include "mach/sync_write.h"
#include "mach/mt_emi_bm.h"
//#include "mach/mt6573_pll.h"
#include "mach/mt_mon.h"
#include "mach/mt_dcm.h"
#include <../../kernel/kernel/trace/trace.h>
 
extern void armV7_perf_mon_enable(unsigned int enable);
extern unsigned int armV7_perf_mon_number(void);
extern void armV7_perf_mon_reset(void);
extern unsigned int armV7_perf_mon_get_cyc_cnt(void);
extern void armV7_perf_mon_set_cyc_cnt_divider(unsigned int n);
extern unsigned int armV7_perf_mon_get_cyc_cnt_divider(void);
extern void armV7_perf_mon_select_event(unsigned int i, unsigned t);
extern unsigned int armV7_perf_mon_get_event_counter(unsigned int i);
extern unsigned int armV7_perf_mon_is_overflow(unsigned int n);

enum {
    L2C_EVT_CO = 1, L2C_EVT_DRHIT = 2, L2C_EVT_DRREQ = 3, L2C_EVT_DWHIT = 4,
    L2C_EVT_DWREQ = 5, L2C_EVT_DWTREQ = 6, L2C_EVT_IRHIT = 7, L2C_EVT_IRREQ = 8,
    L2C_EVT_WA = 9, L2C_EVT_IPFALLOC = 10, L2C_EVT_EPFHIT = 11, 
    L2C_EVT_EPFALLOC = 12, L2C_EVT_SRRCVD = 13, L2C_EVT_SRCONF = 14,
    L2C_EVT_EPFRCVD = 15
};

#define DEF_L2C_CNT0_EVT    (4)
#define DEF_L2C_CNT1_EVT    (5)
#define DEF_BM_RW_TYPE      (BM_BOTH_READ_WRITE)

static unsigned int l2c_cnt0_evt = DEF_L2C_CNT0_EVT;
static unsigned int l2c_cnt1_evt = DEF_L2C_CNT1_EVT;

static unsigned int bm_master_evt = BM_MASTER_AP_MCU;
static unsigned int bm_rw_type_evt = DEF_BM_RW_TYPE;

static MonitorMode register_mode = MODE_FREE;
static unsigned long mon_period_evt;
static unsigned int mon_manual_evt;

struct mt_mon_log *mt_mon_log_buff; //this buffer is allocated for MODE_MANUAL_USER & MODE_MANUAL_KERNEL only
unsigned int mt_mon_log_buff_index;
unsigned int mt6577_kernel_ring_buff_index;

struct arm_pmu *p_pmu;
struct mtk_monitor mtk_mon;

// For avoiding imprecise external abort [L2 Slave Error]
// In order to use l2x0_lock, the lock must be exported from arch/arm/mm/cache-l2x0.c
extern raw_spinlock_t l2x0_lock;

/*
 * mt65xx_mon_init: Initialize the monitor.
 * Return 0.
 */
int mt65xx_mon_init(void)
{
//    extern void dcm_disable_all(void);

    BM_Init();

    // disable system DCM
    dcm_disable_all();

    return 0;
}

/*
 * mt65xx_mon_deinit: De-initialize the monitor.
 * Return 0.
 */
int mt65xx_mon_deinit(void)
{
  //  extern void dcm_enable_all(void);

    BM_DeInit();
    
/*
    if (mt_mon_log_buff) {
        vfree(mt_mon_log_buff);

        mt_mon_log_buff = 0;
    }
*/

    // enable system DCM
    dcm_enable_all();

    return 0;
}

/*
 * mt65xx_mon_enable: Enable hardware monitors.
 * Return 0.
 */
int mt65xx_mon_enable(void)
{
    unsigned long flags;
    p_pmu->reset();

    // enable & start ARM performance monitors
    p_pmu->enable();
    p_pmu->start();

    // reset and enable L2C event counters
    raw_spin_lock_irqsave(&l2x0_lock, flags); 
    __raw_writel(7, PL310_BASE + L2X0_EVENT_CNT_CTRL);
    dsb();
    raw_spin_unlock_irqrestore(&l2x0_lock, flags);

    // stopping EMI monitors will reset all counters
    BM_Enable(0);

    // start EMI monitor counting
    BM_Enable(1);
    
    return 0;
}

/*
 * mt65xx_mon_disable: Disable hardware monitors.
 * Return 0.
 */
int mt65xx_mon_disable(void)
{
    unsigned long flags;
    // disable ARM performance monitors
    p_pmu->stop();
    
    // disable all L2C event counters
    raw_spin_lock_irqsave(&l2x0_lock, flags);
    __raw_writel(0, PL310_BASE + L2X0_EVENT_CNT_CTRL);
    dsb();
    raw_spin_unlock_irqrestore(&l2x0_lock, flags);

    BM_Pause();
    
    return 0;
}

static inline void set_bitmap(unsigned int bit, volatile unsigned long *p)
{
    unsigned long mask = 1UL << (bit & 31);
    
    p += bit >> 5;
    
    *p |= mask;
}

static inline void clear_bitmap(unsigned int bit, volatile unsigned long *p)
{
    unsigned long mask = 1UL << (bit & 31);
    
    p += bit >> 5;
    
    *p &= ~mask;
}

static inline int test_bitmap(unsigned int bit, volatile unsigned long *p)
{
    unsigned int res;
    unsigned long mask = 1UL << (bit & 31);
    
    p += bit >> 5;
    res = *p;
    
    return (res & mask) != 0;
}
/*
 * mt65xx_mon_log: Get the current log from hardware monitors.
 * Return a index to the curret log entry in the log buffer.
 */
unsigned int mt65xx_mon_log(void* log_buff)
{
    struct mt_mon_log* mon_buff;
    unsigned int cur;
    struct pmu_data *pmu_data =  & p_pmu->perf_data;
 
    p_pmu->read_counter();
    
    if( register_mode == MODE_MANUAL_USER || register_mode == MODE_MANUAL_KERNEL){
        cur = mt_mon_log_buff_index++;
        mon_buff = &mt_mon_log_buff[cur];
        mt_mon_log_buff_index %= MON_LOG_BUFF_LEN;
    } else {
        cur = mt6577_kernel_ring_buff_index++;
        mon_buff = (struct mt_mon_log*)log_buff;
    }
    
            
    if (mon_buff) {
        mon_buff->cpu_cyc = pmu_data->cnt_val[0][6];
        mon_buff->cpu_cnt0 = pmu_data->cnt_val[0][0];
        mon_buff->cpu_cnt1 = pmu_data->cnt_val[0][1];
        mon_buff->cpu_cnt2 = pmu_data->cnt_val[0][2];
        mon_buff->cpu_cnt3 = pmu_data->cnt_val[0][3];
        mon_buff->cpu_cnt4 = pmu_data->cnt_val[0][4];
        mon_buff->cpu_cnt5 = pmu_data->cnt_val[0][5];
            
#ifdef CONFIG_SMP        
        mon_buff->cpu1_cyc = pmu_data->cnt_val[1][6];
        mon_buff->cpu1_cnt0 = pmu_data->cnt_val[1][0];
        mon_buff->cpu1_cnt1 = pmu_data->cnt_val[1][1];
        mon_buff->cpu1_cnt2 = pmu_data->cnt_val[1][2];
        mon_buff->cpu1_cnt3 = pmu_data->cnt_val[1][3];
        mon_buff->cpu1_cnt4 = pmu_data->cnt_val[1][4];
        mon_buff->cpu1_cnt5 = pmu_data->cnt_val[1][5];
#endif        

        mon_buff->l2c_cnt0 = readl(PL310_BASE + L2X0_EVENT_CNT0_VAL);
        mon_buff->l2c_cnt1 = readl(PL310_BASE + L2X0_EVENT_CNT1_VAL);

        mon_buff->BM_BCNT = BM_GetBusCycCount();
        mon_buff->BM_TACT = BM_GetTransAllCount();
        mon_buff->BM_TSCT = BM_GetTransCount(1);
        mon_buff->BM_WACT = BM_GetWordAllCount();
        mon_buff->BM_WSCT = BM_GetWordCount(1);
        mon_buff->BM_BACT = BM_GetBandwidthWordCount();
        mon_buff->BM_BSCT = BM_GetOverheadWordCount();
        mon_buff->BM_TSCT2 = BM_GetTransCount(2);
        mon_buff->BM_WSCT2 = BM_GetWordCount(2);
        mon_buff->BM_TSCT3 = BM_GetTransCount(3);
        mon_buff->BM_WSCT3 = BM_GetWordCount(3);
        mon_buff->BM_WSCT4 = BM_GetWordCount(4);
    
        mon_buff->BM_TPCT1 = BM_GetTransTypeCount(1);

        mon_buff->DRAMC_PageHit = DRAMC_GetPageHitCount(DRAMC_ALL);
        mon_buff->DRAMC_PageMiss = DRAMC_GetPageMissCount(DRAMC_ALL);
        mon_buff->DRAMC_Interbank = DRAMC_GetInterbankCount(DRAMC_ALL);
        mon_buff->DRAMC_Idle = DRAMC_GetIdleCount();        
    }
    
    memset(pmu_data->cnt_val[0], 0, sizeof(struct pmu_data));
    return cur;
}

enum print_line_t mt65xx_mon_print_entry(struct mt65xx_mon_entry *entry, struct trace_iterator *iter){
    struct trace_seq *s = &iter->seq;
    int cpu = entry->cpu;
    struct mt_mon_log *log_entry;
    unsigned int log = 0;
    MonitorMode mon_mode_evt = get_mt65xx_mon_mode();

    if(entry == NULL)
        return TRACE_TYPE_HANDLED;
    else{
        log_entry = &entry->field;
        log = entry->log;
    }

#if 0
    if(cpu)
        trace_seq_printf(s, "cpu1 ");
    else
        trace_seq_printf(s, "cpu0 ");
#endif
    if (log == 0) {
        trace_seq_printf(s, "MON_LOG_BUFF_LEN = %d, ", MON_LOG_BUFF_LEN);
        trace_seq_printf(s, "EMI_CLOCK = %d, ", mt6577_get_bus_freq());
    }
    
    if(!cpu || (mon_mode_evt != MODE_SCHED_SWITCH)){
	    trace_seq_printf(
			s,
			"cpu0_cyc = %d, cpu0_cnt0 = %d, cpu0_cnt1 = %d, cpu0_cnt2 = %d, cpu0_cnt3 = %d, cpu0_cnt4 = %d, cpu0_cnt5 = %d, ",
			log_entry->cpu_cyc,
			log_entry->cpu_cnt0,
			log_entry->cpu_cnt1,
			log_entry->cpu_cnt2,
			log_entry->cpu_cnt3,
			log_entry->cpu_cnt4,
			log_entry->cpu_cnt5); 
    }
#ifdef CONFIG_SMP		
		if( cpu || (mon_mode_evt != MODE_SCHED_SWITCH) ) {
			trace_seq_printf(
				s,
				"cpu1_cyc = %d, cpu1_cnt0 = %d, cpu1_cnt1 = %d, cpu1_cnt2 = %d, cpu1_cnt3 = %d, cpu1_cnt4 = %d, cpu1_cnt5 = %d, ",
				log_entry->cpu1_cyc,
				log_entry->cpu1_cnt0,
				log_entry->cpu1_cnt1,
				log_entry->cpu1_cnt2,
				log_entry->cpu1_cnt3,
				log_entry->cpu1_cnt4,
				log_entry->cpu1_cnt5);
		}
#endif

        trace_seq_printf(
            s,
            "l2c_cnt0 = %d, l2c_cnt1 = %d, ",
            log_entry->l2c_cnt0,
            log_entry->l2c_cnt1);

        trace_seq_printf(
            s,
            "BM_BCNT = %d, BM_TACT = %d, BM_TSCT = %d, ",
            log_entry->BM_BCNT,
            log_entry->BM_TACT,
            log_entry->BM_TSCT);

        trace_seq_printf(
            s,
            "BM_WACT = %d, BM_WSCT = %d, BM_BACT = %d, ",
            log_entry->BM_WACT,
            log_entry->BM_WSCT,
            log_entry->BM_BACT);

        trace_seq_printf(
            s,
            "BM_BSCT = %d, ",
            log_entry->BM_BSCT);

        trace_seq_printf(
            s,
            "BM_TSCT2 = %d, BM_WSCT2 = %d, ",
            log_entry->BM_TSCT2,
            log_entry->BM_WSCT2);

        trace_seq_printf(
            s,
            "BM_TSCT3 = %d, BM_WSCT3 = %d, ",
            log_entry->BM_TSCT3,
            log_entry->BM_WSCT3);

        trace_seq_printf(
            s,
            "BM_WSCT4 = %d, BM_TPCT1 = %d, ",
            log_entry->BM_WSCT4,
            log_entry->BM_TPCT1);

        trace_seq_printf(
            s,
            "DRAMC_PageHit = %d, DRAMC_PageMiss = %d, DRAMC_Interbank = %d, DRAMC_Idle = %d\n",
            log_entry->DRAMC_PageHit,
            log_entry->DRAMC_PageMiss,
            log_entry->DRAMC_Interbank,
            log_entry->DRAMC_Idle);
    return TRACE_TYPE_HANDLED;
}

void mt65xx_mon_set_pmu(struct pmu_cfg *p_cfg)
{
    memcpy(&p_pmu->perf_cfg, p_cfg, sizeof(struct pmu_cfg));
}

void mt65xx_mon_get_pmu(struct pmu_cfg *p_cfg)
{
    memcpy(p_cfg, &p_pmu->perf_cfg, sizeof(struct pmu_cfg));
}

void mt65xx_mon_set_l2c(struct l2c_cfg *l_cfg)
{
    unsigned long flags;
    l2c_cnt0_evt = l_cfg->l2c_evt[0];
    l2c_cnt1_evt = l_cfg->l2c_evt[1];

    raw_spin_lock_irqsave(&l2x0_lock, flags);	
    __raw_writel(l2c_cnt0_evt << 2, PL310_BASE + L2X0_EVENT_CNT0_CFG);
    __raw_writel(l2c_cnt1_evt << 2, PL310_BASE + L2X0_EVENT_CNT1_CFG);
    dsb();
    raw_spin_unlock_irqrestore(&l2x0_lock, flags);	
}

void mt65xx_mon_get_l2c(struct l2c_cfg *l_cfg)
{
    l_cfg->l2c_evt[0] = l2c_cnt0_evt;
    l_cfg->l2c_evt[1] = l2c_cnt1_evt;
}

void mt65xx_mon_set_bm_rw(int type)
{
    
    if(type > BM_WRITE_ONLY) {
        printk("invalid event\n");
    } else {
        BM_SetReadWriteType(type);
    }
   
}

#define DEFINE_L2C_CNT_EVT_SHOW(_N) \
static ssize_t l2c_cnt##_N##_evt_show(struct device_driver *driver, char *buf)   \
{   \
    return snprintf(buf, PAGE_SIZE, "counter event = %d\n", l2c_cnt##_N##_evt); \
}

#define DEFINE_L2C_CNT_EVT_STORE(_N)    \
ssize_t l2c_cnt##_N##_evt_store(struct device_driver *driver, const char *buf, size_t count) \
{   \
    unsigned long flags;\
    if (!strncmp(buf, "CO", strlen("CO"))) {   \
        l2c_cnt##_N##_evt = L2C_EVT_CO; \
    } else if (!strncmp(buf, "DRHIT", strlen("DRHIT"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_DRHIT; \
    } else if (!strncmp(buf, "DRREQ", strlen("DRREQ"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_DRREQ; \
    } else if (!strncmp(buf, "DWHIT", strlen("DWHIT"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_DWHIT; \
    } else if (!strncmp(buf, "HWREQ", strlen("HWREQ"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_DWREQ; \
    } else if (!strncmp(buf, "HWTREQ", strlen("HWTREQ"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_DWTREQ; \
    } else if (!strncmp(buf, "IRHIT", strlen("IRHIT"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_IRHIT; \
    } else if (!strncmp(buf, "IRREQ", strlen("IRREQ"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_IRREQ; \
    } else if (!strncmp(buf, "WA", strlen("WA"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_WA; \
    } else if (!strncmp(buf, "IPFALLOC", strlen("IPFALLOC"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_IPFALLOC; \
    } else if (!strncmp(buf, "EPFHIT", strlen("EPFHIT"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_EPFHIT; \
    } else if (!strncmp(buf, "EPFALLOC", strlen("EPFALLOC"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_EPFALLOC; \
    } else if (!strncmp(buf, "SRRCVD", strlen("SRRCVD"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_SRRCVD; \
    } else if (!strncmp(buf, "SRCONF", strlen("SRCONF"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_SRCONF; \
    } else if (!strncmp(buf, "EPFRCVD", strlen("EPFRCVD"))) { \
        l2c_cnt##_N##_evt = L2C_EVT_EPFRCVD; \
    } else {    \
        printk("invalid event\n");  \
        return count;   \
    }   \
    raw_spin_lock_irqsave(&l2x0_lock, flags);\
    __raw_writel(l2c_cnt##_N##_evt << 2, PL310_BASE + L2X0_EVENT_CNT##_N##_CFG);    \
    dsb();\
    raw_spin_unlock_irqrestore(&l2x0_lock, flags);\
    return count;   \
}

DEFINE_L2C_CNT_EVT_SHOW(0)
DEFINE_L2C_CNT_EVT_SHOW(1)
DEFINE_L2C_CNT_EVT_STORE(0)
DEFINE_L2C_CNT_EVT_STORE(1)

static ssize_t bm_master_evt_show(struct device_driver *driver, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "EMI bus monitor master = %d\n", bm_master_evt);
}

ssize_t bm_master_evt_store(struct device_driver *driver, const char *buf, size_t count)
{
    if (!strncmp(buf, "MM", strlen("MM"))) {
        bm_master_evt = BM_MASTER_MULTIMEDIA;
    }else if (!strncmp(buf, "APMCU", strlen("APMCU"))) {
        bm_master_evt = BM_MASTER_AP_MCU;
    }else if (!strncmp(buf, "MDDSP", strlen("MDDSP"))) {
        bm_master_evt = BM_MASTER_MD_DSP;
    }else if (!strncmp(buf, "MDMCU", strlen("MDMCU"))) {
        bm_master_evt = BM_MASTER_MD_MCU;
    }else if (!strncmp(buf, "2G_3G_MDDMA", strlen("2G_3G_MDDMA"))) {
        bm_master_evt = BM_MASTER_2G_3G_MDDMA;
    }else if (!strncmp(buf, "MD_ALL", strlen("MD_ALL"))) {
        bm_master_evt = BM_MASTER_MD_DSP | BM_MASTER_MD_MCU | BM_MASTER_2G_3G_MDDMA;   
    }else if (!strncmp(buf, "PERI", strlen("PERI"))) {
        bm_master_evt = BM_MASTER_PERI;
    }else if (!strncmp(buf, "ALL", strlen("ALL"))) {
        bm_master_evt = BM_MASTER_ALL;    
    }else {
        printk("invalid event\n");

        return count;
    }

    BM_SetMaster(1, bm_master_evt);

    return count;
}

static ssize_t bm_rw_type_evt_show(struct device_driver *driver, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "EMI bus read write type = %d\n", bm_rw_type_evt);
}

ssize_t bm_rw_type_evt_store(struct device_driver *driver, const char *buf, size_t count)
{
    if (!strncmp(buf, "RW", strlen("RW"))) {
        bm_rw_type_evt = BM_BOTH_READ_WRITE;
    } else if (!strncmp(buf, "RO", strlen("RO"))) {
        bm_rw_type_evt = BM_READ_ONLY;
    } else if (!strncmp(buf, "WO", strlen("WO"))) {
        bm_rw_type_evt = BM_WRITE_ONLY;
    } else {
        printk("invalid event\n");
        return count;
    }

    BM_SetReadWriteType(bm_rw_type_evt);

    return count;
}

static ssize_t mon_mode_evt_show(struct device_driver *driver, char *buf)
{
    MonitorMode mon_mode_evt;
    mon_mode_evt = get_mt65xx_mon_mode();
    if(mon_mode_evt == MODE_MANUAL_USER)
        return snprintf(buf, PAGE_SIZE, "Monitor mode = MANUAL_USER\n");
    else if(mon_mode_evt == MODE_SCHED_SWITCH)
        return snprintf(buf, PAGE_SIZE, "Monitor mode = SCHED_SWITCH\n");
    else if(mon_mode_evt == MODE_PERIODIC)
        return snprintf(buf, PAGE_SIZE, "Monitor mode = PERIODIC\n");
    else if(mon_mode_evt == MODE_MANUAL_TRACER)
        return snprintf(buf, PAGE_SIZE, "Monitor mode = MANUAL_TRACER\n");  
    else if(mon_mode_evt == MODE_MANUAL_KERNEL)
        return snprintf(buf, PAGE_SIZE, "Monitor mode = MANUAL_KERNEL\n");
    else if(mon_mode_evt == MODE_FREE)          
        return snprintf(buf, PAGE_SIZE, "Monitor mode = FREE\n");
    else
        return snprintf(buf, PAGE_SIZE, "Monitor mode = Unknown\n");
}

ssize_t mon_mode_evt_store(struct device_driver *driver, const char *buf, size_t count)
{
    MonitorMode mon_mode_evt;
    if (!strncmp(buf, "SCHED_SWITCH", strlen("SCHED_SWITCH"))) {
        mon_mode_evt = MODE_SCHED_SWITCH;
    } else if (!strncmp(buf, "PERIODIC", strlen("PERIODIC"))) {
        mon_mode_evt = MODE_PERIODIC;
    } else if (!strncmp(buf, "MANUAL_TRACER", strlen("MANUAL_TRACER"))) {
        mon_mode_evt = MODE_MANUAL_TRACER;
    } else {
        printk("invalid event\n");
        return count;
    }
    
    set_mt65xx_mon_mode(mon_mode_evt);
    
    return count;
}

static ssize_t mon_period_evt_show(struct device_driver *driver, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "Monitor period = %ld (for periodic mode)\n", get_mt65xx_mon_period());    
}

ssize_t mon_period_evt_store(struct device_driver *driver, const char *buf, size_t count)
{
    
    sscanf(buf, "%ld", &mon_period_evt);    
    set_mt65xx_mon_period(mon_period_evt);
    
    return count;
}

static ssize_t mon_manual_evt_show(struct device_driver *driver, char *buf)
{
    mon_manual_evt = get_mt65xx_mon_manual_start();
    if(mon_manual_evt == 1)
        return snprintf(buf, PAGE_SIZE, "Manual Monitor is Started (for manual mode)\n");
    else if(mon_manual_evt == 0)
        return snprintf(buf, PAGE_SIZE, "Manual Monitor is Stopped (for manual mode)\n");
    else
        return 0;
}

ssize_t mon_manual_evt_store(struct device_driver *driver, const char *buf, size_t count)
{
    
    if (!strncmp(buf, "START", strlen("START"))) {
        mon_manual_evt = 1;
    } else if (!strncmp(buf, "STOP", strlen("STOP"))) {
        mon_manual_evt = 0;
    } else {
        printk("invalid event\n");
        return count;
    }
    
    set_mt65xx_mon_manual_start(mon_manual_evt);
    
    return count;
}

static ssize_t cpu_pmu_evt_show(struct device_driver *driver, char *buf)   
{   

    u32 j;
    int size = 0;
    
    if(p_pmu == NULL) {
        printk("PMU user interface isn't ready now!\n"); 
        return 0;
    }    
    
    for(j = 0; j < NUMBER_OF_EVENT; j++) {
        size += sprintf(buf + size, "Evt%d = 0x%x\t", j, p_pmu->perf_cfg.event_cfg[j]);    
    }
    size += sprintf(buf + size, "\n");
    
    return (size); 
}
 
ssize_t cpu_pmu_evt_store(struct device_driver *driver, const char *buf, size_t count) 
{   
        
    char *p = (char *)buf;    
    char *token[8];
    char *ptr;
    int i = 0;
    
    if((strlen(buf)+1) > 128)
    {
      printk("PMU: command overflow!");
      return -1;
    }
    
    do{
        ptr = strsep (&p, " ");
        token[i] = ptr;
        i++;
    }while(ptr != NULL);
    
    if(i != 8){
        printk("The number of parameter is wrong!!!\n");
        return -1;
    }
    
    
    for(i = 0; i < NUMBER_OF_EVENT; i++) 
        p_pmu->perf_cfg.event_cfg[i] =  simple_strtoul(token[i], &token[i], 16);
        
    for(i=0; i<6; i++)
        printk("%x  ",p_pmu->perf_cfg.event_cfg[i]);
    printk("\n");
    p_pmu->enable();

    return count;   
}


DRIVER_ATTR(l2c_cnt0_evt, 0644, l2c_cnt0_evt_show, l2c_cnt0_evt_store);
DRIVER_ATTR(l2c_cnt1_evt, 0644, l2c_cnt1_evt_show, l2c_cnt1_evt_store);


DRIVER_ATTR(bm_master_evt, 0644, bm_master_evt_show, bm_master_evt_store);
DRIVER_ATTR(bm_rw_type_evt, 0644, bm_rw_type_evt_show, bm_rw_type_evt_store);

DRIVER_ATTR(mon_mode_evt, 0644, mon_mode_evt_show, mon_mode_evt_store);
DRIVER_ATTR(mon_period_evt, 0644, mon_period_evt_show, mon_period_evt_store);
DRIVER_ATTR(mon_manual_evt, 0644, mon_manual_evt_show, mon_manual_evt_store);

DRIVER_ATTR(cpu_pmu_cfg, 0644, cpu_pmu_evt_show, cpu_pmu_evt_store);


struct mt_mon_driver {

    struct device_driver driver;
    const struct  platform_device_id *id_table;
};

static struct mt_mon_driver mt6577_mon_drv = 
{
    .driver = {
        .name = "mt6577_monitor",
        .bus = &platform_bus_type,
        .owner = THIS_MODULE,
    },
    .id_table = NULL,
};


struct mtk_monitor mtk_mon = {
        .init           = mt65xx_mon_init,
        .deinit         = mt65xx_mon_deinit,
        .enable         = mt65xx_mon_enable,
        .disable        = mt65xx_mon_disable,
        .mon_log        = mt65xx_mon_log,
        .set_pmu        = mt65xx_mon_set_pmu,
        .get_pmu        = mt65xx_mon_get_pmu,
        .set_l2c        = mt65xx_mon_set_l2c,
        .get_l2c        = mt65xx_mon_get_l2c,
        .set_bm_rw      = mt65xx_mon_set_bm_rw,
};

/*
 * mt6577_mon_mod_init: module init function
 */
static int __init mt6577_mon_mod_init(void)
{
    int ret;
    unsigned long flags;
    /* register driver and create sysfs files */
    ret = driver_register(&mt6577_mon_drv.driver);

    if (ret) {
        printk("fail to register mt6577_mon_drv\n");
        return ret;
    }

    ret = driver_create_file(&mt6577_mon_drv.driver, &driver_attr_l2c_cnt0_evt);
    ret |= driver_create_file(&mt6577_mon_drv.driver, &driver_attr_l2c_cnt1_evt);
    ret |= driver_create_file(&mt6577_mon_drv.driver, &driver_attr_bm_master_evt);
    ret |= driver_create_file(&mt6577_mon_drv.driver, &driver_attr_bm_rw_type_evt);
    ret |= driver_create_file(&mt6577_mon_drv.driver, &driver_attr_mon_mode_evt);
    ret |= driver_create_file(&mt6577_mon_drv.driver, &driver_attr_mon_period_evt);    
    ret |= driver_create_file(&mt6577_mon_drv.driver, &driver_attr_mon_manual_evt);
    ret |= driver_create_file(&mt6577_mon_drv.driver, &driver_attr_cpu_pmu_cfg);

    if (ret) {
        printk("fail to create mt6577_mon sysfs files\n");

        return ret;
    }

    /* SPNIDEN[12] must be 1 for using ARM11 performance monitor unit */
//    *(volatile unsigned int *)0xF702A000 |= 0x1000;


    ret = register_pmu(&p_pmu);
    if(ret != 0 || p_pmu == NULL) {
        printk("Register PMU Fail\n");
        return ret;
    }
    
    /* set default L2C counter's events */
    raw_spin_lock_irqsave(&l2x0_lock, flags);
    __raw_writel(DEF_L2C_CNT0_EVT << 2, PL310_BASE + L2X0_EVENT_CNT0_CFG);
    __raw_writel(DEF_L2C_CNT1_EVT << 2, PL310_BASE + L2X0_EVENT_CNT1_CFG);
    dsb();
    raw_spin_unlock_irqrestore(&l2x0_lock, flags);

    /* init EMI bus monitor */
    BM_SetReadWriteType(DEF_BM_RW_TYPE);
    BM_SetMonitorCounter(1, BM_MASTER_MULTIMEDIA, BM_TRANS_TYPE_4BEAT | BM_TRANS_TYPE_8Byte | BM_TRANS_TYPE_BURST_WRAP);
    BM_SetMonitorCounter(2, BM_MASTER_AP_MCU, BM_TRANS_TYPE_4BEAT | BM_TRANS_TYPE_8Byte | BM_TRANS_TYPE_BURST_WRAP);
    BM_SetMonitorCounter(3, BM_MASTER_MD_DSP | BM_MASTER_MD_MCU | BM_MASTER_2G_3G_MDDMA, BM_TRANS_TYPE_4BEAT | BM_TRANS_TYPE_8Byte | BM_TRANS_TYPE_BURST_WRAP);
    BM_SetMonitorCounter(4, BM_MASTER_PERI, BM_TRANS_TYPE_4BEAT | BM_TRANS_TYPE_8Byte | BM_TRANS_TYPE_BURST_WRAP);

    return 0;
}

arch_initcall(mt6577_mon_mod_init);

static DEFINE_SPINLOCK(reg_mon_lock);
int register_monitor(struct mtk_monitor **p_mon, MonitorMode mode)
{
    int ret = 0;
    
    spin_lock(&reg_mon_lock);

    if(register_mode != MODE_FREE) {
        goto _err;
    } else {
        register_mode = mode;
        *p_mon = &mtk_mon;
    }    

    if(mode == MODE_MANUAL_USER || mode == MODE_MANUAL_KERNEL) {
        if (!mt_mon_log_buff) {
            mt_mon_log_buff = vmalloc(sizeof(struct mt_mon_log) * MON_LOG_BUFF_LEN);        
            if (!mt_mon_log_buff) {
                printk(KERN_WARNING "fail to allocate the buffer for the monitor log\n");
                register_mode = MODE_FREE;
                goto _err;
            }
            mtk_mon.log_buff = mt_mon_log_buff;
        }    
    }    

    if(mode == MODE_SCHED_SWITCH)
        p_pmu->multicore = 0;
    else 
        p_pmu->multicore = 1;
    
    spin_unlock(&reg_mon_lock);

    return ret;

_err:
    p_mon = NULL;
    spin_unlock(&reg_mon_lock);
    return -1;    
}   
EXPORT_SYMBOL(register_monitor);

void unregister_monitor(struct mtk_monitor **p_mon)
{
    
    spin_lock(&reg_mon_lock);
    
    if(mt_mon_log_buff) {
        vfree(mt_mon_log_buff);
        mt_mon_log_buff = NULL;
    }
    
    register_mode = MODE_FREE;
    *p_mon = NULL;

    mt6577_kernel_ring_buff_index = 0;
    mt_mon_log_buff_index = 0;
    spin_unlock(&reg_mon_lock);
}
EXPORT_SYMBOL(unregister_monitor);
