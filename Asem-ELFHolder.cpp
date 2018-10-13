
#include "Asem-ELFHolder.hpp"
#include "Asem-Enumeration.hpp"
#include "Assembler.hpp"
#include "Asem-Func.hpp"
#include "Asem-FuncEH.hpp"
#include "StringUtil.hpp"

#include <cstring>
#include <iostream>

#define UCHAR unsigned char

namespace asem {
    
    void BinaryVector::addByte(unsigned char  val) {
        
        data.push_back(val);
        
    }
    
    void BinaryVector::addWord(unsigned short val) {

        UCHAR arr[2];
        
        std::memcpy(arr, &val, 2);
        
        data.push_back( arr[0] );
        data.push_back( arr[1] );
        
    }
    
    void BinaryVector::addLong(unsigned int   val) {
        
        UCHAR arr[4];
        
        std::memcpy(arr, &val, 4);
        
        data.push_back( arr[0] );
        data.push_back( arr[1] );
        data.push_back( arr[2] );
        data.push_back( arr[3] );
        
    }
    
    ////////////////////////////////////////////////////////////////////////////
    
    void ELFHolder::clear() {
        
        for (size_t i = 0; i < 4; i += 1) {
            relocations[i].clear();
            sections[i].data.clear();
            skip[0] = 0u;
        }
        
        curr_section = Section::Undefined;
        
    }
    
    void ELFHolder::loadFromAssemblyList(const AssemblyList & al) {
        
        clear();
        
        const SymbolTable & st = al.symtab;
        
        for (const LineOfCode & loc : al.data) {
            
            switch (loc.command) {
            
                // Directives:
                case Command::Global: /* Do nothing */ break;
                case Command::Text:   ElfHldProcSect(*this, Section::Text);   break;
                case Command::Data:   ElfHldProcSect(*this, Section::Data);   break;
                case Command::ROData: ElfHldProcSect(*this, Section::ROData); break;
                case Command::BSS:    ElfHldProcSect(*this, Section::BSS);    break;
                case Command::End:    /* Do nothing */ break;
                case Command::Char:   ElfHldProcChar(*this, st, loc); break; 
                case Command::Word:   ElfHldProcWord(*this, st, loc); break; 
                case Command::Long:   ElfHldProcLong(*this, st, loc); break; 
                case Command::Align:  ElfHldProcAlign(*this, st, loc);break;
                case Command::Skip:   ElfHldProcSkip(*this, st, loc); break;

                // Instructions:
                case Command::Add:
                case Command::Sub:
                case Command::Mul:
                case Command::Div:
                case Command::Cmp:
                case Command::And:
                case Command::Or:
                case Command::Not:
                case Command::Test:
                case Command::Mov:
                case Command::Shl:
                case Command::Shr:
                    
                case Command::Push:
                case Command::Pop:
                case Command::Call:
                    
                case Command::Iret:
                    
                case Command::RetP: 
                case Command::JmpP:  ElfHldProcInstr(*this, st, loc); break;
            
            }
            
        }
        
    }
    
    std::string  ELFHolder::rrToString(Section::Enum sec) const {
        
        if (relocations[sec].empty()) return "";
        
        std::string rv{"#.ret"};
        
        switch (sec) {
            case Section::Text:   rv += ".text\n";   break;
            case Section::Data:   rv += ".data\n";   break;
            case Section::BSS:    rv += ".bss\n";    break;
            case Section::ROData: rv += ".rodata\n"; break;
        }
        
        char buffer[128];
        
        snprintf( buffer, sizeof(buffer)
                , "%-9s  %-10s  %6s\n"
                , "#Offset"
                , "Type"
                , "SymOrd"
                ) ;
        
        rv += buffer;
        
        for (const RelocRecord & rr : relocations[sec]) {
            
            snprintf( buffer, sizeof(buffer)
                    , "0x%07X  %-10s  0x%04X\n"
                    , (int)rr.offset
                    , RelocTypeToString(rr.type)
                    , (int)rr.value
                    ) ;
        
            rv += buffer;
            
        }
        
        rv.pop_back();
        
        return rv;
        
    }
    
    std::string ELFHolder::secToString(Section::Enum sec) const {
        
        if (sections[sec].data.empty()) return "";
        
        std::string rv{"#"};
        
        switch (sec) {
            case Section::Text:   rv += ".text\n";   break;
            case Section::Data:   rv += ".data\n";   break;
            case Section::BSS:    rv += ".bss\n";    break;
            case Section::ROData: rv += ".rodata\n"; break;
        }
        
        rv += gen::buffer_to_hex( &sections[sec].data[0] 
                                ,  sections[sec].data.size() // sizeof(data[0]) = 1
                                ,  ' '
                                ) ;
        
        return rv;
        
    }
    
    void ELFHolder::clearBSS() {
        
        relocations[Section::BSS].clear();
        
        size_t len = sections[Section::BSS].data.size();
        
        std::memset( &(sections[Section::BSS].data[0]), 0, len);
        
    }
    
}

#undef UCHAR