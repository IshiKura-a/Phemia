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

    std::string *escpaeStr(char ch[], int length)
    {
        std::vector<char> cVec;
        int i;
        char prev = '\0';
        for (i = 0; i < length; i++) {
            if (prev != '\\') {
                if (ch[i] != '\\') {
                    cVec.push_back(ch[i]);
                } else {
                    prev = '\\';
                }
            } else {
                switch (ch[i])
                {
                    case 'n': cVec.push_back('\n'); prev = '\0'; break;
                    case 't': cVec.push_back('\t'); prev = '\0'; break;
                    case 'b': cVec.push_back('\b'); prev = '\0'; break;
                    case '\'': cVec.push_back('\''); prev = '\0'; break;
                    case '\"': cVec.push_back('\"'); prev = '\0'; break;
                    case '0': cVec.push_back('\0'); prev = '\0'; break;
                    default: prev = '\0'; break;
                }
            }
        }
        return new std::string(cVec.begin(), cVec.end());
    }
}
#endif //PHEMIA_UTIL_HPP
