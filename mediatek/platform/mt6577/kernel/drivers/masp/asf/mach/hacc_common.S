	.arch armv7-a
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 2
	.eabi_attribute 18, 4
	.file	"hacc_common.c"
	.text
.Ltext0:
	.align	2
	.global	hacc_set_clk
	.type	hacc_set_clk, %function
hacc_set_clk:
.LFB148:
	.file 1 "alps/kernel/mediatek/platform/mt6577/kernel/drivers/masp/asf/mach/hacc_common.c"
	.loc 1 50 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL0:
	mov	ip, sp
.LCFI0:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI1:
	sub	fp, ip, #4
.LCFI2:
	.loc 1 51 0
	cmp	r0, #0
	bne	.L5
	.loc 1 54 0
	bl	SST_Secure_DeInit
.LVL1:
	.loc 1 57 0
	mov	r0, #0
	ldmfd	sp, {fp, sp, pc}
.LVL2:
.L5:
	.loc 1 52 0
	bl	SST_Secure_Init
.LVL3:
	.loc 1 57 0
	mov	r0, #0
	ldmfd	sp, {fp, sp, pc}
.LFE148:
	.size	hacc_set_clk, .-hacc_set_clk
	.align	2
	.global	hacc_enable_clk
	.type	hacc_enable_clk, %function
hacc_enable_clk:
.LFB149:
	.loc 1 63 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
.LCFI3:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI4:
	sub	fp, ip, #4
.LCFI5:
.LVL4:
	.loc 1 66 0
	mov	r0, #1
	bl	hacc_set_clk
.LVL5:
	.loc 1 69 0
	ldmfd	sp, {fp, sp, pc}
.LFE149:
	.size	hacc_enable_clk, .-hacc_enable_clk
	.section	.debug_frame,"",%progbits
.Lframe0:
	.4byte	.LECIE0-.LSCIE0
.LSCIE0:
	.4byte	0xffffffff
	.byte	0x1
	.ascii	"\000"
	.uleb128 0x1
	.sleb128 -4
	.byte	0xe
	.byte	0xc
	.uleb128 0xd
	.uleb128 0
	.align	2
.LECIE0:
.LSFDE0:
	.4byte	.LEFDE0-.LASFDE0
.LASFDE0:
	.4byte	.Lframe0
	.4byte	.LFB148
	.4byte	.LFE148-.LFB148
	.byte	0x4
	.4byte	.LCFI0-.LFB148
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI1-.LCFI0
	.byte	0x8e
	.uleb128 0x2
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8b
	.uleb128 0x4
	.byte	0x4
	.4byte	.LCFI2-.LCFI1
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE0:
.LSFDE2:
	.4byte	.LEFDE2-.LASFDE2
.LASFDE2:
	.4byte	.Lframe0
	.4byte	.LFB149
	.4byte	.LFE149-.LFB149
	.byte	0x4
	.4byte	.LCFI3-.LFB149
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI4-.LCFI3
	.byte	0x8e
	.uleb128 0x2
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8b
	.uleb128 0x4
	.byte	0x4
	.4byte	.LCFI5-.LCFI4
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE2:
	.text
