// ************************************************************************* //
//                             avtContourFilter.h                            //
// ************************************************************************* //

#ifndef AVT_CONTOUR_FILTER_H
#define AVT_CONTOUR_FILTER_H

#include <pipeline_exports.h>

#include <ContourOpAttributes.h>
#include <avtDataTreeStreamer.h>
#include <vector>

using std::string;

class vtkCellDataToPointData;
class vtkVisItContourFilter;
class vtkDataSet;


// ****************************************************************************
//  Class: avtContourFilter
//
//  Purpose:
//      A filter that performs a contour on domains of an avtDataSet.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug 22 15:40:45 PDT 2000
//    Added data member cd2pd so cell-centered data can also be handled.
//
//    Jeremy Meredith, Tue Sep 19 22:29:06 PDT 2000
//    Added data member levels, made constructor initialize using
//    raw levels, and added Equivalent method.
//
//    Jeremy Meredith, Thu Sep 28 12:50:55 PDT 2000
//    Removed CreateOutputDatasets.  Changed interface to ExecuteDomain.
//
//    Kathleen Bonnell, Fri Feb 16 13:28:57 PST 2001 
//    Made inherit from avtDomainTreeStreamer.  Added default constructor,
//    SetLevels method.
//
//    Kathleen Bonnell, Tue Apr 10 11:35:39 PDT 2001 
//    Made inherit from avtDataTreeStreamer.  
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001 
//    Added string argument to Execute method. Added member isoLabels, to
//    hold string representation of computed isoValues.  Added method
//    CreateLabels.
//
// ****************************************************************************

class PIPELINE_API avtContourFilter : public avtDataTreeStreamer
{
  public:
                               avtContourFilter(const ContourOpAttributes &);

    virtual                   ~avtContourFilter();

    void                       ShouldCreateLabels(bool b)
                                    { shouldCreateLabels = b; };
    virtual const char        *GetType(void)  { return "avtContourFilter"; };
    virtual const char        *GetDescription(void) { return "Contouring"; };
    virtual void               ReleaseData(void);

  protected:
    vtkVisItContourFilter     *cf;
    vtkCellDataToPointData    *cd2pd;

    bool                       stillNeedExtents;
    bool                       shouldCreateLabels;

    ContourOpAttributes        atts;
    bool                       logFlag; 
    bool                       percentFlag; 
    int                        nLevels;
    std::vector<double>        isoValues;
    std::vector<std::string>   isoLabels;

    virtual avtPipelineSpecification_p
                               PerformRestriction(avtPipelineSpecification_p);
    virtual avtDataTree_p      ExecuteDataTree(vtkDataSet *, int, string);

    virtual void               RefashionDataObjectInfo(void);
    virtual void               PreExecute(void);

    void                       CreatePercentValues(double, double);
    void                       CreateNIsoValues(double, double);
    void                       SetIsoValues(double, double);
    void                       CreateLabels(void);
};


#endif


