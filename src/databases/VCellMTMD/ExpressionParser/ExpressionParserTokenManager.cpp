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
#include <stdio.h>
#include <string>
    
#include "ExpressionParserTokenManager.h"    
#include "RuntimeException.h"    
#include "IOException.h"    
    
using namespace VCell;
using std::string;
using std::ostream;

int ExpressionParserTokenManager::jjnextStates[] = {35, 36, 41, 42, 31, 32, 31, 32, 33, 22, 23, 39, 40, 43, 44, };    
    
string* ExpressionParserTokenManager::jjstrLiteralImages[] = {    
    new string(""), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,     
    NULL, NULL, new string("\136"), new string("\53"), new string("\55"), new string("\52"), new string("\57"),     
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, new string("\73"), new string("\50"), new string("\51"), new string("\54"),     
};    
    
string ExpressionParserTokenManager::lexStateNames[] = {    
   "DEFAULT",     
};    
    
int64 ExpressionParserTokenManager::jjtoToken[] = { 0x789ff021L, };    
    
int64 ExpressionParserTokenManager::jjtoSkip[] = {  0x1eL, };    
    
void ExpressionParserTokenManager::setDebugStream(ostream* os)    
{    
    debugStream = os;    
}    
    
ExpressionParserTokenManager::ExpressionParserTokenManager(SimpleCharStream* stream)    
{    
    init(stream);    
}    
    
    
ExpressionParserTokenManager::ExpressionParserTokenManager(SimpleCharStream* stream, int lexState)    
{    
    init(stream);    
    SwitchTo(lexState);    
}    
    
void ExpressionParserTokenManager::init(SimpleCharStream* stream)    
{    
    if (SimpleCharStream::staticFlag)    
        throw RuntimeException("ERROR: Cannot use a static CharStream class with a non-static lexical analyzer.");    
    jjnewStateCnt = 0;    
    jjround = 0;    
    jjmatchedKind = 0;    
    jjmatchedPos = 0;    
    curLexState = 0;    
    defaultLexState = 0;    
    jjrounds = new int[45];    
    memset(jjrounds, 0, 45 * sizeof(int));    
    jjstateSet = new int[90];    
    memset(jjstateSet, 0, 90 * sizeof(int));    
    input_stream = stream;    
    debugStream = 0;    
}    
    
ExpressionParserTokenManager::~ExpressionParserTokenManager(void)    
{    
    delete[] jjrounds;    
    delete[] jjstateSet;    
}    
    
int ExpressionParserTokenManager::jjStopStringLiteralDfa_0(int pos, long active0)    
{    
    switch (pos)    
    {    
        default :    
            return -1;    
    }    
}    
    
int ExpressionParserTokenManager::jjStartNfa_0(int pos, long active0)    
{    
    return jjMoveNfa_0(jjStopStringLiteralDfa_0(pos, active0), pos + 1);    
}    
    
int ExpressionParserTokenManager::jjStopAtPos(int pos, int kind)    
{    
    jjmatchedKind = kind;    
    jjmatchedPos = pos;    
    return pos + 1;    
}    
    
int ExpressionParserTokenManager::jjStartNfaWithStates_0(int pos, int kind, int state)    
{    
  jjmatchedKind = kind;    
   jjmatchedPos = pos;    
   try {     
       curChar = input_stream->readChar();     
   } catch(...) {     
       return pos + 1;     
   }    
   return jjMoveNfa_0(state, pos + 1);    
}    
    
int ExpressionParserTokenManager::jjMoveStringLiteralDfa0_0(void)    
{    
    switch(curChar)    
    {    
        case 40:    
            return jjStopAtPos(0, 28);    
        case 41:    
            return jjStopAtPos(0, 29);    
        case 42:    
            return jjStopAtPos(0, 18);    
        case 43:    
            return jjStopAtPos(0, 16);    
        case 44:    
            return jjStopAtPos(0, 30);    
        case 45:    
            return jjStopAtPos(0, 17);    
        case 47:    
            return jjStopAtPos(0, 19);    
        case 59:    
            return jjStopAtPos(0, 27);    
        case 94:    
            return jjStopAtPos(0, 15);    
        default :    
            return jjMoveNfa_0(0, 0);    
    }    
}    
    
