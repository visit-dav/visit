#*****************************************************************************
#
# Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#*****************************************************************************
"""
 File: visit_exprs_parser.py
 Authors: Cyrus Harrison <cyrush@llnl.gov>
          Maysam Moussalem <maysam@tacc.utexas.edu>

 Description:
  ply (python lex & yacc) parser for a simple expression language.
  I used Mayam's visit_exprs.py as a starting point & adapted a subset
  of rules from VisIt's existing expression language parser:
   http://portal.nersc.gov/svn/visit/trunk/src/common/expr/ExprGrammar.C

  I also used the following references:
   http://drdobbs.com/web-development/184405580
   http://www.juanjoconti.com.ar/files/python/ply-examples/

 Usage:
   >>> from parser import *
   >>> print Parser.parse("vx = a(2,3) + b^3 + 4 * var")

"""

import sys
import os

import ply.lex as lex
import ply.yacc as yacc

#------------------------------------------------------------------
# Key objects used to encapsulate the data flow network components.
#------------------------------------------------------------------

class FuncCall(object):
    def __init__(self,name,args=None,params=None):
        self.name   = name
        self.args   = args
        self.params = params
    def __str__(self):
        res = self.name + "("
        if not self.args is None:
            res += str(self.args)
        if not self.params is None:
            if res[-1] != "(": res+= " , "
            res +=  str(self.params)
        res+= ")"
        return res
    def __repr__(self):
       return str(self)

class Assignment(object):
    def __init__(self,name,value):
        self.name = name
        self.value = value
    def __str__(self):
        return str(self.name) + " = " + str(self.value)
    def __repr__(self):
        return str(self)

class Constant(object):
    def __init__(self,value):
        self.value = value
    def __str__(self):
        return "Const(%s)" % str(self.value)
    def __repr__(self):
        return str(self)

class Identifier(object):
    def __init__(self,name):
        self.name = name
    def __str__(self):
        return "Id(" + str(self.name) + ")"
    def __repr__(self):
        return str(self)

#------------------
# Lexer rules
#------------------

#
# lexer token names
#

tokens = ['INT',
          'FLOAT',
          'BOOL',
          'STRING',
          'ID',
          'PLUS',
          'MINUS',
          'MULT',
          'DIV',
          'EXP',
          'GTE',
          'LTE',
          'GT',
          'LT',
          'EQ',
          'ASSIGN',
          'COMMA',
          'LPAREN',
          'RPAREN',
          'LBRACKET',
          'RBRACKET',
          'LBRACE',
          'RBRACE',
          'SEMI',
          "NEWLINE"
          ]

#
# lexer token reg exprs
#

t_PLUS   = r'\+'
t_MINUS  = r'-'
t_MULT   = r'\*'
t_DIV    = r'\\'
t_EXP    = r'\^'

t_GTE    = r'\>\='
t_LTE    = r'\<\='
t_GT     = r'\>'
t_LT     = r'\<'
t_EQ     = r'\=\='
t_ASSIGN = r'\='

t_COMMA  = r'\,'

t_LPAREN = r'\('
t_RPAREN = r'\)'

t_LBRACKET = r'\['
t_RBRACKET = r'\]'

t_LBRACE = r'\{'
t_RBRACE = r'\}'
t_SEMI   = r'\;'


# floating point number
def t_FLOAT(t):
    r'-?\d+\.\d*(e-?\d+)?'
    t.value = float(t.value)
    return t

# integer
def t_INT(t):
    r'-?\d+'
    t.value = int(t.value)
    return t

# boolean value
def t_BOOL(t):
    r'true|false|True|False'
    if t.value.lower() == "true":
        t.value = True
    else:
        t.value = False
    return t

# identifier
def t_ID(t):
    r'[a-zA-Z_][a-zA-Z_0-9]*'
    t.value = Identifier(t.value)
    return t

# string
# Read in a string, as in C.
# The following backslash sequences have their usual special meaning:
#  \", \\, \n, and \t.
def t_STRING(t):
    r'\"([^\\"]|(\\.))*\"'
    escaped = 0
    str = t.value[1:-1]
    new_str = ""
    for i in range(0, len(str)):
        c = str[i]
        if escaped:
            if c == "n":
                c = "\n"
            elif c == "t":
                c = "\t"
            new_str += c
            escaped = 0
        else:
            if c == "\\":
                escaped = 1
            else:
                new_str += c
    t.value = new_str
    return t

