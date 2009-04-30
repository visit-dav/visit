#include "paradisStreaming.h"
#include "paradis.h"
#include <fstream>
#include <string>
#include <iostream> 
#include "debugutil.h"
#include "stringutil.h"
#include "timer.h"

/*!
  This implements a system of reading paraDIS dump files and creating a stream of arms for processing.  
*/ 

using namespace RC_Math; 
 

using namespace std; 
namespace paraDIS {


  //  =======================================================================
  //  ParadisNode
  //  =======================================================================
  
  //===========================================================================
  bool ParadisNode::GetOtherNeighborID(const ParadisNodeID &knownID,  ParadisNodeID &otherID) {
    if (mNeighbors.size() != 2) return false; 
    if (mNeighbors[0] == knownID) {
      otherID = mNeighbors[1]; 
    } else if (mNeighbors[1] == knownID) {
      otherID = mNeighbors[0]; 
    } else {
      throw string("ParadisNode::GetOtherNeighborID-- knownID does not match either of our neighbors"); 
    }
    return true;
  }

  string ParadisNode::Stringify(bool fullinfo=false) {
    string s = string("ParadisNode: id = ")+mID.Stringify(); 
    if (!fullinfo) return s; 
    return s + string(", numNeighbors = ")+doubleToString(mNumNeighbors) + ", location = " + mLocation.Stringify() + ", refcount = " + doubleToString(mArmRefCount);
  }

  //  =======================================================================
  //  ParadisMinimalNode
  //  =======================================================================
  
  ParadisMinimalNode::ParadisMinimalNode(const ParadisNode *other):
    mNumNeighbors(other->mNumNeighbors), mLocation(other->mLocation), 
    mDomainID(other->mID.mDomainID), mNodeID(other->mID.mNodeID) {}
   

//  =======================================================================
//  ParadisArm
//  =======================================================================
//===========================================================================
void ParadisArm::Finalize(void) {
  deque<ParadisNode *>::iterator nodepos = mNodes->begin(), endpos = mNodes->end(); 
  int numNodes = 0;
  ParadisNode *lastTerminalNodeSeen; 
  while(nodepos != endpos) {
    (*nodepos)->mArmRefCount--; 
    dbprintf(5, "Arm::Finalize: storing info from node:  %s\n", (*nodepos)->Stringify(true).c_str()); 
    mMinimalNodes.push_back(ParadisMinimalNode(*nodepos)); 
    if ((*nodepos)->mNumNeighbors != 2) lastTerminalNodeSeen = *nodepos; 
    ++nodepos;
    ++numNodes; 
  }
  if (mMinimalNodes[numNodes-1].mNumNeighbors == 2) {
    // we have a loop (no terminal node at end of arm)
    if (mMinimalNodes[0].mNumNeighbors != 2) {
      // there is a single terminal node at the beginning of the arm, which is therefore both ends of this arm.  So, decrement it again to show both its neighbors were in the same arm here
      lastTerminalNodeSeen->mArmRefCount--; 
    }
  }
  delete mNodes;  
  mNodes = NULL; 
  return; 
}
//===========================================================================
void ParadisArm::push_front(ParadisNode *inNode) {
  dbprintf(5, "ParadisArm::push_front: %s\n", inNode->Stringify(true).c_str()); 
  mNodes->push_front(inNode);  
}

//===========================================================================
void ParadisArm::push_back(ParadisNode *inNode) {
  dbprintf(5, "ParadisArm::push_back: %s\n", inNode->Stringify(true).c_str()); 
  mNodes->push_back(inNode); 
}

  //  =======================================================================
  //  ParadisNodeCache
  //  =======================================================================
  
  //===========================================================================
  ParadisNodeCache::ParadisNodeCache(): 
    mMaxNodes(0) {}
  //===========================================================================
  
