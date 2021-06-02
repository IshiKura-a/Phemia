#ifndef PHEMIA_UTIL_HPP
#define PHEMIA_UTIL_HPP

#include <iostream>
#include <string>
#include <cctype>

namespace util {
    template<typename Base, typename T>
    inline bool instanceof(const T*) {
        return std::is_base_of<Base, T>::value;
    }
    template<typename Base, typename T>
    inline bool instanceof(const T) {
        return std::is_base_of<Base, T>::value;
    }

}
#endif //PHEMIA_UTIL_HPP
