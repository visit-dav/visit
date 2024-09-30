// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtCurveConstructorFilter.h                        //
// ************************************************************************* //

#ifndef AVT_CURVE_CONSTRUCTOR_FILTER_H
#define AVT_CURVE_CONSTRUCTOR_FILTER_H
#include <filters_exports.h>

#include <avtDatasetToDatasetFilter.h>
#include <vectortypes.h>


// ****************************************************************************
//  Class: avtCurveConstructorFilter
//
//  Purpose:
//      A filter that will construct one uniform curve from fragments of
//      curves.
//
//  Programmer: Kathleen Bonnell
//  Creation:   Sat Apr 20 13:01:58 PST 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Jul 12 16:53:11 PDT 2002
//    Removed vtk filters associated with label-creation.  Now handled by
//    the plot.
//
//    Kathleen Bonnell, Mon Dec 23 08:23:26 PST 2002
//    Added UpdateDataObjectInfo.
//
//    Hank Childs, Fri Oct  3 11:10:29 PDT 2003
//    Moved from /plots/Curve.  Renamed to CurveConstructorFilter.
//
//    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006
//    Add PostExecute and outputArray.
//
//    Kathleen Bonnell, Thu Mar 19 17:42:14 PDT 2009
//    Added 'ForceConstruction', needed by curve queries.
//
//    Kathleen Bonnell, Mon Mar 23 09:53:17 PDT 2009
//    Removed 'ForceConstruction'.
//
//    Kathleen Bonnell, Thu Feb 17 09:18:43 PST 2011
//    Added CreateSingeOutput method.
//
//    Kathleen Biagas, Tue Dec 19, 2023
//    Add a MapNode to store multi-curve output for adding to PlotInformation.
//    Add vname and count arguments to CreateSingleOutput.
//
// ****************************************************************************

class AVTFILTERS_API avtCurveConstructorFilter : public avtDatasetToDatasetFilter
{
  public:
                              avtCurveConstructorFilter();
    virtual                  ~avtCurveConstructorFilter();

    virtual const char       *GetType(void)
                                       { return "avtCurveConstructorFilter"; }
    virtual const char       *GetDescription(void)
                                  { return "Constructing Curve"; }

  protected:
    doubleVector              outputArray;
    MapNode                   outputInfo;
    vtkDataSet               *CreateSingleOutput(avtDataTree_p inTree, const std::string &vname, const int count);

    virtual void              Execute(void);
    virtual void              PostExecute(void);
    virtual void              VerifyInput(void);
    avtContract_p             ModifyContract(avtContract_p spec);
    virtual void              UpdateDataObjectInfo(void);
};


#endif