  //===========================================================================
  ParadisNode *ParadisNodeCache::CheckOutNode(ParadisNodeID id, bool create) {
    dbprintf(5, "CheckOutNode  %s: ", id.Stringify().c_str());
    
    static ParadisNode tmpNode;
    tmpNode.mID = id; // yes this is a separate step -- so we don't have to keep allocating on the heap
    ParadisNodeSet::iterator pos = mNodes.find(&tmpNode); 
    if (pos != mNodes.end()) {
      dbprintf(5, "found\n"); 
      return *pos; 
    }
    if (! create) {
      dbprintf(5, "not found (returned NULL)\n"); 
      return NULL; 
    }
    dbprintf(5, "created\n"); 
    ParadisNode *newNode = new ParadisNode(id); 
    mNodes.insert(newNode); 
    
    // statistics for memory usage
    if (mNodes.size() > mMaxNodes) mMaxNodes = mNodes.size(); 
    
    return newNode; 
  }
  
//===========================================================================
void ParadisNodeCache::CheckInNode(ParadisNode *node) {
  dbprintf(5, "CheckInNode %s\n", node->Stringify().c_str());
  ParadisNodeSet::iterator pos = mNodes.find(node); //it's ok to check in a purged node.  Just ignore such requests.  
  if (pos == mNodes.end()) {
    throw string("CheckInNode: not in cache: %s\n", node->Stringify().c_str()); 
  }

  return; 
}

//===========================================================================
void ParadisNodeCache::PurgeNode(ParadisNode *node) {
  dbprintf(5, "PurgeNode %s\n", node->Stringify().c_str());
  int numerased = mNodes.erase(node); 
  if (!numerased) throw string("ParadisNodeCache::PurgeNode: node not in cache"); 
  delete node; 
}

//===========================================================================
  ParadisNode *ParadisNodeCache::GetFirstNode(void) { 
    if (!mNodes.size()) 
      return NULL; 
    return *(mNodes.begin());
  }

//  =======================================================================
//  ParadisDumpFile
//  =======================================================================
  ParadisDumpFile::ParadisDumpFile(): mOldStyle(false), mNewArmCallback(NULL), 
                                      mTotalDumpNodes(0), mTotalNeighbors(0), 
                                      mTotalArmNodes(0), mNumArmsCreated(0){
     
  }
//===========================================================================
void ParadisDumpFile::ReadPreamble(void) {
  dbprintf(2,"Beginning ReadPreamble\n");; 
  if (mDataFileName == "") {
    throw string("ReadPreamble: no dump file name has been given"); 
  }
  uint32_t lineno = 0; 
  char linebuf[2048]="";
  try {
    ifstream dumpfile(mDataFileName.c_str());
    if (!dumpfile.good()){
      throw string("Cannot open dumpfile ") + mDataFileName;
    }
    else {
      dbprintf(2, "Parsing file %s\n", mDataFileName.c_str());
    }
    
    int i=0; 
    if (mOldStyle) {
      string token;
      while (dumpfile.good() && token != "minSideX")
        dumpfile >> token;
      while (dumpfile.good() && i<3) {
        dumpfile >> token >> mDataMin[i] >> token  >> token  >> mDataMax[i] >>token; 
        i++;
      }
    }
    else {
      while (dumpfile.good() && !strstr(linebuf, "Minimum coordinate values"))
        dumpfile.getline(linebuf, 2048);
      if (!dumpfile.good()){
        throw string("Cannot find minimum bounds from dumpfile ") + mDataFileName;  
      }
      dumpfile.getline(linebuf, 2048);//get that stupid "#"
      dumpfile >> mDataMin[0] >> mDataMin[1] >> mDataMin[2];
      
      while (dumpfile.good() && !strstr(linebuf, "Maximum coordinate values"))
        dumpfile.getline(linebuf, 2048);
      if (!dumpfile.good()){
        throw string( "Cannot find maximum bounds from dumpfile ")+ mDataFileName;
      }
      dumpfile.getline(linebuf, 2048);//get that stupid "#"
      dumpfile >> mDataMax[0] >> mDataMax[1] >> mDataMax[2];
    }
    mDataSize  = mDataMax - mDataMin;
  
    if (!mOldStyle) {
      while (dumpfile.good() && !strstr(linebuf, "Node count")) {
        dumpfile.getline(linebuf, 2048);  
        ++lineno; 
      }
      dumpfile.getline(linebuf, 2048);//should be "#"
      ++lineno;
      if (!strstr(linebuf, "#"))
        dbprintf(0, "WARNING: Expected line:\n#\n but got:\n%s\n", linebuf);
      dumpfile.getline(linebuf, 2048);  
      ++lineno;
    }
    else {
      while (dumpfile.good() && !strstr(linebuf, "Number of nodes")) {
        dumpfile.getline(linebuf, 2048);  
        ++lineno;
      }
    }
    if (dumpfile.good()) 
      dumpfile >> mTotalDumpNodes; // did not read the whole line...
    
    if (!mTotalDumpNodes) {
      throw string( "Cannot find number of nodes in dumpfile");
    }
  } catch (string err) {
    throw string("Error in GetBounds:\n")+err;
  }
  
  dbprintf(2,"Done with ReadPreamble\n");; 
  return; 
}
  
