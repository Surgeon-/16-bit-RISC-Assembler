
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <list>

#include "StringUtil.hpp"
#include "Assembler.hpp"
#include "Asem-Func.hpp"
#include "Punning.hpp"
#include "Asem-ELFHolder.hpp"
#include "Asem-Global.hpp"
#include "Asem-Except.hpp"

/*
 * Calling convention:
 *   asm87 path_in path_out start_addr [optional flags...]
 * 
 * Optional flags:
 *   log  - log compilation to console
 * 
 * path_out == stdout -> print to console
 * 
 */

const asem::Section::Enum SECTIONS[4] = 
    { asem::Section::Text
    , asem::Section::Data
    , asem::Section::BSS
    , asem::Section::ROData
    } ;

void DisplayHelp() {
    
    std::cout << "\n";
    std::cout << "The calling conventions for this program are:\n";
    std::cout << "[1]  asm87 \"path_in\" \"path_out\" start_addr [optional flags...]\n";
    std::cout << "   Where optional flags may be (in any order):\n";
    std::cout << "     log - log operations in console.\n";
    std::cout << "[2]  asm87 info\n";
    std::cout << "The first option compiles files, the second one displays program info.\n";
    std::cout << "\n";
    
}

void DisplayInfo() {
    
    std::cout << "\n";
    std::cout << " < Elektrotehnicki fakultet Univerziteta u Beogradu >\n";
    std::cout << " - Projektni zadatak iz predmeta Sistemski Softver  -\n";
    std::cout << " - Asembler za hipoteticki 16-bitni procesor        -\n";
    std::cout << " - Student: Jovan Batnozic 0087/2015                -\n";
    std::cout << " -   Datum: Jun 2018. godine                        -\n";
    
    DisplayHelp();
    
}

std::string SymTabToString(const asem::AssemblyList & al) {
    
    return (al.symtab.toString() + "\n\n");
    
}

std::string RelocRecordsToString(const asem::ELFHolder & eh) {
    
    std::string rv;
    
    for (size_t i = 0; i < 4; i += 1) {
        
        std::string temp = eh.rrToString(SECTIONS[i]);
        
        if (temp.empty()) continue;
        
        rv += temp;
        rv += "\n\n";
         
    }
    
    return rv;
    
}

std::string SectionsToString(const asem::ELFHolder & eh) {
    
    std::string rv;
    
    for (size_t i = 0; i < 4; i += 1) {
        
        std::string temp = eh.secToString(SECTIONS[i]);
        
        if (temp.empty()) continue;
        
        rv += temp;
        rv += "\n\n";
         
    }
    
    return rv;
    
}

#define LOG(text) do{ if (flag_log) std::cout << text; }while(0)

int main(int argc, char** argv) {
   
    bool flag_log  = false;
    
    const char * path_in  = nullptr;
    const char * path_out = nullptr;
    
    int start_addr = 0;
    
    //const char * path = "/home/etf/Desktop/asmtest.txt";
    
    // Main arguments:
    if (argc < 4) {
        
        if (argc == 2 && strcmp(argv[1], "info") == 0) {
            DisplayInfo();
            return 0;
        }
        else {
            std::cout << "Too few arguments.\n";
            DisplayHelp();
            return 1;
        }
        
    }
    
    // Optional flags:
    for (int i = 4; i < argc; i += 1) {
        
        if (strcmp(argv[i], "log") == 0) {
            flag_log = true;
            continue;
        }
        
        if (strcmp(argv[i], "info") == 0) {
            std::cout << "Flag [info] is ignored unless it's the first and only argument.";
            continue;
        }
        
        std::cout << "Unknown flag [" << argv[i] << "]\n.";
        
        return 1;
        
    }
    
    // Start address:
    if (gen::string_is_integer(argv[3])) {
        start_addr = std::stoi(argv[3]);
        if (start_addr < 0 || start_addr >= (1 << 16 - 1)) {
            std::cout << "Starting address (" << start_addr << ") too large or too small.\n."; 
            return 1;
        } 
    }
    else {
       std::cout << "Third argument [" << argv[3] << "] must be an integer.\n."; 
       return 1;
    }
    
    // Everything OK, setup:
    asem::DO_CHAIN_SECTIONS = true;   
    asem::START_LOCATION    = size_t(start_addr);
    
    path_in  = argv[1];
    path_out = argv[2];
    
    ////////////////////////////////////////////////////////////////////////////
    
    asem::AssemblyList al{};
    
    asem::ELFHolder eh{};
    
    try {
    
        // PASS 1:
        
        LOG( "Loading AssemblyList from file... " );
        al.loadFromFile(path_in);
        LOG( "OK\n" );
        
        LOG( "Parsing instruction/directive arguments... " );
        al.parseArguments();
        LOG( "OK\n" );
        
        LOG( "Generating symbol table... " );
        al.generateSymbolTable();
        LOG( "OK\n" );
        
        LOG( "Verifying symbol table... " );
        al.verifySymbolTable();
        LOG( "OK\n" );
        
        // PASS 2:
        LOG( "Generating ELFHolder from AssemblyList... " );
        eh.loadFromAssemblyList(al);
        eh.clearBSS();
        LOG( "OK\n" );
    
    }
    catch (asem::SyntaxError & ex) {
        
        std::cout << "\nSyntax Error: " << ex.what() << std::endl;
        
        return 1;
        
    }
    catch (std::exception & ex) {
        
        std::cout << "\nException caught: " << ex.what() << std::endl;
        
        return 1;
        
    }
    catch (...) {
        
       std::cout << "\nUnknown exception caught (no message provided).\n";
        
       return 1; 
        
    }
    
    // WRITE OUTPUT: ///////////////////////////////////////////////////////////
    
    if (strcmp(path_out, "stdout") == 0) {
        
        std::cout << SymTabToString(al);
        std::cout << RelocRecordsToString(eh);
        std::cout << SectionsToString(eh); 
        
    }
    else {
        
        std::ofstream outfile;
        
        try {
            
            LOG( "Opening file for writing... " );
            outfile.open(path_out, std::ios_base::out | std::ios_base::trunc);
            LOG( "OK\n" );
            
            if (!outfile.is_open()) {
                std::cout << "Could not open file [" << path_out << "] for writing.\n";
                return 1;
            }
            
            LOG( "Writing to file... " );
            outfile << SymTabToString(al);
            outfile << RelocRecordsToString(eh);
            outfile << SectionsToString(eh); 
            LOG( "OK\n" );
            
            LOG( "Flushing and closing file... " );
            outfile.flush();
            outfile.close();
            LOG( "OK\n" );
            
        }
        catch (std::exception & ex) {
            
            std::cout << "\nException caught: " << ex.what() << std::endl;
        
            return 1;
            
        }
        catch (...) {
            
            std::cout << "\nUnknown exception caught (no message provided).\n";
        
            return 1;
            
        }
        
    }
    
    LOG( "All operations completed successfully.\n" ); 
    
    return 0;
    
}

#undef LOG
