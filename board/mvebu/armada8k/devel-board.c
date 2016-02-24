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

/* #define DEBUG */

#include <common.h>
#include <errno.h>
#include <asm/arch-mvebu/soc.h>
#include <asm/arch-mvebu/mpp.h>
#include <mvebu_chip_sar.h>
#include "../common/devel-board.h"
#include "../common/sar.h"

int mvebu_devel_board_init(void)
{
#ifdef CONFIG_MVEBU_MPP_BUS
	struct sar_val sar;
#endif

#ifdef CONFIG_MVEBU_SAR
	sar_init();
#endif

#ifdef CONFIG_MVEBU_MPP_BUS
	mvebu_sar_value_get(SAR_BOOT_SRC, &sar);

	if (sar.bootsrc.type == BOOTSRC_NAND)
		mpp_enable_bus("nand");

	if (sar.bootsrc.type == BOOTSRC_SPI) {
		char name[8];
		sprintf(name, "spi%d", sar.bootsrc.index);
		mpp_enable_bus(name);
	}
#endif /* CONFIG_MVEBU_MPP_BUS */

	return 0;
}

