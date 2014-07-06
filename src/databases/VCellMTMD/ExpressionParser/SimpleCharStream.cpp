/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#include <memory.h>    
#include <stdlib.h>    
    
#include "SimpleCharStream.h"    
#include "IOException.h"    
#include "RuntimeException.h"    
    
using namespace VCell;
using std::istream;

bool SimpleCharStream::staticFlag = false;    
    
SimpleCharStream::SimpleCharStream(istream* dstream, int startline,  int startcolumn, int buffersize)    
{    
    init(dstream, startline, startcolumn, buffersize);    
}    
void SimpleCharStream::init(istream* dstream, int startline,  int startcolumn, int buffersize) {            
    tokenBegin = 0;    
    bufpos = -1;    
    prevCharIsCR = false;    
    prevCharIsLF = false;        
    maxNextCharInd = 0;    
    inBuf = 0;    
    tabSize = 8;    
    
    inputStream = dstream;    
    line = startline;    
    column = startcolumn - 1;    
    
    available = bufsize = buffersize;    
    buffer = new char[buffersize];    
    memset(buffer, 0, buffersize * sizeof(char));    
    
    bufline = new int[buffersize];    
    memset(bufline, 0, buffersize * sizeof(int));    
    bufcolumn = new int[buffersize];    
    memset(bufcolumn, 0, buffersize * sizeof(int));    
}    
    
SimpleCharStream::SimpleCharStream(istream* dstream, int startline,  int startcolumn)    
{    
    init(dstream, startline, startcolumn, 4096);    
}    
    
SimpleCharStream::SimpleCharStream(istream* dstream)    
{    
    init(dstream, 1, 1, 4096);    
}    
    
SimpleCharStream::~SimpleCharStream(void)    
{    
    delete[] buffer;    
    delete[] bufline;    
    delete[] bufcolumn;    
}    
    
void SimpleCharStream::ExpandBuff(bool wrapAround)    
{    
    char* newbuffer = new char[bufsize + 2048];    
    memset(newbuffer, 0, (bufsize + 2048) * sizeof(char));    
    int* newbufline = new int[bufsize + 2048];    
    memset(newbufline, 0, (bufsize + 2048) * sizeof(int));    
    int* newbufcolumn = new int[bufsize + 2048];    
    memset(newbufcolumn, 0, (bufsize + 2048) * sizeof(int));    
    
    try    
    {    
        if (wrapAround)    
        {    
            memcpy(newbuffer, buffer + tokenBegin, (bufsize - tokenBegin) * sizeof(char));    
            memcpy(newbuffer + bufsize - tokenBegin, buffer, bufpos * sizeof(char));    
            delete[] buffer;    
            buffer = newbuffer;    
    
            memcpy(newbufline, bufline + tokenBegin,(bufsize - tokenBegin) * sizeof(int));    
            memcpy(newbufline + bufsize - tokenBegin, bufline, bufpos * sizeof(int));    
            delete[] bufline;    
            bufline = newbufline;    
    
            memcpy(newbufcolumn, bufcolumn + tokenBegin, (bufsize - tokenBegin) * sizeof(int));    
            memcpy(newbufcolumn + bufsize - tokenBegin, bufcolumn, bufpos * sizeof(int));    
            delete[] bufcolumn;    
            bufcolumn = newbufcolumn;    
    
            maxNextCharInd = (bufpos += (bufsize - tokenBegin));    
        } else {    
            memcpy(newbuffer, buffer + tokenBegin, (bufsize - tokenBegin) * sizeof(char));                
            delete[] buffer;    
            buffer = newbuffer;    
    
            memcpy(newbufline, bufline + tokenBegin,(bufsize - tokenBegin) * sizeof(int));    
            delete[] bufline;    
            bufline = newbufline;    
    
            memcpy(newbufcolumn, bufcolumn + tokenBegin, (bufsize - tokenBegin) * sizeof(int));    
            delete[] bufcolumn;    
            bufcolumn = newbufcolumn;    
    
            maxNextCharInd = (bufpos -= tokenBegin);    
        }    
    } catch (char* ex) {    
        throw RuntimeException(ex);    
    } catch (...) {    
        throw RuntimeException("Unknown exception in SimpleCharStream::ExpandBuff");    
    }    
    
    bufsize += 2048;    
    available = bufsize;    
    tokenBegin = 0;    
}    
    
void SimpleCharStream::FillBuff(void)    
{    
    if (maxNextCharInd == available)    
    {    
        if (available == bufsize)    
        {    
            if (tokenBegin > 2048)    
            {    
                bufpos = maxNextCharInd = 0;    
                available = tokenBegin;    
            } else if (tokenBegin < 0)    
                bufpos = maxNextCharInd = 0;    
            else    
                ExpandBuff(false);    
        } else if (available > tokenBegin)    
            available = bufsize;    
        else if ((tokenBegin - available) < 2048)    
            ExpandBuff(true);    
        else    
            available = tokenBegin;    
    }    
    
    int i;    
    try {    
        inputStream->read(buffer + maxNextCharInd, available - maxNextCharInd);            
        if (!inputStream->eof() && (inputStream->fail() || inputStream->bad())) {    
            throw IOException("");    
        } else {    
            i = inputStream->gcount();    
            maxNextCharInd += i;    
        }    
        return;    
    } catch(Exception& ex) {    
        --bufpos;    
        backup(0);    
        if (tokenBegin == -1)    
            tokenBegin = bufpos;    
        Exception::rethrowException(ex);    
    }    
}    
    
