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
	.file	"hacc_hal.c"
	.text
.Ltext0:
	.align	2
	.global	SST_Secure_Init
	.type	SST_Secure_Init, %function
SST_Secure_Init:
.LFB183:
	.file 1 "alps/kernel/mediatek/platform/mt6577/kernel/drivers/masp/asf/mach/hacc_hal.c"
	.loc 1 73 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
.LCFI0:
	stmfd	sp!, {r4, r5, fp, ip, lr, pc}
.LCFI1:
	sub	fp, ip, #4
.LCFI2:
.LVL0:
	.loc 1 76 0
	ldr	r4, .L7
	ldrb	r3, [r4, #0]	@ zero_extendqisi2
	cmp	r3, #0
	beq	.L6
	.loc 1 74 0
	mov	r0, #1
.LVL1:
	.loc 1 90 0
	ldmfd	sp, {r4, r5, fp, sp, pc}
.LVL2:
.L6:
	.loc 1 79 0
	mov	r0, #32
	ldr	r1, .L7+4
	bl	hwEnableClock
.LVL3:
	subs	r5, r0, #0
	.loc 1 81 0
	movne	r0, #1
.LVL4:
	strneb	r0, [r4, #0]
	.loc 1 79 0
	ldmnefd	sp, {r4, r5, fp, sp, pc}
	.loc 1 85 0
	ldr	r0, .L7+8
	ldr	r1, .L7+12
	bl	printk
	mov	r0, r5
	ldmfd	sp, {r4, r5, fp, sp, pc}
.L8:
	.align	2
.L7:
	.word	.LANCHOR0
	.word	.LC0
	.word	.LC1
	.word	.LC2
.LFE183:
	.size	SST_Secure_Init, .-SST_Secure_Init
	.align	2
	.global	SST_Secure_DeInit
	.type	SST_Secure_DeInit, %function
SST_Secure_DeInit:
.LFB184:
	.loc 1 96 0
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
.LCFI3:
	stmfd	sp!, {r4, r5, fp, ip, lr, pc}
.LCFI4:
	sub	fp, ip, #4
.LCFI5:
.LVL5:
	.loc 1 99 0
	ldr	r4, .L14
	ldrb	r3, [r4, #0]	@ zero_extendqisi2
	cmp	r3, #0
	bne	.L13
	.loc 1 97 0
	mov	r0, #1
.LVL6:
	.loc 1 113 0
	ldmfd	sp, {r4, r5, fp, sp, pc}
.LVL7:
.L13:
	.loc 1 102 0
	mov	r0, #32
	ldr	r1, .L14+4
	bl	hwDisableClock
.LVL8:
	subs	r5, r0, #0
	.loc 1 104 0
	movne	r3, #0
	movne	r0, #1
.LVL9:
	strneb	r3, [r4, #0]
	.loc 1 102 0
	ldmnefd	sp, {r4, r5, fp, sp, pc}
	.loc 1 108 0
	ldr	r0, .L14+8
	ldr	r1, .L14+12
	bl	printk
	mov	r0, r5
	ldmfd	sp, {r4, r5, fp, sp, pc}
.L15:
	.align	2
.L14:
	.word	.LANCHOR0
	.word	.LC0
	.word	.LC3
	.word	.LC2
.LFE184:
	.size	SST_Secure_DeInit, .-SST_Secure_DeInit
	.global	bClkEn
	.section	.rodata.str1.4,"aMS",%progbits,1
	.align	2
.LC0:
	.ascii	"HACC\000"
	.space	3
.LC1:
	.ascii	"[%s] Enable CLK fail\012\000"
	.space	2
.LC2:
	.ascii	"SEC_HAL\000"
.LC3:
	.ascii	"[%s] Disable CLK fail\012\000"
	.bss
.LANCHOR0 = . + 0
	.type	bClkEn, %object
	.size	bClkEn, 1
bClkEn:
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
	.byte	0x85
	.uleb128 0x5
	.byte	0x84
	.uleb128 0x6
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
	.4byte	.LCFI3-.LFB184
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
	.byte	0x85
	.uleb128 0x5
	.byte	0x84
	.uleb128 0x6
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
	.file 3 "alps/kernel/../mediatek/platform/mt6577/kernel/core/include/mach/mt_clock_manager.h"
	.file 4 "alps/kernel/arch/arm/include/asm/hwcap.h"
	.file 5 "alps/kernel/include/linux/printk.h"
	.file 6 "alps/kernel/include/linux/kernel.h"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x546
	.2byte	0x2
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF152
	.byte	0x1
	.4byte	.LASF153
	.4byte	.LASF154
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
	.4byte	.LASF155
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
	.4byte	.LASF30
	.byte	0x4
	.byte	0x3
	.byte	0x66
	.4byte	0x18f
	.uleb128 0xb
	.4byte	.LASF22
	.sleb128 0
	.uleb128 0xb
	.4byte	.LASF23
	.sleb128 1
	.uleb128 0xb
	.4byte	.LASF24
	.sleb128 2
	.uleb128 0xb
	.4byte	.LASF25
	.sleb128 3
	.uleb128 0xb
	.4byte	.LASF26
	.sleb128 4
	.uleb128 0xb
	.4byte	.LASF27
	.sleb128 5
	.uleb128 0xb
	.4byte	.LASF28
	.sleb128 6
	.uleb128 0xb
	.4byte	.LASF29
	.sleb128 7
	.byte	0
	.uleb128 0xa
	.4byte	.LASF31
	.byte	0x4
	.byte	0x3
	.byte	0x71
	.4byte	0x48d
	.uleb128 0xb
	.4byte	.LASF32
	.sleb128 0
	.uleb128 0xb
	.4byte	.LASF33
	.sleb128 1
	.uleb128 0xb
	.4byte	.LASF34
	.sleb128 2
	.uleb128 0xb
	.4byte	.LASF35
	.sleb128 3
	.uleb128 0xb
	.4byte	.LASF36
	.sleb128 4
	.uleb128 0xb
	.4byte	.LASF37
	.sleb128 5
	.uleb128 0xb
	.4byte	.LASF38
	.sleb128 6
	.uleb128 0xb
	.4byte	.LASF39
	.sleb128 7
	.uleb128 0xb
	.4byte	.LASF40
	.sleb128 8
	.uleb128 0xb
	.4byte	.LASF41
	.sleb128 9
	.uleb128 0xb
	.4byte	.LASF42
	.sleb128 10
	.uleb128 0xb
	.4byte	.LASF43
	.sleb128 11
	.uleb128 0xb
	.4byte	.LASF44
	.sleb128 12
	.uleb128 0xb
	.4byte	.LASF45
	.sleb128 13
	.uleb128 0xb
	.4byte	.LASF46
	.sleb128 15
	.uleb128 0xb
	.4byte	.LASF47
	.sleb128 16
	.uleb128 0xb
	.4byte	.LASF48
	.sleb128 17
	.uleb128 0xb
	.4byte	.LASF49
	.sleb128 18
	.uleb128 0xb
	.4byte	.LASF50
	.sleb128 19
	.uleb128 0xb
	.4byte	.LASF51
	.sleb128 20
	.uleb128 0xb
	.4byte	.LASF52
	.sleb128 21
	.uleb128 0xb
	.4byte	.LASF53
	.sleb128 22
	.uleb128 0xb
	.4byte	.LASF54
	.sleb128 23
	.uleb128 0xb
	.4byte	.LASF55
	.sleb128 24
	.uleb128 0xb
	.4byte	.LASF56
	.sleb128 25
	.uleb128 0xb
	.4byte	.LASF57
	.sleb128 26
	.uleb128 0xb
	.4byte	.LASF58
	.sleb128 27
	.uleb128 0xb
	.4byte	.LASF59
	.sleb128 28
	.uleb128 0xb
	.4byte	.LASF60
	.sleb128 29
	.uleb128 0xb
	.4byte	.LASF61
	.sleb128 30
	.uleb128 0xb
	.4byte	.LASF62
	.sleb128 31
	.uleb128 0xb
	.4byte	.LASF63
	.sleb128 32
	.uleb128 0xb
	.4byte	.LASF64
	.sleb128 33
	.uleb128 0xb
	.4byte	.LASF65
	.sleb128 64
	.uleb128 0xb
	.4byte	.LASF66
	.sleb128 65
	.uleb128 0xb
	.4byte	.LASF67
	.sleb128 66
	.uleb128 0xb
	.4byte	.LASF68
	.sleb128 67
	.uleb128 0xb
	.4byte	.LASF69
	.sleb128 68
	.uleb128 0xb
	.4byte	.LASF70
	.sleb128 69
	.uleb128 0xb
	.4byte	.LASF71
	.sleb128 70
	.uleb128 0xb
	.4byte	.LASF72
	.sleb128 71
	.uleb128 0xb
	.4byte	.LASF73
	.sleb128 72
	.uleb128 0xb
	.4byte	.LASF74
	.sleb128 96
	.uleb128 0xb
	.4byte	.LASF75
	.sleb128 97
	.uleb128 0xb
	.4byte	.LASF76
	.sleb128 98
	.uleb128 0xb
	.4byte	.LASF77
	.sleb128 99
	.uleb128 0xb
	.4byte	.LASF78
	.sleb128 100
	.uleb128 0xb
	.4byte	.LASF79
	.sleb128 101
	.uleb128 0xb
	.4byte	.LASF80
	.sleb128 102
	.uleb128 0xb
	.4byte	.LASF81
	.sleb128 103
	.uleb128 0xb
	.4byte	.LASF82
	.sleb128 104
	.uleb128 0xb
	.4byte	.LASF83
	.sleb128 105
	.uleb128 0xb
	.4byte	.LASF84
	.sleb128 106
	.uleb128 0xb
	.4byte	.LASF85
	.sleb128 107
	.uleb128 0xb
	.4byte	.LASF86
	.sleb128 108
	.uleb128 0xb
	.4byte	.LASF87
	.sleb128 109
	.uleb128 0xb
	.4byte	.LASF88
	.sleb128 110
	.uleb128 0xb
	.4byte	.LASF89
	.sleb128 111
	.uleb128 0xb
	.4byte	.LASF90
	.sleb128 112
	.uleb128 0xb
	.4byte	.LASF91
	.sleb128 113
	.uleb128 0xb
	.4byte	.LASF92
	.sleb128 114
	.uleb128 0xb
	.4byte	.LASF93
	.sleb128 115
	.uleb128 0xb
	.4byte	.LASF94
	.sleb128 116
	.uleb128 0xb
	.4byte	.LASF95
	.sleb128 117
	.uleb128 0xb
	.4byte	.LASF96
	.sleb128 118
	.uleb128 0xb
	.4byte	.LASF97
	.sleb128 119
	.uleb128 0xb
	.4byte	.LASF98
	.sleb128 120
	.uleb128 0xb
	.4byte	.LASF99
	.sleb128 121
	.uleb128 0xb
	.4byte	.LASF100
	.sleb128 122
	.uleb128 0xb
	.4byte	.LASF101
	.sleb128 123
	.uleb128 0xb
	.4byte	.LASF102
	.sleb128 124
	.uleb128 0xb
	.4byte	.LASF103
	.sleb128 125
	.uleb128 0xb
	.4byte	.LASF104
	.sleb128 126
	.uleb128 0xb
	.4byte	.LASF105
	.sleb128 127
	.uleb128 0xb
	.4byte	.LASF106
	.sleb128 128
	.uleb128 0xb
	.4byte	.LASF107
	.sleb128 129
	.uleb128 0xb
	.4byte	.LASF108
	.sleb128 130
	.uleb128 0xb
	.4byte	.LASF109
	.sleb128 131
	.uleb128 0xb
	.4byte	.LASF110
	.sleb128 132
	.uleb128 0xb
	.4byte	.LASF111
	.sleb128 133
	.uleb128 0xb
	.4byte	.LASF112
	.sleb128 134
	.uleb128 0xb
	.4byte	.LASF113
	.sleb128 135
	.uleb128 0xb
	.4byte	.LASF114
	.sleb128 136
	.uleb128 0xb
	.4byte	.LASF115
	.sleb128 136
	.uleb128 0xb
	.4byte	.LASF116
	.sleb128 160
	.uleb128 0xb
	.4byte	.LASF117
	.sleb128 162
	.uleb128 0xb
	.4byte	.LASF118
	.sleb128 163
	.uleb128 0xb
	.4byte	.LASF119
	.sleb128 164
	.uleb128 0xb
	.4byte	.LASF120
	.sleb128 174
	.uleb128 0xb
	.4byte	.LASF121
	.sleb128 175
	.uleb128 0xb
	.4byte	.LASF122
	.sleb128 176
	.uleb128 0xb
	.4byte	.LASF123
	.sleb128 177
	.uleb128 0xb
	.4byte	.LASF124
	.sleb128 178
	.uleb128 0xb
	.4byte	.LASF125
	.sleb128 179
	.uleb128 0xb
	.4byte	.LASF126
	.sleb128 194
	.uleb128 0xb
	.4byte	.LASF127
	.sleb128 197
	.uleb128 0xb
	.4byte	.LASF128
	.sleb128 198
	.uleb128 0xb
	.4byte	.LASF129
	.sleb128 0
	.uleb128 0xb
	.4byte	.LASF130
	.sleb128 0
	.uleb128 0xb
	.4byte	.LASF131
	.sleb128 31
	.uleb128 0xb
	.4byte	.LASF132
	.sleb128 32
	.uleb128 0xb
	.4byte	.LASF133
	.sleb128 33
	.uleb128 0xb
	.4byte	.LASF134
	.sleb128 64
	.uleb128 0xb
	.4byte	.LASF135
	.sleb128 72
	.uleb128 0xb
	.4byte	.LASF136
	.sleb128 96
	.uleb128 0xb
	.4byte	.LASF137
	.sleb128 127
	.uleb128 0xb
	.4byte	.LASF138
	.sleb128 128
	.uleb128 0xb
	.4byte	.LASF139
	.sleb128 136
	.uleb128 0xb
	.4byte	.LASF140
	.sleb128 160
	.uleb128 0xb
	.4byte	.LASF141
	.sleb128 179
	.uleb128 0xb
	.4byte	.LASF142
	.sleb128 192
	.uleb128 0xb
	.4byte	.LASF143
	.sleb128 198
	.uleb128 0xb
	.4byte	.LASF144
	.sleb128 199
	.byte	0
	.uleb128 0xc
	.byte	0x1
	.4byte	.LASF145
	.byte	0x1
	.byte	0x48
	.byte	0x1
	.4byte	0x7a
	.4byte	.LFB183
	.4byte	.LFE183
	.4byte	.LLST0
	.4byte	0x4ba
	.uleb128 0xd
	.ascii	"ret\000"
	.byte	0x1
	.byte	0x4a
	.4byte	0x7a
	.4byte	.LLST1
	.byte	0
	.uleb128 0xc
	.byte	0x1
	.4byte	.LASF146
	.byte	0x1
	.byte	0x5f
	.byte	0x1
	.4byte	0x7a
	.4byte	.LFB184
	.4byte	.LFE184
	.4byte	.LLST2
	.4byte	0x4e7
	.uleb128 0xd
	.ascii	"ret\000"
	.byte	0x1
	.byte	0x61
	.4byte	0x7a
	.4byte	.LLST3
	.byte	0
	.uleb128 0xe
	.4byte	.LASF147
	.byte	0x4
	.byte	0x23
	.4byte	0x34
	.byte	0x1
	.byte	0x1
	.uleb128 0xf
	.4byte	0x5e
	.4byte	0x4ff
	.uleb128 0x10
	.byte	0
	.uleb128 0xe
	.4byte	.LASF148
	.byte	0x5
	.byte	0x1b
	.4byte	0x4f4
	.byte	0x1
	.byte	0x1
	.uleb128 0xf
	.4byte	0x3b
	.4byte	0x517
	.uleb128 0x10
	.byte	0
	.uleb128 0x11
	.4byte	.LASF149
	.byte	0x6
	.2byte	0x18a
	.4byte	0x525
	.byte	0x1
	.byte	0x1
	.uleb128 0x12
	.4byte	0x50c
	.uleb128 0xe
	.4byte	.LASF150
	.byte	0x2
	.byte	0x2b
	.4byte	0x89
	.byte	0x1
	.byte	0x1
	.uleb128 0x13
	.4byte	.LASF151
	.byte	0x1
	.byte	0x40
	.4byte	0x7a
	.byte	0x1
	.byte	0x5
	.byte	0x3
	.4byte	bClkEn
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
	.uleb128 0xe
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
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
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
	.4byte	.LVL1-.Ltext0
	.2byte	0x2
	.byte	0x31
	.byte	0x9f
	.4byte	.LVL1-.Ltext0
	.4byte	.LVL2-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL2-.Ltext0
	.4byte	.LVL3-.Ltext0
	.2byte	0x2
	.byte	0x31
	.byte	0x9f
	.4byte	.LVL3-.Ltext0
	.4byte	.LVL4-.Ltext0
	.2byte	0x5
	.byte	0x70
	.sleb128 0
	.byte	0x30
	.byte	0x2e
	.byte	0x9f
	.4byte	.LVL4-.Ltext0
	.4byte	.LFE183-.Ltext0
	.2byte	0x5
	.byte	0x75
	.sleb128 0
	.byte	0x30
	.byte	0x2e
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST2:
	.4byte	.LFB184-.Ltext0
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
	.4byte	.LFE184-.Ltext0
	.2byte	0x2
	.byte	0x7b
	.sleb128 4
	.4byte	0
	.4byte	0
.LLST3:
	.4byte	.LVL5-.Ltext0
	.4byte	.LVL6-.Ltext0
	.2byte	0x2
	.byte	0x31
	.byte	0x9f
	.4byte	.LVL6-.Ltext0
	.4byte	.LVL7-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL7-.Ltext0
	.4byte	.LVL8-.Ltext0
	.2byte	0x2
	.byte	0x31
	.byte	0x9f
	.4byte	.LVL8-.Ltext0
	.4byte	.LVL9-.Ltext0
	.2byte	0x5
	.byte	0x70
	.sleb128 0
	.byte	0x30
	.byte	0x2e
	.byte	0x9f
	.4byte	.LVL9-.Ltext0
	.4byte	.LFE184-.Ltext0
	.2byte	0x5
	.byte	0x75
	.sleb128 0
	.byte	0x30
	.byte	0x2e
	.byte	0x9f
	.4byte	0
	.4byte	0
	.section	.debug_pubnames,"",%progbits
.Ldebug_pubnames0:
	.4byte	0x43
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x54a
	.4byte	0x48d
	.ascii	"SST_Secure_Init\000"
	.4byte	0x4ba
	.ascii	"SST_Secure_DeInit\000"
	.4byte	0x537
	.ascii	"bClkEn\000"
	.4byte	0
	.section	.debug_pubtypes,"",%progbits
.Ldebug_pubtypes0:
	.4byte	0xd82
	.2byte	0x2
	.4byte	.Ldebug_info0
	.4byte	0x54a
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
	.4byte	0x152
	.ascii	"mt65xx_clock_category\000"
	.4byte	0x15e
	.ascii	"MT65XX_CLOCK_PERI_PDN0\000"
	.4byte	0x164
	.ascii	"MT65XX_CLOCK_PERI_PDN1\000"
	.4byte	0x16a
	.ascii	"MT65XX_CLOCK_MMSYS1_PDN0\000"
	.4byte	0x170
	.ascii	"MT65XX_CLOCK_MMSYS1_PDN1\000"
	.4byte	0x176
	.ascii	"MT65XX_CLOCK_MMSYS1_PDN2\000"
	.4byte	0x17c
	.ascii	"MT65XX_CLOCK_MMSYS2_PDN\000"
	.4byte	0x182
	.ascii	"MT65XX_CLOCK_AUDIO_PDN\000"
	.4byte	0x188
	.ascii	"MT65XX_CLOCK_CATEGORY_COUNT\000"
	.4byte	0x18f
	.ascii	"mt65xx_clock_id\000"
	.4byte	0x19b
	.ascii	"MT65XX_PDN_PERI_NFI\000"
	.4byte	0x1a1
	.ascii	"MT65XX_PDN_PERI_THERM\000"
	.4byte	0x1a7
	.ascii	"MT65XX_PDN_PERI_PWM1\000"
	.4byte	0x1ad
	.ascii	"MT65XX_PDN_PERI_PWM2\000"
	.4byte	0x1b3
	.ascii	"MT65XX_PDN_PERI_PWM3\000"
	.4byte	0x1b9
	.ascii	"MT65XX_PDN_PERI_PWM456\000"
	.4byte	0x1bf
	.ascii	"MT65XX_PDN_PERI_PWM7\000"
	.4byte	0x1c5
	.ascii	"MT65XX_PDN_PERI_SIMIF0\000"
	.4byte	0x1cb
	.ascii	"MT65XX_PDN_PERI_SIMIF1\000"
	.4byte	0x1d1
	.ascii	"MT65XX_PDN_PERI_USB1\000"
	.4byte	0x1d7
	.ascii	"MT65XX_PDN_PERI_USB2\000"
	.4byte	0x1dd
	.ascii	"MT65XX_PDN_PERI_CCIF\000"
	.4byte	0x1e3
	.ascii	"MT65XX_PDN_PERI_APDMA\000"
	.4byte	0x1e9
	.ascii	"MT65XX_PDN_PERI_APXGPT\000"
	.4byte	0x1ef
	.ascii	"MT65XX_PDN_PERI_MSDC0\000"
	.4byte	0x1f5
	.ascii	"MT65XX_PDN_PERI_MSDC1\000"
	.4byte	0x1fb
	.ascii	"MT65XX_PDN_PERI_MSDC2\000"
	.4byte	0x201
	.ascii	"MT65XX_PDN_PERI_MSDC3\000"
	.4byte	0x207
	.ascii	"MT65XX_PDN_PERI_APHIF\000"
	.4byte	0x20d
	.ascii	"MT65XX_PDN_PERI_MDHIF\000"
	.4byte	0x213
	.ascii	"MT65XX_PDN_PERI_NLI_ARB\000"
	.4byte	0x219
	.ascii	"MT65XX_PDN_PERI_ACCDET\000"
	.4byte	0x21f
	.ascii	"MT65XX_PDN_PERI_IRDA\000"
	.4byte	0x225
	.ascii	"MT65XX_PDN_PERI_UART0\000"
	.4byte	0x22b
	.ascii	"MT65XX_PDN_PERI_UART1\000"
	.4byte	0x231
	.ascii	"MT65XX_PDN_PERI_UART2\000"
	.4byte	0x237
	.ascii	"MT65XX_PDN_PERI_UART3\000"
	.4byte	0x23d
	.ascii	"MT65XX_PDN_PERI_I2C0\000"
	.4byte	0x243
	.ascii	"MT65XX_PDN_PERI_I2C1\000"
	.4byte	0x249
	.ascii	"MT65XX_PDN_PERI_I2C_DUAL\000"
	.4byte	0x24f
	.ascii	"MT65XX_PDN_PERI_AUXADC\000"
	.4byte	0x255
	.ascii	"MT65XX_PDN_PERI_HACC\000"
	.4byte	0x25b
	.ascii	"MT65XX_PDN_PERI_TRNG\000"
	.4byte	0x261
	.ascii	"MT65XX_PDN_MM_VBUF\000"
	.4byte	0x268
	.ascii	"MT65XX_PDN_MM_VDEC\000"
	.4byte	0x26f
	.ascii	"MT65XX_PDN_MM_VENC\000"
	.4byte	0x276
	.ascii	"MT65XX_PDN_MM_SMI_LARB2_ACP_BUS\000"
	.4byte	0x27d
	.ascii	"MT65XX_PDN_MM_SMI_LARB2_260MHZ\000"
	.4byte	0x284
	.ascii	"MT65XX_PDN_MM_SMI_LARB2_EMI\000"
	.4byte	0x28b
	.ascii	"MT65XX_PDN_MM_SMI_LARB2_ACP_BUS_EMI\000"
	.4byte	0x292
	.ascii	"MT65XX_PDN_MM_SMI_LARB1_EMI\000"
	.4byte	0x299
	.ascii	"MT65XX_PDN_MM_SMI_LARB0_EMI\000"
	.4byte	0x2a0
	.ascii	"MT65XX_PDN_MM_VRZ1\000"
	.4byte	0x2a7
	.ascii	"MT65XX_PDN_MM_CSI2\000"
	.4byte	0x2ae
	.ascii	"MT65XX_PDN_MM_FD\000"
	.4byte	0x2b5
	.ascii	"MT65XX_PDN_MM_RESZ_LB\000"
	.4byte	0x2bc
	.ascii	"MT65XX_PDN_MM_TV_ROT\000"
	.4byte	0x2c3
	.ascii	"MT65XX_PDN_MM_LCD\000"
	.4byte	0x2ca
	.ascii	"MT65XX_PDN_MM_RGB_ROT2\000"
	.4byte	0x2d1
	.ascii	"MT65XX_PDN_MM_PRZ1\000"
	.4byte	0x2d8
	.ascii	"MT65XX_PDN_MM_R_DMA1\000"
	.4byte	0x2df
	.ascii	"MT65XX_PDN_MM_VDO_ROT1\000"
	.4byte	0x2e6
	.ascii	"MT65XX_PDN_MM_RGB_ROT1\000"
	.4byte	0x2ed
	.ascii	"MT65XX_PDN_MM_VRZ\000"
	.4byte	0x2f4
	.ascii	"MT65XX_PDN_MM_RGB_ROT0\000"
	.4byte	0x2fb
	.ascii	"MT65XX_PDN_MM_PRZ0_MOUT\000"
	.4byte	0x302
	.ascii	"MT65XX_PDN_MM_PRZ0\000"
	.4byte	0x309
	.ascii	"MT65XX_PDN_MM_VDO_ROT0\000"
	.4byte	0x310
	.ascii	"MT65XX_PDN_MM_MOUT\000"
	.4byte	0x317
	.ascii	"MT65XX_PDN_MM_OVL_DMA_MIMO\000"
	.4byte	0x31e
	.ascii	"MT65XX_PDN_MM_OVL_DMA_BPS\000"
	.4byte	0x325
	.ascii	"MT65XX_PDN_MM_OVL_DMA\000"
	.4byte	0x32c
	.ascii	"MT65XX_PDN_MM_IPP_MOUT\000"
	.4byte	0x333
	.ascii	"MT65XX_PDN_MM_IPP\000"
	.4byte	0x33a
	.ascii	"MT65XX_PDN_MM_EIS\000"
	.4byte	0x341
	.ascii	"MT65XX_PDN_MM_CRZ\000"
	.4byte	0x348
	.ascii	"MT65XX_PDN_MM_JPEG_DMA\000"
	.4byte	0x34f
	.ascii	"MT65XX_PDN_MM_BRZ_MOUT\000"
	.4byte	0x356
	.ascii	"MT65XX_PDN_MM_BRZ\000"
	.4byte	0x35d
	.ascii	"MT65XX_PDN_MM_JPEG_DEC\000"
	.4byte	0x364
	.ascii	"MT65XX_PDN_MM_JPEG_ENC\000"
	.4byte	0x36b
	.ascii	"MT65XX_PDN_MM_R_DMA0_MOUT\000"
	.4byte	0x372
	.ascii	"MT65XX_PDN_MM_R_DMA0\000"
	.4byte	0x379
	.ascii	"MT65XX_PDN_MM_CAM\000"
	.4byte	0x380
	.ascii	"MT65XX_PDN_MM_SCAM\000"
	.4byte	0x387
	.ascii	"MT65XX_PDN_MM_SPI\000"
	.4byte	0x38e
	.ascii	"MT65XX_PDN_MM_TVC\000"
	.4byte	0x395
	.ascii	"MT65XX_PDN_MM_TVE\000"
	.4byte	0x39c
	.ascii	"MT65XX_PDN_MM_DPI\000"
	.4byte	0x3a3
	.ascii	"MT65XX_PDN_MM_DSI\000"
	.4byte	0x3aa
	.ascii	"MT65XX_PDN_MM_SMI_LARB2\000"
	.4byte	0x3b1
	.ascii	"MT65XX_PDN_MM_SMI_LARB1\000"
	.4byte	0x3b8
	.ascii	"MT65XX_PDN_MM_SMI_LARB0\000"
	.4byte	0x3bf
	.ascii	"MT65XX_PDN_MM_LCD_EMI\000"
	.4byte	0x3c6
	.ascii	"MT65XX_PDN_MM_SMI_LARB3_FULL\000"
	.4byte	0x3cd
	.ascii	"MT65XX_PDN_MM_GDC_SHARE_MACRO\000"
	.4byte	0x3d4
	.ascii	"MT65XX_PDN_MM_G2D\000"
	.4byte	0x3db
	.ascii	"MT65XX_PDN_MM_MFG\000"
	.4byte	0x3e2
	.ascii	"MT65XX_PDN_MM_G3D\000"
	.4byte	0x3e9
	.ascii	"MT65XX_PDN_MM_F26M\000"
	.4byte	0x3f0
	.ascii	"MT65XX_PDN_MM_GDC_SHARE_MACRO_HALF\000"
	.4byte	0x3f7
	.ascii	"MT65XX_PDN_MM_SMI_LARB3_HALF\000"
	.4byte	0x3fe
	.ascii	"MT65XX_PDN_MM_AUDIO_CORE\000"
	.4byte	0x405
	.ascii	"MT65XX_PDN_MM_MFG_HALF\000"
	.4byte	0x40c
	.ascii	"MT65XX_PDN_AUDIO_AFE\000"
	.4byte	0x413
	.ascii	"MT65XX_PDN_AUDIO_ADC\000"
	.4byte	0x41a
	.ascii	"MT65XX_PDN_AUDIO_I2S\000"
	.4byte	0x421
	.ascii	"MT65XX_CLOCK_COUNT_BEGIN\000"
	.4byte	0x427
	.ascii	"MT65XX_PERI_PDN0_BEGIN\000"
	.4byte	0x42d
	.ascii	"MT65XX_PERI_PDN0_END\000"
	.4byte	0x433
	.ascii	"MT65XX_PERI_PDN1_BEGIN\000"
	.4byte	0x439
	.ascii	"MT65XX_PERI_PDN1_END\000"
	.4byte	0x43f
	.ascii	"MT65XX_MM1_PDN0_BEGIN\000"
	.4byte	0x446
	.ascii	"MT65XX_MM1_PDN0_END\000"
	.4byte	0x44d
	.ascii	"MT65XX_MM1_PDN1_BEGIN\000"
	.4byte	0x454
	.ascii	"MT65XX_MM1_PDN1_END\000"
	.4byte	0x45b
	.ascii	"MT65XX_MM1_PDN2_BEGIN\000"
	.4byte	0x462
	.ascii	"MT65XX_MM1_PDN2_END\000"
	.4byte	0x469
	.ascii	"MT65XX_MM2_PDN_BEGIN\000"
	.4byte	0x470
	.ascii	"MT65XX_MM2_PDN_END\000"
	.4byte	0x477
	.ascii	"MT65XX_AUDIO_PDN_BEGIN\000"
	.4byte	0x47e
	.ascii	"MT65XX_AUDIO_PDN_END\000"
	.4byte	0x485
	.ascii	"MT65XX_CLOCK_COUNT\000"
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
.LASF91:
	.ascii	"MT65XX_PDN_MM_OVL_DMA_MIMO\000"
.LASF12:
	.ascii	"clean_range\000"
.LASF86:
	.ascii	"MT65XX_PDN_MM_RGB_ROT0\000"
.LASF84:
	.ascii	"MT65XX_PDN_MM_RGB_ROT1\000"
.LASF80:
	.ascii	"MT65XX_PDN_MM_RGB_ROT2\000"
.LASF134:
	.ascii	"MT65XX_MM1_PDN0_BEGIN\000"
.LASF131:
	.ascii	"MT65XX_PERI_PDN0_END\000"
.LASF94:
	.ascii	"MT65XX_PDN_MM_IPP_MOUT\000"
.LASF150:
	.ascii	"outer_cache\000"
.LASF155:
	.ascii	"outer_cache_fns\000"
.LASF1:
	.ascii	"unsigned int\000"
.LASF46:
	.ascii	"MT65XX_PDN_PERI_MSDC0\000"
.LASF47:
	.ascii	"MT65XX_PDN_PERI_MSDC1\000"
.LASF48:
	.ascii	"MT65XX_PDN_PERI_MSDC2\000"
.LASF49:
	.ascii	"MT65XX_PDN_PERI_MSDC3\000"
.LASF98:
	.ascii	"MT65XX_PDN_MM_JPEG_DMA\000"
.LASF11:
	.ascii	"inv_range\000"
.LASF96:
	.ascii	"MT65XX_PDN_MM_EIS\000"
.LASF107:
	.ascii	"MT65XX_PDN_MM_SPI\000"
.LASF34:
	.ascii	"MT65XX_PDN_PERI_PWM1\000"
.LASF35:
	.ascii	"MT65XX_PDN_PERI_PWM2\000"
.LASF18:
	.ascii	"sync\000"
.LASF38:
	.ascii	"MT65XX_PDN_PERI_PWM7\000"
.LASF104:
	.ascii	"MT65XX_PDN_MM_R_DMA0\000"
.LASF82:
	.ascii	"MT65XX_PDN_MM_R_DMA1\000"
.LASF63:
	.ascii	"MT65XX_PDN_PERI_HACC\000"
.LASF31:
	.ascii	"mt65xx_clock_id\000"
.LASF135:
	.ascii	"MT65XX_MM1_PDN0_END\000"
.LASF95:
	.ascii	"MT65XX_PDN_MM_IPP\000"
.LASF99:
	.ascii	"MT65XX_PDN_MM_BRZ_MOUT\000"
.LASF110:
	.ascii	"MT65XX_PDN_MM_DPI\000"
.LASF130:
	.ascii	"MT65XX_PERI_PDN0_BEGIN\000"
.LASF74:
	.ascii	"MT65XX_PDN_MM_VRZ1\000"
.LASF75:
	.ascii	"MT65XX_PDN_MM_CSI2\000"
.LASF138:
	.ascii	"MT65XX_MM1_PDN2_BEGIN\000"
.LASF44:
	.ascii	"MT65XX_PDN_PERI_APDMA\000"
.LASF21:
	.ascii	"float\000"
.LASF69:
	.ascii	"MT65XX_PDN_MM_SMI_LARB2_260MHZ\000"
.LASF145:
	.ascii	"SST_Secure_Init\000"
.LASF8:
	.ascii	"long long unsigned int\000"
.LASF67:
	.ascii	"MT65XX_PDN_MM_VENC\000"
.LASF13:
	.ascii	"flush_range\000"
.LASF66:
	.ascii	"MT65XX_PDN_MM_VDEC\000"
.LASF27:
	.ascii	"MT65XX_CLOCK_MMSYS2_PDN\000"
.LASF93:
	.ascii	"MT65XX_PDN_MM_OVL_DMA\000"
.LASF115:
	.ascii	"MT65XX_PDN_MM_LCD_EMI\000"
.LASF20:
	.ascii	"resume\000"
.LASF87:
	.ascii	"MT65XX_PDN_MM_PRZ0_MOUT\000"
.LASF76:
	.ascii	"MT65XX_PDN_MM_FD\000"
.LASF71:
	.ascii	"MT65XX_PDN_MM_SMI_LARB2_ACP_BUS_EMI\000"
.LASF73:
	.ascii	"MT65XX_PDN_MM_SMI_LARB0_EMI\000"
.LASF10:
	.ascii	"_Bool\000"
.LASF111:
	.ascii	"MT65XX_PDN_MM_DSI\000"
.LASF118:
	.ascii	"MT65XX_PDN_MM_G2D\000"
.LASF22:
	.ascii	"MT65XX_CLOCK_PERI_PDN0\000"
.LASF23:
	.ascii	"MT65XX_CLOCK_PERI_PDN1\000"
.LASF2:
	.ascii	"char\000"
.LASF78:
	.ascii	"MT65XX_PDN_MM_TV_ROT\000"
.LASF53:
	.ascii	"MT65XX_PDN_PERI_ACCDET\000"
.LASF51:
	.ascii	"MT65XX_PDN_PERI_MDHIF\000"
.LASF43:
	.ascii	"MT65XX_PDN_PERI_CCIF\000"
.LASF120:
	.ascii	"MT65XX_PDN_MM_G3D\000"
.LASF129:
	.ascii	"MT65XX_CLOCK_COUNT_BEGIN\000"
.LASF85:
	.ascii	"MT65XX_PDN_MM_VRZ\000"
.LASF144:
	.ascii	"MT65XX_CLOCK_COUNT\000"
.LASF119:
	.ascii	"MT65XX_PDN_MM_MFG\000"
.LASF28:
	.ascii	"MT65XX_CLOCK_AUDIO_PDN\000"
.LASF79:
	.ascii	"MT65XX_PDN_MM_LCD\000"
.LASF24:
	.ascii	"MT65XX_CLOCK_MMSYS1_PDN0\000"
.LASF25:
	.ascii	"MT65XX_CLOCK_MMSYS1_PDN1\000"
.LASF7:
	.ascii	"long long int\000"
.LASF90:
	.ascii	"MT65XX_PDN_MM_MOUT\000"
.LASF154:
	.ascii	""
	.ascii	"alps/kernel/out\000"
.LASF65:
	.ascii	"MT65XX_PDN_MM_VBUF\000"
.LASF54:
	.ascii	"MT65XX_PDN_PERI_IRDA\000"
.LASF64:
	.ascii	"MT65XX_PDN_PERI_TRNG\000"
.LASF50:
	.ascii	"MT65XX_PDN_PERI_APHIF\000"
.LASF116:
	.ascii	"MT65XX_PDN_MM_SMI_LARB3_FULL\000"
.LASF102:
	.ascii	"MT65XX_PDN_MM_JPEG_ENC\000"
.LASF140:
	.ascii	"MT65XX_MM2_PDN_BEGIN\000"
.LASF101:
	.ascii	"MT65XX_PDN_MM_JPEG_DEC\000"
.LASF14:
	.ascii	"flush_all\000"
.LASF141:
	.ascii	"MT65XX_MM2_PDN_END\000"
.LASF55:
	.ascii	"MT65XX_PDN_PERI_UART0\000"
.LASF56:
	.ascii	"MT65XX_PDN_PERI_UART1\000"
.LASF57:
	.ascii	"MT65XX_PDN_PERI_UART2\000"
.LASF58:
	.ascii	"MT65XX_PDN_PERI_UART3\000"
.LASF92:
	.ascii	"MT65XX_PDN_MM_OVL_DMA_BPS\000"
.LASF153:
	.ascii	""
	.ascii	"alps/kernel/mediatek/plat"
	.ascii	"form/mt6577/kernel/drivers/masp/asf/mach/hacc_hal.c"
	.ascii	"\000"
.LASF148:
	.ascii	"console_printk\000"
.LASF70:
	.ascii	"MT65XX_PDN_MM_SMI_LARB2_EMI\000"
.LASF136:
	.ascii	"MT65XX_MM1_PDN1_BEGIN\000"
.LASF139:
	.ascii	"MT65XX_MM1_PDN2_END\000"
.LASF36:
	.ascii	"MT65XX_PDN_PERI_PWM3\000"
.LASF146:
	.ascii	"SST_Secure_DeInit\000"
.LASF5:
	.ascii	"short int\000"
.LASF9:
	.ascii	"long int\000"
.LASF117:
	.ascii	"MT65XX_PDN_MM_GDC_SHARE_MACRO\000"
.LASF143:
	.ascii	"MT65XX_AUDIO_PDN_END\000"
.LASF41:
	.ascii	"MT65XX_PDN_PERI_USB1\000"
.LASF42:
	.ascii	"MT65XX_PDN_PERI_USB2\000"
.LASF30:
	.ascii	"mt65xx_clock_category\000"
.LASF37:
	.ascii	"MT65XX_PDN_PERI_PWM456\000"
.LASF88:
	.ascii	"MT65XX_PDN_MM_PRZ0\000"
.LASF81:
	.ascii	"MT65XX_PDN_MM_PRZ1\000"
.LASF33:
	.ascii	"MT65XX_PDN_PERI_THERM\000"
.LASF133:
	.ascii	"MT65XX_PERI_PDN1_END\000"
.LASF147:
	.ascii	"elf_hwcap\000"
.LASF89:
	.ascii	"MT65XX_PDN_MM_VDO_ROT0\000"
.LASF83:
	.ascii	"MT65XX_PDN_MM_VDO_ROT1\000"
.LASF149:
	.ascii	"hex_asc\000"
.LASF132:
	.ascii	"MT65XX_PERI_PDN1_BEGIN\000"
.LASF127:
	.ascii	"MT65XX_PDN_AUDIO_ADC\000"
.LASF142:
	.ascii	"MT65XX_AUDIO_PDN_BEGIN\000"
.LASF59:
	.ascii	"MT65XX_PDN_PERI_I2C0\000"
.LASF60:
	.ascii	"MT65XX_PDN_PERI_I2C1\000"
.LASF128:
	.ascii	"MT65XX_PDN_AUDIO_I2S\000"
.LASF123:
	.ascii	"MT65XX_PDN_MM_SMI_LARB3_HALF\000"
.LASF17:
	.ascii	"disable\000"
.LASF152:
	.ascii	"GNU C 4.6.x-google 20120106 (prerelease)\000"
.LASF45:
	.ascii	"MT65XX_PDN_PERI_APXGPT\000"
.LASF105:
	.ascii	"MT65XX_PDN_MM_CAM\000"
.LASF0:
	.ascii	"long unsigned int\000"
.LASF137:
	.ascii	"MT65XX_MM1_PDN1_END\000"
.LASF4:
	.ascii	"unsigned char\000"
.LASF151:
	.ascii	"bClkEn\000"
.LASF39:
	.ascii	"MT65XX_PDN_PERI_SIMIF0\000"
.LASF40:
	.ascii	"MT65XX_PDN_PERI_SIMIF1\000"
.LASF15:
	.ascii	"clean_all\000"
.LASF62:
	.ascii	"MT65XX_PDN_PERI_AUXADC\000"
.LASF103:
	.ascii	"MT65XX_PDN_MM_R_DMA0_MOUT\000"
.LASF125:
	.ascii	"MT65XX_PDN_MM_MFG_HALF\000"
.LASF32:
	.ascii	"MT65XX_PDN_PERI_NFI\000"
.LASF52:
	.ascii	"MT65XX_PDN_PERI_NLI_ARB\000"
.LASF126:
	.ascii	"MT65XX_PDN_AUDIO_AFE\000"
.LASF106:
	.ascii	"MT65XX_PDN_MM_SCAM\000"
.LASF124:
	.ascii	"MT65XX_PDN_MM_AUDIO_CORE\000"
.LASF122:
	.ascii	"MT65XX_PDN_MM_GDC_SHARE_MACRO_HALF\000"
.LASF108:
	.ascii	"MT65XX_PDN_MM_TVC\000"
.LASF109:
	.ascii	"MT65XX_PDN_MM_TVE\000"
.LASF3:
	.ascii	"signed char\000"
.LASF121:
	.ascii	"MT65XX_PDN_MM_F26M\000"
.LASF97:
	.ascii	"MT65XX_PDN_MM_CRZ\000"
.LASF6:
	.ascii	"short unsigned int\000"
.LASF19:
	.ascii	"set_debug\000"
.LASF114:
	.ascii	"MT65XX_PDN_MM_SMI_LARB0\000"
.LASF113:
	.ascii	"MT65XX_PDN_MM_SMI_LARB1\000"
.LASF112:
	.ascii	"MT65XX_PDN_MM_SMI_LARB2\000"
.LASF26:
	.ascii	"MT65XX_CLOCK_MMSYS1_PDN2\000"
.LASF68:
	.ascii	"MT65XX_PDN_MM_SMI_LARB2_ACP_BUS\000"
.LASF29:
	.ascii	"MT65XX_CLOCK_CATEGORY_COUNT\000"
.LASF77:
	.ascii	"MT65XX_PDN_MM_RESZ_LB\000"
.LASF72:
	.ascii	"MT65XX_PDN_MM_SMI_LARB1_EMI\000"
.LASF100:
	.ascii	"MT65XX_PDN_MM_BRZ\000"
.LASF16:
	.ascii	"inv_all\000"
.LASF61:
	.ascii	"MT65XX_PDN_PERI_I2C_DUAL\000"
	.ident	"GCC: (GNU) 4.6.x-google 20120106 (prerelease)"
	.section	.note.GNU-stack,"",%progbits
