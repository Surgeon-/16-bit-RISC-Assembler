
#ifndef PUNNING_HPP
#define PUNNING_HPP

#include <cstring>
#include <type_traits>
#include <c++/4.6/type_traits>

namespace gen {
    
    template<typename T>
    typename std::make_unsigned<T>::type
    pun_s_to_u(typename std::make_signed<T>::type value) {
        
        static_assert(std::is_integral<T>::value,
                      "gen::pun_s_to_u(...) - Type must be an integral type.");
        
        typename std::make_unsigned<T>::type rv;
        
        std::memcpy(&rv, &value, sizeof(T));
        
        return rv;
        
    }
    
    template<typename T>
    typename std::make_signed<T>::type 
    pun_u_to_s( typename std::make_unsigned<T>::type value) {
        
        static_assert(std::is_integral<T>::value,
                      "gen::pun_u_to_s(...) - Type must be an integral type.");
        
        typename std::make_signed<T>::type rv;
        
        std::memcpy(&rv, &value, sizeof(T));
        
        return rv;
        
    }
    
}


#endif /* PUNNING_HPP */

