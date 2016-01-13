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
#include <netdev.h>
#include <asm/arch/mbus_reg.h>
#include <asm/arch-mvebu/mbus.h>

/* NB warm reset */
#define MVEBU_NB_WARM_RST_REG	(MVEBU_GPIO_NB_REG_BASE + 0x40)
/* NB warm reset magic number, write it to MVEBU_GPIO_NB_RST_REG triggers warm reset */
#define MVEBU_NB_WARM_RST_MAGIC_NUM	(0x1d1e)

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
	/* TO-DO, get soc ID from PCIe register */
	/* in ArmadaLP, there is no device ID register, like A38x,
	    it needs to be got from PCIe register, like A370 and AXP */
	u32 id = 0x9991;
	return id;
}

void soc_init(void)
{
	/* Do early SOC specific init here */

	/* now there is no timer/MPP driver,
	  * currently we put all this kind of
	  * configuration here, and will remove
	  * this after official driver is ready
	  */
#ifdef CONFIG_PALLADIUM

#ifdef CONFIG_I2C_MV
	/* 0xD0013830[10] = 1'b0 (select GPIO pins to use for I2C_1) */
	writel((readl(0xd0013830) & ~(1 << 10)), 0xd0013830);
#endif /* CONFIG_I2C_MV */

#endif /* CONFIG_PALLADIUM */
	return;
}

#ifdef CONFIG_MVNETA
/**
 * cpu_eth_init()
 *	invoke mvneta_initialize for each port, which is the initialization
 *	entrance of mvneta driver.
 *
 * Input:
 *	bis - db_info
 *
 * Return:
 *	0 - cool
 */
int cpu_eth_init(bd_t *bis)
{
	/* init neta module */
	if (1 != mvneta_initialize(bis)) {
		error("failed to init mvneta\n");
		return 1;
	}
	/* in ArmadaLP, there is a new register, internal Register Base Address, for GBE to
	    access other internal Register. since GBE is on South bridge, not the same island
	    as CPU, here we set internal reg base value 0xf100000 into it.
	    NETA drvier initialization does not rely on this configuration, so do it after
	    mvneta_initialize() */
	writel(MVEBU_REGS_BASE, MVEBU_ARLP_GBE0_INTERNAL_REG_BASE);
	writel(MVEBU_REGS_BASE, MVEBU_ARLP_GBE1_INTERNAL_REG_BASE);

	return 0;
}
#endif /* CONFIG_MVNETA */

#ifdef CONFIG_I2C_MV
void i2c_clk_enable(void)
{
	/* i2c is enabled by default,
	  * but need this empty routine
	  * to pass compilation.
	*/
	return;
}

#endif /* CONFIG_I2C_MV */

int dram_init(void)
{
#ifdef CONFIG_PALLADIUM
	/* NO DRAM init sequence in Pallaidum, so set static DRAM size of 256MB */
	gd->ram_size = 0x20000000;
#else
	struct mbus_win_map win_map;
	struct mbus_win *win;
	int cs;
	u32 size;

	gd->ram_size = 0;

	/* get DRAM window configuration from MBUS */
	mbus_win_map_build(&win_map);

	for (cs = 0, win = &win_map.mbus_windows[cs]; cs < win_map.mbus_win_num; cs++, win++) {
			gd->bd->bi_dram[cs].start = win->base_addr;
			size = (win->win_size + 1) * MBUS_CR_WIN_SIZE_ALIGNMENT;
			gd->bd->bi_dram[cs].size = size;

			gd->ram_size += size;

			debug("DRAM bank %d base 0x%08x size 0x%x ", cs, (u32)win->base_addr, size);
	}

	if (gd->ram_size == 0) {
		error("No DRAM banks detected");
		return 1;
	}
#endif

	return 0;
}

void reset_cpu(ulong ignored)
{
	/* write magic number of 0x1d1e to North Bridge Warm Reset register
	   to trigger warm reset */
	writel(MVEBU_NB_WARM_RST_MAGIC_NUM, MVEBU_NB_WARM_RST_REG);
}
