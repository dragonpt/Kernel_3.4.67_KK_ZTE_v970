#include <ccci.h>
#include <mach/mt_emi_mpu.h>
#include <linux/delay.h>
//#include <mach/sec_osal.h>

int *dsp_img_vir;
int *md_img_vir;
unsigned long mdconfig_base;
unsigned long mdif_base;
unsigned long ccif_base;
int first;
char image_buf[256] = ""; 
int  valid_image_info = 0;
unsigned int ccci_msg_mask = 0x0;

static int    masp_inited = 0;

struct image_info *pImg_info;
static CCCI_REGION_LAYOUT ccci_layout;
static MD_CHECK_HEADER head;
static GFH_CHECK_CFG_v1 gfh_check_head;
//static bool sign_check = false; // Using sec lib v3, mask this. 2012-2-3
static unsigned int sec_tail_length=0;

#define IMG_NAME_LEN	32

static char * product_str[] = {[INVALID_VARSION]=INVALID_STR, 
							   [DEBUG_VERSION]=DEBUG_STR, 
							   [RELEASE_VERSION]=RELEASE_STR};

static char * type_str[] = {[md_type_invalid]=VER_INVALID_STR, 
							[modem_2g]=VER_2G_STR, 
							[modem_3g]=VER_3G_STR,
							[modem_wg]=VER_WG_STR,
							[modem_tg]=VER_TG_STR};
static char	md_image_post_fix[12];

char ap_platform[16]="";

// For 75 reset
unsigned int modem_infra_sys;
unsigned int md_rgu_base;
unsigned int ap_infra_sys_base;
unsigned int md_ccif_base;

unsigned int shr_mem_phy;
unsigned int shr_mem_len;
unsigned int kernel_base;
unsigned int dram_size;

void platform_set_runtime_data(struct modem_runtime_t *runtime)
{
	#define PLATFORM_VER "MT6575E1" 
	#define DRIVER_VER	 0x20110118

	CCCI_WRITEL(&runtime->Platform_L, 0x3536544D); // "MT65"
  CCCI_WRITEL(&runtime->Platform_H, 0x31453537); // "75E1"
	CCCI_WRITEL(&runtime->DriverVersion, 0x20110118);
	
}


//void start_emi_mpu_protect(void)
void enable_emi_mpu_protection(dma_addr_t ccci_smem_phy_addr, int ccci_smem_length)
{
	kernel_base = get_phys_offset();
	dram_size = get_max_DRAM_size();
	shr_mem_phy = (unsigned int)ccci_smem_phy_addr;
	shr_mem_len = (unsigned int)ccci_smem_length;

	/*Start protect MD/DSP region*/
	CCCI_MSG_INF("ctl", "MPU Start protect MD(%d)/DSP(%d) region...\n",
                                  	pImg_info[MD_INDEX].size,pImg_info[DSP_INDEX].size);

   	emi_mpu_set_region_protection(ccci_layout.modem_region_base, 
                                  	(ccci_layout.modem_region_base + pImg_info[MD_INDEX].size + 0xFFFFF), /*MD_IMG_REGION_LEN*/ 
                                  	0, 
                                  	SET_ACCESS_PERMISSON(SEC_R_NSEC_R, SEC_R_NSEC_R, SEC_R_NSEC_R));
                                                          
#if 0 
  /*Start protect MD region*/
  emi_mpu_set_region_protection(ccci_layout.dsp_region_base, ccci_layout.dsp_region_base + 0x100000, 
                                                          1, 
                                                          SET_ACCESS_PERMISSON(SEC_R_NSEC_R, SEC_R_NSEC_R, SEC_R_NSEC_R));
  emi_mpu_set_region_protection(ccci_layout.dsp_region_base+0x100000, ccci_layout.dsp_region_base + ccci_layout.dsp_region_len, 
                                                          2, 
                                                          SET_ACCESS_PERMISSON(SEC_R_NSEC_R, NO_PRETECTION, SEC_R_NSEC_R));
#endif

	/*Start protect AP region*/
    CCCI_MSG_INF("ctl", "MPU Start protect AP region...\n");

    emi_mpu_set_region_protection(kernel_base, ccci_smem_phy_addr, 3,
                                  	SET_ACCESS_PERMISSON(SEC_R_NSEC_R, SEC_R_NSEC_R, NO_PRETECTION));

//#ifndef MTK_DSPIRDBG
//  emi_mpu_set_region_protection(ccci_smem_phy + 0x4000-1, ccci_smem_phy + ccci_smem_size -1, 
//                                                          CCCI_MPU_REGION,
//                                                          SET_ACCESS_PERMISSON(NO_PRETECTION, SEC_R_NSEC_R, NO_PRETECTION));
//#endif                                                          
// 
	emi_mpu_set_region_protection(ccci_smem_phy_addr + ccci_smem_length + EMI_MPU_ALIGNMENT,  
                                  	dram_size,
                                  	5,
                                  	SET_ACCESS_PERMISSON(SEC_R_NSEC_R, SEC_R_NSEC_R, NO_PRETECTION));
    CCCI_MSG_INF("ctl", "MPU Start MM MAU...\n");

    start_mm_mau_protect(ccci_layout.modem_region_base, 
                        	(ccci_layout.modem_region_base + ccci_layout.modem_region_len + ccci_layout.dsp_region_len), 
  					    	0);
    start_mm_mau_protect(ccci_smem_phy_addr + MAU_NO_M4U_ALIGNMENT, 
                         	ccci_smem_phy_addr + ccci_smem_length, 1);

    //add MAU protect for multi-media
    start_mm_mau_protect(dram_size, 0xFFFFFFFF, 2);
}


void dump_firmware_info(void)
{
	
	CCCI_MSG_INF("ctl", "\n");
	CCCI_MSG_INF("ctl", "****************dump dsp&modem firmware info***************\n");
	
	CCCI_MSG_INF("ctl", "(AP)[Type]%s, [Plat]%s\n", pImg_info[MD_INDEX].ap_info.image_type, 
			pImg_info[MD_INDEX].ap_info.platform);

	CCCI_MSG_INF("ctl", "(MD)[Type]%s, [Plat]%s\n", pImg_info[MD_INDEX].img_info.image_type, 
		pImg_info[MD_INDEX].img_info.platform);

	CCCI_MSG_INF("ctl", "(DSP)[Type]%s, [Plat]%s\n", pImg_info[DSP_INDEX].img_info.image_type, 
		pImg_info[DSP_INDEX].img_info.platform);
	
	CCCI_MSG_INF("ctl", "(MD)[Build_Ver]%s, [Build_time]%s, [Product_ver]%s\n",
		pImg_info[MD_INDEX].img_info.build_ver, pImg_info[MD_INDEX].img_info.build_time,
		pImg_info[MD_INDEX].img_info.product_ver);
	
	CCCI_MSG_INF("ctl", "(DSP)[Build_Ver]%s, [Build_time]%s, [Product_ver]%s\n",
		pImg_info[DSP_INDEX].img_info.build_ver, pImg_info[DSP_INDEX].img_info.build_time,
		pImg_info[DSP_INDEX].img_info.product_ver);
	
	CCCI_MSG_INF("ctl", "-----------------dump dsp&modem firmware info---------------\r\n");

        return;
}


