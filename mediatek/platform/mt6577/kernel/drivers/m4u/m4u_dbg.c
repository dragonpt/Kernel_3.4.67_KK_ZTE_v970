#include "m4u_dbg.h"
#include "mach/m4u.h"

#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/timer.h>
#include <linux/fs.h>

#include <linux/pagemap.h>

#define M4UMSG(string, args...)	xlog_printk(ANDROID_LOG_INFO, "M4U_K", string,##args)

extern char* m4u_get_module_name(M4U_MODULE_ID_ENUM moduleID);

struct m4u_dbg_fifo *mfifo = NULL;

unsigned char m4u_dbg_nums[M4U_CLNTMOD_MAX] = 
{
    M4U_DBG_NUM_DEFECT          ,
    M4U_DBG_NUM_CAM             ,
    M4U_DBG_NUM_PCA             ,
    M4U_DBG_NUM_JPEG_DEC        ,
    M4U_DBG_NUM_JPEG_ENC        ,
    M4U_DBG_NUM_VDO_ROT0        ,
    M4U_DBG_NUM_RGB_ROT0        ,
    M4U_DBG_NUM_TVROT           ,
    M4U_DBG_NUM_RDMA0           ,
    M4U_DBG_NUM_FD              ,
    M4U_DBG_NUM_FD_INPUT        ,
    M4U_DBG_NUM_RGB_ROT1        ,
    M4U_DBG_NUM_VDO_ROT1        ,
    M4U_DBG_NUM_RGB_ROT2        ,
    M4U_DBG_NUM_OVL             ,
    M4U_DBG_NUM_LCDC            ,
    M4U_DBG_NUM_LCDC_UI         ,
    M4U_DBG_NUM_RDMA1           ,
    M4U_DBG_NUM_TVC             ,
    M4U_DBG_NUM_SPI             ,
    M4U_DBG_NUM_DPI             ,
    M4U_DBG_NUM_VDEC_DMA        ,
    M4U_DBG_NUM_VENC_DMA        ,
    M4U_DBG_NUM_G2D             ,
    M4U_DBG_NUM_AUDIO           ,
    M4U_DBG_NUM_RDMA_GENERAL    ,
    M4U_DBG_NUM_ROTDMA_GENERAL  ,
    M4U_DBG_NUM_UNKNOWN         ,
};

char* __m4u_evt_name(m4u_dbg_event_t event)
{
    switch(event)
    {
        case M4U_EVT_UNKOWN        : return "M4U_EVT_UNKOWN";
        case M4U_EVT_ALLOC_MVA     : return "M4U_EVT_ALLOC_MVA";
        case M4U_EVT_DEALLOC_MVA   : return "M4U_EVT_DEALLOC_MVA";
        case M4U_EVT_QUERY_MVA     : return "M4U_EVT_QUERY_MVA";
        case M4U_EVT_REGISTER_BUF  : return "M4U_EVT_REGISTER_BUF";
        default : 
                                     M4UMSG("[m4u_dbg] invalid event: %d\n", (int)event);
                                     return "";
    }
}

int m4u_dbg_fifo_init(void)
{
    int i;
    int total_num = 0;
    struct m4u_dbg_info* pInfo = NULL;
    mfifo = (struct m4u_dbg_fifo*)kmalloc(sizeof(struct m4u_dbg_fifo)*M4U_CLNTMOD_MAX, GFP_KERNEL);
    if(mfifo == NULL)
    {
        M4UMSG("[m4u_dbg] kmalloc failed for mfifo\n");
        goto error_out;
    }

    for(i=0; i<M4U_CLNTMOD_MAX; i++)
    {
        total_num += m4u_dbg_nums[i];
    }

    M4UMSG("size(m4u_dbg_fifo)=%d, size(m4u_dbg_info)=%d, size(buffer)=%d\n",
            sizeof(struct m4u_dbg_fifo), sizeof(struct m4u_dbg_info), sizeof(struct m4u_dbg_info)*total_num);

    pInfo = (struct m4u_dbg_info*) kzalloc(sizeof(struct m4u_dbg_info)*total_num, GFP_KERNEL);

    if(pInfo == NULL)
    {
        M4UMSG("[m4u_dbg] kmalloc failed for pInfo\n");
        goto error_out;
    }

    for(i=0; i<M4U_CLNTMOD_MAX; i++)
    {
        pInfo += m4u_dbg_nums[i];
        mfifo[i].buffer = pInfo;
        mfifo[i].top = 0;
        mfifo[i].max_num = m4u_dbg_nums[i];
        spin_lock_init(&(mfifo[i].lock));
    }

    return 0;

error_out:
    if(mfifo != NULL)
        kfree(mfifo);

    if(pInfo != NULL)
        kfree(pInfo);

    return -1;

}

