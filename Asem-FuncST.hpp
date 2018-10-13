
#ifndef ASEM_FUNCST_HPP
#define ASEM_FUNCST_HPP

#include "Assembler.hpp"

#include <vector>
#include <string>

namespace asem {

void SymTabProcGlobal(SymbolTable & st, const LineOfCode & loc);

void SymTabProcLabels(size_t line_ord, SymbolTable & st, const std::vector<std::string> & labels);

void SymTabProcText(size_t line_ord, SymbolTable & st);

void SymTabProcData(size_t line_ord, SymbolTable & st);

void SymTabProcROData(size_t line_ord, SymbolTable & st);

void SymTabProcBSS(size_t line_ord, SymbolTable & st);

void SymTabProcChar(SymbolTable & st, const LineOfCode & loc);

void SymTabProcWord(SymbolTable & st, const LineOfCode & loc);

void SymTabProcLong(SymbolTable & st, const LineOfCode & loc);

void SymTabProcAlign(SymbolTable & st, const LineOfCode & loc);

void SymTabProcSkip(SymbolTable & st, const LineOfCode & loc);

void SymTabProcInstruction(SymbolTable & st, const LineOfCode & loc);

}

#endif /* ASEM_FUNCST_HPP */