void ExpressionParserTokenManager::jjCheckNAdd(int state)    
{    
   if (jjrounds[state] != jjround)    
   {    
      jjstateSet[jjnewStateCnt++] = state;    
      jjrounds[state] = jjround;    
   }    
}    
    
void ExpressionParserTokenManager::jjAddStates(int start, int end)    
{    
    do {    
        jjstateSet[jjnewStateCnt++] = jjnextStates[start];    
    } while (start++ != end);    
}    
    
void ExpressionParserTokenManager::jjCheckNAddTwoStates(int state1, int state2)    
{    
    jjCheckNAdd(state1);    
    jjCheckNAdd(state2);    
}    
    
void ExpressionParserTokenManager::jjCheckNAddStates(int start)    
{    
   jjCheckNAdd(jjnextStates[start]);    
   jjCheckNAdd(jjnextStates[start + 1]);    
}    
    
void ExpressionParserTokenManager::jjCheckNAddStates(int start, int end)    
{    
    do {    
        jjCheckNAdd(jjnextStates[start]);    
    } while (start++ != end);    
}    
    
int ExpressionParserTokenManager::jjMoveNfa_0(int startState, int curPos)    
{    
    int* nextStates;    
    int startsAt = 0;    
    jjnewStateCnt = 45;    
    int i = 1;    
    jjstateSet[0] = startState;    
    int j, kind = 0x7fffffff;    
    for (;;)    
    {    
        if (++jjround == 0x7fffffff)    
            ReInitRounds();    
        if (curChar < 64)    
        {    
            int64 l = ((int64)1LL) << curChar;    
            MatchLoop1: do    
            {    
            switch(jjstateSet[--i])    
            {    
                case 0:    
                    if ((0x3ff000000000000LL & l) != 0LL)    
                        jjCheckNAddStates(0, 3);    
                    else if ((0x5000000000000000LL & l) != 0LL)    
                    {    
                        if (kind > 5)    
                        kind = 5;    
                    }    
                    else if (curChar == 35)    
                        jjstateSet[jjnewStateCnt++] = 28;    
                    else if (curChar == 46)    
                        jjCheckNAdd(20);    
                    else if (curChar == 33)    
                    {    
                        if (kind > 14)    
                        kind = 14;    
                    }    
                    else if (curChar == 38)    
                        jjstateSet[jjnewStateCnt++] = 6;    
                    else if (curChar == 61)    
                        jjCheckNAdd(1);    
                    if ((0x3fe000000000000LL & l) != 0LL)    
                    {    
                        if (kind > 20)    
                        kind = 20;    
                        jjCheckNAdd(25);    
                    }    
                    else if (curChar == 48)    
                    {    
                        if (kind > 20)    
                        kind = 20;    
                    }    
                    else if (curChar == 33)    
                        jjCheckNAdd(1);    
                    else if (curChar == 62)    
                        jjCheckNAdd(1);    
                    else if (curChar == 60)    
                        jjCheckNAdd(1);    
                    break;    
                case 1:    
                    if (curChar == 61 && kind > 5)    
                        kind = 5;    
                    break;    
                case 2:    
                    if (curChar == 60)    
                        jjCheckNAdd(1);    
                    break;    
                case 3:    
                    if (curChar == 62)    
                        jjCheckNAdd(1);    
                    break;    
                case 4:    
                    if (curChar == 61)    
                        jjCheckNAdd(1);    
                    break;    
                case 5:    
                    if (curChar == 33)    
                        jjCheckNAdd(1);    
                    break;    
                case 6:    
                    if (curChar == 38 && kind > 12)    
                        kind = 12;    
                    break;    
                case 7:    
                    if (curChar == 38)    
                        jjstateSet[jjnewStateCnt++] = 6;    
                    break;    
                case 15:    
                    if (curChar == 33 && kind > 14)    
                        kind = 14;    
                    break;    
                case 19:    
                    if (curChar == 46)    
                        jjCheckNAdd(20);    
                    break;    
                case 20:    
                    if ((0x3ff000000000000LL & l) == 0LL)    
                        break;    
                    if (kind > 20)    
                        kind = 20;    
                    jjCheckNAddTwoStates(20, 21);    
                    break;    
                case 22:    
                    if ((0x280000000000LL & l) != 0LL)    
                        jjCheckNAdd(23);    
                    break;    
                case 23:    
                    if ((0x3ff000000000000LL & l) == 0LL)    
                        break;    
                    if (kind > 20)    
                        kind = 20;    
                    jjCheckNAdd(23);    
                    break;    
                case 24:    
                    if ((0x3fe000000000000LL & l) == 0LL)    
                        break;    
                    if (kind > 20)    
                        kind = 20;    
                    jjCheckNAdd(25);    
                    break;    
                case 25:    
                    if ((0x3ff000000000000LL & l) == 0LL)    
                        break;    
                    if (kind > 20)    
                        kind = 20;    
                    jjCheckNAdd(25);    
                    break;    
                case 26:    
                    if (curChar == 48 && kind > 20)    
                        kind = 20;    
                    break;    
                case 27:    
                    if (curChar == 35)    
                        jjstateSet[jjnewStateCnt++] = 28;    
                    break;    
                case 29:    
                    if ((0x3ff000000000000LL & l) == 0LL)    
                        break;    
                    if (kind > 23)    
                        kind = 23;    
                    jjstateSet[jjnewStateCnt++] = 29;    
                    break;    
                case 31:    
                    if ((0x3ff000000000000LL & l) != 0LL)    
                        jjAddStates(4, 5);    
                    break;    
                case 32:    
                    if (curChar == 46)    
                        jjstateSet[jjnewStateCnt++] = 30;    
                    break;    
                case 33:    
                    if ((0x3ff000000000000LL & l) == 0LL)    
                        break;    
                    if (kind > 23)    
                        kind = 23;    
                    jjstateSet[jjnewStateCnt++] = 33;    
                    break;    
                case 34:    
                    if ((0x3ff000000000000LL & l) != 0LL)    
                        jjCheckNAddStates(0, 3);    
                    break;    
                case 35:    
                    if ((0x3ff000000000000LL & l) != 0LL)    
                        jjCheckNAddTwoStates(35, 36);    
                    break;    
                case 36:    
                    if (curChar != 46)    
                        break;    
                    if (kind > 20)    
                        kind = 20;    
                    jjCheckNAddTwoStates(37, 38);    
                    break;    
                case 37:    
                    if ((0x3ff000000000000LL & l) == 0LL)    
                        break;    
                    if (kind > 20)    
                        kind = 20;    
                    jjCheckNAddTwoStates(37, 38);    
                    break;    
                case 39:    
                    if ((0x280000000000LL & l) != 0LL)    
                        jjCheckNAdd(40);    
                    break;    
                case 40:    
                    if ((0x3ff000000000000LL & l) == 0LL)    
                        break;    
                    if (kind > 20)    
                        kind = 20;    
                    jjCheckNAdd(40);    
                    break;    
                case 41:    
                    if ((0x3ff000000000000LL & l) != 0LL)    
                        jjCheckNAddTwoStates(41, 42);    
                    break;    
                case 43:    
                    if ((0x280000000000LL & l) != 0LL)    
                        jjCheckNAdd(44);    
                    break;    
                case 44:    
                    if ((0x3ff000000000000LL & l) == 0LL)    
                        break;    
                    if (kind > 20)    
                        kind = 20;    
                    jjCheckNAdd(44);    
                    break;    
                default : break;    
            }    
            } while(i != startsAt);    
        }    
        else if (curChar < 128)    
        {    
            int64 l = ((int64)1LL) << (curChar & 077);    
            MatchLoop2: do    
            {    
            switch(jjstateSet[--i])    
            {    
                case 0:    
                    if ((0x7fffffe87fffffeLL & l) != 0LL)    
                    {    
                        if (kind > 23)    
                            kind = 23;    
                        jjCheckNAddStates(6, 8);    
                    }    
                    else if (curChar == 124)    
                        jjstateSet[jjnewStateCnt++] = 11;    
                    if ((0x400000004000LL & l) != 0L)    
                        jjstateSet[jjnewStateCnt++] = 17;    
                    else if ((0x800000008000LL & l) != 0LL)    
                        jjstateSet[jjnewStateCnt++] = 14;    
                    else if ((0x200000002LL & l) != 0L)    
                        jjstateSet[jjnewStateCnt++] = 9;    
                    break;    
                case 8:    
                    if ((0x200000002LL & l) != 0L)    
                        jjstateSet[jjnewStateCnt++] = 9;    
                    break;    
                case 9:    
                    if ((0x400000004000LL & l) != 0L)    
                        jjstateSet[jjnewStateCnt++] = 10;    
                    break;    
                case 10:    
                    if ((0x1000000010LL & l) != 0L && kind > 12)    
                        kind = 12;    
                    break;    
                case 11:    
                    if (curChar == 124 && kind > 13)    
                        kind = 13;    
                    break;    
                case 12:    
                    if (curChar == 124)    
                        jjstateSet[jjnewStateCnt++] = 11;    
                    break;    
                case 13:    
                    if ((0x800000008000LL & l) != 0LL)    
                        jjstateSet[jjnewStateCnt++] = 14;    
                    break;    
                case 14:    
                    if ((0x4000000040000LL & l) != 0LL && kind > 13)    
                        kind = 13;    
                    break;    
                case 16:    
                    if ((0x400000004000LL & l) != 0LL)    
                        jjstateSet[jjnewStateCnt++] = 17;    
                    break;    
                case 17:    
                    if ((0x800000008000LL & l) != 0LL)    
                        jjstateSet[jjnewStateCnt++] = 18;    
                    break;    
                case 18:    
                    if ((0x10000000100000LL & l) != 0LL && kind > 14)    
                        kind = 14;    
                    break;    
                case 21:    
                    if ((0x2000000020LL & l) != 0LL)    
                        jjAddStates(9, 10);    
                    break;    
                case 28:    
                case 29:    
                    if ((0x7fffffe87fffffeLL & l) == 0LL)    
                        break;    
                    if (kind > 23)    
                        kind = 23;    
                    jjCheckNAdd(29);    
                    break;    
                case 30:    
                    if ((0x7fffffe87fffffeLL & l) == 0LL)    
                        break;    
                    if (kind > 23)    
                        kind = 23;    
                    jjCheckNAddStates(6, 8);    
                    break;    
                case 31:    
                    if ((0x7fffffe87fffffeLL & l) != 0LL)    
                        jjCheckNAddTwoStates(31, 32);    
                    break;    
                case 33:    
                    if ((0x7fffffe87fffffeLL & l) == 0LL)    
                        break;    
                    if (kind > 23)    
                        kind = 23;    
                    jjCheckNAdd(33);    
                    break;    
                case 38:    
                    if ((0x2000000020LL & l) != 0LL)    
                        jjAddStates(11, 12);    
                    break;    
                case 42:    
                    if ((0x2000000020LL & l) != 0LL)    
                        jjAddStates(13, 14);    
                    break;    
                default : break;    
            }    
            } while(i != startsAt);    
        }    
        else    
        {    
            int i2 = (curChar & 0xff) >> 6;    
            int64 l2 = ((int64)1LL) << (curChar & 077);    
            MatchLoop3: do    
            {    
                switch(jjstateSet[--i])    
                {    
                    default : break;    
                }    
            } while(i != startsAt);    
        }    
        if (kind != 0x7fffffff)    
        {    
            jjmatchedKind = kind;    
            jjmatchedPos = curPos;    
            kind = 0x7fffffff;    
        }    
        ++curPos;    
        if ((i = jjnewStateCnt) == (startsAt = 45 - (jjnewStateCnt = startsAt)))    
            return curPos;    
        try {     
            curChar = input_stream->readChar();     
        } catch (...) {     
            return curPos;     
        }    
    }    
 }    
    
