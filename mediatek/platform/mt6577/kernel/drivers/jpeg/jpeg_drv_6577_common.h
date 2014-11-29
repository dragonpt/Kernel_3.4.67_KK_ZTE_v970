#ifndef __JPEG_DRV_6577_COMMON_H__
#define __JPEG_DRV_6577_COMMON_H__

#include <mach/mt_typedefs.h>

int jpeg_isr_enc_lisr(void);
int jpeg_isr_dec_lisr(void);

#if 0
void jpeg_isr_init(void);
void jpeg_isr_dec_lisr(void);
void jpeg_isr_enc_lisr(void);
void jpeg_isr_hisr(void);

void jpeg_isr_dec_idp_complete_lisr_callback(void *dummy);

void jpeg_isr_set_dec_callback(kal_int32 (*callback)(JPEG_CODEC_STATE_ENUM state));
void jpeg_isr_set_enc_callback(kal_int32 (*callback)(JPEG_CODEC_STATE_ENUM state));

void jpeg_isr_dec_lisr(void);
void jpeg_isr_enc_lisr(void);
void jpeg_isr_hisr(void);

void jpeg_drv_init(void);
void jpeg_drv_dec_power_on(void);
void jpeg_drv_dec_power_off(void);
void jpeg_drv_enc_power_on(void);
void jpeg_drv_enc_power_off(void);
#endif

/* Decoder Driver */
void jpeg_drv_dec_reset(void);
void jpeg_drv_dec_start(void);
void jpeg_drv_dec_set_file_size(kal_uint32 size);
void jpeg_drv_dec_set_file_buffer(kal_uint32 buffer, kal_uint32 size);

kal_uint32 *jpeg_drv_dec_get_aligned_table_address(kal_uint8 *initAddress);
void jpeg_drv_dec_set_table_address(kal_uint32 tableAddress);

void jpeg_drv_dec_set_du(kal_uint32 comp, kal_uint32 du, kal_uint32 dummy_du, kal_uint32 du_per_mcu_row);
void jpeg_drv_dec_set_total_mcu(kal_uint32 total_mcu);
void jpeg_drv_dec_set_mcu_per_row(kal_uint32 mcu_per_row);

void jpeg_drv_dec_set_component_id(kal_uint32 id0, kal_uint32 id1, kal_uint32 id2);
void jpeg_drv_dec_set_q_table_id(kal_uint32 id0, kal_uint32 id1, kal_uint32 id2);
void jpeg_drv_dec_set_progressive(kal_uint32 progressive);

void jpeg_drv_dec_set_prog_buffer(kal_uint32 *addr[]);

void jpeg_drv_dec_calc_prog_buffer(kal_uint32 format, kal_uint32 *poolAddr, kal_uint32 poolSize,
                                   kal_uint32 *addr[], kal_uint32 size[]);

kal_uint32 jpeg_drv_dec_get_result(void);

kal_uint32 jpeg_drv_dec_set_sampling_factor_related(kal_uint32 format);

kal_uint8 *jpeg_drv_dec_get_current_file_addr(void);

void jpeg_drv_dec_query_ext_prog_buffer_size(kal_uint32 *extMemorySize);
void jpeg_drv_dec_query_ext_decode_table_size(kal_uint32 *extMemorySize);

void jpeg_drv_dec_set_prog_intlv_mcu_start_end(kal_uint32 start, kal_uint32 end);
void jpeg_drv_dec_set_prog_non_intlv_du_start_end(kal_uint32 comp, kal_uint32 start, kal_uint32 end);

void jpeg_drv_dec_range_enable(void);
void jpeg_drv_dec_range_disable(void);
void jpeg_drv_dec_set_idct_index(kal_uint32 start, kal_uint32 end);
void jpeg_drv_dec_set_idct_skip_index(kal_uint32 index1, kal_uint32 index2);
void jpeg_drv_dec_set_idec_num(kal_uint32 num);


/* Encoder Driver */

void jpeg_drv_enc_reset(void);
void jpeg_drv_enc_start(void);
void jpeg_drv_enc_set_file_format(kal_uint32 fileFormat);
void jpeg_drv_enc_set_mode(kal_uint32 mode, kal_uint32 frames);
void jpeg_drv_enc_set_sync_reset(unsigned char enable_bit);

void jpeg_drv_enc_set_dst_buffer_info(kal_uint32 addr, kal_uint32 stallOffset, kal_uint32 dstIndex);
void jpeg_drv_enc_set_quality(kal_uint32 quality);

kal_uint32 jpeg_drv_enc_query_required_memory(kal_uint32 *intSize, kal_uint32 *extSize);
kal_uint32 jpeg_drv_enc_set_sample_format_related(kal_uint32 width, kal_uint32 height, kal_uint32 samplingFormat);
kal_uint32 jpeg_drv_enc_get_dma_addr(kal_uint8 index);
kal_uint32 jpeg_drv_enc_get_dst_addr(kal_uint8 index);
kal_uint32 jpeg_drv_enc_get_file_size(kal_uint8 index);
kal_uint32 jpeg_drv_enc_get_result(kal_uint32 *fileSize);

kal_uint8 jpeg_drv_enc_get_current_frame(void);


#endif
