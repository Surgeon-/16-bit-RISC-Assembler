
#include "Asem-Enumeration.hpp"
#include "Asem-ELFHolder.hpp"
#include "Asem-Func.hpp"
#include "Asem-Except.hpp"
#include "Assembler.hpp"
#include "StringUtil.hpp"
#include "Asem-Global.hpp"
#include "Punning.hpp"

#include <iostream> // TEMP!

#define UINT   unsigned int
#define USHORT unsigned short
#define UCHAR  unsigned char

namespace asem {

    void ElfHldProcSect(ELFHolder & eh, Section::Enum sect) {
        
        if (!DO_CHAIN_SECTIONS) {
            
            eh.curr_section = sect;
            
            eh.skip[eh.curr_section] = START_LOCATION;
            
        }
        else {
            
            size_t lc = (eh.curr_section == Section::Undefined)
                      ? (START_LOCATION) : eh.locCnt(eh.curr_section);
            
            eh.curr_section = sect;
            
            eh.skip[eh.curr_section] = lc;
            
        }
        
    }

    void ElfHldProcSkip(ELFHolder & eh, const SymbolTable & st, const LineOfCode & loc) {
        
        size_t num = 0;
        char   val = 0;
        
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

        num = size_t(ev.value);
        
        if (loc.arguments.size() == 2) {
            
            try {
                ev = st.eval(loc.line_ord, loc.arguments[1]);
            } catch (SyntaxError & ex) {
                throw SyntaxError( loc.line_ord, std::string("Cannot evaluate .skip argument >> ") +
                                   ex.what() );
            }

            if (!ev.constant)
                throw SyntaxError( loc.line_ord, std::string(".skip argument [") +
                                   loc.arguments[1] + "] is not a constant." );
            
            val = char(ev.value);
            
        }
    
        for (size_t i = 0; i < num; i += 1)
            eh.sections[eh.curr_section].addByte( gen::pun_s_to_u<char>(val) );
        
    }

    void ElfHldProcAlign(ELFHolder & eh, const SymbolTable & st, const LineOfCode & loc) {

        size_t num;
        char val = 0;
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

        // ---------------------------------------------------------------------
        
        if (loc.arguments.size() >= 2) {

            try {
                ev = st.eval(loc.line_ord, loc.arguments[1]);
            } catch (SyntaxError & ex) {
                throw SyntaxError( loc.line_ord, std::string("Cannot evaluate .align argument >> ") +
                                   ex.what() );
            }

            if (!ev.constant)
                throw SyntaxError( loc.line_ord, std::string(".align argument [") +
                                   loc.arguments[1] + "] is not a constant." );

            val = char(ev.value);

        }
        
        // ---------------------------------------------------------------------
        
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

        if (num == 0 || max == 0) return;

        Section::Enum sec = eh.curr_section;
        
        size_t curr = eh.locCnt(sec);

        if (curr % num == 0) return;

        size_t add = (num - (curr % num));
        
        if (add <= max)
            for (size_t i = 0; i < add; i += 1)
                eh.sections[sec].addByte( gen::pun_s_to_u<char>(val) );
        
    }

