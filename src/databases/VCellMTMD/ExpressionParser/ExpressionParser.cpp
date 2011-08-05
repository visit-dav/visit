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
#include <vector>
using std::vector;
    
#include "ASTOrNode.h"    
#include "ASTAndNode.h"    
#include "ASTRelationalNode.h"    
#include "ASTAddNode.h"    
#include "ASTMinusTermNode.h"    
#include "ASTMultNode.h"    
#include "ASTInvertTermNode.h"    
#include "ASTPowerNode.h"    
#include "ASTNotNode.h"    
#include "ASTFuncNode.h"    
#include "ASTFloatNode.h"    
#include "ASTIdNode.h"    
#include "ExpressionParser.h"    
#include "ParseException.h"    
#include "RuntimeException.h"    
#include "ExpressionParserConstants.h"    
#include "ExpressionParserTreeConstants.h"    
    
using namespace VCell;
using std::istream;

int64 jj_la1_0[] = {0x2000,0x1000,0x30000,0x30000,0xc0000,0xc0000,0x10934000,0x10100000,0x10934000,0x40000000,};    
    
ExpressionParser::ExpressionParser(istream* stream)    
{        
    init();    
    jj_input_stream = new SimpleCharStream(stream, 1, 1);    
    token_source = new ExpressionParserTokenManager(jj_input_stream);    
}    
    
ExpressionParser::ExpressionParser(ExpressionParserTokenManager* tm)    
{    
    init();    
    jj_input_stream = 0;    
    token_source = tm;        
}    
    
ExpressionParser::~ExpressionParser()    
{        
    while (headToken != 0) {    
        Token* token = headToken;    
        headToken = headToken->next;    
        delete token;    
    }    
    delete token_source;    
    delete jjtree;    
    delete jj_input_stream;    
    delete[] jj_lasttokens;    
    delete[] jj_la1;    
    
    for (int i = 0; i < jj_2_rtns_length; i++) {    
        while (jj_2_rtns[i] != 0) {    
            JJCalls* p = jj_2_rtns[i];    
            jj_2_rtns[i] = jj_2_rtns[i]->next;    
            delete p;    
        }    
        delete jj_2_rtns[i];    
    }    
    delete[] jj_2_rtns;    
    
    for (unsigned int i = 0; i < jj_expentries.size(); i ++) {    
        jj_expentries[i]->clear();    
        delete jj_expentries[i];    
    }    
    jj_expentries.clear();    
}    
    
void ExpressionParser::init(void)    
{    
    jjtree = new JJTExpressionParserState();          
    
    jj_la = 0;    
    jj_nt = 0;    
    jj_lasttokens = new int[100];    
    memset(jj_lasttokens, 0, 100*sizeof(int));    
    
    jj_la1 = new int[10];        
    for (int i = 0; i < 10; i++)    
        jj_la1[i] = -1;    
    
    jj_2_rtns_length = 4;    
    jj_2_rtns = new JJCalls*[jj_2_rtns_length];    
    for (int i = 0; i < jj_2_rtns_length; i++)    
        jj_2_rtns[i] = new JJCalls();    
    
    jj_scanpos = 0;    
    jj_lastpos = 0;    
    token = new Token();    
    headToken = token;    
    jj_ntk = -1;    
    jj_gen = 0;    
    jj_rescan = false;    
    jj_gc = 0;    
        
    lookingAhead = false;    
    jj_kind = -1;    
    jj_endpos = 0;    
    
    jj_expentry = 0;    
}    
    
