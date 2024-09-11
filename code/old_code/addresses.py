from dataclasses import dataclass, field
from typing import List

@dataclass
class MemoryAddressLocator:
    base_address: int
    offsets: List[int] = field(default_factory=list)

    def __post_init__(self):
        if isinstance(self.offsets, int):
            self.offsets = [self.offsets]
    

    def __iter__(self):
        yield self.base_address
        yield self.offsets


@dataclass(frozen=True)
class MemoryAddress:
    address: int = 0


@dataclass(frozen=True)
class PlantPlantAddresses:
    seed_slot: MemoryAddress
    x_coord: MemoryAddress
    y_coord: MemoryAddress
    start_address: MemoryAddress


SUN_VALUE = MemoryAddressLocator(0x00331C50, [0x5578])
ZOMBIE_HEALTH = MemoryAddressLocator(
    0x0005FAE0, [0x1A0, 0x14, 0x18, 0x10, 0xA4, 0xC08]
)  # 360 between each ZOMBIE
ZOMBIE_TYPE = MemoryAddressLocator(0x0005FAE0, [0x1A0, 0x14, 0x18, 0x10, 0xA4, 0xC04])
ZOMBIE = MemoryAddressLocator(
    0x331C50, [0x320, 0x18, 0x0, 0x8, 0xA8]
)  # 0xC4 is the type
PLANT = MemoryAddressLocator(0x00331C50, [0x320, 0x18, 0x0, 0x8, 0xC4])  # Add 14C
SEED_SLOT = MemoryAddressLocator(
    0x00331C50, [0x320, 0x18, 0x0, 0x8, 0x2C, 0x0, 0x158]
)  # 50 away is the next one
TEST_VALUE = MemoryAddressLocator(0x00331C50, [0x320, 0x18, 0x0, 0x8, 0x150, 0x28])
CHOOSE_PLANTS = MemoryAddressLocator(0x00331C50, [0x320, 0xA0, 0xD28])
START_TIMER = MemoryAddressLocator(
    0x00331C50, [0x320, 0x18, 0x0, 0x8, 0x2C, 0x0, 0x170, 0x8]
)
START_ROUND_CLOCK = MemoryAddressLocator(
    0x331D08, [0x320, 0x20, 0x18, 0x0, 0x8, 0x4, 0x0, 0x170, 0x8]
)
GAME_SPEED = MemoryAddressLocator(0x00331C50, [0x4F0])
PLANT_NUMBER = MemoryAddressLocator(0x331C50, [0x320, 0x18, 0x0, 0x8, 0xD4])
ZOMBIE_NUMBER = MemoryAddressLocator(0x331C50, [0x320, 0x18, 0x0, 0x8, 0xB8])
SCENE_TYPE = MemoryAddressLocator(0x331C50, [0x91C])
GAME_CLOCK = MemoryAddressLocator(0x331C50, [0x868, 0x5580])


PLANT_DEAD = 0x141
PLANT_ROW = 0x1C
PLANT_COL = 0x28
PLANT_TYPE = 0x24
NEXT_PLANT = 0x14C


NEXT_ZOMBIE = 0x168
ZOMBIE_TYPE = 0xC4
ZOMBIE_DEAD = 0xEC
ZOMBIE_X_POS = 0x2C  # This is a float
ZOMBIE_Y_POS = 0xC


SEED_CURRENT_RECHARGE = 0x4C
SEED_RECHARGE_TIME = 0x50
SEED_TYPE = 0x5C
NEXT_SEED = 0x50


SUN_COSTS = 0x326988
