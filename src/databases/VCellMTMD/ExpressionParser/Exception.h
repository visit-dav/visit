/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef EXCEPTION_H    
#define EXCEPTION_H    
    
#include <string>    
    
namespace VCell {    
    class Exception    
    {    
    public:    
        Exception();    
        Exception(std::string message);    
        Exception(std::string title, std::string message);    
        virtual std::string getExactMessage();    
        std::string getMessage(void);    
        ~Exception(void);    
        static void rethrowException(Exception& ex, std::string replacementMessage="");    
        void replaceMessage(std::string& replacementMessage);    
        static std::string add_escapes(std::string str);    
    //    static std::string add_escape(char ch);    
    
    protected:    
        std::string message;        
        std::string title;        
    };    
}    
    
#endif    
