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

/*#define DEBUG */
#include <common.h>
#include <asm/io.h>
#include <asm/arch-mvebu/comphy.h>
#include <asm/arch-mvebu/comphy_hpipe.h>
#include <asm/arch-mvebu/mvebu.h>

#define COMMON_PHY_CONFIGURATION1_REG           0x0
#define COMMON_PHY_CONFIGURATION2_REG           0x4
#define COMMON_PHY_CONFIGURATION4_REG           0xC

struct comphy_mux_data a38x_comphy_mux_data[] = {
	{4, {{UNCONNECTED, 0x0}, {PEX0, 0x1}, {SATA0, 0x2}, {SGMII0, 0x3} } },
	{8, {{UNCONNECTED, 0x0}, {PEX0, 0x1}, {PEX0, 0x2}, {SATA0, 0x3},
		{SGMII0, 0x4}, {SGMII1, 0x5}, {USB3_HOST0, 0x6}, {QSGMII, 0x7} } },
	{5, {{UNCONNECTED, 0x0}, {PEX1, 0x1}, {PEX0, 0x2}, {SATA1, 0x3}, {SGMII1, 0x4} } },
	{7, {{UNCONNECTED, 0x0}, {PEX3, 0x1}, {PEX0, 0x2}, {SATA3, 0x3}, {SGMII2, 0x4},
		{USB3_HOST0, 0x5}, {USB3_DEVICE, 0x6} } },
	{7, {{UNCONNECTED, 0x0}, {PEX1, 0x1}, {SATA1, 0x2}, {SGMII1, 0x3}, {USB3_HOST0, 0x4},
		{USB3_DEVICE, 0x5}, {SATA2, 0x6} } },
	{6, {{UNCONNECTED, 0x0}, {PEX2, 0x1}, {SATA2, 0x2}, {SGMII2, 0x3}, {USB3_HOST1, 0x4},
		{USB3_DEVICE, 0x5} } },
};

void reg_set(u32 addr, u32 mask, u32 data)
{
	u32 reg_data;
	debug("Write to address = %#010x, data = %#010x (mask = %#010x) - ", addr, data, mask);
	debug("old value = %#010x ==> ", readl(addr));
	reg_data = readl(addr);
	reg_data &= ~mask;
	reg_data |= data;
	writel(reg_data, addr);
	debug("new value %#010x\n", readl(addr));
}

/* comphy_mux_check_config
 * description: this function passes over the COMPHY lanes and check if the type
 *              is valid for specific lane. If the type is not valid, the function
 *              update the struct and set the type of the lane as UNCONNECTED */
static void comphy_mux_check_config(struct comphy_mux_data *mux_data,
		struct comphy_map *comphy_map_data, int comphy_max_lanes)
{
	struct comphy_mux_options *ptr_mux_opt;
	int lane, opt, valid;
	debug_enter();

	for (lane = 0; lane < comphy_max_lanes; lane++, comphy_map_data++, mux_data++) {
		ptr_mux_opt = mux_data->mux_values;
		for (opt = 0, valid = 0; opt < mux_data->max_lane_values; opt++, ptr_mux_opt++) {
			if (ptr_mux_opt->type == comphy_map_data->type) {
				valid = 1;
				break;
			}
		}
		if (valid == 0) {
			debug("lane number %d, had invalid type %d\n", lane, comphy_map_data->type);
			debug("set lane %d as type %d\n", lane, UNCONNECTED);
			comphy_map_data->type = UNCONNECTED;
		} else {
			debug("lane number %d, has type %d\n", lane, comphy_map_data->type);
		}
	}
	debug_exit();
}

static u32 comphy_mux_get_mux_value(struct comphy_mux_data *mux_data, enum phy_type type, int lane)
{
	struct comphy_mux_options *ptr_mux_opt;
	int opt;
	ptr_mux_opt = mux_data->mux_values;
	for (opt = 0 ; opt < mux_data->max_lane_values; opt++, ptr_mux_opt++)
		if (ptr_mux_opt->type == type)
			return ptr_mux_opt->mux_value;
	return 0;
}

