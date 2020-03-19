// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtContract.h                               //
// ************************************************************************* //

#ifndef AVT_CONTRACT_H
#define AVT_CONTRACT_H

#include <iosfwd>

#include <MapNode.h>

#include <pipeline_exports.h>
#include <ref_ptr.h>
#include <avtDataRequest.h>

class avtWebpage;
class avtContract;
typedef ref_ptr<avtContract> avtContract_p;


// ****************************************************************************
//  Class: avtContract
//
//  Purpose:
//      This is the specification of a pipeline.  This includes the
//      specification of which data you want, as well as a pipeline index to
//      be used later for load balancing and a boolean value indicating if
//      streaming is possible.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2001
//
//  Modifications:
//
//    Jeremy Meredith, Thu Jul 26 12:35:52 PDT 2001
//    Added ShouldUseDynamicLoadBalancing.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Hank Childs, Sun Mar 13 09:49:16 PST 2005
//    Added haveStructuredMeshOptimizations.
//
//    Hank Childs, Fri Jun 15 12:41:41 PDT 2007
//    Added support for DebugDump.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and modify definition of assignment 
//    operator to prevent accidental use of default, bitwise copy 
//    implementations.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Sun Mar  9 06:36:49 PST 2008
//    Add new data member for on demand streaming.
//
//    Tom Fogal, Sun May  3 17:52:35 MDT 2009
//    I overloaded operator<< to allow for easier debugging.
//
//    Cyrus Harrison, Sat Feb 20 21:37:24 PST 2010
//    Added Print() which calls 'operator<<' to get around visiblity issues.
//
//    Dave Pugmire, Tue May 25 10:15:35 EDT 2010
//    Add domain single domain replication to all processors.
//
//    Hank Childs, Wed Aug 25 22:45:04 PDT 2010
//    Add data members for whether or not extents should be calculated.
//
// ****************************************************************************

class PIPELINE_API avtContract
{
  friend ostream& operator<<(ostream &, const avtContract&);
  public:
                        avtContract(avtDataRequest_p, int);
                        avtContract(avtContract_p);
                        avtContract(avtContract_p, avtDataRequest_p);
    virtual            ~avtContract();

    bool                ShouldUseStreaming(void) { return canDoStreaming; };
    void                NoStreaming(void) { canDoStreaming = false; };
    void                SetDataRequest(avtDataRequest_p ds) { data = ds; };

    bool                ShouldUseLoadBalancing(void)  
                               { return  useLoadBalancing && 
                                        !doingOnDemandStreaming; };
    void                UseLoadBalancing(bool);

    void                SetHaveRectilinearMeshOptimizations(bool b)
                               { haveRectilinearMeshOptimizations = b; };
    bool                GetHaveRectilinearMeshOptimizations(void)
                               { return haveRectilinearMeshOptimizations; };
    void                SetHaveCurvilinearMeshOptimizations(bool b)
                               { haveCurvilinearMeshOptimizations = b; };
    bool                GetHaveCurvilinearMeshOptimizations(void)
                               { return haveCurvilinearMeshOptimizations; };
                  
    bool                DoingOnDemandStreaming(void)
                               { return doingOnDemandStreaming; };
    void                SetOnDemandStreaming(bool b)
                               { doingOnDemandStreaming = b; };
    bool                ReplicateSingleDomainOnAllProcessors()
                               { return replicateSingleDomainOnAllProcessors; }
    void                SetReplicateSingleDomainOnAllProcessors(bool b)
                               { replicateSingleDomainOnAllProcessors = b; }    

    avtDataRequest_p    GetDataRequest(void)   { return data; };
    int                 GetPipelineIndex(void) { return pipelineIndex; };

    void                AddFilter(void)  { nFilters++; };
    int                 GetNFilters(void)  { return nFilters; };

    void                SetLineType( int type)  { lineType = type; };
    int                 GetLineType(void)  { return lineType; };

    bool                ShouldCalculateMeshExtents(void) { return calculateMeshExtents; };
    void                SetCalculateMeshExtents(bool c)  { calculateMeshExtents = c; };
    bool                ShouldCalculateVariableExtents(const std::string &s);
    void                SetCalculateVariableExtents(const std::string &s, bool v);
    void                SetCalculateVariableExtentsList(const std::vector<std::string> &l)
                                                      { needExtentsForTheseVariables = l; };
    const std::vector<std::string> &  GetCalculateVariableExtentsList(void)
                                                      { return needExtentsForTheseVariables; };
    void                DisableExtentsCalculations(void) { calculateMeshExtents = false; 
                                                           needExtentsForTheseVariables.clear(); };

    template<class T>
    std::string         SetAttribute( AttributeSubject *atts,
                                      int index, T val );
    MapNode*            GetAttribute( std::string );

    avtContract        &operator=(const avtContract &);
    void                DebugDump(avtWebpage *);
    void                Print(ostream &);
  protected:
    avtDataRequest_p    data;
    int                 pipelineIndex;
    bool                canDoStreaming;
    bool                doingOnDemandStreaming;
    bool                useLoadBalancing;
    bool                haveCurvilinearMeshOptimizations;
    bool                haveRectilinearMeshOptimizations;
    bool                replicateSingleDomainOnAllProcessors;
    int                 nFilters;

    std::vector<std::string>    needExtentsForTheseVariables;
    bool                        calculateMeshExtents;

    int                 lineType;
    MapNode             attributeMap;

  private:
    // This method is defined to prevent accidental use of a bitwise copy
    // implementation.  If you want to re-define it to do something
    // meaningful, that's fine.
    avtContract(const avtContract &) {;};
};

// ****************************************************************************
//  Method: avtContract::SetAttribute
//
//  Purpose:
//      Add an operator or plot attribute to the contract so that 
//      communication can happen within the pipeline.
//
//  Programmer: Allen Sanderson
//  Creation:   June 16, 2014
//
// ****************************************************************************

template<class T> std::string
avtContract::SetAttribute( AttributeSubject *atts, int index, T value )
{
  // Create a key string that contains the attribute class name and
  // attribute name.
  std::string key = atts->TypeName();
  key += std::string("::");
  key += atts->GetFieldName( index );

  if( attributeMap.HasEntry( key ) )
  {
    return std::string("");
  }

  attributeMap[ key ] = value;

  return key;
}

#endif
