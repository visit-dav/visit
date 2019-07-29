// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtDataObjectToDatasetFilter.h                      //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_TO_DATASET_FILTER_H
#define AVT_DATA_OBJECT_TO_DATASET_FILTER_H

#include <pipeline_exports.h>

#include <avtFilter.h>
#include <avtDatasetSource.h>


// ****************************************************************************
//  Class: avtDataObjectToDatasetFilter
//
//  Purpose:
//      A filter that takes in a data object as input and has a dataset as
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
//    Hank Childs, Tue Jul 27 08:41:21 PDT 2004
//    Convert unstructured data that could be stored as poly data to poly
//    data.
//
//    Cyrus Harrison, Sat Aug 11 19:53:13 PDT 2007
//    Added support for vtk-debug mode.
//    
// ****************************************************************************

class PIPELINE_API avtDataObjectToDatasetFilter
    : virtual public avtFilter, virtual public avtDatasetSource
{
  public:
                       avtDataObjectToDatasetFilter();
    virtual           ~avtDataObjectToDatasetFilter();

    void               OutputSetActiveVariable(const char *);

    virtual void       PostExecute(void);
    
    static void        SetVTKDebugMode(bool on){vtkDebugMode = on;}
 private:
    static bool        vtkDebugMode;
};


#endif