ASTExpression* ExpressionParser::Expression(void)    
{    
    ASTExpression* jjtn000 = new ASTExpression(JJTEXPRESSION);    
    bool jjtc000 = true;    
    jjtree->openNodeScope(jjtn000);    
    try {    
        LogicalORExpression();    
        jj_consume_token(27);    
        jjtree->closeNodeScope(jjtn000, true);    
        jjtc000 = false;    
        {    
            if (jjtc000) {    
                jjtree->closeNodeScope(jjtn000, true);    
            }    
            if (true)     
                return jjtn000;    
        }    
    } catch (Exception& ex) {    
        if (jjtc000) {    
            jjtree->clearNodeScope(jjtn000);    
            jjtc000 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
        Exception::rethrowException(ex);    
    } catch (...) {    
        if (jjtc000) {    
            jjtree->clearNodeScope(jjtn000);    
            jjtc000 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
    
        throw RuntimeException("Exception in ExpressionParser::Expression");    
    }         
    throw RuntimeException("Missing return statement in function");    
}    
    
void ExpressionParser::LogicalORExpression(void)    
{    
    ASTOrNode* jjtn001 = new ASTOrNode(JJTORNODE);    
    bool jjtc001 = true;    
    jjtree->openNodeScope(jjtn001);    
    try {    
        LogicalANDExpression();    
        while (true) {    
            switch ((jj_ntk == -1) ? jj_ntk_func() : jj_ntk) {    
                case OR:    
                    ;    
                    break;    
                default:    
                    jj_la1[0] = jj_gen;    
                    goto label_1;    
            }    
            jj_consume_token(OR);    
            LogicalANDExpression();    
        }    
label_1:    
        if (jjtc001) {    
            jjtree->closeNodeScope(jjtn001, jjtree->nodeArity() > 1);    
        }    
    } catch (Exception& ex) {    
        if (jjtc001) {    
            jjtree->clearNodeScope(jjtn001);    
            jjtc001 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc001) {    
            jjtree->closeNodeScope(jjtn001, jjtree->nodeArity() > 1);    
        }    
        Exception::rethrowException(ex);    
    } catch (...) {    
        if (jjtc001) {    
            jjtree->clearNodeScope(jjtn001);    
            jjtc001 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc001) {    
            jjtree->closeNodeScope(jjtn001, jjtree->nodeArity() > 1);    
        }    
        throw RuntimeException("Exception in ExpressionParser::LogicalORExpression");    
    }    
}    
    
void ExpressionParser::LogicalANDExpression(void)    
{    
    ASTAndNode* jjtn001 = new ASTAndNode(JJTANDNODE);    
    bool jjtc001 = true;    
    jjtree->openNodeScope(jjtn001);    
    try {    
        RelationalExpression();            
        while (true) {    
            switch ((jj_ntk==-1)?jj_ntk_func():jj_ntk) {    
                case AND:    
                ;    
                break;    
            default:    
                jj_la1[1] = jj_gen;    
                goto label_2;    
            }    
            jj_consume_token(AND);    
            RelationalExpression();    
        }    
label_2:    
        if (jjtc001) {    
            jjtree->closeNodeScope(jjtn001, jjtree->nodeArity() > 1);    
        }    
    } catch (Exception& ex) {    
        if (jjtc001) {    
            jjtree->clearNodeScope(jjtn001);    
            jjtc001 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc001) {    
            jjtree->closeNodeScope(jjtn001, jjtree->nodeArity() > 1);    
        }    
        Exception::rethrowException(ex);    
    } catch (...) {    
        if (jjtc001) {    
            jjtree->clearNodeScope(jjtn001);    
            jjtc001 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc001) {    
            jjtree->closeNodeScope(jjtn001, jjtree->nodeArity() > 1);    
        }    
        throw RuntimeException("Exception in ExpressionParser::LogicalANDExpression.");    
    }        
}    
    
Token* ExpressionParser::jj_consume_token(int kind)    
{    
    Token* oldToken = 0;    
    if ((oldToken = token)->next != 0)     
        token = token->next;    
    else     
        token = token->next = token_source->getNextToken();    
    jj_ntk = -1;    
        
    if (token->kind == kind) {    
        jj_gen++;    
        if (++jj_gc > 100) {    
            jj_gc = 0;    
            for (int i = 0; i < jj_2_rtns_length; i++) {    
                JJCalls* c = jj_2_rtns[i];    
                while (c != 0) {    
                    if (c->gen < jj_gen)     
                        c->first = 0;    
                    c = c->next;    
                }    
            }    
        }    
        return token;    
    }    
    token = oldToken;    
    jj_kind = kind;    
    throw generateParseException();    
}    
    
void ExpressionParser::RelationalExpression(void)    
{    
    Token* t_name = NULL;    
    AdditiveExpression();    
    if (jj_2_1(2147483647)) {    
        t_name = jj_consume_token(RELATIONAL_OPERATOR);    
        AdditiveExpression();    
        ASTRelationalNode* jjtn001 = new ASTRelationalNode(JJTRELATIONALNODE);    
        bool jjtc001 = true;    
        jjtree->openNodeScope(jjtn001);    
        try {    
            jjtree->closeNodeScope(jjtn001,  2);    
            jjtc001 = false;    
            if (jjtn001!=NULL){    
                ((ASTRelationalNode*)jjtn001)->setOperationFromToken(t_name->image);    
            }    
            if (jjtc001) {    
                jjtree->closeNodeScope(jjtn001,  2);    
            }    
        } catch (...) {                
            if (jjtc001) {    
                jjtree->closeNodeScope(jjtn001,  2);    
            }    
        }    
    } else {    
        ;    
    }    
}    
    
int ExpressionParser::jj_ntk_func(void)    
{    
    if ((jj_nt=token->next) == NULL)    
        return (jj_ntk = (token->next=token_source->getNextToken())->kind);    
    else    
        return (jj_ntk = jj_nt->kind);    
}    
    
void ExpressionParser::AdditiveExpression(void)    
{    
    ASTAddNode* jjtn001 = new ASTAddNode(JJTADDNODE);    
    bool jjtc001 = true;    
    jjtree->openNodeScope(jjtn001);    
    try {    
        MultiplicativeExpression();    
    
        while (true) {    
            switch ((jj_ntk==-1)?jj_ntk_func():jj_ntk) {    
                case ADD:    
                case SUB:    
                    ;    
                    break;    
                default:    
                    jj_la1[2] = jj_gen;    
                    goto label_3;    
            }    
            switch ((jj_ntk==-1)?jj_ntk_func():jj_ntk) {    
                case ADD:                        
                    jj_consume_token(ADD);    
                    MultiplicativeExpression();    
                    break;    
                case SUB:    
                    MinusTerm();    
                    break;    
                default:    
                    jj_la1[3] = jj_gen;    
                    jj_consume_token(-1);    
                    throw ParseException();    
            }    
        }    
label_3:    
        if (jjtc001) {    
            jjtree->closeNodeScope(jjtn001, jjtree->nodeArity() > 1);    
        }    
    } catch (Exception& ex) {    
        if (jjtc001) {    
            jjtree->clearNodeScope(jjtn001);    
            jjtc001 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc001) {    
            jjtree->closeNodeScope(jjtn001, jjtree->nodeArity() > 1);    
        }    
        Exception::rethrowException(ex);    
    } catch (...) {    
        if (jjtc001) {    
            jjtree->clearNodeScope(jjtn001);    
            jjtc001 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc001) {    
            jjtree->closeNodeScope(jjtn001, jjtree->nodeArity() > 1);    
        }    
        throw RuntimeException("Exception in ExpressionParser::AdditiveExpression");    
    }     
}    
    
void ExpressionParser::MinusTerm(void)    
{    
    ASTMinusTermNode* jjtn000 = new ASTMinusTermNode(JJTMINUSTERMNODE);    
    bool jjtc000 = true;    
    jjtree->openNodeScope(jjtn000);    
    try {    
        jj_consume_token(SUB);    
        MultiplicativeExpression();    
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
    } catch (Exception& ex) {    
        if (jjtc000) {    
            jjtree->clearNodeScope(jjtn000);    
            jjtc000 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
        Exception::rethrowException(ex);    
    } catch (...) {    
        if (jjtc000) {    
            jjtree->clearNodeScope(jjtn000);    
            jjtc000 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
    
        throw RuntimeException("Exception in ExpressionParser::MinusTerm.");    
    }    
}    
    
void ExpressionParser::MultiplicativeExpression(void)    
{    
    ASTMultNode* jjtn001 = new ASTMultNode(JJTMULTNODE);    
    bool jjtc001 = true;    
    jjtree->openNodeScope(jjtn001);    
    try {    
        PowerTerm();    
    
        while (true) {    
            switch ((jj_ntk==-1)?jj_ntk_func():jj_ntk) {    
                case MULT:    
                case DIV:    
                    ;    
                    break;    
                default:    
                    jj_la1[4] = jj_gen;                        
                    goto label_4;    
            }    
            switch ((jj_ntk==-1)?jj_ntk_func():jj_ntk) {    
                case (MULT):    
                    jj_consume_token(MULT);    
                    PowerTerm();    
                    break;    
                case DIV:    
                    DivTerm();    
                    break;    
                default:    
                    jj_la1[5] = jj_gen;    
                    jj_consume_token(-1);    
                    throw ParseException();    
            }    
        }    
label_4:    
        if (jjtc001) {    
            jjtree->closeNodeScope(jjtn001, jjtree->nodeArity() > 1);    
        }    
    } catch (Exception& ex) {    
        if (jjtc001) {    
            jjtree->clearNodeScope(jjtn001);    
            jjtc001 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc001) {    
            jjtree->closeNodeScope(jjtn001, jjtree->nodeArity() > 1);    
        }    
        Exception::rethrowException(ex);    
    } catch (...) {    
        if (jjtc001) {    
            jjtree->clearNodeScope(jjtn001);    
            jjtc001 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc001) {    
            jjtree->closeNodeScope(jjtn001, jjtree->nodeArity() > 1);    
        }    
        throw RuntimeException("Exception in ExpressionParser::MultiplicativeExpression");    
    }    
}    
    
bool ExpressionParser::jj_2_1(int xla)    
{    
    jj_la = xla;     
    jj_lastpos = jj_scanpos = token;    
    try {    
        bool retval = !jj_3_1();    
        jj_save(0, xla);    
        return retval;    
    } catch (LookaheadSuccess ls) {    
        jj_save(0, xla);    
        return true;    
    }    
}    
    
void ExpressionParser::jj_save(int index, int xla)    
{    
    JJCalls* p = jj_2_rtns[index];    
    while (p->gen > jj_gen) {    
        if (p->next == NULL) {     
            p = p->next = new JJCalls();     
            break;     
        }    
        p = p->next;    
    }    
    p->gen = jj_gen + xla - jj_la;     
    p->first = token;     
    p->arg = xla;    
}    
    
bool ExpressionParser::jj_3_1(void)    
{    
    if (jj_scan_token(RELATIONAL_OPERATOR))     
        return true;    
    return false;    
}    
    
bool ExpressionParser::jj_scan_token(int kind)    
{    
    if (jj_scanpos == jj_lastpos) {    
        jj_la--;    
        if (jj_scanpos->next == NULL) {    
            jj_lastpos = jj_scanpos = jj_scanpos->next = token_source->getNextToken();    
        } else {    
            jj_lastpos = jj_scanpos = jj_scanpos->next;    
        }    
    } else {    
        jj_scanpos = jj_scanpos->next;    
    }    
    if (jj_rescan) {    
        int i = 0;     
        Token* tok = token;    
        while (tok != NULL && tok != jj_scanpos) {     
            i++;     
            tok = tok->next;     
        }    
        if (tok != NULL)     
            jj_add_error_token(kind, i);    
    }    
    if (jj_scanpos->kind != kind)    
        return true;    
    if (jj_la == 0 && jj_scanpos == jj_lastpos)     
        throw jj_ls;    
    return false;    
}    
    
void ExpressionParser::jj_add_error_token(int kind, int pos)    
{    
    if (pos >= 100)     
        return;    
    if (pos == jj_endpos + 1) {    
        jj_lasttokens[jj_endpos++] = kind;    
    } else if (jj_endpos != 0) {            
        jj_expentry = new vector<int>;    
        for (int i = 0; i < jj_endpos; i++) {    
            jj_expentry->push_back(jj_lasttokens[i]);    
        }            
        bool exists = false;            
        for (vector< vector<int>* >::iterator iter = jj_expentries.begin(); iter != jj_expentries.end(); iter ++) {    
            vector<int>* oldentry = (vector<int>*)(*iter);    
            if (oldentry->size() == jj_expentry->size()) {    
                exists = true;    
                for (unsigned int i = 0; i < jj_expentry->size(); i++) {    
                    if ((*oldentry)[i] != (*jj_expentry)[i]) {    
                        exists = false;    
                        break;    
                    }    
                }    
                if (exists) break;    
            }    
        }    
        if (!exists)     
            jj_expentries.push_back(jj_expentry);    
        if (pos != 0)     
            jj_lasttokens[(jj_endpos = pos) - 1] = kind;    
    }    
}    
    
void ExpressionParser::DivTerm(void)    
{    
    ASTInvertTermNode* jjtn000 = new ASTInvertTermNode(JJTINVERTTERMNODE);    
    bool jjtc000 = true;    
    jjtree->openNodeScope(jjtn000);    
    try {    
        jj_consume_token(DIV);    
        PowerTerm();    
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
    } catch (Exception& ex) {    
        if (jjtc000) {    
            jjtree->clearNodeScope(jjtn000);    
            jjtc000 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
        Exception::rethrowException(ex);    
    } catch (...) {    
        if (jjtc000) {    
            jjtree->clearNodeScope(jjtn000);    
            jjtc000 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
        throw RuntimeException("Exception in ExpressionParser::DivTerm");    
    }    
}    
    
void ExpressionParser::PowerTerm(void)    
{    
    UnaryExpression();    
     
    while (true) {    
        if (jj_2_2(2147483647)) {    
            ;    
        } else {    
            goto label_5;    
        }    
        jj_consume_token(POWER);    
        ASTPowerNode* jjtn001 = new ASTPowerNode(JJTPOWERNODE);    
        bool jjtc001 = true;    
        jjtree->openNodeScope(jjtn001);    
        try {    
            UnaryExpression();    
            if (jjtc001) {    
                jjtree->closeNodeScope(jjtn001, 2);    
            }    
        } catch (Exception& ex) {    
            if (jjtc001) {    
                jjtree->clearNodeScope(jjtn001);    
                jjtc001 = false;    
            } else {    
                jjtree->popNode();    
            }    
            if (jjtc001) {    
                jjtree->closeNodeScope(jjtn001, 2);    
            }    
            Exception::rethrowException(ex);    
        } catch (...) {    
            if (jjtc001) {    
                jjtree->clearNodeScope(jjtn001);    
                jjtc001 = false;    
            } else {    
                jjtree->popNode();    
            }    
            if (jjtc001) {    
                jjtree->closeNodeScope(jjtn001, 2);    
            }    
            throw RuntimeException("Exception in ExpressionParser::PowerTerm");    
        }    
    }    
label_5 :    
    ;    
}    
    
bool ExpressionParser::jj_2_2(int xla)    
{    
    jj_la = xla;    
    jj_lastpos = jj_scanpos = token;    
    try {    
        bool retval = !jj_3_2();    
        jj_save(1, xla);    
        return retval;    
    } catch (LookaheadSuccess ls) {    
        jj_save(1, xla);    
        return true;    
    }    
}    
    
bool ExpressionParser::jj_3_2(void)    
{    
    if (jj_scan_token(POWER))    
        return true;    
    return false;    
}    
    
void ExpressionParser::UnaryExpression(void)    
{    
    switch ((jj_ntk == -1) ? jj_ntk_func() : jj_ntk) {    
        case ADD :    
            jj_consume_token(ADD);    
            UnaryExpression();    
            break;    
        case SUB :     
        {    
            ASTMinusTermNode* jjtn001 = new ASTMinusTermNode(JJTMINUSTERMNODE);    
            bool jjtc001 = true;    
            jjtree->openNodeScope(jjtn001);    
            try {    
                jj_consume_token(SUB);    
                UnaryExpression();    
                if (jjtc001) {    
                    jjtree->closeNodeScope(jjtn001, true);    
                }    
            } catch (Exception& ex) {    
                if (jjtc001) {    
                    jjtree->clearNodeScope(jjtn001);    
                    jjtc001 = false;    
                } else {    
                    jjtree->popNode();    
                }    
                if (jjtc001) {    
                    jjtree->closeNodeScope(jjtn001, true);    
                }     
                Exception::rethrowException(ex);    
            } catch (...) {    
                if (jjtc001) {    
                    jjtree->clearNodeScope(jjtn001);    
                    jjtc001 = false;    
                } else {    
                    jjtree->popNode();    
                }    
                if (jjtc001) {    
                    jjtree->closeNodeScope(jjtn001, true);    
                }     
                throw RuntimeException("Exception in ExpressionParser::UnaryExpression");            
            }    
            break;    
        }    
        case NOT : {    
            ASTNotNode* jjtn002 = new ASTNotNode(JJTNOTNODE);    
            bool jjtc002 = true;    
            jjtree->openNodeScope(jjtn002);    
            try {    
                jj_consume_token(NOT);    
                UnaryExpression();    
                if (jjtc002) {    
                    jjtree->closeNodeScope(jjtn002, true);    
                }    
            } catch (Exception& ex) {    
                if (jjtc002) {    
                    jjtree->clearNodeScope(jjtn002);    
                    jjtc002 = false;    
                } else {    
                    jjtree->popNode();    
                }    
                if (jjtc002) {    
                    jjtree->closeNodeScope(jjtn002, true);    
                }    
                Exception::rethrowException(ex);    
            } catch (...) {    
                if (jjtc002) {    
                    jjtree->clearNodeScope(jjtn002);    
                    jjtc002 = false;    
                } else {    
                    jjtree->popNode();    
                }    
                if (jjtc002) {    
                    jjtree->closeNodeScope(jjtn002, true);    
                }    
                throw RuntimeException("Exception in ExpressionParser::UnaryExpression");    
            }    
            break;    
        }    
        case FLOATING_POINT_LITERAL :    
        case IDENTIFIER :    
        case 28 :    
            PrimaryExpression();    
            break;    
        default :    
            jj_la1[6] = jj_gen;    
            jj_consume_token(-1);    
            throw ParseException();    
    }    
}    
    
void ExpressionParser::PrimaryExpression(void)    
{    
    if (jj_2_3(2147483647)) {    
        Function();    
    } else if (jj_2_4(2147483647)) {    
        Name();    
    } else {    
        switch ((jj_ntk == -1) ? jj_ntk_func() : jj_ntk) {    
            case FLOATING_POINT_LITERAL :    
                Float();    
                break;    
            case 28 :    
                jj_consume_token(28);    
                LogicalORExpression();    
                jj_consume_token(29);    
                break;    
            default :    
                jj_la1[7] = jj_gen;    
                jj_consume_token(-1);    
                throw ParseException();    
        }    
    }    
}    
    
bool ExpressionParser::jj_2_3(int xla)    
{    
    jj_la = xla;    
    jj_lastpos = jj_scanpos = token;    
    try {    
        bool retval = !jj_3_3();    
        jj_save(2, xla);    
        return retval;    
    } catch (LookaheadSuccess ls) {    
        jj_save(2, xla);    
        return true;    
    }    
}    
    
bool ExpressionParser::jj_3_3(void)    
{    
    if (jj_scan_token(IDENTIFIER))    
        return true;    
    if (jj_scan_token(28))    
        return true;    
    return false;    
}    
    
bool ExpressionParser::jj_2_4(int xla)    
{    
    jj_la = xla;    
    jj_lastpos = jj_scanpos = token;    
    try {    
        bool retval = !jj_3_4();    
        jj_save(3, xla);    
        return retval;    
    } catch (LookaheadSuccess ls) {    
        jj_save(3, xla);    
        return true;    
    }    
}    
    
bool ExpressionParser::jj_3_4(void)    
{    
    if (jj_3R_7())    
        return true;    
    return false;    
}    
    
bool ExpressionParser::jj_3R_7(void)    
{    
    if (jj_scan_token(IDENTIFIER))    
        return true;    
    return false;    
}    
    
void ExpressionParser::Function(void)    
{    
    ASTFuncNode* jjtn000 = new ASTFuncNode(JJTFUNCNODE);    
    bool jjtc000 = true;    
    jjtree->openNodeScope(jjtn000);    
    Token* t;    
    try {    
        t = jj_consume_token(IDENTIFIER);    
        Arguments();    
        jjtree->closeNodeScope(jjtn000, true);    
        jjtc000 = false;    
        jjtn000->setFunctionFromParserToken(t->image);    
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
    } catch (Exception& ex) {    
        if (jjtc000) {    
            jjtree->clearNodeScope(jjtn000);    
            jjtc000 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
        Exception::rethrowException(ex);    
    } catch (...) {    
        if (jjtc000) {    
            jjtree->clearNodeScope(jjtn000);    
            jjtc000 = false;    
        } else {    
            jjtree->popNode();    
        }    
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
        throw RuntimeException("Exception in ExpressionParser::Function()");    
    }                
}    
    
void ExpressionParser::Arguments(void)    
{    
    jj_consume_token(28);    
    switch ((jj_ntk == -1) ? jj_ntk_func() : jj_ntk) {    
        case NOT :    
        case ADD :    
        case SUB :    
        case FLOATING_POINT_LITERAL :    
        case IDENTIFIER :    
        case 28 :    
            ArgumentList();    
            break;    
        default :    
            jj_la1[8] = jj_gen;    
            ;    
    }    
    jj_consume_token(29);    
}    
    
void ExpressionParser::ArgumentList(void)    
{    
    LogicalORExpression();    
    while (true) {    
        switch ((jj_ntk == -1) ? jj_ntk_func() : jj_ntk) {    
            case 30 :    
                ;    
                break;    
            default :    
                jj_la1[9] = jj_gen;    
                goto label_6;    
        }    
        jj_consume_token(30);    
        LogicalORExpression();    
    }    
label_6 :     
    ;    
}    
    
void ExpressionParser::Float(void)    
{    
    ASTFloatNode* jjtn000 = new ASTFloatNode(JJTFLOATNODE);    
    bool jjtc000 = true;    
    jjtree->openNodeScope(jjtn000);    
    Token* t;    
    try {    
        t = jj_consume_token(FLOATING_POINT_LITERAL);    
        jjtree->closeNodeScope(jjtn000, true);    
        jjtc000 = false;    
        jjtn000->value = atof(t->image.c_str());            
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
    } catch (...) {            
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
    }    
}    
    
void ExpressionParser::Name(void)    
{    
    ASTIdNode* jjtn000 = new ASTIdNode(JJTIDNODE);    
    bool jjtc000 = true;    
    jjtree->openNodeScope(jjtn000);    
    Token* t_name = NULL;    
    try {    
        t_name = jj_consume_token(IDENTIFIER);    
        jjtree->closeNodeScope(jjtn000, true);    
        jjtc000 = false;    
        jjtn000->name = t_name->image;    
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
    } catch(...) {    
        if (jjtc000) {    
            jjtree->closeNodeScope(jjtn000, true);    
        }    
    }    
}    
    
ParseException& ExpressionParser::generateParseException(void)    
{    
    jj_expentries.clear();    
    bool* la1tokens = new bool[31];    
    for (int i = 0; i < 31; i++) {    
        la1tokens[i] = false;    
    }    
    if (jj_kind >= 0) {    
        la1tokens[jj_kind] = true;    
        jj_kind = -1;    
    }    
    
    for (int i = 0; i < 10; i++) {    
        if (jj_la1[i] == jj_gen) {    
            for (int j = 0; j < 32; j++) {    
                 if ((jj_la1_0[i] & (1 << j)) != 0) {    
                    la1tokens[j] = true;    
                }    
            }    
        }    
    }    
    for (int i = 0; i < 31; i++) {    
        if (la1tokens[i]) {    
            jj_expentry = new vector<int>;    
            jj_expentry->push_back(i);    
            jj_expentries.push_back(jj_expentry);    
        }    
    }    
    
    jj_endpos = 0;    
    jj_rescan_token();    
    jj_add_error_token(0, 0);    
    int numETS = (int)jj_expentries.size();    
    int* etsLengthArray = new int[numETS];        
    int** exptokseq = new int*[numETS];    
    for (int i = 0; i < numETS; i++) {    
        vector<int>* entry = (vector<int>*)(jj_expentries.at(i));    
        etsLengthArray[i] = (int)entry->size();    
        exptokseq[i] = new int[etsLengthArray[i]];    
        for (int j = 0; j < etsLengthArray[i]; j ++) {    
            exptokseq[i][j] = entry->at(j);    
        }    
    }    
        
    ParseException* ex = new ParseException(token, exptokseq, numETS, etsLengthArray, tokenImage);    
    throw (*ex);    
}    
    
void ExpressionParser::jj_rescan_token(void)    
{    
    jj_rescan = true;    
    for (int i = 0; i < 4; i++) {    
        try {    
            JJCalls* p = jj_2_rtns[i];    
            do {    
                if (p->gen > jj_gen) {    
                    jj_la = p->arg;    
                    jj_lastpos = jj_scanpos = p->first;    
                    switch (i) {    
                        case 0 :    
                            jj_3_1();    
                            break;    
                        case 1 :    
                            jj_3_2();    
                            break;    
                        case 2 :    
                            jj_3_3();    
                            break;    
                        case 3 :    
                            jj_3_4();    
                            break;    
                    }    
                }    
                p = p->next;    
            } while (p != NULL);    
        } catch(LookaheadSuccess& ls) { }    
    }    
    jj_rescan = false;    
}    
