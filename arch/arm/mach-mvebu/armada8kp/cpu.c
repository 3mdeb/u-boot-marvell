/*
 * Copyright (C) 2017 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	GPL-2.0
 * https://spdx.org/licenses
 */

/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/armv8/mmu.h>

DECLARE_GLOBAL_DATA_PTR;

#define MVEBU_AR_RFU_BASE		(MVEBU_REGISTER(0x6F0000))
#define MVEBU_RFU_GLOBL_SW_RST		(MVEBU_AR_RFU_BASE + 0x184)

#define SZ_1M				0x00100000
#define SZ_256M				0x10000000
#define SZ_1G				0x40000000
#define SZ_4G				0x100000000

/*
 * The following table includes all memory regions for Armada 8k Plus.
 */
static struct mm_region mvebu_mem_map[] = {
	/* Armada 8080 and 70x0 common memory regions start here */
	{
		/* RAM */
		.phys = 0x0UL,
		.virt = 0x0UL,
#ifdef CONFIG_MVEBU_PALLADIUM
		.size = 0x20000000UL,
#else
		.size = 0x80000000UL,
#endif
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	},
	{
		/* SRAM, MMIO regions - AP810 region */
		.phys = 0xec000000UL,
		.virt = 0xec000000UL,
		.size = 0x04000000UL,	/* 32MiB internal registers */
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE
	},
	{
		0,
	}
};

struct mm_region *mem_map = mvebu_mem_map;

void enable_caches(void)
{
	icache_enable();
	dcache_enable();
}

void reset_cpu(ulong ignored)
{
	writel(0x0, MVEBU_RFU_GLOBL_SW_RST);
}

int mvebu_dram_init(void)
{
	/*
	 * For now use 512MiB, later need to read the DRAM size using DRAM
	 * driver or from device tree that passed from ATF
	 */
	gd->ram_size = 0x20000000;

	return 0;
}

void mvebu_dram_init_banksize(void)
{
	/*
	 * Config 2 DRAM banks:
	 * Bank 0 - max size 4G - 1G
	 * Bank 1 - ram size - 4G + 1G
	 */
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	if (gd->ram_size <= SZ_4G - SZ_1G) {
		gd->bd->bi_dram[0].size = min(gd->ram_size,
					      (phys_size_t)(SZ_4G - SZ_1G));
		return;
	}

	gd->bd->bi_dram[0].size = SZ_4G - SZ_1G;
	gd->bd->bi_dram[1].start = SZ_4G;
	gd->bd->bi_dram[1].size = gd->ram_size - SZ_4G + SZ_1G;
}

#if defined(CONFIG_DISPLAY_BOARDINFO)
int print_cpuinfo(void)
{
	printf("No clock prints for now - not implemented yet\n");
	return 0;
}
#endif
