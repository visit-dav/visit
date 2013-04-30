/*! 
  \file paradis.h 

  \brief data structures and API for libparadis
 
      Definition, needed to find a node in a std::set<Node>:  node A > node B iff ( domain A > domain B ) || ( domain A == domain B && node ID A > node ID B ).  segment A > segment B iff segA.nodes[0] > segB.nodes[0].    


   ------------------------------------------------------------------  

      ALGORITHM OVERVIEW
      THIS IS A SERIAL ALGORITHM and parallelizing it would require multi-pass out of core techniques.

   -- create all relevant IB, OOB, and IOOB nodes and related arm segments
   -- create and classify arms
   -- decompose higher-energy arms into lower-energy arms until there is nothing but loops and type 111 arms in the data set.
   -- wrap any segments that cross periodic boundaries
   -- create meta-arms
   
   ------------------------------------------------------------------  


*/ 
#ifndef PARADIS_H
#define PARADIS_H

// set this to 1 to re-enable linked loops code
#define LINKED_LOOPS 0


/* now for the API */  
#include <boost/cstdint.hpp>
using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::uint32_t;
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

#include <stdio.h>
#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <iostream>
#include <ostream> 
#include <math.h>

#include "Point.h"

#include "stringutil.h" /* from RC_c_lib, is this a good idea? */ 
#include "debugutil.h" /* from RC_c_lib, now we're committed. */ 

std::string get_version(const char *progname);

extern std::string doctext;

string BurgersTypeNames(int btype);
string ArmTypeNames(int atype);
string MetaArmTypeNames(int mtype);
    
//=============================================

int BurgersCategory(float burgval); 
int InterpretBurgersType(float burg[3]) ;

//  Segment BURGERS TYPES: (P = plus(+) and M = minus(-))
// These are valued in order of increasing energy levels, corresponding to the sum of the square of the components of the burgers vector.  

#define BURGERS_DECOMPOSED  -2  // for segments that are decomposed
#define BURGERS_UNKNOWN     -1  // analysis failed
#define BURGERS_NONE        0   // no analysis done yet
#define BURGERS_PPP         10  // +++  BEGIN ENERY LEVEL 1
#define BURGERS_PPM         11  // ++-
#define BURGERS_PMP         12  // +-+
#define BURGERS_PMM         13  // +--
#define BURGERS_200         20  // BEGIN ENERGY LEVEL 2
#define BURGERS_020         21  
#define BURGERS_002         22
#define BURGERS_220         30  // BEGIN ENERGY LEVEL 3
#define BURGERS_202         31
#define BURGERS_022         32
#define BURGERS_311         40  // BEGIN ENERGY LEVEL 4
#define BURGERS_131         41
#define BURGERS_113         42
#define BURGERS_222         50 // BEGIN ENERGY LEVEL 5
#define BURGERS_004         60 // BEGIN ENERGY LEVEL 6
//extern const char *BurgersTypeNames[] ; 

// Arm MN types:
#define ARM_EMPTY         -1 // marked for deletion after decomposition step
#define ARM_UNKNOWN        0
#define ARM_UNINTERESTING  1
#define ARM_LOOP           2
#define ARM_MM_111         3 
#define ARM_MN_111         4
#define ARM_NN_111         5 
#define ARM_SHORT_NN_111   6

// MetaArm types:
#define METAARM_UNKNOWN     0  // Not defined, error, or some other odd state
#define METAARM_111         1  // Entirely composed of type 111 arms of the same burgers vector.  Does not include loops. 
#define METAARM_LOOP_111    2  // Contains a loop, composed entirely of type 111 arms.
#define METAARM_LOOP_HIGH_ENERGY    3  // Contains a loop, composed entirely of type 200 arms or higher.



/* ========================================  */ 
/*!
  Two special FullNode types, for special occasions.  PLACEHOLDER_NODE is used in ArmSegments for temporary nodes not yet seen in the data file.  
*/ 
#define PLACEHOLDER_NODE (-42)
/* ========================================  */ 

std::string INDENT(int i); 

//=============================================================================
namespace paraDIS {
  class FullNode; 

  //============================================================
  class NodeID {  
  public: 
    NodeID() {mDomainID = -1; mNodeID = -1; }
    NodeID(const NodeID &other) {
      mDomainID = other.mDomainID;
      mNodeID = other.mNodeID;
    }
    NodeID(int16_t domain, int32_t node) {
      mDomainID = domain; 
      mNodeID = node; 
    }
    /*! 
      for ordering sets and hash lookups 
    */ 
    bool operator == (const NodeID &other) const {
      return (mDomainID == other.mDomainID && mNodeID == other.mNodeID);
    }

    /*!
      Some nodes are created with negative Node IDs, and it's useful to negate them for comparison.  The const return type should allow a compiler return optimization. 
    */ 
    const NodeID operator -() {
      return NodeID(-mDomainID, -mNodeID); 
    }

    /*! 
      for ordering sets of MinimalNodes.  Since MinimalNodes have no negative node IDs, we do not check absolute values here.  
    */ 
    bool operator <(const NodeID &other) const {
      return (mDomainID < other.mDomainID ||
              (mDomainID == other.mDomainID && 
               mNodeID < other.mNodeID));
    }
    /*!
      Accessor function 
    */ 
    void Set(int16_t domain, int32_t node) {
      mDomainID = domain; 
      mNodeID = node; 
      return ; 
    }

    /*!
      Accessor function 
    */ 
    int16_t GetDomainID(void) const { return mDomainID; }
    /*!
      Accessor function 
    */ 
    int32_t GetNodeID(void) const {return mNodeID; }

    /*!
      Convert NodeID to string, defining this gives me << for freee
    */ 
    std::string Stringify(int indent) const {
      
      return INDENT(indent) + str(boost::format("NodeID: (%1%,%2%)")% mDomainID % mNodeID);
    }

  private:
    int16_t mDomainID;      
    int32_t mNodeID; 
  };    


  //===========================================================================
  class Neighbor; // forward declaration
  
  /*! class Node
    \brief Abstract base class
  */
  class Node { 
   //-----------------------------------------------------
    // public API
  public: 
    Node() :mInBounds(false) { }
    Node(const NodeID &id):mID(id), mInBounds(false) { }
    /*! 
      virtual destructor assures proper deletion
    */ 
    virtual ~Node() {}

    /*!
      Used in comparing data read from the file with the current Node. 
    */ 
    bool operator ==(const NodeID &compare) const {
      return this->mID == compare; 
    }
    /*!
      Used for ordering sets?  Might not be needed. 
    */ 
    bool operator ==(const Node &other) const {
      return this->mID == other.GetNodeID(); 
    }
    /*!
      Used for ordering vectors, etc. 
    */ 
    bool operator <(const Node &other) const {
      return this->mID < other.GetNodeID(); 
    }
    
   /*!
      Accessor function 
    */ 
    void InBounds(bool truth)  {  mInBounds = truth; }
 
   /*!
      Accessor function 
    */ 
    bool InBounds(void) const { return mInBounds; }
 
   /*!
      Accessor function 
    */ 
    void SetID(const NodeID &id) {
      mID = id; 
      return ;
    }
    /*!
      Accessor function
    */ 
    const NodeID &GetNodeID(void) const  { return mID; }

    int32_t GetNodeSimulationDomain(void) const { return mID.GetDomainID(); }

    int32_t GetNodeSimulationID(void) const { return mID.GetNodeID(); }

    /*! 
      conversion of Node to string
    */ 
    virtual std::string Stringify(int indent) const; 

    //-----------------------------------------------------   
  protected:
    /*!
      needed to find a node in a set<MinimalNode>
    */ 
    NodeID mID; 
    
    /*! 
      Handy to just set this once -- all nodes need this 
    */ 
    bool mInBounds; 
 

    friend class FullNode; 
  }; 
   
  //=============================================
  /*!  class Neighbor
    Concrete base class 
    Contains no burgers information to save memory. 
    
    Invariant:  mEndpoints[0] < mEndpoints[1]
  */ 
  class Neighbor { 
  public: 
    Neighbor() {    }
    virtual ~Neighbor() {}
    /*!
      Accessor to set endpoints in proper order so that mEndpoints[0] < mEndpoints[1]
    */ 
    void SetEndpoints(const NodeID &ep1, const NodeID &ep2) {
      if (ep1 == ep2) {
        throw std::string("Error in Neighbor::SetEndpoints: both endpoints are the same!"); 
      }
      if (ep1 < ep2) {
        mEndpoints[0] = ep1; 
        mEndpoints[1] = ep2; 
      } else {
        mEndpoints[0] = ep2; 
        mEndpoints[1] = ep1; 
      } 
      return; 
    }
    /*!
      Common operation: we have one node ID, but we're looking to see what the other end of the segment is -- we cannot operate on nodes, because we don't have the global node list, so we use NodeID. 
    */ 
    NodeID GetOtherEndpoint(const NodeID &id) const {
      if (mEndpoints[0] == id) return mEndpoints[1]; 
      return mEndpoints[0]; 
    }

