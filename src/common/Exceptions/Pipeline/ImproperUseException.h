// ************************************************************************* //
//                           ImproperUseException.h                          //
// ************************************************************************* //

#ifndef IMPROPER_USE_EXCEPTION_H
#define IMPROPER_USE_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: ImproperUseException
//
//  Purpose:
//      Thrown when a pipeline object is used improperly.  This is typically
//      only when trying to set an avtDataset as input to a vtk-to-avt-dataset
//      filter or getting the avtDataset output from a avt-to-vtk-dataset
//      filter.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
// **************************************************************************** 

class AVTEXCEPTION_API ImproperUseException : public PipelineException
{
  public:
                          ImproperUseException(std::string = "");
    virtual              ~ImproperUseException() VISIT_THROW_NOTHING {;};
};


#endif


