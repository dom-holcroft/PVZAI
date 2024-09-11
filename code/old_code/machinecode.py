from util import create_bytes_from_hex
from pymem import Pymem
import tomllib
from util import get_locator_address, load_plants_from_config
from code.old_code.addresses import MemoryAddress, PlantPlantAddresses
import code.old_code.addresses as addresses
from const import SetupVariables
import const as const


def setup_code_injection(process: Pymem):
    unsuccessful_plant_count = process.allocate(4)
    restart_flag_address = lawnmower_restarts_game(process).address
    seeds = skip_opening_cutscene(process, restart_flag_address)
    plantPlantAddresses = attacth_plant_plant_function(process, restart_flag_address)
    reward_address = attatch_reward_function(process)
    plant_killed_address = plant_killed_hook(process)
    space_occupied_hook(process, unsuccessful_plant_count)
    not_enough_sun_hook(process, unsuccessful_plant_count)
    change_game_speed(process)
    return SetupVariables(
        plantPlantAddresses,
        restart_flag_address,
        seeds,
        reward_address,
        unsuccessful_plant_count,
        plant_killed_address,
    )


def change_game_speed(process: Pymem):
    with open("config.toml", mode="rb") as f:
        speed_up = tomllib.load(f)["timings"]["speed_up"]
    speed_up_address = get_locator_address(process, addresses.GAME_SPEED)
    process.write_double(speed_up_address, float(speed_up))


def speed_up_start_animation(process: Pymem):
    base = process.base_address

    # next_wave_timer = 0xFDCE #Change to 1

    start_position = 0x3FE56  # change to 84 FE FF FF (-380)
    end_position = 0x3320BC  # 10

    move_back_start = 0x3320C0

    wait_at_zombies = 0x3320A8  # 10
    plant_selector_show = 0x3320A4  # 00

    show_hotbar_start = 0x3320B4  # 00
    show_hotbar_end = 0x3320B8  # 10

    hide_plant_selector_start = 0x3320AC  # 20
    hide_plant_selector_end = 0x3320B0  # 30

    hide_hotbar_start = 0x3320C4  # 20
    hide_hotbar_end = 0x3320C8  # 30

    process.write_bytes(base + start_position, b"\x84\xfe\xff\xff", 4)

    process.write_bytes(base + 0x3FFD2, (b"\xbb\x0a\x00\x00\x00" + b"\x90" * 6), 11)

    process.write_bytes(base + 0x3FE63, (b"\x90" * 2), 2)

    # Controls the lawn mowers
    process.write_bytes(base + 0x403F0, (b"\xbf\x00\x00\x00\x00" + b"\x90" * 11), 16)
    process.write_bytes(base + 0x40446, (b"\x0f" + b"\x90" * 4), 5)

    # Controls starting the game (28 is 40 ticks)
    process.write_bytes(base + 0x408E0, b"\xb8\x28\x00\x00\x00" + b"\x90" * 22, 27)

    process.write_int(base + move_back_start, 10)
    process.write_int(base + end_position, 10)
    process.write_int(base + plant_selector_show, 10)
    process.write_int(base + wait_at_zombies, 10)
    process.write_int(base + plant_selector_show, 0)
    process.write_int(base + show_hotbar_start, 00)
    process.write_int(base + show_hotbar_end, 10)
    process.write_int(base + hide_plant_selector_start, 20)
    process.write_int(base + hide_plant_selector_end, 30)
    process.write_int(base + hide_hotbar_start, 20)
    process.write_int(base + hide_hotbar_end, 30)
    # process.write_bytes(base + next_wave_timer, 10)


def skip_opening_cutscene(process: Pymem, restart_flag_address):
    base = process.base_address
    speed_up_start_animation(process)
    add_plant_address = generate_add_plant_function(process)
    choose_plant_address = generate_choose_plant_function(process)
    lets_rock_address = generate_lets_rock_function(process, restart_flag_address)
    start_round_address, seeds = generate_start_round_function(
        process, choose_plant_address, lets_rock_address, add_plant_address
    )
    timer_address = generate_timer_function(process, start_round_address)
    INJECTION_CODE = (
        b"\xbb" + create_bytes_from_hex(timer_address.address) + b"\xff\xd3"
    )
    process.write_bytes(base + 0x408E5, bytes(INJECTION_CODE), len(INJECTION_CODE))
    print("timer address hook", hex(base + 0x408E5))
    return seeds


