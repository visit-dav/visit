// ************************************************************************* //
//                           avtExpressionStreamer.h                         //
// ************************************************************************* //

#ifndef AVT_EXPRESSION_STREAMER_H
#define AVT_EXPRESSION_STREAMER_H

#include <expression_exports.h>

#include <string>

#include <avtExpressionFilter.h>
#include <avtStreamer.h>


// ****************************************************************************
//  Class: avtExpressionStreamer
//
//  Purpose:
//      This is an abstract type that allows derived types to create 
//      expressions one VTK dataset at a time.
//
//  Notes:      The streaming functionality used to be part of 
//              avtExpressionFilter.  The creation date corresponds to when the
//              class was split.
//
//  Programmer: Hank Childs
//  Creation:   December 27, 2004
//
// ****************************************************************************

class EXPRESSION_API avtExpressionStreamer : virtual public avtStreamer, 
                                             virtual public avtExpressionFilter
{
  public:
                             avtExpressionStreamer();
    virtual                 ~avtExpressionStreamer();

  protected:
    std::string              currentDomainsLabel;
    int                      currentDomainsIndex;

    virtual vtkDataSet      *ExecuteData(vtkDataSet *, int, std::string);
    virtual vtkDataArray    *DeriveVariable(vtkDataSet *) = 0;
};


#endif