    /*!
      Accessor
    */ 
    const NodeID &GetEndpoint(int num) const{
      return mEndpoints[num];
    }
    /*!
      Accessor -- not needed?  
    */ 
    bool operator == (const Neighbor&other) const {      
      return mEndpoints[0] == other.mEndpoints[0];
    }
     
    /*!
      for sorting
    */ 
    bool operator <(const Neighbor&other) const {
      
      return mEndpoints[0] < other.mEndpoints[0] ||
        (mEndpoints[0] == other.mEndpoints[0] && 
         mEndpoints[1] < other.mEndpoints[1]); 
    }
    /*! 
      conversion of Neighbor to string
    */ 
    virtual std::string Stringify(int indent) const {
      return INDENT(indent) +  std::string("Neighbor: endpoints = (") +
        mEndpoints[0].Stringify(0) + string(", ")  +  mEndpoints[1].Stringify(0) + string(")"); 
    } 

  protected: 
    /*!
      \brief mEndpoints[0] < mEndpoints[1]; 
      We use NodeIDs here, although they are more memory than a Node pointer, to allow us to create a new neighbor relation as soon as we see it, even if the node on the other end doesn't exist at the moment.
    */
    NodeID mEndpoints[2]; 
  }; /* end Neighbor */ 


  //=============================================
  /*!
    Minimal Nodes are just enough information to learn what nodes we really need to load into memory.  We have to load all nodes into memory at launch, under my current scheme.  This is avoidable if I rewrite the dump file in sorted binary format with an index.  But that was too much trouble and can always be done later.  LOL 
  */ 
  class MinimalNode: public Node {
  public: 

    MinimalNode()
      :  mKeep(false) {  }        
    MinimalNode(const NodeID &id)
      : Node(id),  mKeep(false) {}
      
   /*!
      Accessor function -- returns whether the node was marked "Keep" earlier
    */
    bool GetKeep(void) const {
      return mKeep; 
    }
    /*!
      Accessor function -- mark node to keep (or not) later when culling
    */
    void SetKeep(bool tf=true) {
      mKeep = tf; 
    }

    /*!
      Accessor
    */ 
    uint32_t GetFileOrderIndex(void) const { return mFileOrderIndex; }

    /*!
      Accessor
    */ 
    void SetFileOrderIndex(uint32_t index) { mFileOrderIndex = index; }

    /*!
      For sorting by file order
    */ 
    bool ComesBeforeInFile(const MinimalNode &other) const {
      return mFileOrderIndex < other.GetFileOrderIndex(); 
    }


    /*! 
      Convert of MinimalNode to string
    */ 
    virtual std::string Stringify(int indent, bool shortform=true) const {
      std::string s =std::string("MinimalNode: ")+Node::Stringify(0) + string("\nClassification: ") + string(mKeep?"KEEP":"DON'T KEEP") + string("\n");  
      s += (intToString(mNeighbors.size()) + " neighbors --------------:\n");
      uint32_t n = 0; 
      if (!shortform) {
        while (n < mNeighbors.size()) {
          s += "neighbor " + intToString(n) + ": "; 
          if (mNeighbors[n]) {
            s += mNeighbors[n]->Stringify(indent+1) + "\n"; 
          } else {
            s += "(NULL)\n"; 
          }
          ++n;
        }
      }
      return INDENT(indent) + s;
    } 
    /*!
      Accessor function
    */ 
    int GetNumNeighbors(void) const { return mNeighbors.size(); }
    /*! 
      Accessor function --  returns const due to the fact that neighbors are stored in a set in the data set.  But you can always create a copy of what you get, or use const_cast<> on it. 
    */ 
    /*  const Neighbor *GetNeighbor(uint32_t num)  const { 
      if (num >= mNeighbors.size())
        throw std::string("subscript out of range in BaseNode::GtNeighbor"); 
      return mNeighbors[num]; 
    }
    */
   
    /*! 
      Accessor function
    */ 
    const std::vector<const Neighbor *> &GetNeighbors(void) const {
      return mNeighbors; 
    }

    /*! 
      Accessor function
    */ 
    void AddNeighbor (const Neighbor *n) {
      mNeighbors.push_back(n); 
    }

  private:
    /*!
      if mKeep is false, full node info is not needed for any reason
    */ 
    bool mKeep ; 
    /*!
      Nodes must be sorted to find while classifying, then resorted in "file order" to most quickly load the full nodes later.  So we keep an index into the file.  
    */ 
    uint32_t mFileOrderIndex; 

    /*! 
      The neighbors for this particular node
    */ 
    std::vector<const Neighbor*> mNeighbors; 
  };

  //============================================= 
  class ArmSegment; //forward declaration
  /*!
    A FullNode is a Node and has extra information as well.  
  */ 
  class FullNode: public Node {
  public:
    FullNode() : Node() {
      init(); 
      return; 
    }
    FullNode(const FullNode &other, bool skipneighbors=false) {
      *this = other; 
      if (skipneighbors) {
        mNeighborSegments.clear(); 
        mNeighborArms.clear(); 
      }
      return;
    }
    FullNode(int16_t domain, int32_t nodeID) {
      init(); 
      mID = NodeID(domain, nodeID); 
    }
    void init(void) {
      mInBounds = false; 
      float location[3] = {0,0,0}; 
      SetLocation(location); 
      mNodeType = 0; 
      mIsLoopNode = false;
      mWrappedCopy = false; 
    }
    /*!
      Full nodes are first copied from MinimalNodes.  This is just a member-wise "shallow" copy.  
    */       
    const FullNode &operator =(Node &otherNode) {
      mID = otherNode.mID; 
      mInBounds = otherNode.mInBounds; 
      return *this; 
    } 
     
   /*!
      Accessor function 
    */ 
    void SetInBounds(void) {
      mInBounds = rclib::InBounds(rclib::Point<float>(mLocation), mBoundsMin, mBoundsMax );
    }
      
 
    /*!
      Accessor function
    */ 
    void SetLocation(float loc[3]) {
      int i=3; while (i--) mLocation[i] = loc[i]; 
    }

    /*!
      Accessor function
    */ 
    float GetLocation(int index) const { return mLocation[index]; }

    /*!
      Accessor function
    */ 
    const float *GetLocation(void) const {
      return mLocation; 
    }

    /*!
      Accessor
    */
    void GetLocation(float oLoc[3]) const {
      int i=3; while (i--) oLoc[i] = mLocation[i]; 
    }

    /*!
      Special Accessor.  Add the given amount to mLocation[index]; 
    */ 
    void AddToLocation(int i, float shift){
      mLocation[i] += shift; 
    }

    /*!
      Compute the distance to another node
    */
    double Distance(const FullNode &other, bool wrap=false) { 
      double dist[3] = {0}, sum=0; 
      int i=3; while (i--) {
        dist[i] = mLocation[i] - other.mLocation[i]; 
        if (wrap && fabs(dist[i]) > mBoundsSize[i]/2.0) {
          dist[i] = mBoundsSize[i] - fabs(dist[i]); 
        }
        sum += dist[i]*dist[i]; 
      }
      return sqrt(sum); 
      /*
        double dx = mLocation[0]-other.mLocation[0],  
        dy = mLocation[1]-other.mLocation[1],        
        dz = mLocation[2]-other.mLocation[2];         
        return sqrt(dx*dx + dy*dy + dz*dz); 
      */ 
    } 
    /*!
      Accessor function.  
      The node will attempt to determine its own type.  
    */ 
    void ComputeNodeType(void); 

   /*!
      Accessor function set the node type.  
    */ 
    void SetNodeType(int8_t itype) { 
      mNodeType = itype; 
      return; 
    }

    /*!
      Accessor function
    */ 
    int8_t GetNodeType(void) { return mNodeType; }
    
    
    /*!
      Query
    */
    bool IsTypeM(void) {
       return mNodeType < 0; 
    }

    bool IsTypeN(void) {
       return mNodeType > 2; 
    }

    bool IsLoopNode(void) {
      return mIsLoopNode;  
    }
    
    void SetLoopNode(bool tf = true) {
      mIsLoopNode = tf; 
    }

    /*!
      Convert FullNode to string
    */ 
    virtual std::string Stringify(int indent, bool shortform=true) const;

    
    /*!
      Add the arm segment to our list of neighbors  
    */
    void AddNeighbor( ArmSegment *segment) {
      mNeighborSegments.push_back(segment); 
      //ComputeNodeType(); 
    }
     
