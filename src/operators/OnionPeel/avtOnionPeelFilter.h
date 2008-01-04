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
//                            avtOnionPeelFilter.h                           //
// ************************************************************************* //

#ifndef AVT_ONION_PEEL_FILTER_H
#define AVT_ONION_PEEL_FILTER_H


#include <avtPluginStreamer.h>
#include <OnionPeelAttributes.h>


class vtkDataSet;
class vtkOnionPeelFilter;
class vtkPolyDataOnionPeelFilter;


// ****************************************************************************
//  Class: avtOnionPeelFilter
//
//  Purpose:
//    A filter that turns dataset into unstructured grid comprised of
//    a seed cell and a number of layers surrounding the seed cell as
//    specified by the user.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 09, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Oct 27 10:23:52 PDT 2000
//    Added argument to ExecuteDomain to match interited interface.
//
//    Jeremy Meredith, Thu Mar  1 13:29:27 PST 2001
//    Made attributes be stored as an OnionPeelAttributes class.
//
//    Jeremy Meredith, Sun Mar  4 16:59:57 PST 2001
//    Added a static Create method.
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001 
//    Changed ExecuteDomain to ExecuteData. 
//
//    Hank Childs, Wed Jun  6 08:58:51 PDT 2001
//    Renamed some methods to fit changes in base class.
//
//    Kathleen bonnell, Tue Oct 16 14:55:41 PDT 2001
//    Added VerifyInput, PerformRestriction. 
//
//    Hank Childs, Wed May 22 17:25:22 PDT 2002
//    Added BadSeedCell.
//
//    Kathleen Bonnell, Thu Aug 15 18:37:59 PDT 2002  
//    Added Pre/PostExecute methods. Added bool argument to BadSeedCell and
//    BadSeedCellCallback.  Added groupCategory, encounteredGhostSeed, and 
//    successfullyExecuted. 
//
// ****************************************************************************

class avtOnionPeelFilter : public avtPluginStreamer
{
  public:
                         avtOnionPeelFilter();
    virtual             ~avtOnionPeelFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtOnionPeelFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Selecting neighbors for onion peel"; };
    virtual void         ReleaseData(void);

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

    void                 BadSeed(int, int, bool);
    static void          BadSeedCallback(void *, int, int, bool);


  protected:
    OnionPeelAttributes   atts;
    vtkOnionPeelFilter   *opf;
    vtkPolyDataOnionPeelFilter   *poly_opf;

    int                   badSeed;
    int                   maximumIds;
    bool                  encounteredBadSeed;
    bool                  encounteredGhostSeed;
    bool                  groupCategory;
    bool                  successfullyExecuted;

    virtual vtkDataSet   *ExecuteData(vtkDataSet *, int, std::string);
    virtual void          PreExecute();
    virtual void          PostExecute();

    virtual void          VerifyInput(void);
    virtual void          RefashionDataObjectInfo(void);
    virtual avtPipelineSpecification_p
                          PerformRestriction(avtPipelineSpecification_p);

};


#endif


