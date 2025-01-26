#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <Windows.h>
#include <TlHelp32.h>
#include <optional>
#include <vector>
#include "constants.h"

void overwriteBytes(HANDLE process, std::vector<uint8_t> byteVector, DWORD startAddress)
{
    SIZE_T bytesWritten;
    if (!WriteProcessMemory(process, reinterpret_cast<void *>(startAddress), byteVector.data(), byteVector.size(), &bytesWritten) || bytesWritten != byteVector.size())
    {
        std::cerr << "Failed to overwrite memory in remote process. Error: " << GetLastError() << std::endl;
        return;
    }
}

DWORD createVariable(HANDLE process)
{
    LPVOID functionAddress = VirtualAllocEx(process, NULL, 4, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    return (DWORD)functionAddress;
}

DWORD createNewFunction(HANDLE process, std::vector<uint8_t> byteVector)
{
    LPVOID functionAddress = VirtualAllocEx(process, NULL, byteVector.size(), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (functionAddress == NULL)
    {
        std::cerr << "Failed to create function region in remote process. Error: " << GetLastError() << std::endl;
        return NULL;
    }
    SIZE_T bytesWritten;
    if (!WriteProcessMemory(process, functionAddress, byteVector.data(), byteVector.size(), &bytesWritten) || bytesWritten != byteVector.size())
    {
        std::cerr << "Failed to write to function region remote process. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(process, functionAddress, 0, MEM_RELEASE); // Clean up allocated memory
        return NULL;
    }
    return (uintptr_t)functionAddress;
}

DWORD allocateMemory(HANDLE process, int memoryBlockSize)
{
    LPVOID functionAddress = VirtualAllocEx(process, NULL, memoryBlockSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (functionAddress == NULL)
    {
        std::cerr << "Failed to allocate memory in remote process. Error: " << GetLastError() << std::endl;
        return NULL;
    }
    return (DWORD)functionAddress;
}

void writeDouble(HANDLE process, double value, DWORD startAddress)
{
    SIZE_T bytesWritten;
    if (!WriteProcessMemory(process, reinterpret_cast<void *>(startAddress), &value, 8, &bytesWritten) || bytesWritten != 8)
    {
        std::cerr << "Failed to write double in remote process. Error: " << GetLastError() << std::endl;
        return;
    }
}

void writeByte(HANDLE process, uint8_t value, DWORD startAddress)
{
    SIZE_T bytesWritten;
    if (!WriteProcessMemory(process, reinterpret_cast<void *>(startAddress), &value, 1, &bytesWritten) || bytesWritten != 1)
    {
        std::cerr << "Failed to write double in remote process. Error: " << GetLastError() << std::endl;
        return;
    }
}

void writeInt(HANDLE process, int value, DWORD startAddress)
{
    SIZE_T bytesWritten;
    if (!WriteProcessMemory(process, reinterpret_cast<void *>(startAddress), &value, 4, &bytesWritten) || bytesWritten != 4)
    {
        std::cerr << "Failed to write integer in remote process. Error: " << GetLastError() << std::endl;
        return;
    }
}

int readInt(HANDLE hProcess, DWORD address) {
    int value;
    SIZE_T bytesRead;
    if (!ReadProcessMemory(hProcess, reinterpret_cast<void *>(address), &value, sizeof(value), &bytesRead) && bytesRead == sizeof(value)) {
        std::cerr << "Failed to read Memory. Error:" << GetLastError() << std::endl;
    }
    return value;
}

// I'm sure there is a better way of doing this, but this works, and it works well
void addNOPToBytes(std::vector<uint8_t> &vec, int count)
{
    for (int i = 0; i < count; ++i)
    {
        vec.push_back(0x90);
    }
}

std::vector<uint8_t> intToByteArray(DWORD value)
{
    std::vector<uint8_t> bytes(4);
    for (size_t i = 0; i < 4; ++i)
    {
        bytes[i] = (value >> (i * 8)) & 0xFF;
    }
    return bytes;
}

std::optional<size_t> replaceConsecutiveBytes(std::vector<uint8_t> &byteArray,
                                              uint8_t repeatedByte,
                                              DWORD replacementValue)
{
    const std::vector<uint8_t> replacementBytes = intToByteArray(replacementValue);
    std::vector<uint8_t> placeholder(replacementBytes.size(), repeatedByte);
    size_t totalReplacements = 0;
    auto it = byteArray.begin();

    while (true)
    {

        it = std::search(it, byteArray.end(), placeholder.begin(), placeholder.end());

        if (it == byteArray.end())
        {
            break;
        }

        if (std::distance(it, byteArray.end()) >= replacementBytes.size())
        {

            std::copy(replacementBytes.begin(), replacementBytes.end(), it);
            totalReplacements++;

            it += replacementBytes.size();
        }
        else
        {
            break;
        }
    }

    if (totalReplacements > 0)
    {
        return totalReplacements;
    }

    return std::nullopt;
}

std::vector<uint8_t> getMoveCameraToLawnBytes()
{
    std::vector<uint8_t> moveCameraToLawnBytes = {
        0xbb, 0x0a, 0x00, 0x00, 0x00};
    addNOPToBytes(moveCameraToLawnBytes, 6);
    return moveCameraToLawnBytes;
}

std::vector<uint8_t> getChangeCameraStartPositionBytes()
{
    std::vector<uint8_t> changeCameraStartPositionBytes = {
        0x84, 0xfe, 0xff, 0xff};
    return changeCameraStartPositionBytes;
}

std::vector<uint8_t> getRemoveMoveCameraAtFlagEnd()
{
    std::vector<uint8_t> removeMoveCameraAtFlagEnd = {
        0x90, 0x90};
    return removeMoveCameraAtFlagEnd;
}

std::vector<uint8_t> getLawnmowerStartSameTimeBytes()
{
    std::vector<uint8_t> lawnmowerStartSameTimeBytes = {
        0x0f};
    addNOPToBytes(lawnmowerStartSameTimeBytes, 4);
    return lawnmowerStartSameTimeBytes;
}

std::vector<uint8_t> getInstantLawnmowerBytes()
{
    std::vector<uint8_t> instantLawnmowerBytes = {
        0xbf, 0x00, 0x00, 0x00, 0x00};
    addNOPToBytes(instantLawnmowerBytes, 11);
    return instantLawnmowerBytes;
}

std::vector<uint8_t> getStartGameEarlierBytes()
{
    std::vector<uint8_t> startGameEarlyBytes = {
        0xb8, 0x28, 0x00, 0x00, 0x00};
    addNOPToBytes(startGameEarlyBytes, 22);
    return startGameEarlyBytes;
}

std::vector<uint8_t> getTimerHookBytes(DWORD timerFunctionAddress)
{

    std::vector<uint8_t> timerHookBytes = {
        0xbb,
        0xAA, 0xAA, 0xAA, 0xAA,
        0xff, 0xd3};
    replaceConsecutiveBytes(timerHookBytes, 0xAA, timerFunctionAddress);
    return timerHookBytes;
}

std::vector<uint8_t> getTimerFunctionBytes(DWORD timerAddress, DWORD limitAddress, DWORD startRoundAddress)
{

    std::vector<uint8_t> timerFunction = {
        0x60,
        0x9c,
        0x83, 0x7e, 0x08, 0x0a,
        0x75, 0x23,
        0x8b, 0x05,
        0xAA, 0xAA, 0xAA, 0xAA, // Replace with timer address
        0x8b, 0x1d,
        0xBB, 0xBB, 0xBB, 0xBB, // Replace with limit address
        0x39, 0xd8,
        0x7e, 0x13,
        0xb8,
        0xCC, 0xCC, 0xCC, 0xCC, // Replace with start round address
        0xff, 0xd0,
        0xc7, 0x05,
        0xAA, 0xAA, 0xAA, 0xAA, // Replace with timer address
        0x00, 0x00, 0x00, 0x00,
        0xeb, 0x06,
        0xff, 0x05,
        0xAA, 0xAA, 0xAA, 0xAA, // Replace with timer address
        0x9d,
        0x61,
        0xc3};


    replaceConsecutiveBytes(timerFunction, 0xAA, timerAddress);
    replaceConsecutiveBytes(timerFunction, 0xBB, limitAddress);
    replaceConsecutiveBytes(timerFunction, 0xCC, startRoundAddress);

    return timerFunction;
}

std::vector<uint8_t> getZombieRewardFunctionBytes(DWORD zombieRewardFunctionAddress, DWORD rewardAddress, std::unordered_map<std::string, int> zombieRewards)
{

    DWORD firstZombieRewardAddress = zombieRewardFunctionAddress + 16;

    // Jumps to the
    std::vector<uint8_t> zombieRewardFunctionBytes = {
        0x8b,0x87,0xc4, 0x00, 0x00, 0x00,
        0x6b, 0xc0, 0x0B,
        0x05,
        0xAA,0xAA, 0xAA, 0xAA,
        0xff,
        0xe0,
    };

    std::vector<uint8_t> zombieRewardFunctionConditionBytes = {
        0x81, 0x05,
        0xAA, 0xAA, 0xAA, 0xAA,
        0xBB, 0xBB, 0xBB, 0xBB,
        0xc3};

    replaceConsecutiveBytes(zombieRewardFunctionBytes, 0xAA, firstZombieRewardAddress);
    replaceConsecutiveBytes(zombieRewardFunctionConditionBytes, 0xAA, rewardAddress);

    for (const auto &possibleZombie : POSSIBLE_ZOMBIES)
    {
        for (const auto &zombie : ZOMBIES)
        {
            if (zombie == possibleZombie)
            {
                std::vector<uint8_t> tempArray = zombieRewardFunctionConditionBytes;
                auto it = zombieRewards.find(zombie);
                int reward = it->second;
                replaceConsecutiveBytes(tempArray, 0xBB, reward);
                zombieRewardFunctionBytes.insert(zombieRewardFunctionBytes.end(), tempArray.begin(), tempArray.end());
            }
        }
    }
    return zombieRewardFunctionBytes;
}

std::vector<uint8_t> getNotEnoughSunHookBytes(DWORD rewardAddress, int unsuccessfulPlantReward)
{

    std::vector<uint8_t> notEnoughSunHookBytes = {
        0x81, 0x05,
        0xAA, 0xAA, 0xAA, 0xAA,
        0xBB, 0xBB, 0xBB, 0xBB,
        0xc3};

    replaceConsecutiveBytes(notEnoughSunHookBytes, 0xAA, rewardAddress);
    replaceConsecutiveBytes(notEnoughSunHookBytes, 0xBB, unsuccessfulPlantReward);

    return notEnoughSunHookBytes;
}

std::vector<uint8_t> getPlantKilledFunctionBytes(DWORD rewardAddress, int plantsKilledReward)
{

    std::vector<uint8_t> plantKilledFunctionBytes = {
        0x81,
        0x05,
        0xAA, 0xAA, 0xAA, 0xAA,
        0xBB, 0xBB, 0xBB, 0xBB,
        0xc3,
    };
    replaceConsecutiveBytes(plantKilledFunctionBytes, 0xAA, rewardAddress);
    replaceConsecutiveBytes(plantKilledFunctionBytes, 0xBB, plantsKilledReward);
    return plantKilledFunctionBytes;
}

std::vector<uint8_t> getPlantKilledHookBytes(DWORD plantKilledFunctionAddress)
{
    plantKilledFunctionAddress -= (0x468F5a + 5);

    std::vector<uint8_t> plantKilledHookBytes = {
        0xe8,
        0xAA, 0xAA, 0xAA, 0xAA,
        0xc3,
    };
    replaceConsecutiveBytes(plantKilledHookBytes, 0xAA, plantKilledFunctionAddress);
    return plantKilledHookBytes;
}

std::vector<uint8_t> getSpaceOccupiedHookBytes(DWORD rewardAddress, int unsuccessfulPlantReward)
{

    std::vector<uint8_t> spaceOccupiedHookBytes = {
        0x81, 0x05,
        0xAA, 0xAA, 0xAA, 0xAA,
        0xBB, 0xBB, 0xBB, 0xBB};
    addNOPToBytes(spaceOccupiedHookBytes, 3);
    replaceConsecutiveBytes(spaceOccupiedHookBytes, 0xAA, rewardAddress);
    replaceConsecutiveBytes(spaceOccupiedHookBytes, 0xBB, unsuccessfulPlantReward);
    return spaceOccupiedHookBytes;
}

std::vector<uint8_t> getZombieRewardHookBytes(DWORD rewardFunctionAddress)
{
    std::vector<uint8_t> zombieRewardHookBytes = {
        0xb8,
        0xAA, 0xAA, 0xAA, 0xAA,
        0xff, 0xd0,
        0x5f,
        0x8b, 0xe5,
        0x5d,
        0xc3};
    replaceConsecutiveBytes(zombieRewardHookBytes, 0xAA, rewardFunctionAddress);
    return zombieRewardHookBytes;
}

std::vector<uint8_t> getStartRoundFunction(std::vector<int> seeds, DWORD choosePlantFunctionAddress,
                                           DWORD letsRockFunctionAddress, DWORD addPlantFunctionAddress)
{
    std::vector<uint8_t> startRoundFunctionBytes = {
        0xb8,
        0xAA, 0xAA, 0xAA, 0xAA,
        0xff, 0xd0,
        0xb8,
        0xBB, 0xBB, 0xBB, 0xBB,
        0xff, 0xd0,
        0xc3};
    replaceConsecutiveBytes(startRoundFunctionBytes, 0xAA, addPlantFunctionAddress);
    replaceConsecutiveBytes(startRoundFunctionBytes, 0xBB, letsRockFunctionAddress);

    std::vector<uint8_t> chooseSeedBytes = {
        0xbf,
        0xAA, 0xAA, 0xAA, 0xAA,
        0xb8,
        0xBB, 0xBB, 0xBB, 0xBB,
        0xff, 0xd0};

    replaceConsecutiveBytes(chooseSeedBytes, 0xBB, choosePlantFunctionAddress);
    auto insertPos = startRoundFunctionBytes.begin();
    for (const int &seed : seeds)
    {
        std::vector<uint8_t> tempArray = chooseSeedBytes;
        replaceConsecutiveBytes(tempArray, 0xAA, seed);
        insertPos = startRoundFunctionBytes.insert(insertPos, tempArray.begin(), tempArray.end());
        insertPos += tempArray.size();
    }

    return startRoundFunctionBytes;
}

std::vector<uint8_t> getLetsRockFunctionBytes()
{
    std::vector<uint8_t> letsRockFunctionBytes = {
        0x8b, 0x35,
        0x50, 0x1C, 0x73, 0x00,
        0x8b, 0xb6, 0x20, 0x03, 0x00, 0x00,
        0x8b, 0xb6, 0xa0, 0x00, 0x00, 0x00,
        0x8b, 0x8e, 0x2c, 0x0d, 0x00, 0x00,
        0x8b, 0x96, 0x5c, 0x01, 0x00, 0x00,
        0xbf, 0x64, 0x00, 0x00, 0x00,
        0xb8, 0x31, 0x00, 0x00, 0x00,
        0xbb, 0x00, 0x00, 0x00, 0x00,
        0xbf,
        0x70, 0x3C, 0x49, 0x00,
        0xff, 0xd7,
        0xc3};
    return letsRockFunctionBytes;
}

std::vector<uint8_t> getAddPlantFunctionBytes()
{
    std::vector<uint8_t> addPlantFunctionBytes = {
        0x6a, 0xff,
        0x6a, 0x00,
        0x6a, 0x00,
        0xb8, 0x00, 0x00, 0x00, 0x00,
        0xba, 0x00, 0x00, 0x00, 0x00,
        0x8b, 0x35,
        0x50, 0x1c, 0x73, 0x00,
        0x8b, 0xb6, 0x20, 0x03, 0x00, 0x00,
        0x8b, 0xb6, 0xa0, 0x00, 0x00, 0x00,
        0x8b, 0x96, 0x28, 0x0d, 0x00, 0x00,
        0x8b, 0x8a, 0x20, 0x03, 0x00, 0x00,
        0xbb,
        0xd0, 0xf9, 0x54, 0x00,
        0xff, 0xd3,
        0xc3};
    return addPlantFunctionBytes;
}

std::vector<uint8_t> getChoosePlantFunctionBytes()
{
    std::vector<uint8_t> choosePlantFunctionBytes = {
        0x8b, 0x35,
        0x50, 0x1c, 0x73, 0x00,
        0x8b, 0xb6, 0x20, 0x03, 0x00, 0x00,
        0x8b, 0xb6, 0xa0, 0x00, 0x00, 0x00,
        0x8b, 0xde,
        0x8b, 0xd7,
        0xc1, 0xe2, 0x04,
        0x29, 0xfa,
        0x8b, 0x8c, 0x96, 0xe0, 0x00, 0x00, 0x00,
        0x8d, 0xac, 0x96, 0xbc, 0x00, 0x00, 0x00,
        0x8b, 0x86, 0x3c, 0x0d, 0x00, 0x00,
        0x89, 0x45, 0x28,
        0xc7, 0x45, 0x24, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x86, 0x3c, 0x0d, 0x00, 0x00,
        0xff, 0x86, 0x38, 0x0d, 0x00, 0x00,
        0xc3};
    return choosePlantFunctionBytes;
}

std::vector<uint8_t> getLawnmowerHookBytes(DWORD restartLevelAddress)
{
    std::vector<uint8_t> lawnmowerHookBytes = {
        0x53,
        0xbb,
        0xAA, 0xAA, 0xAA, 0xAA,
        0xff,
        0xd3,
        0x5b,
    };
    addNOPToBytes(lawnmowerHookBytes, 3);
    replaceConsecutiveBytes(lawnmowerHookBytes, 0xAA, restartLevelAddress);
    return lawnmowerHookBytes;
}

std::vector<uint8_t> getRestartLevelFunction(DWORD restartFlagAddress)
{
    std::vector<uint8_t> restartLevelFunctionBytes = {
        0x60,
        0x9c,
        0xc7, 0x05,
        0xAA, 0xAA, 0xAA, 0xAA,
        0x01, 0x00, 0x00, 0x00,
        0xb8, 0x06, 0x00, 0x00, 0x00,
        0xbb, 0x01, 0x00, 0x00, 0x00,
        0x8b, 0x15,
        0x50, 0x1c, 0x73, 0x00,
        0xc7, 0x82, 0xa8, 0x09, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0x8b, 0x05,
        0x50, 0x1c, 0x73, 0x00,
        0x8b, 0x88, 0x68, 0x08, 0x00, 0x00,
        0x8a, 0x91, 0x78, 0x57, 0x00, 0x00,
        0x88, 0x90, 0xac, 0x09, 0x00, 0x00,
        0x8b, 0x35,
        0x50, 0x1c, 0x73, 0x00,
        0x8b, 0x86, 0x18, 0x09, 0x00, 0x00,
        0x6a, 0x00,
        0x50,
        0xbf,
        0xd0, 0x41, 0x45, 0x00,
        0xff, 0xd7,
        0x83, 0x3d,
        0xAA, 0xAA, 0xAA, 0xAA,
        0x01,
        0x74, 0xf7,
        0x9d,
        0x61,
        0xc3};
    replaceConsecutiveBytes(restartLevelFunctionBytes, 0xAA, restartFlagAddress);
    return restartLevelFunctionBytes;
}

std::vector<uint8_t> getPlacePlantHookBytes(DWORD placePlantAddress)
{

    std::vector<uint8_t> placePlantHookBytes = {
        0x50,
        0xb8,
        0xAA, 0xAA, 0xAA, 0xAA,
        0xff, 0xd0,
        0x58,
        0xc3};
    replaceConsecutiveBytes(placePlantHookBytes, 0xAA, placePlantAddress);
    return placePlantHookBytes;
}

std::vector<uint8_t> getPlacePlantFunctionBytes(DWORD restartFlagAddress,
                                                DWORD seedSlotAddress,
                                                DWORD xCoordAddress,
                                                DWORD yCoordAddress)
{

    std::vector<uint8_t> placePlantFunctionBytes = {
        0x60,
        0x9c,
        0x83, 0x3d,
        0xAA, 0xAA, 0xAA, 0xAA,
        0xff,
        0x0f, 0x84, 0xA3, 0x00, 0x00, 0x00,
        0x8b, 0x1d,
        0x50, 0x1c, 0x73, 0x00,
        0x8b, 0x9b, 0x1c, 0x09, 0x00, 0x00,
        0x83, 0xfb, 0x03,
        0x0f, 0x85, 0x8E, 0x00, 0x00, 0x00,
        0x8b, 0x1d,
        0x50, 0x1c, 0x73, 0x00,
        0x8b, 0x9b, 0x68, 0x08, 0x00, 0x00,
        0x8b, 0x9b, 0x5C, 0x01, 0x00, 0x00,
        0x8b, 0x0d,
        0xAA, 0xAA, 0xAA, 0xAA,
        0x6b, 0xc9, 0x50,
        0x83, 0xc3, 0x5c,
        0x01, 0xcb,
        0x80, 0x7b, 0x14, 0x00,
        0x74, 0x68,
        0xc6, 0x43, 0x14, 0x00,
        0x53,
        0x8b, 0x1b,
        0x8b, 0x3d,
        0x50, 0x1c, 0x73, 0x00,
        0x8b, 0xbf, 0x68, 0x08, 0x00, 0x00,
        0x8b, 0x87, 0x50, 0x01, 0x00, 0x00,
        0x8b, 0x0d,
        0xAA, 0xAA, 0xAA, 0xAA,
        0x89, 0x48, 0x24,
        0x89, 0x58, 0x28,
        0xc7, 0x40, 0x30, 0x01, 0x00, 0x00, 0x00,
        0xff, 0x35,
        0xCC, 0xCC, 0xCC, 0xCC,
        0xff, 0x35,
        0xDD, 0xDD, 0xDD, 0xDD,
        0x57,
        0xba,
        0x50, 0x32, 0x41, 0x00,
        0xff, 0xd2,
        0x8b, 0x87, 0x50, 0x01, 0x00, 0x00,
        0xc7, 0x40, 0x30, 0x00, 0x00, 0x00, 0x00,
        0xc7, 0x40, 0x24, 0xff, 0xff, 0xff, 0xff,
        0xc7, 0x40, 0x28, 0xff, 0xff, 0xff, 0xff,
        0x5b,
        0x39, 0x97, 0x78, 0x01, 0x00, 0x00,
        0x74, 0x04,
        0xc6, 0x43, 0x14, 0x01,
        0xc7, 0x05,
        0xAA, 0xAA, 0xAA, 0xAA,
        0xff, 0xff, 0xff, 0xff,
        0x9d,
        0x61,
        0xc3};
    replaceConsecutiveBytes(placePlantFunctionBytes, 0xAA, seedSlotAddress);
    replaceConsecutiveBytes(placePlantFunctionBytes, 0xBB, restartFlagAddress);
    replaceConsecutiveBytes(placePlantFunctionBytes, 0xCC, yCoordAddress);
    replaceConsecutiveBytes(placePlantFunctionBytes, 0xDD, xCoordAddress);
    return placePlantFunctionBytes;
}
