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
//                             avtLineScanQuery.h                            //
// ************************************************************************* //

#ifndef AVT_LINE_SCAN_QUERY_H
#define AVT_LINE_SCAN_QUERY_H

#include <query_exports.h>

#include <avtDatasetQuery.h>

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
// ****************************************************************************

class QUERY_API avtLineScanQuery : public avtDatasetQuery
{
  public:
                               avtLineScanQuery();
    virtual                   ~avtLineScanQuery();

    virtual const char        *GetType(void)  { return "avtLineScanQuery"; };
    virtual const char        *GetDescription(void)
                                           { return "Querying line scans."; };

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


