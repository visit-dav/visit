// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtLineScanQuery.h                            //
// ************************************************************************* //

#ifndef AVT_LINE_SCAN_QUERY_H
#define AVT_LINE_SCAN_QUERY_H

#include <query_exports.h>

#include <avtDatasetQuery.h>

#include <string>

class     vtkPolyData;
class     vtkIntArray;
class     avtLineScanFilter;


// ****************************************************************************
//  Class: avtLineScanQuery
//
//  Purpose:
//    An abstract query that provides a common base type for queries that
//    operate on line scans.  This query also provides many methods and 
//    services that are useful to concrete line scan queries.
//
//  Programmer: Hank Childs
//  Creation:   August 2, 2006
//
//  Modifications:
//    Kathleen Biagas, Tue Jun 21 11:38:19 PDT 2011
//    Added SetInputParams.
//
//    Kathleen Biagas, Fri Jul 15 16:08:31 PDT 2011
//    Added GetDefaultInputParams.
//
// ****************************************************************************

class QUERY_API avtLineScanQuery : public avtDatasetQuery
{
  public:
                               avtLineScanQuery();
    virtual                   ~avtLineScanQuery();

    virtual const char        *GetType(void)  { return "avtLineScanQuery"; };
    virtual const char        *GetDescription(void)
                                           { return "Querying line scans."; };

    virtual void               SetInputParams(const MapNode &);
    static  void               GetDefaultInputParams(MapNode &);

    void                       SetNumberOfLines(int nl) { numLines = nl; };
    void                       SetNumberOfBins(int nb)  { numBins  = nb; };
    void                       SetRange(double r1, double r2) 
                                { minLength = r1; maxLength = r2; };

    virtual int                GetNFilters(void);


  protected:
    int                        numBins;
    int                        numLines;
    double                     minLength;
    double                     maxLength;
    int                        numLinesPerIteration;
    std::string                varname;

    const double              *lines;  //Set only during Execute.  Stores data for use by base classes.

    virtual void               PreExecute(void);
    virtual void               Execute(vtkDataSet *, const int);
    virtual avtLineScanFilter *CreateLineScanFilter();

    int                        GetCellsForPoint(int ptId, vtkPolyData *pd, 
                                                vtkIntArray *lineids,int lineid,
                                                int &seg1, int &seg2);
    int                        WalkChain(vtkPolyData *pd, int ptId, int cellId, 
                                         std::vector<bool> &usedPoint,
                                         vtkIntArray *lineids, int lineid);
    void                       WalkChain1(vtkPolyData *pd, int ptId, int cellId,
                                          vtkIntArray *lineids, int lineid, 
                                          int &newPtId, int &newCellId);

  private:
    virtual void               Execute(avtDataTree_p);
    virtual void               ExecuteTree(avtDataTree_p);
    virtual void               ExecuteLineScan(vtkPolyData *) = 0;
};


#endif


