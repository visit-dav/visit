// ************************************************************************* //
//                             avtMixedVariable.h                            //
// ************************************************************************* //

#ifndef AVT_MIXED_VARIABLE_H
#define AVT_MIXED_VARIABLE_H

#include <string>

#include <pipeline_exports.h>


// ****************************************************************************
//  Class: avtMixedVariable
//
//  Purpose:
//      Contains a mixed variable.  This can then be used by the MIR to assign
//      the mixed materials in a zone their exact values.
//
//  Programmer: Hank Childs
//  Creation:   September 27, 2001
//
//  Modifications:
//
//    Jeremy Meredith, Tue Dec  4 13:43:54 PST 2001
//    Added const to constructor arguments.
//
//    Hank Childs, Thu Jul  4 13:02:54 PDT 2002
//    Added names for mixed variables.
//
// ****************************************************************************

class PIPELINE_API avtMixedVariable
{
  public:
                        avtMixedVariable(const float *, int, std::string);
    virtual            ~avtMixedVariable();

    static void         Destruct(void *);

    const float        *GetBuffer(void) { return buffer; };
    int                 GetMixlen(void) { return mixlen; };
    const std::string  &GetVarname(void) { return varname; };
  
  protected:
    std::string         varname;
    float              *buffer;
    int                 mixlen;
};


#endif


