#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>

std::vector<int> processArrayThroughMap(const std::vector<std::string>& inputArray, const std::unordered_map<std::string, int>& map);

template <typename T, std::size_t X1, std::size_t Y1, std::size_t Z1, std::size_t X2, std::size_t Y2, std::size_t Z2>
std::array<std::array<std::array<T, Z1>, Y1>, X1 + X2> merge3DArray(
    const std::array<std::array<std::array<T, Z1>, Y1>, X1>& arr1,
    const std::array<std::array<std::array<T, Z2>, Y2>, X2>& arr2) {

    static_assert(Y1 == Y2 && Z1 == Z2, "The second and third dimensions must match for merging");

    std::array<std::array<std::array<T, Z1>, Y1>, X1 + X2> result;

    for (std::size_t i = 0; i < X1; ++i) {
        for (std::size_t j = 0; j < Y1; ++j) {
            std::copy(arr1[i][j].begin(), arr1[i][j].end(), result[i][j].begin());
        }
    }

    for (std::size_t i = 0; i < X2; ++i) {
        for (std::size_t j = 0; j < Y2; ++j) {
            std::copy(arr2[i][j].begin(), arr2[i][j].end(), result[X1 + i][j].begin());
        }
    }

    return result;
}
#endif