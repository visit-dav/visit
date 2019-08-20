// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtShapeletDecompositionQuery.h                      //
// ************************************************************************* //

#ifndef AVT_SHAPELET_DECOMPOSITION_QUERY_H
#define AVT_SHAPELET_DECOMPOSITION_QUERY_H
#include <query_exports.h>
#include <avtDatasetQuery.h>
#include <avtShapeletDecompResult.h>

#include <string>

class vtkDataSet;
class vtkRectilinearGrid;

// ****************************************************************************
//  Class: avtShapeletDecompositionQuery
//
//  Purpose:
//      Base class for queryies based on shapelet decomposition.
//      Given nmax and beta, calculates shapelet coefficents of scalar
//      from a rectilnear dataset.
//
//  Programmer: Cyrus Harrison
//  Creation:   December 14, 2007 
//
//  Modifications:
//    Kathleen Biagas, Mon Jun 20 09:00:09 PDT 2011
//    Added SetInputParams.
//
//    Kathleen Biagas, Fri Jul 15 16:30:09 PDT 2011
//    Added GetDefaultInputParams.
//
// ****************************************************************************

class QUERY_API avtShapeletDecompositionQuery: public avtDatasetQuery
{
  public:
                               avtShapeletDecompositionQuery();
    virtual                   ~avtShapeletDecompositionQuery();

    virtual const char        *GetType(void)
                                   { return "avtShapeletDecompositionQuery"; };
    virtual const char        *GetDescription(void)
                             { return "Constructing Shapelet Decomposition"; };

    virtual void               SetInputParams(const MapNode &);
    static  void               GetDefaultInputParams(MapNode &);
    
    void                       SetBeta(double val) { beta = val;}
    double                     GetBeta() { return beta;}
    
    void                       SetNMax(int val) { nmax = val;}
    int                        GetNMax() { return nmax;}
    
    void                       SetDecompOutputFileName(const std::string &val)
                                {decompOutputFileName=val;}
    void                       SetRecompOutputFileName(const std::string &val)
                                {recompOutputFileName=val;}
                           
  protected:
    virtual void               Execute(vtkDataSet *, const int);
    virtual void               PreExecute(void);
    virtual void               PostExecute(void);
  private:
    double                     ComputeError(vtkRectilinearGrid *a,
                                            vtkRectilinearGrid *b,
                                            const std::string &var_name);

    int                        nmax;
    double                     beta;

    avtShapeletDecompResult   *decompResult;
    double                     recompError;
    
    std::string                decompOutputFileName;
    std::string                recompOutputFileName;
    
};


#endif


