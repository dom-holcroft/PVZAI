#ifndef PVZ_H
#define PVZ_H

#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>
#include "util.h"
#include "constants.h"
#include <array>
#include <vector>
#include <tuple>
#include <algorithm>
#include <memoryconfig.h>

struct GameValues {
    std::vector<int> inputInformation; 
    std::array<std::array<std::array<int, 10>, 5>, POSSIBLE_ZOMBIES.size() + 10> combinedGrid;
    bool gameOver = false;
};

struct SetupCodeInjectionReturn {
    DWORD rewardAddress;
    int actionInterval;
    double survivalReward;
    DWORD restartFlagAddress;
    std::vector<int> seedIDs;
};

HANDLE getProcessByName(const char* processName);
bool isGameRunning(HANDLE hProcess);

void placePlant(HANDLE process, MemoryConfig::PlacePlantAddresses* placePlantAddresses,int col, int row, int seedSlot);
SetupCodeInjectionReturn setupCodeInjection(HANDLE process, MemoryConfig::PlacePlantAddresses* placePlantAddresses);
GameValues getGameValues(HANDLE hProcess, DWORD restartFlagAddress, std::vector<int> seedIDs);

int playStep(HANDLE hProcess, int actionInterval, int previousTime);
void restartGame(HANDLE hProcess,  DWORD restartAddress);
bool isGameRunning(HANDLE hProcess);
int isGameOver(HANDLE hProcess, DWORD restartAddress);
double getReward(HANDLE hProcess, DWORD rewardAddress, double survivalReward);


#endif 