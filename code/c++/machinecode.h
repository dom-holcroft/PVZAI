#ifndef MACHINECODE_H
#define MACHINECODE_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <optional>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include "constants.h"

void overwriteBytes(HANDLE process, std::vector<uint8_t> byteVector, DWORD startAddress);
DWORD createVariable(HANDLE process);
DWORD createNewFunction(HANDLE process, std::vector<uint8_t> byteVector);
DWORD allocateMemory(HANDLE process, int memoryBlockSize);
void writeDouble(HANDLE process, double value, DWORD startAddress);
void writeInt(HANDLE process, int value, DWORD startAddress);
void writeByte(HANDLE process, uint8_t value, DWORD startAddress);

int readInt(HANDLE hProcess, DWORD address);

void addNOPToBytes(std::vector<uint8_t> &vec, int count);
std::vector<uint8_t> intToByteArray(DWORD value);
std::optional<size_t> replaceConsecutiveBytes(std::vector<uint8_t> &byteArray, uint8_t repeatedByte, const std::vector<uint8_t> &replacementBytes);
std::vector<uint8_t> getMoveCameraToLawnBytes();
std::vector<uint8_t> getChangeCameraStartPositionBytes();
std::vector<uint8_t> getRemoveMoveCameraAtFlagEnd();
std::vector<uint8_t> getLawnmowerStartSameTimeBytes();
std::vector<uint8_t> getInstantLawnmowerBytes();
std::vector<uint8_t> getStartGameEarlierBytes();
std::vector<uint8_t> getTimerHookBytes(DWORD timerFunctionAddress);
std::vector<uint8_t> getTimerFunctionBytes(DWORD timerAddress, DWORD limitAddress, DWORD startRoundAddress);
std::vector<uint8_t> getZombieRewardFunctionBytes(DWORD zombieRewardFunctionAddress, DWORD rewardAddress, std::unordered_map<std::string, int> zombieRewards);
std::vector<uint8_t> getNotEnoughSunHookBytes(DWORD rewardAddress, int unsuccessfulPlantReward);
std::vector<uint8_t> getPlantKilledFunctionBytes(DWORD rewardAddress, int plantKilledReward);
std::vector<uint8_t> getPlantKilledHookBytes(DWORD plantKilledFunctionAddress);
std::vector<uint8_t> getSpaceOccupiedHookBytes(DWORD rewardAddress, int unsuccessfulPlantReward);
std::vector<uint8_t> getZombieRewardHookBytes(DWORD rewardFunctionAddress);
std::vector<uint8_t> getStartRoundFunction(std::vector<int> seeds, DWORD choosePlantFunctionAddress, DWORD letsRockFunctionAddress, DWORD addPlantFunctionAddress);
std::vector<uint8_t> getLetsRockFunctionBytes();
std::vector<uint8_t> getAddPlantFunctionBytes();
std::vector<uint8_t> getChoosePlantFunctionBytes();
std::vector<uint8_t> getLawnmowerHookBytes(DWORD restartLevelAddress);
std::vector<uint8_t> getRestartLevelFunction(DWORD restartFlagAddress);
std::vector<uint8_t> getPlacePlantHookBytes(DWORD placePlantAddress);
std::vector<uint8_t> getPlacePlantFunctionBytes(DWORD restartFlagAddress, DWORD seedSlotAddress, DWORD xCoordAddress, DWORD yCoordAddres);

#endif