    void ElfHldProcChar(ELFHolder & eh, const SymbolTable & st, const LineOfCode & loc) {
        
        static const char * sections[4] = { ".text", ".data", ".rodata",
                                            ".bss"};
        
        const Section::Enum sec = eh.curr_section;
        
        //std::cout << "Entered asem::ElfHldProcChar(...).\n";
        
        for (const std::string & a : loc.arguments) {
            
            //std::cout << ".word argument is [" + a + "] -> ";
            
            if (gen::string_is_integer(a)) { // Number literal -----------------
                
                //std::cout << "Integer.\n";
                
                char val = static_cast<char>( std::stoi(a, 0, 0) );
                
                eh.sections[sec].addByte( gen::pun_s_to_u<char>(val) );
                
            }
            else if (gen::string_is_identifier(a)) { // One symbol -------------
                
                //std::cout << "Identifier.\n";
                
                if (st.check(a) == SymbolTable::NOT_PRESENT)
                    throw std::logic_error("asem::ElfHldProcChar(...) -Symbol "
                            "[" + a + "] not present in SymTab but access is"
                            " attempted.");
                
                const SymbolTableEntry & ste = st.data.at(a);
                
                if (ste.defined) {
                    
                    char val = static_cast<char>( ste.value );
                    
                    if (ste.relative_to != Section::Undefined) {
                        
                        size_t off = eh.locCnt(sec);
                        
                        size_t rrval;
                        if (ste.scope == Scope::Local) {
                            val   -= st.data.at(sections[ste.relative_to]).value;
                            rrval  = st.data.at(sections[ste.relative_to]).ordinal;
                        }
                        else {
                            rrval = ste.ordinal;
                            val = 0;
                        }
                        
                        eh.relocations[sec].push_back(
                            RelocRecord(RelocType::Abs_08, off, rrval) ); // PEP
                        
                    }
                    else
                        throw std::logic_error("RelocRecord relative to unknown section.");
                    
                    eh.sections[sec].addByte( gen::pun_s_to_u<char>(val) );
                    
                }
                else { // To be imported
                    
                    size_t off = eh.locCnt(sec);
                        
                    eh.relocations[sec].push_back(
                        RelocRecord(RelocType::Abs_08, off, ste.ordinal) );
                    
                    eh.sections[sec].addByte( 0 );
                    
                }
                
            }
            else { // Expression -----------------------------------------------
                
                //std::cout << "Expression.\n";
                
                ExpressionValue ev = st.eval(loc.line_ord, a);
                
                char val = static_cast<char>( ev.value );
                
                size_t off = eh.locCnt(sec);
                
                if (!ev.constant) {
                    
                    size_t sec_ord = st.data.at(sections[ev.relative_to]).ordinal;
                    
                    val -= st.data.at(sections[ev.relative_to]).value; 
                    
                    eh.relocations[sec].push_back(
                        RelocRecord(RelocType::Abs_08, off, sec_ord) );
                    
                }
                
                eh.sections[sec].addByte( gen::pun_s_to_u<char>(val) );
                
            } // End if/else chain
            
        } // End for
        
    }

    void ElfHldProcWord(ELFHolder & eh, const SymbolTable & st, const LineOfCode & loc) {
       
        static const char * sections[4] = { ".text", ".data", ".rodata",
                                            ".bss"};
        
        const Section::Enum sec = eh.curr_section;
        
        //std::cout << "Entered asem::ElfHldProcWord(...).\n";
        
        for (const std::string & a : loc.arguments) {
            
            //std::cout << ".word argument is [" + a + "] -> ";
            
            if (gen::string_is_integer(a)) { // Number literal -----------------
                
                //std::cout << "Integer.\n";
                
                short val = static_cast<short>( std::stoi(a, 0, 0) );
                
                eh.sections[sec].addWord( gen::pun_s_to_u<short>(val) );
                
            }
            else if (gen::string_is_identifier(a)) { // One symbol -------------
                
                //std::cout << "Identifier.\n";
                
                if (st.check(a) == SymbolTable::NOT_PRESENT)
                    throw std::logic_error("asem::ElfHldProcWord(...) -Symbol "
                            "[" + a + "] not present in SymTab but access is"
                            " attempted.");
                
                const SymbolTableEntry & ste = st.data.at(a);
                
                if (ste.defined) {
                    
                    short val = static_cast<short>( ste.value );
                    
                    if (ste.relative_to != Section::Undefined) {
                        
                        size_t off = eh.locCnt(sec);
                        
                        size_t rrval;
                        if (ste.scope == Scope::Local) {
                            val   -= st.data.at(sections[ste.relative_to]).value;
                            rrval  = st.data.at(sections[ste.relative_to]).ordinal;
                        }
                        else {
                            rrval = ste.ordinal;
                            val = 0;
                        }
                        
                        eh.relocations[sec].push_back(
                            RelocRecord(RelocType::Abs_16, off, rrval) ); // PEP
                        
                    }
                    else
                        throw std::logic_error("RelocRecord relative to unknown section.");
                    
                    eh.sections[sec].addWord( gen::pun_s_to_u<short>(val) );
                    
                }
                else { // To be imported
                    
                    size_t off = eh.locCnt(sec);
                        
                    eh.relocations[sec].push_back(
                        RelocRecord(RelocType::Abs_16, off, ste.ordinal) );
                    
                    eh.sections[sec].addWord( 0 );
                    
                }
                
            }
            else { // Expression -----------------------------------------------
                
                //std::cout << "Expression.\n";
                
                ExpressionValue ev = st.eval(loc.line_ord, a);
                
                short val = static_cast<short>( ev.value );
                
                size_t off = eh.locCnt(sec);
                
                if (!ev.constant) {
                    
                    size_t sec_ord = st.data.at(sections[ev.relative_to]).ordinal;
                    
                    val -= st.data.at(sections[ev.relative_to]).value; 
                    
                    eh.relocations[sec].push_back(
                        RelocRecord(RelocType::Abs_16, off, sec_ord) );
                    
                }
                
                eh.sections[sec].addWord( gen::pun_s_to_u<short>(val) );
                
            } // End if/else chain
            
        } // End for
        
    }

