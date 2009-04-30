/*!
  \file paradis.C
 a new run at libparadis.C from the ground up 
   Need to do this because the previous design was intentionally entwined with povray and HTS operations for efficiency, and assumed the whole dataset would be read at once. 
 */ 
#include "paradis.h"
#include <fstream>
#include <string>
#include <iostream> 
#include "debugutil.h"
#include "stringutil.h"
#include "timer.h"
#include <algorithm>
#include <ext/hash_set>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
using namespace RC_Math; 
 



using namespace std; 
namespace paraDIS {
  
  //===========================================================================
#ifdef DEBUG
  int32_t ArmSegment::mNextID = 0; 
  int32_t Arm::mNextID = 0; 
  rclib::Point<float> FullNode::mBoundsMin, FullNode::mBoundsMax; 
#endif
  double gSegLen = 0 ;
  uint32_t gNumClassified = 0, gNumWrapped = 0, gNumArmSegmentsMeasured=0; 

  //===========================================================================
  std::string Node::Stringify(void) const {
    std::string s(std::string("(Node): ") + 
                  mID.Stringify() + string("\n"));
    s += string("(node): In bounds: ") + (mInBounds?"true":"false");
   
    return s; 
  }
  
  //===========================================================================
  ArmSegment *FullNode::GetOtherNeighbor (const ArmSegment* n) {
    if (mNeighborSegments.size() > 2) {
      throw string("GetOtherNeighbor called, but mNeighborSegments.size() is greater than 2: ")+Stringify(); 
    }
    if (*mNeighborSegments[0] == *n) {
      if (mNeighborSegments.size() == 1) {
        return NULL; 
      }
      return mNeighborSegments[1];
    }
    // else mNeighborSegments[0] != n
    if (mNeighborSegments.size() == 1 || *mNeighborSegments[1] != *n) {
      throw string("GetOtherNeighbor: given segment is not a neighbor of this segment.  This: ")+this->Stringify()+", given: "+n->Stringify();
    }
    return mNeighborSegments[0];
  }
  
  //===========================================================================
  void FullNode::ConfirmNeighbors(void)   {
    int segnum = mNeighborSegments.size(); 
    while (segnum--) {
      mNeighborSegments[segnum]->ConfirmEndpoint(this); 
    }
    return; 
  }
  
  //===========================================================================
  std::string FullNode::Stringify(bool showneighbors) const {
    std::string s(string("(FullNode address )") + pointerToString(this) + 
#ifdef DEBUG
                  " index " + doubleToString(mNodeIndex) + 
#endif
                  " type " + (doubleToString(mNodeType) + string("\n")) + 
                  Node::Stringify() + string("\n") +  "Location: ("); 

    uint32_t i=0; while (i<3) {
      s+= doubleToString(mLocation[i]);
      if (i<2) s+= ", ";
      ++i; 
    }
    s += ")\n"; 
    if (showneighbors) {
      s+= "Neighbors:\n"; 
      i=0; while (i < mNeighborSegments.size()) {
        s += "neighbor " + doubleToString(i) + ": "; 
        if (mNeighborSegments[i]) {
          s+= mNeighborSegments[i]->Stringify();
        } else { 
          s += "(NULL)\n"; 
        } 
        ++i; 
      }
    }
    return s; 
  }
  //===========================================================================
  void FullNode::SetNodeType(int8_t itype) {
    if (itype != -1) {
      /*! 
        Simply set the type as requested
      */ 
      mNodeType = itype; 
    } else {
      /*!
        set my own type according to what I know -- this covers all nodes except butterfly ends. 
      */ 

      mNodeType = mNeighborSegments.size();  // true for vast majority of nodes. 
      if (mNodeType == 4) {//four-armed is forewarned!  Oh, I'm funny. 
        /*!
          check for a monster or special monster 
        */ 
        int num_111 = 0, num_100 = 0; 
        int neighbor = 4; 
        /*!
          btypes:  used to check for duplicates -- if any two arms are the same type, it's not a monster. There are only 8 types, but they are 1-based, so allocate 9 slots. 
        */           
        bool btypes[9] = {false}; 
        while (neighbor--) {
          const ArmSegment *theSegment = 
            dynamic_cast<const ArmSegment *>(mNeighborSegments[neighbor]);
          int8_t btype =  theSegment->GetBurgersType(); 
          if (btypes[btype] || btype == 8) {
            return; //not a monster, we're done
          }
          btypes[btype] = true; 

          if (btype < 4) num_100++; 
          else num_111++; 
        }
        /*! 
          We have four uniquely valued arms.  Are they of the right mix? 
        */  
        if (num_111 == 4) {
          mNodeType = -4; 
        } else if (num_111 == 2 && num_100 == 2 ) {
          mNodeType = -44; 
        }
      }// end if four-armed
    }
    return; 
  }/* end SetNodeType */ 

  
  //===========================================================================
  void ArmSegment::ComputeBurgersType(float burg[3]) {
    mBurgersType = 0;
    // type 100 nodes are valued 1-3, and type 111 nodes are valued 4-7
    int valarray[3] = 
      {Category(burg[0]), Category(burg[1]), Category(burg[2])};
    if (valarray[0] == 1 && valarray[1] == 0 && valarray[2] == 0)
      mBurgersType = 1;
    else if (valarray[0] == 0 && valarray[1] == 1 && valarray[2] == 0)
      mBurgersType = 2;
    else if (valarray[0] == 0 && valarray[1] == 0 && valarray[2] == 1)
      mBurgersType = 3;
    else if ((valarray[0] == 2 && valarray[1] == 2 && valarray[2] == 2) ||
             (valarray[0] == 3 && valarray[1] == 3 && valarray[2] == 3))
      mBurgersType = 4;
    else if ((valarray[0] == 2 && valarray[1] == 2 && valarray[2] == 3) ||
             (valarray[0] == 3 && valarray[1] == 3 && valarray[2] == 2))
      mBurgersType = 5;
    else if ((valarray[0] == 2 && valarray[1] == 3 && valarray[2] == 2) ||
             (valarray[0] == 3 && valarray[1] == 2 && valarray[2] == 3))
      mBurgersType = 6;
    else if ((valarray[0] == 2 && valarray[1] == 3 && valarray[2] == 3) ||
             (valarray[0] == 3 && valarray[1] == 2 && valarray[2] == 2))
      mBurgersType = 7;
    else {
      mBurgersType = 8;
      dbprintf(3, "\n\n********************************\n");
      dbprintf(3, "Warning: segment has odd value/category: value = %d, burg = (%f, %f, %f), valarray=(%d, %d, %d)\n", mBurgersType, burg[0], burg[1], burg[2], valarray[0], valarray[1], valarray[2]); 
      dbprintf(3, "\n********************************\n\n");
    }
  }
  
  //===========================================================================
  bool ArmSegment::Wrap(const rclib::Point<float> &dataSize, 
                        ArmSegment *&oNewSegment, 
                        FullNode *&oWrapped0, FullNode *&oWrapped1) {
    const float *loc0 = mEndpoints[0]->GetLocation(), 
      *loc1 = mEndpoints[1]->GetLocation();     
    float shift_amt[3] = {0}; 
    bool wrap = false;
    int i=3; while (i--) {
      float dist = fabs(loc0[i] - loc1[i]); 
      if (dataSize[i] - dist < dist) {
        wrap = true; 
        shift_amt[i] = dataSize[i]; 
      }
    }
    

    if (!wrap) {
      oWrapped0 = NULL; oWrapped1 = NULL; oNewSegment = NULL; 
      dbprintf(5, "Not wrapped: %s\n", Stringify().c_str()); 
      gSegLen += GetLength(); 
      return false; 
    }
    gNumWrapped++; 
    oWrapped0 = new FullNode(*mEndpoints[0], true);   
    oWrapped1 = new FullNode(*mEndpoints[1], true); 
    
    oNewSegment = new ArmSegment(*this); 
    oNewSegment->mWrapped = true; 
    i = 3; while (i--) {             
      if (oWrapped0->GetLocation(i) < oWrapped1->GetLocation(i)) {
        /*! 
          Wrap node0 "forward" and wrap node1 "backward" in space
        */ 
        oWrapped0->AddToLocation(i, shift_amt[i]); 
        oWrapped1->AddToLocation(i, -shift_amt[i]); 
      } else {
        /*! 
          Wrap node0 "backward" and wrap node1 "forward" in space
        */ 
        oWrapped0->AddToLocation(i, -shift_amt[i]); 
        oWrapped1->AddToLocation(i, shift_amt[i]);           
      }
    }

    oWrapped0->SetInBounds(); 
    oWrapped1->SetInBounds(); 
    
    mEndpoints[1]->SetInBounds(); 
    mEndpoints[0]->SetInBounds(); 
    
    /*! 
      insert the new segment between endpoint 1 and the new node 0
    */ 
    oNewSegment->ReplaceEndpoint(mEndpoints[0], oWrapped0); 
    oWrapped0->AddNeighbor(oNewSegment); 
    mEndpoints[1]->ReplaceNeighbor(this,oNewSegment); 

    /*!
      Insert *this between endpoint 0 and the new node 1
    */ 
    oWrapped1->AddNeighbor(this); 
    this->ReplaceEndpoint(mEndpoints[1], oWrapped1); 
        
    //oWrapped0->ConfirmNeighbors(); 
    //oWrapped1->ConfirmNeighbors(); 
    
    /*!
      Now is the time to most easily check if the wrapping has created a useless pair of nodes (both out of bounds)
    */ 
    /*
    if (!oWrapped0->InBounds() && !mEndpoints[1]->InBounds()) {
      oWrapped0->SetNodeType(USELESS_NODE); 
      mEndpoints[1]->SetNodeType(USELESS_NODE); 
    }
    if (!oWrapped1->InBounds() && !mEndpoints[0]->InBounds()) {
      oWrapped1->SetNodeType(USELESS_NODE); 
      mEndpoints[0]->SetNodeType(USELESS_NODE); 
    }
     
    */
    gSegLen += GetLength(); 
    dbprintf(5, "Wrapped: %s\n", Stringify().c_str()); 
    return true; 
  }