def generate_timer_function(
    process: Pymem, start_round_address: MemoryAddress
) -> MemoryAddress:
    timer = process.allocate(4)
    limit = process.allocate(4)
    process.write_int(limit, 2)

    TIMER_FUNCTION = (
        b"\x50"
        + b"\x51"
        + b"\x52"
        + b"\x53"
        + b"\x55"
        + b"\x56"
        + b"\x57"
        + b"\x83\x7e\x08\x0a"
        + b"\x75\x23"
        + b"\x8b\x05"
        + create_bytes_from_hex(timer)
        + b"\x8b\x1d"
        + create_bytes_from_hex(limit)
        + b"\x39\xd8"
        + b"\x7e\x13"
        + b"\xb8"
        + create_bytes_from_hex(start_round_address.address)
        + b"\xff\xd0"
        + b"\xc7\x05"
        + create_bytes_from_hex(timer)
        + b"\x00\x00\x00\x00"
        + b"\xeb\x06"
        + b"\xff\x05"
        + create_bytes_from_hex(timer)
        + b"\x5f"
        + b"\x5e"
        + b"\x5d"
        + b"\x5b"
        + b"\x5a"
        + b"\x59"
        + b"\x58"
        + b"\xc3"
    )
    timer_address = process.allocate(len(TIMER_FUNCTION))
    process.write_bytes(timer_address, bytes(TIMER_FUNCTION), len(TIMER_FUNCTION))
    print("timer address function", hex(timer_address))
    return MemoryAddress(timer_address)


def not_enough_sun_hook(process: Pymem, unsuccessful_plant_count: MemoryAddress):
    base = process.base_address
    NOT_ENOUGH_SUN = (
        b"\xff\x05"
        + create_bytes_from_hex(
            unsuccessful_plant_count
        )  # inc [unsuccessful_plant_count]
        + b"\xc3"
    )
    print("not enough sun hook", hex(base + 0x1F664))
    process.write_bytes(base + 0x1F664, NOT_ENOUGH_SUN, len(NOT_ENOUGH_SUN))


def plant_killed_hook(process: Pymem):
    base = process.base_address
    plants_killed_count = process.allocate(4)
    GENERATE_PLANT_KILLED_INCREMENT = (
        b'\xff\x05' + create_bytes_from_hex(plants_killed_count)
        + b'\xC3'
    )
    plant_killed_increment_address = process.allocate(len(GENERATE_PLANT_KILLED_INCREMENT))
    
    process.write_bytes(plant_killed_increment_address, GENERATE_PLANT_KILLED_INCREMENT, len(GENERATE_PLANT_KILLED_INCREMENT))
    CALL_PLANT_KILLED_INCREMENT = (
        b'\xE8' + create_bytes_from_hex(plant_killed_increment_address - (base + 0x68F5A) - 5)
        + b'\xc3'
    )
    process.write_bytes(base + 0x68F5A, CALL_PLANT_KILLED_INCREMENT, len(CALL_PLANT_KILLED_INCREMENT))
    print("plant killed hook", hex(base + 0x68F5A))
    return plants_killed_count


def space_occupied_hook(process: Pymem, unsuccessful_plant_count: MemoryAddress):
    base = process.base_address
    UNSUCCESSFUL_PLANT = (
        b"\xff\x05" + create_bytes_from_hex(unsuccessful_plant_count) + b"\x90" * 7
    )
    print("space occupied hook", hex(base + 0x13C4C))
    process.write_bytes(base + 0x13C4C, UNSUCCESSFUL_PLANT, len(UNSUCCESSFUL_PLANT))