//check dsp header structure
//return value: 0, no dsp header; >0, dsp header check ok; <0, dsp header check fail
static int check_dsp_header(struct file *filp,struct image_info *image, unsigned int addr)
{
	int ret = 0;
	char *start_addr = (char *)addr;
	GFH_HEADER *gfh_head = (GFH_HEADER *)addr;
	GFH_FILE_INFO_v1 *gfh_file_head;
	//GFH_CHECK_CFG_v1 gfh_check_head;
	unsigned int dsp_ver = DSP_VER_INVALID;
	unsigned int ap_ver = DSP_VER_INVALID;
	bool file_info_check = false;
	bool header_check = false;
	bool ver_check = false;
	bool image_check = false;
	bool platform_check = false;
	int md_id=0;
	if (gfh_head == NULL) {
		CCCI_MSG_INF("ctl", "ioremap DSP image failed!\n");
		ret = -ENOMEM;
		goto out;
	}

	while((gfh_head->m_magic_ver & 0xFFFFFF) == GFH_HEADER_MAGIC_NO) {
		if(gfh_head->m_type == GFH_FILE_INFO_TYPE) {
			gfh_file_head = (GFH_FILE_INFO_v1 *)gfh_head;
			file_info_check = true;

			//check image type: DSP_ROM or DSP_BL
			if (gfh_file_head->m_file_type == DSP_ROM_TYPE) {
				image_check = true;
			}
		}
		else if(gfh_head->m_type == GFH_CHECK_HEADER_TYPE) {
			gfh_check_head = *(GFH_CHECK_CFG_v1 *)gfh_head;
			header_check = true;

			//check image version: 2G or 3G
			if(gfh_check_head.m_image_type == get_modem_support(md_id))
				ver_check = true;
			image->ap_info.image_type = type_str[get_modem_support(md_id)];
        	image->img_info.image_type = type_str[gfh_check_head.m_image_type];

			//get dsp product version: debug or release
			image->img_info.product_ver = product_str[gfh_check_head.m_product_ver];
		
			//check chip version: MT6573_S02 or MT6575_S01
 			//ap_platform = ccci_get_platform_ver();
			ccci_get_platform_ver(ap_platform);
			
			//if(!strcmp(gfh_check_head.m_platform_id, ap_platform)) {
			if(!strncmp(gfh_check_head.m_platform_id, ap_platform, strlen(ap_platform))) {
				platform_check = true;
			}	
			image->img_info.platform = gfh_check_head.m_platform_id;
			image->ap_info.platform = ap_platform;

			//get build version and build time
			image->img_info.build_ver = gfh_check_head.m_project_id;	
			image->img_info.build_time = gfh_check_head.m_build_time;
		}

		start_addr += gfh_head->m_size;
		gfh_head = (GFH_HEADER *)start_addr;
	}

	CCCI_MSG_INF("ctl", "\n");
	CCCI_MSG_INF("ctl", "**********************DSP image check****************************\n");
	if(!file_info_check && !header_check) {
		CCCI_MSG_INF("ctl", "GFH_FILE_INFO header and GFH_CHECK_HEADER not exist!\n");
		CCCI_MSG_INF("ctl", "[Reason]No DSP_ROM, please check this image!\n");
		ret = -E_DSP_CHECK;
	}
	else if(file_info_check && !header_check) {
		CCCI_MSG_INF("ctl", "GFH_CHECK_HEADER not exist!\n");

		//check the image version from file_info structure
		dsp_ver = (gfh_file_head->m_file_ver >> DSP_2G_BIT)& 0x1;
		dsp_ver = dsp_ver? AP_IMG_2G:AP_IMG_3G;
		ap_ver = get_modem_support(md_id);

		if(dsp_ver == ap_ver)
			ver_check = true;

		image->ap_info.image_type = type_str[ap_ver];        	
		image->img_info.image_type = type_str[dsp_ver];
			
		if(image_check && ver_check) {	
			CCCI_MSG_INF("ctl", "GFH_FILE_INFO header check OK!\n");
		}
		else {
			CCCI_MSG_INF("ctl", "[Error]GFH_FILE_INFO check fail!\n");
			if(!image_check)
				CCCI_MSG_INF("ctl", "[Reason]not DSP_ROM image, please check this image!\n");

			if(!ver_check)
				CCCI_MSG_INF("ctl", "[Reason]DSP type(2G/3G) mis-match to AP!\n");	
				
			ret = -E_DSP_CHECK;
		}
		
		CCCI_MSG_INF("ctl", "(DSP)[type]=%s\n",(image_check?DSP_ROM_STR:DSP_BL_STR));
		CCCI_MSG_INF("ctl", "(DSP)[ver]=%s, (AP)[ver]=%s\n",image->img_info.image_type,
				image->ap_info.image_type);
	}
	else if(!file_info_check && header_check) {		
		CCCI_MSG_INF("ctl", "GFH_FILE_INFO header not exist!\n");
		
		if(ver_check && platform_check) {
			CCCI_MSG_INF("ctl", "GFH_CHECK_HEADER header check OK!\n");	
		}
		else {
			CCCI_MSG_INF("ctl", "[Error]GFH_CHECK_HEADER check fail!\n");
			
			if(!ver_check)
				CCCI_MSG_INF("ctl", "[Reason]DSP type(2G/3G) mis-match to AP!\n");	

			if(!platform_check)
				CCCI_MSG_INF("ctl", "[Reason]DSP platform version mis-match to AP!\n");
			
			ret = -E_DSP_CHECK;
		}
		CCCI_MSG_INF("ctl", "(DSP)[ver]=%s, (AP)[ver]=%s\n",image->img_info.image_type, 
			image->ap_info.image_type);
		CCCI_MSG_INF("ctl", "(DSP)[plat]=%s, (AP)[plat]=%s\n",image->img_info.platform, 
			image->ap_info.platform);
		CCCI_MSG_INF("ctl", "(DSP)[build_Ver]=%s, [build_time]=%s\n", image->img_info.build_ver , 
			image->img_info.build_time);	
		CCCI_MSG_INF("ctl", "(DSP)[product_ver]=%s\n", product_str[gfh_check_head.m_product_ver]);
		
	}
	else {
		if(image_check && ver_check && platform_check) {
			CCCI_MSG_INF("ctl", "GFH_FILE_INFO header and GFH_CHECK_HEADER check OK!\n");	
		}
		else {
			CCCI_MSG_INF("ctl", "[Error]DSP header check fail!\n");
			if(!image_check)
				CCCI_MSG_INF("ctl", "[Reason]No DSP_ROM, please check this image!\n");

			if(!ver_check)
				CCCI_MSG_INF("ctl", "[Reason]DSP type(2G/3G) mis-match to AP!\n");

			if(!platform_check)
				CCCI_MSG_INF("ctl", "[Reason]DSP platform version mis-match to AP!\n");
	
			ret = -E_DSP_CHECK;
		}
		CCCI_MSG_INF("ctl", "(DSP)[type]=%s\n",(image_check?DSP_ROM_STR:DSP_BL_STR));
		CCCI_MSG_INF("ctl", "(DSP)[ver]=%s, (AP)[ver]=%s\n",image->img_info.image_type, 
			image->ap_info.image_type);
		CCCI_MSG_INF("ctl", "(DSP)[plat]=%s, (AP)[plat]=%s\n",image->img_info.platform, 
			image->ap_info.platform);
		CCCI_MSG_INF("ctl", "(DSP)[build_Ver]=%s, [build_time]=%s\n", image->img_info.build_ver , 
			image->img_info.build_time);	
		CCCI_MSG_INF("ctl", "(DSP)[product_ver]=%s\n", product_str[gfh_check_head.m_product_ver]);
		
	}
	CCCI_MSG_INF("ctl", "**********************DSP image check****************************\r\n");

out:
	return ret;
	
}


static int check_md_header(struct file *filp, struct image_info *image, unsigned int addr)
{
	int ret;
	//MD_CHECK_HEADER head;
	bool md_type_check = false;
	bool md_plat_check = false;
	int md_id=0;

	head = *(MD_CHECK_HEADER *)(addr - sizeof(MD_CHECK_HEADER));

	CCCI_MSG_INF("ctl", "\n");
	CCCI_MSG_INF("ctl", "**********************MD image check***************************\n");
	ret = strncmp(head.check_header, MD_HEADER_MAGIC_NO, 12);
	if(ret) {
		CCCI_MSG_INF("ctl", "md check header not exist!\n");
		ret = 0;
	}
	else {
		if(head.header_verno != MD_HEADER_VER_NO) {
			CCCI_MSG_INF("ctl", "[Error]md check header version mis-match to AP:[%d]!\n", 
				head.header_verno);
		}
		else {
			if((head.image_type != md_type_invalid) && (head.image_type == get_modem_support(md_id))) {
				md_type_check = true;
			}

			ccci_get_platform_ver(ap_platform);
            if(!strncmp(head.platform, ap_platform, strlen(ap_platform))) {
				md_plat_check = true;
			}

			image->ap_info.image_type = type_str[get_modem_support(md_id)];
			image->img_info.image_type = type_str[head.image_type];
			image->ap_info.platform = ap_platform;
			image->img_info.platform = head.platform;
			image->img_info.build_time = head.build_time;
			image->img_info.build_ver = head.build_ver;
			image->img_info.product_ver = product_str[head.product_ver];
			
			if(md_type_check && md_plat_check) {
			
				CCCI_MSG_INF("ctl", "Modem header check OK!\n");
			}
			else {
				CCCI_MSG_INF("ctl", "[Error]Modem header check fail!\n");
				if(!md_type_check)
					CCCI_MSG_INF("ctl", "[Reason]MD type(2G/3G) mis-match to AP!\n");
		
				if(!md_plat_check)
					CCCI_MSG_INF("ctl", "[Reason]MD platform mis-match to AP!\n");
		
				ret = -E_MD_CHECK;
			}
			
			CCCI_MSG_INF("ctl", "(MD)[type]=%s, (AP)[type]=%s\n",image->img_info.image_type, image->ap_info.image_type);
			CCCI_MSG_INF("ctl", "(MD)[plat]=%s, (AP)[plat]=%s\n",image->img_info.platform, image->ap_info.platform);
			CCCI_MSG_INF("ctl", "(MD)[build_ver]=%s, [build_time]=%s\n",image->img_info.build_ver, image->img_info.build_time);
			CCCI_MSG_INF("ctl", "(MD)[product_ver]=%s\n",image->img_info.product_ver);

		}
	}
	CCCI_MSG_INF("ctl", "**********************MD image check***************************\r\n");

	return ret;
}

static int load_cipher_firmware_v2(int fp_id,struct image_info *img,unsigned int cipher_img_offset, unsigned int cipher_img_len)
{
	int ret;
	void *addr = ioremap_nocache(img->address,cipher_img_len);
	void *o_addr = addr;
	unsigned int data_offset;
    struct file *filp = NULL;

	if (addr==NULL) {
		CCCI_MSG_INF("ctl", "ioremap image fialed!\n");
		ret = -E_NO_ADDR;
		goto out;
	}

	if(SEC_OK != sec_ccci_decrypt_cipherfmt(fp_id, cipher_img_offset, (char*)addr, cipher_img_len, &data_offset) ) {
		CCCI_MSG_INF("ctl", "cipher image decrypt fail!\n");
		ret = -E_CIPHER_FAIL;
		goto unmap_out;
	}

    //CCCI_MSG_INF("ctl", "cipher image decrypt ok, data offset is %d(0x%x)\n", data_offset, data_offset);
	img->size = cipher_img_len;
	img->offset += data_offset;	
	addr+=cipher_img_len;

    filp = (struct file *)osal_get_filp_struct(fp_id);
	ret=check_md_header(filp, img, ((unsigned int)addr));
	
unmap_out:
	iounmap(o_addr);
out:
	return ret;
}


