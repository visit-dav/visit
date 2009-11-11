/*! 
  \file paradis.h 

  \brief data structures and API for libparadis
 
      Definition, needed to find a node in a std::set<Node>:  node A > node B iff ( domain A > domain B ) || ( domain A == domain B && node ID A > node ID B ).  segment A > segment B iff segA.nodes[0] > segB.nodes[0].    


   ------------------------------------------------------------------  

      ALGORITHM OVERVIEW

   -- create all relevant IB, OOB, and IOOB nodes and related arm segments
   -- classify all arms as 100 or 111 
   -- discover all monster and butterfly nodes 

   ------------------------------------------------------------------  

   1)  DONE 
       Make a first pass through the file:  load all as MinimalNodes, with minimal info: {domain and ID, whether out of bounds, neighbor IDs}

  NOTES: 
   -- Need to use STL set instead of a vector, to make finds be logarithmic, else searching for neighbor nodes will be a big hit.  This is because node IDs are patchy within domains and duplicated across domains.  So minimal_node needs operator <();

   ------------------------------------------------------------------  

   2)  DONE 
       Mark all nodes as one of three possible MinimalNode::NodeTypes:
    A)  NoInfo -- Full information needed (FullNodes) (in bounds or direct neighbor to in bounds or terminal OOB node which terminates a collapsible chain of OOB interior segments).  
    B)  PartialInfo -- Partial information needed (PartialNodes) (need just position and neighbor info to compute the length of a collapsible chain of OOB interior segments)
    C)  NoInfo -- Not needed (none of the above applies) 

   ------------------------------------------------------------------ 
 
   3)  DONE:  cull unneeded MinimalNodes from memory.   Release the memory for the MinimalNeighbors -- it can be most easily just reobtained in the file pass on the next step.


   ------------------------------------------------------------------ 
 
   4)  CHANGED: Make another pass through file:  now as each node is seen, if it corresponds to an existing MinimalNode, then create a FullNode and delete the MinimalNode.  Also, create ArmSegments related to the FullNode if they don't exist, and add the FullNode as a pointer in the ArmSegment. 

   ------------------------------------------------------------------ 
 
   5) ADDED, and DONE BUT UNTESTED:  The data is periodic.  Go back and find all segments that connect two nodes such that if one is "wrapped" around any boundary of the data, it is closer to the other end than if it is not so wrapped.  Each such segment found requires creation of one new segment and two new "wrapped" nodes.  "wrapped" nodes are those created for this purpose, which lie on the intersection of the segment with the bounds of the subspace. When done, the wrapped segment, which normally would draw a crazy line all the way across the data, is replaced with two new segments that are one data space apart from each other. 

   ------------------------------------------------------------------  

   6)  DONE, BUT PROBABLY NEED TO FIX FOR NEW METHODS...  
   We can now create all arms for our region. 
   IN THE FUTURE, to save memory, we will implement the following: 
   Segments that connect two PartialNodes are PartialSegments.  Segments that connect two OOB FullNodes are CollapsedSegments.  Segments that connect an IB FullNode to another node are FullSegments.  Only FullSegments are drawn. 
   HOWEVER, For the first implementation, we will not distinguish between Partial and Full Segments, and we will not even collapse arms, so that we can get a good view of what things are like.  After it is clear that we are making sensible things, we can collapse arms to save memory. 

  ------------------------------------------------------------------  

   6)  DONE
   Go back and classify all terminal nodes as monsters and butterflies.  
   
  ------------------------------------------------------------------  

   7) NEW: 
   Go through all the segments and split any that cross the subspace boundaries into two half-segments, by creating a new node at the intersection of the segment and the data bounds and a new segment for the new second half of the now-split segment. .  

   ------------------------------------------------------------------ 
 
   8)  You now have a complete set of data for the given bounds.  Let the user (e.g. VisIt or povrayDumper) do the filtering to determine what gets drawn.  I could have done filtering within the library for performance reasons, but I don't think this is a good idea.  The bulk of the slowdown is in rendering for PovRay, and VisIt will do good filtering by itself.  

*/ 
#ifndef PARADIS_H
#define PARADIS_H


#define DEBUG 1

/* now for the API */  
#include <stdint.h>
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


