// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtSubsetBlockMergeFilter.h                         //
// ************************************************************************* //

#ifndef AVT_SUBSET_BLOCKMERGE_FILTER_H
#define AVT_SUBSET_BLOCKMERGE_FILTER_H

#include <filters_exports.h>

#include <avtDatasetToDatasetFilter.h>


struct BlockIdDatasetPair
{
    std::string blockId;
    vtkDataSet *dataSet;
};

class vtkAppendPolyData;

// ****************************************************************************
//  Class: avtSubsetBlockMergeFilter
//
//  Purpose: Merge all datasets of the same block into one dataset
//
//
//  Programmer: Kevin Griffin
//  Creation:   October 10, 2014
//
//  Modifications:
//
// ****************************************************************************

class AVTFILTERS_API avtSubsetBlockMergeFilter : public avtDatasetToDatasetFilter
{
  public:
                             avtSubsetBlockMergeFilter();
    virtual                 ~avtSubsetBlockMergeFilter();

    virtual const char      *GetType(void) { return "avtSubsetBlockMergeFilter"; };
    virtual const char      *GetDescription(void) { return "Merge all datasets of the same block into one dataset"; };


  protected:
    virtual void            Execute();
    virtual void            PostExecute();

  private:
    void                    AddDatasetToMap(std::map<int, std::vector<BlockIdDatasetPair> > &, vtkDataSet *, const std::string);
    int                     GetIndexFromBlockId(const std::string, const std::vector<std::string> &) const;
    int                     GetProcessorIdFromBlockId(const int) const;
    avtDataTree_p           CreateOutputDataTree(std::map<std::string, vtkAppendPolyData *> &);
    void                    Send(std::map<int, std::vector<BlockIdDatasetPair> > &, const int);
    void                    Receive(std::map<std::string, vtkAppendPolyData *> &);
};

#endif
