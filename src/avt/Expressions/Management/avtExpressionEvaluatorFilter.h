/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                         avtExpressionEvaluatorFilter.h                    //
// ************************************************************************* //

#ifndef AVT_EXPRESSION_EVALUATOR_FILTER_H
#define AVT_EXPRESSION_EVALUATOR_FILTER_H

#include <expression_exports.h>

#include <avtDatasetToDatasetFilter.h>
#include <avtQueryableSource.h>

#include <ExprPipelineState.h>

class avtDDF;
class avtSourceFromAVTDataset;


typedef avtDDF *   (*GetDDFCallback)(void *, const char *);


// ****************************************************************************
//  Class: avtExpressionEvaluatorFilter
//
//  Purpose:
//      This filter parses out expressions and turns them into executable
//      VTK networks.  It encapsulates the code that used to be in the
//      NetworkManager and the Viewer.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Nov 21 15:15:07 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Aug 27 14:06:00 PST 2003
//   Made it use the right API.
//
//   Hank Childs, Mon Nov 17 16:47:33 PST 2003
//   Add ReleaseData.
//
//   Kathleen Bonnell, Thu Nov 13 08:39:40 PST 2003 
//   Added 'FindElementForPoint'.
//
//   Kathleen Bonnell, Mon Dec 22 14:39:30 PST 2003
//   Added GetDomainName.
//
//   Hank Childs, Thu Feb  5 17:11:06 PST 2004
//   Moved inlined constructor and destructor definitions to .C files
//   because certain compilers have problems with them.
//
//   Kathleen Bonnell, Tue May 25 16:16:25 PDT 2004 
//   Added QueryZoneCenter.
//
//   Kathleen Bonnell, Thu Jun 10 18:29:08 PDT 2004
//   Rename QueryZoneCenter to QueryCoords, added bool arg.
//
//   Kathleen Bonnell, Mon Jun 28 08:01:45 PDT 2004 
//   Added currentTimeState, ExamineContract. 
//
//   Kathleen Bonnell, Thu Dec 16 17:11:19 PST 2004 
//   Added another bool arg to QueryCoords. 
//
//   Hank Childs, Wed Dec 29 08:02:40 PST 2004
//   Added friend access to avtMacroExpressionFilter.  Also cache the
//   terminating source for updates.
//
//   Kathleen Bonnell, Tue Jan 25 07:59:28 PST 2005 
//   Added const char* arg to QueryCoords. 
//
//   Hank Childs, Tue Aug 16 16:50:56 PDT 2005
//   Add method VerifyVariableTypes.
//
//   Hank Childs, Sun Dec  4 17:31:14 PST 2005
//   Added description.
//
//   Hank Childs, Sun Feb 19 09:54:19 PST 2006
//   Add support for DDFs.
//
//   Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//   Added support for rectilinear grids with an inherent transform.
//
//   Hank Childs, Mon Dec 10 17:49:02 PST 2007
//   Add data member for keeping track of expressions generated.
//
// ****************************************************************************

class EXPRESSION_API avtExpressionEvaluatorFilter 
    : virtual public avtDatasetToDatasetFilter,
      virtual public avtQueryableSource
{
    friend class             avtMacroExpressionFilter;

  public:
                             avtExpressionEvaluatorFilter();
    virtual                 ~avtExpressionEvaluatorFilter();
    virtual const char*      GetType(void)
                                     { return "avtExpressionEvaluatorFilter";};
    virtual const char      *GetDescription(void)
                                     { return "Creating expressions"; };


    virtual void             Query(PickAttributes *);
    virtual avtQueryableSource *
                             GetQueryableSource(void) { return this; };
    virtual void             ReleaseData(void);

    virtual bool             FindElementForPoint(const char *, const int, 
                                 const int, const char *, double[3], int &);
    virtual bool             QueryCoords(const std::string&, const int, 
                                 const int, const int, double[3], const bool,
                                 const bool, const char *mn = NULL);

    virtual void             GetDomainName(const std::string &, const int,
                                 const int , std::string &);

    static void              RegisterGetDDFCallback(GetDDFCallback, void *);

  protected:
    virtual void             PreExecute(void) {}
    virtual void             PostExecute(void) {}
    virtual void             Execute(void);
    virtual avtContract_p
                             ModifyContract(avtContract_p);
    virtual int              AdditionalPipelineFilters(void);
    virtual void             ExamineContract(avtContract_p);
    virtual bool             FilterUnderstandsTransformedRectMesh();

  protected:
    ExprPipelineState            pipelineState;
    avtContract_p   lastUsedSpec;
    avtSourceFromAVTDataset     *termsrc;
    std::vector<std::string>     expr_list_fromLastTime;

    static  GetDDFCallback       getDDFCallback;
    static  void                *getDDFCallbackArgs;

  private:
    int                          currentTimeState;

    void                         VerifyVariableTypes(void);
};


#endif


