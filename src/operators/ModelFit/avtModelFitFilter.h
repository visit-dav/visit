// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtModelFitFilter.h
// ************************************************************************* //

#ifndef AVT_MODELFIT_FILTER_H
#define AVT_MODELFIT_FILTER_H

#define BINS 100

#include <avtPluginDataTreeIterator.h>

#include <ModelFitAtts.h>

#include <vtkPointData.h>
#include <vtkCellData.h>

class vtkDataSet;

typedef struct histogram{
    double *range_ends;
    int count;
    int mod_number;
} histogram;

typedef struct variableStats{
    int    num_points;
    char   *name;
    double sum;
    double average;
    double max;
    double min;
    double non_zero_min;
    double variance;
    double scaled_variance;
    bool   calculate_stats;
    bool   calculate_hists;
    histogram hist;
} variableStats;


// ****************************************************************************
//  Class: avtModelFitFilter
//
//  Purpose:
//      A plugin operator for ModelFit.
//
//  Programmer: rob -- generated by xml2avt
//  Creation:   Wed Feb 3 16:15:29 PST 2010
//
//  Modifications:
//    Eric Brugger, Wed Jul 30 19:34:09 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtModelFitFilter : public avtPluginDataTreeIterator
{
  public:
                         avtModelFitFilter();
    virtual             ~avtModelFitFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtModelFitFilter"; };
    virtual const char  *GetDescription(void)
                             { return "ModelFit"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    ModelFitAtts   atts;

    virtual      avtDataRepresentation *ExecuteData(avtDataRepresentation *);
    virtual      avtContract_p ModifyContract(avtContract_p);
    virtual      void          UpdateDataObjectInfo();
    virtual      void          PostExecute();

    void   calculateVariableStats();
    void   createVS(char *);
    void   grabOnePoint       (doubleVector *, int, 
        int, int, int, vtkPointData *, vtkCellData *);
    void   findMatches        (doubleVector *, intVector *, 
        doubleVector, int, int, int, int);
    double calculateDistance  (doubleVector, int);
    double spaceConvert (double, size_t, int);
    double convertToVariableSpace(double, size_t, int);

    avtCentering             Centering;
    std::vector<vtkDataSet        *>out_ds;
    std::vector<variableStats     *>VS;
    //vector<relationshipStats *>RS;
    int        activeTs;
    int        num_relats;
    int        numTimesteps;
    int        entire_data;
    char       *new_pipelineVar;
    const char *pipelineVar;

    int        max_blk_num;
    int        max_blk_size;
    int        total_blk_num;
};


#endif