  //===========================================================================
  int ParadisDumpFile::Category(float num) {
    if (!num)
      return 0;
    else if (fabs(num) > 1.15 && fabs(num) < 1.16)
      return 1;
    else if (num > 0.577 && num < 0.578)
      return 2;
    else if (num < -0.577 && num > -0.578)
      return 3;
    dbprintf(2, "WARNING: Weird value %g encountered in Category\n", num);
    return 4;
  }
  
  //===========================================================================
uint8_t ParadisDumpFile::ComputeBurgersType(float burg[3]) {
  uint8_t burgersType = 0;
  // type 100 nodes are valued 1-3, and type 111 nodes are valued 4-7
  int valarray[3] = 
    {Category(burg[0]), Category(burg[1]), Category(burg[2])};
  if (valarray[0] == 1 && valarray[1] == 0 && valarray[2] == 0)
    burgersType = 1;
  else if (valarray[0] == 0 && valarray[1] == 1 && valarray[2] == 0)
    burgersType = 2;
  else if (valarray[0] == 0 && valarray[1] == 0 && valarray[2] == 1)
    burgersType = 3;
  else if ((valarray[0] == 2 && valarray[1] == 2 && valarray[2] == 2) ||
           (valarray[0] == 3 && valarray[1] == 3 && valarray[2] == 3))
    burgersType = 4;
  else if ((valarray[0] == 2 && valarray[1] == 2 && valarray[2] == 3) ||
           (valarray[0] == 3 && valarray[1] == 3 && valarray[2] == 2))
    burgersType = 5;
  else if ((valarray[0] == 2 && valarray[1] == 3 && valarray[2] == 2) ||
           (valarray[0] == 3 && valarray[1] == 2 && valarray[2] == 3))
    burgersType = 6;
  else if ((valarray[0] == 2 && valarray[1] == 3 && valarray[2] == 3) ||
           (valarray[0] == 3 && valarray[1] == 2 && valarray[2] == 2))
    burgersType = 7;
  else {
    burgersType = 8;
    dbprintf(3, "\n\n********************************\n");
    dbprintf(3, "Warning: segment has odd value/category: value = %d, burg = (%f, %f, %f)\n", burgersType, burg[0], burg[1], burg[2]); 
    dbprintf(3, "\n********************************\n\n");
  }
  return burgersType; 
}


//===========================================================================
/*! 
  EmitArm: 
  Algorithm:  create the arm, then either call the callback or store it in the output vector.  Purge all neighbor-complete nodes except firstNode. 
  Nodes will be added in either one or two chunks, each ending with a terminal node, unless it's a loop.  If the first chunk does NOT start with a terminal node, there will be a second chunk, starting in the other direction from the first node seen, so push_front() all new nodes on the deque until you see the first terminal node, then push_back() the remaining nodes on the deque.  If you see a second terminal node, then check it to see if it's the same as the first and eliminate it if so.  You will then have a nice sorted list of nodes.
  Return true if firstNode is purged, else return false; 
*/
bool ParadisDumpFile::EmitArm(ParadisNode *firstNode, ParadisNode *otherNode, uint8_t burgersType){
  dbprintf(5, "\n********  BEGIN EmitArm *********\n"); 
  bool firstNodePurged = false; 
  ParadisArm *theArm = new ParadisArm(burgersType); 
  
  theArm->SetBurgers(burgersType); 
  theArm->push_front(firstNode); 
  ParadisNode *nextNode = otherNode; 
  ParadisNodeID theID; 
  while (nextNode) {
    ParadisNode *currentNode = nextNode; 
    if (firstNode->mNumNeighbors != 2 || nextNode != otherNode) {
      nextNode = NULL; //there is no second chunk because the firstNode is terminal
    } else {
      // use the other neighbor as the nextNode for the second chunk: 
      if (! firstNode->GetOtherNeighborID(nextNode->mID, theID)) {
        throw string("ParadisDumpFile::EmitArm: GetOtherNeighborID failed on interior firstNode"); 
      }
      nextNode = mNodeCache.CheckOutNode(theID, false); 
      if (!nextNode) {
        throw string("ParadisDumpFile::EmitArm: CheckOutNode failed for second chunk"); 
      }
    }      
    
    ParadisNode *previousNode = firstNode; 
    while (true) {
      if (currentNode == firstNode) {// we've looped, time to bail 
        nextNode = NULL; 
        break; 
      }
      
      if (currentNode->mNumNeighbors != 2 && 
          currentNode == (*(theArm->GetNodes()))[0]) {
        // it's a terminal node and we've seen it before... so the arm only has one terminal node ... don't add it again
        break; 
      }
      /*! 
        store the node, unless it's a terminal node we've seen before
      */ 
      if (nextNode) // we're on chunk 1 of 2 
        theArm->push_front(currentNode); 
      else // we're chunk 1 of 1, or chunk 2 of 2
        theArm->push_back(currentNode); 
    
      
      if (currentNode->mNumNeighbors != 2)
        break; // we're done with this chunk 
      
      // interior node; keep going...
      if (! currentNode->GetOtherNeighborID(previousNode->mID, theID)) {
        throw string("ParadisDumpFile::EmitArm: GetOtherNeighborID failed on currentNode"); 
      }
      previousNode = currentNode; 
      currentNode = mNodeCache.CheckOutNode(theID, false); 
      if (!currentNode) {
        throw string("ParadisDumpFile::EmitArm: CheckOutNode failed on current ID"); 
      }
    }/* end current chunk */ 
  } /* end arm creation */ 
  
  
  deque<ParadisNode *> nodes = *(theArm->GetNodes()); // make a copy


  theArm->Finalize(); 
  deque<ParadisNode *>::iterator nodepos = nodes.begin(), endpos = nodes.end(); 
  while (nodepos != endpos) {
    if (!(*nodepos)->mArmRefCount) {
      if ((*nodepos) == firstNode) firstNodePurged = true; 
      mNodeCache.PurgeNode(*nodepos);
    }
    ++nodepos; 
  }

  //statistics
  mTotalArmNodes += nodes.size(); 
  mNumArmsCreated++; 

  // emit the arm or store it
  if (mNewArmCallback) {
    mNewArmCallback(theArm); 
    delete theArm;
  } else {
    mArms.push_back(theArm);
    dbprintf(4,"mArms.size is %d\n", mArms.size()); 
  }
  dbprintf(5, "********  END EmitArm *********\n\n"); 
  return firstNodePurged; 
}
//===========================================================================
/*!
  ExtendPartialArm:  Mark all nodes as part of partial arm, starting with currentNode and moving away from previousNode.
  Algorithm:
  0) previousNode is already part of a partial arm, and currentNode may not be.  If it is, then we have discovered a complete arm... hmmm.   
  1) Mark currentNode as part of partial arm. 
  2) Get other neighbor.  go to 0
 */ 
bool ParadisDumpFile::ExtendPartialArm(ParadisNode *previousNode, ParadisNode *currentNode) {
  ParadisNodeID theID; 
  
  while (currentNode) {
    if (currentNode->IsPartOfPartialArm()) {
      dbprintf(5, "ExtendPartialArm: %s is already part of partial arm -- we have a complete arm.\n", currentNode->Stringify().c_str()); 
      return true; 
    }
    if (currentNode->mNumNeighbors != 2) {
      // this cannot happen!  It would be part of a partial arm already
      throw string("ParadisDumpFile::CompletePartialArm: not an interior node: ") + currentNode->Stringify(true); 
    }
    dbprintf(5, "ExtendingPartialArm from node %s to node %s\n", previousNode->Stringify().c_str(),  currentNode->Stringify().c_str()); 
    currentNode->IsPartOfPartialArm(true);     
    
    if (!currentNode->GetOtherNeighborID(previousNode->mID, theID)) {
      dbprintf(5, "ExtendPartialArm: Have not seen other neighbor of %s yet.  Giving up\n",  currentNode->Stringify().c_str()); 
      return false; //ain't no other neighbor yet -- stop!
    }

    previousNode = currentNode; 
    currentNode = mNodeCache.CheckOutNode(theID, false); 
    if (!currentNode) {
      throw string("ParadisDumpFile::CompletePartialArm: currentnode has other node id stored, but cannot get out of cache."); 
    }
  }
  //should never get here
  return false; 
}
//===========================================================================
/*! 
  Read the information about the next node from the dump file.  Create any new nodes or neighbors as they are encountered and enter information as available.  If an arm is discovered, emit the appropriate signal.
*/ 
void ParadisDumpFile::ReadNextNodeFromFile(uint32_t &lineno, std::ifstream &datafile){
  dbprintf(5, "--------- ParadisDumpFile::ReadNextNodeFromFile -------\n"); 
  ParadisNode *newNode = NULL; 
  char comma;
  long old_id_junk, constraint_junk; 
  uint32_t numNeighbors; 
  float float_junk; 
  Point<float> location; 
  int domainID, nodeID, neighborDomain, neighborID;
  string junkstring;
  try {
    //-----------------------------------------------
    // read the first line of node information and see if it matches theNode
    if (!mOldStyle) {
      datafile >> domainID >> comma >> nodeID;
      if (!datafile.good()) {
        throw string("error reading domainID and nodeID of node"); 
      }
    } else { // old style
      datafile >> nodeID >> old_id_junk;
      if (!datafile.good()) {
        throw string("error reading nodeID"); 
      }
    }
    /*! 
      read location
    */ 
    int i=0; while (i < 3) datafile >> location[i++];
    if (!datafile.good()) {
      throw string("error reading location"); 
    }
    
    datafile >> numNeighbors >> constraint_junk;
    if (!datafile.good()) {
      throw string("error reading numNeighbors"); 
    }
    if (mOldStyle){
      datafile  >> domainID >>  old_id_junk; 
      if (!datafile.good()) {
        throw string("error reading domainID"); 
      }
    }
    if (!datafile.good()) {
      throw string("error reading node"); 
    }
    ++lineno; 
    newNode = mNodeCache.CheckOutNode(ParadisNodeID(domainID, nodeID), true);
    if (!newNode) {
      throw string("Error:  cache would not give me node:")+ParadisNodeID(domainID, nodeID).Stringify(); 
    }
    newNode->SetLocation(location); 
    newNode->SetNumNeighbors(numNeighbors); 
    if (numNeighbors == 2)     
      newNode->mArmRefCount = 1;
    else
      newNode->mArmRefCount = numNeighbors; 
    dbprintf(5,"Read first line of node %s\n", newNode->Stringify(true).c_str()); 
    if (numNeighbors != 2) newNode->IsPartOfPartialArm(true); 

    mTotalNeighbors += numNeighbors; 
    //------------------------------------------
    // read neighbor information
    uint32_t neighbornum = 0; 
    float burgers[3]; 
    ParadisNode *neighborNode = NULL, *otherNeighbor; 
    ParadisNodeID theID; 
    bool newNodeWasPurged = false; 
    //Neighbor currentNeighbor; //from file
    try {
      while (neighbornum++ < numNeighbors)  {
        bool haveFullArm = false; 
        if (!mOldStyle) {
          datafile >> neighborDomain >> comma >> neighborID;   
          if (!datafile.good()) {
            throw string("error reading neighborDomain and neighborID of neighbor"); 
          }
        }
        else {
          neighborDomain   = 0; // hmmm.  really? 
          datafile >> neighborID;
          if (!datafile.good()) {
            throw string("error reading neighborID"); 
          }
        }
        
        //==============================================
        // step 1 
        
        // step 1-1:
        // read burgers value and discard "nx, ny, nz". 
        datafile >> burgers[0] >> burgers[1] >> burgers[2] >> float_junk >> float_junk >> float_junk;
        lineno += 2; 
        if (!datafile.good()) {
          throw string("error reading burgers values"); 
        }
        
        // steps 1-2 and 1-3:        
        neighborNode = mNodeCache.CheckOutNode(ParadisNodeID(neighborDomain, neighborID), false); 
        if (!neighborNode){
          dbprintf(5, "Neighbor (%d, %d) does not exist yet\n", neighborDomain, neighborID); 
          continue;
        }
        dbprintf(5, "Found neighbor node %s\n", neighborNode->Stringify().c_str()); 
        newNode->MarkAsNeighbors(neighborNode, ComputeBurgersType(burgers)); 
        
        // step 1-4:
        if (neighborNode->IsPartOfPartialArm()) {
          dbprintf(5,"Neighbor %s is part of partial arm\n", neighborNode->Stringify().c_str()); 
          // step 1-4-1:
          if (!newNode->IsPartOfPartialArm()) {
            dbprintf(5,"New node %s is not yet part of partial arm\n", newNode->Stringify().c_str()); 
            
            // step 1-4-1-1
            newNode->IsPartOfPartialArm(true); 
            dbprintf(5,"Marked new node %s as being part of partial arm\n", newNode->Stringify().c_str()); 
            
            // step 1-4-1-2:
            otherNeighbor = NULL;
            if (newNode->GetOtherNeighborID(neighborNode->mID, theID)) {
              dbprintf(5, "Other neighbor of new arm exists: %s\n", theID.Stringify().c_str()); 
              otherNeighbor = mNodeCache.CheckOutNode(theID, false); 
              if (!otherNeighbor) {
                throw string("Cannot get neighbor even though ID %s is valid", theID.Stringify().c_str()); 
              }
              // step 1-4-1-2-1
              if (!otherNeighbor->IsPartOfPartialArm()) {
                dbprintf(5, "Other neighbor %s is part of partial arm already\n", otherNeighbor->Stringify().c_str()); 
                if (ExtendPartialArm(newNode, otherNeighbor)) {
                  haveFullArm = true; 
                }
              } 
              // step 1-4-1-2-2
              else {
                dbprintf(5, "Other neighbor %s is already part of partial arm.\n", otherNeighbor->Stringify().c_str()); 
                haveFullArm = true; 
              }
            } // end step 1-4-1-2
          } // end step 1-4-1
          
          // step 1-4-2
          else {
            dbprintf(5, "New node %s is a terminal node.\n", newNode->Stringify().c_str()); 
            haveFullArm = true; 
          }
        } // end step 1-4
        
        // step 1-5:
        else { // 
          dbprintf(5, "neighbor is not part of partial arm\n"); 
          // step 1-5-1:
          if (newNode->IsPartOfPartialArm()) {
            dbprintf(5, "New node %s is part of partial arm, extending partial arm through neighborNode\n", newNode->Stringify().c_str()); 
            if (ExtendPartialArm(newNode, neighborNode)) 
              haveFullArm = true; 
          }
        }
        
        // step 1-6:
        if (haveFullArm) {
          dbprintf(5, "ARM COMPLETE\n"); 
          if (EmitArm(newNode, neighborNode, ComputeBurgersType(burgers))) {
            newNodeWasPurged = true; 
          }
        }
        else {
          // step 1-7:
          mNodeCache.CheckInNode(neighborNode); 
        }
      } /* while (neighbornum < numNeighbors) */
      // step 2:
      if (!newNodeWasPurged) {
        mNodeCache.CheckInNode(newNode); 
      }
      dbprintf(5, "Done creating node\n"); 
    } catch (string err) {
      throw string("Error in DataSet::ReadNextNode reading neighbor ")+doubleToString(neighbornum)+":" + err; 
    }
  } catch (string err) {
    throw string("Error trying to read full node info corresponding to ")+newNode->Stringify() + ":\n" + err;
  } 

  return; 
  
} /* end ReadFullNodeFromFile */ 

