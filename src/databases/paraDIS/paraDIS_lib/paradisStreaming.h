/*!
  Header file for the streaming library for paraDIS dumpfile visualization and conversion.

*/ 
#ifndef PARADIS_STREAMING_H
#define PARADIS_STREAMING_H

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <set>
#include <deque>
#include <string>
#include <fstream>
#include <iostream>
#include <ostream> 
#include <math.h>

#include "Point.h"
#include "stringutil.h" /* from RC_c_lib, is this a good idea? */ 
#include "debugutil.h" /* from RC_c_lib, now we're committed. */ 

using namespace rclib; 

namespace paraDIS {
  //  =======================================================================
  //  ParadisNodeID
  //  =======================================================================
  class ParadisNodeID {
  public: 
    ParadisNodeID() :mDomainID(0), mNodeID(0) {}

    ParadisNodeID(const ParadisNodeID &other, uint8_t burgers) :
      mDomainID(other.mDomainID), mNodeID(other.mNodeID), mBurgers(burgers) {}

    ParadisNodeID(uint32_t domainNum, uint32_t nodeNum) {
      mDomainID = domainNum;  mNodeID = nodeNum; 
    }
    string Stringify(void) { 
      return string("(")+doubleToString(mDomainID) + "," + 
        doubleToString(mNodeID) + ")"; 
    }
    
    bool operator == (const ParadisNodeID &other) const {
      return mDomainID == other.mDomainID && mNodeID == other.mNodeID; 
    }
    bool operator <(const ParadisNodeID &other) const {
      if (mDomainID != other.mDomainID) {
        return mDomainID < other.mDomainID;
      }
      return mNodeID < other.mNodeID; 
    }
    void test(void); 
    uint32_t mDomainID, mNodeID; 
    uint8_t  mBurgers; 
  }; 
  
  //  =======================================================================
  //  ParadisNode
  //  =======================================================================
  class ParadisNode {
  public: 
    ParadisNode(): mPartOfPartialArm(false), mNumNeighbors(0) {}
      ParadisNode(ParadisNodeID&id): mPartOfPartialArm(false), mID(id), 
        mNumNeighbors(0) {}
    //===================================================================
    // ALGORITHM API: 
    bool IsPartOfPartialArm(void) { return mPartOfPartialArm; }
    void IsPartOfPartialArm(bool setvalue) {  mPartOfPartialArm = setvalue; }
    bool IsNeighborComplete(void) { return mNumNeighbors == mNeighbors.size(); }
    bool IsTerminalNode(void) { return mNumNeighbors != 2; }
    void SetNumNeighbors(uint8_t num) { mNumNeighbors = num; }
    uint8_t NumNeighbors(void) { return mNumNeighbors; }
    
    string Stringify(bool fullinfo); 
    void SetLocation(const Point<float>&location) { mLocation = location; }
    
    /*!
      Given one known neighborID, return true if the otherID can be set, false if not, and throw an exception if it should be possible but isn't.
    */ 
    bool GetOtherNeighborID(const ParadisNodeID &knownID,  
                            ParadisNodeID &otherID);
    /*!
      This makes inNode our neighbor, and makes us their neighbor.  No check is made to see if we already were neighbors, because this is only called when a node sees a neighbor that already exists, which only happens once per pair of neighbors.   
    */ 
    void MarkAsNeighbors(ParadisNode *inNode, uint8_t burgers) {
      /* saved check for redundancy... not necessary as I noted...
         if (!mNeighbors.size() || 
         find(mNeighbors.begin(), mNeighbors.end(), inNode->mID) 
         != mNeighbors.end()){
      */ 
      mNeighbors.push_back(ParadisNodeID(inNode->mID, burgers));
      inNode->mNeighbors.push_back(ParadisNodeID(mID, burgers)); 
      //}
    return; 
    }
    
    // END ALGORITHM API 
    //===================================================================
    
    
    
    bool mPartOfPartialArm; 
    uint8_t mArmRefCount; // number of arms we are members of that have not been emitted.  When this reaches zero, we can be purged from the cache. 
    Point<float> mLocation; 
    ParadisNodeID mID; 
    uint8_t mNumNeighbors; // to determine if terminal and if neighbor-complete 
    vector<ParadisNodeID> mNeighbors; 
  }; // end ParadisNode 
  
  //  =======================================================================
  //  ParadisMinimalNode
  //  =======================================================================
  /*! 
    A class to minimize the memory footprint of an arm, stored in an easy way. 
  */ 
  struct ParadisMinimalNode {
    ParadisMinimalNode(const ParadisNode *other); 

    uint8_t mNumNeighbors; // to determine if terminal and if neighbor-complete 
    Point<float> mLocation; 
    uint32_t mDomainID, mNodeID; 
  }; 

  //  =======================================================================
  //  ParadisArm
  //  =======================================================================
  class  ParadisArm {
  public: 
    ParadisArm(uint8_t burgers) :mBurgersType(burgers) {mNodes = new deque<ParadisNode*>;}
      void push_front(ParadisNode *inNode); 
      void push_back(ParadisNode *inNode) ;
      void PurgeExcess(void) { delete mNodes; mNodes = NULL; }
      deque<ParadisNode *> *GetNodes(void) { return mNodes; }

      /*! 
        After all nodes are added, then call this function to transfer all node information into a more efficient form, then clears mNodes to save memory.  Therefore you must call GetNodes() BEFORE you call this function.  LOL 
        IMPORTANT NOTE: For loops, the last node will not be a terminal node, and the first one might not be either.  So how to draw consistently?  Not to worry:  if the number of neighbors of the last node is 2, then you know that the first node is its next neighbor.  
      */ 
      void Finalize(void); 
      
      
      void SetBurgers(uint8_t burgers) {mBurgersType = burgers; }
      
      
  private:
      deque<ParadisNode *> *mNodes; 
      
