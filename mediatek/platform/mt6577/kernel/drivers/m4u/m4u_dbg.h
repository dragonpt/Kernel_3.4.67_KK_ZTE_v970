
#ifndef __M4U_DBG_H__
#define __M4U_DBG_H__

#include "mach/m4u.h"
#include <linux/xlog.h>
#include <linux/aee.h>

#define M4U_DBG_FIFO_EN

//#define M4U_DBG_NR  32 //m4u debug info number for each module

typedef enum
{
    M4U_EVT_UNKOWN = 1,
    M4U_EVT_ALLOC_MVA,
    M4U_EVT_DEALLOC_MVA,
    M4U_EVT_QUERY_MVA,
    M4U_EVT_QUERY_ALLOC,
    M4U_EVT_REGISTER_BUF,
}m4u_dbg_event_t;

struct m4u_dbg_info 
{
    unsigned char event;
    unsigned short flags;
    unsigned int va;
    unsigned int size;
    unsigned int mva;
    unsigned int tgid;
    //unsigned int time;
};

struct m4u_dbg_fifo
{
    struct m4u_dbg_info *buffer;
    int top;
    int max_num;
    spinlock_t lock;
};

#define    M4U_DBG_NUM_DEFECT          (2)   
#define    M4U_DBG_NUM_CAM             (8)   
#define    M4U_DBG_NUM_PCA             (2)   
#define    M4U_DBG_NUM_JPEG_DEC        (16)   
#define    M4U_DBG_NUM_JPEG_ENC        (16)   
#define    M4U_DBG_NUM_VDO_ROT0        (16)       
#define    M4U_DBG_NUM_RGB_ROT0        (16)        
#define    M4U_DBG_NUM_TVROT           (0)   
#define    M4U_DBG_NUM_RDMA0           (16)   
#define    M4U_DBG_NUM_FD              (4)   
#define    M4U_DBG_NUM_FD_INPUT        (4)   
#define    M4U_DBG_NUM_RGB_ROT1        (16)    
#define    M4U_DBG_NUM_VDO_ROT1        (16)    
#define    M4U_DBG_NUM_RGB_ROT2        (16)       
#define    M4U_DBG_NUM_OVL             (2)   
#define    M4U_DBG_NUM_LCDC            (12)   
#define    M4U_DBG_NUM_LCDC_UI         (0)   
#define    M4U_DBG_NUM_RDMA1           (16)   
#define    M4U_DBG_NUM_TVC             (0)   
#define    M4U_DBG_NUM_SPI             (0)   
#define    M4U_DBG_NUM_DPI             (4)   
#define    M4U_DBG_NUM_VDEC_DMA        (24)   
#define    M4U_DBG_NUM_VENC_DMA        (24)   
#define    M4U_DBG_NUM_G2D             (24)   
#define    M4U_DBG_NUM_AUDIO           (0)   
#define    M4U_DBG_NUM_RDMA_GENERAL    (24)    
#define    M4U_DBG_NUM_ROTDMA_GENERAL  (24)        
#define    M4U_DBG_NUM_UNKNOWN         (2)        
 

#ifdef M4U_DBG_FIFO_EN

int m4u_dbg_fifo_init(void);
int m4u_dbg_push(m4u_dbg_event_t event, M4U_MODULE_ID_ENUM eModuleID, unsigned int va, 
        unsigned int size, unsigned int mva, unsigned int tgid);
int m4u_dbg_dump_nolock(M4U_MODULE_ID_ENUM eModuleId);
int m4u_dbg_dump_lock(M4U_MODULE_ID_ENUM eModuleId);

#else

#define  m4u_dbg_fifo_init() 
#define  m4u_dbg_push(event, eModuleID, va, size, mva, tgid)
#define  m4u_dbg_dump_nolock(eModuleId)
#define  m4u_dbg_dump_lock(eModuleId)

#endif





#endif


