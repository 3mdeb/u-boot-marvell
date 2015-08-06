/*
 * ***************************************************************************
 * Copyright (C) 2015 Marvell International Ltd.
 * ***************************************************************************
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ***************************************************************************
 */

#include <common.h>
#include <spl.h>
#include <fdtdec.h>
#include <asm/arch-mvebu/fdt.h>
#include <asm/arch-mvebu/comphy.h>
#include <asm/arch-mvebu/tools.h>
#ifdef CONFIG_MVEBU_SPL_DDR_OVER_PCI_SUPPORT
#include <asm/arch-mvebu/dram_over_pci.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

extern void static_dram_init(void);

static int setup_fdt(void)
{
#ifdef CONFIG_OF_CONTROL
#ifdef CONFIG_OF_EMBED
	/* Get a pointer to the FDT */
	gd->fdt_blob = __dtb_dt_begin;
#else
	#error "Support only embedded FDT mode in SPL"
#endif
#endif
	return 0;
}

void board_init_f(ulong silent)
{
	gd = &gdata;
	gd->baudrate = CONFIG_BAUDRATE;

#ifndef CONFIG_TARGET_ARMADA_38X
	if (silent)
		gd->flags |= GD_FLG_SILENT;
#endif

	setup_fdt();
	preloader_console_init();

#ifndef CONFIG_MVEBU_SPL_DDR_OVER_PCI_SUPPORT
/* when DDR over PCIE is enabled, add delay before and after the comphy_init
   to verify that the PCIE card init done, before setting the comphy to avoid
   collisions. and no ddr init require */
#if CONFIG_MVEBU_COMPHY_SUPPORT
	if (comphy_init(gd->fdt_blob))
		error("COMPHY initialization failed\n");
#endif
#ifndef CONFIG_PALLADIUM
	static_dram_init();
#endif
#else
	dram_over_pci_init(gd->fdt_blob);
#endif /* CONFIG_MVEBU_SPL_DDR_OVER_PCI_SUPPORT */
#ifdef CONFIG_MVEBU_SPL_MEMORY_TEST
	if (run_memory_test())
		printf("**** DRAM test failed ****\n");
#endif
}
