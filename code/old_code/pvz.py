from pymem import Pymem
from pymem.exception import MemoryReadError
import code.old_code.addresses as addresses
import numpy as np
from code.old_code.addresses import PlantPlantAddresses, MemoryAddressLocator
from util import (
    get_locator_value,
    get_locator_address,
    get_locator_byte,
    get_locator_float,
)
import const
from numpy.typing import NDArray
import math


def get_sun(process: Pymem):
    return get_locator_value(process, addresses.SUN_VALUE)


def get_plant_recharges(process: Pymem):
    recharge_times_array = np.zeros(10)
    seed_slot = get_locator_address(process, addresses.SEED_SLOT)
    for i in range(10):
        current_recharge = get_locator_value(
            process,
            MemoryAddressLocator(
                seed_slot, i * addresses.NEXT_SEED + addresses.SEED_CURRENT_RECHARGE
            ),
            False,
        )
        recharge_time = get_locator_value(
            process,
            MemoryAddressLocator(
                seed_slot, i * addresses.NEXT_SEED + addresses.SEED_RECHARGE_TIME
            ),
            False,
        )
        if current_recharge == 0:
            recharge_times_array[i] = 0
        else:
            recharge_times_array[i] = recharge_time - current_recharge
    return recharge_times_array


def get_plant_prices(process: Pymem):
    base = process.base_address
    sun_costs_array = np.zeros(10)
    seed_slot = get_locator_address(process, addresses.SEED_SLOT)
    for i in range(10):
        seed_type = get_locator_value(
            process,
            MemoryAddressLocator(
                seed_slot, i * addresses.NEXT_SEED + addresses.SEED_TYPE
            ),
            0,
        )
        offset_value = 4 * (seed_type + seed_type * 8)
        sun_cost = addresses.SUN_COSTS + offset_value
        sun_costs_array[i] = process.read_int(base + sun_cost)
    return sun_costs_array


def read_environment(process: Pymem, seeds: NDArray):
    try:
        return const.InputValues(
            get_sun(process),
            get_plant_recharges(process),
            get_plant_prices(process),
            add_zombies_to_grid(process),
            add_plants_to_grid(process, seeds),
        )
    except:
        return


def add_zombies_to_grid(process: Pymem):
    grid = np.zeros((5, 10, len(const.POSSIBLE_ZOMBIES)))
    zombie_number = get_locator_value(process, addresses.ZOMBIE_NUMBER)
    offset = 0
    zombie_address = get_locator_address(process, addresses.ZOMBIE)
    for i in range(zombie_number):
        current_zombie_offset = (i + offset) * addresses.NEXT_ZOMBIE
        while get_locator_byte(
            process,
            MemoryAddressLocator(
                zombie_address,
                current_zombie_offset + addresses.ZOMBIE_DEAD,
            ),
            False
        ):
            offset += 1
            current_zombie_offset = (i + offset) * addresses.NEXT_ZOMBIE
        x_pos = get_locator_float(
            process,
            MemoryAddressLocator(
                zombie_address, current_zombie_offset + addresses.ZOMBIE_X_POS
            ),
            False
        )
        col = math.floor((x_pos + 30) // 80)
        if col <= 9:
            y_pos = get_locator_float(
                process,
                MemoryAddressLocator(
                    zombie_address, current_zombie_offset + addresses.ZOMBIE_Y_POS
                ),
                False
            )
            row = int((y_pos) // 100)
            zombie_type = get_locator_value(
                process,
                MemoryAddressLocator(
                    zombie_address, current_zombie_offset + addresses.ZOMBIE_TYPE
                ),
                False
            )
            hot_one_encoded_zombie_type = const.POSSIBLE_ZOMBIES.index(
                const.ZOMBIES[zombie_type]
            )
            grid[row][col][hot_one_encoded_zombie_type] = 1
    return grid


def add_plants_to_grid(process: Pymem, seeds: NDArray):
    grid = np.zeros((5, 10, 10))
    plant_number = get_locator_value(process, addresses.PLANT_NUMBER)
    offset = 0
    plants_address = get_locator_address(process, addresses.PLANT)
    for i in range(plant_number):
        current_plant_offset = (i + offset) * addresses.NEXT_PLANT
        while get_locator_byte(
            process,
            MemoryAddressLocator(
                plants_address, current_plant_offset + addresses.PLANT_DEAD
            ),
            False,
        ):
            offset += 1
            current_plant_offset = (i + offset) * addresses.NEXT_PLANT

        col = get_locator_value(
            process,
            MemoryAddressLocator(
                plants_address, current_plant_offset + addresses.PLANT_COL
            ),
            False,
        )
        row = get_locator_value(
            process,
            MemoryAddressLocator(
                plants_address, current_plant_offset + addresses.PLANT_ROW
            ),
            False,
        )
        plant_type = get_locator_value(
            process,
            MemoryAddressLocator(
                plants_address, current_plant_offset + addresses.PLANT_TYPE
            ),
            False,
        )
        seed_slot_position = np.where(seeds == plant_type)
        grid[row][col][seed_slot_position] = 1
    return grid


def plant_plant(
    process: Pymem,
    x_coord: hex,
    y_coord: hex,
    seed_slot: hex,
    plantPlantAddresses: PlantPlantAddresses,
) -> None:
    x_coord = x_coord * 80 + 40
    y_coord = y_coord * 130 + 50
    process.write_int(plantPlantAddresses.x_coord, x_coord)
    process.write_int(plantPlantAddresses.y_coord, y_coord)
    process.write_int(plantPlantAddresses.seed_slot, seed_slot)


def game_running(process: Pymem):
    scene_type = get_locator_value(process, addresses.SCENE_TYPE)
    if scene_type == 3:
        return 1
    else:
        return 0


def get_clock_time(process):
    try:
        return get_locator_value(process, addresses.GAME_CLOCK)
    except MemoryReadError:
        return -1
