// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtConnComponentsSummaryQuery.h                     //
// ************************************************************************* //

#ifndef AVT_CONN_COMPONENTS_SUMMARY_QUERY_H
#define AVT_CONN_COMPONENTS_SUMMARY_QUERY_H
#include <query_exports.h>

#include <avtDatasetQuery.h>
#include <avtConnComponentsQuery.h>

#include <string>
#include <vector>

class avtEdgeLength;
class avtRevolvedVolume;
class avtVMetricArea;
class avtVMetricVolume;

class vtkDataSet;


// ****************************************************************************
//  Class: avtConnComponentsSummaryQuery
//
//  Purpose:
//      Finds the connected components of a mesh and creates a point dataset 
//      with per component quantities. This dataset is saved in as an Xmdv
//      file.  
// 
//      Component Fields:
//       x,y,z              Component centroid location
//       comp_length        Component length (if 1D topo data)
//       comp_area          Component area   (if 2D topo data)
//       comp_volume        Component volume (if 3D topo or revolved volume data)
//       comp_sum           Component variable sum 
//       comp_weighted_sum  Component weighted variable sum
//       comp_bb_{x,y,z}_{min,max} Component bounding box values.
//
//
//  Programmer: Cyrus Harrison
//  Creation:   March 1, 2007 
//
//  Modifications:
//    Cyrus Harrison, Mon Mar 30 11:34:27 PDT 2009
//    Added support for per component bounding boxes.
//
//    Cyrus Harrison, Wed Dec 22 14:53:33 PST 2010
//    Added output for the number of processors each component spans.
//
//    Cyrus Harrison, Mon Jun  6 17:02:12 PDT 2011
//    Added lengthFilter.
//
//    Kathleen Biagas, Tue Jun 21 10:42:07 PDT 2011
//    Added SetInputParams.
//
//    Kathleen Biagas, Fri Jul 15 16:05:35 PDT 2011
//    Added GetDefaultInputParams.
//
//    Cyrus Harrison, Wed Jul 16 13:54:00 PDT 2014
//    Added PrepareMapNodeResult.
//
// ****************************************************************************

class QUERY_API avtConnComponentsSummaryQuery 
: public avtConnComponentsQuery
{
  public:
                               avtConnComponentsSummaryQuery();
    virtual                   ~avtConnComponentsSummaryQuery();

    virtual const char        *GetType(void)
                         { return "avtConnComponentsSummaryQuery"; };
    virtual const char        *GetDescription(void)
                         { return "Generating connected components summary.";};

    virtual void               SetInputParams(const MapNode &);
    static  void               GetDefaultInputParams(MapNode &);

    void                       SetOutputFileName(const std::string &fname)
                                 {outputFileName = fname; }
  protected:

    std::string                outputFileName;

    avtEdgeLength             *lengthFilter;
    avtRevolvedVolume         *revolvedVolumeFilter;
    avtVMetricArea            *areaFilter;
    avtVMetricVolume          *volumeFilter;

    std::string                variableName;

    std::vector<int>           nCellsPerComp;
    std::vector<int>           nProcsPerComp;

    std::vector<double>        xCentroidPerComp;
    std::vector<double>        yCentroidPerComp;
    std::vector<double>        zCentroidPerComp;

    // for computing per component bounds
    std::vector<double>        xMinPerComp;
    std::vector<double>        xMaxPerComp;
    std::vector<double>        yMinPerComp;
    std::vector<double>        yMaxPerComp;
    std::vector<double>        zMinPerComp;
    std::vector<double>        zMaxPerComp;

    std::vector<double>        lengthPerComp;
    std::vector<double>        areaPerComp;
    std::vector<double>        volPerComp;

    std::vector<double>        sumPerComp;
    std::vector<double>        wsumPerComp;

    bool                       findLength;
    bool                       findArea;
    bool                       findVolume;

    virtual void               Execute(vtkDataSet *, const int);
    virtual void               PreExecute(void);
    virtual void               PostExecute(void);
    virtual avtDataObject_p    ApplyFilters(avtDataObject_p);
    virtual void               VerifyInput(void);

    void                       SaveComponentResults(std::string fname);
    void                       PrepareComponentResults(std::vector<double> &);
    void                       PrepareMapNodeResult(MapNode &result_node);
};


#endif