  //===========================================================================
  bool Arm::HaveFourUniqueType111ExternalArms(void) {

    if (mTerminalNodes.size() != 2) return false;

    int btypes[9] = {false}; 
    int nodenum = 2; 
    while (nodenum--) {
      FullNode *thisNode = mTerminalNodes[nodenum]; 
      int neighbornum = thisNode->GetNumNeighbors(); 

      if (neighbornum != 3) return false; 

      while (neighbornum--) {
        const ArmSegment *thisSegment = thisNode->GetNeighbor(neighbornum);
        /*!
          Only check exterior segments (those which don't belong to *this).
        */ 
        if (thisSegment == mTerminalSegments[0] || 
            thisSegment == mTerminalSegments[1]) continue; 

        int btype = thisSegment->GetBurgersType(); 
        if (btype <= 3 || btype == 8 || btypes[btype]) return false; 
        btypes[btype] = true; 
      }
    }
    /*! 
      At this point, we know that we have four unique type 111 arms 
    */ 
    return true;  
  }/* end HaveFourUniqueType111ExternalArms */ 

  //===========================================================================
  void Arm::Classify(void) {
    if (mTerminalNodes.size() == 1) {
      mArmType = ARM_LOOP; 
    } else {
      if (mTerminalNodes[0]->IsTypeM() && mTerminalNodes[1]->IsTypeM()) {
        mArmType = ARM_MM_111; 
      } else if (mTerminalNodes[0]->IsTypeM() || mTerminalNodes[1]->IsTypeM()){
        mArmType = ARM_MN_111; 
      } else {
        mArmType = ARM_NN_111; 
      }
      
      // This changes _111 to _100 by definition
      int btype = mTerminalSegments[0]->GetBurgersType(); 
      if (btype <= 3) {
        mArmType += 3; 
      }
    }
    
    /*!
      Now set every segment in this arm to the same type...
    */ 
    ArmSegment *currentSegment = const_cast<ArmSegment*>(mTerminalSegments[0]);
    FullNode *currentNode;  
    if (currentSegment->GetEndpoint(0) == mTerminalNodes[0] ||
        currentSegment->GetEndpoint(1) == mTerminalNodes[0]) {
      currentNode = mTerminalNodes[0];
    } else if (mTerminalNodes.size() == 2 && 
               ( currentSegment->GetEndpoint(0) == mTerminalNodes[1] ||
                 currentSegment->GetEndpoint(1) == mTerminalNodes[1])) {
      currentNode = mTerminalNodes[1];
    } else {
      throw string("Cannot find matching terminal node in arm for either segment endpoint"); 
    } 

    ArmSegment *lastSegment; 
    uint32_t numSeen = 0; 
    if (mTerminalSegments.size() == 1)  lastSegment = currentSegment; 
    else lastSegment = const_cast<ArmSegment*>(mTerminalSegments[1]); 
    while (true) {
      dbprintf(5, "Classifying segment %s\n", currentSegment->Stringify().c_str()); 
      currentSegment->SetMNType(mArmType); 
      gNumClassified++; 
      numSeen ++; 
      if (currentSegment == lastSegment) {
        break; 
      }
      currentNode = currentSegment->GetOtherEndpoint(currentNode); 
      currentSegment = currentNode->GetOtherNeighbor(currentSegment);       
    }
#ifdef DEBUG
    if (numSeen != mNumSegments) {
      throw string("Error in Arm ")+doubleToString(mArmID)+":  classified "+doubleToString(numSeen)+" segments, but expected "+ doubleToString(mNumSegments); 
    }
#endif
    return; 
  }

  //===========================================================================
  double Arm::GetLength(void) {
    ArmSegment *currentSegment = const_cast<ArmSegment*>(mTerminalSegments[0]);
    double length = 0; 
    FullNode *currentNode = NULL, *lastNode=NULL;  
    if (*currentSegment->GetEndpoint(0) == *mTerminalNodes[0] ||
        *currentSegment->GetEndpoint(1) == *mTerminalNodes[0]) {
      currentNode = mTerminalNodes[0];
      if (mTerminalNodes.size() == 2) {
        lastNode = mTerminalNodes[1];    
      } else {
        lastNode = currentNode; 
      }
    } else if (mTerminalNodes.size() == 2 && 
               ( *currentSegment->GetEndpoint(0) == *mTerminalNodes[1] ||
                 *currentSegment->GetEndpoint(1) == *mTerminalNodes[1])) {
      currentNode = mTerminalNodes[1];
      lastNode = mTerminalNodes[0]; 
    } else {
      throw string("Cannot find matching terminal node in arm for either segment endpoint"); 
    }

    /*!
      Compute the arm length in a loop
    */
    ArmSegment *firstSegment = currentSegment, *lastSegment = currentSegment; 
    uint32_t numMeasured = 0; 
    if (mTerminalSegments.size() == 2)  {
      lastSegment = const_cast<ArmSegment*>(mTerminalSegments[1]); 
    }
    dbprintf(5, "firstSegment is %s\n", firstSegment->Stringify().c_str()); 
    dbprintf(5, "lastSegment is %s\n", lastSegment->Stringify().c_str()); 
    while (1) {
      length += currentSegment->GetLength(); 
      gNumArmSegmentsMeasured++; 
      numMeasured ++; 
      dbprintf(5, "Added length to arm %d: segment %d, length %.2f\n", mArmID, currentSegment->GetID(), currentSegment->GetLength()); 
      if (*currentSegment == *lastSegment) {
        dbprintf(5, "currentSegment is the last segment.  Stopping\n"); 
        break; 
      }
      currentNode = currentSegment->GetOtherEndpoint(currentNode); 
      dbprintf(5, "(1) currentNode is %s\n", currentNode->Stringify().c_str()); 
      if (currentNode == lastNode) {
        dbprintf(5, "Found last node\n"); 
        break; 
      }
      if (currentNode->GetNumNeighbors() == 2) {
        currentSegment = currentNode->GetOtherNeighbor(currentSegment); 
        dbprintf(5, "currentNode is interior, so currentSegment is now %s\n", currentSegment->Stringify().c_str()); 
      } else  {
        // Find the corresponding ghost node that lets us continue 
        if (currentSegment->mGhostEndpoints.size() != 1) {              
          throw string("Error -- expected a single ghost endpoint, found ")+doubleToString(currentSegment->mGhostEndpoints.size() ); 
        }
        if (! (*currentSegment->mGhostEndpoints[0] == *currentNode)) {
          throw string("Error, expected ghost endpoint of current segment to equal current node"); 
        }
        currentNode =  currentSegment->mGhostEndpoints[0];
        dbprintf(5, "currentNode is wrapped, using duplicate %s\n", currentNode->Stringify().c_str()); 
        if (*currentNode == *lastNode) {
          dbprintf(5, "currentNode is last node, stopping\n"); 
          break; 
        } else {
          if (currentNode->GetNumNeighbors() != 2) {
            throw string("Error -- current node must have two neighbors as it is not an endpoint"); 
          }
          currentSegment = currentNode->GetOtherNeighbor(currentSegment); 
          dbprintf(5, "currentSegment is now %s\n", currentSegment->Stringify().c_str()); 
        }
        
         if (!currentSegment) {
          throw string("Error in GetLength: cannot find way to continue along arm past node with single neighbor."); 
        }
        
      }     
    } 
#ifdef DEBUG
    if (numMeasured != mNumSegments) {
      throw string("Error in GetLength: arm ")+doubleToString(mArmID)+" has "+doubleToString(mNumSegments)+" but we only measured "+doubleToString(numMeasured); 
    }
#endif
    return length; 
  }    
  //===========================================================================
  void Arm::CheckForButterfly(void) {
    /*!
      There must be two terminal nodes else give up
    */ 
    if (mTerminalNodes.size() != 2 )  return; 
    
    /*! 
      The arm must be type 100, else give up. 
    */ 
    if (mTerminalSegments[0]->GetBurgersType() > 3 || 
        mTerminalSegments[0]->GetBurgersType() == 0)  return; 


    /*!
      One of the nodes must be a type 3 node and the other must be either type 3 or type -4 or type -44.  Note that if a node is both a type -3 and type -33, it is considered a type -3 node, which means that here if the node is already type -3, we don't consider it, but if it is type -33, we do.  
    */
    
    /*!
      first check node 0 against node 1, then node 1 against node 0
    */ 
    int firstNodeNum = 2; 
    while (firstNodeNum --) {
      FullNode *firstNode = mTerminalNodes[firstNodeNum], 
        *otherNode = mTerminalNodes[1-firstNodeNum]; 
      
      int firstNodeType = firstNode->GetNodeType(), 
        otherNodeType =otherNode->GetNodeType();
      
      if (firstNodeType == 3 && otherNodeType ==  -44) {
        /*! 
          type3node is a "special butterfly"  
          Note -- we don't check if firstNodeType is -33 -- would be redundant.
          We don't check if firstNodeType is -3, because -33 "loses" to -3. 
        */ 
        firstNode->SetNodeType(-33); 
        
      } else if (firstNodeType == 3 || firstNodeType == -33) {          
        /*! 
          Both terminal nodes might be "normal butterflies" (type -3). Need to collect and examine all "non-body" armsegments.    If any two are duplicates or are not type 111 arms, then it is not a special butterfly.  We are only interested in first 3 types but they are 1-based, so allocate four slots.     
        */ 
        if (HaveFourUniqueType111ExternalArms()) {
          firstNode->SetNodeType(-3); 
          otherNode->SetNodeType(-3); 
          /*! 
            Since type -3 "outranks" all other node types, we can stop here
          */ 
          return;
        }
      }
    }
    return ;
  }/* end CheckForButterfly()*/ 

