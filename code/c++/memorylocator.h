#ifndef MEMORY_LOCATOR_H
#define MEMORY_LOCATOR_H

#include <Windows.h>
#include <vector>

class MemoryLocator {
private:
    DWORD baseAddress;                  // The base address to start from
    std::vector<ptrdiff_t> offsets;     // A vector of offsets to navigate through
    bool pointerCached;                 // Flag to indicate if the pointer is cached
    DWORD cachedPointerAddress;         // Cached pointer address

public:
    // Constructor
    MemoryLocator(DWORD baseAddress, std::vector<ptrdiff_t> offsets);

    // Function to get the final address after applying the offsets
    DWORD getAddressFromOffsets(HANDLE hProcess);
    int getInt(HANDLE hProcess);
    float getFloat(HANDLE hProcess);
    std::vector<uint8_t> getData(HANDLE hProcess, SIZE_T blockSize);
};

#endif // MEMORY_LOCATOR_H