void ExpressionParserTokenManager::ReInit(SimpleCharStream* stream)    
{    
    jjmatchedPos = jjnewStateCnt = 0;    
    curLexState = defaultLexState;    
    input_stream = stream;    
    ReInitRounds();    
}    
    
void ExpressionParserTokenManager::ReInitRounds(void)    
{    
    int i;    
    jjround = 0x80000001;    
    for (i = 45; i-- > 0;)    
        jjrounds[i] = 0x80000000;    
}    
    
void ExpressionParserTokenManager::ReInit(SimpleCharStream* stream, int lexState)    
{    
   ReInit(stream);    
   SwitchTo(lexState);    
}    
    
void ExpressionParserTokenManager::SwitchTo(int lexState)    
{    
    if (lexState >= 1 || lexState < 0) {    
        char ex[20];    
        sprintf(ex, "%d\0", lexState);    
        throw RuntimeException("Error: Ignoring invalid lexical state: " + string(ex) + ".State unchanged.");    
    }    
    else    
        curLexState = lexState;    
}    
    
Token* ExpressionParserTokenManager::jjFillToken(void)    
{    
    Token* t = Token::newToken(jjmatchedKind);    
    t->kind = jjmatchedKind;    
    string* im = jjstrLiteralImages[jjmatchedKind];    
    t->image = (im == NULL) ? input_stream->GetImage() : *im;    
    t->beginLine = input_stream->getBeginLine();    
    t->beginColumn = input_stream->getBeginColumn();    
    t->endLine = input_stream->getEndLine();    
    t->endColumn = input_stream->getEndColumn();    
    return t;    
}    
    
