
#include "Assembler.hpp"
#include "Asem-Except.hpp"

#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include "StringUtil.hpp"
#include "Asem-Global.hpp"
#include "Asem-Func.hpp"
#include "Asem-FuncST.hpp"

#include <iostream> // TEMP

using namespace gen;

namespace asem {
    
bool LineOfCode::empty() const {

    return (command == Command::Unknown && labels.empty() && arguments.empty());

}

void LineOfCode::verify() const {
    
    for (const std::string & l : labels) {
        
        if ( !(string_is_identifier(l) && !char_is_digit(l[0])) )
            throw SyntaxError(line_ord, "Label [" + l + "] is not an identifier.");
        
    }
    
    // Commands are verified in file-reading phase
    
    // Doesn't verify arguments because they are command-specific
      
}
    
////////////////////////////////////////////////////////////////////////////////

SymbolTable::SymbolTable() {

    reset();
    
    }

void SymbolTable::reset() {
    
    data.clear();
    counter[0] = counter[1] = counter[2] = counter[3] = size_t(0);
    curr_section = Section::Undefined;
    
}

int SymbolTable::check(const std::string & symbol_name) const {
    
    auto iter = data.find(symbol_name);
    
    if (iter == data.end()) return NOT_PRESENT;
    
    if ((*iter).second.defined == false) return UNDEFINED;
    
    return DEFINED;
    
}

void SymbolTable::toOrderedVector(std::vector<std::pair<std::string,SymbolTableEntry>> & vec) const {
    
    vec.resize(data.size());
    
    for (auto pair : data) {
        
        size_t index = pair.second.ordinal;
        
        vec[index].first = pair.first;
        
        vec[index].second = pair.second;
        
    }
    
}

ExpressionValue SymbolTable::eval(size_t line_ord, std::string expr) const {
    
    if (expr.empty()) throw std::logic_error("asem::AssemblyList::eval(...) - "
                                             "Expression is an empty string.");
    
    size_t first = expr.find_first_not_of(" ");
    if (first != std::string::npos && expr[first] == '-')
        expr = "0" + expr;
    
    int offsets[4] = { 0, 0, 0, 0};
    int sign = 1;
    
    std::vector<std::string> tokens;
    std::vector<int> numbers;
    
    string_tokenize_vec_multi(expr, "+-", tokens, true);
    
    numbers.resize( tokens.size() );
    
    size_t cnt = 0;
    for (std::string & s : tokens) {
        
        s = string_crop(s);
        
        if (cnt % 2 == 0) { // Even: Should be number
            
            int i;
            bool is_num = false;
            
            if (string_is_integer(s)) {
            
                try {
                    i = std::stoi(s, 0, 0);
                    is_num = true;
                } catch (...) {
                    is_num = false;
                }
                
            }
            
            if (!is_num) {
                
                if  (!(string_is_identifier(s) && !char_is_digit(s[0])))
                    throw SyntaxError(line_ord, "Cannot evaluate token [" + s + "] in expression (not an identifier).");
                
                if (check(s) != DEFINED)
                    throw SyntaxError(line_ord, "Cannot evaluate token [" + s + "] in expression (undefined).");
                  
                i = data.at(s).value;
                
                offsets[ data.at(s).section ] += sign;
                
            }
            
            numbers[cnt] = i;
            
            //std::cout << "Eval: numbers[" << cnt << "] = " << i << "\n";
            
        } else { // Odd: Should be operator
            
            if (s != "+" && s != "-")
                throw SyntaxError(line_ord, "Operator expected, [" + s + "] found.");
            
            switch (s[0]) {
                
                case '+':
                    sign = 1;
                    break;                    
                    
                case '-':
                    sign = -1;
                    break;
                
            }
            
            //std::cout << "Eval: op[" << cnt << "] = " << s << "\n";
            
        }
        
        cnt += 1;
        
    }
    
    // Get value:
    int res = numbers[0];
    
    for (size_t i = 2; i < tokens.size(); i += 2) {
        
        int mul;
        
        if (tokens[i - 1] == "+")
            mul = 1;
        else
            mul = -1;
        
        res += numbers[i] * mul;
        
    }
    
    // Get relative_to:
    Section::Enum relative_to = Section::Undefined;
    
    if (!offsets[Section::Text] && !offsets[Section::Data] && // No relation
        !offsets[Section::BSS]  && !offsets[Section::ROData]) {
        
        return ExpressionValue(res, Section::Undefined, true);
        
    }
    
    if ( offsets[Section::Text] && !offsets[Section::Data] && // Text
        !offsets[Section::BSS]  && !offsets[Section::ROData]) {
        
        if (offsets[Section::Text] != 1) goto RELATIVE_TO_ERROR;
        
        return ExpressionValue(res, Section::Text);
        
    }
    
    if (!offsets[Section::Text] &&  offsets[Section::Data] && // Data
        !offsets[Section::BSS]  && !offsets[Section::ROData]) {
        
        if (offsets[Section::Data] != 1) goto RELATIVE_TO_ERROR;
        
        return ExpressionValue(res, Section::Data);
        
    }
    
    if (!offsets[Section::Text] && !offsets[Section::Data] && // BSS
         offsets[Section::BSS]  && !offsets[Section::ROData]) {
        
        if (offsets[Section::BSS] != 1) goto RELATIVE_TO_ERROR;
        
        return ExpressionValue(res, Section::BSS);
        
    }
    
    if (!offsets[Section::Text] && !offsets[Section::Data] && // ROData
        !offsets[Section::BSS]  &&  offsets[Section::ROData]) {
        
        if (offsets[Section::ROData] != 1) goto RELATIVE_TO_ERROR;
        
        return ExpressionValue(res, Section::ROData);
        
    }
    
    RELATIVE_TO_ERROR:
    
    throw SyntaxError(line_ord, "Expression value must have a positive offset "
                                "to at most one section.");
    
}

void SymbolTable::verify() const {
    
    for (const std::pair<std::string, SymbolTableEntry> & pair : data) {
        
        if (pair.second.scope != Scope::Global && pair.second.defined == false) {
            
            if (pair.second.ordinal == 0) continue; // NULL Symbol
            
            throw SyntaxError(0, "\b\b\b<unknown>: Symbol [" + pair.first + "] used but not defined or imported.");
            
        }
        
    }
    
}

std::string SymbolTable::toString() const {
    
    char buffer[200];
    
    std::vector<std::pair<std::string, asem::SymbolTableEntry>> st_vec;
    
    toOrderedVector(st_vec);
    
    std::string rv{"#.symtab\n"};
    
    snprintf(buffer, sizeof(buffer)
            , "#%3s   %-16s   %5s   %-7s   %-6s \n"
            , "Ord"
            , "Symbol name"
            , "Value"
            , "Section"
            , "Scope"
            ) ;
    
    rv += buffer;
    
    rv += "#=================================================\n";
    
    for (auto & pair : st_vec) {
        
        snprintf(buffer, sizeof(buffer)
                , " %3d   %-16s   %5s   %-7s   %-6s \n"
                , pair.second.ordinal
                , pair.first.c_str()
                , pair.second.defined ? (std::to_string(pair.second.value).c_str()) : ("?")
                , SectionToString( pair.second.section )
                , ScopeToString( pair.second.scope )
                ) ;
        
        rv += buffer;
        
    }
    
    rv += "#=================================================";
    
    return rv;
    
}

////////////////////////////////////////////////////////////////////////////////

void AssemblyList::verifyAndTidy() {
    
    for (auto iter = data.begin(); iter != data.end(); ) {
        
        (*iter).verify();
        
        if ( (*iter).command == Command::Unknown && (*iter).arguments.empty() &&
            !(*iter).labels.empty() ) {
            
            if (std::next(iter) != data.end()) {
            
                for (std::string & l : (*iter).labels) {
                    
                    (*std::next(iter)).labels.push_back(l);
                    
                }
                
                iter = data.erase(iter);
                
            } else
                iter = std::next(iter);
            
        } else
            iter = std::next(iter);
        
    }
    
}

void AssemblyList::loadFromFile(const std::string & path) {
    
    AsmLoadFile(path, data);
    
    verifyAndTidy();
    
}

void AssemblyList::parseArguments() {
    
    for (LineOfCode & loc : data) {
        
        int cra = CommandRequiredArgs(loc.command);
        
        int cnt = (int)loc.arguments.size();
        
        if (!CommandIsInstruction(loc.command)) { // DIRECTIVES
            
            switch (cra) {
                
                case REQ_ARGS_ANY:
                    // Do nothing
                    break;
                    
                case REQ_ARGS_1_TO_2: // Skip
                    if (cnt >= 1 && cnt <=2) continue;
                    
                case REQ_ARGS_1_TO_3: // Align
                    if (cnt >= 1 && cnt <=3) continue;
                            
                default:
                    if (cnt == cra) continue;
                
                throw SyntaxError(loc.line_ord, "Incorrect number of arguments (" +
                    std::to_string(cnt) + " provided).");
                
            }
            
            // Continue outer loop
            
        } else { // INSTRUCTIONS
            
            if (cnt != cra)
                throw SyntaxError(loc.line_ord, "Incorrect number of arguments (" +
                    std::to_string(cnt) + " provided).");
            
            loc.operands.resize( loc.arguments.size() );
            
            for (size_t i = 0; i < loc.arguments.size(); i += 1) {
                
                bool error = !IdentifyOperand(loc.arguments[i], loc.operands[i]);
                
                //std::cout << loc.operands[i].type << "\n";
                
                if (error)
                    throw SyntaxError( loc.line_ord, "Error parsing operand [" + 
                                       loc.arguments[i] + "].");
                
            }
            
        }
        
    }
    
}

void AssemblyList::generateSymbolTable() {
    
    SymbolTable & st = symtab;
    
    st.reset();
    
    st.data.emplace("<UNDEFINED>", SymbolTableEntry() ); 
    
    for (const LineOfCode & loc : data) {
    
        //std::cout << "ST - Command = " << loc.command << "\n";
        
        SymTabProcLabels(loc.line_ord, st, loc.labels);
        
        size_t lo = loc.line_ord;
        
        switch (loc.command) {
            
            // Directives:
            case Command::Global: SymTabProcGlobal(st, loc); break;
            case Command::Text:   SymTabProcText(lo, st);    break;
            case Command::Data:   SymTabProcData(lo, st);    break;
            case Command::ROData: SymTabProcROData(lo, st);  break;
            case Command::BSS:    SymTabProcBSS(lo, st);     break;
            case Command::End:    /* Do nothing */          break;
            case Command::Char:   SymTabProcChar(st, loc);  break; 
            case Command::Word:   SymTabProcWord(st, loc);  break; 
            case Command::Long:   SymTabProcLong(st, loc);  break; 
            case Command::Align:  SymTabProcAlign(st, loc); break;
            case Command::Skip:   SymTabProcSkip(st, loc);  break;

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
            case Command::Push:
            case Command::Pop:
            case Command::Call:
            case Command::Iret:
            case Command::Mov:
            case Command::Shl:
            case Command::Shr:
            case Command::RetP: 
            case Command::JmpP:   SymTabProcInstruction(st, loc); break;
            
        }
        
            
    }
    
}

void AssemblyList::verifySymbolTable() const {
    
    symtab.verify();
    
}

////////////////////////////////////////////////////////////////////////////////

void AsmLoadFile(const std::string & path, std::list<LineOfCode> & list) {
    
    #define CURR list.back()

    std::ifstream file{path, std::ifstream::in};

    if (!file.is_open()) throw std::runtime_error("Could not open file for reading [" + path + "].");

    list.clear();
    list.emplace_back();

    size_t line_ord = 1;

    for (std::string line; std::getline(file, line); line_ord += 1) {

        if (!CURR.empty()) list.emplace_back();

        CURR.line_ord = (int)line_ord;

        line = gen::string_crop( gen::string_replace_all(line, "\t", " "), " ");

        // PROCESS LINE ////////////////////////////////////////////////////////
        std::string temp, rest;
        size_t pos;

        // Remove comments:
        pos = line.find(COMMENT_TOKEN);

        if (pos != std::string::npos) {

            line.erase(pos, std::string::npos);

        }

        if (line.empty()) continue;

        // Find label:
        while (true) {

            pos = line.find(':');

            if (pos != std::string::npos) {

                temp = gen::string_crop( std::string(line, 0, pos) );

                line.erase(0, pos + 1);

                CURR.labels.push_back(temp);

            }
            else {

                break;

            }
        }

        if (line.empty()) continue;

        // Find command: 
        line = gen::string_crop(line);

        pos = line.find(' ');

        if (pos != std::string::npos) {

            temp = std::string(line, 0, pos);

            rest = gen::string_crop( line.erase(0, pos) );

        }
        else {

            temp = line;

            rest = "";

        }

        Predicate::Enum pred;
        auto comm = EnumerateCommand(temp, pred);

        if (comm == Command::Unknown)
            throw SyntaxError(line_ord, "Unknown command " + temp + " encountered.");

        CURR.command = comm;
        CURR.predicate = pred;

        gen::string_tokenize_vec(rest, ',', CURR.arguments);

        for (std::string & arg : CURR.arguments) {
            arg = gen::string_crop(arg, " ");
        }

        if (temp == ".end") break;

        // END PROCESS LINE ////////////////////////////////////////////////////

    }

    if (list.back().empty()) list.pop_back();

#undef CURR

}

} // End namespace gen