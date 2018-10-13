
#ifndef ASM_ENUMERATION_HPP
#define ASM_ENUMERATION_HPP

namespace asem {
        
struct Scope {

    enum Enum {

        Undefined,
        Global,
        Local

    };

};
    
struct Section {

    enum Enum {

        Undefined = -1,
        Text = 0,
        Data = 1,
        ROData = 2,
        BSS = 3

    };

};

struct Command {
    
    enum Enum {
        
        Unknown,
        
        // Directives:
        Global,
        Text,
        Data,
        ROData,
        BSS,
        End,
        Char,
        Word,
        Long,
        Align,
        Skip,
        
        // Instructions:
        Add,
        Sub,
        Mul,
        Div,
        Cmp,
        And,
        Or,
        Not,
        Test,
        Push,
        Pop,
        Call,
        Iret,
        Mov,
        Shl,
        Shr,
        
        RetP,
        JmpP,
        
        // Count:
        Count
        
    };
    
};

struct Predicate {
    
    enum Enum {
        
        Eq = 0,
        Ne = 1,
        Gt = 2,
        Al = 3
        
    };
    
};

struct AddrMode {
    
    enum Enum {
        
        Imm    = 0,
        RegDir = 1,
        MemDir = 2,
        RegInd = 3
        
    };
    
};

struct Operand {
    
    enum Enum {
        
        Literal      = 0, // 20
        SymbolValue  = 1, // &x
        SymbolDeref  = 2, //  x
        LiteralIndir = 3, // *x
        RegDirect    = 4, // r5
        RegIndir     = 5, // r[20]
        RegIndirExp  = 6, // r[x]
        PCRel        = 7, // $x
        PSW          = 8, // psw
        
    };
    
};

struct RelocType {
    
    enum Enum {
        
        Undefined,
        Abs_08,
        Abs_16,
        Abs_32,
        PCRel_16
        
    };
    
};
    
}

#endif /* ASMENUMERATION_HPP */

