#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <iostream>
#include <unordered_map>
#include <map>
#include <string>
#include <array>

const int NUMBEROFSEEDS = 10;
const int MAXPLANTIDNUMBER = 50;

const std::string CONFIG_FILE_NAME = "config.toml";

const std::vector<std::string> CHOSEN_PLANTS = {
    "peashooter",     
    "sunflower",
    "wall-nut",
    "squash",
    "spikeweed",
    "starfruit",
    "pumpkin",
    "melon-pult",
    "winter melon",
    "blover",     
};

const std::string ZOMBIES[] = {
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
};


constexpr std::array<const char*, 18> POSSIBLE_ZOMBIES = {
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
};


const std::unordered_map<std::string, int> PLANTS {
    {"peashooter", 0},
    {"sunflower", 1},
    {"cherry bomb", 2},
    {"wall-nut", 3},
    {"potato mine", 4},
    {"snow pea", 5},
    {"chomper", 6},
    {"repeater", 7},
    {"puff-shroom", 8},
    {"sun-shroom", 9},
    {"fume-shroom", 10},
    {"grave buster", 11},
    {"hypno-shroom", 12},
    {"scaredy-shroom", 13},
    {"ice-shroom", 14},
    {"doom-shroom", 15},
    {"lily pad", 16},
    {"squash", 17},
    {"threepeater", 18},
    {"tangle kelp", 19},
    {"jalapeno", 20},
    {"spikeweed", 21},
    {"torchwood", 22},
    {"tall-nut", 23},
    {"sea-shroom", 24},
    {"plantern", 25},
    {"cactus", 26},
    {"blover", 27},
    {"split pea", 28},
    {"starfruit", 29},
    {"pumpkin", 30},
    {"magnet-shroom", 31},
    {"cabbage-pult", 32},
    {"flower pot", 33},
    {"kernel-pult", 34},
    {"coffee bean", 35},
    {"garlic", 36},
    {"umbrella leaf", 37},
    {"marigold", 38},
    {"melon-pult", 39},
    {"gatling pea", 40},
    {"twin sunflower", 41},
    {"gloom-shroom", 42},
    {"cattail", 43},
    {"winter melon", 44},
    {"gold magnet", 45},
    {"spikerock", 46},
};

#endif