    /*! 
      Remove one instance of the neighbor segment if present, leaving duplicates.
    */
    void RemoveNeighbor(ArmSegment *oldseg, bool doall = false) {
      if (doall) {
        mNeighborSegments.erase(remove(mNeighborSegments.begin(), mNeighborSegments.end(), oldseg), mNeighborSegments.end()); 
      } else {
        vector<ArmSegment*>::iterator pos = 
          find(mNeighborSegments.begin(), mNeighborSegments.end(), oldseg); 
        if (pos == mNeighborSegments.end()) {
          dbprintf(5, "RemoveNeighbor ERROR:  no matching neighbor found to remove!\n"); 

#ifdef DEBUG
// HOOKS_IGNORE
          abort(); 
#else
          return;
#endif

        }
        mNeighborSegments.erase(pos); 
      }
      //ComputeNodeType(); 
    }

    /*!
      Replace a neighbor of this node with a new neighbor -- done when wrapping nodes
    */ 
    void ReplaceNeighbor(ArmSegment *oldseg,  ArmSegment *newseg); 
    /*!
      Assuming we have two neighbors or less, return the neighbor not passed to us
    */ 
    ArmSegment *GetOtherNeighbor (const ArmSegment* n, bool usePointers=false);
    
      
    /*!
      For each neighbor armsegment, ask it to confirm that we are one of its endpoints.  Yet another piece of bookkeeping from wrapping nodes
    */ 
    void ConfirmNeighbors(void) ;

    /*!
      Called by our neighbor segments destructors to make sure that no dangling pointers are kept as neighbors.  Maintains the number of neighbors, but replaces the neighbor given with a NULL pointer. 
    */ 
    void NullifyNeighborReference(ArmSegment *seg) {
      int num = mNeighborSegments.size(); 
      while (num--) {
        if (mNeighborSegments[num] == seg){
          mNeighborSegments[num] = NULL; 
          return; 
        }
      }
      /*! 
        It's ok if the neighbor was not found -- the segment might have ghost neighbors
      */ 
      return; 
    }

    /*!
      Accessor -- set the global bounds for every point 
    */
    static void SetBounds(rclib::Point<float> &min, rclib::Point<float> &max) {
      mBoundsMin = min; 
      mBoundsMax = max; 
      mBoundsSize = max-min; 
    }


    /*!
      Accessor function
    */ 
    int GetNumNeighborSegments(void) const { return mNeighborSegments.size(); }
    int GetNumNeighborArms(void) const { return mNeighborArms.size(); }
    /*! 
      Accessor function 
    */ 
    ArmSegment *GetNeighborSegment(uint32_t num)  const { 
      if (num >= mNeighborSegments.size()) 
        throw std::string("subscript out of range in GetNeighborSegment"); 
      return mNeighborSegments[num]; 
    }

    struct Arm *GetNeighborArm(uint32_t num)  const { 
      if (num >= mNeighborArms.size()) 
        throw std::string("subscript out of range in GetNeighborArm"); 
      return mNeighborArms[num]; 
    }

    // used to remove an arm which has been decomposed
    void RemoveNeighbor(struct Arm *neighbor, bool doall = false) {
      if (doall) {
        mNeighborArms.erase(remove(mNeighborArms.begin(), mNeighborArms.end(), neighbor), mNeighborArms.end()); 
      } else {
        vector<Arm*>::iterator pos = find(mNeighborArms.begin(), mNeighborArms.end(), neighbor); 
        if (pos == mNeighborArms.end()) {
          dbprintf(5, "Error:  no matching neighbor arm found to remove!\n"); 

#ifdef DEBUG
// HOOKS_IGNORE
          abort(); 
#else
          return;
#endif

        }
        mNeighborArms.erase(pos); 
      }
    }

    void AddNeighbor(struct Arm *neighbor) {
      mNeighborArms.push_back(neighbor); 
    }
        
    /*! 
      Accessor function
    */ 
    const std::vector< int> GetNeighborArmIDs(void) const;

    /*!
      Accessor function
    */
    int32_t GetIndex(void) { return mNodeIndex; }
    
    /*!
      Accessor
    */ 
    void SetIndex(int32_t index=-1) { 
      if (index == -1) {
        mNodeIndex = mFullNodes.size(); 
      } else {
        mNodeIndex = index; 
      }
    }

    bool IsWrappedCopy(void) { return mWrappedCopy; }
    
    void SetWrappedCopy(bool tf) { mWrappedCopy = tf; }

    /*!
      Connectivity to our neighboring arms.
      Only used for terminal nodes.  
    */ 
    std::vector<struct Arm *> mNeighborArms; 

    /*! 
      all fullnodes in the data set.  DO NOT SORT THIS -- each node has an Index which is its position in this array when created -- used to find its counterpart in the wrapped nodes for tracing along arms to find their endpoints and lengths.  
    */ 
    static std::vector<FullNode *> mFullNodes; 
    
    /*!
      to avoid memory leaks, track all these in one place
    */ 
    static std::vector<FullNode *> mWrappedNodes; 

    //===============================================================
    // PRIVATE
    //===============================================================
  private:
    /*! 
      Absolute location in global space
    */ 
    float mLocation[3]; 
    /*!
      Node Type is whether we are a butterfly, monster, or normal node (or a placeholder in a segment)
    */ 
    int8_t mNodeType; 
    
    bool mIsLoopNode; 

    bool mWrappedCopy; // when wrapping, mark the copies as such to prevent double-counting in statistics

    /*!
      This is needed for things like Visit, where nodes are accessed by node ID, whereas this library uses pointers.  Since it is also our index inthe global array of nodes, it turns out to be a good way to get the "real" counterpart of a wrapped node.  
    */ 
    int32_t mNodeIndex; 
    /*!
      Connectivity to our neighboring nodes encapsulated in ArmSegments
    */ 
    std::vector< ArmSegment *> mNeighborSegments; 

    // statistics:
    std::vector<uint32_t> mNumMonsterNodes; 
    /*!
      Static member to keep track of subspace bounds for checking if we are in bounds or not
    */ 
    static rclib::Point<float> mBoundsMin, mBoundsMax, mBoundsSize; 

  }; /* end FullNode */  
  

  // ==================================================================
  // ==================================================================
  /*! 
    Arm segments are like Neighbors in that they contain neighbor relationships, but these are encoded as pointers to nodes instead of NodeIDs, for faster access to complete node data as needed. They also contain burgers and arm-type information for later analysis.  I almost called them "FullNeighbor", but in common terminology they are called "arm segments," since one or more of them comprise an Arm, so I just called them that.  
  */ 
  class ArmSegment
    {
  public: 
    ArmSegment(): mParentArm(NULL) { 
      init(); 
    }
    ArmSegment(const ArmSegment &other){
      init(); 
      *this = other; 
    }

    void init(void) {
      //mMNType = ARM_UNKNOWN; 
      mBurgersType = 0; 
      mSeen = false; 
      int i=2; while (i--) {
        mEndpoints[i] = NULL; 
      }
      mGhostEndpoints.clear(); 
      mSegmentID = -1; 
      mWrapped = false; 
      mNumDuplicates = 0; 
    }
    /*!
      Destructor
    */ 
    ~ArmSegment() {

      int nodenum=2; 
      while (nodenum--) {
        FullNode *epnode = mEndpoints[nodenum]; 
        if (epnode && epnode->GetNodeType() == PLACEHOLDER_NODE) {
          delete epnode; 
          epnode = NULL; 
        }
        if (epnode  != NULL) {
          epnode->NullifyNeighborReference(this); 
        }
      }
      nodenum = mGhostEndpoints.size(); 
      while (nodenum--) {
        mGhostEndpoints[nodenum]->NullifyNeighborReference(this); 
      }
      return; 
    }/* end destructor */ 
    
    /*!
      operator <() is required for set ordering.  All it requires is it be consistent, so we'll just see if our first node is less than the other's first node, if so, then yay, else if it is equal, then check the other endpoint.  Otherwise, it's false.  Pretty standard stuff. Important thing is this is all done by ID, not by pointer values.  
    */ 
    bool operator < (const ArmSegment &other) const {
      return (*mEndpoints[0] < *other.mEndpoints[0] || 
              ( *mEndpoints[0] == *other.mEndpoints[0] && 
                *mEndpoints[1] < *other.mEndpoints[1])); 
    }
    
