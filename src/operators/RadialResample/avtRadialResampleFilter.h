// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtRadialResampleFilter.h
// ************************************************************************* //

#ifndef AVT_RadialResample_FILTER_H
#define AVT_RadialResample_FILTER_H


#include <avtPluginDataTreeIterator.h>
#include <RadialResampleAttributes.h>
#include <avtPluginDatasetToDatasetFilter.h>

#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSet.h>


// ****************************************************************************
//  Class: avtRadialResampleFilter
//
//  Purpose:
//      A plugin operator for RadialResample.
//
//  Programmer: Kevin Griffin
//  Creation:   Tue May 20 13:15:11 PST 2014
//
//  Modifications:
//
//    Kevin Griffin, Fri Mar 24 18:06:06 PDT 2017
//    Added ModifyContract and UpdateDataObjectInfo methods.
//
// ****************************************************************************

class avtRadialResampleFilter : public avtPluginDatasetToDatasetFilter
{
  public:
                         avtRadialResampleFilter();
    virtual             ~avtRadialResampleFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtRadialResampleFilter"; };
    virtual const char  *GetDescription(void) { return "Radial Resample"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);
    virtual avtContract_p   ModifyContract(avtContract_p);
    virtual void            UpdateDataObjectInfo(void);

  protected:
    RadialResampleAttributes   atts;
    std::string resampleVarName;
    
    bool GetBounds(double[6]);

    virtual void Execute();
    
    static vtkStructuredGrid    *CreateFast2DRadialGrid(int[3], const float *, float, float, float);
    static vtkUnstructuredGrid  *CreateNormal2DRadialGrid(int[3], const float *, float, float, float);
    static vtkDataSet           *Create2DRadialGrid(const double *, const float *, float, float, float, float, float, bool);
    
    static vtkStructuredGrid    *CreateFast3DRadialGrid(int[3], const float *, float, float, float, float, float);
    static vtkUnstructuredGrid  *CreateNormal3DRadialGrid(int[3], const float *, float, float, float, float, float);
    static vtkDataSet           *Create3DRadialGrid(const double *, const float *, float, float, float, float, float, float, float, float, bool);
};


#endif
