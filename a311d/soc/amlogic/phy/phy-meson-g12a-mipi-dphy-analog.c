// SPDX-License-Identifier: GPL-2.0
/*
 * Meson G12A MIPI DSI Analog PHY
 *
 * Copyright (C) 2018 Amlogic, Inc. All rights reserved
 * Copyright (C) 2020 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */
#include <dt-bindings/phy/phy.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/module.h>
#include <linux/phy/phy.h>
#include <linux/regmap.h>
#include <linux/delay.h>
#include <linux/mfd/syscon.h>
#include <linux/platform_device.h>

#define HHI_MIPI_CNTL0 0x00
#define HHI_MIPI_CNTL0_DIF_REF_CTL1 GENMASK(31, 16)
#define HHI_MIPI_CNTL0_DIF_REF_CTL0 GENMASK(15, 0)

#define HHI_MIPI_CNTL1 0x04
#define HHI_MIPI_CNTL1_BANDGAP BIT(16)
#define HHI_MIPI_CNTL2_DIF_REF_CTL2 GENMASK(15, 0)

#define HHI_MIPI_CNTL2 0x08
#define HHI_MIPI_CNTL2_DIF_TX_CTL1 GENMASK(31, 16)
#define HHI_MIPI_CNTL2_CH_EN GENMASK(15, 11)
#define HHI_MIPI_CNTL2_DIF_TX_CTL0 GENMASK(10, 0)

#define DSI_LANE_0 BIT(4)
#define DSI_LANE_1 BIT(3)
#define DSI_LANE_CLK BIT(2)
#define DSI_LANE_2 BIT(1)
#define DSI_LANE_3 BIT(0)

struct phy_g12a_mipi_dphy_analog_priv {
    struct phy *phy;
    struct regmap *regmap;
    struct phy_configure_opts_mipi_dphy config;
};

static int phy_g12a_mipi_dphy_analog_configure(struct phy *phy, union phy_configure_opts *opts)
{
    struct phy_g12a_mipi_dphy_analog_priv *priv = phy_get_drvdata(phy);
    int ret;

    ret = phy_mipi_dphy_config_validate(&opts->mipi_dphy);
    if (ret) {
        return ret;
    }

    memcpy(&priv->config, opts, sizeof(priv->config));

    return 0;
}

static int phy_g12a_mipi_dphy_analog_power_on(struct phy *phy)
{
    struct phy_g12a_mipi_dphy_analog_priv *priv = phy_get_drvdata(phy);
    unsigned int reg;

    regmap_write(priv->regmap, HHI_MIPI_CNTL0,
                 FIELD_PREP(HHI_MIPI_CNTL0_DIF_REF_CTL0, 0x8) | FIELD_PREP(HHI_MIPI_CNTL0_DIF_REF_CTL1, 0xa487));

    regmap_write(priv->regmap, HHI_MIPI_CNTL1, FIELD_PREP(HHI_MIPI_CNTL2_DIF_REF_CTL2, 0x2e) | HHI_MIPI_CNTL1_BANDGAP);

    regmap_write(priv->regmap, HHI_MIPI_CNTL2,
                 FIELD_PREP(HHI_MIPI_CNTL2_DIF_TX_CTL0, 0x45a) | /* UNIONMAN modify: 0x459 */
                     FIELD_PREP(HHI_MIPI_CNTL2_DIF_TX_CTL1, 0x2680));

    reg = DSI_LANE_CLK;
    // UNIONMAN: Maybe there is a bug in SoC, we must enable all lanes
    unsigned int lanes = 4;
    switch (lanes) { /* priv->config.lanes */
        case 4L:
            reg |= DSI_LANE_3;
            fallthrough;
        case 3L:
            reg |= DSI_LANE_2;
            fallthrough;
        case 2L:
            reg |= DSI_LANE_1;
            fallthrough;
        case 1:
            reg |= DSI_LANE_0;
            break;
        default:
            reg = 0;
    }

    regmap_update_bits(priv->regmap, HHI_MIPI_CNTL2, HHI_MIPI_CNTL2_CH_EN, FIELD_PREP(HHI_MIPI_CNTL2_CH_EN, reg));

    return 0;
}

static int phy_g12a_mipi_dphy_analog_power_off(struct phy *phy)
{
    struct phy_g12a_mipi_dphy_analog_priv *priv = phy_get_drvdata(phy);

    regmap_write(priv->regmap, HHI_MIPI_CNTL0, 0);
    regmap_write(priv->regmap, HHI_MIPI_CNTL1, 0);
    regmap_write(priv->regmap, HHI_MIPI_CNTL2, 0);

    return 0;
}

static const struct phy_ops phy_g12a_mipi_dphy_analog_ops = {
    .configure = phy_g12a_mipi_dphy_analog_configure,
    .power_on = phy_g12a_mipi_dphy_analog_power_on,
    .power_off = phy_g12a_mipi_dphy_analog_power_off,
    .owner = THIS_MODULE,
};

static int phy_g12a_mipi_dphy_analog_probe(struct platform_device *pdev)
{
    struct phy_provider *phy;
    struct device *dev = &pdev->dev;
    struct phy_g12a_mipi_dphy_analog_priv *priv;
    struct device_node *np = dev->of_node;
    struct regmap *map;
    int ret;

    priv = devm_kmalloc(dev, sizeof(*priv), GFP_KERNEL);
    if (!priv) {
        return -ENOMEM;
    }

    /* Get the hhi system controller node */
    map = syscon_node_to_regmap(of_get_parent(dev->of_node));
    if (IS_ERR(map)) {
        dev_err(dev, "failed to get HHI regmap\n");
        return PTR_ERR(map);
    }

    priv->regmap = map;

    priv->phy = devm_phy_create(dev, np, &phy_g12a_mipi_dphy_analog_ops);
    if (IS_ERR(priv->phy)) {
        ret = PTR_ERR(priv->phy);
        if (ret != -EPROBE_DEFER) {
            dev_err(dev, "failed to create PHY\n");
        }
        return ret;
    }

    phy_set_drvdata(priv->phy, priv);
    dev_set_drvdata(dev, priv);

    phy = devm_of_phy_provider_register(dev, of_phy_simple_xlate);

    dev_info(dev, "create PHY : %d\n", PTR_ERR_OR_ZERO(phy));

    return PTR_ERR_OR_ZERO(phy);
}

static const struct of_device_id phy_g12a_mipi_dphy_analog_of_match[] = {
    {
        .compatible = "amlogic,g12a-mipi-dphy-analog",
    },
    {},
};
MODULE_DEVICE_TABLE(of, phy_g12a_mipi_dphy_analog_of_match);

static struct platform_driver phy_g12a_mipi_dphy_analog_driver = {
    .probe = phy_g12a_mipi_dphy_analog_probe,
    .driver =
        {
            .name = "phy-meson-g12a-mipi-dphy-analog",
            .of_match_table = phy_g12a_mipi_dphy_analog_of_match,
        },
};
module_platform_driver(phy_g12a_mipi_dphy_analog_driver);

MODULE_AUTHOR("Neil Armstrong <narmstrong@baylibre.com>");
MODULE_DESCRIPTION("Meson G12A MIPI Analog D-PHY driver");
MODULE_LICENSE("GPL v2");
