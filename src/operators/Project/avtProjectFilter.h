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
//  Modifications:
//    Jeremy Meredith, Thu Sep  9 16:44:38 PDT 2004
//    Added PerformRestriction so pick could get the node/zone numbers if
//    needed.  Force a rectilinear dataset to always project into a
//    curvilinear one.  Added projection of vectors.
//
//    Hank Childs, Thu Jan 20 10:27:29 PST 2005
//    Added extents calculation in PostExecute.
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

    virtual vtkDataSet         *ExecuteData(vtkDataSet *, int, std::string);
    virtual void                PostExecute(void);
    virtual void                RefashionDataObjectInfo(void);
    avtPipelineSpecification_p  PerformRestriction(avtPipelineSpecification_p);

  private:
    void                ProjectPoint(float &x, float &y, float &z);
    vtkPointSet        *ProjectPointSet(vtkPointSet*);
    vtkPointSet        *ProjectRectilinearGrid(vtkRectilinearGrid*);
    void                ProjectVectors(vtkDataSet*,vtkDataSet*,
                                       vtkDataArray*,vtkDataArray*,bool);
};


#endif