    /*!
      operator ==() is required for set ordering.  It tests equality of endpoints, which is done by NodeID, not by location.  
    */ 
    bool operator == (const ArmSegment &other) const {
      return 
        (*mEndpoints[0] == *other.mEndpoints[0] && *mEndpoints[1] == *other.mEndpoints[1]) ||
        (*mEndpoints[0] == *other.mEndpoints[1] && *mEndpoints[1] == *other.mEndpoints[0]); 
    }
    /*!
      inequality
    */
    bool operator != (const ArmSegment &other) const { 
      return !(*this == other); 
    }

    /*!      
      For each wrapped segment, there is an identical unwrapped segment with the same node ID's.  One of each such pair of segments has a ghost endpoint with ID greater than one of its nodes and equal to the other, and one has a ghost endpoint with ID less than one of its nodes and equal to the other. This function returns true for one of them and not the other.  Used in counting total segment lengths.  
    */
    bool Cullable(void) { 
      if (mGhostEndpoints.size() > 1) {
        throw string("Error:  There cannot be more than one ghost endpoint for any segment"); 
      }
      if (!mGhostEndpoints.size()) return false; 
      return (*(mGhostEndpoints[0]) < *(mEndpoints[0]) || 
              *(mGhostEndpoints[0]) < *(mEndpoints[1]));
    }

    
    int8_t GetBurgersType(void) const { return mBurgersType; } 

   /*!
      Return the distance between the endpoints
    */ 
    double GetLength(bool wrap=false) const { 
      return mEndpoints[0]->Distance(*mEndpoints[1], wrap); 
    } 
      
    /*!
      Accessor function
    */ 
    int32_t GetNodeIndex(int num) {
      return mEndpoints[num]->GetIndex(); 
    } 

    /*! 
      Set the segment id to the next available global ID
    */ 
    void SetID(void) {
      mSegmentID = mNextID++; 
      return; 
    }
    /*!
      accessor -- noop if not debug mode
    */ 
    uint32_t GetID(void) {
      return mSegmentID; 
    }

    /* Get the metaarm ID for the parent of this segment */ 
    uint32_t GetMetaArmID(void); 

    /* Get the metaarm Type for the parent of this segment */ 
    uint8_t GetMetaArmType(void);
    
    /*! 
      Accessor for MN type
    */ 
    int8_t GetMNType(void) const;

     /* Get the arm ID for the parent of this segment */ 
    uint32_t GetArmID(void);
    
    /*!
      Accessor function
    */ 
    void GetNodeIndices(uint32_t indices[2]) {
      indices[0] = mEndpoints[0]->GetIndex(); 
      indices[1] = mEndpoints[1]->GetIndex(); 
    }
    /*!
      Accessor function
    */ 
    bool Seen(void) const { return mSeen; }
    
    /*!
      Accessor function
    */
    void SetSeen(bool tf) {mSeen = tf; }

    /*!
      convert ArmSegment to string
    */ 
    std::string Stringify(int indent) const; 
 
    /*! 
      Given the float values for the burgers vector from the data set, set the burgers type for the arm segment 
    */ 
    void ComputeBurgersType(float burg[3]); 
   
    void SetBurgersType(int btype) {
      mBurgersType = btype; 
    }

    /*!
      SetEndpoint -- given an actual new FullNode and ID information for an as-yet uncreated neighborNode, we set the endpoints to be the address of the actualNode and the address of an internal placeholder, a copy of neighborNode.  We will delete the placeholder FullNode later, either when the actual FullNode is created, or when our destructor is called.
    */ 
    void SetEndpoints(FullNode *actualNode, int16_t neighborDomain, int32_t neighborNodeID) {
      FullNode *placeholder = new FullNode(neighborDomain, neighborNodeID); 
      placeholder->SetNodeType(PLACEHOLDER_NODE); //mark as a dummy node; 
      placeholder->SetIndex(-42); 
      SetEndpoints(actualNode, placeholder); 
      return ;
    }

    /*! 
      Store the given nodes as endpoints, properly ordered to allow searching in sets and hashes. 
    */ 
    void SetEndpoints(FullNode *node1, FullNode *node2) {
      if (*node1 < *node2) {
        mEndpoints[0] = node1; 
        mEndpoints[1] = node2; 
      } else if (*node2 < *node1) {
        mEndpoints[1] = node1; 
        mEndpoints[0] = node2; 
      } else {
        throw string("Error in SetEndpoints -- endpoints are identical!");
      }
    
      return; 
    }

    /*! 
      Accessor
    */ 
    FullNode * GetGhostEndpoint(void) const {
      if (!mGhostEndpoints.size()) return NULL;
      return mGhostEndpoints[0]; 
    }

    /*! 
      Called by a node as part of bookkeeping when wrapping a segment.  Make sure the given node is one of our endpoints -- add it as a ghost if not
    */ 
    void ConfirmEndpoint(FullNode *ep)  {
      if (mEndpoints[0] == ep || mEndpoints[1] == ep) return ; 
      int epnum = mGhostEndpoints.size(); 
      while (epnum --) {
        if (mGhostEndpoints[epnum] == ep) return;         
      }
      mGhostEndpoints.push_back(ep); 
    }

    bool HasEndpoint(FullNode *ep) {
      return (mEndpoints[0] == ep || mEndpoints[1] == ep);
    }

    /*!
      We are setting the placeholder endpoint of the segment to be an actual node in memory.  This requires that SetEndpoints() was called previously, resulting in the creation of a placeholder node.  
    */ 
    void ReplacePlaceholder(FullNode *actualNode) {
      int epnum=2; while (epnum--) {
        if (mEndpoints[epnum]->GetNodeType() == PLACEHOLDER_NODE) {
          if (!(*actualNode == *mEndpoints[epnum])){
            throw string("Error in SetActualEndpoint: placeholder node does not match replacement."); 
          }
          string me = this->Stringify(0), 
            actuals = actualNode->Stringify(0); 
          dbprintf(5, "ReplacePlaceholder in segment %s replacing ep with actualNode %s\n", me.c_str(), actuals.c_str() ); 
          delete mEndpoints[epnum];
          mEndpoints[epnum] = actualNode; 
          return; 
        }
      }
      throw string("Error in SetActualEndpoint: There is no placeholder to replace.  Segment is ")+this->Stringify(0) + " and actualNode at "+pointerToString(actualNode)+" is " + actualNode->Stringify(0); 
      return; // ha ha
    }
      
    /*! 
      This is called only for wrapped segments, when you need to replace a "real" node pointer with a pointer to a wrapped node.  We still need to keep the old pointer to the real node to notify it if we are deleted. 
    */     
    void ReplaceEndpoint(FullNode *oldEP, FullNode *newEP, bool doGhost) {
      if (mEndpoints[0] == oldEP) {
        if (doGhost) mGhostEndpoints.push_back(mEndpoints[0]); 
        mEndpoints[0] = newEP; 
      } else if (mEndpoints[1] == oldEP) {
        if (doGhost) mGhostEndpoints.push_back(mEndpoints[1]); 
        mEndpoints[1] = newEP; 
      } else {
        throw string("Error in ReplaceEndpoint -- there is no such endpoint"); 
      }
      return;
    }


    /*! 
      Given the size of the periodic bounds of the data, determine if the nodes are closer if you wrap one of them.  If not, just return false and set oNewSegment to NULL.   
      It will be true if the segment crosses any face of the periodic data bounds. If the segment crosses one face, it must also cross a second.  So if there is a crossing, create two new nodes, representing the node reflected so as to represent the closest neighbor in all bounding data periods.  Match endpoint[1] of (*this) with the nearest of these new nodes and create a new segment containing our old endpoint[1] and its new neighbor node.  Replace endpoint[1] in (*this) with the other node.  Return true and set oNewSegment to point to the new segment.  
      These new wrapped nodes will be the same as their "real" counterparts, but have a negative nodeID, and of course a different location from wrapping, and one of their neighbor segments may be different.  The new segment will simply have a pointer to a real node and a pointer to a wrapped node.  This difference only affects BuildArms and where it is important to traverse an entire arm.  In those cases, when a wrapped node is seen, its equivalent can be gotten by referencing mFullNodes(wrappedNode->GetIndex()); 
    */ 
    bool Wrap(const rclib::Point<float> &dataSize, ArmSegment *&oNewSegment, 
              FullNode *&oWrapped0, FullNode *&oWrapped1);
    
     /*!
      When iterating through an arm, if you find a node with only one 
      neighbor, it's a wrapped node and you have to switch to the segment's
      "wrapped double" on the other side of the data set to continue.  This
      function returns the double and changes wrappedNode to the new node so
      you can continue iterating. 
    */ 
    ArmSegment *SwitchToWrappedDouble(FullNode *originalNode, FullNode **wrappedOriginal, FullNode **wrappedOtherEnd) const; 