t_ignore = " \t"

# support python style comments
def t_COMMENT(t):
    r'\#.*\n*'
    pass
    # No return value. Token discarded


# track line numbers and handle newlines
def t_NEWLINE(t):
    r'\n+'
    t.lexer.lineno += len(t.value)
    return t

# catch all error handler
def t_error(t):
    print("Illegal character '%s'" % t.value[0])
    t.lexer.skip(1)

# Build the lexer
lex.lex()


#------------------
# Parsing rules
#------------------


# used to map symbols to eventual
# data flow filter names
binary_expr_names = {"+":"add",
                     "-":"sub",
                     "*":"mult",
                     "^":"pow",
                     "/":"divide",
                     ">=":"gte",
                     "<=":"lte",
                     ">":"gt",
                     "<":"lt",
                     "==":"equal"}

# Adding precedence rules
precedence = (
    ('left', 'PLUS', 'MINUS'),
    ('left', 'MULT', 'DIV'),
    ('left', 'EXP'),
    ('right', 'EQ', 'LT', 'GT', 'LTE', 'GTE')
)

def p_statements(t):
    """
    statements : statements statement
               | statement
    """
    if len(t) > 2:
        t[0] = t[1] + [t[2]]
    else:
        t[0] = [t[1]]


def p_statement(t):
    """
    statement : assign_expr NEWLINE
              | assign_expr SEMI NEWLINE
              | assign_expr
    """
    t[0] = t[1]

def p_statement_newline(t):
    """
    statement : NEWLINE
    """
    pass


def p_assign(t):
    """
    assign_expr : ID ASSIGN expr
    """
    t[0] = Assignment(t[1].name,t[3])

def p_expr(t):
    """
    expr : binary_expr
         | unary_expr
         | var
         | func
    """
    t[0] = t[1]

def p_expr_paren(t):
    """
    expr : LPAREN expr RPAREN
    """
    t[0] = t[2]

def p_binary_expr(t):
    """
    binary_expr : expr PLUS  expr
                | expr MINUS expr
                | expr MULT  expr
                | expr EXP   expr
                | expr DIV   expr
                | expr GTE   expr
                | expr LTE   expr
                | expr GT    expr
                | expr LT    expr
                | expr EQ    expr
    """
    t[0] = FuncCall(binary_expr_names[t[2]],[t[1],t[3]])

def p_unary_expr(t):
    """
    unary_expr  : MINUS expr
    """
    t[0] = FuncCall(t[1],[t[2]])

def p_func(t):
    """
    func : ID LPAREN args RPAREN
         | ID LPAREN RPAREN
         | LBRACE args RBRACE
         | ID LBRACKET INT RBRACKET
    """
    if t[2] == ")":
        t[0] = FuncCall(t[1].name)
    elif t[1] == "{":
        t[0] = FuncCall("compose",t[2])
    elif t[2] == "[":
        # note, we will need better 'params' support in the
        # future
        t[0] = FuncCall("decompose",[t[1]],{"index":t[3]})
    else:
        t[0] = FuncCall(t[1].name, t[3])

def p_var(t):
    """
    var : const
        | ID
    """
    t[0] = t[1]

def p_const(t):
    """
    const : INT
          | FLOAT
          | BOOL
          | STRING
    """
    t[0] = Constant(t[1])

def p_args_extend(t):
    """
    args : args COMMA expr
    """
    t[0] = t[1] + [t[3]]

def p_args_expr(t):
    """
    args : expr
    """
    t[0] = [t[1]]

# catch all parsing error handler
def p_error(p):
    if p:
        print "<line",p.lineno, "> Syntax Error", p.type, p.value

# Build the parser
yacc.yacc()

class Parser(object):
    @classmethod
    def parse(cls,txt):
        """
        Main entry point for parsing from outside of this module.
        """
        return yacc.parse(txt)


__all__ = ["Parser","FuncCall","Assignment","Constant","Identifier"]
