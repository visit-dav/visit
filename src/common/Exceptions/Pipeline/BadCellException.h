// ************************************************************************* //
//                              BadCellException.h                           //
// ************************************************************************* //

#ifndef BAD_CELL_EXCEPTION_H
#define BAD_CELL_EXCEPTION_H

#include <avtexception_exports.h>

#include <PipelineException.h>
#include <vector>

// ****************************************************************************
//  Class: BadCellException
//
//  Purpose:
//      The exception that should be called when a bad cell is encountered.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Aug 16 11:14:25 PDT 2002
//    Add constructor that accepts an int vector.
//
//    Kathleen Bonnell, Tue Jan 18 19:36:33 PST 2005 
//    Add constructor that accepts a string.
//
// ****************************************************************************

class AVTEXCEPTION_API BadCellException : public PipelineException
{
  public:
                    BadCellException(int, int);
                    BadCellException(int, std::string &);
                    BadCellException(std::vector<int> &);
    virtual        ~BadCellException() VISIT_THROW_NOTHING {;};
};


#endif