  //===========================================================================
  std::string Arm::Stringify(void) const {
    std::string s(string("(arm): ") + 
#ifdef DEBUG
                  "number " + doubleToString(mArmID) + 
                  ", numSegments = " +doubleToString(mNumSegments) +
#endif
                  ", Type " +  doubleToString(mArmType) + string("\n"));
    int num = 0, max = mTerminalNodes.size(); 
    while (num < max) {
      s+= "Terminal Node " + doubleToString(num) + string(": ");
      if (mTerminalNodes[num]) {
        s += mTerminalNodes[num]->Stringify() + string("\n"); 
      } else {
        s += "(NULL)\n"; //deleted because it was useless.  
      }
      ++num; 
    }
    max = mTerminalSegments.size(); num = 0; 
    while (num < max) {
      s+= "Terminal Segment " + doubleToString(num) + string(": "); 
      if (mTerminalSegments[num]) {
        s += mTerminalSegments[num]->Stringify() + string("\n"); 
      } else {
        s+= "(NULL)\n"; 
      }
      ++num; 
    }
     return s; 
  }
  //===========================================================================
  // see also DebugPrintArms() -- this just tabulates a summary
  void DataSet::PrintArmStats(void) {
    //if (!dbg_isverbose()) return;
    //dbprintf(3, "Beginning PrintArmStats()"); 
    double armLengths[9] = {0}, totalArmLength=0; 
    uint32_t numArms[9] = {0}, totalArms=0;  // number of arms of each type

    vector<Arm>::iterator armpos = mArms.begin(), armend = mArms.end(); 
    while (armpos != armend) { 
      double length = armpos->GetLength(); 
      armLengths[armpos->mArmType] += length;
      totalArmLength += length; 
      numArms[armpos->mArmType]++; 
      totalArms++; 
      ++armpos; 
    }

    printf("\n"); 
    printf("===========================================\n"); 
    printf("total Number of arms: %d\n", totalArms); 
    printf("total length of all arms: %.2f\n", totalArmLength); 
    printf("Number of segments classified in arm: %d\n", gNumClassified); 
    printf("Number of segments measured in arm: %d\n", gNumArmSegmentsMeasured); 
    printf("Number of segments wrapped: %d\n", gNumWrapped); 
    printf("===========================================\n"); 
    printf("ARM_UNKNOWN: number of arms = %d\n", numArms[0]);
    printf("ARM_UNKNOWN: total length of arms = %.2f\n", armLengths[0]); 
    printf("----------------------\n"); 
    printf("ARM_UNINTERESTING: total number of arms = %d\n", numArms[1]); 
    printf("ARM_UNINTERESTING: total length of arms = %.2f\n", armLengths[1]); 
    printf("----------------------\n"); 
    printf("ARM_LOOP: total number of arms = %d\n", numArms[2]); 
    printf("ARM_LOOP: total length of arms = %.2f\n", armLengths[2]); 
    printf("--------------------------------------------\n"); 
    printf("ARM_MM_111: total number of arms = %d\n", numArms[3]); 
    printf("ARM_MM_111: total length of arms = %.2f\n", armLengths[3]); 
    printf("----------------------\n"); 
    printf("ARM_MN_111: total number of arms = %d\n", numArms[4]); 
    printf("ARM_MN_111: total length of arms = %.2f\n", armLengths[4]); 
    printf("----------------------\n"); 
    printf("ARM_NN_111: total number of arms = %d\n", numArms[5]); 
    printf("ARM_NN_111: total length of arms = %.2f\n", armLengths[5]); 
    printf("--------------------------------------------\n"); 
    printf("ARM_MM_100: total number of arms = %d\n", numArms[6]); 
    printf("ARM_MM_100: total length of arms = %.2f\n", armLengths[6]); 
    printf("----------------------\n"); 
    printf("ARM_MN_100: total number of arms = %d\n", numArms[7]); 
    printf("ARM_MN_100: total length of arms = %.2f\n", armLengths[7]); 
    printf("----------------------\n"); 
    printf("ARM_NN_100: total number of arms = %d\n", numArms[8]); 
    printf("ARM_NN_100: total length of arms = %.2f\n", armLengths[8]); 

    printf("\n\n");
    
    // check against segment lengths: 
    uint32_t numSegments[9] = {0}, totalSegments=0, culledSegments=0;  // number of arms of each type
    double segmentLengths[9] = {0}, totalSegmentLength=0, culledLength=0; 
    std::vector<ArmSegment *>::iterator segpos = mFinalArmSegments.begin(), segend = mFinalArmSegments.end(); 
    while (segpos != segend) {
      ArmSegment *seg = *segpos; 
      double length = seg->GetLength(); 
      // Cull out half the wrapped segments in such a way that for each culled, there is an identical one remaining:
      if (!seg->Cullable()) {
        segmentLengths[seg->GetMNType()] += length;  
        totalSegmentLength += length; 
        numSegments[seg->GetMNType()]++; 
        totalSegments++; 
      } else {
        culledSegments++;
        culledLength +=length; 
      }
      ++segpos; 
    }
        
    
    printf("===========================================\n"); 
    printf("REALITY CHECK:  total length of all segments, skipping wrapped segments\n"); 
    printf("total Number of segments: %d\n", totalSegments); 
    printf("total length of all segments: %.2f\n", totalSegmentLength); 
    printf("===========================================\n"); 
    printf("ARM_UNKNOWN: number of segs = %d\n", numSegments[0]);
    printf("ARM_UNKNOWN: total length of segments = %.2f\n", segmentLengths[0]); 
    printf("----------------------\n"); 
    printf("ARM_UNINTERESTING: total number of segments = %d\n", numSegments[1]); 
    printf("ARM_UNINTERESTING: total length of segments = %.2f\n", segmentLengths[1]); 
    printf("----------------------\n"); 
    printf("ARM_LOOP: total number of segments = %d\n", numSegments[2]); 
    printf("ARM_LOOP: total length of segments = %.2f\n", segmentLengths[2]); 
    printf("--------------------------------------------\n"); 
    printf("ARM_MM_111: total number of segments = %d\n", numSegments[3]); 
    printf("ARM_MM_111: total length of segments = %.2f\n", segmentLengths[3]); 
    printf("----------------------\n"); 
    printf("ARM_MN_111: total number of segments = %d\n", numSegments[4]); 
    printf("ARM_MN_111: total length of segments = %.2f\n", segmentLengths[4]); 
    printf("----------------------\n"); 
    printf("ARM_NN_111: total number of segments = %d\n", numSegments[5]); 
    printf("ARM_NN_111: total length of segments = %.2f\n", segmentLengths[5]); 
    printf("--------------------------------------------\n"); 
    printf("ARM_MM_100: total number of segments = %d\n", numSegments[6]); 
    printf("ARM_MM_100: total length of segments = %.2f\n", segmentLengths[6]); 
    printf("----------------------\n"); 
    printf("ARM_MN_100: total number of segments = %d\n", numSegments[7]); 
    printf("ARM_MN_100: total length of segments = %.2f\n", segmentLengths[7]); 
    printf("----------------------\n"); 
    printf("ARM_NN_100: total number of segments = %d\n", numSegments[8]); 
    printf("ARM_NN_100: total length of segments = %.2f\n", segmentLengths[8]);
    
    printf("\n----------------------\n"); 
    printf("CULLED segments = %d\n", culledSegments); 
    printf("CULLED length = %.2f\n", culledLength); 

    printf("Wrapped lengths: %.2f\n", gSegLen); 
   return; 
  }

  //===========================================================================
  void DataSet::TestRestrictSubspace(void) {
    rclib::Point<float> datamin , datamax;
    GetBounds(datamin, datamax); 
    datamax /= 2.0; 
    SetSubspace(datamin, datamax);
    return; 
  }

