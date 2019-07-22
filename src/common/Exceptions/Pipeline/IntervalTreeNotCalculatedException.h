// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                     IntervalTreeNotCalculateException.h                   //
// ************************************************************************* //

#ifndef INTERVAL_TREE_NOT_CALCULATED_EXCEPTION_H
#define INTERVAL_TREE_NOT_CALCULATED_EXCEPTION_H
#include <avtexception_exports.h>


#include <PipelineException.h>


// ****************************************************************************
//  Class: IntervalTreeNotCalculatedException
//
//  Purpose:
//      The exception that is thrown when the interval tree is accessed 
//      although it has not been fully calculated.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2000
//
// ****************************************************************************

class AVTEXCEPTION_API IntervalTreeNotCalculatedException : public PipelineException
{
  public:
               IntervalTreeNotCalculatedException();
    virtual   ~IntervalTreeNotCalculatedException() VISIT_THROW_NOTHING {;};
};


#endif


