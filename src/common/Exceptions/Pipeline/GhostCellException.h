// ************************************************************************* //
//                            GhostCellException.h                           //
// ************************************************************************* //

#ifndef GHOST_CELL_EXCEPTION_H
#define GHOST_CELL_EXCEPTION_H

#include <avtexception_exports.h>
#include <PipelineException.h>
#include <vector>

// ****************************************************************************
//  Class: GhostCellException
//
//  Purpose:
//      The exception that should be called when a ghost cell is encountered
//      but a real cell was expected. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 14, 2002 
//
// ****************************************************************************

class AVTEXCEPTION_API GhostCellException : public PipelineException
{
  public:
                    GhostCellException(int, char* m = NULL);
                    GhostCellException(const std::vector<int> &, const char* m = NULL);
    virtual        ~GhostCellException() VISIT_THROW_NOTHING {;};
};


#endif


