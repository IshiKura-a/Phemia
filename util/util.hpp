#ifndef PHEMIA_UTIL_HPP
#define PHEMIA_UTIL_HPP

#include <iostream>
#include <string>
#include <cctype>

namespace util {
    uint64_t calArrayDim(std::vector<std::string* >* arrDim, std::vector<uint32_t>* arrSize) {
        uint64_t size = 1;
        for (auto dim: *arrDim) {
            uint32_t n = strtol(dim->c_str(), nullptr, 10);
            size *= n;
            arrSize->push_back(n);
        }
        return size;
    }

    uint64_t calArrayDim(std::vector<std::string* >* arrDim) {
        uint64_t size = 1;
        for (auto dim: *arrDim) {
            uint32_t n = strtol(dim->c_str(), nullptr, 10);
            size *= n;
        }
        return size;
    }
}
#endif //PHEMIA_UTIL_HPP