      vector <ParadisMinimalNode> mMinimalNodes; 
      
      uint8_t mBurgersType; 
      
  }; // end ParadisArm
  
  //=============================================
  /*!
    Functor object for sorting the node set by comparing pointers correctly (by dereferencing, not by pointer address)
  */ 
  class CompareNodePtrs {
  public:
    bool operator() ( const ParadisNode *n1, const ParadisNode *n2) {
      return n1->mID < n2->mID; 
    }
  };
  
  typedef set<ParadisNode *, CompareNodePtrs>  ParadisNodeSet;
  //=============================================
  
  
  //  =======================================================================
  //  ParadisNodeCache
  //  =======================================================================
  class ParadisNodeCache {
  public:
    ParadisNodeCache() ;
    /*!
      Check to see if a node is already in the cache.  If it is, return a pointer to it.  If not, then if create is true, then create it, else return NULL. 
    */ 
    ParadisNode *CheckOutNode(const ParadisNodeID id, bool create); 
    /*!
      Check the node back in, allowing the cache system to flush it to disk if need be.  Currently nothing but a reality check to see if the node actually should be in the cache. 
    */ 
    void CheckInNode(ParadisNode *checkedOutNode); 
    /*!
      Purge a node from the cache.  No checks are made for appropriateness. Be careful!  If the node is not neighbor complete, this could be trouble. 
    */ 
    void PurgeNode(ParadisNode *node); 
    
    uint32_t GetNumNodes(void) { return mNodes.size(); }

    ParadisNode *GetFirstNode(void);

    uint32_t GetMaxNodeStat(void) { return mMaxNodes; }

  private:
    /*!
      set:  always sorted, so fast, but in order to modify, must remove an element, modify it, and reinsert it... or use const_cast<> 
    */ 
    ParadisNodeSet mNodes; //for now, just keep in memory and see what happens

    uint32_t mMaxNodes; //what was the maximum number of nodes at any one time? 
 };  // end ParadisNodeCache
  
  //  =======================================================================
  //  ParadisDumpFile
  //  =======================================================================
  /*!
    ParadisDumpFile -- this is the user's api to this system. 
    It can operate in two ways, either streaming or not. 
    Streaming:  User creates a HandleNewArm callback function and passes a pointer to it to ParadisDumpFile. When the user then calls ProcessDumpFile, then each time a new arm is created, the function is called with the new arm as an argument.  When the function returns, the arm is deleted from memory.  This is most memory-efficient. 
    Non-streaming:  User just waits until ProcessDumpFile is complete, and calls GetArms() to get the entire chunk of arms to deal with at once.  Less memory efficient, but might be easier to code.  
  */ 
  typedef void NewArmCallback(ParadisArm *newArm);
  
  class ParadisDumpFile {
  public:
    void PrintStats(void); 
    ParadisDumpFile();
    /*! 
      Open the file, scan its contents, and call NewArm() each time a new arm is generated
    */ 
    void ProcessDumpFile(void); 
    
    void SetDumpFile(string filename) {mDataFileName = filename; }
    void SetCallback(NewArmCallback *callback) { mNewArmCallback = callback; }
    vector<ParadisArm *> &GetArms(void) { return mArms; }
  private:
    /*!
      Using value of mOldStyle, parse the file until you can determine the data bounds and the number of nodes.  Throws exception on error.  This is a good way to detect whether mOldStyle is set correctly and to validate the file.  
    */ 
    void ReadPreamble(void);  
    
    void ReadNextNodeFromFile(uint32_t &lineno, std::ifstream &datafile); 
    
    /*!
      Mark all nodes as part of partial arm, starting with currentNode and moving away from previousNode.  If you see another partial arm, return true, else return false.  
    */
    bool ExtendPartialArm(ParadisNode *previousNode, ParadisNode *currentNode); 
    
    /*! 
      Compute a burgersType based on the burgers vector
    */ 
    uint8_t ComputeBurgersType(float burgers[3]);
    /*!
      Helper to ComputeBurgersType()
    */
    int Category(float num);
      
    /*!
      Emit the unique arm that contains both of these nodes and flush all eligible nodes from memory, meaning all member nodes that are neighbor-complete, EXCEPT firstNode, which needs to stick around for one last check.  nextNode's sole purpose is to disambiguate the arms in case firstNode is a terminal node. 
  Return true if firstNode is purged, else return false; 
    */ 
    bool EmitArm(ParadisNode *firstNode, ParadisNode *nextNode, uint8_t burgersType); 
    
    /*!
      Attempt to complete the given arm by marching to the endpoint and storing all nodes seen along the way. 
      If this succeeds, return true.  If this only results in a single endpoint and no loop, then return false.
    */ 
    bool BuildArm(ParadisArm *theArm, ParadisNode *firstNode, ParadisNode *nextNode); 
    
    void ProcessNonterminalCycles(void); 

    string mDataFileName; 
    
    bool mOldStyle; 
    
    
    ParadisNodeCache mNodeCache; 
    
    /*!
      Extents of the full data set: 
    */ 
    rclib::Point<float> mDataMin, mDataMax, mDataSize;
    
    NewArmCallback *mNewArmCallback; 
    vector<ParadisArm*> mArms; // only used if no callback is set.

    
    uint32_t mTotalDumpNodes, 
      mTotalNeighbors; // how many neighbors in all those nodes
    uint32_t mTotalArmNodes, mNumArmsCreated; 
  }; //end ParadisDumpFile
  
} // end namespace ParaDIS
  

#endif