def attatch_reward_function(process: Pymem):
    base = process.base_address
    reward_function_address, reward_address = generate_reward_function(process)
    HOOK_REWARD_FUNCTION = (
        b"\xb8"
        + create_bytes_from_hex(reward_function_address)
        + b"\xff\xd0"
        + b"\x5f"
        + b"\x8b\xe5"
        + b"\x5d"
        + b"\xc3"
    )
    process.write_bytes(
        base + 0x145113, HOOK_REWARD_FUNCTION, len(HOOK_REWARD_FUNCTION)
    )
    print("reward function hook", hex(base+0x145113))
    return reward_address


def generate_reward_function(process: Pymem):
    reward_function_address = process.allocate(256)

    reward_address = process.allocate(4)
    REWARD_FUNCTION = (
        b"\x8B\x87\xc4\x00\x00\x00" 
        + b"\x6B\xC0\x0B"
        + b"\x05" + create_bytes_from_hex(reward_function_address + 16)
        +b"\xFF\xE0"
    )
    for zombie in const.POSSIBLE_ZOMBIES:
        new_reward = const.ZOMBIE_REWARDS[zombie]
        REWARD_FUNCTION += (
            b"\x81\x05"
            + create_bytes_from_hex(reward_address)
            + create_bytes_from_hex(new_reward, 4)
            + b"\xc3"
        )
        process.write_bytes(
        reward_function_address, bytes(REWARD_FUNCTION), len(REWARD_FUNCTION)
    )
    print("reward function address", hex(reward_function_address))
    return reward_function_address, reward_address


def generate_start_round_function(
    process: Pymem,
    choose_function_address: MemoryAddress,
    lets_rock_address: MemoryAddress,
    add_plant_address: MemoryAddress,
) -> MemoryAddress:
    seeds = load_plants_from_config()
    print(seeds[0])
    START_ROUND = b""
    for i in range(10):
        START_ROUND += (
            b"\xbf"
            + create_bytes_from_hex(seeds[i])
            + b"\xb8"
            + create_bytes_from_hex(choose_function_address.address)
            + b"\xff\xd0"
        )
    START_ROUND += (
        b"\xb8"
        + create_bytes_from_hex(add_plant_address.address)
        + b"\xff\xd0"
        + b"\xb8"
        + create_bytes_from_hex(lets_rock_address.address)
        + b"\xff\xd0"
        + b"\xc3"
    )
    start_round_address = process.allocate(len(START_ROUND))

    process.write_bytes(start_round_address, bytes(START_ROUND), len(START_ROUND))
    print("start round function", hex(start_round_address))
    return MemoryAddress(start_round_address), seeds


def generate_lets_rock_function(process: Pymem, restart_flag_address):
    base_address = process.base_address

    LETS_ROCK = (
        b"\x83\x3d"
        + create_bytes_from_hex(restart_flag_address)
        + b"\x01"
        + b"\x74\xf7"
        + b"\x8b\x35"
        + create_bytes_from_hex(base_address + 0x331C50)
        + b"\x8b\xb6\x20\x03\x00\x00"
        + b"\x8b\xb6\xa0\x00\x00\x00"
        + b"\x8b\x8e\x2c\x0d\x00\x00"
        + b"\x8b\x96\x5c\x01\x00\x00"
        + b"\xbf\x64\x00\x00\x00"
        + b"\xb8\x31\x00\x00\x00"
        + b"\xbb\x00\x00\x00\x00"
        + b"\xbf"
        + create_bytes_from_hex(base_address + 0x93C70)
        + b"\xff\xd7"
        + b"\xc3"
    )
    lets_rock_address = process.allocate(len(LETS_ROCK))
    process.write_bytes(lets_rock_address, bytes(LETS_ROCK), len(LETS_ROCK))
    print("lets rock function", hex(lets_rock_address))
    return MemoryAddress(lets_rock_address)


