#include "memorylocator.h"
#include <stdlib.h>
#include <stdbool.h>
#include <vector> 
#include <iostream>
#include <Windows.h>


MemoryLocator::MemoryLocator(DWORD baseAddress, std::vector<ptrdiff_t> offsets)
    : baseAddress(baseAddress), offsets(std::move(offsets)) {
    this->pointerCached = false;
    this->cachedPointerAddress = 0;
}

DWORD MemoryLocator::getAddressFromOffsets(HANDLE hProcess) {
    DWORD currentAddress = this->baseAddress;
    for (int i = 0; i < this->offsets.size(); ++i) {
        DWORD buffer = 0;

        if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(currentAddress), &buffer, sizeof(DWORD), NULL)) {
            std::cerr << "Failed to follow memory address: " << GetLastError() << std::endl;
            return 0;
        }

        currentAddress = buffer + this->offsets[i];
    }
    return currentAddress;
}

int MemoryLocator::getInt(HANDLE hProcess) {
    int value = 0;
    DWORD intAddress = getAddressFromOffsets(hProcess);
    if (intAddress == 0) {
        std::cerr << "Memory Address was invalid" << std::endl;
        return 0;
    }
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(intAddress), &value, sizeof(DWORD), NULL)) {
        std::cerr  << "Failed to read memory value: " << GetLastError() << "Address" << intAddress << std::endl;
        return 0;
    }
    return value;
}

float MemoryLocator::getFloat(HANDLE hProcess) {
    float value = 0;
    DWORD floatAddress = getAddressFromOffsets(hProcess);
    if (floatAddress == 0) {
        std::cerr << "Memory Address was invalid" << std::endl;
        return 0;
    }
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(floatAddress), &value, sizeof(int), NULL)) {
        std::cerr << "Failed to read memory value: " << GetLastError() << std::endl;
        return 0;
    }
    return value;
}

std::vector<uint8_t> MemoryLocator::getData(HANDLE hProcess, SIZE_T blockSize) {
    std::vector<uint8_t> data(blockSize);
    DWORD dataAddress = getAddressFromOffsets(hProcess);
    if(dataAddress == 0) {
        std::cerr << "Issues getting to memory address" << std::endl;
        return {};
    }
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(dataAddress), data.data(), blockSize, NULL)) {
        std::cerr << "Failed to read data block: " << GetLastError() << std::endl;
        return {};
    }
    return data;
}

