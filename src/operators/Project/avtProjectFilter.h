// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtProjectFilter.h
// ************************************************************************* //

#ifndef AVT_Project_FILTER_H
#define AVT_Project_FILTER_H

#include <avtPluginDataTreeIterator.h>

#include <ProjectAttributes.h>

class vtkDataSet;
class vtkPointSet;
class vtkRectilinearGrid;


// ****************************************************************************
//  Class: avtProjectFilter
//
//  Purpose:
//      A plugin operator for Project.
//
//  Programmer: Jeremy Meredith
//  Creation:   Tue May 18 14:35:38 PST 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Sep  9 16:44:38 PDT 2004
//    Added ModifyContract so pick could get the node/zone numbers if
//    needed.  Force a rectilinear dataset to always project into a
//    curvilinear one.  Added projection of vectors.
//
//    Hank Childs, Thu Jan 20 10:27:29 PST 2005
//    Added extents calculation in PostExecute.
//
//    Jeremy Meredith, Thu Apr  1 14:41:33 EDT 2010
//    Added double precision version of projection for increased
//    accuracy.
//
//    Eric Brugger, Thu Jul 31 14:42:52 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtProjectFilter : public avtPluginDataTreeIterator
{
  public:
                        avtProjectFilter();
    virtual            ~avtProjectFilter();

    static avtFilter   *Create();

    virtual const char *GetType(void)  { return "avtProjectFilter"; }
    virtual const char *GetDescription(void) { return "Project"; }

    virtual void        SetAtts(const AttributeGroup*);
    virtual bool        Equivalent(const AttributeGroup*);

  protected:
    ProjectAttributes   atts;

    virtual avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual void                PostExecute(void);
    virtual void                UpdateDataObjectInfo(void);
    avtContract_p  ModifyContract(avtContract_p);

  private:
    void                ProjectPoint(float &x, float &y, float &z);
    void                ProjectPoint(double &x, double &y, double &z);
    vtkPointSet        *ProjectPointSet(vtkPointSet*);
    vtkPointSet        *ProjectRectilinearGrid(vtkRectilinearGrid*);

    template <class T> 
    void                ProjectVectors(vtkDataSet*,vtkDataSet*,
                                       vtkDataArray*,vtkDataArray*,bool);
};


#endif