/* ========================================  */ 
/*!
  Two special FullNode types, for special occasions.  PLACEHOLDER_NODE is used in ArmSegments for temporary nodes not yet seen in the data file.  USELESS_NODE is an out-of-bounds node without any in-bounds neighbors.   
*/ 
#define PLACEHOLDER_NODE (-42)
#define USELESS_NODE (-43)
/* ========================================  */ 

//=============================================================================
namespace paraDIS {
  class FullNode; 
  /*!
    Unary predicate for trying to delete nodes.  this is way harder than it should be. 
  */ 
  bool NodeIsUseless(FullNode *node);

  //============================================================
  class NodeID {  
  public: 
    NodeID() {mDomainID = mNodeID = -1; }
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
      return mDomainID == other.mDomainID && mNodeID == other.mNodeID;
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
              mDomainID == other.mDomainID && 
              mNodeID < other.mNodeID);
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
    std::string Stringify(void) const {
      std::string s("NodeID: (domain id, node id) = (");
      s += doubleToString(mDomainID);
      s +=  ", ";
      s += doubleToString(mNodeID);
      s += ")";
      return s; 
    }

  private:
    int16_t mDomainID;      
    int32_t mNodeID; 
  };    


  //===========================================================================
  struct Neighbor; // forward declaration
  
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
    virtual std::string Stringify(void) const; 

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
  /*!  struct Neighbor
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
        mEndpoints[0] == other.mEndpoints[0] && 
        mEndpoints[1] < other.mEndpoints[1]; 
    }
    /*! 
      conversion of Neighbor to string
    */ 
    virtual std::string Stringify(void) const {
      return std::string("Neighbor: endpoints = (") + 
        mEndpoints[0].Stringify() + string(", ")  +  mEndpoints[1].Stringify() + string(")"); 
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
    virtual std::string Stringify(bool showneighbors=false) const {
      std::string s =std::string("MinimalNode: ")+Node::Stringify() + string("\nClassification: ") + string(mKeep?"KEEP":"DON'T KEEP") + string("\n");  
      s += (doubleToString(mNeighbors.size()) + " neighbors --------------:\n");
      uint32_t n = 0; 
      if (showneighbors) {
        while (n < mNeighbors.size()) {
          s += "neighbor " + doubleToString(n) + ": "; 
          if (mNeighbors[n]) {
            s += mNeighbors[n]->Stringify() + "\n"; 
          } else {
            s += "(NULL)\n"; 
          }
          ++n;
        }
      }
      return s;
    } 
    /*!
      Accessor function
    */ 
    int GetNumNeighbors(void) const { return mNeighbors.size(); }
    /*! 
      Accessor function --  returns const due to the fact that neighbors are stored in a set in the data set.  But you can always create a copy of what you get, or use const_cast<> on it. 
    */ 
    const Neighbor *GetNeighbor(uint32_t num)  const { 
      if (num >= mNeighbors.size()) 
        throw std::string("subscript out of range in BaseNode::GtNeighbor"); 
      return mNeighbors[num]; 
    }

   
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
      if (skipneighbors) mNeighborSegments.clear(); 
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
    float GetLocation(int index) { return mLocation[index]; }

    /*!
      Accessor function
    */ 
    const float *GetLocation(void) {
      return mLocation; 
    }

    /*!
      Accessor
    */
    void GetLocation(float oLoc[3]) {
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
    double Distance(const FullNode &other) { 
      double dx = mLocation[0]-other.mLocation[0],  
        dy = mLocation[1]-other.mLocation[1],        
        dz = mLocation[2]-other.mLocation[2];         
      
      return sqrt(dx*dx + dy*dy + dz*dz); 
    } 
    /*!
      Accessor function.  
      \param itype If given, sets the type.  If this is not given, then the node will attempt to determine its own type.  
    */ 
    void SetNodeType(int8_t itype=-1); 
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

    /*!
      Convert FullNode to string
    */ 
    virtual std::string Stringify(bool showneighbors=false) const;

    /*!
      Add the arm segment to our list of neighbors  
    */
    void AddNeighbor( ArmSegment *segment) {
      mNeighborSegments.push_back(segment); 
    }
     
    /*!
      Replace a neighbor of this node with a new neighbor -- done when wrapping nodes
    */ 
    void ReplaceNeighbor (ArmSegment *oldseg,  ArmSegment *newseg) {
      int segnum = mNeighborSegments.size(); 
      while (segnum--) {
        if (oldseg == mNeighborSegments[segnum]) {
          mNeighborSegments[segnum] = newseg;
          return; 
        }
      }
      throw string("Error: segment to replace is not a neighbor of this node\n"); 
    }

    /*!
      Assuming we two neighbors or less, return the neighbor not passed to us
    */ 
    ArmSegment *GetOtherNeighbor (const ArmSegment* n);
    
      
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
    }

    /*!
      Accessor function
    */ 
    int GetNumNeighbors(void) const { return mNeighborSegments.size(); }
    /*! 
      Accessor function --  returns const due to the fact that neighbors are stored in a set in the data set.  But you can always create a copy of what you get, or use const_cast<> on it. 
    */ 
    const ArmSegment *GetNeighbor(uint32_t num)  const { 
      if (num >= mNeighborSegments.size()) 
        throw std::string("subscript out of range in BaseNode::GtNeighbor"); 
      return mNeighborSegments[num]; 
    }

   
    /*! 
      Accessor function
    */ 
    const std::vector< ArmSegment *> &GetNeighbors(void) const {
      return mNeighborSegments; 
    }

    /*!
      Accessor function
    */
    int32_t GetIndex(void) { return mNodeIndex; }
    
    /*!
      Accessor
    */ 
    void SetIndex(int32_t index) { mNodeIndex = index; }

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
    
    /*!
      This is needed for things like Visit, where nodes are accessed by node ID, whereas this library uses pointers.  Since it is also our index inthe global array of nodes, it turns out to be a good way to get the "real" counterpart of a wrapped node.  
    */ 
    int32_t mNodeIndex; 
    /*!
      Connectivity to our neighboring nodes encapsulated in ArmSegments
    */ 
    std::vector< ArmSegment *> mNeighborSegments; 

    /*!
      Static member to keep track of subspace bounds for checking if we are in bounds or not
    */ 
    static rclib::Point<float> mBoundsMin, mBoundsMax; 

  }; /* end FullNode */  
  

  //=============================================