  //===========================================================================
  void DataSet::ReadBounds(void) {
    dbprintf(2,"Beginning ReadBounds\n");

    try {
      ifstream dumpfile(mDataFilename.c_str());
      if (!dumpfile.good()){
        throw string("Cannot open dumpfile ") + mDataFilename;
      }
      else {
        dbprintf(2, "Parsing file %s\n", mDataFilename.c_str());
      }
      mFileVersion = 42; 
      dbprintf(2, "Looking for file version..."); 
      int lineno = 0; 
      //  we need to check file version -- if not found, then 
      char linebuf[2048]="";
      while (dumpfile.good()) {
        dbprintf(5, "scanning new line \"%s\"\n", linebuf); 
        if (strstr(linebuf, "File version number")) {
          // bingo
          dumpfile.getline(linebuf, 2048); // get comment
          dumpfile.getline(linebuf, 2048); // blank line
          dumpfile >> mFileVersion; 
          break; 
        }
        else if (strstr(linebuf, "dataFileVersion =")) {
          // for versions 4 and newer
          sscanf(linebuf, "dataFileVersion = %lu", &mFileVersion);
          break; 
        }
        else if (strstr(linebuf, "Primary lines")) {
          // no version available
          mFileVersion = 0; // just to be clear
          break;
        }
        dumpfile.getline(linebuf, 2048);
        lineno++;
      }
      dumpfile.seekg(0, std::ios::beg);// seek to beginning again 
      dbprintf(2, "file version set to %u\n", mFileVersion); 
      
      if (mFileVersion == 42) {
        throw string("Cannot find file version -- not a paraDIS file?"); 
      }
      int i=0; 
      if (mFileVersion == 0) {
        string token;
        while (dumpfile.good() && token != "minSideX")
          dumpfile >> token;
        while (dumpfile.good() && i<3) {
          dumpfile >> token >> mDataMin[i] >> token  >> token  >> mDataMax[i] >>token; 
          i++;
        }
      }
      else  if (mFileVersion < 4) {
        char linebuf[2048]="";
        while (dumpfile.good() && !strstr(linebuf, "Minimum coordinate values"))
          dumpfile.getline(linebuf, 2048);
        if (!dumpfile.good()){
          throw string("Cannot find minimum bounds from dumpfile ") + mDataFilename;  
      }
        dumpfile.getline(linebuf, 2048);//get that stupid "#"
        dumpfile >> mDataMin[0] >> mDataMin[1] >> mDataMin[2];
        
        while (dumpfile.good() && !strstr(linebuf, "Maximum coordinate values"))
          dumpfile.getline(linebuf, 2048);
        if (!dumpfile.good()){
          throw string( "Cannot find maximum bounds from dumpfile ")+ mDataFilename;
        }
        dumpfile.getline(linebuf, 2048);//get that stupid "#"
        dumpfile >> mDataMax[0] >> mDataMax[1] >> mDataMax[2];
      } 
      else {  // must be file version 4 
        char linebuf[2048]="";
        while (dumpfile.good() && !strstr(linebuf, "minCoordinates")){
          dumpfile.getline(linebuf, 2048);
        }
        dumpfile >> mDataMin[0] >> mDataMin[1] >> mDataMin[2];
        
        while (dumpfile.good() && !strstr(linebuf, "maxCoordinates")){
          dumpfile.getline(linebuf, 2048);
        }
        dumpfile >> mDataMax[0] >> mDataMax[1] >> mDataMax[2];
      }
      mDataSize  = mDataMax - mDataMin;
    } catch (string err) {
      throw string("Error in GetBounds:\n")+err;
    }
    
    dbprintf(2,"Done with ReadBounds\n");
    return; 
  }
  
  
  //====================================================================== 
  void DataSet::ReadMinimalNodeFromFile(uint32_t &lineno, std::ifstream &datafile) {
    MinimalNode theNode;
    char comma;
    int domainID, nodeID; 
    long old_id_junk, constraint_junk, numNeighbors; 
    float x,y,z, float_junk; 
    //-----------------------------------------------
    // read the first line of node information
    if (mFileVersion > 0) {
      datafile >> domainID >> comma >> nodeID;
    }
    else { // old-style
      datafile >> nodeID >> old_id_junk;
    }
    datafile >> x >> y >> z >> numNeighbors >> constraint_junk;    
    if (mFileVersion == 0){
      datafile  >> domainID >>  old_id_junk; 
    }
    if (!datafile.good()) {
      throw string("error reading node"); 
    }
    ++lineno;

    NodeID theID(domainID, nodeID); 
    //------------------------------------------------
    // done reading first line of node information
    if (rclib::InBounds(rclib::Point<float>(x,y,z), mSubspaceMin, mSubspaceMax ))
      theNode.InBounds(true); 

    theNode.SetID(theID); 
    
    //------------------------------------------
    // read neighbor information
    Neighbor theNeighbor; 
    int neighbornum = 0; 
    while (neighbornum < numNeighbors && datafile.good()) {
      if (mFileVersion > 0)
        datafile >> domainID >> comma >> nodeID;   
      else {
        domainID   = 0;
        datafile >> nodeID;
      }
      // read and discard the entire next line, as we do not need burgers values right now. . 
      NodeID neighborID(domainID, nodeID); 
      datafile >> float_junk >> float_junk >> float_junk >> float_junk >> float_junk >> float_junk;
      theNeighbor.SetEndpoints(theNode.GetNodeID(), neighborID); 
      
      mMinimalNeighbors.insert(theNeighbor);  //set semantics guarantee it will only be inserted if unique, yay!
      // however, this means we have to get it back out to save its address
      std::set<Neighbor>::iterator pos = 
        mMinimalNeighbors.find(theNeighbor); 
      theNode.AddNeighbor(&(*pos)); 
      
      if (!datafile.good()) {
        throw string("error reading neighbor number ") + doubleToString(neighbornum);
      }
      ++lineno; 
      ++neighbornum; 
    }; 
    
    //------------------------------------------------
    // done reading neighbor information
    theNode.SetFileOrderIndex(mMinimalNodes.size()); 
    mMinimalNodes.push_back(theNode); 
    dbprintf(5, "pushed back new Minimal Node: %s\n", theNode.Stringify().c_str()); 
    return; 
  }
  
  //===========================================================================
  void DataSet::CreateMinimalNodes(void){
    dbprintf(2, "CreateMinimalNodes started...\n"); 
    timer theTimer; 
    theTimer.start(); 
    dbprintf(2, "Size of a minimal_node is %d bytes\n",  sizeof(MinimalNode));
    char linebuf[2048]="";
    uint32_t nodenum = 0;
    uint32_t lineno = 1; 
    ReadBounds(); 
    try {
      ifstream datafile(mDataFilename.c_str()); 
      if (!datafile.good()){
        throw string("Error: cannot open datafile ")+ mDataFilename;
      }
      // =================================================
      /* get number of nodes */ 
      mTotalDumpNodes = 0; 
      if (mFileVersion == 0) {
        while (datafile.good() && !strstr(linebuf, "Number of nodes")) {
          datafile.getline(linebuf, 2048);  
          ++lineno;
        }
        if (!mTotalDumpNodes && datafile.good()) 
          datafile >> mTotalDumpNodes; // did not read the whole line...
      }
      else if (mFileVersion < 4 ) {
        while (datafile.good() && !strstr(linebuf, "Node count")) {
          datafile.getline(linebuf, 2048);
        }
        datafile.getline(linebuf, 2048);//should be "#"
        ++lineno;
        if (!strstr(linebuf, "#")) {
          dbprintf(0, "ERROR: Expected line:\n#\n but got:\n%s\n", linebuf);
        }
        datafile >> mTotalDumpNodes;
        if (!datafile) {
          throw string("Error getting number of nodes"); 
        }
      } 
      else { //  
        while (datafile.good() && !strstr(linebuf, "nodeCount =")) {
          datafile.getline(linebuf, 2048);  
          ++lineno;
        }
        // dataFileVersion 4  just has different header, just parse it and treat it like the old "new" version
        long nodeCount = 0; 
        sscanf(linebuf, "nodeCount = %lu", &nodeCount);
        mTotalDumpNodes = nodeCount; 
      }
         
      if (!mTotalDumpNodes) {
        throw string( "Cannot find number of nodes in datafile");
      }
      // got number of nodes
      // =================================================
     
      //ready to read nodes in....
      dbprintf(1, "Expecting %d nodes, for a total of %.1f megabytes in basic form\n",
               mTotalDumpNodes, (float)mTotalDumpNodes*sizeof(MinimalNode)/(1024.0*1024));
      
      while (datafile.good() && !strstr(linebuf, "Secondary lines:")) {
        datafile.getline(linebuf, 2048);  
        ++lineno; 
      }
      
      if (mFileVersion > 0 && mFileVersion < 4) {
        datafile.getline(linebuf, 2048);//should be "#"
        ++lineno; 
        if (!strstr(linebuf, "#")){
          dbprintf(0, "ERROR: Expected line:\n#\n but got:\n%s\n", linebuf);
          throw string("Error in file format"); 
        }
      }
      
      if (!datafile.good()) {
        throw string( "Error: cannot find first node in data file");
      }
      
      double theTime=theTimer.elapsed_time(), thePercent=0;
      while (datafile.good() && nodenum++ < mTotalDumpNodes) {
        if (dbg_isverbose()) 
          Progress(theTimer, nodenum, mTotalDumpNodes, thePercent, 5, theTime, 60, "Reading datafile"); 
        ReadMinimalNodeFromFile(lineno, datafile);  
        
      }
      dbprintf(1, "\n"); 
    } catch (string err) {
      throw string("Error in GetNodes while reading node ") + doubleToString(nodenum) +" at line " + doubleToString(lineno) + ":\n" + err; 
    }
    return; 
    dbprintf(2, "CreateMinimalNodes ended...\n"); 
  }

