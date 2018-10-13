
#ifndef ASEM_FUNC_HPP
#define ASEM_FUNC_HPP

#include "Asem-Enumeration.hpp"

#include <string>

namespace asem {

    Command::Enum EnumerateCommand(std::string command, Predicate::Enum & predicate);

    bool CommandIsInstruction(Command::Enum comm);

    int  CommandRequiredArgs(Command::Enum comm);

    size_t GetArgumentSize(const std::string & argument);

    const char * SectionToString(Section::Enum section);

    const char * ScopeToString(Scope::Enum scope);

    const char * RelocTypeToString(RelocType::Enum mode);

    bool IdentifyOperand(const std::string & arg, OperandDesc & desc);

    AddrMode::Enum OperandAddrMode(Operand::Enum type);
       
}

#endif /* ASEM_FUNC_HPP */

