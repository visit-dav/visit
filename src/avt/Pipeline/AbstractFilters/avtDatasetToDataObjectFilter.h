// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtDatasetToDataObjectFilter.h                      //
// ************************************************************************* //

#ifndef AVT_DATASET_TO_DATA_OBJECT_FILTER_H
#define AVT_DATASET_TO_DATA_OBJECT_FILTER_H

#include <pipeline_exports.h>

class     vtkObject;

#include <avtFilter.h>
#include <avtDatasetSink.h>


// ****************************************************************************
//  Class: avtDatasetToDataObjectFilter
//
//  Purpose:
//      A filter that takes in a dataset as input and has a data object as
//      output.
//
//  Programmer: Hank Childs
//  Creation:   May 31, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Fri Dec  3 14:28:02 PST 2004
//    Added variable name argument to SearchDataForDataExtents.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Hank Childs, Sun Nov 28 06:19:25 PST 2010
//    Add methods for caching VTK objects in the database.
//
//    Hank Childs, Tue Nov 30 20:38:36 PST 2010
//    Add method SearchDataForSpatialExtents.
//
// ****************************************************************************

class PIPELINE_API avtDatasetToDataObjectFilter
    : virtual public avtFilter, virtual public avtDatasetSink
{
  public:
                       avtDatasetToDataObjectFilter();
    virtual           ~avtDatasetToDataObjectFilter();

  protected:
    //                 Note that these variables are only used when an active
    //                 variable is set.
    bool               activeVariableIsPointData;
    bool               hasPointVars;
    bool               hasCellVars;

    void               InputSetActiveVariable(const char *);
    virtual void       SearchDataForDataExtents(double *, const char *);
    virtual void       SearchDataForSpatialExtents(double *);
    virtual void       PreExecute(void);

    vtkObject         *FetchArbitraryVTKObject(int dependencies, const char *name, int dom, 
                                               int ts, const char *type);
    void               StoreArbitraryVTKObject(int dependencies, const char *name, int dom, 
                                               int ts, const char *type, vtkObject *);
};


#endif