def generate_add_plant_function(process: Pymem):
    base_address = process.base_address
    ADD_PLANT = (
        b"\x6a\xff"
        + b"\x6a\x00"
        + b"\x6a\x00"
        + b"\xb8\x00\x00\x00\x00"
        + b"\xba\x00\x00\x00\x00"
        + b"\x8b\x35"
        + create_bytes_from_hex(base_address + 0x331C50)
        + b"\x8b\xb6\x20\x03\x00\x00"
        + b"\x8b\xb6\xa0\x00\x00\x00"
        + b"\x8b\x96\x28\x0d\x00\x00"
        + b"\x8b\x8a\x20\x03\x00\x00"
        + b"\xbb"
        + create_bytes_from_hex(base_address + 0x14F9D0)
        + b"\xff\xd3"
        + b"\xc3"
    )
    add_plant_address = process.allocate(len(ADD_PLANT))

    process.write_bytes(add_plant_address, bytes(ADD_PLANT), len(ADD_PLANT))
    print("add plant address", hex(add_plant_address))
    return MemoryAddress(add_plant_address)


def generate_choose_plant_function(process: Pymem):
    base_address = process.base_address

    CHOOSE_PLANT = (
        b"\x8b\x35"
        + create_bytes_from_hex(base_address + 0x331C50)
        + b"\x8b\xb6\x20\x03\x00\x00"
        + b"\x8b\xb6\xa0\x00\x00\x00"
        + b"\x8b\xde"
        + b"\x8b\xd7"
        + b"\xc1\xe2\x04"
        + b"\x29\xfa"
        + b"\x8b\x8c\x96\xe0\x00\x00\x00"
        + b"\x8d\xac\x96\xbc\x00\x00\x00"
        + b"\x8b\x86\x3c\x0d\x00\x00"
        + b"\x89\x45\x28"
        + b"\xc7\x45\x24\x00\x00\x00\x00"
        + b"\xff\x86\x3c\x0d\x00\x00"
        + b"\xff\x86\x38\x0d\x00\x00"
        + b"\xc3"
    )
    choose_plant_address = process.allocate(len(CHOOSE_PLANT))
    print("choose plant function", hex(choose_plant_address))
    process.write_bytes(choose_plant_address, bytes(CHOOSE_PLANT), len(CHOOSE_PLANT))
    return MemoryAddress(choose_plant_address)


def lawnmower_restarts_game(process: Pymem) -> MemoryAddress:
    base = process.base_address
    restart_level_address, restart_flag = generate_restart_level_function(process)
    LAWNMOWER_ACTIVATE = (
        b"\x53"
        + b"\xbb"
        + create_bytes_from_hex(restart_level_address.address)
        + b"\xff\xd3"
        + b"\x5b"
        + b"\x90" * 3
    )
    process.write_bytes(
        base + 0x5E765, bytes(LAWNMOWER_ACTIVATE), len(LAWNMOWER_ACTIVATE)
    )
    print("lawnmower restarts hook", hex(0x5e765+base))
    return restart_flag


def generate_restart_level_function(process: Pymem) -> MemoryAddress:
    base = process.base_address
    restart_flag = process.allocate(4)
    RESTART_LEVEL = (
        b"\x60"
        + b"\x9c"
        + b"\xc7\x05"
        + create_bytes_from_hex(restart_flag)
        + b"\x01\x00\x00\x00"
        + b"\xb8\x06\x00\x00\x00"
        + b"\xbb\x01\x00\x00\x00"
        + b"\x8b\x15"
        + create_bytes_from_hex(base + 0x331C50)
        + b"\xc7\x82\xa8\x09\x00\x00\03\x00\x00\x00"
        + b"\x8b\x05"
        + create_bytes_from_hex(base + 0x331C50)
        + b"\x8b\x88\x68\x08\x00\x00"
        + b"\x8a\x91\x78\x57\x00\x00"
        + b"\x88\x90\xac\x09\x00\x00"
        + b"\x8b\x35"
        + create_bytes_from_hex(base + 0x331C50)
        + b"\x8b\x86\x18\x09\x00\x00"
        + b"\x6a\x00"
        + b"\x50"
        + b"\xbf"
        + create_bytes_from_hex(base + 0x541D0)
        + b"\xff\xd7"
        + b"\x9d"
        + b"\x61"
        + b"\xc3"
    )
    restart_level_address = process.allocate(len(RESTART_LEVEL))
    process.write_bytes(restart_level_address, bytes(RESTART_LEVEL), len(RESTART_LEVEL))
    print("restart_level function address", hex(restart_level_address))
    return MemoryAddress(restart_level_address), MemoryAddress(restart_flag)


