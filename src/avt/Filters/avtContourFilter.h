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
//                             avtContourFilter.h                            //
// ************************************************************************* //

#ifndef AVT_CONTOUR_FILTER_H
#define AVT_CONTOUR_FILTER_H

#include <filters_exports.h>

#include <ContourOpAttributes.h>
#include <avtSIMODataTreeIterator.h>
#include <vector>

using std::string;

class vtkCellDataToPointData;
class vtkVisItContourFilter;
class vtkDataSet;


// ****************************************************************************
//  Class: avtContourFilter
//
//  Purpose:
//      A filter that performs a contour on domains of an avtDataSet.
//
//  Programmer: Hank Childs
//  Creation:   July 24, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug 22 15:40:45 PDT 2000
//    Added data member cd2pd so cell-centered data can also be handled.
//
//    Jeremy Meredith, Tue Sep 19 22:29:06 PDT 2000
//    Added data member levels, made constructor initialize using
//    raw levels, and added Equivalent method.
//
//    Jeremy Meredith, Thu Sep 28 12:50:55 PDT 2000
//    Removed CreateOutputDatasets.  Changed interface to ExecuteDomain.
//
//    Kathleen Bonnell, Fri Feb 16 13:28:57 PST 2001 
//    Made inherit from avtDomainTreeDataTreeIterator.  Added default constructor,
//    SetLevels method.
//
//    Kathleen Bonnell, Tue Apr 10 11:35:39 PDT 2001 
//    Made inherit from avtSIMODataTreeIterator.  
//
//    Kathleen Bonnell, Wed Sep 19 12:55:57 PDT 2001 
//    Added string argument to Execute method. Added member isoLabels, to
//    hold string representation of computed isoValues.  Added method
//    CreateLabels.
//
//    Hank Childs, Mon Aug 30 09:03:38 PDT 2004
//    Do a better job of providing progress.  Added two data members, nnodes
//    and current_node to help with bookkeeping.
//
//    Hank Childs, Sun Mar  6 08:18:53 PST 2005
//    Removed "centering conversion module" data member.
//
// ****************************************************************************

class AVTFILTERS_API avtContourFilter : public avtSIMODataTreeIterator
{
  public:
                               avtContourFilter(const ContourOpAttributes &);

    virtual                   ~avtContourFilter();

    void                       ShouldCreateLabels(bool b)
                                    { shouldCreateLabels = b; };
    virtual const char        *GetType(void)  { return "avtContourFilter"; };
    virtual const char        *GetDescription(void) { return "Contouring"; };
    virtual void               ReleaseData(void);

  protected:
    vtkVisItContourFilter     *cf;

    bool                       stillNeedExtents;
    bool                       shouldCreateLabels;

    ContourOpAttributes        atts;
    bool                       logFlag; 
    bool                       percentFlag; 
    int                        nLevels;
    std::vector<double>        isoValues;
    std::vector<std::string>   isoLabels;

    int                        nnodes;
    int                        current_node;

    virtual avtContract_p
                               ModifyContract(avtContract_p);
    virtual avtDataTree_p      ExecuteDataTree(vtkDataSet *, int, string);

    virtual void               UpdateDataObjectInfo(void);
    virtual void               PreExecute(void);

    void                       CreatePercentValues(double, double);
    void                       CreateNIsoValues(double, double);
    void                       SetIsoValues(double, double);
    void                       CreateLabels(void);
};


#endif


