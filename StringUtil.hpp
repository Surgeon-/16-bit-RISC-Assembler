
#ifndef STRINGUTIL_HPP
#define STRINGUTIL_HPP

#include <cstring>
#include <string>
#include <vector>
#include <stdexcept>

namespace gen {
    
    inline
    std::string string_replace_all(const std::string & str, const char * substr,
                                   const char * replacement) {
        
        if(!substr[0]) return str;
        
        std::string rv{str};
        
        size_t start_pos = 0;
        size_t sub_len = std::strlen(substr);
        size_t rep_len = std::strlen(replacement);
        
        while ((start_pos = rv.find(substr, start_pos)) != std::string::npos) {
            
            rv.replace(start_pos, sub_len, replacement);
            
            start_pos += rep_len;
            
        }
        
        return rv;
        
    }
    
    inline 
    bool char_is_digit(const char c) {
        
        if (c >= '0' && c <= '9') return true;
        
        return false;
        
    }
    
    inline
    bool _string_is_integer_underlying(const std::string & str) {
        
        // May not work with strings containing white-spaces
        
        size_t len = str.size();
        
        const char * buf = str.c_str();
        
        bool hex = false;
        
        if (len == 0) return false;
        
        if (len == 1) return char_is_digit(buf[0]);
        
        if (buf[0] == '-' || buf[0] == '+') {
            buf += 1;
            len -= 1;
        }
        
        if (len > 2 && buf[0] == '0' && (buf[1] == 'x' || buf[1] == 'X')) {
            buf += 2;
            len -= 2;
            hex = true;
        }
        
        for (size_t i = 0; i < len; i += 1) {
            
            char c = buf[i];
            
            if (char_is_digit(c) ||
                ( hex && ((c >= 'a' && c <= 'f') || (c >= 'A' && c<= 'F')) )
               ) continue;
            
            return false;
            
        }
        
        return true;
        
    }
    
    inline
    bool string_is_integer(const std::string & str) {
        
        return _string_is_integer_underlying( 
            string_replace_all(str, " ", "") );
        
    }
    
    inline
    bool string_is_identifier(const char * ptr) {
        
        if (*ptr == '\0') return false;
        
        do {
            
            char c = *ptr;
            ptr += 1;
            
            if ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                (c == '_')) continue;
            
            return false;
            
        }
        while (*ptr != '\0');
        
        return true;
        
    }
    
    inline
    bool string_is_identifier(const std::string & str) {
        
        return string_is_identifier(str.c_str());
        
    }
    
    inline
    std::string string_crop(const std::string & str, const std::string & tokens = " \t") {
        
        if (str.empty()) return std::string("");
        
        size_t head = 0;
        size_t tail = str.size() - 1;
        
        while (tokens.find(str[head]) != std::string::npos)
            head += 1;
        
        while (tail >= head && tokens.find(str[tail]) != std::string::npos)
            tail -= 1;
        
        if (tail < head) return std::string("");
        
        return std::string(str.c_str() + head, tail - head + 1);
        
    }
    
    inline 
    void string_tokenize_vec(const std::string & str, char separator, std::vector<std::string> & dst) {
        
        if (str.empty()) return;
        
        size_t head = 0;
        
        while (true) {
            
            size_t pos = str.find(separator, head);
            
            if (pos != std::string::npos) {
               
                if (pos - head > 0)
                    dst.emplace_back(str, head, pos - head);
                else if (pos == head)
                    dst.emplace_back("");
                
                head = pos + 1;
                
            }
            else {
                
                dst.emplace_back(str, head, std::string::npos);
                
                break;
                
            }
            
        }
        
    }
    
    inline 
    void string_tokenize_vec_multi(const std::string & str, const char * separators, std::vector<std::string> & dst, bool incl_sep = false) {
        
        if (str.empty()) return;
        
        size_t tail = 0;
        size_t head = 0;
        
        while (true) {
            
            bool in = false;
            
            if (strchr(separators, str[head]) != 0) { // Found separator
                
                if (tail < head)
                    dst.emplace_back(str, tail, head - tail);
                else if (tail == head)
                    dst.emplace_back("");
                
                if (incl_sep)
                    dst.push_back( std::string(1u, str[head]) );
                
                tail = head + 1;
                head = tail;
                
                in = true;
                
            } else {
                
                head += 1;
                
            }
            
            if (head > str.size() || str[head] == '\0') {
                
                if (!in)
                    dst.emplace_back(str, tail, std::string::npos);
                
                break;
                
            }
            
        }
        
    }
    
    inline
    std::string buffer_to_hex(const void * buffer, size_t length, char separator = '\0') {
        
        if (length == 0) return "";
        
        std::string rv;
        
        if (separator == '\0')
            rv.resize(length * 2u);
        else
            rv.resize(length * 3u);
        
        // ***
        
        char * dst = &rv[0];
        
        for (size_t i = 0; i < length; i += 1) {
            
            int val = static_cast<int>( *((const unsigned char*)buffer + i) );
            
            snprintf(dst, 3u, "%02X", val);
            
            dst += 2;
            
            if (separator != '\0') {
                *dst = separator;
                dst += 1;
            }
            
        }
        
        // ***
        
        if (separator != '\0') rv.pop_back();
        
        return rv;
        
    }
    
    inline
    std::string string_replace_all_identifiers(const std::string & str,
                                               const char * substr,
                                               const char * replacement) {
        
        if ( ! (string_is_identifier(substr) && !char_is_digit(substr[0])))
            throw std::logic_error("gen::string_replace_all_identifiers(...) - "
                                   "substring is not an identifier.");
        
        if(!substr[0]) return str;
        
        std::string rv{str};
        
        size_t start_pos = 0;
        size_t sub_len = std::strlen(substr);
        size_t rep_len = std::strlen(replacement);
        
        while (start_pos < rv.size() &&
               ((start_pos = rv.find(substr, start_pos)) != std::string::npos)) {
            
            char buffer[2] = { '\0', '\0'};
            
            if (start_pos + sub_len < rv.size()) {
                
                buffer[0] = rv[start_pos + sub_len];
                
                if (string_is_identifier(buffer)) {
                    start_pos += sub_len;
                    continue;
                }
                
            }
            
            if (start_pos > 0) {
                
                buffer[0] = rv[start_pos - 1];
                
                if (string_is_identifier(buffer)) {
                    start_pos += sub_len;
                    continue;
                }
            }
            
            rv.replace(start_pos, sub_len, replacement);
            
            start_pos += rep_len;
            
        }
        
        return rv;
        
    }
    
}



#endif /* STRINGUTIL_HPP */

