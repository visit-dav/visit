// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtMetaData.h                               //
// ************************************************************************* //

#ifndef AVT_META_DATA_H
#define AVT_META_DATA_H

#include <pipeline_exports.h>

#include <avtContract.h>

class     avtDataSelection;
class     avtFacelist;
class     avtHistogramSpecification;
class     avtIdentifierSelection;
class     avtIntervalTree;
class     avtMaterial;
class     avtMixedVariable;
class     avtSpecies;
class     avtOriginatingSource;
class     avtDomainBoundaries;
class     avtDomainNesting;

// ****************************************************************************
//  Class: avtMetaData
//
//  Purpose:
//      This object is responsible for holding and possibly calculating meta-
//      data.  All of the meta-data is calculated in a lazy evaluation style -
//      only meta-data that is requested is calculated.  In addition, the
//      source is given an opportunity to give its copy of meta-data, in case
//      it is a source from a database and can go fetch some preprocessed 
//      meta-data.  The meta-data object currently only holds spatial and data
//      extents.
//
//  Programmer: Hank Childs
//  Creation:   August 7, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jun  5 08:05:29 PDT 2001
//    Changed interface to allow for a data specification to be considered
//    when getting materials or facelists.  Blew away outdated comments.
//
//    Jeremy Meredith, Thu Dec 13 16:00:29 PST 2001
//    Removed GetMaterial since it was never used.
//
//    Hank Childs, Tue Aug 12 10:27:25 PDT 2003
//    Added GetMaterial for matvf expressions.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Jeremy Meredith, Wed Jun  9 09:12:09 PDT 2004
//    Added GetSpecies.
//
//    Kathleen Bonnell, Mon Jun 28 08:05:38 PDT 2004 
//    Added optional int arg (for time) to GetMaterial, GetSpecies. 
//
//    Kathleen Bonnell, Thu Jul  1 16:41:57 PDT 2004 
//    Added GetMixedVar.
//
//    Mark C. Miller, Mon Oct 18 13:02:37 PDT 2004
//    Added optional var args to GetDataExtents/GetSpatialExtents
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Cyrus Harrison, Wed Jan 30 13:23:30 PST 2008
//    Added variable name argument to GetMixedVar, allowing this method
//    to be used when the desired mixed var differs from the active
//    variable in the contract. 
//
//    Cyrus Harrison, Wed Feb 20 09:14:15 PST 2008
//    Added post ghost option to GetMaterial & GetMixedVar
//
//    Hank Childs, Tue Jan 27 11:11:30 PST 2009
//    Added code to get histogram specifications and identifiers.
//
//    Eduard Deines / Hank Childs, Thu Aug  5 18:18:39 MDT 2010
//    Add calls to grab domain nesting and boundary information.
//
// ****************************************************************************

class PIPELINE_API avtMetaData
{
  public:
                                 avtMetaData(avtOriginatingSource *);
    virtual                     ~avtMetaData();

    avtIntervalTree             *GetDataExtents(const char *var = NULL);
    avtIntervalTree             *GetSpatialExtents(const char *var = NULL);
    avtIntervalTree             *GetSpatialExtents(int timeSlice, const char *var = NULL);
    bool                         GetHistogram(avtHistogramSpecification *);
    avtIdentifierSelection      *GetIdentifiers(std::vector<avtDataSelection *>);

    avtFacelist                 *GetExternalFacelist(int);
    avtMaterial                 *GetMaterial(int, int = -1, bool = false);
    avtSpecies                  *GetSpecies(int, int = -1);
    avtMixedVariable            *GetMixedVar(const char *,int, 
                                             int = -1,
                                             bool = false);

    avtDomainNesting            *GetDomainNesting(void);
    avtDomainBoundaries         *GetDomainBoundaries(void);

  protected:
    avtOriginatingSource        *source;

    avtContract_p   GetContract(void);
    avtContract_p   GetContract(int domain, int timeSlice = -1);

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtMetaData(const avtMetaData &) {;};
    avtMetaData         &operator=(const avtMetaData &) { return *this; };
};

#endif


