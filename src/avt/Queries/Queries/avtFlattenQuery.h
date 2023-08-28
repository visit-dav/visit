// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtFlattenQuery.h                              //
// ************************************************************************* //

#ifndef AVT_FLATTEN_QUERY_H
#define AVT_FLATTEN_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>

#include <memory>

class vtkDataset;

// ****************************************************************************
//  Class: avtFlattenQuery
//
//  Purpose:
//    A query that provides flattened field data
//
//  Programmer: Chris Laganella
//  Creation:   Tue Jan 11 17:21:22 EST 2022
//
//  Modifications:
//
// ****************************************************************************

class QUERY_API avtFlattenQuery : public avtDatasetQuery
{
  public:
                              avtFlattenQuery();
    virtual                  ~avtFlattenQuery();


    virtual const char       *GetType(void)
                              { return "avtFlattenQuery"; };
    virtual const char       *GetDescription(void)
                              { return "Provides flattened field data."; };

    virtual void              SetInputParams(const MapNode &);
    static void               GetDefaultInputParams(MapNode &);
    virtual void              GetSecondaryVars(stringVector &);

  protected:

    virtual void              VerifyInput(void);
    virtual void              PreExecute(void);
    virtual void              Execute(vtkDataSet *, const int);
    virtual void              Execute(avtDataTree_p);
    virtual void              SetOutputQueryAtts(QueryAttributes*, bool);
    void                      BuildOutputInfo(const intVector &varNComps,
                                              const intVector &varTypes,
                                              const long nodeSize,
                                              const long zoneSize,
                                              MapNode &outInfo);
    void                      WriteSharedMemory() const;

    static const int NODE_DATA;
    static const int ZONE_DATA;
    struct impl;
    std::unique_ptr<impl> pimpl;
};


#endif