    /*! 
      Accessor function.  
      \param epnum Must be 0 or 1
    */  
    FullNode *GetEndpoint(int epnum) const { return mEndpoints[epnum]; }

    /*!
      Common Accessor operation: we have one node ID, but we're looking to see what the other end of the segment is -- we cannot operate on nodes, because we don't have the global node list, so we use NodeID. 
    */ 
    FullNode *GetOtherEndpoint(const FullNode *node) const {
      int idx = 0;
      while (idx < 2) {
        if (*mEndpoints[idx] == *node  ) {
          break; 
        }
        ++idx;
      }
      if (idx == 2) {
        throw string("Error in GetOtherEndpoint: cannot find node corresponding to ")+node->Stringify(0); 
        
      }
      return mEndpoints[1-idx];
    }
    /*!
      purely for debugging
    */ 
    int32_t mSegmentID; 

    /*! 
      Used by SetID() to determine the next available segment ID:
    */ 
    static int32_t mNextID; 
    /*!
      to find out if wrapped
    */
    bool mWrapped; 
    
    /*! 
      If this segment is part of an extended arm, that's useful to know for visualization as there will be two or more superposed segments in the same place. 
    */ 
    int mNumDuplicates; 

    /*!
      A bucket for new segments:  "extended" segments from arm decomposition
    */ 
    static std::vector<ArmSegment *> mExtendedArmSegments; 


  protected:

     /*!
      The burgers-type is defined above.  
    */ 
    int8_t mBurgersType; 
    
    /*! 
      The MN_type of the segment is set by its parent arm.  See Arm struct for definitions, but it describes whether the segment is 200 or 111 and whether its parent arm has any monsters at either end. 
    */ 
    //int8_t mMNType; 
     
    /* The metaArm type that it belongs to.  */ 
    int8_t mMetaArmType; 

    /*!
      Marker used for "once-through" operations like building arms that must look at every segment, but which will usually discover echo particular segment more than once. 
    */ 
    bool mSeen; 

    /*!
      Pointers to actual nodes, as opposed to just NodeID's as in Neighbors.  
    */ 
    FullNode * mEndpoints[2]; 


  public:
    struct Arm *mParentArm; 
    /*!
      We usually need two slots for endpoints, but may need extra slots for "ghost endpoints" created when nodes are wrapped.  When this segment is deleted, it goes through its endpoints and tells all of them they are gone.  But wrapping causes some segments to be the neighbor of 3 or even (very rarely) more nodes. So we need to track those special cases. 
    */ 
    std::vector<FullNode*>mGhostEndpoints; 
      
  }; /* end ArmSegment */ 

  // ===============================================================================
  /*! 
    This class is used for decomposed arms to signal that it is a duplicated arm.  
    This happens during the decomposition of a high-energy Arm.
    The originalSegment will not be used in any Arms after decomposition. 
  */ 
  class DuplicatedArmSegment : public ArmSegment {
  public:
    DuplicatedArmSegment(ArmSegment *original): 
      mOriginalSegment( original) {
      *(ArmSegment *)this = *original; 
      return;
    }
    ArmSegment *mOriginalSegment; 
  };

  // ===============================================================================
  /*!
    A wrapper class to enable me to store ArmSegment *'s.  If I just use pointers, it does not work properly with operator ==() in that it compares the pointers for equality,  which is not what I want.  Using this class allows me to overload operator ==() so that the pointers are dereferenced before they are compared.  I also provide unary operator *() for convenience of notation. 
  */ 
  class ArmSegmentSetElement {
  public:
    ArmSegmentSetElement(ArmSegment *segment) {
      mSegment = segment; 
    }
    bool operator == (const ArmSegmentSetElement &other) const {
      return ***this == **other; 
    }

    ArmSegment *operator *() const {
      return mSegment; 
    }
  private:
    ArmSegment *mSegment; 

  }; 
        
   //=============================================
  /*!
    This is a unary function object to be used with the hash in the DataSet.  operator () must return a size_t... 
  */ 
  class ArmSegmentHash {
  public: 
    ArmSegmentHash() {
      mDenominator = 1 << (8*sizeof(size_t)/2 - 2) ; 
    }
    size_t operator () (const ArmSegmentSetElement &segment) const {
      NodeID id0 = (*segment)->GetEndpoint(0)->GetNodeID(), 
        id1 = (*segment)->GetEndpoint(1)->GetNodeID(); 
      return 1000*(id0.GetDomainID() + id1.GetDomainID()) + id0.GetNodeID() + id1.GetNodeID(); 
    }
  private:
    uint32_t mDenominator;
  };
   
  typedef vector<FullNode*>::iterator FullNodeIterator; 
  //==============================================
  /*! 
    Arms are conceptually a list of segments, but all we need to store is the two (or one, for a cycle) terminal ArmSegments for the arm, and the two (or one) terminal Nodes.  This allows us to recreate the arm for drawing or analysis, but saves lots of memory. (Tradeoff of speed to get memory). The assumption is that traversing the arm will be fast enough and not done very often.  If it starts using lots of time, we can always store more information if it makes it faster later.  
    Arms are used just for classifying nodes and segments and are not expected to be useful to the user of this library; 
  */ 
  struct Arm { 
    Arm() {
      this->init();
    }


    void init(void) {
      mArmType = ARM_UNKNOWN;
      mArmLength=0;
      mSeen=false;
      mSeenInMeta=false; 
      mParentMetaArm=NULL;
      mNumSegments = 0; 
#if LINKED_LOOPS
      mPartOfLinkedLoop=false; 
      mCheckedForLinkedLoop=false;
#endif
    }
    /*!
      Clear all data from the Arm for reuse
    */ 
    void Clear(void) { 
      mTerminalSegments.clear();  
      mTerminalNodes.clear(); 
      mArmType = ARM_UNKNOWN; 
      mNumSegments = 0; 
    }
    
    /*!
      A helper for Decompose() function.  
    */ 
    void ExtendByArm(Arm *sourceArm, FullNode *sharedNode, int numDuplicates); 
    
    /*!
      Merge with neighbor arms. 
    */ 
    bool Decompose(int energyLevel); 
    
   /*! 
        create a string out of the arm
    */ 
    std::string Stringify(int indent, bool shortform=true) const ; 

    /*!
      Give the exact Burgers type of its segments. 
      Return 0 is no terminal segments. 
    */
    int8_t GetBurgersType(void) const {
      if (!mTerminalSegments.size())  {
        return BURGERS_UNKNOWN; 
      }
      return mTerminalSegments[0]->GetBurgersType(); 
    }

    /*!
      to set the arm ID for debugging.  In Debug code, this will do nothing
    */ 
    void SetID(void) { 
      mArmID = mNextID++; 
      return; 
    }

    /* Get the metaarm ID for the parent of this arm */ 
    uint32_t GetMetaArmID(void);

    /* Get the metaarm Type for the parent of this arm */ 
    uint8_t GetMetaArmType(void);

  #if LINKED_LOOPS
   /*! 
      A linked loop is defined as:
      A) Two arms which have the same four-armed terminal nodes 
      OR
      B) Three arms which all have the same three-armed terminal nodes.
    */
    void CheckForLinkedLoops(void); 
#endif

    /*!
      Return a  vector with pointers to all nodes in arm,
      computed in order from end to end.  In a loop, the startNode is 
      repeated as first and last node.  This ensures that the number of nodes
      returned is always numSegments + 1
    */ 
    vector<FullNode*> GetNodes(FullNode *startNode = NULL) const;

    // same as GetNodes(mTerminalNodes[1]); If only one terminal node, issue warning and just call GetNodes().  Returns empty vector if only no terminal node.
    vector<FullNode*> GetNodesReversed(void) {
      FullNode *startNode = NULL; 
      if (mTerminalNodes.size() == 2) {
        startNode = mTerminalNodes[1]; 
      } else  if (!mTerminalNodes.size())  {
        vector<FullNode *>nodes; 
        return nodes;    
      } else {
        dbprintf(1, "WARNING: GetNodesReversed() called with  %d terminal nodes\n", mTerminalNodes.size()); 
      }
      return GetNodes(startNode); 
    }
    
    /*!
      Return a new vector with pointers to all segments in arm,
      computed in order from end to end. 
    */ 
    vector<ArmSegment*> GetSegments(FullNode *startNode = NULL) const;
    vector<ArmSegment*> GetSegmentsReversed(void) const{
      if (mTerminalNodes.size() == 2) 
        return GetSegments(mTerminalNodes[1]); 
      
      vector<ArmSegment *>segs; 
      return segs;    
    }
    
