#ifndef ENUM_H
#define ENUM_H

#include <qstring.h>
#include <vector>
using std::vector;

// ****************************************************************************
//  Class:  EnumType
//
//  Purpose:
//    Abstraction for an enumerated type.
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 28, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Oct 17 15:57:44 PDT 2002
//    Moved the static data to a .C file.
//
// ****************************************************************************
class EnumType
{
  public:
    static vector<EnumType*> enums;
    static EnumType *FindEnum(const QString &s)
    {
        EnumType *e = NULL;
        for (int i=0; i<enums.size(); i++)
        {
            if (enums[i]->type == s)
            {
                e = enums[i];
            }
        }
        if (!e)
            throw QString().sprintf("unknown enum subtype '%s'",s.latin1());
        return e;
    }
  public:
    QString         type;
    vector<QString> values;
  public:
    EnumType(const QString &s) : type(s) 
    { 
    }
    void AddValue(const QString &s)
    {
        values.push_back(s);
    }
    const QString& GetValue(int index)
    {
        if (index < 0  ||  index >= values.size())
            throw QString().sprintf("tried to access out-of-bounds enum type %d",index);
        return values[index];
    }
    void Print(ostream &out)
    {
        cout << "Enum: " << type << endl;
        for (int i=0; i<values.size(); i++)
        {
            cout << "    " << values[i] << endl;
        }
    }
};

#endif
