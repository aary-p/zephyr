/*
 * Copyright (c) 2025 MediaTek
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT mediatek_mt8370_gpio

#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio/gpio_utils.h>

#include "gpio_mtk_common.h"

/* Register offsets in order of offset values. */
#define GPIO_OFFSET_DIR_0      0x0000
#define GPIO_OFFSET_DIR_0_SET  0x0004
#define GPIO_OFFSET_DIR_0_CLR  0x0008
#define GPIO_OFFSET_DIR_1      0x0010
#define GPIO_OFFSET_DIR_1_SET  0x0014
#define GPIO_OFFSET_DIR_1_CLR  0x0018
#define GPIO_OFFSET_DIR_2      0x0020
#define GPIO_OFFSET_DIR_2_SET  0x0024
#define GPIO_OFFSET_DIR_2_CLR  0x0028
#define GPIO_OFFSET_DIR_3      0x0030
#define GPIO_OFFSET_DIR_3_SET  0x0034
#define GPIO_OFFSET_DIR_3_CLR  0x0038
#define GPIO_OFFSET_DIR_4      0x0040
#define GPIO_OFFSET_DIR_4_SET  0x0044
#define GPIO_OFFSET_DIR_4_CLR  0x0048
#define GPIO_OFFSET_DIR_5      0x0050
#define GPIO_OFFSET_DIR_5_SET  0x0054
#define GPIO_OFFSET_DIR_5_CLR  0x0058
#define GPIO_OFFSET_DOUT_0     0x0100
#define GPIO_OFFSET_DOUT_0_SET 0x0104
#define GPIO_OFFSET_DOUT_0_CLR 0x0108
#define GPIO_OFFSET_DOUT_1     0x0110
#define GPIO_OFFSET_DOUT_1_SET 0x0114
#define GPIO_OFFSET_DOUT_1_CLR 0x0118
#define GPIO_OFFSET_DOUT_2     0x0120
#define GPIO_OFFSET_DOUT_2_SET 0x0124
#define GPIO_OFFSET_DOUT_2_CLR 0x0128
#define GPIO_OFFSET_DOUT_3     0x0130
#define GPIO_OFFSET_DOUT_3_SET 0x0134
#define GPIO_OFFSET_DOUT_3_CLR 0x0138
#define GPIO_OFFSET_DOUT_4     0x0140
#define GPIO_OFFSET_DOUT_4_SET 0x0144
#define GPIO_OFFSET_DOUT_4_CLR 0x0148
#define GPIO_OFFSET_DOUT_5     0x0150
#define GPIO_OFFSET_DOUT_5_SET 0x0154
#define GPIO_OFFSET_DOUT_5_CLR 0x0158
#define GPIO_OFFSET_DIN_0      0x0200
#define GPIO_OFFSET_DIN_1      0x0210
#define GPIO_OFFSET_DIN_2      0x0220
#define GPIO_OFFSET_DIN_3      0x0230
#define GPIO_OFFSET_DIN_4      0x0240
#define GPIO_OFFSET_DIN_5      0x0250

#define GPIO_OFFSET_DIN_DELTA      (GPIO_OFFSET_DIN_1 - GPIO_OFFSET_DIN_0)
#define GPIO_OFFSET_DOUT_SET_DELTA (GPIO_OFFSET_DOUT_1_SET - GPIO_OFFSET_DOUT_0_SET)
#define GPIO_OFFSET_DOUT_CLR_DELTA (GPIO_OFFSET_DOUT_1_CLR - GPIO_OFFSET_DOUT_0_CLR)
#define GPIO_OFFSET_DIR_SET_DELTA  (GPIO_OFFSET_DIR_1_SET - GPIO_OFFSET_DIR_0_SET)
#define GPIO_OFFSET_DIR_CLR_DELTA  (GPIO_OFFSET_DIR_1_CLR - GPIO_OFFSET_DIR_0_CLR)