    string GetSegmentsAsString(FullNode *startNode = NULL) {  
      string s; 
      vector<ArmSegment *> segments = GetSegments(startNode); 
      int seg = 0, numsegs = segments.size(); 
      s += intToString(numsegs) + string(" segments: \n"); 
      while (seg < numsegs) {
        s += (string("   ") + segments[seg]->Stringify(0) + "\n"); 
         ++ seg; 
      }
      return s;
    }

     double ComputeLength(void); 

  /*!
      Classify the arm as one of NN, MN or MM, combined with 200 or 111...
    */ 
    void Classify(void) ; 

    bool isTypeMM(void) const {
      return mArmType == ARM_MM_111; 
    }
           
    bool isTypeUnknown(void) const { 
      return mArmType == ARM_UNKNOWN; 
    }

    bool isType111(void) {
      return mTerminalSegments.size() && mTerminalSegments[0]->GetBurgersType() >= BURGERS_PPP && mTerminalSegments[0]->GetBurgersType() <= BURGERS_PMM;
    }

    bool isHighEnergy(void) {
      return  mTerminalSegments.size() && mTerminalSegments[0]->GetBurgersType() >= BURGERS_200; 
    }

    FullNode *GetCommonNode(Arm *other) {
      uint32_t myNode = mTerminalNodes.size();
      while (myNode--) {
        uint32_t otherNode = other->mTerminalNodes.size();
        while (otherNode--) {
          if (mTerminalNodes[myNode] == other->mTerminalNodes[otherNode]) {
            return mTerminalNodes[myNode];
          }
        }
      }
      return NULL; 
    }

      

    uint32_t GetNumNeighborArms(void) {
      uint32_t num = 0; 
      uint32_t tnode = mTerminalNodes.size(); 
      while (tnode--) {
        num += (mTerminalNodes[tnode]->mNeighborArms.size() - 1); 
      }
      return num; 
    }

    Arm *GetNeighborArm (int num) {
      uint32_t n = num; 
      uint32_t tnode = 0; 
      while (tnode < mTerminalNodes.size()) {
        uint32_t tnodeNumNeighbors = mTerminalNodes[tnode]->mNeighborArms.size();  
        if (n < tnodeNumNeighbors-1) {
          uint8_t i = 0; 
          for (i=0; i <= n; i++) {
            if (mTerminalNodes[tnode]->mNeighborArms[i] == this) n++; 
          }
          return mTerminalNodes[tnode]->mNeighborArms[n]; 
        }
        n -= (tnodeNumNeighbors-1); // cannot be negative, which is good, (uint32_t)
        tnode ++; 
      }
      return NULL; // no such neighbor
    }

    /*! 
      Return the sum of the length of all segments in the arm
    */ 
    double GetLength(void) const { 
      return mArmLength; 
    }

    uint8_t GetArmType(void) const { return mArmType; }

    struct MetaArm *GetParentMetaArm(void) const { return mParentMetaArm; }

    void SetParentMetaArm(struct MetaArm *ma) { 
     mParentMetaArm = ma; 
    }
    /*! 
      Check to see if this is the body of a "butterfly," which is two three armed nodes connected by a type 200 arm, and which have four uniquely valued type 111 exterior arms ("exterior" means the arms not connecting the two).  If so, mark each terminal node as -3 (normal butterfly.  If one of the terminal nodes is a type -44 "special monster" node, then mark the other terminal node as being type -34 ("special butterfly"). Finally, could be a -35 connected to a -5 node, which is means, a 3 armed connected to 5 armed, such that exterior arms include all four 111 arm types.  
    */ 
    void CheckForButterfly(void); 
    /*! 
      This is a necessary component to CheckForButterfly, broken out for readability in the code. 
    */ 
    bool HaveFourUniqueType111ExternalArms(void); 
    string StringifyExternalArms(int indent) const ;
    bool HaveTwoMatchingType111ExternalArms(void); 

    vector < ArmSegment *> mTerminalSegments; // At least one, but not more than two
    vector <FullNode *> mTerminalNodes;  // At least one, but not more than two
    int8_t mArmType;
    int8_t mMetaArmType; // of its parent if it exists
    double mArmLength; 
    static double mThreshold; // shorter than this and an arm is "short"
    static double mDecomposedLength; // shorter than this and an arm is "short"
#if LINKED_LOOPS
    bool mPartOfLinkedLoop, mCheckedForLinkedLoop; 
#endif
    static double mLongestLength; // for binning
    bool mSeen, mSeenInMeta; // for tracing MetaArms -- each arm need only be viewed once
   /*!
      number of segments in arm
    */ 
    uint32_t mNumSegments; 
    /*! 
      purely for debugging
    */ 
    int32_t mArmID; 

     /*! 
      purely for debugging
    */ 
    static int32_t mNextID; 

    private: 
    struct MetaArm * mParentMetaArm; 
  };

  //=============================================
  /*!
    Functor object for sorting the armsegment set by comparing pointers correctly (by dereferencing, not by pointer address)
  */ 
  class CompareSegPtrs {
  public:
    bool operator() ( const ArmSegment *seg1, const ArmSegment *seg2) {
      return *seg1 < *seg2; 
    }
  };

  typedef set<ArmSegment*, CompareSegPtrs>  ArmSegmentSet;


  //=============================================
  /*!
    MetaArm:  A collection of arms.  Theoretically should span from M to M, but this is not always the case yet.  
  */
  struct MetaArm {
    MetaArm(): mLength(0.0),  mMetaArmType(METAARM_UNKNOWN),  mNumSegments(0) , mCombo(false) , mFound111(false)
    {return;}
    ~MetaArm() {}

    std::string Stringify(int indent); 
    // vector<ArmSegment*> GetArmSegments(void);

    vector<FullNode* > GetNodes(void);    
 
    vector<rclib::Point<float> > GetNodeLocations(bool wrapEndpoints);    
 
    uint32_t GetNumSegments(bool wrapEndpoints) { 
      if (!mNumSegments) {
        dbprintf(1, "WARNING: MetaArm::GetNumSegments(): mNumSegments is 0.  You should call GetNodeLocations() before GetNumSegments() to avoid a permormance penalty.\n"); 
        GetNodeLocations(wrapEndpoints); 
      }
      return mNumSegments; 
    }

    bool FindEndpoint(Arm *seed, FullNode *previous, Arm* candidate);
    void FindEndpoints(Arm *seedArm); 

    // ======================
    /* Just to clean the code, I encapsulate this to make debugging easier */
    void AddArm(Arm *candidate ) {
      if (candidate->GetParentMetaArm() == this)  {
        dbprintf(0, "WARNING! Already added arm %d to this metaarm!\n"); 
      }
      candidate->SetParentMetaArm(this);
      mLength += candidate->GetLength(); 
      mFoundArms.push_back(candidate); 
      return; 
    }
    // ======================
    inline void AddTerminalNode(FullNode *node) {
      uint16_t nodenum = mTerminalNodes.size(); 
      while (nodenum--) {
        if (mTerminalNodes[nodenum] == node) {
          dbprintf(4, "Warning: node not added as it is a duplicate.\n"); 
          return; 
        }
      }
      mTerminalNodes.push_back(node);  
      return; 
    }
    // ======================
    inline void AddTerminalArm(Arm *arm) {
      uint16_t armnum = mTerminalArms.size(); 
      while (armnum--) {
        if (mTerminalArms[armnum] == arm) {
          dbprintf(4, "Warning: arm not added as it is a duplicate.\n"); 
        }
      }
      AddArm(arm); 
      mTerminalArms.push_back(arm);  
      return; 
    }
    inline void CapturePath(bool doreverse) {
      dbprintf(4, "Capture Path %d called\n", (int)doreverse);  
      if (doreverse) {
        reverse(mFoundArms.begin(), mFoundArms.end());
      }
      uint32_t i = 0; 
      while (i < mFoundArms.size()) {
        dbprintf(4, "Capture Path: add arm %d\n", mFoundArms[i]->mArmID); 
        i++; 
      }
      mAllArms.insert(mAllArms.end(), mFoundArms.begin(), mFoundArms.end());
      mFoundArms.clear(); 
      return; 
    }
    
    int8_t GetMetaArmType(void) { return mMetaArmType; }
    int32_t GetMetaArmID(void) { return mMetaArmID; }

    vector<FullNode *>mTerminalNodes; // 
    vector<Arm*> mTerminalArms; // one or two arms
    double mLength; 
    int8_t mMetaArmType; 
    uint32_t mMetaArmID; 
    uint32_t mNumSegments, mNumNodes; 
    static rclib::Point<float> mWrappedNode; // Used in GetNodeLocations -- used to indicate a gap in the point list from a MetaArm where a segment is skipped due to wrapping.  
    bool mCombo; 
    bool mFound111; // if we see a 111 on our search
    vector<Arm*> mFoundArms, mAllArms, mDeadEnds; // only use if debugging is turned on.  
  }; 