static void comphy_mux_reg_write(struct comphy_mux_data *mux_data,
		struct comphy_map *comphy_map_data, int comphy_max_lanes, u32 base_addr, u32 bitcount)
{
	u32 lane, value, offset, mask;
	u32 comphy_selector_base = base_addr + 0xfc;

	debug_enter();
	for (lane = 0; lane < comphy_max_lanes; lane++, comphy_map_data++, mux_data++) {
		offset = lane * bitcount;
		mask = (((1 << bitcount) - 1) << offset);
		value = (comphy_mux_get_mux_value(mux_data, comphy_map_data->type, lane) << offset);
		reg_set(comphy_selector_base, mask, value);
	}
	debug_exit();
}

static void comphy_mux_init(struct chip_serdes_phy_config *ptr_chip_cfg, struct comphy_map *comphy_map_data)
{
	struct comphy_mux_data *mux_data;
	u32 comphy_base, mux_bitcount;
	u32 comphy_max_lanes;

	debug_enter();

	comphy_max_lanes = ptr_chip_cfg->comphy_lanes_count;
	mux_data = ptr_chip_cfg->mux_data;
	comphy_base = ptr_chip_cfg->comphy_base_addr;
	mux_bitcount = ptr_chip_cfg->comphy_mux_bitcount;

	/* check if the configuration is valid */
	comphy_mux_check_config(mux_data, comphy_map_data, comphy_max_lanes);
	/* Init COMPHY selectors */
	comphy_mux_reg_write(mux_data, comphy_map_data, comphy_max_lanes, comphy_base, mux_bitcount);

	debug_exit();
}

static int comphy_pcie_power_up(u32 pex_index, u32 comphy_addr, u32 hpipe_addr)
{
	debug_enter();

	comphy_pcie_unit_general_config(pex_index);

	/* power up sequence */
	debug("**** start of PCIe comphy power up sequence ****\n");
	reg_set(comphy_addr + COMMON_PHY_CONFIGURATION1_REG, 0x3FC7F806, 0x4471804);
	reg_set(comphy_addr + COMMON_PHY_CONFIGURATION2_REG, 0x5C, 0x58);
	reg_set(comphy_addr + COMMON_PHY_CONFIGURATION4_REG, 0x3, 0x1);
	reg_set(comphy_addr + COMMON_PHY_CONFIGURATION1_REG, 0x7800, 0x6000);
	reg_set(hpipe_addr + HPIPE_GLOBAL_CLK_CTRL, 0x3D, 0x35);
	reg_set(hpipe_addr + HPIPE_GLOBAL_MISC_CTRL, 0xC0, 0x0);
	reg_set(hpipe_addr + HPIPE_MISC_REG, 0x4C0, 0x80);
	udelay(20);

	/* TODO: Add configuration for 20Mhz */
	/* configuration seq for REF_CLOCK_100MHz */
	debug("**** start of PCIe comphy ref clock configuration ****\n");
	reg_set(hpipe_addr + HPIPE_POWER_AND_PLL_CTRL_REG, 0x1F, 0x0);
	reg_set(hpipe_addr + HPIPE_MISC_REG, 0x400, 0x0);
	reg_set(hpipe_addr + HPIPE_GLOBAL_PM_CTRL, 0xFF, 0x1E);
	reg_set(hpipe_addr + HPIPE_INTERFACE_REG, 0xC00, 0x400);
	udelay(20);

	/* PEX - electrical configuration seq */
	debug("**** start of PCIe electrical configuration sequence ****\n");
	reg_set(hpipe_addr + HPIPE_G1_SETTINGS_0_REG, 0xF000, 0xB000);
	reg_set(hpipe_addr + HPIPE_G1_SETTINGS_1_REG, 0x3FF, 0x3C9);
	reg_set(hpipe_addr + HPIPE_G1_SETTINGS_3_REG, 0xFF, 0xCF);
	reg_set(hpipe_addr + HPIPE_G2_SETTINGS_1_REG, 0x3FF, 0x3C9);
	reg_set(hpipe_addr + HPIPE_G2_SETTINGS_3_REG, 0xFF, 0xAF);
	reg_set(hpipe_addr + HPIPE_G2_SETTINGS_4_REG, 0x300, 0x300);
	reg_set(hpipe_addr + HPIPE_DFE_REG0, 0x8000, 0x8000);
	reg_set(hpipe_addr + HPIPE_PCIE_REG1, 0xF80, 0xD00);
	reg_set(hpipe_addr + HPIPE_PCIE_REG3, 0xFF00, 0xAF00);
	reg_set(hpipe_addr + HPIPE_LANE_CFG4_REG, 0x8, 0x8);
	reg_set(hpipe_addr + HPIPE_VTHIMPCAL_CTRL_REG, 0xFF00, 0x3000);
	udelay(20);

	/* PEX - TX configuration sequence 2 */
	debug("**** start of PCIe TX configuration sequence 2 ****\n");
	reg_set(hpipe_addr + HPIPE_RESET_DFE_REG, 0x401, 0x401);
	udelay(20);

	/* PEX - TX configuration sequence 3 */
	debug("**** start of PCIe TX configuration sequence 3 ****\n");
	reg_set(hpipe_addr + HPIPE_RESET_DFE_REG, 0x401, 0x0);
	udelay(20000);
	reg_set(hpipe_addr + HPIPE_RX_REG3, 0xFF, 0xDC);
	reg_set(hpipe_addr + HPIPE_RX_REG3, 0x100, 0x100);
	reg_set(hpipe_addr + HPIPE_RX_REG3, 0x100, 0x0);

	/* PEX - TX configuration sequence 1 */
	debug("**** start of PCIe TX configuration sequence 1 ****\n");
	reg_set(hpipe_addr + HPIPE_GLOBAL_CLK_CTRL, 0x1, 0x0);
	udelay(20000);

	debug_exit();
	return readl(hpipe_addr + HPIPE_LANE_STATUS0_REG) & 0x1;
}