#define MT8370_IOCFG_RM_BASE 0x11c00000U
#define MT8370_IOCFG_LT_BASE 0x11e10000U
#define MT8370_IOCFG_LM_BASE 0x11e20000U
#define MT8370_IOCFG_RT_BASE 0x11ea0000U

typedef enum {
	MT8370_PULL_PU_PD = 0, /* separate PU and PD register bits */
	MT8370_PULL_PUPD,      /* single PUPD direction bit         */
} mt8370_pull_type_t;

typedef struct {
	uint32_t base;   /* physical address of iocfg bank */
	uint16_t pu_off; /* PU (or PUPD) register offset   */
	uint16_t pd_off; /* PD register offset (PU_PD only) */
	uint8_t  bit;    /* bit position within register    */
	uint8_t  type;   /* mt8370_pull_type_t              */
} mt8370_pull_desc_t;

static const mt8370_pull_desc_t mt8370_pull_desc[177] = {
	/* 0-11: i_base=1 (iocfg_rm), PU=0x00e0, PD=0x00b0 */
	/* pin  0 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0,  6, MT8370_PULL_PU_PD},
	/* pin  1 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0,  7, MT8370_PULL_PU_PD},
	/* pin  2 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0,  8, MT8370_PULL_PU_PD},
	/* pin  3 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0,  9, MT8370_PULL_PU_PD},
	/* pin  4 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 10, MT8370_PULL_PU_PD},
	/* pin  5 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 11, MT8370_PULL_PU_PD},
	/* pin  6 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 12, MT8370_PULL_PU_PD},
	/* pin  7 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 13, MT8370_PULL_PU_PD},
	/* pin  8 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 14, MT8370_PULL_PU_PD},
	/* pin  9 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 15, MT8370_PULL_PU_PD},
	/* pin 10 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 16, MT8370_PULL_PU_PD},
	/* pin 11 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 17, MT8370_PULL_PU_PD},
	/* 12-15: i_base=2 (iocfg_lt), PU=0x00d0, PD=0x00a0 */
	/* pin 12 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 12, MT8370_PULL_PU_PD},
	/* pin 13 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 13, MT8370_PULL_PU_PD},
	/* pin 14 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 14, MT8370_PULL_PU_PD},
	/* pin 15 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 15, MT8370_PULL_PU_PD},
	/* 16-17: i_base=3 (iocfg_lm), PU=0x0070, PD=0x0050 */
	/* pin 16 */ {MT8370_IOCFG_LM_BASE, 0x0070, 0x0050,  1, MT8370_PULL_PU_PD},
	/* pin 17 */ {MT8370_IOCFG_LM_BASE, 0x0070, 0x0050,  2, MT8370_PULL_PU_PD},
	/* 18-24: i_base=4 (iocfg_rt), PU=0x0080, PD=0x0060 */
	/* pin 18 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060,  3, MT8370_PULL_PU_PD},
	/* pin 19 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060,  5, MT8370_PULL_PU_PD},
	/* pin 20 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060,  4, MT8370_PULL_PU_PD},
	/* pin 21 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060,  6, MT8370_PULL_PU_PD},
	/* pin 22 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060,  0, MT8370_PULL_PU_PD},
	/* pin 23 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060,  1, MT8370_PULL_PU_PD},
	/* pin 24 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060,  2, MT8370_PULL_PU_PD},
	/* 25-41: i_base=1 (iocfg_rm), mixed offsets */
	/* pin 25 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0,  3, MT8370_PULL_PU_PD},
	/* pin 26 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0,  2, MT8370_PULL_PU_PD},
	/* pin 27 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0,  5, MT8370_PULL_PU_PD},
	/* pin 28 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0,  4, MT8370_PULL_PU_PD},
	/* pin 29 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0,  0, MT8370_PULL_PU_PD},
	/* pin 30 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0,  1, MT8370_PULL_PU_PD},
	/* pin 31 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0, 11, MT8370_PULL_PU_PD},
	/* pin 32 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0, 10, MT8370_PULL_PU_PD},
	/* pin 33 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0, 13, MT8370_PULL_PU_PD},
	/* pin 34 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0, 12, MT8370_PULL_PU_PD},
	/* pin 35 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0, 15, MT8370_PULL_PU_PD},
	/* pin 36 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0, 14, MT8370_PULL_PU_PD},
	/* pin 37 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 21, MT8370_PULL_PU_PD},
	/* pin 38 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 18, MT8370_PULL_PU_PD},
	/* pin 39 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 19, MT8370_PULL_PU_PD},
	/* pin 40 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 20, MT8370_PULL_PU_PD},
	/* pin 41 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 22, MT8370_PULL_PU_PD},
	/* 42-45: PUPD type, i_base=2 (iocfg_lt), PUPD=0x00c0 */
	/* pin 42 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000, 12, MT8370_PULL_PUPD},
	/* pin 43 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000, 13, MT8370_PULL_PUPD},
	/* pin 44 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000, 14, MT8370_PULL_PUPD},
	/* pin 45 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000, 15, MT8370_PULL_PUPD},
	/* pin 46: i_base=3 (iocfg_lm) */
	/* pin 46 */ {MT8370_IOCFG_LM_BASE, 0x0070, 0x0050,  0, MT8370_PULL_PU_PD},
	/* 47-49: i_base=1 (iocfg_rm) */
	/* pin 47 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 25, MT8370_PULL_PU_PD},
	/* pin 48 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 24, MT8370_PULL_PU_PD},
	/* pin 49 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 23, MT8370_PULL_PU_PD},
	/* 50-54: i_base=3 (iocfg_lm) */
	/* pin 50 */ {MT8370_IOCFG_LM_BASE, 0x0070, 0x0050,  5, MT8370_PULL_PU_PD},
	/* pin 51 */ {MT8370_IOCFG_LM_BASE, 0x0070, 0x0050,  4, MT8370_PULL_PU_PD},
	/* pin 52 */ {MT8370_IOCFG_LM_BASE, 0x0070, 0x0050,  3, MT8370_PULL_PU_PD},
	/* pin 53 */ {MT8370_IOCFG_LM_BASE, 0x0070, 0x0050,  6, MT8370_PULL_PU_PD},
	/* pin 54 */ {MT8370_IOCFG_LM_BASE, 0x0070, 0x0050,  7, MT8370_PULL_PU_PD},
	/* 55-56: i_base=1 (iocfg_rm) */
	/* pin 55 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 26, MT8370_PULL_PU_PD},
	/* pin 56 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 29, MT8370_PULL_PU_PD},
	/* 57-58: i_base=2 (iocfg_lt) */
	/* pin 57 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0,  6, MT8370_PULL_PU_PD},
	/* pin 58 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0,  9, MT8370_PULL_PU_PD},
	/* 59-62: i_base=1 (iocfg_rm) */
	/* pin 59 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 27, MT8370_PULL_PU_PD},
	/* pin 60 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 30, MT8370_PULL_PU_PD},
	/* pin 61 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 28, MT8370_PULL_PU_PD},
	/* pin 62 */ {MT8370_IOCFG_RM_BASE, 0x00e0, 0x00b0, 31, MT8370_PULL_PU_PD},
	/* 63-64: i_base=2 (iocfg_lt) */
	/* pin 63 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0,  7, MT8370_PULL_PU_PD},
	/* pin 64 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0, 10, MT8370_PULL_PU_PD},
	/* 65-68: i_base=4 (iocfg_rt), PU=0x0080, PD=0x0060 */
	/* pin 65 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060,  7, MT8370_PULL_PU_PD},
	/* pin 66 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060,  9, MT8370_PULL_PU_PD},
	/* pin 67 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060,  8, MT8370_PULL_PU_PD},
	/* pin 68 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060, 10, MT8370_PULL_PU_PD},
	/* 69-78: i_base=1 (iocfg_rm), PU=0x00f0, PD=0x00c0 */
	/* pin 69 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0,  1, MT8370_PULL_PU_PD},
	/* pin 70 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0,  0, MT8370_PULL_PU_PD},
	/* pin 71 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0,  5, MT8370_PULL_PU_PD},
	/* pin 72 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0,  4, MT8370_PULL_PU_PD},
	/* pin 73 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0,  2, MT8370_PULL_PU_PD},
	/* pin 74 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0,  3, MT8370_PULL_PU_PD},
	/* pin 75 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0,  7, MT8370_PULL_PU_PD},
	/* pin 76 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0,  6, MT8370_PULL_PU_PD},
	/* pin 77 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0,  9, MT8370_PULL_PU_PD},
	/* pin 78 */ {MT8370_IOCFG_RM_BASE, 0x00f0, 0x00c0,  8, MT8370_PULL_PU_PD},
	/* 79-82: i_base=4 (iocfg_rt) */
	/* pin 79 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060, 12, MT8370_PULL_PU_PD},
	/* pin 80 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060, 11, MT8370_PULL_PU_PD},
	/* pin 81 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060, 14, MT8370_PULL_PU_PD},
	/* pin 82 */ {MT8370_IOCFG_RT_BASE, 0x0080, 0x0060, 13, MT8370_PULL_PU_PD},
	/* 83-130: i_base=2 (iocfg_lt), mixed offsets */
	/* pin 83 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0, 16, MT8370_PULL_PU_PD},
	/* pin 84 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0, 15, MT8370_PULL_PU_PD},
	/* pin 85 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0, 17, MT8370_PULL_PU_PD},
	/* pin 86 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0, 19, MT8370_PULL_PU_PD},
	/* pin 87 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0, 18, MT8370_PULL_PU_PD},
	/* pin 88 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0, 20, MT8370_PULL_PU_PD},
	/* pin 89 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0, 22, MT8370_PULL_PU_PD},
	/* pin 90 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0, 21, MT8370_PULL_PU_PD},
	/* pin 91 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0, 23, MT8370_PULL_PU_PD},
	/* pin 92 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0,  3, MT8370_PULL_PU_PD},
	/* pin 93 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0,  2, MT8370_PULL_PU_PD},
	/* pin 94 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0,  5, MT8370_PULL_PU_PD},
	/* pin 95 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0,  4, MT8370_PULL_PU_PD},
	/* pin 96 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 31, MT8370_PULL_PU_PD},
	/* pin 97 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0,  0, MT8370_PULL_PU_PD},
	/* pin 98 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0,  8, MT8370_PULL_PU_PD},
	/* pin 99 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 30, MT8370_PULL_PU_PD},
	/* pin100 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0,  1, MT8370_PULL_PU_PD},
	/* pin101 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0,  0, MT8370_PULL_PU_PD},
	/* pin102 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0,  5, MT8370_PULL_PU_PD},
	/* pin103 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0,  3, MT8370_PULL_PU_PD},
	/* pin104 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0,  4, MT8370_PULL_PU_PD},
	/* pin105 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0,  1, MT8370_PULL_PU_PD},
	/* pin106 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0,  2, MT8370_PULL_PU_PD},
	/* pin107 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 21, MT8370_PULL_PU_PD},
	/* pin108 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 16, MT8370_PULL_PU_PD},
	/* pin109 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 22, MT8370_PULL_PU_PD},
	/* pin110 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 17, MT8370_PULL_PU_PD},
	/* pin111 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 18, MT8370_PULL_PU_PD},
	/* pin112 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 19, MT8370_PULL_PU_PD},
	/* pin113 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 20, MT8370_PULL_PU_PD},
	/* pin114 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 28, MT8370_PULL_PU_PD},
	/* pin115 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 23, MT8370_PULL_PU_PD},
	/* pin116 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 29, MT8370_PULL_PU_PD},
	/* pin117 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 24, MT8370_PULL_PU_PD},
	/* pin118 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 25, MT8370_PULL_PU_PD},
	/* pin119 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 26, MT8370_PULL_PU_PD},
	/* pin120 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 27, MT8370_PULL_PU_PD},
	/* 121-124: i_base=3 (iocfg_lm), PU=0x0070, PD=0x0050 */
	/* pin121 */ {MT8370_IOCFG_LM_BASE, 0x0070, 0x0050,  8, MT8370_PULL_PU_PD},
	/* pin122 */ {MT8370_IOCFG_LM_BASE, 0x0070, 0x0050, 11, MT8370_PULL_PU_PD},
	/* pin123 */ {MT8370_IOCFG_LM_BASE, 0x0070, 0x0050, 10, MT8370_PULL_PU_PD},
	/* pin124 */ {MT8370_IOCFG_LM_BASE, 0x0070, 0x0050,  9, MT8370_PULL_PU_PD},
	/* 125-130: i_base=2 (iocfg_lt) */
	/* pin125 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0,  6, MT8370_PULL_PU_PD},
	/* pin126 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0,  7, MT8370_PULL_PU_PD},
	/* pin127 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0,  8, MT8370_PULL_PU_PD},
	/* pin128 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0,  9, MT8370_PULL_PU_PD},
	/* pin129 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 10, MT8370_PULL_PU_PD},
	/* pin130 */ {MT8370_IOCFG_LT_BASE, 0x00d0, 0x00a0, 11, MT8370_PULL_PU_PD},
	/* 131-150: PUPD type, i_base=1 (iocfg_rm), PUPD=0x00d0 */
	/* pin131 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000,  1, MT8370_PULL_PUPD},
	/* pin132 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000,  2, MT8370_PULL_PUPD},
	/* pin133 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000,  9, MT8370_PULL_PUPD},
	/* pin134 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000, 10, MT8370_PULL_PUPD},
	/* pin135 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000, 11, MT8370_PULL_PUPD},
	/* pin136 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000, 12, MT8370_PULL_PUPD},
	/* pin137 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000, 13, MT8370_PULL_PUPD},
	/* pin138 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000, 14, MT8370_PULL_PUPD},
	/* pin139 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000, 15, MT8370_PULL_PUPD},
	/* pin140 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000, 16, MT8370_PULL_PUPD},
	/* pin141 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000,  3, MT8370_PULL_PUPD},
	/* pin142 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000,  4, MT8370_PULL_PUPD},
	/* pin143 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000,  5, MT8370_PULL_PUPD},
	/* pin144 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000,  6, MT8370_PULL_PUPD},
	/* pin145 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000,  7, MT8370_PULL_PUPD},
	/* pin146 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000,  8, MT8370_PULL_PUPD},
	/* pin147 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000, 18, MT8370_PULL_PUPD},
	/* pin148 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000, 19, MT8370_PULL_PUPD},
	/* pin149 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000, 17, MT8370_PULL_PUPD},
	/* pin150 */ {MT8370_IOCFG_RM_BASE, 0x00d0, 0x0000,  0, MT8370_PULL_PUPD},
	/* 151-162: PUPD type, i_base=2 (iocfg_lt), PUPD=0x00c0 */
	/* pin151 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000,  9, MT8370_PULL_PUPD},
	/* pin152 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000,  8, MT8370_PULL_PUPD},
	/* pin153 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000,  7, MT8370_PULL_PUPD},
	/* pin154 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000,  6, MT8370_PULL_PUPD},
	/* pin155 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000, 11, MT8370_PULL_PUPD},
	/* pin156 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000,  1, MT8370_PULL_PUPD},
	/* pin157 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000,  0, MT8370_PULL_PUPD},
	/* pin158 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000,  5, MT8370_PULL_PUPD},
	/* pin159 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000,  4, MT8370_PULL_PUPD},
	/* pin160 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000,  3, MT8370_PULL_PUPD},
	/* pin161 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000,  2, MT8370_PULL_PUPD},
	/* pin162 */ {MT8370_IOCFG_LT_BASE, 0x00c0, 0x0000, 10, MT8370_PULL_PUPD},
	/* 163-168: PUPD type, i_base=4 (iocfg_rt), PUPD=0x0070 */
	/* pin163 */ {MT8370_IOCFG_RT_BASE, 0x0070, 0x0000,  1, MT8370_PULL_PUPD},
	/* pin164 */ {MT8370_IOCFG_RT_BASE, 0x0070, 0x0000,  0, MT8370_PULL_PUPD},
	/* pin165 */ {MT8370_IOCFG_RT_BASE, 0x0070, 0x0000,  2, MT8370_PULL_PUPD},
	/* pin166 */ {MT8370_IOCFG_RT_BASE, 0x0070, 0x0000,  3, MT8370_PULL_PUPD},
	/* pin167 */ {MT8370_IOCFG_RT_BASE, 0x0070, 0x0000,  4, MT8370_PULL_PUPD},
	/* pin168 */ {MT8370_IOCFG_RT_BASE, 0x0070, 0x0000,  5, MT8370_PULL_PUPD},
	/* 169-174: PUPD type, i_base=3 (iocfg_lm), PUPD=0x0060 */
	/* pin169 */ {MT8370_IOCFG_LM_BASE, 0x0060, 0x0000,  1, MT8370_PULL_PUPD},
	/* pin170 */ {MT8370_IOCFG_LM_BASE, 0x0060, 0x0000,  0, MT8370_PULL_PUPD},
	/* pin171 */ {MT8370_IOCFG_LM_BASE, 0x0060, 0x0000,  2, MT8370_PULL_PUPD},
	/* pin172 */ {MT8370_IOCFG_LM_BASE, 0x0060, 0x0000,  3, MT8370_PULL_PUPD},
	/* pin173 */ {MT8370_IOCFG_LM_BASE, 0x0060, 0x0000,  4, MT8370_PULL_PUPD},
	/* pin174 */ {MT8370_IOCFG_LM_BASE, 0x0060, 0x0000,  5, MT8370_PULL_PUPD},
	/* 175-176: PU_PD_RSEL type treated as PU_PD, i_base=2 (iocfg_lt) */
	/* pin175 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0, 11, MT8370_PULL_PU_PD},
	/* pin176 */ {MT8370_IOCFG_LT_BASE, 0x00e0, 0x00b0, 12, MT8370_PULL_PU_PD},
};

