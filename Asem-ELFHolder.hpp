/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Asem-ELFHolder.hpp
 * Author: etf
 *
 * Created on June 11, 2018, 10:40 AM
 */

#ifndef ASEM_ELFHOLDER_HPP
#define ASEM_ELFHOLDER_HPP

#include "Asem-Enumeration.hpp"
#include "Assembler.hpp"

#include <list>
#include <vector>

namespace asem {
    
    using std::size_t;
    
    struct RelocRecord {
        
        RelocType::Enum type;
        size_t offset;
        size_t value;
        
        RelocRecord
            ( RelocType::Enum type = RelocType::Undefined
            , size_t offset = 0
            , size_t value = 0
            )
            : type(type)
            , offset(offset)
            , value(value)
            { }
        
    };
    
    ////////////////////////////////////////////////////////////////////////////
    
    struct BinaryVector {
        
        std::vector<unsigned char> data;
        
        void addByte(unsigned char  val);
        void addWord(unsigned short val);
        void addLong(unsigned int   val);
        
    };
    
    ////////////////////////////////////////////////////////////////////////////
    
    class ELFHolder {
        
    public:
        
        std::list<RelocRecord> relocations[4];
        
        BinaryVector sections[4];
        
        size_t skip[4];
        
        Section::Enum curr_section;
        
        void clear();
        void loadFromAssemblyList(const AssemblyList & al);
        
        std::string  rrToString(Section::Enum sec) const;
        std::string secToString(Section::Enum sec) const;
        
        size_t locCnt(Section::Enum sec) const {
            
            return (sections[sec].data.size() + skip[sec]);
            
        }
        
        void clearBSS();
        
    };
    
}

#endif /* ASEM_ELFHOLDER_HPP */