char SimpleCharStream::BeginToken(void)    
{    
    tokenBegin = -1;    
    char c = readChar();    
    tokenBegin = bufpos;    
    
    return c;    
}    
    
void SimpleCharStream::UpdateLineColumn(char c)    
{    
     column++;    
    
     if (prevCharIsLF)    
     {    
        prevCharIsLF = false;    
        line += (column = 1);    
     }    
     else if (prevCharIsCR)    
     {    
        prevCharIsCR = false;    
        if (c == '\n')    
        {    
           prevCharIsLF = true;    
        }    
        else    
           line += (column = 1);    
     }    
    
     switch (c)    
     {    
        case '\r' :    
           prevCharIsCR = true;    
           break;    
        case '\n' :    
           prevCharIsLF = true;    
           break;    
        case '\t' :    
           column--;    
           column += (tabSize - (column % tabSize));    
           break;    
        default :    
           break;    
     }    
    
     bufline[bufpos] = line;    
     bufcolumn[bufpos] = column;    
}    
    
char SimpleCharStream::readChar(void)    
{    
     if (inBuf > 0)    
     {    
        --inBuf;    
    
        if (++bufpos == bufsize)    
           bufpos = 0;    
    
        return buffer[bufpos];    
     }    
    
     if (++bufpos >= maxNextCharInd)    
        FillBuff();    
    
     char c = buffer[bufpos];    
    
     UpdateLineColumn(c);    
     return (c);    
}    
    
int SimpleCharStream::getColumn(void)    
{    
     return bufcolumn[bufpos];    
}    
    
int SimpleCharStream::getLine(void)    
{    
    return bufline[bufpos];    
}    
    
int SimpleCharStream::getEndColumn(void)    
{    
     return bufcolumn[bufpos];    
}    
    
int SimpleCharStream::getEndLine(void)    
{    
     return bufline[bufpos];    
}    
    
int SimpleCharStream::getBeginColumn(void)    
{    
     return bufcolumn[tokenBegin];    
}    
    
int SimpleCharStream::getBeginLine(void)    
{    
     return bufline[tokenBegin];    
}    
    
void SimpleCharStream::backup(int amount)    
{    
    inBuf += amount;    
    if ((bufpos -= amount) < 0)    
       bufpos += bufsize;    
}    
    
std::string SimpleCharStream::GetImage(void)    
{    
    if (bufpos >= tokenBegin)    
        return std::string(buffer, tokenBegin, bufpos - tokenBegin + 1);    
    else    
        return std::string(buffer, tokenBegin, bufsize - tokenBegin) + std::string(buffer, 0, bufpos + 1);    
}    
    
char* SimpleCharStream::GetSuffix(int len)    
{    
    char* ret = new char[len + 1];    
    memset(ret, 0, (len + 1) * sizeof(char));    
    
    if ((bufpos + 1) >= len)    
        memcpy(ret, buffer + bufpos - len + 1, len * sizeof(char));    
    else    
    {    
        memcpy(ret, buffer + bufsize - (len - bufpos - 1), (len - bufpos - 1) * sizeof(char));                
        memcpy(ret + len - bufpos - 1, buffer, (bufpos + 1) * sizeof(char));    
    
    }    
    
    return ret;    
}    
    
void SimpleCharStream::Done(void)    
{    
     buffer = 0;    
     bufline = 0;    
     bufcolumn = 0;    
}    
    
void SimpleCharStream::adjustBeginLineColumn(int newLine, int newCol)    
{    
     int start = tokenBegin;    
     int len;    
    
     if (bufpos >= tokenBegin)    
     {    
        len = bufpos - tokenBegin + inBuf + 1;    
     }    
     else    
     {    
        len = bufsize - tokenBegin + bufpos + 1 + inBuf;    
     }    
    
     int i = 0, j = 0, k = 0;    
     int nextColDiff = 0, columnDiff = 0;    
    
    while (i < len &&    
        //bufline[j = start % bufsize] == bufline[k = ++start % bufsize])    
        bufline[j = start % bufsize] == bufline[k = (start+1) % bufsize]) ///TODO: check on fix for undefined operation warning
    {    
        bufline[j] = newLine;    
        nextColDiff = columnDiff + bufcolumn[k] - bufcolumn[j];    
        bufcolumn[j] = newCol + columnDiff;    
        columnDiff = nextColDiff;    
        i++;++start;
    }     
    
    if (i < len)    
        {    
        bufline[j] = newLine++;    
        bufcolumn[j] = newCol + columnDiff;    
    
        while (i++ < len)    
        {    
            //if (bufline[j = start % bufsize] != bufline[++start % bufsize])    
            if (bufline[j = start % bufsize] != bufline[(start+1) % bufsize])///TODO: check on fix for undefined operation on start warning
                bufline[j] = newLine++;    
            else    
                bufline[j] = newLine;    
            ++start;
        }    
    }    
    
     line = bufline[j];    
     column = bufcolumn[j];    
}    