.Letext0:
	.file 2 "alps/kernel/arch/arm/include/asm/outercache.h"
	.file 3 "alps/kernel/arch/arm/include/asm/hwcap.h"
	.file 4 "alps/kernel/include/linux/printk.h"
	.file 5 "alps/kernel/include/linux/kernel.h"
	.file 6 "alps/kernel/../mediatek/platform/mt6577/kernel/core/include/mach/mt_typedefs.h"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x20f
	.2byte	0x2
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF29
	.byte	0x1
	.4byte	.LASF30
	.4byte	.LASF31
	.4byte	.Ltext0
	.4byte	.Letext0
	.4byte	.Ldebug_line0
	.4byte	.Ldebug_pubnames0
	.4byte	.Ldebug_pubtypes0
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF0
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF1
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF2
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.4byte	.LASF3
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF4
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.4byte	.LASF5
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.4byte	.LASF6
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.4byte	.LASF7
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.4byte	.LASF8
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.4byte	.LASF9
	.uleb128 0x2
	.byte	0x1
	.byte	0x2
	.4byte	.LASF10
	.uleb128 0x4
	.byte	0x4
	.4byte	0x87
	.uleb128 0x5
	.byte	0x1
	.uleb128 0x6
	.4byte	.LASF32
	.byte	0x28
	.byte	0x2
	.byte	0x1a
	.4byte	0x122
	.uleb128 0x7
	.4byte	.LASF11
	.byte	0x2
	.byte	0x1b
	.4byte	0x133
	.byte	0x2
	.byte	0x23
	.uleb128 0
	.uleb128 0x7
	.4byte	.LASF12
	.byte	0x2
	.byte	0x1c
	.4byte	0x133
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0x7
	.4byte	.LASF13
	.byte	0x2
	.byte	0x1d
	.4byte	0x133
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x7
	.4byte	.LASF14
	.byte	0x2
	.byte	0x1e
	.4byte	0x81
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.uleb128 0x7
	.4byte	.LASF15
	.byte	0x2
	.byte	0x1f
	.4byte	0x81
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x7
	.4byte	.LASF16
	.byte	0x2
	.byte	0x20
	.4byte	0x81
	.byte	0x2
	.byte	0x23
	.uleb128 0x14
	.uleb128 0x7
	.4byte	.LASF17
	.byte	0x2
	.byte	0x21
	.4byte	0x81
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x7
	.4byte	.LASF18
	.byte	0x2
	.byte	0x23
	.4byte	0x81
	.byte	0x2
	.byte	0x23
	.uleb128 0x1c
	.uleb128 0x7
	.4byte	.LASF19
	.byte	0x2
	.byte	0x25
	.4byte	0x145
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x7
	.4byte	.LASF20
	.byte	0x2
	.byte	0x26
	.4byte	0x81
	.byte	0x2
	.byte	0x23
	.uleb128 0x24
	.byte	0
	.uleb128 0x8
	.byte	0x1
	.4byte	0x133
	.uleb128 0x9
	.4byte	0x2d
	.uleb128 0x9
	.4byte	0x2d
	.byte	0
	.uleb128 0x4
	.byte	0x4
	.4byte	0x122
	.uleb128 0x8
	.byte	0x1
	.4byte	0x145
	.uleb128 0x9
	.4byte	0x2d
	.byte	0
	.uleb128 0x4
	.byte	0x4
	.4byte	0x139
	.uleb128 0xa
	.ascii	"U32\000"
	.byte	0x6
	.byte	0x51
	.4byte	0x34
	.uleb128 0x2
	.byte	0x4
	.byte	0x4
	.4byte	.LASF21
	.uleb128 0xb
	.4byte	.LASF22
	.byte	0x6
	.byte	0x8b
	.4byte	0x49
	.uleb128 0xc
	.byte	0x1
	.4byte	.LASF23
	.byte	0x1
	.byte	0x31
	.byte	0x1
	.4byte	0x14b
	.4byte	.LFB148
	.4byte	.LFE148
	.4byte	.LLST0
	.4byte	0x195
	.uleb128 0xd
	.4byte	.LASF33
	.byte	0x1
	.byte	0x31
	.4byte	0x15d
	.4byte	.LLST1
	.byte	0
	.uleb128 0xc
	.byte	0x1
	.4byte	.LASF24
	.byte	0x1
	.byte	0x3e
	.byte	0x1
	.4byte	0x14b
	.4byte	.LFB149
	.4byte	.LFE149
	.4byte	.LLST2
	.4byte	0x1c2
	.uleb128 0xe
	.ascii	"ret\000"
	.byte	0x1
	.byte	0x40
	.4byte	0x14b
	.4byte	.LLST3
	.byte	0
	.uleb128 0xf
	.4byte	.LASF25
	.byte	0x3
	.byte	0x23
	.4byte	0x34
	.byte	0x1
	.byte	0x1
	.uleb128 0x10
	.4byte	0x5e
	.4byte	0x1da
	.uleb128 0x11
	.byte	0
	.uleb128 0xf
	.4byte	.LASF26
	.byte	0x4
	.byte	0x1b
	.4byte	0x1cf
	.byte	0x1
	.byte	0x1
	.uleb128 0x10
	.4byte	0x3b
	.4byte	0x1f2
	.uleb128 0x11
	.byte	0
	.uleb128 0x12
	.4byte	.LASF27
	.byte	0x5
	.2byte	0x18a
	.4byte	0x200
	.byte	0x1
	.byte	0x1
	.uleb128 0x13
	.4byte	0x1e7
	.uleb128 0xf
	.4byte	.LASF28
	.byte	0x2
	.byte	0x2b
	.4byte	0x89
	.byte	0x1
	.byte	0x1
	.byte	0
	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x6
	.uleb128 0x2134
	.uleb128 0x6
	.uleb128 0x2135
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x15
	.byte	0
	.uleb128 0x27
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",%progbits
.Ldebug_loc0:
.LLST0:
	.4byte	.LFB148-.Ltext0
	.4byte	.LCFI0-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI0-.Ltext0
	.4byte	.LCFI2-.Ltext0
	.2byte	0x2
	.byte	0x7c
	.sleb128 0
	.4byte	.LCFI2-.Ltext0
	.4byte	.LFE148-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST1:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL1-1-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL2-.Ltext0
	.4byte	.LVL3-1-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST2:
	.4byte	.LFB149-.Ltext0
	.4byte	.LCFI3-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI3-.Ltext0
	.4byte	.LCFI5-.Ltext0
	.2byte	0x2
	.byte	0x7c
	.sleb128 0
	.4byte	.LCFI5-.Ltext0
	.4byte	.LFE149-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST3:
	.4byte	.LVL4-.Ltext0
	.4byte	.LVL5-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL5-.Ltext0
	.4byte	.LFE149-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
	.section	.debug_pubnames,"",%progbits