  //=============================================================================
  /*!
    After all nodes are read in, all arms that have a terminal node in them are emitted.  However, some arms are just cycles of interior nodes.  We need to emit these too.  
  */ 
  void ParadisDumpFile::ProcessNonterminalCycles(void) {
    ParadisNode  *currentNode = NULL, *neighborNode = NULL; 
    dbprintf(4, "ProcessNonterminalCycles\n"); 
    int numcycles = 0; 
    currentNode = mNodeCache.GetFirstNode();
    while (currentNode) {
      dbprintf(4, "ProcessNonterminalCycles: %s\n", currentNode->Stringify().c_str()); 
      if (currentNode->mNumNeighbors != 2) {
        throw string("ProcessNonterminalCycles: discovered non-interior node ") + currentNode->Stringify();
      }
      if (!currentNode->mNeighbors.size()) {
        throw string("ProcessNonterminalCycles: node has no stored neighbors: ") + currentNode->Stringify();
      }
      neighborNode = mNodeCache.CheckOutNode(currentNode->mNeighbors[0], false); 

      if (!EmitArm(currentNode, neighborNode, currentNode->mNeighbors[0].mBurgers)) {
        throw string("ProcessNonterminalCycles: emitting arm of node did not purge it:") + currentNode->Stringify();
      }
      numcycles++; 

      currentNode = mNodeCache.GetFirstNode();
    }
    dbprintf(4, "Number of cycles found: %d\n", numcycles); 
    return; 
  }
  
