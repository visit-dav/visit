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
//                      avtStructuredDomainNesting.h                         //
// ************************************************************************* //

#ifndef AVT_STRUCTURED_DOMAIN_NESTING_H
#define AVT_STRUCTURED_DOMAIN_NESTING_H

#include <database_exports.h>

#include <avtDomainNesting.h>

#include <vector>
using std::vector;

class vtkDataSet;
class vtkDataArray;

// ****************************************************************************
//  Class:  avtStructuredDomainNesting
//
//  Purpose: Encapsulate domain nesting information for structured meshes
//
//  In this context, a "domain" is a refinement patch. To construct this
//  object, you need to know how many refinement levels there are and
//  how many total patches (summed over all refinement levels).
//
//  For each refinement level, use SetLevelRefinementRatios to specify the
//  refinement ratios used in going from level L-1 to level L where 'L' is
//  the value passed for the 'level' argument to SetLevelRefinementRatios.
//
//  For each domain (or patch), use SetNestingForDomain to specify the
//  domain's level, the list of child domains (in the global address
//  space of domains) and the logical extents of the domain. The logical
//  extents specified on cell-centers as the lowest IJK cell-indices 
//  followed by the highest IJK cell-indices for the patch.
//
//  The domain logical bounds should be specified as though the coarsest
//  resolution mesh (level 0) was refined, everywhere, to the resolution
//  of the given domain's level.
//
//  Programmer:  Mark C. Miller 
//  Creation:    October 13, 2003
//
//  Modifications:
//
//    Mark C. Miller, Wed Aug 18 18:20:27 PDT 2004
//    Added GetSelectedDescendents
//
//    Hank Childs, Thu Jan  6 16:00:27 PST 2005
//    Added ConfirmMesh.
//
//    Kathleen Bonnell, Fri Jun 22 16:43:59 PDT 2007  
//    Added GetRatiosForLevel.
//
//    Hank Childs, Fri Jul 27 13:15:21 PDT 2007
//    Added GetNestingForDomain.
//
//    Hank Childs, Mon May 12 08:15:13 PDT 2008
//    Changed signature for GetSelectedDescdendants.
//
// ****************************************************************************

typedef struct {
    int         level;
    vector<int> childDomains;
    vector<int> logicalExtents;
} avtNestedDomainInfo_t; 

class DATABASE_API avtStructuredDomainNesting : public avtDomainNesting
{
    public:
                      avtStructuredDomainNesting(int nDoms, int nLevels)
                         : avtDomainNesting(),
                         numDimensions(3),
                         domainNesting(vector<avtNestedDomainInfo_t>(nDoms)),
                         levelRatios(vector< vector<int> >(nLevels)) {} ;
        virtual      ~avtStructuredDomainNesting() {} ;

        static void   Destruct(void*);

        bool          ApplyGhost(vector<int> domainList,
                                 vector<int> allDomainList,
                                 vector<vtkDataSet*> meshes); 

        void          SetNumDimensions(int numDims)
                          {numDimensions = numDims; };

        void          SetLevelRefinementRatios(int level, vector<int> ratios)
                          { levelRatios[level] = ratios; };

        void          SetNestingForDomain(int dom, int level,
                          vector<int> childDomains, vector<int> exts)
                          { domainNesting[dom].level          = level;
                            domainNesting[dom].childDomains   = childDomains;
                            domainNesting[dom].logicalExtents = exts; } ;
        void          GetNestingForDomain(int dom, vector<int> &exts,
                            vector<int> &childDomains, vector<int> &childExts);

        vector<int>   GetRatiosForLevel(int level, int dom);

        virtual bool  ConfirmMesh(vector<int> &, vector<vtkDataSet*> &);

    protected:

        void          GetSelectedDescendents(const vector<int>& allDomainList,
                          int dom, vector<int>& selectedDescendents,
                          const vector<bool>& lookup) const;

        int numDimensions;

        vector<avtNestedDomainInfo_t> domainNesting; 

        vector< vector<int> > levelRatios; 
};

#endif
