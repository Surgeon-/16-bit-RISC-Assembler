
#include "Asem-Enumeration.hpp"

#include <iostream> // TEMP!!!
#include <stdexcept>
#include <string>

#include "StringUtil.hpp"
#include "Assembler.hpp"
#include "Asem-Except.hpp"
#include "Asem-Global.hpp"
#include "Asem-Func.hpp"

namespace asem {

    Command::Enum EnumerateCommand(std::string command, Predicate::Enum & predicate) {
        
        if (command.size() < 2) {
            return Command::Unknown;
        }
        
        if (command == ".global") return Command::Global;
        if (command == ".text")   return Command::Text;
        if (command == ".data")   return Command::Data;
        if (command == ".rodata") return Command::ROData;
        if (command == ".bss")    return Command::BSS;
        if (command == ".end")    return Command::End;
        if (command == ".char")   return Command::Char;
        if (command == ".word")   return Command::Word;
        if (command == ".long")   return Command::Long;
        if (command == ".align")  return Command::Align;
        if (command == ".skip")   return Command::Skip;
        
        size_t len = command.size();
        
        std::string pred = command.substr(len - 2);
        
        if (pred == "eq") {
            predicate = Predicate::Eq;
        } else if (pred == "ne") {
            predicate = Predicate::Ne;
        } else if (pred == "gt") {
            predicate = Predicate::Gt;
        } else if (pred == "al") {
            predicate = Predicate::Al;
        } else {
            predicate = Predicate::Al;
            goto RETURN;
        }
        
        command = command.substr(0, len - 2);
        len -= 2;
        
        RETURN:
        
        //std::cout << command << " | "<< (int)predicate << "\n";
        
        if (command == "add")  return Command::Add;
        if (command == "sub")  return Command::Sub;
        if (command == "mul")  return Command::Mul;
        if (command == "div")  return Command::Div;
        if (command == "cmp")  return Command::Cmp;
        if (command == "and")  return Command::And;
        if (command == "or")   return Command::Or;
        if (command == "not")  return Command::Not;
        if (command == "test") return Command::Test;
        if (command == "push") return Command::Push;
        if (command == "pop")  return Command::Pop;
        if (command == "call") return Command::Call;
        if (command == "iret") return Command::Iret;
        if (command == "mov")  return Command::Mov;
        if (command == "shl")  return Command::Shl;
        if (command == "shr")  return Command::Shr;
        if (command == "ret")  return Command::RetP;
        if (command == "jmp")  return Command::JmpP;
        
        return Command::Unknown;
        
    }

    bool CommandIsInstruction(Command::Enum comm) {
        
        return ((comm >= Command::Add) && (comm < Command::Count));
        
    }

    int  CommandRequiredArgs(Command::Enum comm) {
        
        switch (comm) {
            
            // Directives:
            case Command::Global:
                return REQ_ARGS_ANY;
                
            case Command::Text:
            case Command::Data:
            case Command::ROData:
            case Command::BSS:
            case Command::End:
                return 0;
            
            case Command::Char:
            case Command::Word:
            case Command::Long:
                return REQ_ARGS_ANY;
                
            case Command::Align:
                return REQ_ARGS_1_TO_3;
                
            case Command::Skip:
                return REQ_ARGS_1_TO_2;
        
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
            case Command::Shl:
            case Command::Shr:
            case Command::Mov:
                return 2;
                
            case Command::Push:
            case Command::Pop:
            case Command::Call:
            case Command::JmpP:
                return 1;
                
            case Command::Iret:
            case Command::RetP:
                return 0;
                
            // Unknown:
            default:
                return REQ_ARGS_UNKNOWN;
      
        }
        
    }

    size_t GetArgumentSize(const std::string & argument) {
        
        if (argument.size() == 2) { 
            
            if (argument[0] == 'r' && gen::char_is_digit(argument[1]))
                return size_t(0);
            
            if (argument == "pc" || argument == "sp") return size_t(0);
        
        }
        
        if (argument == "psw") return size_t(0);
        
        return size_t(2);
        
    }

    const char * SectionToString(Section::Enum section) {
        
        switch (section) {
            
            case Section::Undefined: return "Undef.";
                
            case Section::ROData: return "ROData";
            case Section::Text:   return "Text";
            case Section::Data:   return "Data";
            case Section::BSS:    return "BSS";
               
            default: return "Error";
            
        }
        
    }

    const char * ScopeToString(Scope::Enum scope) {
        
        switch (scope) {
            
            case Scope::Undefined: return "Undef.";
            case Scope::Global:    return "Global";
            case Scope::Local:     return "Local";
               
            
        }
        
    }