int m4u_dbg_push(m4u_dbg_event_t event, M4U_MODULE_ID_ENUM eModuleId, unsigned int va, 
                unsigned int size, unsigned int mva, unsigned int tgid)
{
    struct m4u_dbg_fifo *pfifo = NULL;
    unsigned int index;
    
    if(eModuleId >= M4U_CLNTMOD_MAX)
    {
        M4UMSG("[m4u_dbg] moduleId > M4U_CLNTMOD_MAX : %d\n", (unsigned int)eModuleId);
        return -1;
    }
    pfifo = mfifo+eModuleId;

    //no need to record this event
    if(pfifo->max_num == 0)
        return 0;

    //move top pointer
    spin_lock(&(pfifo->lock));
    index = pfifo->top;
    pfifo->top++;
    if(pfifo->top == pfifo->max_num)
        pfifo->top = 0;

    //M4UMSG("[m4u_dbg] push: module=%d, index=%d, top=%d, max=%d, pfifo=%x, lock=0x%x\n",
    //        eModuleId, index, pfifo->top, pfifo->max_num, pfifo, &(pfifo->lock));
    
    spin_unlock(&(pfifo->lock));

    //fill content
    (pfifo->buffer)[index].event = event;
    (pfifo->buffer)[index].va=va;
    (pfifo->buffer)[index].size=size;
    (pfifo->buffer)[index].mva=mva;
    (pfifo->buffer)[index].tgid=tgid;
    //(pfifo->buffer)[index].time=local_clock();

    return 0;
}

int m4u_dbg_dump_nolock(M4U_MODULE_ID_ENUM eModuleId)
{
    struct m4u_dbg_fifo *pfifo = NULL;
    struct m4u_dbg_info *pbuffer;
    int index;
    int i;

    M4UMSG("==== dump event for module: %s(%d) =====\n", m4u_get_module_name(eModuleId), eModuleId);
    if(eModuleId >= M4U_CLNTMOD_MAX)
    {
        M4UMSG("[m4u_dbg] moduleId > M4U_CLNTMOD_MAX : %d\n", (unsigned int)eModuleId);
        return -1;
    }
    pfifo = mfifo+eModuleId;

    index = pfifo->top;

    for(i=0; i<pfifo->max_num; i++)
    {
        index--;
        if(index < 0) index = pfifo->max_num-1;

        pbuffer = pfifo->buffer + index;
        if(pbuffer->event != 0)
        {
            M4UMSG("[m4u_dbg](%d) evt=%s,va=0x%x,size=0x%x,mva=0x%x,tgid=%d\n", index,
                    __m4u_evt_name(pbuffer->event),pbuffer->va,pbuffer->size,pbuffer->mva,pbuffer->tgid);
        }
    }
    return 0;
}


int m4u_dbg_dump_lock(M4U_MODULE_ID_ENUM eModuleId)
{
    struct m4u_dbg_fifo *pfifo = NULL;

    if(eModuleId >= M4U_CLNTMOD_MAX)
    {
        M4UMSG("[m4u_dbg] moduleId > M4U_CLNTMOD_MAX : %d\n", (unsigned int)eModuleId);
        return -1;
    }
    pfifo = mfifo+eModuleId;

    spin_lock(&(pfifo->lock));
    m4u_dbg_dump_nolock(eModuleId);
    spin_unlock(&(pfifo->lock));
    return 0;
}


