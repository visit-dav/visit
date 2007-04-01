#ifndef TOKENPARSETREENODE_H
#define TOKENPARSETREENODE_H

#include <ParseTreeNode.h>
#include <Token.h>

class PARSER_API TokenParseTreeNode : public ParseTreeNode
{
  public:
    TokenParseTreeNode(Token *t) : ParseTreeNode(t->GetPos()), token(t) { }
    virtual ~TokenParseTreeNode() {}
    virtual const std::string GetTypeName() {return "TokenParseTreeNode";}
    virtual void PrintNode(ostream &o);
    Token *GetToken() { return token; }
  protected:
    Token *token;
};

#endif