Token* ExpressionParserTokenManager::getNextToken(void)    
{    
    int kind;    
    Token* specialToken = 0;    
    Token* matchedToken = 0;    
    int curPos = 0;    
    
    for (;;)    
    {       
        try       
        {         
            curChar = input_stream->BeginToken();    
        } catch(IOException& ex) {            
            jjmatchedKind = 0;    
            matchedToken = jjFillToken();    
            return matchedToken;    
        }    
    
        try {     
            input_stream->backup(0);    
            while (curChar <= 32 && (0x100002600LL & (((int64)1LL) << curChar)) != 0LL)    
                curChar = input_stream->BeginToken();    
        } catch (...) {     
            goto EOFLoop;     
        }    
        jjmatchedKind = 0x7fffffff;    
        jjmatchedPos = 0;    
        curPos = jjMoveStringLiteralDfa0_0();    
        if (jjmatchedKind != 0x7fffffff)    
        {    
            if (jjmatchedPos + 1 < curPos)    
                input_stream->backup(curPos - jjmatchedPos - 1);    
            if ((jjtoToken[jjmatchedKind >> 6] & (((int64)1LL) << (jjmatchedKind & 077))) != 0LL)    
            {    
                matchedToken = jjFillToken();    
                return matchedToken;    
            }    
            else    
            {    
                goto EOFLoop;    
            }    
        }    
        int error_line = input_stream->getEndLine();    
        int error_column = input_stream->getEndColumn();    
        string error_after = "";    
        bool EOFSeen = false;    
        try {     
            input_stream->readChar();     
            input_stream->backup(1);     
        } catch (...) {    
            EOFSeen = true;    
            error_after = curPos <= 1 ? "" : input_stream->GetImage();    
            if (curChar == '\n' || curChar == '\r') {    
                error_line++;    
                error_column = 0;    
            }    
            else    
                error_column++;    
        }    
        if (!EOFSeen) {    
            input_stream->backup(1);    
            error_after = curPos <= 1 ? "" : input_stream->GetImage();    
        }    
        char chrs[1000];    
        if (EOFSeen)    
            sprintf(chrs, "Lexical error at line %d, column %d.  Encountered:  <EOF>\0", error_line, error_column);    
        else  {    
            string a = Exception::add_escapes(string(&curChar, 1));    
            string b = Exception::add_escapes(error_after);    
            sprintf(chrs, "Lexical error at line %d, column %d.  Encountered: \"%s\" (%d) after : \"%s\"\0", error_line, error_column, a.c_str(), curChar, b.c_str());    
        }    
    
        throw RuntimeException(chrs);    
    }    
EOFLoop :    
    return 0;    
}    
