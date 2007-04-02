// ************************************************************************* //
//                            avtFacelistFilter.h                            //
// ************************************************************************* //

#ifndef AVT_FACELIST_FILTER_H
#define AVT_FACELIST_FILTER_H

#include <filters_exports.h>

#include <avtStreamer.h>


class   vtkRectilinearGridFacelistFilter;
class   vtkStructuredGridFacelistFilter;
class   vtkUnstructuredGridFacelistFilter;
class   vtkPolyData;

class   avtMultiFacelist;


// ****************************************************************************
//  Class: avtFacelistFilter
//
//  Purpose:
//      A filter that takes determines the facelist and outputs it as polydata.
//
//  Programmer: Hank Childs
//  Creation:   October 26, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Apr 12 10:49:10 PDT 2001
//    Renamed ExecuteDomain as ExecuteData.
//
//    Hank Childs, Thu Sep  6 11:14:38 PDT 2001
//    Removed logic for preventing dynamic load balancing.
//
//    Eric Brugger, Wed Jan 23 15:18:39 PST 2002
//    I modified the class to use vtkUnstructuredGridFacelistFilter instead
//    of vtkGeometryFilter for unstructured grids.
//
//    Jeremy Meredith, Tue Jul  9 14:00:32 PDT 2002
//    Added the "create3DCellNumbers" flag.
//
//    Hank Childs, Sun Aug 18 11:18:20 PDT 2002
//    Make special accomodations for meshes that are made up of disjoint
//    elements.
//
//    Hank Childs, Tue Sep 10 12:36:42 PDT 2002
//    Redefine ReleaseData.
//
//    Hank Childs, Wed Oct  2 16:59:10 PDT 2002
//    Removed unused data member f2d.
//
//    Hank Childs, Wed Aug 11 09:46:53 PDT 2004
//    Added PerformRestriction.
//
//    Kathleen Bonnell, Fri Feb 18 11:13:16 PST 2005 
//    Added ConvertToPolys. 
//
//    Hank Childs, Fri Mar 11 08:07:26 PST 2005
//    Remove data member filters.  Also remove ReleaseData.
//
//    Hank Childs, Fri Sep 23 10:48:36 PDT 2005
//    Add a flag that will cause edge list generation for 2D datasets.
//
// ****************************************************************************

class AVTFILTERS_API avtFacelistFilter : public avtStreamer
{
  public:
                                         avtFacelistFilter();
    virtual                             ~avtFacelistFilter();

    virtual const char                  *GetType(void)
                                               { return "avtFacelistFilter"; };
    virtual const char                  *GetDescription(void)
                                     { return "Calculating external faces"; };

    void                                 SetCreate3DCellNumbers(bool);
    void                                 SetForceFaceConsolidation(bool);
    void                                 SetCreateEdgeListFor2DDatasets(bool);

  protected:
    bool                                 useFacelists;
    bool                                 create3DCellNumbers;
    bool                                 createEdgeListFor2DDatasets;
    int                                  forceFaceConsolidation;

    virtual vtkDataSet                  *ExecuteData(vtkDataSet *, int,
                                                     std::string);
    vtkDataSet                          *Take2DFaces(vtkDataSet *);
    vtkDataSet                          *FindEdges(vtkDataSet *);
    vtkDataSet                          *Take3DFaces(vtkDataSet *, int);
    vtkDataSet                          *TakeFacesForDisjointElementMesh(
                                                            vtkDataSet *, int);
    vtkDataSet                          *ConvertToPolys(vtkDataSet *, int);

    virtual void                         InitializeFilter(void);

    virtual void                         RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p   PerformRestriction(
                                                   avtPipelineSpecification_p);
};


#endif