    void ElfHldProcLong(ELFHolder & eh, const SymbolTable & st, const LineOfCode & loc) {
        
        static const char * sections[4] = { ".text", ".data", ".rodata",
                                            ".bss"};
        
        const Section::Enum sec = eh.curr_section;
        
        //std::cout << "Entered asem::ElfHldProcLong(...).\n";
        
        for (const std::string & a : loc.arguments) {
            
            //std::cout << ".word argument is [" + a + "] -> ";
            
            if (gen::string_is_integer(a)) { // Number literal -----------------
                
                //std::cout << "Integer.\n";
                
                int val = static_cast<int>( std::stoi(a, 0, 0) );
                
                eh.sections[sec].addLong( gen::pun_s_to_u<int>(val) );
                
            }
            else if (gen::string_is_identifier(a)) { // One symbol -------------
                
                //std::cout << "Identifier.\n";
                
                if (st.check(a) == SymbolTable::NOT_PRESENT)
                    throw std::logic_error("asem::ElfHldProcChar(...) -Symbol "
                            "[" + a + "] not present in SymTab but access is"
                            " attempted.");
                
                const SymbolTableEntry & ste = st.data.at(a);
                
                if (ste.defined) {
                    
                    int val = static_cast<int>( ste.value );
                    
                    if (ste.relative_to != Section::Undefined) {
                        
                        size_t off = eh.locCnt(sec);
                        
                        size_t rrval;
                        if (ste.scope == Scope::Local) {
                            val   -= st.data.at(sections[ste.relative_to]).value;
                            rrval  = st.data.at(sections[ste.relative_to]).ordinal;
                        }
                        else {
                            rrval = ste.ordinal;
                            val = 0;
                        }
                        
                        eh.relocations[sec].push_back(
                            RelocRecord(RelocType::Abs_32, off, rrval) ); // PEP
                        
                    }
                    else
                        throw std::logic_error("RelocRecord relative to unknown section.");
                    
                    eh.sections[sec].addLong( gen::pun_s_to_u<int>(val) );
                    
                }
                else { // To be imported
                    
                    size_t off = eh.locCnt(sec);
                        
                    eh.relocations[sec].push_back(
                        RelocRecord(RelocType::Abs_32, off, ste.ordinal) );
                    
                    eh.sections[sec].addLong( 0 );
                    
                }
                
            }
            else { // Expression -----------------------------------------------
                
                //std::cout << "Expression.\n";
                
                ExpressionValue ev = st.eval(loc.line_ord, a);
                
                int val = static_cast<int>( ev.value );
                
                size_t off = eh.locCnt(sec);
                
                if (!ev.constant) {
                    
                    size_t sec_ord = st.data.at(sections[ev.relative_to]).ordinal;
                    
                    val -= st.data.at(sections[ev.relative_to]).value; 
                    
                    eh.relocations[sec].push_back(
                        RelocRecord(RelocType::Abs_32, off, sec_ord) );
                    
                }
                
                eh.sections[sec].addLong( gen::pun_s_to_u<int>(val) );
                
            } // End if/else chain
            
        } // End for
        
    }

