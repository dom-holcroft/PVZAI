from dataclasses import dataclass
from code.old_code.addresses import PlantPlantAddresses, MemoryAddress
import numpy as np
from numpy.typing import NDArray


@dataclass(frozen=True)
class SetupVariables:
    plantPlantAddresses: PlantPlantAddresses
    restart_flag_address: MemoryAddress
    seeds: NDArray
    reward_address: MemoryAddress
    unsuccessful_plant_count: MemoryAddress
    plant_killed_address: MemoryAddress


@dataclass
class InputValues:
    sun_value: int
    seed_recharge: list
    plant_cost: list
    zombie_grid: list
    plant_grid: list


PLANTS = {
    "peashooter": 0,
    "sunflower": 1,
    "cherry bomb": 2,
    "wall-nut": 3,
    "potato mine": 4,
    "snow pea": 5,
    "chomper": 6,
    "repeater": 7,
    "puff-shroom": 8,
    "sun-shroom": 9,
    "fume-shroom": 10,
    "grave buster": 11,
    "hypno-shroom": 12,
    "scaredy-shroom": 13,
    "ice-shroom": 14,
    "doom-shroom": 15,
    "lily pad": 16,
    "squash": 17,
    "threepeater": 18,
    "tangle kelp": 19,
    "jalapeno": 20,
    "spikeweed": 21,
    "torchwood": 22,
    "tall-nut": 23,
    "sea-shroom": 24,
    "plantern": 25,
    "cactus": 26,
    "blover": 27,
    "split pea": 28,
    "starfruit": 29,
    "pumpkin": 30,
    "magnet-shroom": 31,
    "cabbage-pult": 32,
    "flower pot": 33,
    "kernel-pult": 34,
    "coffee bean": 35,
    "garlic": 36,
    "umbrella leaf": 37,
    "marigold": 38,
    "melon-pult": 39,
    "gatling pea": 40,
    "twin sunflower": 41,
    "gloom-shroom": 42,
    "cattail": 43,
    "winter melon": 44,
    "gold magnet": 45,
    "spikerock": 46,
}


ZOMBIES = [
    "Normal Zombie",
    "Flag Zombie",
    "Conehead Zombie",
    "Pole Vaulting Zombie",
    "Buckethead Zombie",
    "Newspaper Zombie",
    "Screen Door Zombie",
    "Football Zombie",
    "Dancing Zombie",
    "Backup Dancer",
    "Ducky Tube Zombie",
    "Snorkel Zomboe",
    "Zomboni",
    "Zombie Bobsled Team",
    "Dolphin Rider Zombie",
    "Jack-in-the-Box Zombie",
    "Balloon Zombie",
    "Digger Zombie",
    "Pogo Zombie",
    "Zombie Yeti",
    "Bungee Zombie",
    "Ladder Zombie",
    "Catapult Zombie",
    "Gargantuar",
    "Imp",
]

POSSIBLE_ZOMBIES = [
    "Normal Zombie",
    "Flag Zombie",
    "Conehead Zombie",
    "Buckethead Zombie",
    "Newspaper Zombie",
    "Screen Door Zombie",
    "Football Zombie",
    "Dancing Zombie",
    "Backup Dancer",
    "Zomboni",
    "Jack-in-the-Box Zombie",
    "Balloon Zombie",
    "Digger Zombie",
    "Pogo Zombie",
    "Ladder Zombie",
    "Catapult Zombie",
    "Gargantuar",
    "Imp",
]


ZOMBIE_REWARDS = {
    "Normal Zombie": 1,
    "Flag Zombie": 1,
    "Conehead Zombie": 2,
    "Buckethead Zombie": 4,
    "Newspaper Zombie": 2,
    "Screen Door Zombie": 4,
    "Football Zombie": 8,
    "Dancing Zombie": 3,
    "Backup Dancer": 1,
    "Zomboni": 4,
    "Jack-in-the-Box Zombie": 2,
    "Balloon Zombie": 6,
    "Digger Zombie": 6,
    "Pogo Zombie": 3,
    "Ladder Zombie": 3,
    "Catapult Zombie": 4,
    "Gargantuar": 12,
    "Imp": 2,
}