#define ARM_UNKNOWN 0
#define ARM_UNINTERESTING 1
#define ARM_LOOP 2
#define ARM_MM_111 3 
#define ARM_MN_111 4
#define ARM_NN_111 5 
#define ARM_MM_100 6
#define ARM_MN_100 7
#define ARM_NN_100 8
  /*! 
    Arm segments are like Neighbors in that they contain neighbor relationships, but these are encoded as pointers to nodes instead of NodeIDs, for faster access to complete node data as needed. They also contain burgers and arm-type information for later analysis.  I almost called them "FullNeighbor", but in common terminology they are called "arm segments," since one or more of them comprise an Arm, so I just called them that.  
  */ 
  class ArmSegment {

  public: 
    ArmSegment():mBurgersType(0), mMNType(ARM_UNKNOWN), mSeen(0){ init(); }
    ArmSegment(const ArmSegment &other){
      init(); 
      *this = other; 
    }

    void init(void) {
      mBurgersType = 0; 
      mSeen = false; 
      int i=2; while (i--) {
        mEndpoints[i] = NULL; 
      }
      mGhostEndpoints.clear(); 
#ifdef DEBUG
      mSegmentID = -1; 
      mWrapped = false; 
#endif
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
      For each wrapped segment, there is an identical unwrapped segment with the same node ID's.  One of each such pair of segments has a ghost endpoint with ID greater than one of its nodes and equal to the other, and one has a ghost endpoint with ID less than one of its nodes and equal to the other. This function returns true for one of them and not the other.  
    */
    bool Cullable(void) { 
      if (mGhostEndpoints.size() > 1) {
        throw string("Error:  I don't know how there can be more than one ghost endpoint for any segment"); 
      }
      if (!mGhostEndpoints.size()) return false; 
      return (*(mGhostEndpoints[0]) < *(mEndpoints[0]) || 
              *(mGhostEndpoints[0]) < *(mEndpoints[1]));
    }

    
    int8_t GetBurgersType(void) const { return mBurgersType; } 

    /*! 
      Accessors for MN type values (set by parent arm)
    */ 
    int8_t GetMNType(void) const { return mMNType; } 

    void SetMNType(int8_t val)  {  mMNType=val; } 
    /*!
      Return the distance between the endpoints
    */
    double GetLength(void) const { 
      return mEndpoints[0]->Distance(*mEndpoints[1]); 
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
#ifdef DEBUG
      mSegmentID = mNextID++; 
#endif
      return; 
    }
    /*!
      accessor -- noop if not debug mode
    */ 
    int32_t GetID(void) {
#ifdef DEBUG
      return mSegmentID; 
#else
      return -1; 
#endif
    }
      
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
    std::string Stringify(void) const {
      string s(string("ArmSegment at ") + pointerToString(this) +  
#ifdef DEBUG
               " number " + doubleToString(mSegmentID) + 
#endif
               ": \nBurgersType: " + doubleToString(mBurgersType)+", length: "+doubleToString(GetLength())+"\n");
      uint32_t epnum = 0; while (epnum < 2) {
        s+= "ep"+doubleToString(epnum)+": "; 
        if (mEndpoints[epnum]) s+= mEndpoints[epnum]->Stringify(); 
        else s+= "(NULL)"; 
        epnum++; 
      }
      epnum = 0; while (epnum < mGhostEndpoints.size()) {
        s+= "GHOST ep"+doubleToString(epnum)+": ";
        if (mGhostEndpoints[epnum]) {
          s+=mGhostEndpoints[epnum]->Stringify(); 
        } else {
          s += "(NULL)";
        } 
        ++epnum; 
      }
      return s; 
    }

    /*! 
      Given the float values for the burgers vector from the data set, set the burgers type for the arm segment 
    */ 
    void ComputeBurgersType(float burg[3]); 
   
       
    /*!
      SetEndpoint -- given an actual new FullNode and ID information for an as-yet uncreated neighborNode, we set the endpoints to be the address of the actualNode and the address of an internal placeholder, a copy of neighborNode.  We will delete the placeholder FullNode later, either when the actual FullNode is created, or when our destructor is called.
    */ 
    void SetEndpoints(FullNode *actualNode, int16_t neighborDomain, int32_t neighborNodeID) {
      FullNode *placeholder = new FullNode(neighborDomain, neighborNodeID); 
      placeholder->SetNodeType(PLACEHOLDER_NODE); //mark as a dummy node; 
      placeholder->SetIndex(-42); 
      SetEndpoints_internal(actualNode, placeholder); 
      return ;
    }

    /*! 
      Called by a node as part of bookkeeping when wrapping a segment.  Make sure the given node is one of our neighbors -- add it if not
    */ 
    void ConfirmEndpoint(FullNode *ep)  {
      if (mEndpoints[0] == ep || mEndpoints[1] == ep) return ; 
      int epnum = mGhostEndpoints.size(); 
      while (epnum --) {
        if (mGhostEndpoints[epnum] == ep) return;         
      }
      mGhostEndpoints.push_back(ep); 
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
          dbprintf(5, "ReplacePlaceholder in segment %s replacing ep with actualNode %s\n", this->Stringify().c_str(),  actualNode->Stringify().c_str()); 
          delete mEndpoints[epnum];
          mEndpoints[epnum] = actualNode; 
          return; 
        }
      }
      throw string("Error in SetActualEndpoint: There is no placeholder to replace.  Segment is ")+this->Stringify() + " and actualNode at "+pointerToString(actualNode)+" is " + actualNode->Stringify(); 
      return; // ha ha
    }
      
    /*! 
      This is called only for wrapped segments, when you need to replace a "real" node pointer with a pointer to a wrapped node.  We still need to keep the old pointer to the real node to notify it if we are deleted. 
    */     
    void ReplaceEndpoint(FullNode *oldEP, FullNode *newEP) {
      if (mEndpoints[0] == oldEP) {
        mGhostEndpoints.push_back(mEndpoints[0]); 
        mEndpoints[0] = newEP; 
      } else if (mEndpoints[1] == oldEP) {
        mGhostEndpoints.push_back(mEndpoints[1]); 
        mEndpoints[1] = newEP; 
      } else {
        throw string("Error in ReplaceEndpoint -- there is no such endpoint"); 
      }
      return;
    }

    /*!
      Just immediately returns true if a useless endpoint is in mEndpoints, which means this segment itself is useless!
      If not, then  goes through and removes all the leftover crappy useless endpoints from our ghost endpoints, since they will just be deleted and become dangling references anyhow.  The returns false. 
    */ 
    bool IsUseless(void) const {
      if( mEndpoints[0]->GetNodeType() == USELESS_NODE ||
          mEndpoints[1]->GetNodeType() == USELESS_NODE || 
          (!mEndpoints[0]->InBounds() && ! mEndpoints[1]->InBounds())) {  
        return true; 
      }
      vector<FullNode*>::iterator nodepos = 
        remove_if(const_cast<ArmSegment*>(this)->mGhostEndpoints.begin(), const_cast<ArmSegment*>(this)->mGhostEndpoints.end(), NodeIsUseless); 
      const_cast<ArmSegment*>(this)->mGhostEndpoints.erase(nodepos, const_cast<ArmSegment*>(this)->mGhostEndpoints.end()); 
      return false; 
    }

    /*! 
      Given the size of the periodic bounds of the data, determine if the nodes are closer if you wrap one of them.  If not, just return false and set oNewSegment to NULL.   
      It will be true if the segment crosses any face of the periodic data bounds. If the segment crosses one face, it must also cross a second.  So if there is a crossing, create two new nodes, representing the node reflected so as to represent the closest neighbor in all bounding data periods.  Match endpoint[1] of (*this) with the nearest of these new nodes and create a new segment containing our old endpoint[1] and its new neighbor node.  Replace endpoint[1] in (*this) with the other node.  Return true and set oNewSegment to point to the new segment.  
      These new wrapped nodes will be the same as their "real" counterparts, but have a negative nodeID, and of course a different location from wrapping, and one of their neighbor segments may be different.  The new segment will simply have a pointer to a real node and a pointer to a wrapped node.  This difference only affects BuildArms and where it is important to traverse an entire arm.  In those cases, when a wrapped node is seen, its equivalent can be gotten by referencing mFullNodes(wrappedNode->GetIndex()); 
    */ 
    bool Wrap(const rclib::Point<float> &dataSize, ArmSegment *&oNewSegment, 
              FullNode *&oWrapped0, FullNode *&oWrapped1);
    
    
    /*! 
      Accessor function.  
      \param epnum Must be 0 or 1
    */  
    FullNode *GetEndpoint(int epnum) const { return mEndpoints[epnum]; }

      
    /*!
      Common Accessor operation: we have one node ID, but we're looking to see what the other end of the segment is -- we cannot operate on nodes, because we don't have the global node list, so we use NodeID. 
    */ 
    FullNode *GetOtherEndpoint(const FullNode *node) const {
      if (*mEndpoints[0] == *node) 
        return mEndpoints[1];
      else if (*mEndpoints[1] == *node) 
        return mEndpoints[0]; 
      throw string("Error in GetOtherEndpoint: cannot find node corresponding to ")+node->Stringify(); 

    }

