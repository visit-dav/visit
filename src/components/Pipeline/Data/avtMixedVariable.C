// ************************************************************************* //
//                           avtMixedVariable.C                              //
// ************************************************************************* //

#include <avtMixedVariable.h>

// For NULL
#include <stdio.h>


using std::string;


// ****************************************************************************
//  Method: avtMixedVariable constructor
//
//  Arguments:
//      b       The mixed variable buffer
//      ml      The number of elements in b.
//      vn      The variable name with this mixed var.
//
//  Programmer: Hank Childs
//  Creation:   September 27, 2001
//
//  Modifications:
//
//    Jeremy Meredith, Tue Dec  4 13:44:28 PST 2001
//    Added const to constructor arguments.
//
//    Hank Childs, Thu Jul  4 13:02:54 PDT 2002
//    Add variable names.
//
// ****************************************************************************

avtMixedVariable::avtMixedVariable(const float *b, int ml, string vn)
{
    mixlen  = ml;
    varname = vn;
    buffer = new float[mixlen];
    for (int i = 0 ; i < mixlen ; i++)
    {
        buffer[i] = b[i];
    }
}


// ****************************************************************************
//  Method: avtMixedVariable destructor
//
//  Programmer: Hank Childs
//  Creation:   September 27, 2001
//
// ****************************************************************************

avtMixedVariable::~avtMixedVariable()
{
    if (buffer != NULL)
    {
        delete [] buffer;
        buffer = NULL;
    }
}


// ****************************************************************************
//  Method: avtMixedVariable::Destruct
//
//  Purpose:
//      Destructs a mixed variable.  This is for void-reference purposes.
//
//  Programmer: Hank Childs
//  Creation:   September 24, 2002
//
// ****************************************************************************

void
avtMixedVariable::Destruct(void *p)
{
    avtMixedVariable *mv = (avtMixedVariable *) p;
    if (mv != NULL)
    {
        delete mv;
    }
}