  //=============================================
  /*!
    The paraDIS data set.  Note that all you really want is to read the data and return the segments and nodes.  Identifying specific arms in a scene is not likely to be of interest, so arm information is kept internal.  
  */ 
  class DataSet {
  public: 
    DataSet() { init(); }
    /*!
      if this is true, then complete dumps of all data are done, 
      into files named mDebugOutputPrefix + <type> + ".debug"
    */ 
    void EnableDebugOutput(bool tf=true) {mDoDebugOutput = tf; }
    /*!
      By default, this is "./paradis-debug".  See mDoDebugOutput.
    */ 
    void SetDebugOutputDir(std::string dir) { 
      if (dir != "") {
        mDoDebugOutput =true;
      }
      mDebugOutputDir = dir; 
    }

    void EnableMetaArmSearch (void) { 
      mMetaArmFile = "metaArmFile.txt"; 
    }
    void EnableMetaArmSearch (std::string filename) { 
      mMetaArmFile = filename; 
    }
    /*!
      verbosity goes from 0-5, based on dbg_setverbose() from librc.a
      filename if null means stderr
    */ 
    void SetVerbosity(int level, const string filename = "") { 
      if (filename != "") dbg_setfile(filename.c_str()); 
      dbg_setverbose(level); 
    }

    void SetThreshold(double threshold) {
      mThreshold = threshold;
      Arm::mThreshold = threshold; 
      return; 
    }

    void SetNumBins(long numbins) {
      mNumBins = numbins; 
      return; 
    }
    /*!
      Print out arm statistics
    */
    void PrintArmStats(FILE *); 
      
 
   /*!
      Print all arms in a simple format into a file for analysis
    */
    void PrintMetaArmFile(void);

    /*!
      Print all arms in a simple format into a file for analysis
    */
    void PrintArmFile(char*filename);

    /*!
      Parse the input file just for the data bounds. 
    */ 
    void GetBounds( rclib::Point<float> &outDataMin, 
                   rclib::Point<float> &outDataMax, 
                   std::string filename="") {
      if (filename!="") {
        mDataFilename = filename; 
      }
      ReadBounds(); 
      outDataMin = mDataMin; 
      outDataMax = mDataMax; 
      return; 
    }

    /*!
      For debugging, force the subspace max to be half the max of the bounds.  E.g. if data max is (100,100,100), then this causes subspace max to be (50,50,50).
    */ 
    void TestRestrictSubspace(void);


    /*!
s      Tell the data set which file to read
    */ 
    void SetDataFile(std::string datafile) {
      mDataFilename = datafile;
    }

    /*!
      Tag all METAARM_LOOP nodes
    */
    void TagNodes(void);
 
    /*!
      Write out a copy of the input file that has all FullNode tags in it.
    */
    void WriteTagFile(char *outputname); 

    /*!
      Using 3-way binary decomposition, determine our chunk of subspace based on our processor number
    */ 
    void ComputeSubspace(void); 

    /*!
      For parallel processing, tell dataset which procnum we are. 
      If this is set, then the data set will use a certain subspace of the data based on a decomposition in space
    */ 
    void SetProcNum(int procnum, int numprocs) {
      mProcNum = procnum; 
      mNumProcs = numprocs; 
    }

    /*! 
      Accessor -- procnum (0 based) for parallelism
    */ 
    int GetProcNum(void) { return mProcNum; }

    /*! 
      Accessor -- number of procs in parallel
    */ 
    int GetNumProcs(void) { return mNumProcs; }

    /*! 
      Parse the paradis data file and create a full set of arms and nodes
    */ 
    void ReadData(std::string datafile="");    
 
    /* Starting from the given arm, trace out the containing meta arm and return a shared_ptr to the result */
    boost::shared_ptr<MetaArm> TraceMetaArm(vector<Arm>::const_iterator arm); 

    
    /*!
      Accessor function.
    */ 
    uint32_t GetNumNodes(void) { return FullNode::mFullNodes.size(); }
    /*!
      Accessor returns begin of node vector. 
    */ 
    vector<FullNode *>::const_iterator GetNodesBegin(void) { 
      return FullNode::mFullNodes.begin(); 
    }
    /*!
      Accessor returns end of node vector. 
    */ 
    vector<FullNode *>::const_iterator GetNodesEnd(void) {
      return FullNode::mFullNodes.end(); 
    }

   /*! 
      Accessor function. 
    */ 
    FullNode * GetNode(uint32_t nodenum) { return FullNode::mFullNodes[nodenum]; }

   /*! 
      Accessor function. 
    */ 
    uint32_t GetNumArmSegments(void) const { return mFinalArmSegments.size(); }

    /*!
      Accessor returns begin of arm vector. 
    */ 
    vector<ArmSegment *>::const_iterator GetArmSegmentsBegin(void) { 
      return mFinalArmSegments.begin(); 
    }
    /*!
      Accessor returns end of arm vector. 
    */ 
    vector<ArmSegment *>::const_iterator GetArmSegmentsEnd(void) {
      return mFinalArmSegments.end(); 
    }

    /*!
      Accessor function
    */
    ArmSegment *GetArmSegment(uint32_t segnum) { 
      return mFinalArmSegments[segnum]; 
    }

    /*!
      Accessor function
    */
    uint32_t GetNumMetaArms(void) { return mMetaArms.size(); }

    /*!
      Accessor function
    */
    boost::shared_ptr<MetaArm> GetMetaArm(uint32_t metaArmNum) {
      return mMetaArms[metaArmNum]; 
    }

    /*!
      Set the bounds for our chunk of space 
    */ 
    void SetSubspace(const rclib::Point<float> &min, const rclib::Point<float> &max){
      if (min.Exceeds(max)) {
        throw std::string("Error in DataSet::SetSubspace: A value in min vector exceeds max");
      } 
      mSubspaceMin = min; mSubspaceMax = max; 
      return; 
    }

    /*!
      Wrap one or the other locations if they cross a periodic boundary.
      Return true if any change was made. 
    */ 
    static bool Wrap(float *loc0, float *loc2);

    /*! 
        Free up all memory and go back to virgin state. 
    */ 
    void Clear(void) {
      init(); 
    }

    /*! 
      Optionally create the list of meta-arms to trace out the M-N network on a higher level. 
    */ 
    void FindMetaArms(void);    

    /*!
      set:  always sorted so fast, but in order to modify, must remove an element, modify it, and reinsert it... or use const_cast<> 
    */ 
    static set<ArmSegment *, CompareSegPtrs> mQuickFindArmSegments; 
    //=======================================================================
    // PRIVATE 
    //======================================================================
  private:
    /*!
      Initialize things to virginal values. 
    */ 
    void init(void) {  
      mNumBins = 0; 
      mThreshold = -1.0;
      mFileVersion = 0; 
      mMinimalNodes.clear(); 
      mMinimalNeighbors.clear(); 
      vector<FullNode *>::iterator nodepos = FullNode::mFullNodes.begin(), nodeend = FullNode::mFullNodes.end(); 
      while (nodepos != nodeend) delete *nodepos++; 
      FullNode::mFullNodes.clear(); 
      
      nodepos = FullNode::mWrappedNodes.begin(), nodeend = FullNode::mWrappedNodes.end(); 
      while (nodepos != nodeend) delete *nodepos++; 
      FullNode::mWrappedNodes.clear(); 
      
      vector<ArmSegment *>::iterator pos = mFinalArmSegments.begin(), endpos = mFinalArmSegments.end(); 
      while (pos != endpos)  delete *(pos++);  //delete internal pointers. 
      mFinalArmSegments.clear(); 
      
      pos = mWrappedArmSegments.begin();  
      while (pos != mWrappedArmSegments.end()) delete *(pos++); 


      set<ArmSegment*, CompareSegPtrs>::iterator spos = mQuickFindArmSegments.begin(), send = mQuickFindArmSegments.end(); 
      while (spos != send) delete *(spos++); 
      mQuickFindArmSegments.clear(); 
      /*ArmSegmentSet::iterator qfpos = mQuickFindArmSegments.begin(), qfendpos = mQuickFindArmSegments.end(); 
      while (qfpos != qfendpos) delete **(qfpos++); 
      mQuickFindArmSegments.clear(); 
      */ 
      mArms.clear();      
      mProcNum = mNumProcs = mFileVersion = 0; 
      mTotalDumpNodes =0;
      mDoDebugOutput=false; 
      mDebugOutputDir = "./paradis-debug";
      mDataMin = mDataMax = mDataSize = mSubspaceMin = mSubspaceMax = 
        rclib::Point<float>(0.0); 
    }
    /*!
      Parse the input file just for the data bounds. 
    */ 
    void ReadBounds(void);
    /*!
      make a directory or return false.  prints error if fails
    */
    bool Mkdir(const char *); 
    /*! 
      Very chatty output of all minimal nodes, with neighbor info 
    */ 
    void DebugPrintMinimalNodes(void); 