#ifdef DEBUG
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
#endif

  protected:
    /*! 
      Store the given nodes as endpoints, properly ordered to allow searching in sets and hashes. 
    */ 
    void SetEndpoints_internal(FullNode *node1, FullNode *node2) {
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

    int Category(float num) {
      if (!num)
        return 0;
      else if (fabs(fabs(num) - 1.15) < 0.01 || fabs(fabs(num) - 1.00) < 0.01) 
        return 1;
      else if (fabs(num - 0.577) < 0.001 || fabs(num - 0.500) < 0.001)
        return 2;
      else if (fabs(num + 0.577) < 0.001 || fabs(num + 0.500) < 0.001)
        return 3;
      dbprintf(2, "\n\n********************************\n");
      dbprintf(2, "WARNING: Weird value %g encountered in Category\n", num);
      dbprintf(2, "\n********************************\n\n");
      return 4;
    }
     /*!
      The burgers-type is as follows (A 1 means a value +/-1.14, a + means +0.577 and - means -0.577):      
      1 = "1 0 0"
      2 = "0 1 0"
      3 = "0 0 1"
      4 = "+ + +" or "- - -" 
      5 = "+ + -" or "- - +" 
      6 = "+ - +" or "- + -" 
      7 = "- + +" or "+ - -" 
      8 = none of the above
    */ 
    int8_t mBurgersType; 
    
    /*! 
      The MN_type of the segment is set by its parent arm.  See Arm struct for definitions, but it describes whether the segment is 100 or 111 and whether its parent arm has any monsters at either end. 
    */ 
    int8_t mMNType; 
     
      
    /*!
      Marker used for "once-through" operations like building arms that must look at every segment, but which will usually discover echo particular segment more than once 
    */ 
    bool mSeen;   
    /*!
      Pointers to actual nodes, as opposed to just NodeID's as in Neighbors.  
    */ 
    FullNode * mEndpoints[2]; 
  public:
    /*!
      We usually need two slots for endpoints, but may need extra slots for "ghost endpoints" created when nodes are wrapped.  When this segment is deleted, it goes through its endpoints and tells all of them they are gone.  But wrapping causes some segments to be the neighbor of 3 or even (very rarely) more nodes. So we need to track those special cases. 
    */ 
    std::vector<FullNode*>mGhostEndpoints; 
      
  }; /* end ArmSegment */ 

 
  // ==============================================
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
    Arm():mArmType(0)
    {return; }
    /*!
      Clear all data from the Arm for reuse
    */ 
    void Clear(void) { 
      mTerminalSegments.clear();  
      mTerminalNodes.clear(); 
      mArmType = 0; 
#ifdef DEBUG
      mNumSegments = 0; 
#endif 
    }
    /*! 
        create a string out of the arm
    */ 
    std::string Stringify(void) const ; 

    /*!
      to set the arm ID for debugging.  In Debug code, this will do nothing
    */ 
    void SetID(void) { 
#ifdef DEBUG
      mArmID = mNextID++; 
#endif
      return; 
    }

    /*!
      Classify the arm as one of NN, MN or MM, combined with 100 or 111...
    */ 
    void Classify(void) ; 
    /*!
      If either terminal segment is useless,mark it as NULL and return true. 
      Else, return false.
    */ 
    bool IsUseless(void) {
     int segnum =  mTerminalSegments.size(); 
     bool useless = false; 
      while (segnum--) {
        if (mTerminalSegments[segnum]->IsUseless()) {
          useless = true; 
          mTerminalSegments[segnum] = NULL; 
        }
      }
      int nodenum = mTerminalNodes.size(); 
      while (nodenum--) {
        if (mTerminalNodes[nodenum]->GetNodeType() == USELESS_NODE) {
          mTerminalNodes[nodenum] = NULL; 
        }
      }
      return false; 
    }
               
    /*! 
      Return the sum of the length of all segments in the arm
    */ 
    double GetLength(void); 

    /*! 
      Check to see if this is the body of a "butterfly," which is two three armed nodes connected by a type 100 arm, and which have four uniquely valued type 111 exterior arms ("exterior" means the arms not connecting the two).  If so, mark each terminal node as -3 (normal butterfly.  If one of the terminal nodes is a type -44 "special monster" node, then mark the other terminal node as being type -33 ("special butterfly"). 
    */ 
    void CheckForButterfly(void); 
    /*! 
      This is a necessary component to CheckForButterfly, broken out for readability in the code. 
    */ 
    bool HaveFourUniqueType111ExternalArms(void); 
    vector <const ArmSegment *> mTerminalSegments; // At least one, but not more than two
    vector <FullNode *> mTerminalNodes;  // At least one, but not more than two
    int8_t mArmType; 

