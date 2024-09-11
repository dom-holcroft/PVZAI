#include <iostream>
#include <unordered_map>
#include <vector>
#include <array>
#include <string>
#include <iostream>

std::vector<int> processArrayThroughMap(const std::vector<std::string>& inputArray, const std::unordered_map<std::string, int>& map) {
    std::vector<int> outputArray;

    for (const std::string& value : inputArray) {
        auto it = map.find(value);
        if (it != map.end()) {
            outputArray.push_back(it->second);
        } else {
            outputArray.push_back(-1); 
        }
    }

    return outputArray;
}