static uint32_t reg_offset(const struct device *dev, reg_type_t reg_type);

static uint32_t reg_offset(const struct device *dev, reg_type_t reg_type)
{
	const gpio_mtk_config_t *gpio_config = dev->config;

	switch (reg_type) {
	case REG_TYPE_DIN:
		return (uint32_t)(GPIO_OFFSET_DIN_0 + (gpio_config->idx * GPIO_OFFSET_DIN_DELTA));
		break;

	case REG_TYPE_DOUT_SET:
		return (uint32_t)(GPIO_OFFSET_DOUT_0_SET +
				  (gpio_config->idx * GPIO_OFFSET_DOUT_SET_DELTA));
		break;

	case REG_TYPE_DOUT_CLR:
		return (uint32_t)(GPIO_OFFSET_DOUT_0_CLR +
				  (gpio_config->idx * GPIO_OFFSET_DOUT_CLR_DELTA));
		break;

	case REG_TYPE_DIR_SET:
		return (uint32_t)(GPIO_OFFSET_DIR_0_SET +
				  (gpio_config->idx * GPIO_OFFSET_DIR_SET_DELTA));
		break;

	case REG_TYPE_DIR_CLR:
		return (uint32_t)(GPIO_OFFSET_DIR_0_CLR +
				  (gpio_config->idx * GPIO_OFFSET_DIR_CLR_DELTA));
		break;

	default:
		break;
	}

	return INV_REG_OFFSET;
}