#ifdef DEBUG
    /*! 
      purely for debugging
    */ 
    int32_t mArmID; 
    /*! 
      purely for debugging
    */ 
    static int32_t mNextID; 
    /*!
      statistics for debugging
    */ 
    uint32_t mNumSegments; 
#endif
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
    void SetDebugOutputDir(std::string dir) { mDebugOutputDir = dir; }

    /*!
      verbosity goes from 0-5, based on dbg_setverbose() from librc.a
      filename if null means stderr
    */ 
    void SetVerbosity(int level, const char *filename) { 
      if (filename) dbg_setfile(filename); 
      dbg_setverbose(level); 
    }

    /*!
      Print out arm statistics
    */
    void PrintArmStats(void); 
      

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
 
    /*!
      Accessor function.
    */ 
    uint32_t GetNumNodes(void) { return mFullNodes.size(); }
    /*!
      Accessor returns begin of node vector. 
    */ 
    vector<FullNode *>::const_iterator GetNodesBegin(void) { 
      return mFullNodes.begin(); 
    }
    /*!
      Accessor returns end of node vector. 
    */ 
    vector<FullNode *>::const_iterator GetNodesEnd(void) {
      return mFullNodes.end(); 
    }

   /*! 
      Accessor function. 
    */ 
    FullNode * GetNode(uint32_t nodenum) { return mFullNodes[nodenum]; }

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
        Free up all memory and go back to virgin state. 
    */ 
    void Clear(void) {
      init(); 
    }

    //=======================================================================
    // PRIVATE 
    //======================================================================
  private:
    /*!
      Initialize things to virginal values. 
    */ 
    void init(void) {    
      mFileVersion = 0; 
      mMinimalNodes.clear(); 
      mMinimalNeighbors.clear(); 
      vector<FullNode *>::iterator nodepos = mFullNodes.begin(), nodeend = mFullNodes.end(); 
      while (nodepos != nodeend) delete *nodepos++; 
      mFullNodes.clear(); 
      
      nodepos = mWrappedNodes.begin(), nodeend = mWrappedNodes.end(); 
      while (nodepos != nodeend) delete *nodepos++; 
      mWrappedNodes.clear(); 
      
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
      Prints out all full nodes 
    */ 
    void DebugPrintFullNodes(char *name = NULL); 
   /*! 
      Prints out all arms
    */ 
    void DebugPrintArms(void);
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
      Create all arms for our region. This function is a bit long because we are avoiding recursion by using loops.  Recursion for these arms would get pretty deep. 
      IN THE FUTURE, to save memory, we will implement the following: 
      Segments that connect two PartialNodes are PartialSegments.  Segments that connect two OOB FullNodes are CollapsedSegments.  Segments that connect an IB FullNode to another node are FullSegments.  Only FullSegments are drawn. 
      HOWEVER, For the first implementation, we will not distinguish between Partial and Full Segments, and we will not even collapse arms, so that we can get a good view of what things are like.  After it is clear that we are making sensible things, we can collapse arms to save memory. 
    */ 
    void BuildArms();

   /*!
    Starting at the given first node and heading out in the direction of the given first segment in an arm, trace along the arm until you find its other endpoint (terminal segment and node).  When wrapped nodes are found, use their real counterparts instead.  
    This will be where we actually discriminate between node types, etc.  But as mentioned for BuildArms, we don't do that yet.  
  */ 
    void FindEndOfArm(FullNodeIterator &firstNode, FullNode **oFoundEndNode, 
                      const ArmSegment *firstSegment, const ArmSegment *&foundEndSegment
#ifdef DEBUG
, Arm &theArm
#endif
);
      
    /*!
      Classify arms and nodes as butterflies
    */ 
    void FindButterflies(void); 

    /*!
      Make another run through the arms now that we know all the butterflies, and put the final classification on them as ARM_XX_YYY.  
    */ 
    void ClassifyArms(void); 

    /*!
      Identify all useless nodes, which are out of bounds and have no in-bounds neighbors.  Then delete all segments connecting two useless nodes.  Finally, delete all useless nodes.  
    */ 
    void DeleteUselessNodesAndSegments(void); 


    /*!
      Go through and renumber the nodes so that their index is the same as their position in the vector
    */ 
    void RenumberNodes(void); 

    /*!
      Extents of the full data set: 
    */ 
    rclib::Point<float> mDataMin, mDataMax, mDataSize;
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
      all fullnodes in the data set.  DO NOT SORT THIS -- each node has an Index which is its position in this array when created -- used to find its counterpart in the wrapped nodes for tracing along arms to find their endpoints and lengths.  
    */ 
    std::vector<FullNode *> mFullNodes; 
    
    /*!
      Use a set of node pointers to know what to delete.  This complication is because it looks like remove_copy_if() is broken in g++ 3.2.3
    */ 
    std::vector<FullNode *>mUselessNodes; 

    /*!
      to avoid memory leaks, track all these in one place
    */ 
    std::vector<FullNode *> mWrappedNodes; 

    /*!
      set:  always sorted so fast, but in order to modify, must remove an element, modify it, and reinsert it... or use const_cast<> 
    */ 
    set<ArmSegment *, CompareSegPtrs> mQuickFindArmSegments; 
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
    std::vector<Arm> mArms; 
    
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
  };


} //end namespace paraDIS

#endif /* defined PARADIS_H */