  //===========================================================================
  bool DataSet::Mkdir(const char *dirname) {
    dbprintf(3, "Mkdir(%s)",dirname); 
    DIR *dir = opendir(dirname); 
    if (!dir) {
      if (mkdir(dirname, S_IRWXU | S_IRWXG | S_IRWXO )) {
        string errmsg = string("Warning:  could not create debug output directory: ") + mDebugOutputDir+ ". Stats will not be written." ; 
        cerr << errmsg << endl; 
        dbprintf(1, errmsg.c_str()); 
        return false; 
      } else {
        dbprintf(1, "Warning:  created debug output directory ", dirname);         
      }
    }else {
      closedir(dir); 
    }
    return true; 
  }
  //===========================================================================
  void DataSet::DebugPrintMinimalNodes(void) {


    std::string filename = mDebugOutputDir + string("/MinimalNodes-list.txt"); 
    dbprintf(1, "Writing minimal nodes to debug file %s\n", filename.c_str()); 
    if (!Mkdir(mDebugOutputDir.c_str())) return; 
    ofstream debugfile (filename.c_str()); 
    if (!debugfile) {
      string errmsg = string("Warning:  cannot open debug file ")+filename+". " + strerror(errno);
      cerr << errmsg << endl; 
      dbprintf(1, "%s\n", errmsg.c_str()); 
      return; 
    }
    debugfile << "data bounds: " << mDataMin.Stringify() << ", " << mDataMax.Stringify() << endl; 
    debugfile << "subspace bounds: " << mSubspaceMin.Stringify() << ", " << mSubspaceMax.Stringify() << endl; 
    std::vector<MinimalNode>::iterator pos = mMinimalNodes.begin(),
      endpos = mMinimalNodes.end(); 
    uint32_t nodenum = 0; 
    while (pos != endpos) {
      debugfile << "MinimalNode " << nodenum++ <<":\n"<< pos->Stringify(true) << endl;
      debugfile << "***************************************************" << endl << endl; 
      ++pos; 
    }
    debugfile <<"Total minimal nodes: " << nodenum << endl; 
    debugfile << "Total memory for minimal nodes: " << nodenum * sizeof(MinimalNode) << endl; 
    return ;
  }
  //===========================================================================
  std::vector<MinimalNode *> DataSet::GetNeighborMinimalNodes(const MinimalNode &inode) { 
    std::vector<MinimalNode *> neighborNodes; 
    
    std::vector<const Neighbor *>::const_iterator neighbor_pos = inode.GetNeighbors().begin(), neighbor_endpos = inode.GetNeighbors().end(); 
    
    while (neighbor_pos != neighbor_endpos) {      
      std::vector<MinimalNode>::iterator otherEnd =  
        lower_bound(mMinimalNodes.begin(), mMinimalNodes.end(), 
                    MinimalNode((*neighbor_pos)->GetOtherEndpoint(inode.GetNodeID())));

      if (otherEnd == mMinimalNodes.end()) 
        throw string("Error in DataSet::GetNeighborMinimalNodes -- cannot find other end of neighbor relation from node ")+inode.Stringify(true); 
      // we have assumed that otherEnd exists in mMinimalNodes...
      if (! (*otherEnd == inode))
        neighborNodes.push_back(&(*otherEnd)); 

      ++neighbor_pos; 
    }

    return neighborNodes; 
  }    
  //===========================================================================
  void DataSet::MarkOOBNodeChain( MinimalNode &node,  MinimalNode &original) {
    MinimalNode *current = &node, *previous = &original;
    
    /* 
       Create a loop, a bit torturous this way but avoids recursion, which might be very deep for long arms 
    */ 
    while (current != NULL) {
      /* 
         If terminal, mark with full info and return 
      */ 
      std::vector<MinimalNode *>neighborNodes = GetNeighborMinimalNodes(*current); 
      if (neighborNodes.size() != 2) {
        current->SetKeep();    
        return; 
      } 
      
      /*
        Interior node: determine if it is connected to IB, if not, mark "need to keep"
      */ 
      current->SetKeep(); 
      std::vector<MinimalNode *>::iterator neighbor = neighborNodes.begin(); 
      if (*neighbor == previous) {
        ++neighbor; 
      } if (*neighbor == previous) { 
        throw std::string("Error in DataSet::MarkOOBNodeChain -- a cycle of only two nodes was found."); 
      }
      /* 
         If neighbors of IB nodes, then end of chain:  return
      */
      if ((*neighbor)->InBounds()) {
        return; 
      }     
      previous = current; 
      current = *neighbor;       
    }/* end while (current != NULL) */ 
    return; 
  }

  //===========================================================================
  void DataSet::ClassifyMinimalNodes(void){ 
    /* Look at each node.  If it's in bounds, it is FULL, and each neighbor is alos full.  If the neibhbor is out of bounds and unmarked, then you must call MarkOOBNodeChain on it to capture PARTIAL and FULL out of bounds nodes. */
    dbprintf(2, "ClassifyMinimalNodes started...\n"); 
    
    sort(mMinimalNodes.begin(), mMinimalNodes.end()); 
    std::vector<MinimalNode>::iterator node_pos = mMinimalNodes.begin(), node_endpos = mMinimalNodes.end(); 
    int nodenum = 0; 
    try {
      while (node_pos != node_endpos) {
        if (node_pos->GetKeep() || !  node_pos->InBounds()) {
          /* At the top level, we only inspect nodes that are inbounds, that we have not seen before.  OOB neighbors are caught in CollapseNodeChain */ 
          ++node_pos; 
          continue;  
        }
        node_pos->SetKeep(); /* We need it, yes, my Precious */ 
        std::vector<MinimalNode *> neighborNodes = 
          GetNeighborMinimalNodes(*node_pos); 
        std::vector<MinimalNode *>::iterator neighbor = neighborNodes.begin();
        while (neighbor != neighborNodes.end()) {
          if (!(*neighbor)->InBounds() && !(*neighbor)->GetKeep()) {
            MarkOOBNodeChain(*(*neighbor), *node_pos); 
          }
          ++neighbor; 
        }
        ++node_pos; 
        ++nodenum; 
      } 
    } catch (string err) {
      err = string("Error in ClassifyMinimalNodes, node ") + doubleToString(nodenum) + ": " + err; 
      throw err; 
    }
    dbprintf(2, "ClassifyMinimalNodes ended...\n\n");     
    return; 
  }

  //===========================================================================
  bool cullminimalnode(const MinimalNode &node) { 
    return !node.GetKeep(); 
  } 
  
  //===========================================================================
  /*!
    for use in CullAndResortMinimalNodes
   */
  bool FileOrderPredicate(const MinimalNode &n1, const MinimalNode &n2) {
    return n1.ComesBeforeInFile(n2); 
  }

  //===========================================================================
  void DataSet::CullAndResortMinimalNodes(void) {
    dbprintf(2, "CullMinimalNodes started...\n\n"); 
    std::vector<MinimalNode>::iterator last = 
      remove_if(mMinimalNodes.begin(), mMinimalNodes.end(), cullminimalnode); 
    mMinimalNodes.erase(last, mMinimalNodes.end()); 

    /*!
      Re-sort in file order to make finding full node counterparts go quicker
    */ 
    sort(mMinimalNodes.begin(), mMinimalNodes.end(), FileOrderPredicate); 
    
    dbprintf(2, "CullMinimalNodes ended...\n\n"); 
    return; 
  }



