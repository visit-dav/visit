/*    
 * Copyright (C) 1999-2011 University of Connecticut Health Center    
 *    
 * Licensed under the MIT License (the "License").    
 * You may not use this file except in compliance with the License.    
 * You may obtain a copy of the License at:    
 *    
 *  http://www.opensource.org/licenses/mit-license.php    
 */    
    
#ifndef EXPRESSIONPARSER_H    
#define EXPRESSIONPARSER_H    
    
#include <vector>    
    
#include "Token.h"    
#include "ASTExpression.h"    
#include "ExpressionParserTokenManager.h"    
#include "JJTExpressionParserState.h"    
#include "SimpleCharStream.h"    
#include "ParseException.h"    
    
struct JJCalls {    
    int gen;    
    Token* first;    
    int arg;    
    JJCalls* next;    
};    
    
class LookaheadSuccess : public VCell::Exception {    
public:    
    LookaheadSuccess() : Exception("LookaheadSuccess", "") {};    
    LookaheadSuccess(std::string msg) : Exception("LookaheadSuccess", msg) {};    
};    
    
class ExpressionParser {    
protected:    
    JJTExpressionParserState* jjtree;      
    SimpleCharStream* jj_input_stream;      
    
public:    
    ExpressionParser(ExpressionParserTokenManager* tm);    
    ExpressionParser(std::istream* stream);    
    ~ExpressionParser();    
    ASTExpression* Expression(void);    
    
private:    
    bool lookingAhead;    
    ExpressionParserTokenManager* token_source;    
    Token* token, *jj_nt, *headToken;    
    void LogicalORExpression(void);    
    void LogicalANDExpression(void);    
    void AdditiveExpression(void);    
    void MinusTerm(void);    
    void MultiplicativeExpression(void);    
    void RelationalExpression(void);    
    void DivTerm(void);    
    void PowerTerm(void);    
    void UnaryExpression(void);    
    void PrimaryExpression(void);    
    void Function(void);    
    void Arguments(void);    
    void ArgumentList(void);    
    void Float(void);    
    void Name(void);        
    VCell::ParseException& generateParseException(void);    
    
    int jj_ntk;    
    Token* jj_scanpos, *jj_lastpos;    
    int jj_la;      
    int jj_gen;    
    int* jj_la1;    
            
    JJCalls** jj_2_rtns;    
    int jj_2_rtns_length;    
    bool jj_rescan;    
    int jj_gc;    
    
    LookaheadSuccess jj_ls;    
    
    std::vector< std::vector<int>* > jj_expentries;    
    std::vector<int> *jj_expentry;    
    int jj_kind;    
    int* jj_lasttokens;    
    int jj_endpos;        
    Token* jj_consume_token(int kind);    
    int jj_ntk_func(void);    
    bool jj_2_1(int xla);    
    void jj_save(int index, int xla);    
    bool jj_3_1(void);    
    bool jj_scan_token(int kind);    
    void jj_add_error_token(int kind, int pos);    
    bool jj_2_2(int xla);    
    bool jj_3_2(void);    
    bool jj_2_3(int xla);    
    bool jj_3_3(void);    
    bool jj_2_4(int xla);    
    bool jj_3_4(void);    
    bool jj_3R_7(void);    
    
    void init(void);        
    void jj_rescan_token(void);        
};    
    
#endif    
