// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtIntegralCurveInfoQuery.h                       //
// ************************************************************************* //

#ifndef AVT_INTEGRAL_CURVE_INFO_QUERY_H
#define AVT_INTEGRAL_CURVE_INFO_QUERY_H
#include <query_exports.h>
#include <avtDatasetQuery.h>

#include <vector>


// ****************************************************************************
//  Class: avtIntegralCurveInfoQuery
//
//  Purpose:
//      Query information on an integral curve operator.
//
//  Programmer: Dave Pugmire
//  Creation:   9 Nov 2010
//
//  Modifications:
//    Kathleen Biagas, Fri Jun 17 15:39:21 PDT 2011
//    Added SetInputParams.
//
//    Kathleen Biagas, Fri Jul 15 16:34:54 PDT 2011
//    Add GetDefaultInputParams.
//
// ****************************************************************************

class QUERY_API avtIntegralCurveInfoQuery : public avtDatasetQuery
{
  public:
                              avtIntegralCurveInfoQuery();
    virtual                  ~avtIntegralCurveInfoQuery(); 

    virtual const char        *GetType(void) { return "avtIntegralCurveInfoQuery"; };
    virtual const char        *GetDescription(void) 
                                  { return "Integral Curve Information"; };

    virtual void             SetInputParams(const MapNode &);
    static  void             GetDefaultInputParams(MapNode &);

    void                     SetDumpIndex      (bool v) {dumpIndex = v;}
    void                     SetDumpCoordinates(bool v) {dumpCoordinates = v;}
    void                     SetDumpArcLength  (bool v) {dumpArcLength = v;}
    void                     SetDumpValues     (bool v) {dumpValues = v;}

  protected:
    virtual void             VerifyInput(void);
    virtual void             PreExecute(void);
    virtual void             PostExecute(void);
    virtual void             Execute(vtkDataSet *ds, const int chunk);

    bool dumpIndex;
    bool dumpCoordinates;
    bool dumpArcLength;
    bool dumpValues;

    std::vector<double> slData;
};

#endif