static int mt8370_pull_configure(const struct device *dev, gpio_pin_t pin, gpio_flags_t flags)
{
	/* pin is the absolute pin number across all ports, 0-176. */
	const gpio_mtk_config_t *gpio_config = dev->config;
	uint16_t abs_pin = (gpio_config->idx * 32u) + pin;
	uint32_t mask;
	uint32_t val;
	uintptr_t pu_addr;
	uintptr_t pd_addr;
 
	if (abs_pin >= ARRAY_SIZE(mt8370_pull_desc)) {
		return -EINVAL;
	}
 
	const mt8370_pull_desc_t *d = &mt8370_pull_desc[abs_pin];
	mask = BIT(d->bit);
	pu_addr = (uintptr_t)(d->base + d->pu_off);
 
	if (d->type == MT8370_PULL_PU_PD) {
		pd_addr = (uintptr_t)(d->base + d->pd_off);
 
		if ((flags & GPIO_PULL_UP) != 0) {
			/* Enable PU, clear PD. */
			val = sys_read32(pd_addr);
			sys_write32(val & ~mask, pd_addr);
			val = sys_read32(pu_addr);
			sys_write32(val | mask, pu_addr);
		} else if ((flags & GPIO_PULL_DOWN) != 0) {
			/* Clear PU, enable PD. */
			val = sys_read32(pu_addr);
			sys_write32(val & ~mask, pu_addr);
			val = sys_read32(pd_addr);
			sys_write32(val | mask, pd_addr);
		} else {
			/* Disable both. */
			val = sys_read32(pu_addr);
			sys_write32(val & ~mask, pu_addr);
			val = sys_read32(pd_addr);
			sys_write32(val & ~mask, pd_addr);
		}
	} else {
		val = sys_read32(pu_addr);
		if ((flags & GPIO_PULL_UP) != 0) {
			sys_write32(val | mask, pu_addr);  /* PUPD=1 → pull-up  */
		} else if ((flags & GPIO_PULL_DOWN) != 0) {
			sys_write32(val & ~mask, pu_addr); /* PUPD=0 → pull-down */
		}
	}
 
	return 0;
}

