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

#define DEBUG

#include <common.h>
#include <asm/io.h>
#include <asm/arch-mvebu/soc.h>
#include <asm/arch-mvebu/unit-info.h>
#include <asm/arch-armada8k/armada8k.h>
#include <asm/arch/regs-base.h>

#define ADDRESS_SHIFT			(20)
#define MAX_CCU_WINDOWS			(8)
#define DRAM_0_TID			0x03
#define CCU_WIN_CR_OFFSET(win)		(MVEBU_ADEC_AP_BASE + 0x0 + (0x10 * win))
#define CCU_TARGET_ID_OFFSET		(8)
#define CCU_TARGET_ID_MASK		(0x7F)
#define CCU_WIN_ALR_OFFSET(win)		(MVEBU_ADEC_AP_BASE + 0x8 + (0x10 * win))
#define CCU_WIN_AHR_OFFSET(win)		(MVEBU_ADEC_AP_BASE + 0xC + (0x10 * win))

int soc_early_init_f(void)
{
	return 0;
}

int soc_get_rev(void)
{
	/* This should read the soc rev from some register*/
	return 0;
}

int soc_get_id(void)
{
	/* This should read the soc id from some register*/
	return CONFIG_ARMADA_8K_SOC_ID;
}

#ifdef CONFIG_MVEBU_PCIE
static void soc_pcie_init(void)
{
	u32 reg;

	reg = readl(MVEBU_PCIE_MAC_CTL);

	/* Set PCIe transactions towards A2 as:
	 * - read allocate
	 * - write non alocate
	 * - outer sharable */
	reg &= ~(0xF << 8);
	reg |= (0x7 << 8);

	/* Set the Port x4 */
	reg |= (1 << 14);

	/* Enable PCIe unit */
	reg = 1;

	writel(reg, MVEBU_PCIE_MAC_CTL);
}
#endif

struct mvebu_soc_family *soc_init(void)
{
#ifdef CONFIG_MVEBU_PCIE
	soc_pcie_init();
#endif

	return &a8k_family_info;
}

int dram_init(void)
{
#ifdef CONFIG_PALLADIUM
	gd->ram_size = 0x20000000;
#else
	u32 alr, ahr;
	u32 target_id, ctrl;
	u32 win;

	for (win = 0; win < MAX_CCU_WINDOWS; win++) {
		ctrl = readl(CCU_WIN_CR_OFFSET(win));
		target_id = (ctrl >> CCU_TARGET_ID_OFFSET) & CCU_TARGET_ID_MASK;

		if (target_id == DRAM_0_TID) {
			alr = readl(CCU_WIN_ALR_OFFSET(win)) << ADDRESS_SHIFT;
			ahr = readl(CCU_WIN_AHR_OFFSET(win)) << ADDRESS_SHIFT;
			gd->ram_size = ahr - alr + 1;
			gd->bd->bi_dram[0].size = gd->ram_size;
			gd->bd->bi_dram[0].start = alr;

			debug("DRAM base 0x%08x size 0x%x\n", alr, (uint)gd->ram_size);
		}
	}
#endif

	return 0;
}