    /*!
       Done in two places, so break out code here: 
    */ 
    std::string GetMonsterNodeSummary(void);

    /*! 
      Prints out all full nodes 
    */ 
    void DebugPrintFullNodes(const char *name = NULL); 
   /*! 
      Prints out all arms
    */ 
    void DebugPrintArms(void);

    /*! 
      Prints out all MetaArms
    */ 
    void DebugPrintMetaArms(void);

    /*!
      Read a node from the input file and write it out with its tag to the tagfile
    */
    void CopyNodeFromFile(uint32_t &lineno, uint32_t &fullNodeNum, std::ifstream &datafile, std::ofstream &tagfile);
    /*! 
      Read a node and its neighbors from a file.  This has to be done in DataSet because we avoid duplicate neighbor structs by using pointers into a global neighbor array.
    */ 
    void ReadMinimalNodeFromFile(uint32_t &lineno, std::ifstream &datafile);
    /*!
      Make a first pass through the file:  load all as MinimalNodes, with minimal info: {domain and ID, whether out of bounds, neighbor IDs}
    */ 
    void CreateMinimalNodes(void); 

   /*!
    Mark all nodes as one of three possible MinimalNode::NodeTypes:
    A)  NoInfo -- Full information needed (FullNodes) (in bounds or direct neighbor to in bounds or terminal OOB node which terminates a collapsible chain of OOB interior segments).  
    B)  PartialInfo -- Partial information needed (PartialNodes) (need just position and neighbor info to compute the length of a collapsible chain of OOB interior segments)
    C)  NoInfo -- Not needed (none of the above applies) 
  */ 
   void ClassifyMinimalNodes(void); 
    /*!
      Given a node, return pointers to all its neighbor nodes, as found by looking in whichNodes to see which have matching ID's to the inode's neighbors.  
    */ 
    std::vector<MinimalNode *> GetNeighborMinimalNodes(const MinimalNode &inode);
    /*!
      Called from ClassifyMinimalNodes, this is used to identify a string of OOB nodes connected to an IB node and find the other end.  Interior nodes are marked Partial, and the terminal node is marked Full, whether it's IB or an OOB terminal.
    */ 
    void MarkOOBNodeChain( MinimalNode &node,  MinimalNode &previous); 
    
  /*! 
    Used for culling nodes from mMinimalNodes, using STL
    */ 
    void CullAndResortMinimalNodes(void);  

    /*!
      Create FullNodes and ArmSegments, using the information contained in the minimal nodes and neighbors discovered.  Work backwards through the MinimalNodes and release them as FullNodes are created.  Create the ArmSegments as you go.  When a segment is first created, one of the nodes it points to will be a dummy placeholder FullNode.  The second time it is seen, the dummy will be replaced by the actual FullNode.  See ArmSegment::SetEndpoints() for implementation details. 
    */ 
    void CreateFullNodesAndArmSegments(void); 
    /*! 
      Read the data file until we find the node specified in theNode.  Create a FullNode from the information in the file. Find each Neighbor as it is seen and create  and its neighbors from a file.  This has to be done in DataSet because we avoid duplicate neighbor structs by using pointers into a global neighbor array.
    */ 
    void ReadFullNodeFromFile(std::ifstream &datafile, MinimalNode &theNode);

    /*!
      Some segments connect a node at one edge of the subspace to a node outside the subspace.  I think these are ok to draw as-is.  But some segments connect a node to a node on the other side ("wrapped" segment).  For a "wrapped" segment, create two new "wrapped" nodes outside the subspace boundary.  Connect each end of the segment to the appropriate wrapped node.  This requires a new segment as well.  
    */ 
    void WrapBoundarySegments(void); 

  /*!
    Starting at the given first node and heading out in the direction of the given first segment in an arm, trace along the arm until you find its other endpoint (terminal segment and node).  When wrapped nodes are found, use their real counterparts instead.  
    This will be where we actually discriminate between node types, etc.  But as mentioned for BuildArms, we don't do that yet.  
  */ 
    void FindEndOfArm(FullNodeIterator &firstNode, FullNode **oFoundEndNode, 
                      ArmSegment *firstSegment,  ArmSegment *&foundEndSegment, Arm *theArm);
    /*! 
      Create all arms for our region. This function is a bit long because we are avoiding recursion by using loops.  Recursion for these arms would get pretty deep. 
      IN THE FUTURE, to save memory, we will implement the following: 
      Segments that connect two PartialNodes are PartialSegments.  Segments that connect two OOB FullNodes are CollapsedSegments.  Segments that connect an IB FullNode to another node are FullSegments.  Only FullSegments are drawn. 
      HOWEVER, For the first implementation, we will not distinguish between Partial and Full Segments, and we will not even collapse arms, so that we can get a good view of what things are like.  After it is clear that we are making sensible things, we can collapse arms to save memory. 
    */ 
    void BuildArms(void);

       
   /*!
      Find all arms that are decomposible, and make them into two arms each.
      Since this will destroy all the original burgers and add NodeID information, 
      statistics might need to be gathered here, 
    */ 
     void DecomposeArms(void);

     /* put all arm segments into mFinalArmSegments */ 
     void CollectAllArmSegments(void);

    /*!
      Makethe final classification on arms as ARM_XX_YYY.  
    */ 
    void ClassifyArms(void); 

    double ComputeArmLengths(void); 

    /*! 
      Compute all node types
    */ 
    void ComputeNodeTypes(void); 

 
    /*!
      Go through and renumber the nodes so that their index is the same as their position in the vector
    */ 
    void RenumberNodes(void); 


    
    /*!
      Extents of the full data set: 
    */ 
    static rclib::Point<float> mDataMin, mDataMax, mDataSize;
    /*!
      Subspace of interest: 
    */ 
    rclib::Point<float> mSubspaceMin, mSubspaceMax; 

    /*! 
      can modify but must sort and insert/delete are slow
    */ 
    std::vector<MinimalNode> mMinimalNodes; 
    /*!
      cannot modify but always sorted so fast
    */ 
    std::set<Neighbor> mMinimalNeighbors; 


    /*!
      The QuickFind array does not allow duplicates, so put wrapped arm segments here 
    */ 
    std::vector<ArmSegment *> mWrappedArmSegments; 
    /*!
      The actual final arm segments to hand out 
    */ 
    std::vector<ArmSegment *> mFinalArmSegments; 

    /*!
      This is useless to the user, only used for classification 
    */ 
    std::vector<Arm *> mArms; 

    double mTotalArmLengthBeforeDecomposition, mTotalArmLengthAfterDecomposition; // sum of all arms.  This is computed twice, once before decomposition, and once after.  
    /*!
      A MetaArm is a chain of arms with all the same Burgers vector value, but can also include 200, 020, and 002 types as "unzipped" portions along the meta-arm.  
    */  
    std::vector<boost::shared_ptr<MetaArm> > mMetaArms; 

    std::string mMetaArmFile; 

    /*! 
      Number of nodes in full dump data
    */ 
    uint32_t mTotalDumpNodes; 

    /*!
      if this is true, then complete dumps of all data are done, 
      into files named mDebugOutputPrefix + <type> + ".debug"
    */ 
    bool mDoDebugOutput; 
    /*!
      By default, this is "./paradis-debug".  See mDoDebugOutput.
    */ 
    std::string mDebugOutputDir; 

    /*!
      The name of the data file that will be read.  
    */ 
    std::string mDataFilename; 

    /*!
      If set, then the file to be read is "old school" paraDIS data, of a slightly different format.  If there is a dataFileVersion string, then mFileVersion will reflect that.  Otherwise: 
      0 == "old style" -- probably not needed any more
      1 == "new style" -- before versioning was done
    */ 
    unsigned long mFileVersion;

    /*! 
      processor number for parallelism.  
    */ 
    int mProcNum; 
    /*!
      number of processors for parallelism.  If zero or one, then serial. 
    */ 
    int mNumProcs; 
    /*!
      A hack to get at some interesting linked loops for Moono Rhee. 
      If an arm is less than a certain length, then he assumes
      it is part of a particular loop configuration I call a "linked loop."  
    */ 
    double mThreshold; 
    /*!
      Moono would like to print out binned arm lengths.  He will give a number of bins and I will bin the arms into those many buckets when examining them at the end. 
    */ 
    int mNumBins; 
  };


} //end namespace paraDIS

#endif /* defined PARADIS_H */