static DEVICE_API(gpio, gpio_mtk_driver_api) = {
	.pin_configure = gpio_mtk_pin_configure,
	.port_get_raw = gpio_mtk_port_get_raw,
	.port_set_masked_raw = gpio_mtk_port_set_masked_raw,
	.port_set_bits_raw = gpio_mtk_port_set_bits_raw,
	.port_clear_bits_raw = gpio_mtk_port_clr_bits_raw,
	.port_toggle_bits = gpio_mtk_port_toggle_bits,
	.pin_interrupt_configure = gpio_mtk_pin_interrupt_configure,
	.manage_callback = gpio_mtk_manage_callback,
};

#define GPIO_DECLARE_CFG(n)                                                                        \
	static gpio_mtk_data_t gpio_mtk_##n##_data;                                                \
                                                                                                   \
	static const gpio_mtk_config_t gpio_mtk_##n##_config = {                                   \
		.common = {.port_pin_mask = GPIO_PORT_PIN_MASK_FROM_DT_INST(0)},                   \
		DEVICE_MMIO_NAMED_ROM_INIT(reg_base, DT_INST_PARENT(n)),                           \
		.eint_dev = &DEVICE_DT_NAME_GET(DT_INST_PROP(n, interrupt_parent)),                \
		.idx = DT_INST_REG_ADDR(n),                                                        \
		.num_gpio_pins = DT_INST_PROP(n, ngpios),                                          \
		.gpio_pin_mask = GPIO_PORT_PIN_MASK_FROM_NGPIOS(DT_INST_PROP(n, ngpios)),          \
		.reg_offset = reg_offset,                                                          \
		.pull_configure = mt8370_pull_configure,                                           \
	};

#define GPIO_INIT(n)                                                                               \
	GPIO_DECLARE_CFG(n)                                                                        \
	DEVICE_DT_INST_DEFINE(n, &gpio_mtk_init, NULL, &gpio_mtk_##n##_data,                       \
			      &gpio_mtk_##n##_config, PRE_KERNEL_1, CONFIG_GPIO_INIT_PRIORITY,     \
			      &gpio_mtk_driver_api);

DT_INST_FOREACH_STATUS_OKAY(GPIO_INIT)
