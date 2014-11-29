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
	.file	"hacc_lib.c"
	.text
.Ltext0:
	.align	2
	.global	SST_Secure_Algo
	.type	SST_Secure_Algo, %function
SST_Secure_Algo:
.LFB148:
	.file 1 "alps/kernel/mediatek/platform/mt6577/kernel/drivers/masp/asf/mach/hacc_lib.c"
	.loc 1 74 0
	@ args = 4, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL0:
	mov	ip, sp
.LCFI0:
	stmfd	sp!, {r4, r5, r6, r7, fp, ip, lr, pc}
.LCFI1:
	sub	fp, ip, #4
.LCFI2:
	sub	sp, sp, #8
.LCFI3:
.LVL1:
	.loc 1 74 0
	mov	r6, r0
	mov	r7, r1
	.loc 1 81 0
	mov	r0, r3
.LVL2:
	mov	r1, #16
.LVL3:
	.loc 1 74 0
	mov	r4, r2
	ldr	r5, [fp, #4]
	.loc 1 81 0
	bl	sp_hacc_init
.LVL4:
	.loc 1 83 0
	subs	r2, r0, #0
	bne	.L2
.LVL5:
	.loc 1 93 0
	cmp	r6, #0
	bne	.L12
	.loc 1 106 0
	mov	ip, #1
	mov	r0, r7
.LVL6:
	mov	r1, r4
	mov	r2, ip
.LVL7:
	mov	r3, #2
	str	ip, [sp, #0]
	bl	sp_hacc_dec
.LVL8:
.L5:
	.loc 1 116 0 discriminator 1
	cmp	r4, #0
	beq	.L1
	mov	r3, #0
.LVL9:
.L7:
	.loc 1 118 0
	ldrb	r2, [r0, r3]	@ zero_extendqisi2
	strb	r2, [r5, r3]
	.loc 1 116 0 discriminator 2
	add	r3, r3, #1
.LVL10:
	.loc 1 116 0 is_stmt 0 discriminator 1
	cmp	r3, r4
	bne	.L7
.LVL11:
.L1:
	.loc 1 128 0 is_stmt 1
	sub	sp, fp, #28
	ldmfd	sp, {r4, r5, r6, r7, fp, sp, pc}
.LVL12:
.L12:
	.loc 1 93 0
	cmp	r6, #1
	.loc 1 110 0
	movne	r2, #24576
	.loc 1 93 0
	beq	.L13
.LVL13:
.L2:
	.loc 1 125 0 discriminator 1
	ldr	r1, .L14
	ldr	r0, .L14+4
	bl	printk
.LVL14:
	.loc 1 126 0 discriminator 1
#APP
@ 126 "alps/kernel/mediatek/platform/mt6577/kernel/drivers/masp/asf/mach/hacc_lib.c" 1
	.word 0xe7f001f2
@ 0 "" 2
.LVL15:
.L13:
	.loc 1 99 0
	mov	r0, r7
.LVL16:
	mov	r1, r4
	mov	r2, r6
	mov	r3, #2
	str	r6, [sp, #0]
	bl	sp_hacc_enc
.LVL17:
	.loc 1 100 0
	b	.L5
.L15:
	.align	2
.L14:
	.word	.LC1
	.word	.LC0
.LFE148:
	.size	SST_Secure_Algo, .-SST_Secure_Algo
	.section	.rodata.str1.4,"aMS",%progbits,1
	.align	2
.LC0:
	.ascii	"[%s] error (0x%x)\012\000"
	.space	1
.LC1:
	.ascii	"CE\000"
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
	.byte	0x87
	.uleb128 0x5
	.byte	0x86
	.uleb128 0x6
	.byte	0x85
	.uleb128 0x7
	.byte	0x84
	.uleb128 0x8
	.byte	0x4
	.4byte	.LCFI2-.LCFI1
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE0:
	.text
.Letext0:
	.file 2 "alps/kernel/arch/arm/include/asm/outercache.h"
	.file 3 "alps/kernel/mediatek/platform/mt6577/kernel/drivers/masp/asf/inc/sec_typedef.h"
	.file 4 "alps/kernel/arch/arm/include/asm/hwcap.h"
	.file 5 "alps/kernel/include/linux/printk.h"
	.file 6 "alps/kernel/include/linux/kernel.h"
	.file 7 "alps/kernel/mediatek/platform/mt6577/kernel/drivers/masp/asf/inc/hacc_export.h"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x28e
	.2byte	0x2
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF37
	.byte	0x1
	.4byte	.LASF38
	.4byte	.LASF39
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
	.4byte	.LASF40
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
	.uleb128 0x2
	.byte	0x4
	.byte	0x4
	.4byte	.LASF21
	.uleb128 0xa
	.byte	0x4
	.byte	0x7
	.byte	0x2a
	.4byte	0x16d
	.uleb128 0xb
	.4byte	.LASF22
	.sleb128 0
	.uleb128 0xb
	.4byte	.LASF23
	.sleb128 1
	.uleb128 0xb
	.4byte	.LASF24
	.sleb128 2
	.byte	0
	.uleb128 0xc
	.4byte	.LASF25
	.byte	0x3
	.byte	0x32
	.4byte	0x34
	.uleb128 0xc
	.4byte	.LASF26
	.byte	0x3
	.byte	0x33
	.4byte	0x49
	.uleb128 0xd
	.byte	0x1
	.4byte	.LASF41
	.byte	0x1
	.byte	0x49
	.byte	0x1
	.4byte	.LFB148
	.4byte	.LFE148
	.4byte	.LLST0
	.4byte	0x23b
	.uleb128 0xe
	.4byte	.LASF27
	.byte	0x1
	.byte	0x49
	.4byte	0x178
	.4byte	.LLST1
	.uleb128 0xe
	.4byte	.LASF28
	.byte	0x1
	.byte	0x49
	.4byte	0x16d
	.4byte	.LLST2
	.uleb128 0xe
	.4byte	.LASF29
	.byte	0x1
	.byte	0x49
	.4byte	0x16d
	.4byte	.LLST3
	.uleb128 0xe
	.4byte	.LASF30
	.byte	0x1
	.byte	0x49
	.4byte	0x23b
	.4byte	.LLST4
	.uleb128 0xf
	.4byte	.LASF31
	.byte	0x1
	.byte	0x49
	.4byte	0x23b
	.byte	0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x10
	.ascii	"err\000"
	.byte	0x1
	.byte	0x4b
	.4byte	0x16d
	.4byte	.LLST5
	.uleb128 0x10
	.ascii	"src\000"
	.byte	0x1
	.byte	0x4c
	.4byte	0x23b
	.4byte	.LLST6
	.uleb128 0x10
	.ascii	"dst\000"
	.byte	0x1
	.byte	0x4c
	.4byte	0x23b
	.4byte	.LLST7
	.uleb128 0x11
	.4byte	.LASF32
	.byte	0x1
	.byte	0x4c
	.4byte	0x23b
	.4byte	.LLST8
	.uleb128 0x10
	.ascii	"i\000"
	.byte	0x1
	.byte	0x4d
	.4byte	0x16d
	.4byte	.LLST9
	.uleb128 0x12
	.4byte	.LASF42
	.byte	0x1
	.byte	0x7b
	.4byte	.L2
	.byte	0
	.uleb128 0x4
	.byte	0x4
	.4byte	0x178
	.uleb128 0x13
	.4byte	.LASF33
	.byte	0x4
	.byte	0x23
	.4byte	0x34
	.byte	0x1
	.byte	0x1
	.uleb128 0x14
	.4byte	0x5e
	.4byte	0x259
	.uleb128 0x15
	.byte	0
	.uleb128 0x13
	.4byte	.LASF34
	.byte	0x5
	.byte	0x1b
	.4byte	0x24e
	.byte	0x1
	.byte	0x1
	.uleb128 0x14
	.4byte	0x3b
	.4byte	0x271
	.uleb128 0x15
	.byte	0
	.uleb128 0x16
	.4byte	.LASF35
	.byte	0x6
	.2byte	0x18a
	.4byte	0x27f
	.byte	0x1
	.byte	0x1
	.uleb128 0x17
	.4byte	0x266
	.uleb128 0x13
	.4byte	.LASF36
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
	.uleb128 0x4
	.byte	0x1
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
	.uleb128 0xb
	.uleb128 0x28
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1c
	.uleb128 0xd
	.byte	0
	.byte	0
	.uleb128 0xc
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
	.uleb128 0xd
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
	.uleb128 0xe
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
	.uleb128 0xf
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
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x10
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
	.uleb128 0x11
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
	.uleb128 0x2
	.uleb128 0x6
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0xa
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x11
	.uleb128 0x1
	.byte	0
	.byte	0
	.uleb128 0x13
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
	.uleb128 0x14
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x16
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
	.uleb128 0x17
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
	.4byte	.LVL2-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST2:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL3-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL3-.Ltext0
	.4byte	.LFE148-.Ltext0
	.2byte	0x1
	.byte	0x57
	.4byte	0
	.4byte	0
.LLST3:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL4-1-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL4-1-.Ltext0
	.4byte	.LFE148-.Ltext0
	.2byte	0x1
	.byte	0x54
	.4byte	0
	.4byte	0
.LLST4:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL4-1-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	0
	.4byte	0
.LLST5:
	.4byte	.LVL4-.Ltext0
	.4byte	.LVL6-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL6-.Ltext0
	.4byte	.LVL7-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL12-.Ltext0
	.4byte	.LVL13-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL13-.Ltext0
	.4byte	.LVL14-1-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL15-.Ltext0
	.4byte	.LVL16-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST6:
	.4byte	.LVL5-.Ltext0
	.4byte	.LVL13-.Ltext0
	.2byte	0x1
	.byte	0x57
	.4byte	.LVL15-.Ltext0
	.4byte	.LFE148-.Ltext0
	.2byte	0x1
	.byte	0x57
	.4byte	0
	.4byte	0
.LLST7:
	.4byte	.LVL5-.Ltext0
	.4byte	.LVL8-.Ltext0
	.2byte	0x2
	.byte	0x91
	.sleb128 0
	.4byte	.LVL8-.Ltext0
	.4byte	.LVL12-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL12-.Ltext0
	.4byte	.LVL13-.Ltext0
	.2byte	0x2
	.byte	0x91
	.sleb128 0
	.4byte	.LVL15-.Ltext0
	.4byte	.LVL17-.Ltext0
	.2byte	0x2
	.byte	0x91
	.sleb128 0
	.4byte	.LVL17-.Ltext0
	.4byte	.LFE148-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST8:
	.4byte	.LVL1-.Ltext0
	.4byte	.LVL4-1-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	0
	.4byte	0
.LLST9:
	.4byte	.LVL1-.Ltext0
	.4byte	.LVL9-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL10-.Ltext0
	.4byte	.LVL11-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL12-.Ltext0
	.4byte	.LFE148-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	0
	.4byte	0
	.section	.debug_pubnames,"",%progbits
.Ldebug_pubnames0:
	.4byte	0x22
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x292
	.4byte	0x183
	.ascii	"SST_Secure_Algo\000"
	.4byte	0
	.section	.debug_pubtypes,"",%progbits
.Ldebug_pubtypes0:
	.4byte	0x104
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x292
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
	.ascii	"float\000"
	.4byte	0x16d
	.ascii	"uint32\000"
	.4byte	0x178
	.ascii	"uchar\000"
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
.LASF35:
	.ascii	"hex_asc\000"
.LASF40:
	.ascii	"outer_cache_fns\000"
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
.LASF30:
	.ascii	"CustomSeed\000"
.LASF0:
	.ascii	"long unsigned int\000"
.LASF6:
	.ascii	"short unsigned int\000"
.LASF37:
	.ascii	"GNU C 4.6.x-google 20120106 (prerelease)\000"
.LASF26:
	.ascii	"uchar\000"
.LASF31:
	.ascii	"ResText\000"
.LASF14:
	.ascii	"flush_all\000"
.LASF33:
	.ascii	"elf_hwcap\000"
.LASF36:
	.ascii	"outer_cache\000"
.LASF29:
	.ascii	"ContentLen\000"
.LASF25:
	.ascii	"uint32\000"
.LASF1:
	.ascii	"unsigned int\000"
.LASF41:
	.ascii	"SST_Secure_Algo\000"
.LASF8:
	.ascii	"long long unsigned int\000"
.LASF16:
	.ascii	"inv_all\000"
.LASF34:
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
.LASF22:
	.ascii	"HACC_USER1\000"
.LASF32:
	.ascii	"seed\000"
.LASF27:
	.ascii	"Direction\000"
.LASF9:
	.ascii	"long int\000"
.LASF28:
	.ascii	"ContentAddr\000"
.LASF39:
	.ascii	""
	.ascii	"alps/kernel/out\000"
.LASF3:
	.ascii	"signed char\000"
.LASF12:
	.ascii	"clean_range\000"
.LASF17:
	.ascii	"disable\000"
.LASF23:
	.ascii	"HACC_USER2\000"
.LASF24:
	.ascii	"HACC_USER3\000"
.LASF10:
	.ascii	"_Bool\000"
.LASF42:
	.ascii	"_error\000"
.LASF38:
	.ascii	""
	.ascii	"alps/kernel/mediatek/plat"
	.ascii	"form/mt6577/kernel/drivers/masp/asf/mach/hacc_lib.c"
	.ascii	"\000"
	.ident	"GCC: (GNU) 4.6.x-google 20120106 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
