#ifndef INCLUDE_H
#define INCLUDE_H

#include <qstring.h>
#include <visitstream.h>

// ****************************************************************************
//  Class:  Include
//
//  Purpose:
//    Abstraction for an include file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 15, 2002
//
// ****************************************************************************
class Include
{
  public:
    QString include;
    QString target;
    bool    quoted;
  public:
    Include(const QString &t, bool q) : target(t), quoted(q)
    {
        if (t != "header" && t != "source")
            throw QString().sprintf("Bad target file '%s' for include.",t.latin1());
    }
    void SetInclude(const QString &i)
    {
        include = i;
    }
    void Print(ostream &out)
    {
        out << "        Include: " << include << endl;
    }
};

#endif
