// ************************************************************************* //
//  File: avtProjectFilter.h
// ************************************************************************* //

#ifndef AVT_Project_FILTER_H
#define AVT_Project_FILTER_H


#include <avtPluginStreamer.h>
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
// ****************************************************************************

class avtProjectFilter : public avtPluginStreamer
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

    virtual vtkDataSet  *ExecuteData(vtkDataSet *, int, std::string);
    virtual void         RefashionDataObjectInfo(void);
    void                 ProjectPoint(float &x, float &y, float &z);
    vtkDataSet          *ProjectPointSet(vtkPointSet*);
    vtkDataSet          *ProjectRectilinearGrid(vtkRectilinearGrid*);
    vtkDataSet          *ProjectRectilinearToRectilinear(vtkRectilinearGrid*);
    vtkDataSet          *ProjectRectilinearToCurvilinear(vtkRectilinearGrid*);
};


#endif
