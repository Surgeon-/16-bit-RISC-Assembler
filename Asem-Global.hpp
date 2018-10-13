
#ifndef ASEM_GLOBAL_HPP
#define ASEM_GLOBAL_HPP

namespace asem {
    
    const int REQ_ARGS_ANY     = -1;
    const int REQ_ARGS_UNKNOWN = -2;
    const int REQ_ARGS_1_TO_3  = -3; // Align
    const int REQ_ARGS_1_TO_2  = -4; // Skip
    
    const char COMMENT_TOKEN = ';';
    
    extern bool DO_CHAIN_SECTIONS;
    
    extern size_t START_LOCATION;
    
}

#endif /* ASEM_GLOBAL_HPP */