int comphy_a38x_init(struct chip_serdes_phy_config *ptr_chip_cfg, struct comphy_map *serdes_map)
{
	struct comphy_map *ptr_comphy_map;
	u32 comphy_base_addr, hpipe3_base_addr;
	u32 comphy_max_count, lane, ret = 0;
	bool is_pex_enabled = false;

	debug_enter();

	/* PHY mux initialize */
	ptr_chip_cfg->mux_data = a38x_comphy_mux_data;
	if (ptr_chip_cfg->comphy_base_addr != 0)
		comphy_mux_init(ptr_chip_cfg, serdes_map);

	comphy_max_count = ptr_chip_cfg->comphy_lanes_count;
	comphy_base_addr = ptr_chip_cfg->comphy_base_addr;
	hpipe3_base_addr = ptr_chip_cfg->hpipe3_base_addr;
	for (lane = 0, ptr_comphy_map = serdes_map; lane < comphy_max_count; lane++, ptr_comphy_map++) {
		debug("Initialize serdes number %d\n", lane);
		debug("Serdes type = 0x%x\n", ptr_comphy_map->type);
		switch (ptr_comphy_map->type) {
		case UNCONNECTED:
			continue;
		case PEX0:
		case PEX1:
		case PEX2:
		case PEX3:
			is_pex_enabled = true;
			/* TODO: add support for PEX by4 initialization */
			ret = comphy_pcie_power_up(ptr_comphy_map->type - PEX0,
					comphy_base_addr + 0x28 * lane, hpipe3_base_addr + 0x800 * lane);
			break;
		default:
			debug("Unknown SerDes type, skip initialize SerDes %d\n", lane);
			continue;
		}
		if (ret == 0)
			printf("PLL is not locked - Failed to initialize lane %d\n", lane);
	}

	if (is_pex_enabled) {
		/* PEX unit configuration set */
		comphy_pcie_config_set(comphy_max_count, serdes_map);
		comphy_pcie_config_detect(comphy_max_count, serdes_map);
	}

	debug_exit();
	return 0;
}
