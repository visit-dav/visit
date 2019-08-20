// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Modifications:
//
//    Tom Fogal, Sat Jun 14 18:48:35 EDT 2008
//    Constant constructor argument avoids implicit conversion issues; made it
//    a reference too, to avoid copying strings frequently.
//
// **************************************************************************** 

class AVTEXCEPTION_API ImproperUseException : public PipelineException
{
  public:
                          ImproperUseException(const std::string & = "");
    virtual              ~ImproperUseException() VISIT_THROW_NOTHING {;};
};

#endif
