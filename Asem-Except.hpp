/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Asem-Except.hpp
 * Author: etf
 *
 * Created on June 3, 2018, 10:08 AM
 */

#ifndef ASEM_EXCEPT_HPP
#define ASEM_EXCEPT_HPP

#include <exception>
#include <string>

namespace asem {

class SyntaxError : public std::exception {
        
    std::string message;
    size_t line_ord;

public:

    SyntaxError(size_t line_ord, const std::string& msg) {
        message = std::string("Syntax error near line ") +
                  std::to_string(line_ord) +
                  ": " + msg;
    }

    SyntaxError(size_t line_ord, const char * msg) {
        message = std::string("Syntax error near line ") +
                  std::to_string(line_ord) +
                  ": " + msg;
    }

    const char* what() const noexcept {
        return message.c_str();         
    }

    SyntaxError(const SyntaxError & other) {
        message = other.message;
    }

    SyntaxError & operator=(const SyntaxError & other) noexcept {
        message = other.message;
    }

};

}

#endif /* ASEM_EXCEPT_HPP */

