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
	.file	"hacc_export.c"
	.text
.Ltext0:
	.align	2
	.type	sp_hacc_internal, %function
sp_hacc_internal:
.LFB183:
	.file 1 "alps/kernel/mediatek/platform/mt6577/kernel/drivers/masp/asf/mach/hacc_export.c"
	.loc 1 74 0
	@ args = 12, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL0:
	mov	ip, sp
.LCFI0:
	stmfd	sp!, {r4, r5, r6, r7, r8, fp, ip, lr, pc}
.LCFI1:
	sub	fp, ip, #4
.LCFI2:
	sub	sp, sp, #12
.LCFI3:
.LVL1:
	.loc 1 74 0
	ldrb	r5, [fp, #4]	@ zero_extendqisi2
	mov	r4, r0
	mov	r6, r1
	ldr	r7, [fp, #8]
	.loc 1 80 0
	cmp	r5, #1
	.loc 1 74 0
	ldrb	r8, [fp, #12]	@ zero_extendqisi2
	.loc 1 80 0
	beq	.L18
.LVL2:
.L2:
	.loc 1 95 0
	cmp	r2, #0
	bne	.L4
	.loc 1 100 0
	ldr	r3, .L22
	ldrb	r3, [r3, #0]	@ zero_extendqisi2
	cmp	r3, #0
	beq	.L11
	.loc 1 106 0
	mov	r0, r2
	mov	r1, #32
	bl	hacc_set_key
.LVL3:
	.loc 1 108 0
	subs	r2, r0, #0
	bne	.L3
.L15:
	.loc 1 158 0
	mov	r2, r4
	mov	r0, r7
.LVL4:
	mov	r1, r4
	bic	r3, r6, #127
	bl	hacc_do_aes
.LVL5:
	.loc 1 160 0
	subs	r2, r0, #0
	bne	.L3
	.loc 1 173 0
	cmp	r5, #1
	beq	.L19
.LVL6:
.L9:
	.loc 1 185 0
	mov	r0, r4
	sub	sp, fp, #32
	ldmfd	sp, {r4, r5, r6, r7, r8, fp, sp, pc}
.L4:
	.loc 1 118 0
	cmp	r3, #1
	beq	.L7
	bcs	.L20
	.loc 1 124 0
	adds	r0, r8, #0
	ldr	r1, .L22+4
	movne	r0, #1
.L16:
	.loc 1 135 0
	bl	HACC_V3_Init
	.loc 1 137 0
	mov	r0, r4
	mov	r1, r6
	mov	r2, r4
	bl	HACC_V3_Run
	.loc 1 139 0
	bl	HACC_V3_Terminate
	.loc 1 173 0
	cmp	r5, #1
	bne	.L9
.L19:
	.loc 1 175 0
	bl	osal_hacc_unlock
	b	.L9
.L20:
	.loc 1 118 0
	cmp	r3, #2
	.loc 1 165 0
	movwne	r2, #24588
	.loc 1 118 0
	beq	.L21
.L3:
.LVL7:
	.loc 1 181 0 discriminator 1
	ldr	r1, .L22+8
	ldr	r0, .L22+12
	bl	printk
.LVL8:
	.loc 1 182 0 discriminator 1
#APP
@ 182 "alps/kernel/mediatek/platform/mt6577/kernel/drivers/masp/asf/mach/hacc_export.c" 1
	.word 0xe7f001f2
@ 0 "" 2
.L7:
	.loc 1 135 0
	adds	r0, r8, #0
	ldr	r1, .L22+16
	movne	r0, #1
	b	.L16
.LVL9:
.L18:
	.loc 1 83 0
	str	r2, [fp, #-40]
	str	r3, [fp, #-44]
	bl	osal_hacc_lock
.LVL10:
	.loc 1 85 0
	ldr	r2, [fp, #-40]
	ldr	r3, [fp, #-44]
	cmp	r0, #0
	beq	.L2
	.loc 1 87 0
	movw	r2, #24586
	b	.L3
.LVL11:
.L21:
	.loc 1 146 0
	ldr	r2, .L22
	ldrb	r2, [r2, #1]	@ zero_extendqisi2
	cmp	r2, #0
	beq	.L13
	.loc 1 153 0
	mov	r0, r3
	mov	r1, #32
	bl	hacc_set_key
.LVL12:
	.loc 1 155 0
	subs	r2, r0, #0
	beq	.L15
	b	.L3
.LVL13:
.L11:
	.loc 1 102 0
	movw	r2, #24584
	b	.L3
.L13:
	.loc 1 148 0
	movw	r2, #24583
	b	.L3
.L23:
	.align	2
.L22:
	.word	.LANCHOR0
	.word	.LANCHOR1
	.word	.LC1
	.word	.LC0
	.word	.LANCHOR1+32
.LFE183:
	.size	sp_hacc_internal, .-sp_hacc_internal
	.align	2
	.global	sp_hacc_enc
	.type	sp_hacc_enc, %function
sp_hacc_enc:
.LFB184:
	.loc 1 191 0
	@ args = 4, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL14:
	mov	ip, sp
.LCFI4:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI5:
	sub	fp, ip, #4
.LCFI6:
	sub	sp, sp, #16
.LCFI7:
	.loc 1 192 0
	mov	ip, #1
	str	ip, [sp, #4]
	str	ip, [sp, #8]
	ldrb	ip, [fp, #4]	@ zero_extendqisi2
	str	ip, [sp, #0]
	bl	sp_hacc_internal
.LVL15:
	.loc 1 193 0
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
.LFE184:
	.size	sp_hacc_enc, .-sp_hacc_enc
	.align	2
	.global	sp_hacc_dec
	.type	sp_hacc_dec, %function
sp_hacc_dec:
.LFB185:
	.loc 1 200 0
	@ args = 4, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL16:
	mov	ip, sp
.LCFI8:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI9:
	sub	fp, ip, #4
.LCFI10:
	sub	sp, sp, #16
.LCFI11:
	.loc 1 201 0
	mov	ip, #0
	str	ip, [sp, #4]
	str	ip, [sp, #8]
	ldrb	ip, [fp, #4]	@ zero_extendqisi2
	str	ip, [sp, #0]
	bl	sp_hacc_internal
.LVL17:
	.loc 1 202 0
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
.LFE185:
	.size	sp_hacc_dec, .-sp_hacc_dec
	.align	2
	.global	sp_hacc_blk_sz
	.type	sp_hacc_blk_sz, %function
sp_hacc_blk_sz:
.LFB186:
	.loc 1 208 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
.LCFI12:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI13:
	sub	fp, ip, #4
.LCFI14:
	.loc 1 210 0
	mov	r0, #16
	ldmfd	sp, {fp, sp, pc}
.LFE186:
	.size	sp_hacc_blk_sz, .-sp_hacc_blk_sz
	.align	2
	.global	sp_hacc_init
	.type	sp_hacc_init, %function
sp_hacc_init:
.LFB187:
	.loc 1 216 0
	@ args = 0, pretend = 0, frame = 40
	@ frame_needed = 1, uses_anonymous_args = 0
.LVL18:
	mov	ip, sp
.LCFI15:
	stmfd	sp!, {fp, ip, lr, pc}
.LCFI16:
	sub	fp, ip, #4
.LCFI17:
	sub	sp, sp, #48
.LCFI18:
.LVL19:
	.loc 1 220 0
	cmp	r1, #16
	.loc 1 222 0
	movwne	r0, #24580
.LVL20:
	.loc 1 220 0
	beq	.L32
.LVL21:
.L28:
	.loc 1 235 0
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
.LVL22:
.L32:
	.loc 1 225 0
	sub	r1, fp, #12
.LVL23:
	.loc 1 226 0
	mov	r3, #0
	.loc 1 225 0
	mov	r2, #32
	str	r2, [fp, #-48]
.LVL24:
.L29:
	.loc 1 215 0
	sub	ip, fp, #48
	.loc 1 228 0
	ldrb	r2, [r0, r3]	@ zero_extendqisi2
	.loc 1 215 0
	add	lr, ip, r3
	.loc 1 226 0 discriminator 2
	add	r3, r3, #1
.LVL25:
	.loc 1 226 0 is_stmt 0 discriminator 1
	cmp	r3, #16
	.loc 1 229 0 is_stmt 1
	add	ip, r2, #1
	.loc 1 228 0
	strb	r2, [lr, #4]
	.loc 1 229 0
	strb	ip, [r1, #-1]!
	.loc 1 226 0 discriminator 1
	bne	.L29
	.loc 1 232 0 discriminator 1
	ldrb	ip, [fp, #-41]	@ zero_extendqisi2
	ldrb	r1, [fp, #-44]	@ zero_extendqisi2
	ldrb	r2, [fp, #-43]	@ zero_extendqisi2
	ldrb	r3, [fp, #-42]	@ zero_extendqisi2
.LVL26:
	ldr	r0, .L33
	str	ip, [sp, #0]
	bl	printk
	.loc 1 234 0
	sub	r0, fp, #48
	bl	hacc_init
	b	.L28
.L34:
	.align	2
.L33:
	.word	.LC2
.LFE187:
	.size	sp_hacc_init, .-sp_hacc_init
	.global	bHACC_SWKeyInit
	.global	bHACC_HWWrapKeyInit
	.section	.rodata
	.align	2
.LANCHOR1 = . + 0
	.type	g_HACC_CFG_1, %object
	.size	g_HACC_CFG_1, 32
g_HACC_CFG_1:
	.word	-1630272512
	.word	15238305
	.word	-470776899
	.word	793669002
	.word	-8155556
	.word	-381640477
	.word	-1924272954
	.word	1174145417
	.type	g_HACC_CFG_2, %object
	.size	g_HACC_CFG_2, 32
g_HACC_CFG_2:
	.word	-1437324070
	.word	1431445780
	.word	-470776899
	.word	1431445780
	.word	-1437324070
	.word	-1437324070
	.word	1431445780
	.word	-1437324070
	.section	.rodata.str1.4,"aMS",%progbits,1
	.align	2
.LC0:
	.ascii	"[%s] Fail (0x%x)\012\000"
	.space	2
.LC1:
	.ascii	"ASF\000"
.LC2:
	.ascii	"0x%x,0x%x,0x%x,0x%x\012\000"
	.bss
.LANCHOR0 = . + 0
	.type	bHACC_SWKeyInit, %object
	.size	bHACC_SWKeyInit, 1
bHACC_SWKeyInit:
	.space	1
	.type	bHACC_HWWrapKeyInit, %object
	.size	bHACC_HWWrapKeyInit, 1
bHACC_HWWrapKeyInit:
	.space	1
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
	.4byte	.LFB183
	.4byte	.LFE183-.LFB183
	.byte	0x4
	.4byte	.LCFI0-.LFB183
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
	.byte	0x88
	.uleb128 0x5
	.byte	0x87
	.uleb128 0x6
	.byte	0x86
	.uleb128 0x7
	.byte	0x85
	.uleb128 0x8
	.byte	0x84
	.uleb128 0x9
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
	.4byte	.LFB184
	.4byte	.LFE184-.LFB184
	.byte	0x4
	.4byte	.LCFI4-.LFB184
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI5-.LCFI4
	.byte	0x8e
	.uleb128 0x2
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8b
	.uleb128 0x4
	.byte	0x4
	.4byte	.LCFI6-.LCFI5
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE2:
.LSFDE4:
	.4byte	.LEFDE4-.LASFDE4
.LASFDE4:
	.4byte	.Lframe0
	.4byte	.LFB185
	.4byte	.LFE185-.LFB185
	.byte	0x4
	.4byte	.LCFI8-.LFB185
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI9-.LCFI8
	.byte	0x8e
	.uleb128 0x2
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8b
	.uleb128 0x4
	.byte	0x4
	.4byte	.LCFI10-.LCFI9
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE4:
.LSFDE6:
	.4byte	.LEFDE6-.LASFDE6
.LASFDE6:
	.4byte	.Lframe0
	.4byte	.LFB186
	.4byte	.LFE186-.LFB186
	.byte	0x4
	.4byte	.LCFI12-.LFB186
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI13-.LCFI12
	.byte	0x8e
	.uleb128 0x2
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8b
	.uleb128 0x4
	.byte	0x4
	.4byte	.LCFI14-.LCFI13
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE6:
.LSFDE8:
	.4byte	.LEFDE8-.LASFDE8
.LASFDE8:
	.4byte	.Lframe0
	.4byte	.LFB187
	.4byte	.LFE187-.LFB187
	.byte	0x4
	.4byte	.LCFI15-.LFB187
	.byte	0xd
	.uleb128 0xc
	.byte	0x4
	.4byte	.LCFI16-.LCFI15
	.byte	0x8e
	.uleb128 0x2
	.byte	0x8d
	.uleb128 0x3
	.byte	0x8b
	.uleb128 0x4
	.byte	0x4
	.4byte	.LCFI17-.LCFI16
	.byte	0xc
	.uleb128 0xb
	.uleb128 0x4
	.align	2
.LEFDE8:
	.text
.Letext0:
	.file 2 "alps/kernel/arch/arm/include/asm/outercache.h"
	.file 3 "alps/kernel/mediatek/platform/mt6577/kernel/drivers/masp/asf/inc/hacc_export.h"
	.file 4 "alps/kernel/mediatek/platform/mt6577/kernel/drivers/masp/asf/inc/sec_typedef.h"
	.file 5 "alps/kernel/../mediatek/platform/mt6577/kernel/core/include/mach/mt_typedefs.h"
	.file 6 "alps/kernel/mediatek/platform/mt6577/kernel/drivers/masp/asf/inc/hacc_def.h"
	.file 7 "alps/kernel/arch/arm/include/asm/hwcap.h"
	.file 8 "alps/kernel/include/linux/printk.h"
	.file 9 "alps/kernel/include/linux/kernel.h"
	.file 10 "alps/kernel/../mediatek/platform/mt6577/kernel/core/include/mach/mt_clock_manager.h"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x511
	.2byte	0x2
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF65
	.byte	0x1
	.4byte	.LASF66
	.4byte	.LASF67
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
	.4byte	.LASF68
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
	.byte	0x4
	.byte	0x3
	.byte	0x2a
	.4byte	0x166
	.uleb128 0xb
	.4byte	.LASF21
	.sleb128 0
	.uleb128 0xb
	.4byte	.LASF22
	.sleb128 1
	.uleb128 0xb
	.4byte	.LASF23
	.sleb128 2
	.byte	0
	.uleb128 0xc
	.4byte	.LASF24
	.byte	0x3
	.byte	0x2e
	.4byte	0x14b
	.uleb128 0xc
	.4byte	.LASF25
	.byte	0x4
	.byte	0x32
	.4byte	0x34
	.uleb128 0xc
	.4byte	.LASF26
	.byte	0x4
	.byte	0x33
	.4byte	0x49
	.uleb128 0xd
	.ascii	"U8\000"
	.byte	0x5
	.byte	0x4d
	.4byte	0x49
	.uleb128 0xd
	.ascii	"U32\000"
	.byte	0x5
	.byte	0x51
	.4byte	0x34
	.uleb128 0x2
	.byte	0x4
	.byte	0x4
	.4byte	.LASF27
	.uleb128 0xc
	.4byte	.LASF28
	.byte	0x5
	.byte	0x8b
	.4byte	0x49
	.uleb128 0xa
	.byte	0x4
	.byte	0x6
	.byte	0x60
	.4byte	0x1c3
	.uleb128 0xb
	.4byte	.LASF29
	.sleb128 0
	.uleb128 0xb
	.4byte	.LASF30
	.sleb128 1
	.byte	0
	.uleb128 0xc
	.4byte	.LASF31
	.byte	0x6
	.byte	0x63
	.4byte	0x1ae
	.uleb128 0xa
	.byte	0x4
	.byte	0x6
	.byte	0x66
	.4byte	0x1e9
	.uleb128 0xb
	.4byte	.LASF32
	.sleb128 16
	.uleb128 0xb
	.4byte	.LASF33
	.sleb128 24
	.uleb128 0xb
	.4byte	.LASF34
	.sleb128 32
	.byte	0
	.uleb128 0xa
	.byte	0x4
	.byte	0x6
	.byte	0x6d
	.4byte	0x204
	.uleb128 0xb
	.4byte	.LASF35
	.sleb128 0
	.uleb128 0xb
	.4byte	.LASF36
	.sleb128 1
	.uleb128 0xb
	.4byte	.LASF37
	.sleb128 2
	.byte	0
	.uleb128 0xe
	.byte	0x24
	.byte	0x6
	.byte	0x77
	.4byte	0x229
	.uleb128 0x7
	.4byte	.LASF38
	.byte	0x6
	.byte	0x78
	.4byte	0x191
	.byte	0x2
	.byte	0x23
	.uleb128 0
	.uleb128 0x7
	.4byte	.LASF39
	.byte	0x6
	.byte	0x79
	.4byte	0x229
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.byte	0
	.uleb128 0xf
	.4byte	0x187
	.4byte	0x239
	.uleb128 0x10
	.4byte	0x34
	.byte	0x1f
	.byte	0
	.uleb128 0xc
	.4byte	.LASF40
	.byte	0x6
	.byte	0x7a
	.4byte	0x204
	.uleb128 0x11
	.4byte	.LASF69
	.byte	0x4
	.byte	0xa
	.byte	0x66
	.4byte	0x281
	.uleb128 0xb
	.4byte	.LASF41
	.sleb128 0
	.uleb128 0xb
	.4byte	.LASF42
	.sleb128 1
	.uleb128 0xb
	.4byte	.LASF43
	.sleb128 2
	.uleb128 0xb
	.4byte	.LASF44
	.sleb128 3
	.uleb128 0xb
	.4byte	.LASF45
	.sleb128 4
	.uleb128 0xb
	.4byte	.LASF46
	.sleb128 5
	.uleb128 0xb
	.4byte	.LASF47
	.sleb128 6
	.uleb128 0xb
	.4byte	.LASF48
	.sleb128 7
	.byte	0
	.uleb128 0x12
	.4byte	.LASF70
	.byte	0x1
	.byte	0x49
	.byte	0x1
	.4byte	0x31e
	.4byte	.LFB183
	.4byte	.LFE183
	.4byte	.LLST0
	.4byte	0x31e
	.uleb128 0x13
	.ascii	"buf\000"
	.byte	0x1
	.byte	0x49
	.4byte	0x31e
	.4byte	.LLST1
	.uleb128 0x14
	.4byte	.LASF38
	.byte	0x1
	.byte	0x49
	.4byte	0x171
	.4byte	.LLST2
	.uleb128 0x13
	.ascii	"bAC\000"
	.byte	0x1
	.byte	0x49
	.4byte	0x1a3
	.4byte	.LLST3
	.uleb128 0x14
	.4byte	.LASF49
	.byte	0x1
	.byte	0x49
	.4byte	0x166
	.4byte	.LLST4
	.uleb128 0x15
	.4byte	.LASF50
	.byte	0x1
	.byte	0x49
	.4byte	0x1a3
	.byte	0x2
	.byte	0x91
	.sleb128 0
	.uleb128 0x15
	.4byte	.LASF51
	.byte	0x1
	.byte	0x49
	.4byte	0x1c3
	.byte	0x2
	.byte	0x91
	.sleb128 4
	.uleb128 0x16
	.ascii	"bEn\000"
	.byte	0x1
	.byte	0x49
	.4byte	0x1a3
	.byte	0x2
	.byte	0x91
	.sleb128 8
	.uleb128 0x17
	.ascii	"err\000"
	.byte	0x1
	.byte	0x4b
	.4byte	0x171
	.4byte	.LLST5
	.uleb128 0x18
	.4byte	.LASF71
	.byte	0x1
	.byte	0xb4
	.4byte	.L3
	.byte	0
	.uleb128 0x4
	.byte	0x4
	.4byte	0x17c
	.uleb128 0x19
	.byte	0x1
	.4byte	.LASF52
	.byte	0x1
	.byte	0xbe
	.byte	0x1
	.4byte	0x31e
	.4byte	.LFB184
	.4byte	.LFE184
	.4byte	.LLST6
	.4byte	0x38c
	.uleb128 0x13
	.ascii	"buf\000"
	.byte	0x1
	.byte	0xbe
	.4byte	0x31e
	.4byte	.LLST7
	.uleb128 0x14
	.4byte	.LASF38
	.byte	0x1
	.byte	0xbe
	.4byte	0x171
	.4byte	.LLST8
	.uleb128 0x13
	.ascii	"bAC\000"
	.byte	0x1
	.byte	0xbe
	.4byte	0x1a3
	.4byte	.LLST9
	.uleb128 0x14
	.4byte	.LASF49
	.byte	0x1
	.byte	0xbe
	.4byte	0x166
	.4byte	.LLST10
	.uleb128 0x15
	.4byte	.LASF50
	.byte	0x1
	.byte	0xbe
	.4byte	0x1a3
	.byte	0x2
	.byte	0x91
	.sleb128 0
	.byte	0
	.uleb128 0x19
	.byte	0x1
	.4byte	.LASF53
	.byte	0x1
	.byte	0xc7
	.byte	0x1
	.4byte	0x31e
	.4byte	.LFB185
	.4byte	.LFE185
	.4byte	.LLST11
	.4byte	0x3f4
	.uleb128 0x13
	.ascii	"buf\000"
	.byte	0x1
	.byte	0xc7
	.4byte	0x31e
	.4byte	.LLST12
	.uleb128 0x14
	.4byte	.LASF38
	.byte	0x1
	.byte	0xc7
	.4byte	0x171
	.4byte	.LLST13
	.uleb128 0x13
	.ascii	"bAC\000"
	.byte	0x1
	.byte	0xc7
	.4byte	0x1a3
	.4byte	.LLST14
	.uleb128 0x14
	.4byte	.LASF49
	.byte	0x1
	.byte	0xc7
	.4byte	0x166
	.4byte	.LLST15
	.uleb128 0x15
	.4byte	.LASF50
	.byte	0x1
	.byte	0xc7
	.4byte	0x1a3
	.byte	0x2
	.byte	0x91
	.sleb128 0
	.byte	0
	.uleb128 0x1a
	.byte	0x1
	.4byte	.LASF72
	.byte	0x1
	.byte	0xcf
	.byte	0x1
	.4byte	0x171
	.4byte	.LFB186
	.4byte	.LFE186
	.4byte	.LLST16
	.uleb128 0x19
	.byte	0x1
	.4byte	.LASF54
	.byte	0x1
	.byte	0xd7
	.byte	0x1
	.4byte	0x171
	.4byte	.LFB187
	.4byte	.LFE187
	.4byte	.LLST17
	.4byte	0x464
	.uleb128 0x14
	.4byte	.LASF55
	.byte	0x1
	.byte	0xd7
	.4byte	0x31e
	.4byte	.LLST18
	.uleb128 0x14
	.4byte	.LASF38
	.byte	0x1
	.byte	0xd7
	.4byte	0x171
	.4byte	.LLST19
	.uleb128 0x1b
	.4byte	.LASF56
	.byte	0x1
	.byte	0xd9
	.4byte	0x239
	.byte	0x2
	.byte	0x91
	.sleb128 -52
	.uleb128 0x17
	.ascii	"i\000"
	.byte	0x1
	.byte	0xda
	.4byte	0x171
	.4byte	.LLST20
	.byte	0
	.uleb128 0xf
	.4byte	0x171
	.4byte	0x474
	.uleb128 0x10
	.4byte	0x34
	.byte	0x7
	.byte	0
	.uleb128 0x1b
	.4byte	.LASF57
	.byte	0x1
	.byte	0x3c
	.4byte	0x485
	.byte	0x5
	.byte	0x3
	.4byte	g_HACC_CFG_1
	.uleb128 0x1c
	.4byte	0x464
	.uleb128 0x1b
	.4byte	.LASF58
	.byte	0x1
	.byte	0x41
	.4byte	0x49b
	.byte	0x5
	.byte	0x3
	.4byte	g_HACC_CFG_2
	.uleb128 0x1c
	.4byte	0x464
	.uleb128 0x1d
	.4byte	.LASF59
	.byte	0x7
	.byte	0x23
	.4byte	0x34
	.byte	0x1
	.byte	0x1
	.uleb128 0xf
	.4byte	0x5e
	.4byte	0x4b8
	.uleb128 0x1e
	.byte	0
	.uleb128 0x1d
	.4byte	.LASF60
	.byte	0x8
	.byte	0x1b
	.4byte	0x4ad
	.byte	0x1
	.byte	0x1
	.uleb128 0xf
	.4byte	0x3b
	.4byte	0x4d0
	.uleb128 0x1e
	.byte	0
	.uleb128 0x1f
	.4byte	.LASF61
	.byte	0x9
	.2byte	0x18a
	.4byte	0x4de
	.byte	0x1
	.byte	0x1
	.uleb128 0x1c
	.4byte	0x4c5
	.uleb128 0x1d
	.4byte	.LASF62
	.byte	0x2
	.byte	0x2b
	.4byte	0x89
	.byte	0x1
	.byte	0x1
	.uleb128 0x20
	.4byte	.LASF63
	.byte	0x1
	.byte	0x36
	.4byte	0x1a3
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.4byte	bHACC_HWWrapKeyInit
	.uleb128 0x20
	.4byte	.LASF64
	.byte	0x1
	.byte	0x37
	.4byte	0x1a3
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.4byte	bHACC_SWKeyInit
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
	.uleb128 0xe
	.uleb128 0x13
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
	.uleb128 0xf
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x4
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
	.uleb128 0x12
	.uleb128 0x2e
	.byte	0x1
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
	.uleb128 0x13
	.uleb128 0x5
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
	.uleb128 0x14
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
	.uleb128 0x15
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
	.uleb128 0x16
	.uleb128 0x5
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
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x17
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
	.uleb128 0x18
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
	.uleb128 0x19
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
	.uleb128 0x1a
	.uleb128 0x2e
	.byte	0
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
	.byte	0
	.byte	0
	.uleb128 0x1b
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
	.uleb128 0xa
	.byte	0
	.byte	0
	.uleb128 0x1c
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1d
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
	.uleb128 0x1e
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x1f
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
	.uleb128 0x20
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
	.uleb128 0x2
	.uleb128 0xa
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",%progbits
.Ldebug_loc0:
.LLST0:
	.4byte	.LFB183-.Ltext0
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
	.4byte	.LFE183-.Ltext0
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
	.4byte	.LVL2-.Ltext0
	.4byte	.LFE183-.Ltext0
	.2byte	0x1
	.byte	0x54
	.4byte	0
	.4byte	0
.LLST2:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL2-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL2-.Ltext0
	.4byte	.LFE183-.Ltext0
	.2byte	0x1
	.byte	0x56
	.4byte	0
	.4byte	0
.LLST3:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL2-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL9-.Ltext0
	.4byte	.LVL10-1-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	0
	.4byte	0
.LLST4:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL2-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL9-.Ltext0
	.4byte	.LVL10-1-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	0
	.4byte	0
.LLST5:
	.4byte	.LVL1-.Ltext0
	.4byte	.LVL2-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL3-.Ltext0
	.4byte	.LVL4-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL5-.Ltext0
	.4byte	.LVL6-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL7-.Ltext0
	.4byte	.LVL8-1-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL9-.Ltext0
	.4byte	.LVL10-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL10-.Ltext0
	.4byte	.LVL11-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL12-.Ltext0
	.4byte	.LVL13-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST6:
	.4byte	.LFB184-.Ltext0
	.4byte	.LCFI4-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI4-.Ltext0
	.4byte	.LCFI6-.Ltext0
	.2byte	0x2
	.byte	0x7c
	.sleb128 0
	.4byte	.LCFI6-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST7:
	.4byte	.LVL14-.Ltext0
	.4byte	.LVL15-1-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST8:
	.4byte	.LVL14-.Ltext0
	.4byte	.LVL15-1-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	0
	.4byte	0
.LLST9:
	.4byte	.LVL14-.Ltext0
	.4byte	.LVL15-1-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	0
	.4byte	0
.LLST10:
	.4byte	.LVL14-.Ltext0
	.4byte	.LVL15-1-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	0
	.4byte	0
.LLST11:
	.4byte	.LFB185-.Ltext0
	.4byte	.LCFI8-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI8-.Ltext0
	.4byte	.LCFI10-.Ltext0
	.2byte	0x2
	.byte	0x7c
	.sleb128 0
	.4byte	.LCFI10-.Ltext0
	.4byte	.LFE185-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST12:
	.4byte	.LVL16-.Ltext0
	.4byte	.LVL17-1-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST13:
	.4byte	.LVL16-.Ltext0
	.4byte	.LVL17-1-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	0
	.4byte	0
.LLST14:
	.4byte	.LVL16-.Ltext0
	.4byte	.LVL17-1-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	0
	.4byte	0
.LLST15:
	.4byte	.LVL16-.Ltext0
	.4byte	.LVL17-1-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	0
	.4byte	0
.LLST16:
	.4byte	.LFB186-.Ltext0
	.4byte	.LCFI12-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI12-.Ltext0
	.4byte	.LCFI14-.Ltext0
	.2byte	0x2
	.byte	0x7c
	.sleb128 0
	.4byte	.LCFI14-.Ltext0
	.4byte	.LFE186-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST17:
	.4byte	.LFB187-.Ltext0
	.4byte	.LCFI15-.Ltext0
	.2byte	0x2
	.byte	0x7d
	.sleb128 0
	.4byte	.LCFI15-.Ltext0
	.4byte	.LCFI17-.Ltext0
	.2byte	0x2
	.byte	0x7c
	.sleb128 0
	.4byte	.LCFI17-.Ltext0
	.4byte	.LFE187-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST18:
	.4byte	.LVL18-.Ltext0
	.4byte	.LVL20-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	0
	.4byte	0
.LLST19:
	.4byte	.LVL18-.Ltext0
	.4byte	.LVL21-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL22-.Ltext0
	.4byte	.LVL23-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	0
	.4byte	0
.LLST20:
	.4byte	.LVL19-.Ltext0
	.4byte	.LVL21-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL22-.Ltext0
	.4byte	.LVL24-.Ltext0
	.2byte	0x2
	.byte	0x30
	.byte	0x9f
	.4byte	.LVL25-.Ltext0
	.4byte	.LVL26-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	0
	.4byte	0
	.section	.debug_pubnames,"",%progbits
.Ldebug_pubnames0:
	.4byte	0xb5
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x515
	.4byte	0x281
	.ascii	"sp_hacc_internal\000"
	.4byte	0x324
	.ascii	"sp_hacc_enc\000"
	.4byte	0x38c
	.ascii	"sp_hacc_dec\000"
	.4byte	0x3f4
	.ascii	"sp_hacc_blk_sz\000"
	.4byte	0x40d
	.ascii	"sp_hacc_init\000"
	.4byte	0x474
	.ascii	"g_HACC_CFG_1\000"
	.4byte	0x48a
	.ascii	"g_HACC_CFG_2\000"
	.4byte	0x4f0
	.ascii	"bHACC_HWWrapKeyInit\000"
	.4byte	0x502
	.ascii	"bHACC_SWKeyInit\000"
	.4byte	0
	.section	.debug_pubtypes,"",%progbits
.Ldebug_pubtypes0:
	.4byte	0x245
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x515
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
	.4byte	0x166
	.ascii	"HACC_USER\000"
	.4byte	0x171
	.ascii	"uint32\000"
	.4byte	0x17c
	.ascii	"uchar\000"
	.4byte	0x187
	.ascii	"U8\000"
	.4byte	0x191
	.ascii	"U32\000"
	.4byte	0x19c
	.ascii	"float\000"
	.4byte	0x1a3
	.ascii	"BOOL\000"
	.4byte	0x1c3
	.ascii	"AES_OPS\000"
	.4byte	0x239
	.ascii	"AES_KEY_SEED\000"
	.4byte	0x244
	.ascii	"mt65xx_clock_category\000"
	.4byte	0x250
	.ascii	"MT65XX_CLOCK_PERI_PDN0\000"
	.4byte	0x256
	.ascii	"MT65XX_CLOCK_PERI_PDN1\000"
	.4byte	0x25c
	.ascii	"MT65XX_CLOCK_MMSYS1_PDN0\000"
	.4byte	0x262
	.ascii	"MT65XX_CLOCK_MMSYS1_PDN1\000"
	.4byte	0x268
	.ascii	"MT65XX_CLOCK_MMSYS1_PDN2\000"
	.4byte	0x26e
	.ascii	"MT65XX_CLOCK_MMSYS2_PDN\000"
	.4byte	0x274
	.ascii	"MT65XX_CLOCK_AUDIO_PDN\000"
	.4byte	0x27a
	.ascii	"MT65XX_CLOCK_CATEGORY_COUNT\000"
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
.LASF28:
	.ascii	"BOOL\000"
.LASF52:
	.ascii	"sp_hacc_enc\000"
.LASF62:
	.ascii	"outer_cache\000"
.LASF5:
	.ascii	"short int\000"
.LASF55:
	.ascii	"sec_seed\000"
.LASF41:
	.ascii	"MT65XX_CLOCK_PERI_PDN0\000"
.LASF42:
	.ascii	"MT65XX_CLOCK_PERI_PDN1\000"
.LASF64:
	.ascii	"bHACC_SWKeyInit\000"
.LASF30:
	.ascii	"AES_ENC\000"
.LASF20:
	.ascii	"resume\000"
.LASF60:
	.ascii	"console_printk\000"
.LASF26:
	.ascii	"uchar\000"
.LASF21:
	.ascii	"HACC_USER1\000"
.LASF66:
	.ascii	""
	.ascii	"alps/kernel/mediatek/plat"
	.ascii	"form/mt6577/kernel/drivers/masp/asf/mach/hacc_expor"
	.ascii	"t.c\000"
.LASF19:
	.ascii	"set_debug\000"
.LASF27:
	.ascii	"float\000"
.LASF47:
	.ascii	"MT65XX_CLOCK_AUDIO_PDN\000"
.LASF7:
	.ascii	"long long int\000"
.LASF34:
	.ascii	"AES_KEY_256\000"
.LASF11:
	.ascii	"inv_range\000"
.LASF2:
	.ascii	"char\000"
.LASF63:
	.ascii	"bHACC_HWWrapKeyInit\000"
.LASF9:
	.ascii	"long int\000"
.LASF59:
	.ascii	"elf_hwcap\000"
.LASF46:
	.ascii	"MT65XX_CLOCK_MMSYS2_PDN\000"
.LASF24:
	.ascii	"HACC_USER\000"
.LASF4:
	.ascii	"unsigned char\000"
.LASF13:
	.ascii	"flush_range\000"
.LASF48:
	.ascii	"MT65XX_CLOCK_CATEGORY_COUNT\000"
.LASF71:
	.ascii	"_err\000"
.LASF33:
	.ascii	"AES_KEY_192\000"
.LASF3:
	.ascii	"signed char\000"
.LASF8:
	.ascii	"long long unsigned int\000"
.LASF35:
	.ascii	"AES_SW_KEY\000"
.LASF68:
	.ascii	"outer_cache_fns\000"
.LASF0:
	.ascii	"long unsigned int\000"
.LASF31:
	.ascii	"AES_OPS\000"
.LASF57:
	.ascii	"g_HACC_CFG_1\000"
.LASF58:
	.ascii	"g_HACC_CFG_2\000"
.LASF49:
	.ascii	"user\000"
.LASF6:
	.ascii	"short unsigned int\000"
.LASF36:
	.ascii	"AES_HW_KEY\000"
.LASF72:
	.ascii	"sp_hacc_blk_sz\000"
.LASF22:
	.ascii	"HACC_USER2\000"
.LASF23:
	.ascii	"HACC_USER3\000"
.LASF39:
	.ascii	"seed\000"
.LASF15:
	.ascii	"clean_all\000"
.LASF32:
	.ascii	"AES_KEY_128\000"
.LASF10:
	.ascii	"_Bool\000"
.LASF67:
	.ascii	""
	.ascii	"alps/kernel/out\000"
.LASF51:
	.ascii	"aes_type\000"
.LASF14:
	.ascii	"flush_all\000"
.LASF40:
	.ascii	"AES_KEY_SEED\000"
.LASF18:
	.ascii	"sync\000"
.LASF12:
	.ascii	"clean_range\000"
.LASF53:
	.ascii	"sp_hacc_dec\000"
.LASF16:
	.ascii	"inv_all\000"
.LASF25:
	.ascii	"uint32\000"
.LASF56:
	.ascii	"keyseed\000"
.LASF38:
	.ascii	"size\000"
.LASF37:
	.ascii	"AES_HW_WRAP_KEY\000"
.LASF61:
	.ascii	"hex_asc\000"
.LASF50:
	.ascii	"bDoLock\000"
.LASF29:
	.ascii	"AES_DEC\000"
.LASF70:
	.ascii	"sp_hacc_internal\000"
.LASF17:
	.ascii	"disable\000"
.LASF69:
	.ascii	"mt65xx_clock_category\000"
.LASF1:
	.ascii	"unsigned int\000"
.LASF54:
	.ascii	"sp_hacc_init\000"
.LASF43:
	.ascii	"MT65XX_CLOCK_MMSYS1_PDN0\000"
.LASF44:
	.ascii	"MT65XX_CLOCK_MMSYS1_PDN1\000"
.LASF45:
	.ascii	"MT65XX_CLOCK_MMSYS1_PDN2\000"
.LASF65:
	.ascii	"GNU C 4.6.x-google 20120106 (prerelease)\000"
	.ident	"GCC: (GNU) 4.6.x-google 20120106 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
