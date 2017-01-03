/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <cstddef>
#include <vector>

#include <avtDomainNesting.h>

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
//  Addendum (Gunther H. Weber):
//  For the AMRStitchCell operator to work correctly, use SetLevelCellSizes()
//  to provide the grid spacing in this level. Usually, cell sizes for level
//  L should be the cell sizes for level L-1 divided by the refinement ratio
//  for level L. However, some formats (e.g., BoxLib) specify this information
//  for each level, and to avoid numerical inaccuracies, the cell size in the 
//  reader (when constructing a mesh) should be the same as the cell size used
//  by stitch cell generations (otherwise, levels and stitch cells do not match
//  up).
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
//    Tom Fogal, Fri Aug  6 16:15:11 MDT 2010
//    Add method to get the total number of domains we know about.
//
//    Gunther H. Weber, Wed Jan 18 17:51:27 PST 2012
//    Added information about cell sizes in levels and methods to set and
//    access them. Added method to get number of levels. Added method to
//    access level refinement ratios. In constructor pass arguments directly
//    instead of creating object to be copied.
//
//    Gunther H. Weber, Fri Jan 20 11:16:03 PST 2012
//    Pass arrays to SetLevelRefinementRatios() and SetLevelCellSizes() by
//    const reference instead of copy.
//
// ****************************************************************************

typedef struct {
    int         level;
    std::vector<int> childDomains;
    std::vector<int> logicalExtents;
    int childBoundingBox[6];
} avtNestedDomainInfo_t; 

class DATABASE_API avtStructuredDomainNesting : public avtDomainNesting
{
    public:
                      avtStructuredDomainNesting(int nDoms, int nLevels)
                         : avtDomainNesting(), numDimensions(3),
                         domainNesting(nDoms), levelRatios(nLevels),
                         levelCellSizes(nLevels) {} ;
        virtual      ~avtStructuredDomainNesting() {} ;

        static void   Destruct(void*);

        bool          ApplyGhost(std::vector<int> domainList,
                                 std::vector<int> allDomainList,
                                 std::vector<vtkDataSet*> meshes); 

        void          SetNumDimensions(int numDims)
                          { numDimensions = numDims; };

        void          SetLevelRefinementRatios(int level, const std::vector<int>& ratios)
                          { levelRatios[level] = ratios; };
        const std::vector<int>&
                      GetLevelRefinementRatios(int level)
                          { return levelRatios[level]; }

        void          SetLevelCellSizes(int level, const std::vector<double>& sizes)
                          { levelCellSizes[level] = sizes; };
        const std::vector<double>&
                      GetLevelCellSizes(int level)
                          { return levelCellSizes[level]; }

        void          SetNestingForDomain(int dom, int level,
                                          std::vector<int> childDomains,
                                          std::vector<int> exts)
                          { domainNesting[dom].level          = level;
                            domainNesting[dom].childDomains   = childDomains;
                            domainNesting[dom].logicalExtents = exts; }
        void          GetNestingForDomain(int dom, std::vector<int> &exts,
                            std::vector<int> &childDomains, std::vector<int> &childExts);

        std::vector<int>   GetRatiosForLevel(int level, int dom);

        virtual bool  ConfirmMesh(std::vector<int> &, std::vector<vtkDataSet*> &);

        void          ComputeChildBoundingBox(int domain);
        bool          InsideChildBoundingBox(int domain, int ijk[6]);
        void          GetChildrenForLogicalIndex(int domain, int ijk[3],
                                                 std::vector<int> &children,
                                                 std::vector<int> &chExts);
        void          GetChildrenForLogicalRange(int domain, int ijk[6],
                                                 std::vector<int> &children,
                                                 std::vector<int> &chExts);
        std::vector<int>   GetDomainChildren(int domain);
        int           GetDomainLevel(int domain);
        std::vector<int>   GetDomainLogicalExtents(int domain);
        int           GetNumberOfChildren(int domain);
        size_t        GetNumberOfDomains() const;
        size_t        GetNumberOfLevels() const
                          { return levelRatios.size(); }

    protected:

        void          GetSelectedDescendents(const std::vector<int>& allDomainList,
                          int dom, std::vector<int>& selectedDescendents,
                          const std::vector<bool>& lookup) const;

        int numDimensions;

        std::vector<avtNestedDomainInfo_t> domainNesting; 

        std::vector< std::vector<int> > levelRatios; 
        std::vector< std::vector<double> > levelCellSizes;
};
#endif
