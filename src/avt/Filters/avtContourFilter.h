// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtContourFilter.h                            //
// ************************************************************************* //

#ifndef AVT_CONTOUR_FILTER_H
#define AVT_CONTOUR_FILTER_H

#include <filters_exports.h>

#include <avtSIMODataTreeIterator.h>

#include <ContourOpAttributes.h>

#include <string>
#include <vector>


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
//    Made inherit from avtDomainTreeDataTreeIterator.  Added default constructor,
//    SetLevels method.
//
//    Kathleen Bonnell, Tue Apr 10 11:35:39 PDT 2001
//    Made inherit from avtSIMODataTreeIterator.
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001
//    Added string argument to Execute method. Added member isoLabels, to
//    hold string representation of computed isoValues.  Added method
//    CreateLabels.
//
//    Hank Childs, Mon Aug 30 09:03:38 PDT 2004
//    Do a better job of providing progress.  Added two data members, nnodes
//    and current_node to help with bookkeeping.
//
//    Hank Childs, Sun Mar  6 08:18:53 PST 2005
//    Removed "centering conversion module" data member.
//
//    Eric Brugger, Mon Jul 21 10:13:44 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Cameron Christensen, Tuesday, June 10, 2014
//    Modified the class to work with EAVL.
//
//    Eric Brugger, Thu Jan 14 08:46:42 PST 2016
//    Modified the class to work with VTKM.
//
//    Kathleen Biagas, Wed Jan 30 10:41:17 PST 2019
//    Removed EAVL support.
//
//    Kathleen Biagas, Wed July 14, 2021
//    Added ability to retrieve the actual iso values used for contouring.
//
// ****************************************************************************

class AVTFILTERS_API avtContourFilter : public avtSIMODataTreeIterator
{
  public:
                               avtContourFilter(const ContourOpAttributes &);

    virtual                   ~avtContourFilter();

    void                       ShouldCreateLabels(bool b)
                                    { shouldCreateLabels = b; }
    virtual const char        *GetType(void)  { return "avtContourFilter"; }
    virtual const char        *GetDescription(void) { return "Contouring"; }
    virtual void               ReleaseData(void);

    void                       GetIsoValues(std::vector<double> &v);

  protected:
    bool                       stillNeedExtents;
    bool                       shouldCreateLabels;

    ContourOpAttributes        atts;
    bool                       logFlag;
    bool                       percentFlag;
    int                        nLevels;
    std::vector<double>        isoValues;
    std::vector<std::string>   isoLabels;

    int                        nnodes;
    int                        current_node;
    int                        timeslice_index;

    virtual avtContract_p
                               ModifyContract(avtContract_p);
    virtual avtDataTree_p      ExecuteDataTree(avtDataRepresentation *);

    virtual void               UpdateDataObjectInfo(void);
    virtual void               PreExecute(void);

    void                       CreatePercentValues(double, double);
    void                       CreateNIsoValues(double, double);
    void                       SetIsoValues(double, double);
    void                       CreateLabels(void);

 private:
    avtDataTree_p      ExecuteDataTree_VTK(avtDataRepresentation *);
    avtDataTree_p      ExecuteDataTree_VTKM(avtDataRepresentation *);

};


#endif


