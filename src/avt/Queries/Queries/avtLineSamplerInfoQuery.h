// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtLineSamplerInfoQuery.h                       //
// ************************************************************************* //

#ifndef AVT_LINESAMPLER_INFO_QUERY_H
#define AVT_LINESAMPLER_INFO_QUERY_H
#include <query_exports.h>
#include <avtDatasetQuery.h>

#include <vector>


// ****************************************************************************
//  Class: avtLineSamplerInfoQuery
//
//  Purpose:
//      Query information on a line sampler operator.
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

class QUERY_API avtLineSamplerInfoQuery : public avtDatasetQuery
{
  public:
                              avtLineSamplerInfoQuery();
    virtual                  ~avtLineSamplerInfoQuery(); 

    virtual const char        *GetType(void) { return "avtLineSamplerInfoQuery"; };
    virtual const char        *GetDescription(void) 
                                  { return "LineSampler information"; };

    virtual void             SetInputParams(const MapNode &);
    static  void             GetDefaultInputParams(MapNode &);

    void                     SetDumpCoordinates(bool v) {dumpCoordinates = v;}
    void                     SetDumpValues(bool v) {dumpValues = v;}

  protected:
    virtual void             VerifyInput(void);
    virtual void             PreExecute(void);
    virtual void             PostExecute(void);
    virtual void             Execute(vtkDataSet *ds, const int chunk);

    bool dumpCoordinates;
    bool dumpValues;
    std::vector<float> lsData;
};

#endif

