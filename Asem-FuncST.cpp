
#include "Assembler.hpp"
#include "Asem-Except.hpp"
#include "Asem-Enumeration.hpp"
#include "Asem-Func.hpp"
#include "Asem-FuncST.hpp"
#include "Asem-Global.hpp"
#include "StringUtil.hpp"

#include <vector>
#include <string>

#include <iostream> // TEMP!!!

using namespace gen;

namespace asem {

void SymTabProcGlobal(SymbolTable & st, const LineOfCode & loc) {

    for (const std::string & a : loc.arguments) {
        
        if ( ! (string_is_identifier(a) && !char_is_digit(a[0])) )
            throw SyntaxError(loc.line_ord, "Arguments of .global directive "
                                            "must be valid identifiers - "
                                            "Offender = [" + a + "].");
        
        switch (st.check(a)) {
            
            case SymbolTable::NOT_PRESENT:
                st.data.emplace( a, SymbolTableEntry(st.data.size(),
                                 Scope::Global, Section::Undefined,
                                 Section::Undefined, 0, false) );
                break;
                
            case SymbolTable::UNDEFINED:
            case SymbolTable::DEFINED:
                st.data[a].scope = Scope::Global;
                break;
            
        }
        
        //std::cout << "Setting symbol [" + a + "] to global.\n"; 
        
    }

}

void SymTabProcLabels(size_t line_ord, SymbolTable & st, const std::vector<std::string> & labels) {

    if (labels.empty()) return;
    
    Section::Enum sec = st.curr_section;

    if (sec == Section::Undefined)
        throw SyntaxError(line_ord, "Cannot have labels in an undefined section.");

    for (const std::string & l : labels) {

        int chk = st.check(l);

        if (chk == SymbolTable::DEFINED)
            throw SyntaxError(line_ord, "Attempting symbol [" + l + "] redefinition.");

        if (chk == SymbolTable::UNDEFINED) {

            st.data[l].section = sec;
            st.data[l].relative_to = sec;
            st.data[l].value   = (int)st.counter[sec];
            st.data[l].defined = true;              
            continue;

        }

        // Else:
        st.data.emplace(l,
            SymbolTableEntry(st.data.size(), Scope::Local, sec, sec, st.counter[sec], true) );

    }

}

void SymTabProcText(size_t line_ord, SymbolTable & st) {
    
    size_t lc = (st.curr_section == Section::Undefined || !DO_CHAIN_SECTIONS)
              ? (START_LOCATION) : (st.counter[st.curr_section]);

    st.curr_section = Section::Text;

    switch (st.check(".text")) {

        case SymbolTable::NOT_PRESENT:
            st.data.emplace( ".text", SymbolTableEntry(st.data.size(),
                             Scope::Local, st.curr_section, Section::Undefined, lc, true) );
            st.counter[st.curr_section] = lc;
            break;

        case SymbolTable::UNDEFINED:
            st.data[".text"].section = st.curr_section;
            st.data[".text"].value   = lc;
            st.data[".text"].defined = true;
            st.counter[st.curr_section] = lc;
            break;

        case SymbolTable::DEFINED:
            if (DO_CHAIN_SECTIONS)
                throw SyntaxError(line_ord, "Cannot split sections in Section-"
                                            "chaining mode.");
            break;

    }
    
}

void SymTabProcData(size_t line_ord, SymbolTable & st) {
    
    size_t lc = (st.curr_section == Section::Undefined || !DO_CHAIN_SECTIONS)
              ? (START_LOCATION) : (st.counter[st.curr_section]);
    
    st.curr_section = Section::Data;
    
    switch (st.check(".data")) {
            
        case SymbolTable::NOT_PRESENT:
            st.data.emplace( ".data", SymbolTableEntry(st.data.size(),
                             Scope::Local, st.curr_section, Section::Undefined, lc, true) );
            st.counter[st.curr_section] = lc;
            break;

        case SymbolTable::UNDEFINED:
            st.data[".data"].section = st.curr_section;
            st.data[".data"].value   = lc;
            st.data[".data"].defined = true;
            st.counter[st.curr_section] = lc;
            break;
            
        case SymbolTable::DEFINED:
            if (DO_CHAIN_SECTIONS)
                throw SyntaxError(line_ord, "Cannot split sections in Section-"
                                            "chaining mode.");
            break;
            
    }
    
}

void SymTabProcROData(size_t line_ord, SymbolTable & st) {
    
    size_t lc = (st.curr_section == Section::Undefined || !DO_CHAIN_SECTIONS)
              ? (START_LOCATION) : (st.counter[st.curr_section]);
    
    st.curr_section = Section::ROData;
    
    switch (st.check(".rodata")) {
            
        case SymbolTable::NOT_PRESENT:
            st.data.emplace( ".rodata", SymbolTableEntry(st.data.size(),
                             Scope::Local, st.curr_section, Section::Undefined, lc, true) );
            st.counter[st.curr_section] = lc;
            break;

        case SymbolTable::UNDEFINED:
            st.data[".rodata"].section = st.curr_section;
            st.data[".rodata"].value   = lc;
            st.data[".rodata"].defined = true;
            st.counter[st.curr_section] = lc;
            break;
            
        case SymbolTable::DEFINED:
            if (DO_CHAIN_SECTIONS)
                throw SyntaxError(line_ord, "Cannot split sections in Section-"
                                            "chaining mode.");
            break;
            
    }
    
}

void SymTabProcBSS(size_t line_ord, SymbolTable & st) {
    
    size_t lc = (st.curr_section == Section::Undefined || !DO_CHAIN_SECTIONS)
              ? (START_LOCATION) : (st.counter[st.curr_section]);
    
    st.curr_section = Section::BSS;
    
    switch (st.check(".bss")) {
            
        case SymbolTable::NOT_PRESENT:
            st.data.emplace( ".bss", SymbolTableEntry(st.data.size(),
                             Scope::Local, st.curr_section, Section::Undefined, lc, true) );
            st.counter[st.curr_section] = lc;
            break;

        case SymbolTable::UNDEFINED:
            st.data[".bss"].section = st.curr_section;
            st.data[".bss"].value   = lc;
            st.data[".bss"].defined = true;
            st.counter[st.curr_section] = lc;
            break;
            
        case SymbolTable::DEFINED:
            if (DO_CHAIN_SECTIONS)
                throw SyntaxError(line_ord, "Cannot split sections in Section-"
                                            "chaining mode.");
            break;
            
    }
    
}

void SymTabProcChar(SymbolTable & st, const LineOfCode & loc) {
    
    if (st.curr_section == Section::Undefined)
        throw SyntaxError(loc.line_ord, "Cannot have chars in an undefined section.");
    
    st.counter[st.curr_section] += loc.arguments.size() * (1);
    
}

void SymTabProcWord(SymbolTable & st, const LineOfCode & loc) {
    
    if (st.curr_section == Section::Undefined)
        throw SyntaxError(loc.line_ord, "Cannot have words in an undefined section.");
    
    st.counter[st.curr_section] += loc.arguments.size() * (2);
    
}

void SymTabProcLong(SymbolTable & st, const LineOfCode & loc) {
    
    if (st.curr_section == Section::Undefined)
        throw SyntaxError(loc.line_ord, "Cannot have longs in an undefined section.");
    
    st.counter[st.curr_section] += loc.arguments.size() * (4);
    
}

void SymTabProcAlign(SymbolTable & st, const LineOfCode & loc) {
    
    if (st.curr_section == Section::Undefined)
        throw SyntaxError(loc.line_ord, "Cannot align in an undefined section.");
    
    if (loc.arguments.size() < 1u)
        throw SyntaxError(loc.line_ord, ".align must have at least one argument.");
    
    ////////////////////////////////////////////////////////////////////////////
    
    size_t num;
    size_t max = 256;
    
    ExpressionValue ev;
    
    try {
        ev = st.eval(loc.line_ord, loc.arguments[0]);
    } catch (SyntaxError & ex) {
        throw SyntaxError( loc.line_ord, std::string("Cannot evaluate .align argument >> ") +
                           ex.what() );
    }
    
    if (!ev.constant)
        throw SyntaxError( loc.line_ord, std::string(".align argument [") +
                           loc.arguments[0] + "] is not a constant." );
    
    num = size_t(ev.value);
    
    if (num > 256)
        throw SyntaxError( loc.line_ord, "Maximum alignment (.align) is 256 bytes.");
    
    if (loc.arguments.size() == 3) {
            
            try {
                ev = st.eval(loc.line_ord, loc.arguments[2]);
            } catch (SyntaxError & ex) {
                throw SyntaxError( loc.line_ord, std::string("Cannot evaluate .align argument >> ") +
                                   ex.what() );
            }

            if (!ev.constant)
                throw SyntaxError( loc.line_ord, std::string(".align argument [") +
                                   loc.arguments[2] + "] is not a constant." );
            
            max = size_t(ev.value);
            
        }
    
    ////////////////////////////////////////////////////////////////////////////
    
    /*std::cout << loc.arguments.size() << "\n";
    std::cout << "ALIGN: num = " << num << ", max = " << max << "\n";*/
    
    if (num == 0 || max == 0) return;
    
    size_t curr = st.counter[st.curr_section];
    
    if (curr % num == 0) return;
    
    size_t add = (num - (curr % num));
    
    if (add <= max)
        st.counter[st.curr_section] += add;
    
}

void SymTabProcSkip(SymbolTable & st, const LineOfCode & loc) {
    
    if (st.curr_section == Section::Undefined)
        throw SyntaxError(loc.line_ord, "Cannot skip in an undefined section.");
    
    if (loc.arguments.size() < 1u)
        throw SyntaxError(loc.line_ord, ".skip must have at least one argument.");
    
    ExpressionValue ev;
    try {
        ev = st.eval(loc.line_ord, loc.arguments[0]);
    } catch (SyntaxError & ex) {
        throw SyntaxError( loc.line_ord, std::string("Cannot evaluate .skip argument >> ") +
                           ex.what() );
    }
    
    if (!ev.constant)
        throw SyntaxError( loc.line_ord, std::string(".skip argument [") +
                           loc.arguments[0] + "] is not a constant." );
    
    st.counter[st.curr_section] += ev.value;
    
}

void SymTabProcInstruction(SymbolTable & st, const LineOfCode & loc) {
    
    if (st.curr_section == Section::Undefined)
        throw SyntaxError(loc.line_ord, "Cannot have instructions in an undefined section.");
    
    if (loc.arguments.size() > 2)
        throw SyntaxError(loc.line_ord, "No instruction may have more than 2 arguments.");
    
    size_t size = 2;
    
    for (const std::string & a : loc.arguments) {
        size += GetArgumentSize(a);
    }
    
    if (size > 4)
        throw SyntaxError(loc.line_ord, "Instruction size would be " +
                          std::to_string(size) + " bytes (max is 4).");
    
    st.counter[st.curr_section] += size;
    
    // Search for used symbols:
    for (const OperandDesc & desc : loc.operands) {
        
        std::string symbol = "";
        
        switch (desc.type) {
            
            case Operand::SymbolValue:
            case Operand::SymbolDeref:
            case Operand::RegIndirExp:
            case Operand::PCRel:
                symbol = desc.identifier;
                break;
                
            default:
                // Do nothing
                break;
                
        }
        
        if (!symbol.empty()) {
            
            if (st.check(symbol) == SymbolTable::NOT_PRESENT) {
                
                st.data.emplace( symbol, SymbolTableEntry(st.data.size(),
                                 Scope::Local, Section::Undefined, Section::Undefined, 0, false) );
                
            }
            
        }
        
    }
    
}

}