.Ldebug_pubnames0:
	.4byte	0x33
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x213
	.4byte	0x168
	.ascii	"hacc_set_clk\000"
	.4byte	0x195
	.ascii	"hacc_enable_clk\000"
	.4byte	0
	.section	.debug_pubtypes,"",%progbits
.Ldebug_pubtypes0:
	.4byte	0x100
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x213
	.4byte	0x2d
	.ascii	"long unsigned int\000"
	.4byte	0x34
	.ascii	"unsigned int\000"
	.4byte	0x3b
	.ascii	"char\000"
	.4byte	0x42
	.ascii	"signed char\000"
	.4byte	0x49
	.ascii	"unsigned char\000"
	.4byte	0x50
	.ascii	"short int\000"
	.4byte	0x57
	.ascii	"short unsigned int\000"
	.4byte	0x5e
	.ascii	"int\000"
	.4byte	0x65
	.ascii	"long long int\000"
	.4byte	0x6c
	.ascii	"long long unsigned int\000"
	.4byte	0x73
	.ascii	"long int\000"
	.4byte	0x7a
	.ascii	"_Bool\000"
	.4byte	0x89
	.ascii	"outer_cache_fns\000"
	.4byte	0x14b
	.ascii	"U32\000"
	.4byte	0x156
	.ascii	"float\000"
	.4byte	0x15d
	.ascii	"BOOL\000"
	.4byte	0
	.section	.debug_aranges,"",%progbits
	.4byte	0x1c
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x4
	.byte	0
	.2byte	0
	.2byte	0
	.4byte	.Ltext0
	.4byte	.Letext0-.Ltext0
	.4byte	0
	.4byte	0
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.section	.debug_str,"MS",%progbits,1
.LASF20:
	.ascii	"resume\000"
.LASF22:
	.ascii	"BOOL\000"
.LASF33:
	.ascii	"enable\000"
.LASF11:
	.ascii	"inv_range\000"
.LASF13:
	.ascii	"flush_range\000"
.LASF21:
	.ascii	"float\000"
.LASF18:
	.ascii	"sync\000"
.LASF4:
	.ascii	"unsigned char\000"
.LASF0:
	.ascii	"long unsigned int\000"
.LASF6:
	.ascii	"short unsigned int\000"
.LASF29:
	.ascii	"GNU C 4.6.x-google 20120106 (prerelease)\000"
.LASF30:
	.ascii	""
	.ascii	"alps/kernel/mediatek/plat"
	.ascii	"form/mt6577/kernel/drivers/masp/asf/mach/hacc_commo"
	.ascii	"n.c\000"
.LASF14:
	.ascii	"flush_all\000"
.LASF25:
	.ascii	"elf_hwcap\000"
.LASF32:
	.ascii	"outer_cache_fns\000"
.LASF28:
	.ascii	"outer_cache\000"
.LASF1:
	.ascii	"unsigned int\000"
.LASF24:
	.ascii	"hacc_enable_clk\000"
.LASF8:
	.ascii	"long long unsigned int\000"
.LASF16:
	.ascii	"inv_all\000"
.LASF23:
	.ascii	"hacc_set_clk\000"
.LASF26:
	.ascii	"console_printk\000"
.LASF7:
	.ascii	"long long int\000"
.LASF2:
	.ascii	"char\000"
.LASF19:
	.ascii	"set_debug\000"
.LASF15:
	.ascii	"clean_all\000"
.LASF5:
	.ascii	"short int\000"
.LASF27:
	.ascii	"hex_asc\000"
.LASF9:
	.ascii	"long int\000"
.LASF31:
	.ascii	""
	.ascii	"alps/kernel/out\000"
.LASF3:
	.ascii	"signed char\000"
.LASF12:
	.ascii	"clean_range\000"
.LASF17:
	.ascii	"disable\000"
.LASF10:
	.ascii	"_Bool\000"
	.ident	"GCC: (GNU) 4.6.x-google 20120106 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