  //=============================================================================
  void ParadisDumpFile::PrintStats(void) {
    float averageNeighbors = (float)mTotalNeighbors/mTotalDumpNodes; 
    float averageNodeSize = averageNeighbors *sizeof(ParadisNodeID) + sizeof(ParadisNode); 
    uint32_t maxNodes = mNodeCache.GetMaxNodeStat();
    float maxNodeMem = averageNodeSize * maxNodes; 
    dbprintf(2, "Size of node is %d bytes plus %d bytes per neighbor.  Average number of neighbors per node is %f, so average node size is %f bytes.  Created %d nodes.  Max number of nodes in memory was %d and max memory used by nodes at any time was %.2f megabytes.\n", 
             sizeof(ParadisNode), sizeof(ParadisNodeID),  averageNeighbors, 
             averageNodeSize, mTotalDumpNodes, 
             maxNodes, maxNodeMem/1000000.); 
  
    float averageNodesPerArm = mTotalArmNodes / mNumArmsCreated; 
    float averageArmMemory =
      averageNodesPerArm * sizeof(ParadisMinimalNode) + sizeof(ParadisArm); 
    dbprintf(2, "Size of arm is %d bytes + %d bytes per node. The average number of nodes per arm was %f, so average arm memory was %f bytes.  Created %d arms, so total memory potential usage for arms is %.2f megabytes.\n", 
             sizeof(ParadisArm), sizeof(ParadisMinimalNode),  averageNodesPerArm, 
             averageArmMemory, mNumArmsCreated, mNumArmsCreated * averageArmMemory/1000000.); 

    return; 
  }
  
