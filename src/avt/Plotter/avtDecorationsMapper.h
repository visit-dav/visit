// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtDecorationsMapper.h                            //
// ************************************************************************* //

#ifndef AVT_DECORATIONS_MAPPER_H
#define AVT_DECORATIONS_MAPPER_H

#include <plotter_exports.h>

#include <avtTerminatingDatasetSink.h>
#include <avtDrawable.h>
#include <avtLabelActor.h>



// ****************************************************************************
//  Class:  avtDecorationsMapper
//
//  Purpose:
//      This takes geometry and makes a decorations drawable.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   July 12, 2002 
//
// ****************************************************************************

class PLOTTER_API avtDecorationsMapper : public avtTerminatingDatasetSink
{
  public:
                                 avtDecorationsMapper();
    virtual                     ~avtDecorationsMapper();

    avtDrawable_p                GetDrawable();

  protected:
    avtDrawable_p                drawable;

    std::vector<avtLabelActor_p> actors;

    void                         ClearSelf(void);
    void                         SetUpMappers(void);

    virtual void                 ChangedInput(void);
    virtual void                 CustomizeMappers(void) = 0;
    virtual void                 MapperChangedInput(void);
    virtual void                 InputIsReady(void);

    virtual void                 SetUpFilters(int);
    virtual void                 SetDatasetInput(vtkDataSet *, int) = 0;
};


#endif