/***************************************************************************************************************************
 * New signature check version. 2012-2-2. 
 * Change to use sec_ccci_signfmt_verify_file(char *file_path, unsigned int *data_offset, unsigned int *data_sec_len)
 *  sec_ccci_signfmt_verify_file parameter description
 *    @ file_path: such as etc/firmware/modem.img
 *    @ data_offset: the offset address that bypass signature header
 *    @ data_sec_len: length of signature header + tail
 *    @ return value: 0-success;
 ***************************************************************************************************************************/
//extern int sec_ccci_signfmt_verify_file(char *file_path, unsigned int *data_offset, unsigned int *data_sec_len);
static int signature_check_v2(char* file_path, unsigned int *sec_tail_length)
{
	unsigned int bypass_sec_header_offset=0;
	unsigned int sec_total_len=0;

	if( sec_ccci_signfmt_verify_file(file_path, &bypass_sec_header_offset, &sec_total_len) == 0 ){
		//signature lib check success
		//-- check return value
		CCCI_MSG_INF("ctl", "sign check ret value 0x%x, 0x%x!\n", bypass_sec_header_offset, sec_total_len);
		if(bypass_sec_header_offset > sec_total_len){
			CCCI_MSG_INF("ctl", "sign check get invalid ret value 0x%x, 0x%x!\n", bypass_sec_header_offset, sec_total_len);
			return -E_SIGN_FAIL;
		} else {
			CCCI_MSG_INF("ctl", "sign check success!\n");
			*sec_tail_length = sec_total_len - bypass_sec_header_offset;
			return (int)bypass_sec_header_offset; // Note here, offset is more than 2G is not hoped 
		}
	} else {
		CCCI_MSG_INF("ctl", "sign lib return fail!\n");
		return -E_SIGN_FAIL;
	}
}

/*
 * load_firmware
 */
static int load_firmware(struct file *filp, struct image_info *img)
{
    void *start;
    int   ret = 0;
    int check_ret = 0;
    int   read_size = 0;
    mm_segment_t curr_fs;
    unsigned long load_addr;
    unsigned int end_addr;
    const int size_per_read = 1024 * 1024;
    const int size = 1024;

    curr_fs = get_fs();
    set_fs(KERNEL_DS);

    //load_addr = (img->address + img->offset);
    load_addr = img->address;
    filp->f_pos = img->offset;
	
    while (1) {
        start = ioremap_nocache((load_addr + read_size), size_per_read);
        if (start <= 0) {
		CCCI_MSG_INF("ctl", "CCCI_MD: Firmware ioremap failed:%d\n", (unsigned int)start);
		set_fs(curr_fs);
		return -E_NOMEM;
        }
		
        ret = filp->f_op->read(filp, start, size_per_read, &filp->f_pos);
        if ((ret < 0) || (ret > size_per_read)) {
		CCCI_MSG_INF("ctl", "modem image read failed=%d\n", ret);
		ret = -E_FILE_READ;
		goto error;
        }
	else if(ret == size_per_read) {
	    	//if(read_size == 0)
			//	CCCI_MSG_INF("ctl", "start VM address of %s is:0x%x\n", img->file_name, (unsigned int)start);
			
		read_size += ret;
        	iounmap(start);
	    }	
	 else {
		read_size += ret;
		//if(sign_check)  // Using sec lib v3, mask this. 2012-2-3
		//	img->size = (read_size - HASH_SIG_LEN);
		//	else
	    	img->size = read_size - sec_tail_length; /* Note here, for DSP, sec_tail_length always be ZERO. */	
		CCCI_MSG_INF("ctl", "%s image size=0x%x,read size:%d, tail:%d\n", img->file_name, img->size, read_size, sec_tail_length);
                iounmap(start);
                break;
        }
        
    }
	
	#ifndef android_bring_up_prepare  
	if(img->type == MD_INDEX) {
		start = ioremap_nocache((img->size - size), size);
		end_addr = ((unsigned int)start + size);
		//CCCI_MSG_INF("ctl", "VM address of %s is:0x%x - 0x%x\n", img->file_name, (unsigned int)start,end_addr);
		
		if((check_ret = check_md_header(filp, img, end_addr)) < 0) {
			ret = check_ret;
			goto error;
		}
		iounmap(start);
	}
	else if(img->type == DSP_INDEX) {
		start = ioremap_nocache(load_addr, size);
		//CCCI_MSG_INF("ctl", "start VM address of %s is:0x%x\n", img->file_name, (unsigned int)start);
		if((check_ret = check_dsp_header(filp, img, (unsigned int)start))<0){
			ret = check_ret;
			goto error;	
		}
		iounmap(start);
	}
#endif
	
	set_fs(curr_fs);
	CCCI_MSG_INF("ctl", "Load %s (size=%d) to 0x%lx\n", img->file_name, read_size, load_addr);

	return read_size;
	
error:
	iounmap(start);
	set_fs(curr_fs);
	return ret;
	
}

static int find_img_to_open(int md_id,int img_type, char found_name[])
{
	char			img_name[3][IMG_NAME_LEN];
	char			full_path[64];
	int				i,j;
	char			post_fix[12];
	char			post_fix_ex[12];
	struct file		*filp = NULL;
	char 			*img_path_list[]={CONFIG_MODEM_FIRMWARE_CIP_PATH,CONFIG_MODEM_FIRMWARE_PATH};
	// Gen file name
	get_md_post_fix(md_id,post_fix, post_fix_ex);

	if(img_type == MD_INDEX){ // Gen MD image name
		snprintf(img_name[0], IMG_NAME_LEN, "modem_%s.img", post_fix_ex); 
		snprintf(img_name[1], IMG_NAME_LEN, "modem_%s.img", post_fix);
		snprintf(img_name[2], IMG_NAME_LEN, "%s", MOEDM_IMAGE_NAME); 
	} else if (img_type == DSP_INDEX) { // Gen DSP image name
		snprintf(img_name[0], IMG_NAME_LEN, "DSP_ROM_%s", post_fix_ex); 
		snprintf(img_name[1], IMG_NAME_LEN, "DSP_ROM_%s", post_fix);
		snprintf(img_name[2], IMG_NAME_LEN, "%s", DSP_IMAGE_NAME);
	} else {
		CCCI_MSG_INF("ctl", "[Error]Invalid img type%d\n", img_type);
		return -1;
	}	
	for(j=0;j<sizeof(img_path_list);j++)
	{
		CCCI_MSG_INF("ctl","Find img in %s\n",img_path_list[j]);
		for(i=0; i<3; i++)
		{
			CCCI_MSG_INF("ctl","try to open %s ...\n", img_name[i]);
			snprintf(full_path, 64, "%s%s", img_path_list[j], img_name[i]);
			filp = filp_open(full_path, O_RDONLY, 0644);
			CCCI_MSG_INF("ctl","opened %s ,err=%d\n", img_name[i],IS_ERR(filp));
			if (IS_ERR(filp)==0) 
			{
				snprintf(found_name, 64, full_path);
				filp_close(filp, current->files);
				if(i==1) {
					snprintf(md_image_post_fix, sizeof(md_image_post_fix), "%s", post_fix);
				} else if(i==0) {
					snprintf(md_image_post_fix, sizeof(md_image_post_fix), "%s", post_fix_ex);
				} else {
					md_image_post_fix[0] = '\0';
				}
				return 0;
			}
		}
	}
	md_image_post_fix[0] = '\0';
	CCCI_MSG_INF("ctl","[Error] No Image file found, Orz...\n");
	return -1;
}

#ifndef android_bring_up_prepare
//extern char sec_emmc_project(void);
int sec_lib_version_check(void)
{
	int ret = 0;
	/* Note here, must sync with sec lib, if ccci and sec has dependency change */
	#define CURR_SEC_CCCI_SYNC_VER		(1)
	int sec_lib_ver = sec_ccci_version_info();
	if(sec_lib_ver != CURR_SEC_CCCI_SYNC_VER){
		CCCI_MSG_INF("ctl", "sec lib for ccci mismatch! sec ver:%d, ccci:%d\n", sec_lib_ver, CURR_SEC_CCCI_SYNC_VER);
		ret = -1;
	}

//	if(sec_emmc_project() == 0){
//		CCCI_MSG_INF("ctl", "ccci using nand sec lib\n");
//	} else {
//		CCCI_MSG_INF("ctl", "ccci using emmc sec lib\n");
//	}
	return ret;
}
#endif