  //===========================================================================
  void DataSet::ReadFullNodeFromFile(std::ifstream &datafile, MinimalNode &theNode){
    FullNode *fullNode = new FullNode; 
    char comma;
    long old_id_junk, constraint_junk, numNeighbors; 
    float float_junk, location[3]; 
    int domainID, nodeID, neighborDomain, neighborID, numskipped=0;
    string junkstring;
    dbprintf(5, "ReadFullNodeFromFile\n"); 
    try {
      while (!( theNode == *fullNode)) {
        //-----------------------------------------------
        // read the first line of node information and see if it matches theNode
        if (mFileVersion > 0) {
          datafile >> domainID >> comma >> nodeID;
          if (!datafile.good()) {
            throw string("error reading domainID and nodeID of node"); 
          }
          dbprintf(5, "Got node id (%d, %d)\n", domainID, nodeID); 
        } else {
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
        dbprintf(5, "read location (%f, %f, %f)\n", location[0], location[1], location[2]); 
        datafile >> numNeighbors >> constraint_junk;
        if (!datafile.good()) {
          throw string("error reading numNeighbors"); 
        }
        if (mFileVersion == 0){
          datafile  >> domainID >>  old_id_junk; 
          if (!datafile.good()) {
            throw string("error reading domainID"); 
          }
        }
        if (!datafile.good()) {
          throw string("error reading node"); 
        }
        
        if (theNode == NodeID(domainID, nodeID)) {
          *fullNode = theNode; 
        }
        else {
          /*!
            read past the remaining node information for this node. 
          */ 
          //dbprintf(5, "skipping node: (%d, %d)\n", domainID, nodeID); 
          getline(datafile, junkstring); // finish the current line
          if (!datafile.good()) {
            throw string("error looking for end of line"); 
          }
          int neighborNum = 0; 
          while (neighborNum < numNeighbors) {
            int linenum = 0; while (linenum < 2) {
              getline(datafile, junkstring);  
              if (!datafile.good()) {
                throw string("error reading line ")+ doubleToString(linenum) + string(" of neighbor ") + doubleToString(neighborNum);
              }
              ++linenum;
            }
            ++neighborNum; 
          } 
          ++numskipped; 
        }      
      }
      dbprintf(5, "Found node info for minimal node (%d, %d)\n", domainID, nodeID); 
      //------------------------------------------------
      // done reading first line of node information    
      fullNode->SetLocation(location); 
      fullNode->SetIndex(mFullNodes.size()); 
      
      //------------------------------------------
      // read neighbor information
      int neighbornum = 0; 
      float burgers[3]; 
      //Neighbor currentNeighbor; //from file
      try {
        ArmSegment *currentSegment = NULL; 
        while (neighbornum < numNeighbors && datafile.good()) {
          currentSegment = new ArmSegment; 
          if (mFileVersion > 0) {
            datafile >> neighborDomain >> comma >> neighborID;   
            if (!datafile.good()) {
              throw string("error reading neighborDomain and neighborID of neighbor"); 
            }
          }
          else {
            neighborDomain   = 0;
            datafile >> neighborID;
            if (!datafile.good()) {
              throw string("error reading neighborID"); 
            }
          }
          currentSegment->SetEndpoints(fullNode, neighborDomain, neighborID); 
          
          // read burgers value and discard "nx, ny, nz". 
          datafile >> burgers[0] >> burgers[1] >> burgers[2] >> float_junk >> float_junk >> float_junk;
          if (!datafile.good()) {
            throw string("error reading burgers values"); 
          }
          
          currentSegment->ComputeBurgersType(burgers); 
          
          ArmSegmentSet::iterator foundSegment = mQuickFindArmSegments.find(currentSegment); 
          if (foundSegment == mQuickFindArmSegments.end()) {
            currentSegment->SetID(); 
            mQuickFindArmSegments.insert(currentSegment); 
            foundSegment = mQuickFindArmSegments.find(currentSegment); 
          } else {
            (*foundSegment)->ReplacePlaceholder(fullNode);             
          }
          fullNode->AddNeighbor((*foundSegment)); 
         
          ++neighbornum; 
        }; // done reading neighbor information
      } catch (string err) {
        throw string("Error in DataSet::ReadFullNode reading neighbor ")+doubleToString(neighbornum)+":" + err; 
      }
    } catch (string err) {
      throw string("Error trying to read full node info corresponding to ")+theNode.Stringify(true) + string("numskipped is ")+doubleToString(numskipped) + string("\n") + err;
    } 

    fullNode->SetNodeType(); 
    dbprintf(5, "Done creating full node %s\n", fullNode->Stringify(true).c_str()); 


    mFullNodes.push_back(fullNode); 
    return; 

  } /* end ReadFullNodeFromFile */ 
  //===========================================================================
  void DataSet::CreateFullNodesAndArmSegments(void){
    dbprintf(2, "CreateFullNodes started...\n"); 
    reverse(mMinimalNodes.begin(), mMinimalNodes.end());
    timer theTimer; 
    theTimer.start(); 
    dbprintf(2, "Size of a full node is %d bytes, so expect to use %d megabytes\n",  sizeof(FullNode), mMinimalNodes.size()*sizeof(FullNode)/1000000);
    char linebuf[2048]="";
    uint32_t nodenum = 0;
    try {
      ifstream datafile(mDataFilename.c_str()); 
      if (!datafile.good()){
        throw string("Error: cannot open datafile ")+ mDataFilename;
      }
 
      
      while (datafile.good() && !strstr(linebuf, "Secondary lines:"))
        datafile.getline(linebuf, 2048);  
      
      if (mFileVersion > 0 && mFileVersion < 4) {
        datafile.getline(linebuf, 2048);//should be "#"
        if (!strstr(linebuf, "#")) {
          dbprintf(0, "WARNING: Expected line:\n#\n but got:\n%s\n", linebuf);
          throw string("Bad file format in line: ")+string(linebuf); 
        }
      }
      
      if (!datafile.good()) {
        throw string( "Error: cannot find first node in data file");
      }
      
      double theTime=theTimer.elapsed_time(), thePercent=0;
      uint32_t nodelimit = mMinimalNodes.size(); 
      std::vector<MinimalNode>::reverse_iterator rpos = mMinimalNodes.rbegin(), rend = mMinimalNodes.rend(); 
      //dbprintf(2, "\n"); 
      while (datafile.good() && nodenum < nodelimit && rpos != rend) {
        if (dbg_isverbose()) 
          Progress( theTimer, nodenum, nodelimit, thePercent, 5, theTime, 60, "Reading datafile"); 
        ReadFullNodeFromFile(datafile, *rpos++);  
        /*!
          We have "used up" the last node in the vector, delete it to save memory
        */ 
        mMinimalNodes.pop_back(); 
        ++nodenum; 
      }
      dbprintf(1, "\n"); 
      if (nodenum != nodelimit ) throw string("terminating before nodenum == nodelimit\n"); 
      if (rpos != rend) throw string("terminating before rpos == rend\n"); 
    } catch (string err) {
      throw string("Error in DataSet::CreateFullNodes while reading node ") + doubleToString(nodenum) +":\n" + err; 
    }
    dbprintf(2, "CreateFullNodes ended...\n"); 
    return; 
  }

  
  //===========================================================================
  void DataSet::WrapBoundarySegments(void) {
    ArmSegment *newSegment = NULL; 
    FullNode *newnode0, *newnode1; 
    ArmSegmentSet::const_iterator segpos = mQuickFindArmSegments.begin(), 
      endseg = mQuickFindArmSegments.end(); 
    while (segpos != endseg) {      
      if ((*segpos)->Wrap(mDataSize, newSegment, newnode0, newnode1)) {
        newSegment->SetID(); 
        mWrappedArmSegments.push_back(newSegment); 
        newnode0->SetIndex(mFullNodes.size()); 
        mFullNodes.push_back(newnode0); 
        newnode1->SetIndex(mFullNodes.size()); 
        mFullNodes.push_back(newnode1);
        dbprintf(5, "\n***********\nWrapBoundarySegments: Created new wrapped node %s", newnode0->GetNodeID().Stringify().c_str()); 
        dbprintf(5, "\n***********\nWrapBoundarySegments: Created new wrapped node %s", newnode1->GetNodeID().Stringify().c_str()); 
        dbprintf(5, "\n***********\nWrapBoundarySegments: Created new wrapped segment %s", newSegment->Stringify().c_str()); 
        
      } 
      ++segpos; 
    }
    return; 
  }

  //===========================================================================
  void DataSet::DebugPrintFullNodes(char *name) {
    uint32_t monsterTypes[5] = {0}; /* types: -44, -4, -3, -33, sum of monsters */ 
    
    string basename="FullNodes-list.txt";
    if (name)  basename = name; 
    std::string filename = mDebugOutputDir + "/" + basename; 
    dbprintf(1, "Writing full nodes to debug file %s\n", filename.c_str()); 

    if (!Mkdir (mDebugOutputDir.c_str())) return;

    ofstream debugfile (filename.c_str()); 

    std::vector<FullNode*>::iterator nodepos = mFullNodes.begin(), 
      endnodepos = mFullNodes.end(); 
    uint32_t nodenum=0; 
    while (nodepos != endnodepos) {
      debugfile << "FullNode " << nodenum++ <<":\n" << (*nodepos)->Stringify(true ) << endl; 
      debugfile << "**************************************************************" << endl << endl; 
      int nodetype = (*nodepos)->GetNodeType(); 
      if (nodetype == -44) monsterTypes[0]++; 
      else if (nodetype == -4) monsterTypes[1]++;
      else if (nodetype == -3) {
        debugfile << "Butterfly Node: " << (*nodepos)->GetNodeID().Stringify() << endl;
        monsterTypes[2]++;
      }
      else if (nodetype == -33) monsterTypes[3]++;
      if (nodetype < 0) monsterTypes[4]++; 
      
      ++nodepos; 
    }
    debugfile <<"Total full nodes: " << nodenum << endl; 
    debugfile << monsterTypes[0] << " type -44 nodes" << endl; 
    debugfile << monsterTypes[1] << " type -4 nodes" << endl; 
    debugfile << monsterTypes[2] << " type -3 nodes" << endl; 
    debugfile << monsterTypes[3] << " type -33 nodes" << endl; 
    debugfile << monsterTypes[4] << " monster nodes" << endl; 
    debugfile <<"Total memory for nodes and their pointers: " << mFullNodes.size() * (sizeof(FullNode) + sizeof(FullNode *)) << endl; 
    return;  
  }
  
  //===========================================================================
  void DataSet::FindEndOfArm(FullNodeIterator &iStartNode, FullNode **oEndNode, const ArmSegment *iStartSegment, const ArmSegment *&oEndSegment
#ifdef DEBUG
, Arm &theArm
#endif
) {
    //FullNodeIterator currentNode = iStartNode, otherEnd; 
    FullNode *currentNode = *iStartNode, *otherEnd; 
    const ArmSegment *currentSegment = iStartSegment; 
    /* loop, don't recurse */ 
    while(true) {
#ifdef DEBUG
      if (!currentSegment->Seen()) {
        ++theArm.mNumSegments; 
        dbprintf(5, "Arm %d: adding segment %s\n", theArm.mArmID, currentSegment->Stringify().c_str());
      }
#endif
      const_cast<ArmSegment *>(currentSegment)->SetSeen(true);  
      
      otherEnd = currentSegment->GetOtherEndpoint(currentNode);
      if ((otherEnd)->GetNodeID() == (*iStartNode)->GetNodeID() ||
          (otherEnd)->GetNumNeighbors() != 2) {
        if ((otherEnd)->GetNumNeighbors() == 2) {
          dbprintf(4, "Arm %d: LOOP detected\n", theArm.mArmID);
        }
        /*!
          we have looped or found a terminal node -- stop 
        */ 
        oEndSegment = currentSegment; 
        *oEndNode = otherEnd; 
        return;
      }
           
      /*! 
        Move on to the next segment, we are not done yet.
      */ 
      if (*currentSegment == *(otherEnd)->GetNeighbor(0)) {
        currentSegment = dynamic_cast<const ArmSegment*>((otherEnd)->GetNeighbor(1)); 
      } else {
        currentSegment = dynamic_cast<const ArmSegment*>((otherEnd)->GetNeighbor(0)); 
      }     
      currentNode = otherEnd; 
    }
    return;     
  }

  //===========================================================================
  void DataSet::BuildArms(void) {
    dbprintf(2, "BuildArms started.\n"); 
    Arm theArm; // reuse to avoid calling umpteen constructors 
    uint32_t armnum = 0; 
    /*! 
      For now, just look at every inbounds node and if it has not been looked at, make an arm out of it.  
    */ 
    vector<FullNode*>::iterator nodepos = mFullNodes.begin(), nodeend = mFullNodes.end(); 
    int nodenum = 0; 
    /*!
      If you start from an out of bounds node, you will often trace out arms that have no nodes in them!  That would be segfault fodder. 
    */ 
    try {
      while (nodepos != nodeend) {
        if (!(*nodepos)->InBounds()) {
          ++nodepos; 
          continue;
        }
        int neighbornum = 0, numneighbors = (*nodepos)->GetNumNeighbors();       
        
        FullNode *endNode0, *endNode1; 
        const ArmSegment *endSegment0 = NULL, *endSegment1 = NULL, 
          *startSegment0 = NULL, *startSegment1 = NULL; 
        if (numneighbors == 2) {
          startSegment0 = dynamic_cast<const ArmSegment*>((*nodepos)->GetNeighbor(0));
          if (!startSegment0->Seen()) {
#ifdef DEBUG
            dbprintf(5, "Starting in middle of arm\n"); 
#endif
            theArm.Clear();
            theArm.SetID(); 
            FindEndOfArm(nodepos, &endNode0, startSegment0, endSegment0 
#ifdef DEBUG
                         , theArm
#endif
                         ); 
            theArm.mTerminalNodes.push_back(endNode0); 
            theArm.mTerminalSegments.push_back(endSegment0); 
            
            startSegment1 = dynamic_cast<const ArmSegment*>((*nodepos)->GetNeighbor(1));
            FindEndOfArm(nodepos, &endNode1,startSegment1, endSegment1
#ifdef DEBUG
                         , theArm
#endif
                         ); 
            
            if (endNode0 != endNode1) {
              theArm.mTerminalNodes.push_back(endNode1); 
            }
            if (endSegment0 != endSegment1) {
              theArm.mTerminalSegments.push_back(endSegment1); 
            }
            dbprintf(5, "(1) Pushing back arm %d: %s\n", armnum++, theArm.Stringify().c_str()); 
            mArms.push_back(theArm);
          }
        } else {
          while (neighbornum < numneighbors) {
            startSegment0 = 
              dynamic_cast<const ArmSegment*>((*nodepos)->GetNeighbor(neighbornum));
            if (!startSegment0->Seen()) {
#ifdef DEBUG
              dbprintf(5, "Starting at one end of arm\n"); 
#endif
              theArm.Clear();
              theArm.SetID(); 
              FindEndOfArm(nodepos, &endNode0, startSegment0, endSegment0
#ifdef DEBUG
                           , theArm
#endif
                           ); 
              theArm.mTerminalNodes.push_back(*nodepos); 
              if (endNode0 != *nodepos ) {
                theArm.mTerminalNodes.push_back(endNode0); 
              }
              theArm.mTerminalSegments.push_back(startSegment0); 
              if (endSegment0 != startSegment0) {
                theArm.mTerminalSegments.push_back(endSegment0); 
              }
              dbprintf(5, "(2) Pushing back arm %d: %s\n", armnum++, theArm.Stringify().c_str()); 
              mArms.push_back(theArm);          
            }
            ++neighbornum; 
          }
        }
        ++nodepos; 
        ++nodenum; 
        
      }   
    } catch (string err) {
      throw string("Arm #")+doubleToString(armnum)+": "+err;
    }
    dbprintf(2, "BuildArms ended; %d arms created.\n", armnum); 
    return; 
  }

  //===========================================================================
  void DataSet::DebugPrintArms(void) {
    std::string filename = mDebugOutputDir + string("/Arms-list.txt"); 
    dbprintf(1, "Writing arms to debug file %s\n", filename.c_str()); 
    if (!Mkdir (mDebugOutputDir.c_str())) return; 

    ofstream debugfile (filename.c_str()); 
    
    debugfile <<"Printout of all arms." << endl; 
    vector<Arm>::iterator pos = mArms.begin(), endpos = mArms.end(); 
    uint32_t armnum = 0; 
    while (pos != endpos) {
      debugfile << "Arm #" << armnum << ": " << pos->Stringify() << endl; 
      //vector<ArmSegment *>segments = pos->GetArmSegments(); 
      //vector<FullNode *>nodes = pos->GetNodes(); 
      debugfile << "******************************************************" << endl << endl; 
      ++armnum; 
      ++pos; 
    }
    debugfile << "Number of arms: " << mArms.size()<< endl; 
    debugfile << "Total memory used by arms: " << mArms.size() * sizeof(Arm) << endl; 
    debugfile << "Number of arm segments: " << mFinalArmSegments.size() << endl; 
    debugfile << "Memory used by arm segments and their pointers: " << mFinalArmSegments.size() * (sizeof(ArmSegment) + sizeof(ArmSegmentSetElement)) << endl; 
    return; 
  }

  
  //===========================================================================
  void DataSet::FindButterflies(void) {
    dbprintf(2, "FindButterflies starting...\n");
    int armnum=0; 
    vector<Arm>::iterator armpos = mArms.begin(), armend = mArms.end(); 
    while (armpos != armend) {

      armpos->CheckForButterfly();      
      ++armnum; 
      ++armpos; 
    }
    dbprintf(2, "FindButterflies ended.\n");
    return; 
  }

  //===========================================================================
  void DataSet::ClassifyArms(void) {
    dbprintf(2, "ClassifyArms starting...\n");
    int armnum=0; 
    vector<Arm>::iterator armpos = mArms.begin(), armend = mArms.end(); 
    while (armpos != armend) {      
      armpos->Classify(); 
      ++armnum; 
      ++armpos; 
    }
    dbprintf(2, "ClassifyArms ended.\n");
    return; 
  }
  
  //==========================================================================
  /*!
    Unary predicate for using STL to remove all useless nodes in a range
  */ 
   bool NodeIsUseless(FullNode *node) {
    return node->GetNodeType() == USELESS_NODE; 
  }

  //==========================================================================
  /*!
    This is a unary predicate used to efficiently delete all useless arms, that have at least one useless terminal nodes.   
  */ 
  bool ArmIsUseless( Arm &theArm) {
    return theArm.IsUseless(); 
  }

  //==========================================================================
  /*! 
    To be used with for_each() to delete all nodes in the container
  */ 
  void DeleteNode(FullNode *node) {
    delete node; 
  } 

  //===========================================================================
  void DataSet::DeleteUselessNodesAndSegments(void) {
    dbprintf(2, "DeleteUselessNodesAndSegments starting...\n");
    int numdeleted = 0; 
    /*!
      First identify every node that is out of bounds and has no inbound neighbor -- these are useless nodes.  
    */ 
    dbprintf(2, "Identifying useless nodes...\n "); 
    bool deletable = false; 
    vector<FullNode *>::iterator nodepos = mFullNodes.begin(), 
      nodeend = mFullNodes.end(); 
    while (nodepos != nodeend) {      
      if ((*nodepos)->GetNodeType() != USELESS_NODE && 
          !(*nodepos)->InBounds()) {

        deletable = true; 
        int nbr = (*nodepos)->GetNumNeighbors(); 
        while (nbr--) {
          if ((*nodepos)->GetNeighbor(nbr)->GetOtherEndpoint((*nodepos))->InBounds()) {
            deletable=false; 
            break; 
          }
        }
        if (deletable) (*nodepos)->SetNodeType(USELESS_NODE); 
        
      }
      ++nodepos; 
    }   

    /*!
      Next find all arms that have a useless node as either endpoint.  Delete them.  
    */ 
    dbprintf(2, "Identifying and deleting useless arms...\n"); 
    numdeleted = mArms.size(); 
    vector<Arm>::iterator armpos = mArms.begin(), armend = mArms.end(); 
    armpos = remove_if(armpos, armend, ArmIsUseless); 
    mArms.erase(armpos, mArms.end()); 
    numdeleted -= mArms.size(); 
    dbprintf(2, "Deleted %d arms.\n", numdeleted); 

    /*!
      Useless arm segments are arm segments that have a useless node as either endpoint, or two out of bounds endpoints.  Delete those while copying non-useless arm segments into the final vector.
    */ 
    dbprintf(2, "Identifying and deleting useless segments...\n"); 
    numdeleted = mQuickFindArmSegments.size() + mWrappedArmSegments.size(); 

    ArmSegmentSet::const_iterator segpos = mQuickFindArmSegments.begin();
    ArmSegment *theSegment = NULL; 
    while (segpos != mQuickFindArmSegments.end()) {
      theSegment = *segpos; 
      if ((*segpos)->IsUseless()) {
        dbprintf(5, "Deleting segment %d\n", theSegment->GetID()); 
        delete theSegment; 
      } else {
        dbprintf(5, "Keeping segment %d\n", theSegment->GetID()); 
        mFinalArmSegments.push_back(theSegment);         
      }
      ++segpos;   
    }
    mQuickFindArmSegments.clear(); 

    vector<ArmSegment*>::iterator wrappos = mWrappedArmSegments.begin(); 
    while (wrappos != mWrappedArmSegments.end()) {
      if ((*wrappos)->IsUseless()) {
        delete *wrappos; 
      } else { 
        mFinalArmSegments.push_back(*wrappos); 
      }
      ++wrappos;
    }
    mWrappedArmSegments.clear(); 

    numdeleted -= mFinalArmSegments.size(); 
    dbprintf(2, "Deleted %d segments\n", numdeleted); 

    /*! 
      Finally, go through and delete all the useless nodes
    */ 
    dbprintf(2, "Deleting useless nodes...\n"); 
    numdeleted = mFullNodes.size(); 
    nodepos = mFullNodes.begin(); 
    while (nodepos != mFullNodes.end()) {
      int nodetype = (*nodepos)->GetNodeType();
      if (nodetype == USELESS_NODE) {
        mUselessNodes.push_back(*nodepos); 
      } else {
        if (nodetype > 8) {
          string err = string("Error: bad type ") + doubleToString( nodetype) +  " in node: " + (*nodepos)->Stringify();
          throw err;          
        }
      }
      ++nodepos; 
    }
    nodepos = remove_if(mFullNodes.begin(), mFullNodes.end(), NodeIsUseless); 
    mFullNodes.erase(nodepos, mFullNodes.end()); 
    for_each(mUselessNodes.begin(), mUselessNodes.end(), DeleteNode);     
    mUselessNodes.clear(); 

    numdeleted -= mFullNodes.size(); 
    dbprintf(2, "Deleted %d nodes\n", numdeleted); 

    dbprintf(2, "DeleteUselessNodesAndSegments ended.\n");
    return; 
  }
  //===========================================================================
  void DataSet::ComputeSubspace(void){
    dbprintf(2, "starting ComputeSubspace\n"); 
    mSubspaceMax = mDataMax;  
    mSubspaceMin = mDataMin; 
    rclib::Point<float> chunksize = mDataSize; 
    if (mNumProcs > 1) {
      // 2) return; 
      
      /*!
        Break space up into chunks
      */ 
      int numsplits = 0; 
      int numpieces = 1; 
      int current_dimension = 0; // X, Y or Z 
      int  numchunks[3] = {1, 1, 1}; 
      while (numpieces < mNumProcs) {
        ++numsplits; 
        numpieces *= 2; 
        chunksize[current_dimension] /= 2.0; 
        numchunks[current_dimension] ++; 
        current_dimension = (current_dimension + 1)%3; 
      }
      

      /*!
        Now exactly locate our chunk bounds
      */ 
      int leftover_pieces = numpieces % mNumProcs;  
      int currentproc = 0; 
      rclib::Point<float> currentMin = mSubspaceMin; 
      int i,j,k;
      bool keepGoing = true; 
      for (k = 0, currentMin[0] = mSubspaceMin[0];
           k < numchunks[2] && keepGoing; 
           k++, currentMin[2] += chunksize[2]){        
        for (j = 0, currentMin[1] = mSubspaceMin[1]; 
             j < numchunks[1] && keepGoing; 
             j++, currentMin[1] += chunksize[1]){          
          for (i = 0, currentMin[0] = mSubspaceMin[0]; 
               i < numchunks[0] && keepGoing;
               i++,  currentMin[0] += chunksize[0]){
            if (currentproc < mProcNum) {
              if (currentproc < leftover_pieces) {
                /*!
                  The leftover pieces get given to the first procs
                */ 
                currentMin[0] += chunksize[0]; 
                i++; // it's ok, i is always a multiple of two
              }
            } else {
              mSubspaceMin = currentMin; 
              mSubspaceMax = mSubspaceMin + chunksize; 
              if (currentproc < leftover_pieces) {
                mSubspaceMax[0] += chunksize[0]; 
              }           
              keepGoing = false; 
            }
            ++currentproc; 
          }
        }
      }
              
      dbprintf(2, "done with ComputeSubspace\n"); 
    }

    dbprintf(2, "Computed subspace min = %s and max = %s\n", mSubspaceMin.Stringify().c_str(), mSubspaceMax.Stringify().c_str()); 
    
    return; 
  }
  //=========================================================================
  void DataSet::RenumberNodes(void) {
    uint32_t nodenum = 0, numnodes = mFullNodes.size(); 
    while (nodenum != numnodes) {
      mFullNodes[nodenum]->SetIndex(nodenum); 
      ++nodenum;
    }
  }

  //===========================================================================
  void DataSet::ReadData(std::string filename){
    dbprintf(1, "ReadData called with debug level %d\n", dbg_isverbose()); 
    try { 
      if (filename != "") {
        mDataFilename = filename; 
      }
      dbprintf(2, "mDataFilename is %s\n",  mDataFilename.c_str());
      if (mDataFilename == "") throw string("Empty filename in ReadData"); 

      ReadBounds();      
      
      if (mSubspaceMin == mSubspaceMax && mSubspaceMax == rclib::Point<float>(0.0)) {
        /*! 
          if user hasn't set min and max, then figure out our spatial bounds.  
        */ 
        ComputeSubspace(); 
      }

#ifdef DEBUG_SUBSPACES
      /*!
        Testing code generates bounds for lots of processor configs and prints them out to make sure that subspaces are being correctly computed.  
      */ 
      ReadBounds(); 
      mNumProcs = 8; 
      while (mNumProcs) {
        cout << endl << "*************************************************" << endl;
        cout << "mNumProcs = " << mNumProcs << endl; 
        mProcNum = mNumProcs; 
        while (mProcNum--) {
          cout << "******************" << endl; 
          cout << "mProcNum = " << mProcNum << endl; 
          ComputeSubspace(); 
          cout << "mSubspaceMin: " << mSubspaceMin.Stringify() << endl; 
          cout << "mSubspaceMax: " << mSubspaceMax.Stringify() << endl; 
        }
        mNumProcs --; 
      }
#endif

      /*!
        Announce to the nodes what their bounds are for wrapping
      */ 
      FullNode::SetBounds(mSubspaceMin, mSubspaceMax); 
      
      CreateMinimalNodes(); 

      ClassifyMinimalNodes(); 

      if (mDoDebugOutput) {
        DebugPrintMinimalNodes(); 
      }

      CullAndResortMinimalNodes(); 

      CreateFullNodesAndArmSegments(); 
      
      BuildArms(); 

      FindButterflies(); 
      
      ClassifyArms(); 

      //PrintArmStats(); 

#ifdef DEBUG
      if (mDoDebugOutput) {
        DebugPrintFullNodes("NodesBeforeDeletion");       
      }
#endif
      /* this used to go before BuildArms() */ 
      WrapBoundarySegments();  
      
      DeleteUselessNodesAndSegments(); 

      RenumberNodes(); 
      
      if (mDoDebugOutput) {
        DebugPrintArms(); 
      }      

      if (mDoDebugOutput) {
        DebugPrintFullNodes(); 
      }
      
    } catch (string err) {
      cerr << "An exception occurred" << endl; 
      throw string("Error in DataSet::ReadData reading data from file ")+mDataFilename+":\n" + err; 
    }
    dbprintf(1, "ReadData complete\n"); 
    return; 
  } 
  
  
  
  
} // end namespace paraDIS 