def attacth_plant_plant_function(process: Pymem, restart_flag_address: hex):
    
    base = process.base_address
    plantPlantAddress = generate_plant_plant_function(process, restart_flag_address)
    CALL_PLANT_PLANT = (
        b"\xb8"
        + create_bytes_from_hex(plantPlantAddress.start_address)
        + b"\xff\xd0"
        + b"\xc2\x04\x00"
    )

    process.write_bytes(base + 0x1DB41, bytes(CALL_PLANT_PLANT), len(CALL_PLANT_PLANT))
    process.write_int(plantPlantAddress.seed_slot, -1)
    print("plant plant hook address", hex(base + 0x1db41))
    return plantPlantAddress


def generate_plant_plant_function(process: Pymem, restart_flag_address):
    base_address = process.base_address
    seed_slot = process.allocate(4)
    x_coord = process.allocate(4)
    y_coord = process.allocate(4)
    plant_function_area = process.allocate(2048)
    print("plant plant function address", hex(plant_function_area))
    PLANT_PLANT = (
        b"\x60" + b"\x9c" + b"\x83\x3d" + create_bytes_from_hex(seed_slot) + b"\xff"
        b"\x0f\x84\xb2\x00\x00\x00"
        + b"\x8b\x1d"
        + create_bytes_from_hex(restart_flag_address)
        + b"\x83\xfb\x01"
        + b"\x0f\x84\xa3\x00\x00\x00"
        + b"\x8b\x1d"
        + create_bytes_from_hex(base_address + 0x331C50)
        + b"\x8b\x9b\x20\x03\x00\x00"
        + b"\x8b\x5b\x18"
        + b"\x8b\x1b"
        + b"\x8b\x5b\x08"
        + b"\x8b\x5b\x2c"
        + b"\x8b\x1b"
        + b"\x8b\x9b\x58\x01\x00\x00"
        + b"\x8b\x0d"
        + create_bytes_from_hex(seed_slot)
        + b"\x6b\xc9\x50"
        + b"\x83\xc3\x5c"
        + b"\x01\xcb"
        + b"\x80\x7b\x14\x00"
        + b"\x74\x70"
        + b"\xc6\x43\x14\x00"
        + b"\x53"
        + b"\x8b\x1b"
        + b"\x8b\x3d"
        + create_bytes_from_hex(base_address + 0x331C50)
        + b"\x8b\xbf\x20\x03\x00\x00"
        + b"\x8b\x7f\x18"
        + b"\x8b\x3f"
        + b"\x8b\x7f\x08"
        + b"\x8b\x87\x50\x01\x00\x00"
        + b"\x8b\x0d"
        + create_bytes_from_hex(seed_slot)
        + b"\x89\x48\x24"
        + b"\x89\x58\x28"
        + b"\xc7\x40\x30\x01\x00\x00\x00"
        + b"\xff\x35"
        + create_bytes_from_hex(y_coord)
        + b"\xff\x35"
        + create_bytes_from_hex(x_coord)
        + b"\x57"
        + b"\xba"
        + create_bytes_from_hex(base_address + 0x13250)
        + b"\xff\xd2"
        + b"\x8b\x87\x50\x01\x00\x00"
        + b"\xc7\x40\x30\x00\x00\x00\x00"
        + b"\xc7\x40\x24\xff\xff\xff\xff"
        + b"\xc7\x40\x28\xff\xff\xff\xff"
        + b"\x5b"
        + b"\x39\x97\x78\x01\x00\x00"
        + b"\x74\x04"
        + b"\xc6\x43\x14\x01"
        + b"\xc7\x05"
        + create_bytes_from_hex(seed_slot)
        + b"\xff\xff\xff\xff"
        + b"\x9d"
        + b"\x61"
        + b"\xc3"
    )

    process.write_bytes(plant_function_area, bytes(PLANT_PLANT), len(PLANT_PLANT))
    return PlantPlantAddresses(seed_slot, x_coord, y_coord, plant_function_area)
