#include <ParserInterface.h>
#include <ExprParser.h>

ParserInterface*
ParserInterface::MakeParser(ExprNodeFactory *f)
{
    return new ExprParser(f);
}
