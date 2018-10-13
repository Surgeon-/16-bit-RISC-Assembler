/* 
 * File:   Assembler.hpp
 * Author: etf
 *
 * Created on May 20, 2018, 11:45 AM
 */

#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include "Asem-Enumeration.hpp"

#include <string>
#include <vector>
#include <list>
#include <unordered_map>

namespace asem {
    
    struct OperandDesc {
        
        Operand::Enum type;
        std::string identifier;
        int reg_number;
        int number;
        
    };
    
    ////////////////////////////////////////////////////////////////////////////
    
    struct LineOfCode {
        
        int line_ord;
        
        Command::Enum command;
        Predicate::Enum predicate;
        
        std::vector<std::string> labels;
        std::vector<std::string> arguments;
        std::vector<OperandDesc> operands;
        
        LineOfCode()
            : labels()
            , arguments() {
        
            labels.reserve(1);
            arguments.reserve(3);
            
            command = Command::Unknown;
            predicate = Predicate::Al;
            
            line_ord = -1;
            
        }
        
        bool empty() const;
        
        void verify() const;
        
    };
    
    ////////////////////////////////////////////////////////////////////////////
    
    struct ExpressionValue {
        
        int value;
        Section::Enum relative_to;
        bool constant;
        
        ExpressionValue
            ( int value = 0
            , Section::Enum relative_to = Section::Undefined
            , bool is_constant = false
            )
            : value(value)
            , relative_to(relative_to)
            , constant(is_constant)
            { }
        
    };
    
    struct SymbolTableEntry {
        
        //std::string name;
        size_t ordinal;
        Scope::Enum scope;
        Section::Enum section;
        Section::Enum relative_to;
        int value;
        bool defined;
        
        SymbolTableEntry
            //( const std::string & name = "UND"
            ( size_t ordinal = 0
            , Scope::Enum scope = Scope::Undefined
            , Section::Enum section = Section::Undefined
            , Section::Enum relative_to = Section::Undefined
            , int value = 0
            , bool defined = false
            )
            //: name(name)
            : ordinal(ordinal)
            , scope(scope)
            , section(section)
            , relative_to(relative_to)
            , value(value)
            , defined(defined)
            { }
        
    };
    
    struct SymbolTable {
        
        static const int NOT_PRESENT = 0;
        static const int UNDEFINED   = 1;
        static const int DEFINED     = 2;
        
        std::unordered_map<std::string, SymbolTableEntry> data;
        size_t counter[4];
        Section::Enum curr_section;
        
        SymbolTable();
        
        void reset();
        
        int check(const std::string & symbol_name) const;
        
        void toOrderedVector(std::vector<std::pair<std::string,SymbolTableEntry>> & vec) const;
        
        ExpressionValue eval(size_t line_ord, std::string expr) const;
        
        void verify() const;
        
        std::string toString() const;
        
    };
    
    ////////////////////////////////////////////////////////////////////////////
    
    class AssemblyList {
        
    public:
        
        std::list<LineOfCode> data;
        
        SymbolTable symtab;
        
        void loadFromFile(const std::string & path);
        
        void generateSymbolTable();
        
        void parseArguments();
        
        void verifySymbolTable() const;
        
        void verifyAndTidy();
       
    };
    
    ////////////////////////////////////////////////////////////////////////////

    void AsmLoadFile(const std::string & path, std::list<LineOfCode> & list);
    
}

#endif /* ASSEMBLER_HPP */

