#ifndef ASEM_FUNCEH_HPP
#define ASEM_FUNCEH_HPP

#include "Asem-Enumeration.hpp"
#include "Asem-ELFHolder.hpp"
#include "Assembler.hpp"

namespace asem {

    void ElfHldProcSect(ELFHolder & eh, Section::Enum sect);

    void ElfHldProcSkip(ELFHolder & eh, const SymbolTable & st, const LineOfCode & loc);

    void ElfHldProcAlign(ELFHolder & eh, const SymbolTable & st, const LineOfCode & loc);

    void ElfHldProcChar(ELFHolder & eh, const SymbolTable & st, const LineOfCode & loc);

    void ElfHldProcWord(ELFHolder & eh, const SymbolTable & st, const LineOfCode & loc);

    void ElfHldProcLong(ELFHolder & eh, const SymbolTable & st, const LineOfCode & loc);
    
    void ElfHldProcInstr(ELFHolder & eh, const SymbolTable & st, const LineOfCode & loc);
    
}

#endif /* ASEM_FUNCEH_HPP */

