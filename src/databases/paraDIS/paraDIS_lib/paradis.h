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
using boost::uint8_t;
using boost::uint32_t;
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

#include <stdio.h>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <ostream> 
#include <math.h>

#include "Point.h"
#include "pathutil.h"
#include "stringutil.h" /* from RC_c_lib, is this a good idea? */ 
#include "debugutil.h" /* from RC_c_lib, now we're committed. */ 

#ifdef RC_CPP_VISIT_BUILD
#define dbprintf dbstream_printf
void dbstream_printf(int level, const char *fmt, ...);
#endif

std::string GetLibraryVersionString(const char *progname);
std::string GetLibraryVersionNumberString(void);


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
  extern std::string doctext;

  //===========================================================================
  class Neighbor; // forward declaration
  
  struct NodeID {
    // -------------------------------------------------
    NodeID() {
      mDomainID = -1; mNodeID = -1; 
    }
    // -------------------------------------------------
    NodeID(const NodeID &other) {
      mDomainID = other.mDomainID;
      mNodeID = other.mNodeID;
    }
    // -------------------------------------------------
    NodeID(int16_t domain, int32_t node) {
      mDomainID = domain; 
      mNodeID = node; 
    }
    // -------------------------------------------------
    uint64_t Hash(void) const {
      if (mNodeID >= 1000*1000) {
        cerr << "Warning: Hash() is no longer unique" << endl; 
      }
      return mDomainID * 1000*1000 + mNodeID; 
    }
    // -------------------------------------------------
    std::string Stringify(int indent) const {      
      return INDENT(indent) + str(boost::format("NodeID: (%1%,%2%)")% mDomainID % mNodeID);
    }
    
    uint32_t mDomainID; 
    uint64_t mNodeID; 
  };
  
  /*! 
    class Node
    \brief Abstract base class
  */
  
  class Node { 
    //-----------------------------------------------------
    // public API
  public: 
    Node() :mInBounds(false) { 
      return; 
    }
      // 
    Node(const NodeID &id):mID(id), mInBounds(false) { 
      return; 
    }
    
    /*! 
      virtual destructor assures proper deletion
    */ 
    virtual ~Node() {}
      
        /*!
          Used in comparing data read from the file with the current Node. 
        */ 
        bool operator ==(const NodeID &compare) const {
      return this->mID.Hash() == compare.Hash(); 
    }
    /*!
      Used for ordering sets?  Might not be needed. 
    */ 
    bool operator ==(const Node &other) const {
      return this->mID.Hash() == other.GetNodeID().Hash(); 
    }
    /*!
      Used for ordering vectors, etc. 
    */ 
    bool operator <(const Node &other) const {
      return this->mID.Hash() < other.GetNodeID().Hash(); 
    }
    
    // return a hash based on NodeID
    uint64_t Hash(void) const {
      return mID.Hash(); 
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
    void SetNodeID(const NodeID &id) {
      mID = id; 
      return ;
    }
    /*!
      Accessor function
    */ 
    const NodeID &GetNodeID(void) const  { return mID; }

    int32_t GetNodeSimulationDomain(void) const { return mID.mDomainID; }

    int32_t GetNodeSimulationID(void) const { return mID.mNodeID; }

    /*! 
      conversion of Node to string
    */ 
    virtual std::string Stringify(int indent) const; 

    //-----------------------------------------------------   
  protected:
    /*!
      needed for hashing
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
      if (ep1.Hash() == ep2.Hash()) {
        throw std::string("Error in Neighbor::SetEndpoints: both endpoints are the same!"); 
      }
      if (ep1.Hash() < ep2.Hash()) {
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
      if (mEndpoints[0].Hash() == id.Hash()) return mEndpoints[1]; 
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
      return mEndpoints[0].Hash() == other.mEndpoints[0].Hash();
    }
     
    /*!
      for sorting
    */ 
    bool operator <(const Neighbor&other) const {      
      return mEndpoints[0].Hash() < other.mEndpoints[0].Hash(); 
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

    // Bookkeeping for timestep changes etc.  
    static void Clear(void) {
      mMinimalNodes.clear(); 
      return;
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

    /*! 
      can modify but must sort and insert/delete are slow
    */ 
    static std::vector<MinimalNode> mMinimalNodes; 

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
    /*!
      ===========================================
      constructor
    */ 
    FullNode() : Node() {
      init(); 
      return; 
    }
   
    /*!
      ===========================================
      constructor
    */ 
    FullNode(const FullNode &other, bool skipneighbors=false) {
      init(); 
      uint32_t saved = mNodeIndex; 
      *this = other; 
      mNodeIndex = saved; 
      if (skipneighbors) {
        mNeighborSegments.clear();         
        mNeighborArms.clear(); 
      }
      return;
    }
    /*!
      ===========================================
      destructor
    */ 
    ~FullNode()  {
      if (mFullNodes.find(mNodeIndex) != mFullNodes.end()){
        mFullNodes.erase(mNodeIndex); // mFullNodes[mNodeIndex] = NULL; 
      }
      return; 
    }
    
    /*!
      ===========================================
      constructor
    */ 
    FullNode(int16_t domain, int32_t nodeID) {
      init(); 
      mID = NodeID(domain, nodeID); 
    }
    /*!
      ===========================================
      Clear all nodes
    */ 
    static void Clear(void) {
      for (map<uint32_t, FullNode *>::iterator nodepos = mFullNodes.begin(); nodepos != mFullNodes.end(); nodepos++) {
        delete nodepos->second;
      }
      mFullNodes.clear(); 
      mFullNodeVector.clear(); 
      mTraceFileBasename = "";
      mTraceNodes.clear(); 
      mNextNodeID = 0; 
    }
    
    /*!
      ===========================================
      initializer
    */
    void init(void) {
      mNodeIndex = mNextNodeID; 
      mNextNodeID++;       
      mFullNodes[mNodeIndex] = this; 
      mInBounds = false; 
      mLocation.resize(3,0); 
      mNodeType = 0; 
      mIsLoopNode = false;
      mWrappedCopy = false; 
      mSeen = false; 
    }
    /*!
      ===========================================
      Full nodes are first copied from MinimalNodes.  This is just a member-wise "shallow" copy.  
    */       
    const FullNode &operator =(Node &otherNode) {
      mID = otherNode.mID; 
      mInBounds = otherNode.mInBounds; 
      return *this; 
    } 
    
    /*!
      ===========================================
      Check to see if the node's internal structures make sense.
      Assumes all data has been parsed.
    */       
    bool Test(void);
       
       
    /*!
      ===========================================
      Add the given arm to the list of arms to trace. 
      See WriteTraceFiles()
    */ 
    static void TraceNode(int32_t nodeID){
      mTraceNodes.push_back(nodeID);       
    }

    /*!
      ===========================================
      Give a name to distinguish tracefiles from files from other runs.
    */ 
    static void SetTraceFileBasename(string basename) {
      mTraceFileBasename = basename; 
    }
    
   /*!
      ===========================================
      Accessor function 
    */ 
    void SetInBounds(void) {
      mInBounds = rclib::InBounds(rclib::Point<float>(mLocation), mBoundsMin, mBoundsMax );
    }
    
    
    /*!
      ===========================================
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
    vector<float> GetLocation(void) const { return mLocation; }
    
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
        mNeighborSegments.erase(find(mNeighborSegments.begin(), mNeighborSegments.end(), oldseg)); 
     }
      //ComputeNodeType(); 
    }
    
    /*!
      Handy wrapper
    */ 
    static void PrintAllNodeTraces(string stepname); 

  /* 
     For each node that we want to trace, we write one set of files out before any decompositions, then one for decomposition step of each arm containing this node.
   */ 
    void WriteTraceFiles(string stepname);

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
        // assumes that the arm exists in mNeighborArms! 
        mNeighborArms.erase(find(mNeighborArms.begin(), mNeighborArms.end(), neighbor)); 
      }
    }

    bool HasNeighbor(struct Arm *neighbor) {
      return (find(mNeighborArms.begin(), mNeighborArms.end(), neighbor) != mNeighborArms.end());
    }

    void AddNeighbor(struct Arm *neighbor) {
      mNeighborArms.push_back(neighbor);       
    }
        
    /*! 
      Accessor function
    */ 
    const std::vector< int> GetNeighborArmIDs(void) const;


    /*!
      Identify arms which cross over this node for  DetachCrossArms(); 
      Broken out separately to enable query and debug output
    */ 
    vector<vector<Arm *> >IdentifyCrossArms(void); 

    /*!
      Identify arms which cross over this node and glue them together. 
      Simplifies decomposition of arms.  
    */ 
    void DetachCrossArms(void); 


   /*!
      Accessor function
    */
    int32_t GetIndex(void) { return mNodeIndex; }
 
    string GetNodeIDString(void) { return GetNodeID().Stringify(0); }

    /*!
      Accessor
    */ 
    void SetIndex(int32_t index) { 
      mNodeIndex = index; 
    }
    
    bool IsWrappedCopy(void) { return mWrappedCopy; }
    
    void SetWrappedCopy(bool tf) { mWrappedCopy = tf; }

    /*!
      for debugging
    */ 
    bool mSeen; 


    /*!
      Connectivity to our neighboring arms.
      Only used for terminal nodes.  
    */ 
    std::vector<struct Arm *> mNeighborArms; 

    /*! 
      all fullnodes in the data set.  DO NOT SORT THIS -- each node has an Index which is its position in this array when created -- used to find its counterpart in the wrapped nodes for tracing along arms to find their endpoints and lengths.  
    */ 
    static std::map<uint32_t, FullNode *> mFullNodes; 
    static std::vector<FullNode *> mFullNodeVector; 

    static uint32_t mNextNodeID; 
    
    /*!
      Static member to keep track of subspace bounds for checking if we are in bounds or not
    */ 
    static rclib::Point<float> mBoundsMin, mBoundsMax, mBoundsSize; 
    //===============================================================
    // PRIVATE
    //===============================================================
  private:
    /*! 
      Absolute location in global space
    */ 
    vector<float> mLocation; 
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

    static string mTraceFileBasename; 
    static vector<uint32_t> mTraceNodes; 

    // statistics:
    std::vector<uint32_t> mNumMonsterNodes; 

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
      uint32_t saved = mSegmentID; 
      *this = other; 
      mSegmentID = saved; 
    }

    void init(void) {
      mSegmentID = mNextSegmentID; // mArmSegments.size();
      mArmSegments[mSegmentID] = this; //.push_back(this); 
      mNextSegmentID++; // mNumArmSegments++; 

      mBurgersType = 0; 
      mSeen = false; 
      int i=2; while (i--) {
        mEndpoints[i] = NULL; 
      }
      mGhostEndpoints.clear(); 
      mWrapped = false; 
      mNumDuplicates = 0; 
    }
    /*!
      Destructor
    */ 
    ~ArmSegment() {
      mArmSegments.erase(mSegmentID); // mArmSegments[mSegmentID] = NULL; 
      //mNumArmSegments--; 
     int i=2; while (i--) {
        if (mEndpoints[i] && mEndpoints[i]->GetNodeType() == PLACEHOLDER_NODE) {
          delete mEndpoints[i]; // memory leak fix
        }
      }
      return; 
    }
    
    static void Clear(void) {

      for (map<uint32_t, ArmSegment *>::iterator pos = mArmSegments.begin(); pos != mArmSegments.end(); ++pos) {
        delete pos->second; 
      }
      mArmSegments.clear(); 
      mArmSegmentVector.clear();
      
      mSegLen = 0; 
      mNumClassified = 0; 
      mNumWrapped = 0; 
      mNumArmSegmentsMeasured = 0; 
      mNextSegmentID = 0; 
    }

    /*!
      Accessor returns begin of arm vector. 
    */ 
    /* static vector<ArmSegment *>::const_iterator GetArmSegmentsBegin(void) { 
      return mArmSegments.begin(); 
      }*/
    /*!
      Accessor returns end of arm vector. 
    */ 
    /*
    static vector<ArmSegment *>::const_iterator GetArmSegmentsEnd(void) {
      return mArmSegments.end(); 
    }
    */

    /*!
      Accessor function
    */
    /*static ArmSegment *GetArmSegment(uint32_t segnum) { 
      return mArmSegments[segnum]; 
      }*/ 

    /*! 
      Accessor function. 
    */ 
    /* static uint32_t GetNumArmSegments(void)  { 
      return mArmSegments.size(); 
    }
    */
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
    void SetIndex(uint32_t id) {
      mSegmentID = id; 
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
        string err = str(boost::format("Error in SetEndpoints -- endpoints are identical!  Endpoints are %1% and %2%")%node1->Stringify(0)%node2->Stringify(0)); 
        dbprintf(0, (err + "\n").c_str()); 
        throw err;
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
      This is called  for wrapped segments, when you need to replace a "real" node pointer with a pointer to a wrapped node.  We still need to keep the old pointer to the real node to notify it if we are deleted. 
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
      to find out if wrapped
    */
    bool mWrapped; 
    
    /*! 
      If this segment is part of an extended arm, that's useful to know for visualization as there will be two or more superposed segments in the same place. 
    */ 
    int mNumDuplicates; 


    static double mSegLen;
    static uint32_t mNumClassified, mNumWrapped, mNumArmSegmentsMeasured; 
    static uint32_t mNextSegmentID; 

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
    /*!
      The global list of valid arm segments
    */ 
    static std::map<uint32_t, ArmSegment *> mArmSegments; 
    static vector<ArmSegment *> mArmSegmentVector; 

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
  /*  class ArmSegmentHash {
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
  */
  //==============================================
  /*! 
    Arms are conceptually a list of segments, but all we need to store is the two (or one, for a cycle) terminal ArmSegments for the arm, and the two (or one) terminal Nodes.  This allows us to recreate the arm for drawing or analysis, but saves lots of memory. (Tradeoff of speed to get memory). The assumption is that traversing the arm will be fast enough and not done very often.  If it starts using lots of time, we can always store more information if it makes it faster later.  
    Arms are used just for classifying nodes and segments and are not expected to be useful to the user of this library; 
  */ 
  struct Arm { 
    Arm() {
      this->init();
    }

    ~Arm() {
      mArms[mArmID] = NULL; 
    }

    void init(void) {
      mArmType = ARM_UNKNOWN;
      mArmLength=0;
      mSeen=false;
      mSeenInMeta=false; 
      mParentMetaArm=NULL;
      mNumSegments = 0; 
      mNumWrappedSegments = 0; 
      mTerminalSegments.clear();  
      mTerminalNodes.clear(); 
      mDecomposing = false; 
      mExtendOrDetach = false; 
      mArmID = mArms.size();
      mArms.push_back(this); 

#if LINKED_LOOPS
      mPartOfLinkedLoop=false; 
      mCheckedForLinkedLoop=false;
#endif
      //mTraceArms.clear(); 
      //mTraceDepth = 2; 
    }
    
    // bookkeeping for e.g. timestep changes. 
    static void Clear(void) {
      for (vector<Arm*>::iterator arm = mArms.begin(); arm != mArms.end(); arm++) {
        if (*arm) delete *arm; 
      }
      mArms.clear(); 
      mTraceArms.clear(); 
      mTraceDepth = 2; 
      mTraceFileBasename = ""; 
      mLongestLength = 0.0; 
      mDecomposedLength = 0.0; 
      mNumDecomposed.resize(7,0);
      mNumDestroyedInDetachment = 0; 
      mTotalArmLengthBeforeDecomposition = 0.0;
      mTotalArmLengthAfterDecomposition = 0.0;  
      mThreshold = -1; 
    }

    /*!
      When one arm is gobbled up by another, the gobblee becomes ancestor to the gobbler
    */ 
    void MakeAncestor(Arm *sourceArm) {
      mAncestorArms.push_back(sourceArm->mArmID); 
      for  (uint32_t a = 0; a < sourceArm->mAncestorArms.size(); a++) {
        mAncestorArms.push_back(sourceArm->mAncestorArms[a]); 
      }
      return;
    }
    
    /*!
      Helper for DetachLoopFromNode and DetachAndFuse, does the detach part
    */ 
    void DetachAndReplaceNode(FullNode *node, FullNode *replacement); 

    /*!
      During decomposition, cross arms are removed from terminal nodes.  
      If the cross arm is a loop, then this is called to detach it.
    */ 
    void DetachLoopFromNode(FullNode *node); 

     /*!
      During decomposition, cross arms are removed from terminal nodes.  
      This actually detaches one such cross arm from a node, emptying the other node and stealing its contents while decrementing the node neighbor count by two. 
    */ 
   void DetachAndFuse(FullNode *node, Arm *other); 

    /*!
      A helper for ExtendByArm() function.  
    */ 
    void ExtendBySegments(Arm *sourceArm, vector<ArmSegment*> &sourceSegments, FullNode *sharedNode, int numDuplicates);

    /*!
      A helper for Decompose() function.  
    */ 
    void ExtendByArm(Arm *sourceArm, vector<ArmSegment*> &sourceSegments, FullNode *sharedNode, int numDuplicates); 
    

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
      if (!mTerminalSegments.size() || !mTerminalSegments[0])  {        
        return BURGERS_UNKNOWN; 
      }
      return mTerminalSegments[0]->GetBurgersType(); 
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
      keepclose parameter:  If false, output nodes in their natural state.  If true, then output nodes such that neighbors never are far apart by adding mbounds components to their coordinates as needed. This is useful when outputting a single arm to prevent long straight interior segments.   
    */ 
    vector<FullNode*> GetNodes(FullNode *startNode = NULL) const;

    uint32_t GetNumNodes(void) { 
      // The math here is a bit odd, due to wrapping: 
      if (!mNumSegments) return 0; 
      return mNumSegments + 1 + mNumWrappedSegments; 
    }

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

    /*!
      Return number of neighbor arms, not including this arm, but including duplicates
    */ 
    uint32_t GetNumNeighborArms(void) {
      uint32_t num = 0; 
      for  (uint32_t node = 0; node < mTerminalNodes.size(); node++) {
        for (uint32_t arm=0; arm < mTerminalNodes[node]->mNeighborArms.size(); arm++) {
          if (mTerminalNodes[node]->mNeighborArms[arm] != this) {
            num++;
          }
        }
      }
      return num; 
    }

    /*! 
      Return nth neighbor arm, not including this but including duplicates
    */ 
    Arm *GetNeighborArm (int num) {
      for  (uint32_t node = 0; node < mTerminalNodes.size(); node++) {
        for (uint32_t arm=0; arm < mTerminalNodes[node]->mNeighborArms.size(); arm++) {
          if (mTerminalNodes[node]->mNeighborArms[arm] != this) {
            if (!num)
              return mTerminalNodes[node]->mNeighborArms[arm]; 
            else
              num--; 
          }
        }
      }
      return NULL; 
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
      This prints out an arm and its neighboring arms using BFS order
      to the given depth to a text file and a VTK file.  
      File created: basename.txt, basename.vtk
    */ 
     void WriteTraceFiles(string stepname); 
    
    /*! 
      Helper function for Arm::WriteTraceFiles and FullNode::WriteTraceFiles
    */ 
    void FindBFSNeighbors(vector<Arm *> &arms, vector<uint32_t> &armdepths, vector<int> &action);

    /*!
      Add the given arm to the list of arms to trace. 
      See WriteTraceFiles()
    */ 
    static void TraceArm(int32_t armID){
      mTraceArms.push_back(armID);       
    }

    /*!
      Set the tracing depth for all traced arms.  
      See WriteTraceFiles()
    */ 
    static void SetTraceDepth(uint32_t depth) {
      mTraceDepth = depth; 
    }

    /*!
      Give a name to distinguish tracefiles from files from other runs.
    */ 
    static void SetTraceFileBasename(string basename) {
      mTraceFileBasename = basename; 
    }

    /*! 
      Check to see if this is the body of a "butterfly," which is two three armed nodes connected by a type 200 arm, and which have four uniquely valued type 111 exterior arms ("exterior" means the arms not connecting the two).  If so, mark each terminal node as -3 (normal butterfly.  If one of the terminal nodes is a type -44 "special monster" node, then mark the other terminal node as being type -34 ("special butterfly"). Finally, could be a -35 connected to a -5 node, which is means, a 3 armed connected to 5 armed, such that exterior arms include all four 111 arm types.  
    */ 
    //void CheckForButterfly(void); 
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
    /*!
      This is useless to the user, only used for classification 
    */ 
    static std::vector<Arm *> mArms; 
    static double mThreshold; // shorter than this and an arm is "short"
    static double mDecomposedLength; // statistics
    static vector<int32_t> mNumDecomposed; // statistics
    static int32_t mNumDestroyedInDetachment; // statistics
    static double mTotalArmLengthBeforeDecomposition, 
      mTotalArmLengthAfterDecomposition;  
    bool mDecomposing, mExtendOrDetach; 
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
      number of segments that got wrapped (affects node and segments counts)
    */ 
    uint8_t mNumWrappedSegments; 
    /*! 
      purely for debugging
    */ 
    int32_t mArmID; 


    /*!
      An ancestor of this arm was one which was assimilated into this arm,
      during decomposition.  Useful for history tracing. 
    */ 
    vector<int32_t> mAncestorArms; 

    /*! 
      To trace out arms in a text and vtk file, add them to this vector
    */ 
    static vector<int32_t> mTraceArms; 

    /*!
      uniquify files from this run
    */ 
    static string mTraceFileBasename; 

    /*! 
      When tracing arms, how deep to BFS for neighbors? 
      0 is no neighbors. 1 is immediate neighbors, etc. 
    */ 
    static uint8_t mTraceDepth; 

    private: 
    struct MetaArm * mParentMetaArm; 
  };

    
  //=============================================
  /*!
    Functor object for (BinaryPredicate) sorting the armsegment set by comparing pointers correctly (by dereferencing, not by pointer address)
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
    vector<Arm*> mFoundArms, mAllArms, mDeadEnds; 
  }; 

  //=============================================
  /*!
    The paraDIS data set.  Note that all you really want is to read the data and return the segments and nodes.  Identifying specific arms in a scene is not likely to be of interest, so arm information is kept internal.  
  */ 
  class DataSet {
  public: 
    DataSet() { init(); }

    ~DataSet() { 
      return; 
    }

    /*!
      if this is true, then complete dumps of all data are done, 
      into files named mDebugOutputPrefix + <type> + ".debug"
    */ 
    void EnableDebugOutput(bool tf=true) {
      mDoDebugOutput = tf; 
    }
    /*!
      if this is true, then create arm and metaarm files.  
    */ 
    void EnableStatsOutput(bool tf=true) {
      mDoStats = tf; 
    }
    /*!
      if this is true, then create tag file.  
    */ 
    void EnableTagFileOutput(bool tf=true) {
      mDoTagFile = tf; 
    }

    /*!
      if this is true, then create tag file.  
    */ 
    void EnableVTKFileOutput(bool tf=true) {
      mDoVTKFile = tf; 
    }
    /*!
s      Tell the data set which file to read
    */ 
    void SetDataFile(std::string datafile) {
      mDataFilename = datafile;
      if (mOutputBasename == "") {
        string basename = Replace(mDataFilename, ".data", ""); 
        basename = Basename(Replace(basename, ".dat", ""));         
        SetOutputBaseName(basename); 
      }
      SetOutputDir(); 
    }

    /*! 
      If mOutputDir is set, leave it alone.
      If not, then set it based on the mOutputBaseName
    */ 
    void SetOutputDir(void) {
      if (mOutputDir != "") {
        return; 
      }
      std::string::size_type slash = mOutputBasename.rfind("/"); 
      if (slash == 0) {
        SetOutputDir("/"); 
      } else if (slash != std::string::npos) {
        SetOutputDir(mOutputBasename.substr(0, slash)); 
      } else {
        SetOutputDir("."); 
      }
    }
        
      
    /*!
      By default, this is "./paradis-output".  
    */ 
    void SetOutputDir(std::string dir) { 
       mOutputDir = dir;     
       if (!Mkdir(mOutputDir.c_str())) {
         cerr << "Warning: could not create output directory " << mOutputDir << endl; 
       }
       return;
    }

    /*!
      By default, this is the datafile name without its extension.  
    */ 
    void SetOutputBaseName(std::string name) { 
       mOutputBasename = name; 
       SetOutputDir(); 
       FullNode::SetTraceFileBasename(mOutputDir + "/" + mOutputBasename);   
       Arm::SetTraceFileBasename(mOutputDir + "/" + mOutputBasename);   
    }

    /*!
      verbosity goes from 0-5, based on dbg_setverbose() from librc.a
      filename if null means stderr
    */ 
    void SetVerbosity(int level, string filename = "") { 
      if (filename != "") {
        if (filename[0] != '/') {
          filename = mOutputDir + "/" + filename; 
        }
        if (!Mkdir(Dirname(filename).c_str())) {
          cerr << "Warning: could not create output directory " << Dirname(filename) << endl; 
        }        
        dbg_setfile(filename.c_str()); 
      }
      dbg_setverbose(level); 
    }

    /*!
      Trace decomposition of the given arm to the given depth.  
    */ 
    void TraceNode(int32_t nodeID){
      FullNode::TraceNode(nodeID); 
    }
    
    /*!
      Trace decomposition of the given arm to the given depth.  
    */ 
    void TraceArm(int32_t armID){
      Arm::TraceArm(armID);
    }
    
    /*! 
      Set trace depth for arm tracing
    */ 
    void SetTraceDepth(uint32_t depth) {
      Arm::SetTraceDepth(depth); 
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
      altname provides a way to write a 2nd file for debugging arms before decomposition 
    */
    void PrintArmFile(char*altname = NULL);

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
      Tag all METAARM_LOOP nodes
    */
    void TagNodes(void);
 
    /*!
      Write out a copy of the input file that has all FullNode tags in it.
    */
    void WriteTagFile(void); 

    /*!
      Write out a vtk fileset containing all nodes and segments. 
      Also writes a .visit file identifying each file as a separate block. 
      There is no ordering intentionally in the file.  
    */
    void WriteVTKFiles(void); 

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
      Run a self-diagnostic.  used by paradisTest
    */ 
    bool Test(void); 

   /*! 
      Parse the paradis data file and create a full set of arms and nodes
      I've made it optional.  
    */ 
    void ReadData(std::string datafile="", bool createVectors=false);    
 

    /* Starting from the given arm, trace out the containing meta arm and return a shared_ptr to the result */
    boost::shared_ptr<MetaArm> TraceMetaArm(vector<Arm>::const_iterator arm); 

    
    /*!
      Accessor function.
    */ 
    uint32_t GetNumNodes(void) { return FullNode::mFullNodeVector.size(); }
    /*!
      Accessor returns begin of node vector. 
    */ 
    /* vector<FullNode *>::const_iterator GetNodesBegin(void) { 
      return FullNode::mFullNodes.begin(); 
      }*/
    /*!
      Accessor returns end of node vector. 
    */ 
    /*
    vector<FullNode *>::const_iterator GetNodesEnd(void) {
      return FullNode::mFullNodes.end(); 
    }
    */

   /*! 
      Accessor function. 
    */ 
    FullNode * GetNode(uint32_t nodenum) { 
      if (nodenum >= FullNode::mFullNodeVector.size()) {
        cerr << "ERROR: GetNode() -- out of bounds index" << endl; 
        return NULL; 
      }
      return FullNode::mFullNodeVector[nodenum]; 
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
      Accessor function
    */
    ArmSegment *GetArmSegment(uint32_t segnum) { 
      if (segnum >= ArmSegment::mArmSegmentVector.size()) {
        cerr << "" << endl; 
        return NULL; 
      }
      return ArmSegment::mArmSegmentVector[segnum]; 
    }

    /*! 
      Accessor function. 
    */ 
    uint32_t GetNumArmSegments(void)  { 
      return  ArmSegment::mArmSegmentVector.size(); 
    }

    /*!
      This is a redundant data structure that allows us to find arm segments that have the same endpoints during the initial reading of the dump file. 
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
      Arm::Clear(); 
      ArmSegment::Clear(); 
      mMinimalNeighbors.clear();
      FullNode::Clear(); 
      MinimalNode::Clear();           
      // Note:  There is no MetaArm::Clear(); 

       // mQuickFindArmSegments is cleared in CollectAllArmSegments()
      mProcNum = mNumProcs = mFileVersion = 0; 
      mTotalDumpNodes =0;
      mElementsPerVTKFile = 50*1000; 
      mDoDebugOutput=false; 
      mDoTagFile = false;
      mDoVTKFile = false; 
      mDoStats = false; 
      mOutputDir = "./paradis-debug";
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
    void DebugPrintFullNodes(void); 
   /*! 
      Prints out all arms
    */ 
    void DebugPrintArms(const char *altname = NULL);

    /*! 
      Prints out all MetaArms
    */ 
    void DebugPrintMetaArms(void);

    /*!
      Read a node from the input file and write it out with its tag to the tagfile
    */
    void CopyNodeFromFile(uint32_t &lineno, map<uint64_t, FullNode*> &nodehash, std::ifstream &datafile, std::ofstream &tagfile);
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
    void FindEndOfArm(FullNode *firstNode, FullNode **oFoundEndNode, 
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

     /* put all arm segments into mArmSegments */ 
     // void CollectAllArmSegments(void);

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
      Create two vectors that VisIt can read sequentially.  
      For memory efficiency, we could simply rehash them, but I don't
      think these global arrays are that big. 
    */ 
    void CreateNodeSegmentVectors(void); 



    /*!
      A MetaArm is a chain of arms with all the same Burgers vector value, but can also include 200, 020, and 002 types as "unzipped" portions along the meta-arm.  
    */  
     std::vector<boost::shared_ptr<MetaArm> > mMetaArms; 

    /*!
      Extents of the full data set: 
    */ 
    static rclib::Point<float> mDataMin, mDataMax, mDataSize;
    /*!
      Subspace of interest: 
    */ 
    rclib::Point<float> mSubspaceMin, mSubspaceMax; 

    /*!
      cannot modify but always sorted so fast
    */ 
    std::set<Neighbor> mMinimalNeighbors; 


 
    /*!
      When writing out VTK files, this is how many to put per block max.
      Aids in speedy visualization. 
    */ 
    uint32_t mElementsPerVTKFile; 

    /*! 
      Number of nodes in full dump data
    */ 
    uint32_t mTotalDumpNodes; 

    /*!
      if this is true, then complete dumps of all data are done, 
      into files named mOutputPrefix + <type> + ".debug"
    */ 
    bool mDoDebugOutput; 
    /*!
      if this is true, then create arm and metaarm files.  
    */ 
    bool mDoStats; 
    /*!
      if this is true, then create tag file.  
    */ 
    bool mDoTagFile; 

    /*!
      if this is true, then create VTK file at end of analysis.  
    */ 
    bool mDoVTKFile; 

     /*!
      By default, this is "./paradis-output".  See mDoOutput.
    */ 
    std::string mOutputDir; 

    /*!
      By default, this is mDataFilename without the extension.  
      E.g., mDataFilename = rs0020.data ---->  mOutputBasename = rs0020
    */ 
    std::string mOutputBasename; 

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