    void ElfHldProcInstr(ELFHolder & eh, const SymbolTable & st, const LineOfCode & loc) {
        
        if (!CommandIsInstruction(loc.command))
            throw std::logic_error("asem::ElfHldProcInstr(...) - Command is not an instruction.");
        
        Section::Enum sec = eh.curr_section;
        
        if (sec != Section::Text)
            throw SyntaxError(loc.line_ord, "Instructions can only go in .text section.");
        
        USHORT instr = 0;
        USHORT data  = 0;
        
        AddrMode::Enum mode;
        int extend = -1;
        
        //std::cout << "ProcInstr; Instruction = " << loc.command - Command::Add << "\n";
        
        // PREDICATE / OPCODE: /////////////////////////////////////////////////
        
        instr |= ((USHORT)loc.predicate << 14);
        
        if (loc.command == Command::RetP) {
            instr |= (((USHORT)Command::Pop - Command::Add) << 10);
        }
        else if (loc.command == Command::JmpP) {
            Command::Enum jmp_com = (loc.operands[0].type == Operand::PCRel) ?
                                     Command::Add : Command::Mov;
            instr |= (((USHORT)jmp_com - Command::Add) << 10);
        }
        else
            instr |= (((USHORT)loc.command - Command::Add) << 10);
        
        //std::cout << std::hex << instr << std::dec << "\n";
        
        // OPERANDS: ///////////////////////////////////////////////////////////
        
        if (loc.operands.size() == 2) {
            
            // Dst:
            mode = OperandAddrMode(loc.operands[0].type);
            instr |= ( (USHORT)mode << 8 );
            instr |= ( (USHORT)loc.operands[0].reg_number << 5 );
            if (mode != AddrMode::RegDir) extend = 0;
            
            if (mode == AddrMode::Imm && loc.operands[0].reg_number != 0x7 &&
                loc.command != Command::Cmp && loc.command != Command::Test)
                throw SyntaxError(loc.line_ord, "Destination is immediate value.");
            
            // Src:
            mode = OperandAddrMode(loc.operands[1].type);
            instr |= ( (USHORT)mode << 3 );
            instr |= ( (USHORT)loc.operands[1].reg_number << 0 );
            if (mode != AddrMode::RegDir) extend = 1;
            
        }
        else {
            
            switch (loc.command) {
                
                case Command::Push:
                case Command::Call:
                    mode = OperandAddrMode(loc.operands[0].type);
                    instr |= ( (USHORT)mode << 3 );
                    instr |= ( (USHORT)loc.operands[0].reg_number << 0 );
                    if (mode != AddrMode::RegDir) extend = 0;
                    break;
                    
                case Command::Pop:
                    mode = OperandAddrMode(loc.operands[0].type);
                    instr |= ( (USHORT)mode << 8 );
                    instr |= ( (USHORT)loc.operands[0].reg_number << 5 );
                    if (mode != AddrMode::RegDir) extend = 0;
                    if (mode == AddrMode::Imm)
                        throw SyntaxError(loc.line_ord, "Destination is immediate value.");
                    break;
                    
                case Command::RetP:
                    mode = AddrMode::RegDir;
                    instr |= ( (USHORT)mode << 8 );
                    instr |= ( (USHORT)0x07 << 5 );
                    break;
                    
                case Command::JmpP:
                    // Dst
                    mode = AddrMode::RegDir;
                    instr |= ( (USHORT)mode << 8 );
                    instr |= ( (USHORT)0x07 << 5 );
                    instr |= ( (USHORT)loc.operands[0].reg_number << 0 );
                    // Src:
                    if (loc.operands[0].type != Operand::PCRel)
                        mode = OperandAddrMode(loc.operands[0].type);
                    else {
                        mode = AddrMode::Imm;
                        instr &= ~0x7; // reg_number <= 0
                    }
                    instr |= ( (USHORT)mode << 3 );
                    if (mode != AddrMode::RegDir) extend = 0;
                    break;
                    
                case Command::Iret:
                    // Do nothing
                    break;
                
            }
            
        }
        
        //std::cout << std::hex << instr << std::dec << "\n";
        
        /*std::cout << "Command = " << (loc.command - Command::Add) 
                  << "; instr = " << std::hex << instr << "\n";*/
        
        // Append instruction:
        eh.sections[sec].addWord(instr);
        
        // DATA: ///////////////////////////////////////////////////////////////
        
        if (extend != -1) {
            
            const OperandDesc & desc = loc.operands[extend];
            
            if (desc.type == Operand::PSW) goto SKIP_DATA;
            
            switch (desc.type) {
                
                case Operand::Literal:
                case Operand::RegIndir:
                case Operand::LiteralIndir:
                    data = gen::pun_s_to_u<short>( short(desc.number) );
                    break;
                    
                case Operand::SymbolValue:
                case Operand::SymbolDeref:
                case Operand::RegIndirExp:
                {
                    
                    const SymbolTableEntry & ste = st.data.at(desc.identifier);
                    
                    size_t off = eh.locCnt(sec);
                    
                    if (ste.scope == Scope::Global) {
                        
                        data = 0;
                        
                        eh.relocations[sec].emplace_back( RelocType::Abs_16,
                                                          off, ste.ordinal );
                        
                    }
                    else {
                        
                        size_t secord;
                        int    secval;
                        
                        switch (ste.section) {
                            case Section::Text: 
                                secord = st.data.at(".text").ordinal;
                                secval = st.data.at(".text").value;
                                break;
                            case Section::Data: 
                                secord = st.data.at(".data").ordinal;
                                secval = st.data.at(".data").value;
                                break;
                            case Section::BSS: 
                                secord = st.data.at(".bss").ordinal;
                                secval = st.data.at(".bss").value;
                                break;
                            case Section::ROData: 
                                secord = st.data.at(".rodata").ordinal;
                                secval = st.data.at(".rodata").value;
                                break;
                            default:
                                throw SyntaxError(loc.line_ord, "Symbol [" +
                                        desc.identifier + "] does not have "
                                        "its section defined.");
                        }
                        
                        data = gen::pun_s_to_u<short>( short(ste.value - secval) );
                        
                        eh.relocations[sec].emplace_back( RelocType::Abs_16,
                                                          off, secord );
                        
                    }
                    
                }
                break;
                
                case Operand::PCRel:
                {
                    
                    const SymbolTableEntry & ste = st.data.at(desc.identifier);
                    
                    size_t off = eh.locCnt(sec);
                    
                    if (ste.scope == Scope::Global) {
                        
                        data = gen::pun_s_to_u<short>( short(-4) );
                        
                        eh.relocations[sec].emplace_back( RelocType::PCRel_16,
                                                          off, ste.ordinal );
                        
                    }
                    else {
                        
                        size_t secord;
                        int    secval;
                        
                        switch (ste.section) {
                            case Section::Text:
                                secord = st.data.at(".text").ordinal;
                                secval = st.data.at(".text").value;
                                break;
                            case Section::Data: 
                                secord = st.data.at(".data").ordinal;
                                secval = st.data.at(".data").value;
                                break;
                            case Section::BSS: 
                                secord = st.data.at(".bss").ordinal;
                                secval = st.data.at(".bss").value;
                                break;
                            case Section::ROData: 
                                secord = st.data.at(".rodata").ordinal;
                                secval = st.data.at(".rodata").value;
                                break;
                            default:
                                throw SyntaxError(loc.line_ord, "Symbol [" +
                                        desc.identifier + "] does not have "
                                        "its section defined.");
                        }
                        
                        data = gen::pun_s_to_u<short>( short(ste.value - secval - 4) );
                        
                        eh.relocations[sec].emplace_back( RelocType::PCRel_16,
                                                          off, secord );
                        
                    }
                        
                }
                    break;
                
                case Operand::RegDirect:
                    throw std::logic_error("Trying to extend RegDir.");
                    break;
                
            }
            
            // Append data:
            eh.sections[sec].addWord(data);
            
            SKIP_DATA: { }
            
        }
        
    }
    
}

#undef UINT
#undef USHORT
#undef UCHAR