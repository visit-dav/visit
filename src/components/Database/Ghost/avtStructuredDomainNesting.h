#ifndef AVT_STRUCTURED_DOMAIN_NESTING_H
#define AVT_STRUCTURED_DOMAIN_NESTING_H
#include <database_exports.h>

#include <avtDomainNesting.h>

#include <vector>
using std::vector;

class vtkDataSet;
class vtkDataArray;

// ****************************************************************************
//  Class:  avtDomainNesting
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
// ****************************************************************************
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

    protected:

        void          GetSelectedDescendents(const vector<int>& allDomainList,
                          int dom, vector<int>& selectedDescendents) const;

        typedef struct {
           int         level;
           vector<int> childDomains;
           vector<int> logicalExtents;
        } avtNestedDomainInfo_t; 

        int numDimensions;

        vector<avtNestedDomainInfo_t> domainNesting; 

        vector< vector<int> > levelRatios; 
};

#endif
