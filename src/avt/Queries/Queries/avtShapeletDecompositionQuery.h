/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
                                            const string &var_name);

    int                        nmax;
    double                     beta;

    avtShapeletDecompResult   *decompResult;
    double                     recompError;
    
    std::string                decompOutputFileName;
    std::string                recompOutputFileName;
    
};


#endif


