from code.old_code.addresses import MemoryAddressLocator
from pymem import Pymem
import const as const
import tomllib
import numpy as np
from numpy.typing import NDArray


def create_bytes_from_hex(hex_value: hex, size: int = 4) -> str:
    """Converts bytes to a string format needed to add variables to the machine code

    Args:
        hex_value: value to be converted
        size: how many bytes does the hex value represent

    Returns:
        string representation of bytes
    """
    return int(hex_value).to_bytes(size, "little")


def load_plants_from_config() -> NDArray:
    seed_slots = np.zeros(10, dtype=int)
    with open("config.toml", mode="rb") as f:
        seeds = tomllib.load(f)["seeds"]

    for i, (k, v) in enumerate(seeds.items()):
        seed_slots[i] = plant_to_number(v)

    return seed_slots


def plant_to_number(plant_name: str):
    return const.PLANTS[plant_name.lower()]


def get_locator_address(
    process: Pymem, memoryAddressLocator: MemoryAddressLocator, add_base: bool = True
):
    base, offsets = memoryAddressLocator
    address = base + process.base_address * add_base
    for offset in offsets:
        address = process.read_int(address)
        address += offset
    return address


def get_locator_value(
    process: Pymem, memoryAddressLocator: MemoryAddressLocator, add_base: int = 1
):
    address = get_locator_address(process, memoryAddressLocator, add_base)
    return process.read_int(address)


def get_locator_byte(
    process: Pymem, memoryAddressLocator: MemoryAddressLocator, add_base: int = 1
):
    address = get_locator_address(process, memoryAddressLocator, add_base)
    return process.read_bool(address)


def get_locator_float(
    process: Pymem, memoryAddressLocator: MemoryAddressLocator, add_base: int = 1
):
    address = get_locator_address(process, memoryAddressLocator, add_base)
    return process.read_float(address)


def get_action_interval():
    with open("config.toml", mode="rb") as f:
        delay = tomllib.load(f)["timings"]["action_interval"] * 100
    return delay
