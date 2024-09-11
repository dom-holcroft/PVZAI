#ifndef MEMORYCONFIG_H
#define MEMORYCONFIG_H

#include "memorylocator.h"
#include <cstddef>
#include <windows.h>

namespace MemoryConfig {
    struct MemoryAddresses{
        MemoryLocator gameSpeed;
        MemoryLocator sunValue;
        MemoryLocator zombie;
        MemoryLocator plant;
        MemoryLocator seedSlot;
        MemoryLocator plantNumber;
        MemoryLocator zombieNumber;
        MemoryLocator zombieMaxNumber;
        MemoryLocator sceneType;
        MemoryLocator sunCosts;
        MemoryLocator gameClock;
        MemoryLocator gamePaused;
    };

    struct Offsets {
        ptrdiff_t plantDead;
        ptrdiff_t plantCol;
        ptrdiff_t plantRow;
        ptrdiff_t plantType;
        ptrdiff_t plantNext;

        ptrdiff_t zombieDead;
        ptrdiff_t zombieXPos;
        ptrdiff_t zombieYPos;
        ptrdiff_t zombieType;
        ptrdiff_t zombieNext;

        ptrdiff_t seedCurrentRecharge;
        ptrdiff_t seedRechargeTime;
        ptrdiff_t seedType;
        ptrdiff_t seedNext;
    };

    struct PlacePlantAddresses {
        DWORD placePlantFunctionAddress;
        DWORD seedSlotAddress;
        DWORD plantColAddress;
        DWORD plantRowAddress;
    };

    struct GameRewardAddresses {
        DWORD zombieDeathRewardAddress;
        DWORD plantDeathAddress;
        DWORD unsuccesfulPlantAddress;
    };

    extern MemoryAddresses memoryAddresses;
    extern Offsets offsets;
}

#endif