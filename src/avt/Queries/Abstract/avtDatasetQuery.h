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
//                              avtDatasetQuery.h                            //
// ************************************************************************* //

#ifndef AVT_DATASET_QUERY_H
#define AVT_DATASET_QUERY_H
#include <query_exports.h>


#include <avtDataObjectQuery.h>
#include <avtDatasetSink.h>
#include <QueryAttributes.h>
#include <string.h>

class vtkDataSet;

// ****************************************************************************
//  Class: avtDatasetQuery
//
//  Purpose:
//      This is a data object query whose input is a dataset.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 12, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Made queryAtts a protected member so that derived types may have access
//    to the atts used to perform the query.  Added private Execute method
//    that operates on a data tree.  Added private members totalNodes and
//    currentNode for progress-tracking.  Removed un-used SetAtts method.
//
//    Jeremy Meredith, Fri Apr 11 10:04:16 PDT 2003
//    Added a const to SetMessage so c-strings would be accepted.
//
//    Jeremy Meredith, Thu Apr 17 12:55:25 PDT 2003
//    Made some methods visible/virtual to subclasses so they could override
//    them. This was necessary so I could create the avtTwoPassDatasetQuery.
//
//    Kathleen Bonnell,  Fri Jul 11 16:17:12 PDT 2003
//    Added value and Set/Get methods. Renamed Set/GetMessage to 
//    Set/GetResultMessage
//    
//    Kathleen Bonnell,  Wed Nov 12 18:18:19 PST 2003 
//    Made resValue a doubleVector.  Add more Get/Set methods so that queries
//    that only return 1 value don't need to be updated. 
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Kathleen Bonnell, Thu Apr  1 19:02:38 PST 2004 
//    Added method PerformQueryInTime.
//
//    Kathleen Bonnell, Fri Apr  2 08:51:17 PST 2004 
//    Changed args to PerformQueryInTime.
//
//    Kathleen Bonnell, Thu Jun 24 07:45:47 PDT 2004 
//    Added intVector and string arguments to PerformQueryInTime.
//
//    Kathleen Bonnell, Mon Jan  3 15:12:19 PST 2005 
//    Removed PerformQueryInTime. 
//
//    Cyrus Harrison, Tue Dec 18 08:16:10 PST 2007
//    Added GetXmlResult(), SetXmlResult() and xmlResult.
//
// ****************************************************************************

class QUERY_API avtDatasetQuery : public avtDataObjectQuery, 
                                     public avtDatasetSink
{
  public:
                              avtDatasetQuery();
    virtual                  ~avtDatasetQuery();


    virtual void             PerformQuery(QueryAttributes *);
    virtual std::string      GetResultMessage(void) { return resMsg; };
    virtual void             SetResultMessage(const std::string &m) 
                                 { resMsg = m; }; 

    virtual double           GetResultValue(const int i = 0); 
    virtual void             SetResultValue(const double &d, const int i = 0);

    virtual void             AddResultValue(const double &d)
                                 { resValue.push_back(d); };
    virtual doubleVector     GetResultValues(void) { return resValue; };
    virtual void             SetResultValues(const doubleVector &d)
                                 { resValue = d; };
    
    virtual std::string      GetXmlResult(void) { return xmlResult; };
    virtual void             SetXmlResult(const std::string &xml) 
                                 { xmlResult= xml; }; 
                                 
  protected:

    virtual void             PreExecute(void);
    virtual void             PostExecute(void);

    virtual void             Execute(vtkDataSet*, const int) = 0;
    virtual avtDataObject_p  ApplyFilters(avtDataObject_p);

    virtual void             GetSecondaryVars( std::vector<std::string> &outVars );
    
    QueryAttributes          queryAtts;

    int                      totalNodes;
    int                      currentNode;

  private:
    virtual void             Execute(avtDataTree_p);
    std::string              resMsg;
    doubleVector             resValue;
    std::string              xmlResult;
};


#endif


