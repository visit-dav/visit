/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef SIMPLECHARSTREAM_H    
#define SIMPLECHARSTREAM_H    
    
#include <iostream>    
#include <string>    
    
class SimpleCharStream    
{    
protected:    
    int bufsize;    
    int available;    
    int tokenBegin;    
    
private:    
    int* bufline;    
    int* bufcolumn;    
    int column;    
    int line;    
    bool prevCharIsCR;    
    bool prevCharIsLF;    
    std::istream* inputStream;    
    char* buffer;    
    int maxNextCharInd;    
    int inBuf;    
    int tabSize;    
    void ExpandBuff(bool wrapAround);    
    void FillBuff(void);    
    void UpdateLineColumn(char c);    
    void init(std::istream* dstream, int startline,  int startcolumn, int buffersize);    
    
public:    
    SimpleCharStream(std::istream* dstream, int startline,  int startcolumn, int buffersize);    
    SimpleCharStream(std::istream* dstream, int startline,  int startcolumn);    
    SimpleCharStream(std::istream* dstream);    
    ~SimpleCharStream(void);    
    
    static bool staticFlag;    
    int bufpos;    
    
    char BeginToken(void);    
    char readChar(void);    
    int getColumn(void);    
    int getLine(void);    
    int getEndColumn(void);    
    int getEndLine(void);    
    int getBeginColumn(void);    
    int getBeginLine(void);    
    void backup(int amount);    
    std::string GetImage(void);    
    char* GetSuffix(int len);    
    void Done(void);    
    void adjustBeginLineColumn(int newLine, int newCol);    
};    
    
#endif    
