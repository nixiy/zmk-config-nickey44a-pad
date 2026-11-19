"""Validate built yuPad43 firmware configuration; hardware checks remain manual."""

import argparse
from pathlib import Path
import re
import struct
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--zephyr-base", required=True, type=Path)
    parser.add_argument("--left-build", required=True, type=Path)
    parser.add_argument("--right-build", required=True, type=Path)
    args = parser.parse_args()
    sys.path.insert(0, str(args.zephyr_base / "scripts/dts/python-devicetree/src"))
    from devicetree import dtlib

    def cells(prop):
        return list(struct.unpack(">" + "I" * (len(prop.value) // 4), prop.value))

    trees = []
    for side, build in (("left", args.left_build), ("right", args.right_build)):
        dt = dtlib.DT(str(build / "zephyr/zephyr.dts"))
        trees.append(dt)
        cfg = dict(re.findall(r"^(CONFIG_\w+)=(.+)$",
                              (build / "zephyr/.config").read_text(), re.MULTILINE))
        if side == "left":
            assert cfg.get("CONFIG_INPUT_IQS9151") != "y"
            assert cfg.get("CONFIG_ZMK_SPLIT_ROLE_CENTRAL") != "y"
            assert cfg.get("CONFIG_ZMK_SPLIT") == "y"
            assert cfg.get("CONFIG_ZMK_BATTERY_REPORTING") == "y"
            assert cfg.get("CONFIG_ZMK_SLEEP") == "y"
            assert "yupad43" not in dt.label2node
            assert "left_yupad43_split" not in dt.label2node
            assert (build / "zephyr/zmk.uf2").is_file(), side
            continue
        for name in ("INPUT_IQS9151", "GPIO", "I2C", "INPUT", "ZMK_POINTING",
                     "ZMK_SPLIT",
                     "ZMK_SLEEP", "ZMK_BATTERY_REPORTING",
                     "INPUT_IQS9151_1F_TAP_ENABLE", "INPUT_IQS9151_2F_TAP_ENABLE"):
            assert cfg.get("CONFIG_" + name) == "y", (side, name)
        for name in ("INPUT_TPS43", "INPUT_IQS9151_CURSOR_INERTIA_ENABLE",
                     "INPUT_IQS9151_SCROLL_INERTIA_ENABLE"):
            assert cfg.get("CONFIG_" + name) != "y", (side, name)
        assert cfg["CONFIG_INPUT_IQS9151_RESOLUTION_X"] == "1300"
        assert cfg["CONFIG_INPUT_IQS9151_RESOLUTION_Y"] == "1192"
        assert cfg["CONFIG_INPUT_IQS9151_ROTATE_90"] == "y"
        assert (cfg.get("CONFIG_ZMK_SPLIT_ROLE_CENTRAL") == "y") == (side == "right")
        if side == "right":
            for name in ("ZMK_STUDIO", "ZMK_INPUT_LISTENER", "ZMK_POINTING_SMOOTH_SCROLLING",
                         "ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_PROXY",
                         "ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING"):
                assert cfg.get("CONFIG_" + name) == "y", name
        pad = dt.label2node["yupad43"]
        assert pad.props["compatible"].to_string() == "azoteq,iqs9151"
        assert pad.props["reg"].to_nums() == [0x56]
        gpio0 = dt.label2node["gpio0"].props["phandle"].to_num()
        assert cells(pad.props["irq-gpios"]) == [gpio0, 15, 17]
        assert "rst-gpios" not in pad.props
        assert "yupad43_reset_idle" not in dt.label2node["gpio0"].nodes
        for state in ("default", "sleep"):
            group = dt.label2node["yupad43_i2c0_" + state].nodes["group1"]
            assert [v & 0x1ff for v in group.props["psels"].to_nums()] == [9, 33]
        assert pad.parent.props["clock-frequency"].to_num() == 100000
        # Active GPIO consumers must not share the pad's RDY or reset lines.
        for node in dt.node_iter():
            if any(p.props.get("status") and p.props["status"].to_string() == "disabled"
                   for p in ancestors(node)):
                continue
            for prop in node.props.values():
                if prop.name.endswith("-gpios"):
                    values = cells(prop)
                    for index in range(0, len(values), 3):
                        if values[index:index + 2] in ([gpio0, 15], [gpio0, 19]):
                            assert node is pad and prop.name == "irq-gpios", node.path
        assert not any("zmk,input-processor-touch-swipe" in n.props["compatible"].to_strings()
                       or "zmk,input-processor-touch-inertia" in n.props["compatible"].to_strings()
                       for n in dt.node_iter() if "compatible" in n.props)
        assert (build / "zephyr/zmk.uf2").is_file(), side

    left, right = trees
    assert "left_yupad43_split" not in right.label2node
    listeners = [n for n in right.node_iter() if "compatible" in n.props and
                 "zmk,input-listener" in n.props["compatible"].to_strings() and
                 n.props["device"].to_node() is right.label2node["yupad43"]]
    assert listeners == [right.label2node["yupad43_listener"]]
    assert "yupad43_ignore_swipes" not in right.label2node
    for label, device in (("yupad43_listener", right.label2node["yupad43"]),):
        listener = right.label2node[label]
        assert listener.props["device"].to_node() is device
        assert cells(listener.props["input-processors"]) == [
            right.label2node["zip_scroll_scaler"].props["phandle"].to_num(), 1, 12]
    print("PASS: yuPad43 pins, active-low RDY, address, rotation and firmware outputs")
    print("PASS: right-only pad, single listener, basic input, battery/sleep/Studio")
    print("Hardware direction, calibration, BLE and sleep/wake checks remain manual.")


def ancestors(node):
    while node is not None:
        yield node
        node = node.parent


if __name__ == "__main__":
    main()