  //=============================================================================
  void ParadisDumpFile::ProcessDumpFile(void) {
  try {
    ReadPreamble(); 
  } catch (...) {
    mOldStyle = true; 
    try {
      ReadPreamble(); 
    } catch (string err) {
      throw string("ProcessDumpFile got error from ReadPreamble trying to determine if the file is old-style or not: ")+err; 
    }
  }

  // at this point, we know we have a good file, or at least one we can open:
  uint32_t lineno = 0; 
  ifstream datafile(mDataFileName.c_str()); 
  char linebuf[2048]="";

  //=============================================================
  // find the first node: 
  while (datafile.good() && !strstr(linebuf, "Secondary lines:")) {
    datafile.getline(linebuf, 2048);  
    ++lineno; 
  }
  
  if (!mOldStyle) {
    datafile.getline(linebuf, 2048);//should be "#"
    ++lineno; 
    if (!strstr(linebuf, "#"))
      dbprintf(0, "WARNING: Expected line:\n#\n but got:\n%s\n", linebuf);
  }
  
  if (!datafile.good()) {
    throw string( "Error: cannot find first node in data file");
  }
  
  
  //=============================================================
  // continue parsing nodes until done
  uint32_t nodenum = 0; 
  try {
   timer theTimer; 
    theTimer.start(); 
    double theTime=theTimer.elapsed_time(), thePercent=0;
    while (datafile.good() && nodenum++ < mTotalDumpNodes) {
      if (dbg_isverbose())
        Progress(theTimer, nodenum, mTotalDumpNodes, thePercent, 5, theTime, 60, "Reading datafile"); 
      ReadNextNodeFromFile(lineno, datafile);        
    }
    ProcessNonterminalCycles(); // handle cycles with no terminal nodes

    PrintStats(); 
  } catch (string err) {
    throw string("Error in ProcessDumpFile while reading node ") + doubleToString(nodenum)+ ":\n" + err; 
  }


  dbprintf(5, "Nodes remaining (should be zero): %d\n", mNodeCache.GetNumNodes()); 
  return; 


  
}

} // end namespace ParaDIS