static int load_firmware_func(struct image_info *img)
{
    struct file *filp = NULL;
    int fp_id = OSAL_FILE_NULL;
    //CIPHER_HEADER cipher_header;
    int ret=0;
    int offset=0;
    unsigned int img_len=0;
	
	CCCI_MSG_INF("ctl","load_firmware_func() masp_inited:%d\n", masp_inited);
	if (!masp_inited) {
		#ifndef android_bring_up_prepare  
		if ((ret=sec_boot_init()) !=0) {
			CCCI_MSG_INF("ctl", "sec_boot_init failed ret=%d\n",ret);
			ret= -EIO;
      return ret;
		}
	
		if(sec_lib_version_check()!= 0) {
			CCCI_MSG_INF("ctl", "sec lib version check error\n");
			ret= -EIO;
      return ret;
		}	
		#endif
		masp_inited = 1;
	}

    //get modem&dsp image name with E1&E2 
	if(find_img_to_open(0,img->type, img->file_name)<0) {
		CCCI_MSG_INF("ctl", "Find image  %s failed\n",img->file_name);
		filp = NULL;
		goto out;
	}

    fp_id = osal_filp_open_read_only(img->file_name);  
    filp = (struct file *)osal_get_filp_struct(fp_id);
    if (IS_ERR(filp)) {
        CCCI_MSG_INF("ctl","open %s fail(%ld), try to open modem.img/DSP_ROM\n",
			img->file_name, PTR_ERR(filp));
		goto open_file;
    }
    else
		goto check_head;

open_file:
    //get default modem&dsp image name (modem.img & DSP_ROM)
    snprintf(img->file_name,sizeof(img->file_name),
			CONFIG_MODEM_FIRMWARE_PATH "%s",(img->type?DSP_IMAGE_NAME:MOEDM_IMAGE_NAME));	
	
    CCCI_MSG_INF("ctl", "open %s \n",img->file_name);
    fp_id = osal_filp_open_read_only(img->file_name);
    //filp = filp_open(img->file_name, O_RDONLY, 0777);
    filp = (struct file *)osal_get_filp_struct(fp_id);
    if (IS_ERR(filp)) {
        CCCI_MSG_INF("ctl", "open %s failed:%ld\n",img->file_name, PTR_ERR(filp));
        ret = -E_FILE_OPEN;
		filp = NULL;
		goto out;
    }

check_head:
    //only modem.img need check signature and cipher header
    //sign_check = false;
    sec_tail_length = 0;
    if(img->type == MD_INDEX) {
        //step1:check if need to signature
        //offset=signature_check(filp);
        
	#ifndef android_bring_up_prepare
        offset = signature_check_v2(img->file_name, &sec_tail_length);
        CCCI_MSG_INF("ctl", "signature_check offset:%d, tail:%d\n",offset, sec_tail_length);
        if (offset<0) {
            CCCI_MSG_INF("ctl", "signature_check failed ret=%d\n",offset);
            ret=offset;
            goto out;
        }
        #endif

        img->offset=offset;
		//if(offset == sizeof(SEC_IMG_HEADER))  // Using sec lib v3, mask this. 2012-2-3
		//	sign_check = true;

		//step2:check if need to cipher
		#if 0 // Using new cipher api, mask the following. 2012-3-8
		ret=kernel_read(filp,offset,(char*)&cipher_header,sizeof(CIPHER_HEADER));	
		if (ret!=sizeof(CIPHER_HEADER)) {
			CCCI_MSG_INF("ctl", "read cipher header failed:ret=%d!\n",ret);
			ret = -E_KERN_READ;
			goto out;
    	}

    	if (cipher_header.magic_number==CIPHER_IMG_MAGIC) {
			cipher_header.image_offset +=offset;
			CCCI_MSG_INF("ctl", "img_len:%d img_offset:%d cipher_len:%d cipher_offset:%d cust_name:%s.\n",
			cipher_header.image_length,cipher_header.image_offset,cipher_header.cipher_length,
			cipher_header.cipher_offset,cipher_header.cust_name[0]?(char*)cipher_header.cust_name:"Unknow");
		#endif
        
		#ifndef android_bring_up_prepare       
		if (sec_ccci_is_cipherfmt(fp_id,offset,&img_len)) {// Cipher image
			CCCI_MSG_INF("ctl", "cipher image\n");
			//ret=load_cipher_firmware(filp,img,&cipher_header);
			ret=load_cipher_firmware_v2(fp_id,img,offset, img_len);
			if(ret<0) {
				CCCI_MSG_INF("ctl", "load_cipher_firmware failed:ret=%d!\n",ret);
				goto out;
			}
			CCCI_MSG_INF("ctl", "load_cipher_firmware done! (=%d)\n",ret);
		} 
		else {
		#endif	
			CCCI_MSG_INF("ctl", "Not cipher image\n");
			ret=load_firmware(filp,img);
			if(ret<0) {
   				CCCI_MSG_INF("ctl", "load_firmware failed:ret=%d!\n",ret);
				goto out;
    		}
		#ifndef android_bring_up_prepare           
		}
		#endif        
	}
	//dsp image check signature during uboot, and ccci not need check for dsp.
	else if(img->type == DSP_INDEX) {
		ret=load_firmware(filp,img);
		if(ret<0) {
   			CCCI_MSG_INF("ctl", "load_firmware for %s failed:ret=%d!\n",img->file_name,ret);
			goto out;
    	}
	}

out:
	if(filp != NULL){ 
        osal_filp_close(fp_id);
	    //filp_close(filp,current->files);
    }

	return ret;
}

enum {
	MD_DEBUG_REL_INFO_NOT_READY = 0,
	MD_IS_DEBUG_VERSION,
	MD_IS_RELEASE_VERSION
};
static int modem_is_debug_ver = MD_DEBUG_REL_INFO_NOT_READY;
static void store_modem_dbg_rel_info(char *str)
{
	if( NULL == str)
		modem_is_debug_ver = MD_DEBUG_REL_INFO_NOT_READY;
	else if(strcmp(str, "Debug") == 0)
		modem_is_debug_ver = MD_IS_DEBUG_VERSION;
	else
		modem_is_debug_ver = MD_IS_RELEASE_VERSION;
}
int is_modem_debug_ver(void)
{
	return modem_is_debug_ver;
}

static ccci_aed_cb_t aed_cb_func_p = NULL;
static void ccci_aed_cb(unsigned int flag, char* aed_str)
{
	if(aed_cb_func_p)
		aed_cb_func_p(flag, aed_str);
}
void ccci_aed_cb_register(ccci_aed_cb_t funcp)
{
	if(NULL == funcp){
		CCCI_MSG_INF("ctl", "Got null aed function pointer\n");
	}else if(aed_cb_func_p){
		CCCI_MSG_INF("ctl", "aed function pointer has registered\n");
	}else{
		aed_cb_func_p = funcp;
		CCCI_MSG_INF("ctl", "aed function pointer registy done\n");
	}
}

int  ccci_load_firmware(unsigned int load_flag)
{
    int i;
    int ret = 0;
    int ret_a[IMG_CNT] = {0,0};
    char err_buf[128] = "";
    unsigned int img_load = 0;
	
    //step1:get modem&dsp image info
    if(load_flag == LOAD_MD_DSP) {
    	pImg_info = kzalloc((2*sizeof(struct image_info)),GFP_KERNEL);
    	if (pImg_info == NULL) {
			CCCI_MSG_INF("ctl", "kmalloc for image_info structure fail!\n");
			ret = -E_NOMEM;
			goto out;
    	}
    }

    pImg_info[MD_INDEX].type = MD_INDEX;
    pImg_info[DSP_INDEX].type = DSP_INDEX;
    pImg_info[MD_INDEX].address = ccci_layout.modem_region_base;
    pImg_info[MD_INDEX].offset = pImg_info[DSP_INDEX].offset = 0;
    pImg_info[DSP_INDEX].address = ccci_layout.dsp_region_base;
    pImg_info[MD_INDEX].load_firmware = load_firmware_func;
    pImg_info[DSP_INDEX].load_firmware = load_firmware_func;
	
    	
 	//step2: load image
	//clear modem protection when start to load firmware
	CCCI_CTL_MSG("Clear region protect...\n");
    emi_mpu_set_region_protection(ccci_layout.modem_region_base, (ccci_layout.modem_region_base + pImg_info[MD_INDEX].size + 0xFFFFF), 0, SET_ACCESS_PERMISSON(NO_PRETECTION, NO_PRETECTION, NO_PRETECTION));
    emi_mpu_set_region_protection(0, 0, 1, SET_ACCESS_PERMISSON(NO_PRETECTION, NO_PRETECTION, NO_PRETECTION));
    emi_mpu_set_region_protection(0, 0, 2, SET_ACCESS_PERMISSON(NO_PRETECTION, NO_PRETECTION, NO_PRETECTION));
    emi_mpu_set_region_protection(kernel_base, shr_mem_phy, 3, SET_ACCESS_PERMISSON(NO_PRETECTION, NO_PRETECTION, NO_PRETECTION));
    emi_mpu_set_region_protection(shr_mem_phy + shr_mem_len + EMI_MPU_ALIGNMENT, dram_size, 5, SET_ACCESS_PERMISSON(NO_PRETECTION, NO_PRETECTION, NO_PRETECTION));
    

	for(i = 0; i < IMG_CNT; i++) {
		img_load = load_flag&(1<<i);
		if (img_load && pImg_info[i].load_firmware) {
			if((ret_a[i]= pImg_info[i].load_firmware(&pImg_info[i])) < 0){
				CCCI_MSG_INF("ctl", "load firmware fail for %s!\n", pImg_info[i].file_name);
			}
		}
		else if (img_load == 0) {
			CCCI_MSG_INF("ctl", "Not load firmware for %s!\n", (i?DSP_IMAGE_NAME:MOEDM_IMAGE_NAME));
    	        }
		else {
			CCCI_MSG_INF("ctl", "load null firmware for %s!\n", pImg_info[i].file_name);
			ret_a[i] = -E_FIRM_NULL; 
    	        }
	}

	//need protect after load firmeware is completed
	//start_emi_mpu_protect();

out:
	/* Construct image information string */
	if(ret_a[DSP_INDEX] == -E_FILE_OPEN)	
		sprintf(image_buf, "DSP:%s*%s*%s*%s\nMD:%s*%s*%s*%s\nAP:%s*%s (DSP)%s (MD)%s\n",	
			pImg_info[DSP_INDEX].img_info.image_type,pImg_info[DSP_INDEX].img_info.platform,
			pImg_info[DSP_INDEX].img_info.build_ver,pImg_info[DSP_INDEX].img_info.build_time,
			pImg_info[MD_INDEX].img_info.image_type,pImg_info[MD_INDEX].img_info.platform, 
			pImg_info[MD_INDEX].img_info.build_ver,pImg_info[MD_INDEX].img_info.build_time,
			pImg_info[MD_INDEX].ap_info.image_type,pImg_info[MD_INDEX].ap_info.platform,
			pImg_info[DSP_INDEX].img_info.product_ver,pImg_info[MD_INDEX].img_info.product_ver);
	else
		sprintf(image_buf, "DSP:%s*%s*%s*%s\nMD:%s*%s*%s*%s\nAP:%s*%s (DSP)%s (MD)%s\n",	
			pImg_info[DSP_INDEX].img_info.image_type,pImg_info[DSP_INDEX].img_info.platform,
			pImg_info[DSP_INDEX].img_info.build_ver,pImg_info[DSP_INDEX].img_info.build_time,
			pImg_info[MD_INDEX].img_info.image_type,pImg_info[MD_INDEX].img_info.platform, 
			pImg_info[MD_INDEX].img_info.build_ver,pImg_info[MD_INDEX].img_info.build_time,
			pImg_info[DSP_INDEX].ap_info.image_type,pImg_info[DSP_INDEX].ap_info.platform,
			pImg_info[DSP_INDEX].img_info.product_ver,pImg_info[MD_INDEX].img_info.product_ver);

	valid_image_info = 1;
	store_modem_dbg_rel_info(pImg_info[MD_INDEX].img_info.product_ver);

	if(ret_a[MD_INDEX] == -E_SIGN_FAIL) {
		sprintf(err_buf, "%s Signature check fail\n", pImg_info[i].file_name);
		CCCI_MSG_INF("ctl", "signature check fail!\n");
		ccci_aed_cb(0, err_buf);
	}
	else if(ret_a[MD_INDEX] == -E_CIPHER_FAIL) {
		sprintf(err_buf, "%s Cipher chekc fail\n", pImg_info[i].file_name);
		CCCI_MSG_INF("ctl", "cipher check fail!\n");
		ccci_aed_cb(0, err_buf);
	}
	else if((ret_a[DSP_INDEX] == -E_FILE_OPEN) || (ret_a[MD_INDEX] == -E_FILE_OPEN)) {
		ccci_aed_cb(0, "[ASSERT] Modem/DSP image not exist\n");
	}
	else if((ret_a[DSP_INDEX] == -E_DSP_CHECK)&&(ret_a[MD_INDEX] != -E_MD_CHECK)) {
		ccci_aed_cb(0, "[ASSERT] DSP mismatch to AP\n");
	}
	else if((ret_a[DSP_INDEX] != -E_DSP_CHECK)&&(ret_a[MD_INDEX] == -E_MD_CHECK)) {
		ccci_aed_cb(0, "[ASSERT] Modem mismatch to AP\n\n");
	}
	else if((ret_a[DSP_INDEX] == -E_DSP_CHECK)&&(ret_a[MD_INDEX] == -E_MD_CHECK)) {
		ccci_aed_cb(0, "[ASSERT] Modem&DSP mismatch to AP\n\n");
	}
	
	if((ret_a[MD_INDEX] < 0) || (ret_a[DSP_INDEX] < 0))
		ret = -E_LOAD_FIRM;
	
	return ret;	
}


#if 0
static void dsp_debug(void *data __always_unused)
{
	my_mem_dump(dsp_img_vir,DSP_IMG_DUMP_SIZE,my_default_end,
		"\n\n[CCCI_MD_IMG][%p][%dBytes]:",dsp_img_vir,DSP_IMG_DUMP_SIZE);
	
}
#endif
static spinlock_t md_wdt_mon_lock;
typedef int (*int_func_void_t)(void);
int_func_void_t md_wdt_notify = NULL;

static irqreturn_t md_dsp_wdt_isr(int irq,void *data __always_unused)
{

	DEBUG_INFO_T debug_info;
	unsigned long flags;
	unsigned int  sta = 0;

	memset(&debug_info,0,sizeof(DEBUG_INFO_T));
	debug_info.type=-1;
	switch (irq)
	{
		case MT_MDWDT_IRQ_LINE:
			if(md_rgu_base) {
				spin_lock_irqsave(&md_wdt_mon_lock, flags);
				sta = WDT_MD_STA(md_rgu_base);			
				//*MD_WDT_CTL =(0x22<<8);
				WDT_MD_STA(md_rgu_base) = WDT_MD_MODE_KEY;
				spin_unlock_irqrestore(&md_wdt_mon_lock, flags);
				CCCI_MSG_INF("ctl", "MD_WDT_STA=%04x(%d)\n", sta, irq);
			}
			debug_info.name="MD wdt timeout";
			break;
		case MT_DSPWDT_IRQ_LINE:
			if(md_rgu_base) {
				CCCI_MSG_INF("ctl", "DSP_WDT_STA=%04x(%d)\n", WDT_DSP_STA(md_rgu_base), irq);
				//*DSP_WDT_CTL =(0x22<<8);
				WDT_DSP_STA(md_rgu_base) = WDT_DSP_MODE_KEY;
			}
			debug_info.name="DSP wdt timeout";
			//debug_info.platform_call=dsp_debug;
			break;
	}
	
	if((md_wdt_notify)&&(sta!=0))
		md_wdt_notify();

	return IRQ_HANDLED;
}

void ccci_md_wdt_notify_register(int_func_void_t funcp)
{
	if(NULL == funcp){
		CCCI_MSG_INF("ctl", "Got null md wdt notify function pointer\n");
	}else if(md_wdt_notify){
		CCCI_MSG_INF("ctl", "Md wdt notify function pointer has registered\n");
	}else{
		md_wdt_notify = funcp;
		CCCI_MSG_INF("ctl", "Md wdt notify function pointer registy done\n");
	}
}

/* Notice this workaround, this is for hardware limitation
 * For MT6575 and MT6577, Edge interrupt will be loss if CPU is at shut down stage,
 * When system is resume, we check whether interrupt has been lost
 */

static void recover_md_wdt_irq(void)
{
	#if 0
	unsigned int status = 0;
	unsigned long flags;

	if(md_rgu_base) {
		spin_lock_irqsave(&md_wdt_mon_lock, flags);
		status = WDT_MD_STA(md_rgu_base);
		WDT_MD_STA(md_rgu_base) = WDT_MD_MODE_KEY;
		spin_unlock_irqrestore(&md_wdt_mon_lock, flags);
	}

	if(status){
		// This means md irq missed
		CCCI_MSG_INF("ctl", "recover MD_WDT_STA=%04x.\n", status);
		if(md_wdt_notify)
			md_wdt_notify();
	}
	// status == 0 means nothing happen, return directly
	#endif
}


static int __init md_dsp_irq_init(void)
{
	int ret;
	
	//mt65xx_irq_set_sens(MT6577_MDWDT_IRQ_LINE, MT65xx_EDGE_SENSITIVE);
	ret=request_irq(MT_MDWDT_IRQ_LINE,md_dsp_wdt_isr,IRQF_TRIGGER_FALLING,"MD-WDT",NULL);
	if (ret) {
		CCCI_MSG_INF("ctl", "Failed for MDWDT_IRQ(%d).\n",ret);
		return ret;
	}
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
	mt65xx_irq_unmask(MT_MDWDT_IRQ_LINE);
	#endif
	
	//mt65xx_irq_set_sens(MT_DSPWDT_IRQ_LINE, MT65xx_EDGE_SENSITIVE);
	ret=request_irq(MT_DSPWDT_IRQ_LINE,md_dsp_wdt_isr,IRQF_TRIGGER_FALLING,"DSP-WDT",NULL);
	if (ret) {
		CCCI_MSG_INF("ctl", "Failed for DSPWDT_IRQ(%d).\n",ret);
		free_irq(MT_MDWDT_IRQ_LINE,NULL);
		return ret;
	}
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
	mt65xx_irq_unmask(MT_DSPWDT_IRQ_LINE);
	#endif

	spin_lock_init(&md_wdt_mon_lock);

	register_resume_notify(RSM_ID_RESUME_WDT_IRQ, recover_md_wdt_irq);
	
	return 0;
}

static void md_dsp_irq_deinit(void)
{
	free_irq(MT_MDWDT_IRQ_LINE,NULL);
	//mt65xx_irq_mask(MT_MDWDT_IRQ_LINE);
	disable_irq(MT_MDWDT_IRQ_LINE);

	free_irq(MT_DSPWDT_IRQ_LINE,NULL);
	//mt65xx_irq_mask(MT_DSPWDT_IRQ_LINE);
	disable_irq(MT_DSPWDT_IRQ_LINE);

}


static void map_md_side_register(void)
{
	modem_infra_sys = (int)ioremap_nocache(MD_INFRA_BASE, 0x1000);
	md_rgu_base = (int)ioremap_nocache(0xD10C0000, 0x1000);
	ap_infra_sys_base = INFRA_SYS_CFG_BASE;
	md_ccif_base = 0xF1020000; // MD CCIF Bas;
}

void ungate_md(void)
{
	CCCI_MSG_INF("rst", "ungate_md\n");		
	
	if ( (!modem_infra_sys)||(!md_rgu_base) ) {		
		CCCI_MSG_INF("rst", "fail map md ifrasys and md rgu base!\n");		
		//return -ENOMEM;	
		return;
	}

	/* AP MCU release MD_MCU reset via AP_MD_RGU_SW_MDMCU_RSTN */
	if(WDT_MD_MCU_RSTN(md_rgu_base) & 0x8000) {
		CCCI_MSG_INF("rst", "AP MCU release MD_MCU reset via AP_MD_RGU_SW_MDMCU_RSTN\n");
		WDT_MD_MCU_RSTN(md_rgu_base) = 0x37;
	}

	/* Setting MD & DSP to its default status */
	WDT_MD_LENGTH(md_rgu_base) = WDT_MD_LENGTH_DEFAULT|WDT_MD_LENGTH_KEY;
	WDT_MD_RESTART(md_rgu_base) = WDT_MD_RESTART_KEY;
	WDT_MD_MODE(md_rgu_base) = WDT_MD_MODE_DEFAULT|WDT_MD_MODE_KEY;

	CCCI_CTL_MSG("md_infra_base <%d>, jumpaddr_val <%x>\n",
		modem_infra_sys, *((unsigned int*)(modem_infra_sys + BOOT_JUMP_ADDR)));	
	 
	/*set the start address to let modem to run*/ 
	*((unsigned int*)(modem_infra_sys + BOOT_JUMP_ADDR)) = 0x00000000;  

	//*(unsigned int*)(modem_infra_sys + CLK_SW_CON2) = 0x0155;  
	//*(unsigned int*)(modem_infra_sys + CLK_SW_CON0) = 0x1557;  
	//*(unsigned int*)(modem_infra_sys + CLK_SW_CON2) = 0x0555;  
}

void gate_md(void)
{
	void *p=ioremap(0xd2000800,4);
	if(0 == md_rgu_base){
		CCCI_MSG_INF("ctl", "<rst> md_rgu_base map fail\n");
		return;
	}
	if(0 == ap_infra_sys_base){
		CCCI_MSG_INF("ctl", "<rst> ap_infra_sys_base map fail\n");
		return;
	}
	if(0 == md_ccif_base){
		CCCI_MSG_INF("ctl", "<rst> md_ccif_base map fail\n");
		return;
	}
	/* Disable MD & DSP WDT */
	WDT_MD_MODE(md_rgu_base) = WDT_MD_MODE_KEY;
	WDT_DSP_MODE(md_rgu_base) = WDT_DSP_MODE_KEY;

	/* AP MCU block MDSYS's AXI masters in APCONFIG */
	CCCI_CTL_MSG("<rst> AP MCU block MDSYS's AXI masters in APCONFIG\n");
	INFRA_TOP_AXI_PROTECT_EN(ap_infra_sys_base) |= (0xf<<5);
	dsb();

	/* AP MCU polling MDSYS AXI master IDLE */
	CCCI_CTL_MSG("<rst> AP MCU polling MDSYS AXI master IDLE\n");
	while( (INFRA_TOP_AXI_PROTECT_STA(ap_infra_sys_base)&(0xf<<5)) != (0xf<<5) )
		yield();

	/* Block every bus access form AP to MD */
	CCCI_CTL_MSG("<rst> Block every bus access form AP to MD\n");
	INFRA_TOP_AXI_SI4_CTL(ap_infra_sys_base) &= ~(0x1<<7);
	dsb();

	/* AP MCU assert MD SYS watchdog SW reset in AP_RGU */
	CCCI_CTL_MSG("<rst> AP MCU assert MD SYS watchdog SW reset in AP_RGU\n");
	*(volatile unsigned int*)(AP_RGU_BASE+0x18) = (1<<2)|(0x15<<8);
	dsb();

	/* AP MCU release MD SYS's AXI masters */
	CCCI_CTL_MSG("<rst> AP MCU release MD SYS's AXI masters\n");
	INFRA_TOP_AXI_PROTECT_EN(ap_infra_sys_base) &= ~(0xf<<5);
	dsb();

	CCCI_CTL_MSG("<rst> Wait 0.5s.\n");	
	schedule_timeout_interruptible(HZ/2);

	/* Release MDSYS SW reset in AP RGU */
	CCCI_CTL_MSG("<rst> AP MCU release MD SYS SW reset\n");
	*(volatile unsigned int*)(AP_RGU_BASE+0x18) = (0x15<<8);

	/* AP MCU release AXI bus slave way enable in AP Config */
	CCCI_CTL_MSG("<rst> AP MCU release AXI bus slave way enable in AP Config\n");
	INFRA_TOP_AXI_SI4_CTL(ap_infra_sys_base) |= (0x1<<7);
	dsb();

	/* AP MCU release MD_MCU reset via AP_MD_RGU_SW_MDMCU_RSTN */
	CCCI_CTL_MSG("<rst> AP MCU release MD_MCU reset via AP_MD_RGU_SW_MDMCU_RSTN\n");
	WDT_MD_MCU_RSTN(md_rgu_base) = 0x37;
	dsb();

	/* Tell DSP Reset occour */
	CCCI_CTL_MSG("<rst> AP MCU write FRST to notify DSP\n");
	if (p){
		*((volatile unsigned int *)p)= 0x46525354;   //*((unsigned int*)"FRST"); 
		iounmap(p);
	}else
		CCCI_MSG_INF("ctl", "<rst> remap failed for 0x85000800(0xd2000800)\n");

	/* AP MCU release DSP_MCU reset via AP_MD_RGU_SW_MDMCU_RSTN */
	CCCI_CTL_MSG("<rst> AP MCU release DSP_MCU reset via AP_MD_RGU_SW_DSPMCU_RSTN\n");
	WDT_DSP_MCU_RSTN(md_rgu_base) = 0x48;
	dsb();

	/* Write MD CCIF Ack to clear AP CCIF busy register */
	CCCI_CTL_MSG("<rst> Write MD CCIF Ack to clear AP CCIF busy register\n");
	MD_CCIF_ACK(md_ccif_base)=~0U;
	first=0;
}

int platform_init(void)
{
	int ret=0;

	ccci_get_region_layout(&ccci_layout);
	CCCI_CTL_MSG("mdconfig=0x%lx, ccif=0x%lx, mdif=0x%lx,dsp=0x%lx\n", ccci_layout.mdcfg_region_base,
		ccci_layout.ccif_region_base, ccci_layout.mdif_region_base,ccci_layout.dsp_region_base);

	CCCI_CTL_MSG("mdconfig_len=0x%lx, ccif_len=0x%lx, mdif_len=0x%lx,dsp_len=0x%lx\n", 
		ccci_layout.mdcfg_region_len,ccci_layout.ccif_region_len, ccci_layout.mdif_region_len,ccci_layout.dsp_region_len);

	md_img_vir = (int *)ioremap_nocache(ccci_layout.modem_region_base, MD_IMG_DUMP_SIZE);
	if (!md_img_vir)
	{
		CCCI_MSG_INF("ctl", "MD region ioremap fail!\n");
		return -ENOMEM;
	}

	dsp_img_vir=(int *)ioremap_nocache(ccci_layout.dsp_region_base, DSP_IMG_DUMP_SIZE);
	if (!dsp_img_vir) {
		CCCI_MSG_INF("ctl", "DSP region ioremap fail!\n");
		ret= -ENOMEM;
		goto err_out4;
	}
	CCCI_MSG_INF("ctl", "md_img_vir=0x%lx, dsp_img_vir=0x%lx\n",md_img_vir, dsp_img_vir);

	// FIXME: just a temp solution
	map_md_side_register();

	mdconfig_base=(unsigned long)ioremap_nocache(ccci_layout.mdcfg_region_base, ccci_layout.mdcfg_region_len);
	if (!mdconfig_base) {
		CCCI_MSG_INF("ctl", "mdconfig region ioremap fail!\n");
		ret=-ENOMEM;
		goto err_out3;
	}

	ccif_base=(unsigned long)ioremap_nocache(ccci_layout.ccif_region_base, ccci_layout.ccif_region_len);
	if (!ccif_base) {
        	CCCI_MSG_INF("ctl", "ccif region ioremap fail!\n");
		ret=-ENOMEM;
		goto err_out2;
	}
	
	mdif_base=(unsigned long)ioremap_nocache(ccci_layout.mdif_region_base, ccci_layout.mdif_region_len);
	if (!mdif_base) {
		CCCI_MSG_INF("ctl", "mdif region ioremap fail!\n");
		ret=-ENOMEM;
		goto err_out1;
	}
	CCCI_CTL_MSG("mdconfig_vir=0x%lx, ccif_vir=0x%lx, mdif_vir=0x%lx\n", 
		mdconfig_base, ccif_base, mdif_base);
	
	if ((ret=md_dsp_irq_init()) != 0) 
		goto err_out;

	goto out;
	
err_out_isr:
	md_dsp_irq_deinit();
err_out:
	//ccci_rpc_exit();
//err_out0:
	iounmap((void*)mdif_base);
err_out1:	
	iounmap((void*)ccif_base);	
err_out2:
	iounmap((void*)mdconfig_base);
err_out3:
	iounmap((void*)dsp_img_vir);
err_out4:
	iounmap((void*)md_img_vir);
out:
	return ret;
}

void __exit platform_deinit(void)
{
	iounmap((void*)ccif_base);	
	iounmap((void*)mdconfig_base);
	iounmap((void*)dsp_img_vir);
	iounmap((void*)md_img_vir);
	md_dsp_irq_deinit();
	//ccci_rpc_exit();	

}

/*********************************************************************************************
 * RPC Secton
 *********************************************************************************************/
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/kfifo.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/semaphore.h>
#include <linux/version.h>


unsigned char tst_tmp[512];
unsigned char tst_tmp1[512];
unsigned int  tst_len=512;

#ifndef android_bring_up_prepare 
extern unsigned char sec_secro_en(void);
extern unsigned int sec_secro_md_len(void);
extern unsigned int sec_secro_md_get_data(unsigned char *buf, unsigned int offset, unsigned int len);
extern unsigned int sec_secro_blk_sz(void);
#endif




void ccci_rpc_work_helper(int *p_pkt_num, RPC_PKT pkt[], RPC_BUF *p_rpc_buf, unsigned int tmp_data[])
{
	// tmp_data[] is used to make sure memory address is valid after this function return
	int pkt_num = *p_pkt_num;
	
	CCCI_RPC_MSG("ccci_rpc_work_helper++\n");

	tmp_data[0] = 0;
		
	switch(p_rpc_buf->op_id)
	{
	case IPC_RPC_CPSVC_SECURE_ALGO_OP:
		{
		unsigned char Direction = 0;
		unsigned int  ContentAddr = 0;
		unsigned int  ContentLen = 0;
		sed_t CustomSeed = SED_INITIALIZER;
		unsigned char *ResText __always_unused= NULL;
		unsigned char *RawText __always_unused= NULL;
		unsigned int i __always_unused= 0;

		if(pkt_num < 4)
		{
			CCCI_MSG_INF("rpc", "invalid pkt_num %d for RPC_SECURE_ALGO_OP!\n", pkt_num);
			tmp_data[0] = FS_PARAM_ERROR;
			pkt[pkt_num].len = sizeof(unsigned int);
			pkt[pkt_num++].buf = (void*) &tmp_data[0];
			break;
		}

		Direction = *(unsigned char*)pkt[0].buf;
		ContentAddr = (unsigned int)pkt[1].buf;				
		CCCI_RPC_MSG("RPC_SECURE_ALGO_OP: Content_Addr = 0x%08X, RPC_Base = 0x%08X, RPC_Len = 0x%08X\n", 
			ContentAddr, (unsigned int)p_rpc_buf, sizeof(RPC_BUF));
		if(ContentAddr < (unsigned int)p_rpc_buf || 
							ContentAddr > ((unsigned int)p_rpc_buf + sizeof(RPC_BUF)))
		{
			CCCI_MSG_INF("rpc", "invalid ContentAdddr[0x%08X] for RPC_SECURE_ALGO_OP!\n", ContentAddr);
			tmp_data[0] = FS_PARAM_ERROR;
			pkt[pkt_num].len = sizeof(unsigned int);
			pkt[pkt_num++].buf = (void*) &tmp_data[0];
			break;
		}
		ContentLen = *(unsigned int*)pkt[2].buf;	
		//	CustomSeed = *(sed_t*)pkt[3].buf;
		WARN_ON(sizeof(CustomSeed.sed)<pkt[3].len);
		memcpy(CustomSeed.sed,pkt[3].buf,pkt[3].len);

#ifdef ENCRYPT_DEBUG
			unsigned char log_buf[128];

			if(Direction == TRUE)
				CCCI_MSG_INF("rpc", "HACC_S: EnCrypt_src:\n");
			else
				CCCI_MSG_INF("rpc", "HACC_S: DeCrypt_src:\n");
			for(i = 0; i < ContentLen; i++)
			{
				if(i % 16 == 0){
					if(i!=0){
						CCCI_RPC_MSG("%s\n", log_buf);
					}
					curr = 0;
					curr += snprintf(log_buf, sizeof(log_buf)-curr, "HACC_S: ");
				}
				//CCCI_MSG("0x%02X ", *(unsigned char*)(ContentAddr+i));
				curr += snprintf(&log_buf[curr], sizeof(log_buf)-curr, "0x%02X ", *(unsigned char*)(ContentAddr+i));					
				//sleep(1);
			}
			CCCI_RPC_MSG("%s\n", log_buf);
				
			RawText = kmalloc(ContentLen, GFP_KERNEL);
			if(RawText == NULL)
				CCCI_MSG_INF("rpc", "Fail alloc Mem for RPC_SECURE_ALGO_OP!\n");
			else
				memcpy(RawText, (unsigned char*)ContentAddr, ContentLen);
#endif

		ResText = kmalloc(ContentLen, GFP_KERNEL);
		if(ResText == NULL)
		{
			CCCI_MSG_INF("rpc", "Fail alloc Mem for RPC_SECURE_ALGO_OP!\n");
			tmp_data[0] = FS_PARAM_ERROR;
			pkt[pkt_num].len = sizeof(unsigned int);
			pkt[pkt_num++].buf = (void*) &tmp_data[0];
			break;
		}

		#ifndef android_bring_up_prepare  
		if(!SST_Secure_Init())
		{
			CCCI_MSG_INF("rpc", "SST_Secure_Init fail!\n");
			ASSERT(0);
		}
		
		CCCI_RPC_MSG("RPC_SECURE_ALGO_OP: Dir=0x%08X, Addr=0x%08X, Len=0x%08X, Seed=0x%016llX\n", 
				Direction, ContentAddr, ContentLen, *(long long *)CustomSeed.sed);
		SST_Secure_Algo(Direction, ContentAddr, ContentLen, CustomSeed.sed, ResText);

		if(!SST_Secure_DeInit())
			CCCI_MSG_INF("rpc", "SST_Secure_DeInit fail!\n");
                #endif
		
		pkt_num = 0;
		pkt[pkt_num].len = sizeof(unsigned int);
		pkt[pkt_num++].buf = (void*) &tmp_data[0];
		pkt[pkt_num].len = ContentLen;	
		
		#ifndef android_bring_up_prepare 	
		memcpy(pkt[pkt_num++].buf, ResText, ContentLen);
		#else
		memcpy(pkt[pkt_num++].buf, (void *)ContentAddr, ContentLen);
		#endif
		CCCI_MSG_INF("rpc","RPC_Secure memory copy OK: %d!\n", ContentLen);	
		
#ifdef ENCRYPT_DEBUG
		
			if(Direction == TRUE)
				CCCI_RPC_MSG("HACC_D: EnCrypt_dst:\n");
			else
				CCCI_RPC_MSG("HACC_D: DeCrypt_dst:\n");
			for(i = 0; i < ContentLen; i++)
			{
				if(i % 16 == 0){
					if(i!=0){
						CCCI_RPC_MSG("%s\n", log_buf);
					}
					curr = 0;
					curr += snprintf(&log_buf[curr], sizeof(log_buf)-curr, "HACC_D: ");
				}
				//CCCI_MSG("%02X ", *(ResText+i));
				curr += snprintf(&log_buf[curr], sizeof(log_buf)-curr, "0x%02X ", *(ResText+i));
				//sleep(1);
			}
			
			CCCI_RPC_MSG("%s\n", log_buf);
#if 0
			if(Direction == true)
			{
				RPC_DBG("DeCrypt_back:");			
				SST_Secure_Algo(false, (unsigned int)pkt[pkt_num-1].buf, ContentLen, &CustomSeed, ResText);
			}
			else
			{
				RPC_DBG("EnCrypt_back:");
				SST_Secure_Algo(true, (unsigned int)pkt[pkt_num-1].buf, ContentLen, &CustomSeed, ResText);
			}
			for(i = 0; i < ContentLen; i++)
			{
				if(i % 16 == 0)
					printk("\nHACC_B: ");
				printk("%02X ", *(ResText+i));					
				msleep(10);
			}				
			printk("\n\n");
			msleep(100);

			if(memcmp(ResText, RawText, ContentLen))
				RPC_DBG("Compare Fail!\n");
#endif

			if(RawText)
				kfree(RawText);
		#endif


		kfree(ResText);
		break;
		}

	#ifndef android_bring_up_prepare 
	case IPC_RPC_GET_SECRO_OP:
		{
		unsigned char *addr = NULL;
		unsigned int img_len = 0;
		unsigned int img_len_bak = 0;
		unsigned int blk_sz = 0;
		unsigned int tmp = 1;
		unsigned int cnt = 0;
		unsigned int req_len = 0;	
	
		if(pkt_num != 1) {
			CCCI_MSG("<rpc> RPC_GET_SECRO_OP: invalid parameter: pkt_num=%d \n", pkt_num);
			tmp_data[0] = FS_PARAM_ERROR;
			pkt_num = 0;
			pkt[pkt_num].len = sizeof(unsigned int);
			pkt[pkt_num++].buf = (void*) &tmp_data[0];
			pkt[pkt_num].len = sizeof(unsigned int);
			tmp_data[1] = img_len;
			pkt[pkt_num++].buf = (void*) &tmp_data[1];
			break;
		}
			
		req_len = *(unsigned int*)(pkt[0].buf);				
		if(sec_secro_en()) {
			img_len = sec_secro_md_len();

			if((img_len > IPC_RPC_MAX_BUF_SIZE) || (req_len > IPC_RPC_MAX_BUF_SIZE)) {
				pkt_num = 0;
				tmp_data[0] = FS_MEM_OVERFLOW;
				pkt[pkt_num].len = sizeof(unsigned int);
				pkt[pkt_num++].buf = (void*) &tmp_data[0];
				//set it as image length for modem ccci check when error happens
				pkt[pkt_num].len = img_len;
				///pkt[pkt_num].len = sizeof(unsigned int);
				tmp_data[1] = img_len;
				pkt[pkt_num++].buf = (void*) &tmp_data[1];
				CCCI_MSG("<rpc>RPC_GET_SECRO_OP: md request length is larger than rpc memory: (%d, %d) \n", 
					req_len, img_len);
				break;
			}
			
			if(img_len > req_len) {
				pkt_num = 0;
				tmp_data[0] = FS_NO_MATCH;
				pkt[pkt_num].len = sizeof(unsigned int);
				pkt[pkt_num++].buf = (void*) &tmp_data[0];
				//set it as image length for modem ccci check when error happens
				pkt[pkt_num].len = img_len;
				///pkt[pkt_num].len = sizeof(unsigned int);
				tmp_data[1] = img_len;
				pkt[pkt_num++].buf = (void*) &tmp_data[1];
				CCCI_MSG("<rpc>RPC_GET_SECRO_OP: AP mis-match MD request length: (%d, %d) \n", 
					req_len, img_len);
				break;
			}

            /* TODO : please check it */
            /* save original modem secro length */
            CCCI_MSG("<rpc>RPC_GET_SECRO_OP: save MD SECRO length: (%d) \n",img_len);
            img_len_bak = img_len;
   
			blk_sz = sec_secro_blk_sz();
			for(cnt = 0; cnt < blk_sz; cnt++) {
				tmp = tmp*2;
				if(tmp >= blk_sz)
					break;
			}
			++cnt;
			img_len = ((img_len + (blk_sz-1)) >> cnt) << cnt;

			addr = p_rpc_buf->buf + 4*sizeof(unsigned int);
			tmp_data[0] = sec_secro_md_get_data(addr, 0, img_len);


            /* TODO : please check it */
            /* restore original modem secro length */
            img_len = img_len_bak;

			CCCI_MSG("<rpc>RPC_GET_SECRO_OP: restore MD SECRO length: (%d) \n",img_len);             

			if(tmp_data[0] != 0) {
				CCCI_MSG("<rpc>RPC_GET_SECRO_OP: get data fail:%d \n", tmp_data[0]);
				pkt_num = 0;
				pkt[pkt_num].len = sizeof(unsigned int);
				pkt[pkt_num++].buf = (void*) &tmp_data[0];
				pkt[pkt_num].len = sizeof(unsigned int);
				tmp_data[1] = img_len;
				pkt[pkt_num++].buf = (void*) &tmp_data[1];
			}else {
				CCCI_MSG("<rpc>RPC_GET_SECRO_OP: get data OK: %d,%d \n", img_len, tmp_data[0]);
				pkt_num = 0;
				pkt[pkt_num].len = sizeof(unsigned int);
				//pkt[pkt_num++].buf = (void*) &img_len;
				tmp_data[1] = img_len;
				pkt[pkt_num++].buf = (void*)&tmp_data[1];
				pkt[pkt_num].len = img_len;
				pkt[pkt_num++].buf = (void*) addr;
				//tmp_data[2] = (unsigned int)addr;
				//pkt[pkt_num++].buf = (void*) &tmp_data[2];
			}
		}else {
			CCCI_MSG("<rpc>RPC_GET_SECRO_OP: secro disable \n");
			tmp_data[0] = FS_NO_FEATURE;
			pkt_num = 0;
			pkt[pkt_num].len = sizeof(unsigned int);
			pkt[pkt_num++].buf = (void*) &tmp_data[0];
			pkt[pkt_num].len = sizeof(unsigned int);
			tmp_data[1] = img_len;
			pkt[pkt_num++].buf = (void*) &tmp_data[1];	
		}

		break;
		}
	#endif

		//call eint API to get TDD EINT configuration for modem EINT initial
	case IPC_RPC_GET_TDD_EINT_NUM_OP:
	case IPC_RPC_GET_TDD_GPIO_NUM_OP:
	case IPC_RPC_GET_TDD_ADC_NUM_OP:
                {
                int get_num = 0;
		unsigned char * name = NULL;
		unsigned int length = 0;	

		if(pkt_num < 2)	{
			CCCI_MSG_INF("rpc", "invalid parameter for [0x%X]: pkt_num=%d!\n", 
                                p_rpc_buf->op_id, pkt_num);
			tmp_data[0] = FS_PARAM_ERROR;
			goto err1;
		}

                if((length = pkt[0].len) < 1) {
			CCCI_MSG_INF("rpc", "invalid parameter for [0x%X]: pkt_num=%d, name_len=%d!\n", 
				p_rpc_buf->op_id, pkt_num, length);
			tmp_data[0] = FS_PARAM_ERROR;
			goto err1;
		}

		name = kmalloc(length, GFP_KERNEL);
		if(name == NULL) {
			CCCI_MSG_INF("rpc", "Fail alloc Mem for [0x%X]!\n", p_rpc_buf->op_id);
			tmp_data[0] = FS_ERROR_RESERVED;
			goto err1;
		}
		else {
			memcpy(name, (unsigned char*)(pkt[0].buf), length);

			if(p_rpc_buf->op_id == IPC_RPC_GET_TDD_EINT_NUM_OP) {
				if((get_num = get_td_eint_info(name, length)) < 0) {
					get_num = FS_FUNC_FAIL;
				}
			}else if(p_rpc_buf->op_id == IPC_RPC_GET_TDD_GPIO_NUM_OP) {
				if((get_num = get_md_gpio_info(name, length)) < 0)	{
					get_num = FS_FUNC_FAIL;
				}
			}
			else if(p_rpc_buf->op_id == IPC_RPC_GET_TDD_ADC_NUM_OP) {
					if((get_num = get_md_adc_info(name, length)) < 0)	{
						get_num = FS_FUNC_FAIL;
					}
				}
	
			CCCI_MSG_INF("rpc", "[0x%08X]: name:%s, len=%d, get_num:%d\n",p_rpc_buf->op_id,
				name, length, get_num);	
			pkt_num = 0;

			/* NOTE: tmp_data[1] not [0] */
			tmp_data[1] = (unsigned int)get_num;	// get_num may be invalid after exit this function
			pkt[pkt_num].len = sizeof(unsigned int);
			pkt[pkt_num++].buf = (void*)(&tmp_data[1]);	//get_num);
			pkt[pkt_num].len = sizeof(unsigned int);
			pkt[pkt_num++].buf = (void*)(&tmp_data[1]);	//get_num);
			kfree(name);
		}
		break;
   
        		err1:
			pkt_num = 0;
			pkt[pkt_num].len = sizeof(unsigned int);
			pkt[pkt_num++].buf = (void*) &tmp_data[0];
			pkt[pkt_num].len = sizeof(unsigned int);
			pkt[pkt_num++].buf = (void*) &tmp_data[0];
			break;
                }

	case IPC_RPC_GET_EMI_CLK_TYPE_OP:
	{
		int dram_type = 0;
		int dram_clk = 0;
	
		if(pkt_num != 0) {
			CCCI_MSG_INF("rpc", "invalid parameter for [0x%X]: pkt_num=%d!\n", 
                                p_rpc_buf->op_id, pkt_num);
			tmp_data[0] = FS_PARAM_ERROR;
			goto err2;
		}

		if(get_dram_type_clk(&dram_clk, &dram_type)) {
			tmp_data[0] = FS_FUNC_FAIL;
			goto err2;
		}
		else {
			tmp_data[0] = 0;
			CCCI_MSG_INF("rpc", "[0x%08X]: dram_clk: %d, dram_type:%d \n",
				p_rpc_buf->op_id, dram_clk, dram_type);	
		}
	
		tmp_data[1] = (unsigned int)dram_type;
		tmp_data[2] = (unsigned int)dram_clk;
		
		pkt_num = 0;
		pkt[pkt_num].len = sizeof(unsigned int);
		pkt[pkt_num++].buf = (void*)(&tmp_data[0]);	
		pkt[pkt_num].len = sizeof(unsigned int);
		pkt[pkt_num++].buf = (void*)(&tmp_data[1]);	
		pkt[pkt_num].len = sizeof(unsigned int);
		pkt[pkt_num++].buf = (void*)(&tmp_data[2]);	
		break;
		
		err2:
			pkt_num = 0;
			pkt[pkt_num].len = sizeof(unsigned int);
			pkt[pkt_num++].buf = (void*) &tmp_data[0];
			pkt[pkt_num].len = sizeof(unsigned int);
			pkt[pkt_num++].buf = (void*) &tmp_data[0];
			pkt[pkt_num].len = sizeof(unsigned int);
			pkt[pkt_num++].buf = (void*) &tmp_data[0];
			break;
        }
	default:
		CCCI_MSG_INF("rpc", "Unknow Operation ID (0x%x)\n", p_rpc_buf->op_id);			
		tmp_data[0] = FS_NO_OP;
		pkt_num = 0;
		pkt[pkt_num].len = sizeof(int);
		pkt[pkt_num++].buf = (void*) &tmp_data[0];	

	}
	*p_pkt_num = pkt_num;

	CCCI_RPC_MSG("ccci_rpc_work_helper--\n");
}

int __init ccci_mach_init(void)
{
	return 0;
}

void __exit ccci_mach_exit(void)
{
}

EXPORT_SYMBOL(image_buf);
EXPORT_SYMBOL(ccci_load_firmware);
EXPORT_SYMBOL(is_modem_debug_ver);
EXPORT_SYMBOL(ungate_md);
EXPORT_SYMBOL(gate_md);
EXPORT_SYMBOL(md_img_vir);
EXPORT_SYMBOL(enable_emi_mpu_protection);
EXPORT_SYMBOL(platform_deinit);
EXPORT_SYMBOL(platform_init);
EXPORT_SYMBOL(ccci_md_wdt_notify_register);
EXPORT_SYMBOL(ccci_aed_cb_register);
EXPORT_SYMBOL(ccci_msg_mask);
EXPORT_SYMBOL(ccci_rpc_work_helper);
EXPORT_SYMBOL(platform_set_runtime_data);

module_init(ccci_mach_init);
module_exit(ccci_mach_exit);

MODULE_DESCRIPTION("CCCI Plaform Driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("MTK");