    const char * RelocTypeToString(RelocType::Enum mode) {
        
        switch (mode) {
            
            
            case RelocType::Abs_08: return "R_ABS_08";
            case RelocType::Abs_16: return "R_ABS_16";
            case RelocType::Abs_32: return "R_ABS_32";
                
            case RelocType::PCRel_16: return "R_PCR_16";
                
            default: return "R_UNDEF.";
            
        }
        
    }

    bool IdentifyOperand(const std::string & arg, OperandDesc & desc) {
        
        if (arg.empty()) return false;
        
        // SPECIAL IDENTIFIERS (pc, sp, psw):
        
        if (arg == "pc") {
            desc.type = Operand::RegDirect;
            desc.reg_number = 7;
            return true;   
        }
        
        if (arg == "sp") {
            desc.type = Operand::RegDirect;
            desc.reg_number = 6;
            return true;
        }
        
        if (arg == "psw") {
            desc.type = Operand::PSW;
            desc.reg_number = 7;
            return true;
        }
        
        ////////////////////////////////////////////////////////////////////////
        
        if (gen::string_is_integer(arg)) {
            desc.type = Operand::Literal;
            desc.number = std::stoi(arg, 0, 0);
            return true;
        }
        
        if (arg.size() == 1 && gen::string_is_identifier(arg) && !gen::char_is_digit(arg[0])) {
            desc.type = Operand::SymbolDeref;
            desc.identifier = arg;
            return true;
        }
        
        if (arg.size() < 2) return false;
        
        if (arg.size() == 2 && arg[0] == 'r' && gen::char_is_digit(arg[1])) {
            desc.type = Operand::RegDirect;
            desc.reg_number = (arg[1] - '0');
            return true;            
        }
        
        if (gen::string_is_identifier(arg) && !gen::char_is_digit(arg[0])) {
            desc.type = Operand::SymbolDeref;
            desc.identifier = arg;
            return true;
        }
        
        if (arg[0] == '&' && gen::string_is_identifier(arg.c_str() + 1) &&
            gen::char_is_digit(arg[1]) == false) {
            
            desc.type = Operand::SymbolValue;
            desc.identifier = std::string{arg, 1, std::string::npos};
            return true;
            
        }
        
        if (arg[0] == '$' && gen::string_is_identifier(arg.c_str() + 1) &&
            gen::char_is_digit(arg[1]) == false) {
            
            desc.type = Operand::PCRel;
            desc.identifier = std::string{arg, 1, std::string::npos};
            desc.reg_number = 7;
            return true;
            
        }
        
        if (arg[0] == '*' && gen::string_is_integer( arg.c_str() + 1 )) {
            
            desc.type = Operand::LiteralIndir;
            desc.number = std::stoi( std::string{arg, 1, std::string::npos} , 0, 0);
            return true;
            
        }
        
        ////////////////////////////////////////////////////////////////////////
        
        size_t open = arg.find('[');
        size_t clos = arg.find(']');
        
        if (open == std::string::npos || clos == std::string::npos || clos <= open + 1)
            return false;
        
        std::string head{arg, 0, open};
        head = gen::string_crop(head);
        
        if (head.size() != 2u) return false;
        
        // SPECIAL IDENTIFIERS (ps, sp):
        head = gen::string_replace_all_identifiers(head, "pc", "r7");
        head = gen::string_replace_all_identifiers(head, "sp", "r6");
        
        if (head[0] != 'r' || !gen::char_is_digit(head[1])) return false;
        
        std::string index = std::string{arg, open + 1, clos - (open + 1)};
        
        if (gen::string_is_integer(index)) {
            desc.type = Operand::RegIndir;
            desc.reg_number = (head[1] - '0');
            desc.number = std::stoi(index, 0, 0);
            return true;
        }
        
        if (gen::string_is_identifier(index) && !gen::char_is_digit(index[0])) {
            desc.type = Operand::RegIndirExp;
            desc.reg_number = (head[1] - '0');
            desc.identifier = index;
            return true;
        }
            
        ////////////////////////////////////////////////////////////////////////
        
        return false;
        
    }

    AddrMode::Enum OperandAddrMode(Operand::Enum type) {
        
        switch (type) {
            
            case Operand::Literal:
            case Operand::SymbolValue:
            case Operand::PSW:
                return AddrMode::Imm;
                
            case Operand::SymbolDeref:
            case Operand::LiteralIndir:
                return AddrMode::MemDir;
                
            case Operand::RegDirect:
                return AddrMode::RegDir;
                
            case Operand::RegIndir:
            case Operand::RegIndirExp:
            case Operand::PCRel:
                return AddrMode::RegInd;
            
        }
        
    }
    
}