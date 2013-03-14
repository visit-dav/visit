/*!
  \file paradis.C
 a new run at libparadis.C from the ground up 
   Need to do this because the previous design was intentionally entwined with povray and HTS operations for efficiency, and assumed the whole dataset would be read at once. 
 */ 
#include "paradis.h"
#include <cstring>
#include <fstream>
#include <string>
#include <iostream> 
#include "debugutil.h"
#include "stringutil.h"
#include "timer.h"
#include <algorithm>
#ifndef WIN32
#  include <dirent.h>
#else
#  include <direct.h>
#endif
#include <sys/stat.h>
#include <errno.h>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

#ifdef RC_CPP_VISIT_BUILD
#define errexit return 
#define errexit1 return err
#else
#define errexit abort() // HOOKS_IGNORE
#define errexit1 abort() // HOOKS_IGNORE
#endif


std::string INDENT(int i) {
  if (!i) return "";
  return str(boost::format(str(boost::format("%%1$%1%s")%(i*3)))%" "); 
}
#define STARTPROGRESS()                                       \
  cerr << endl; timer theTimer;  theTimer.start();            \
  double theTime=theTimer.elapsed_time(), thePercent=0;   

#define UPDATEPROGRESS(count, total, description)                       \
  if (dbg_isverbose()) {                                                \
    Progress(theTimer, count, total, thePercent, 1, theTime, 60, description);  }

string BurgersTypeNames(int btype) {
  switch (btype) {
  case BURGERS_UNKNOWN  : return "BURGERS_UNKNOWN"; 
  case BURGERS_NONE   : return "BURGERS_NONE"; 
  case BURGERS_PPP     : return "BURGERS_PPP"; 
  case BURGERS_PPM    : return "BURGERS_PPM"; 
  case BURGERS_PMP   : return "BURGERS_PMP"; 
  case BURGERS_PMM   : return "BURGERS_PMM"; 
  case BURGERS_200     : return "BURGERS_200"; 
  case BURGERS_020      : return "BURGERS_020"; 
  case BURGERS_002    : return "BURGERS_002"; 
  case BURGERS_220     : return "BURGERS_220"; 
  case BURGERS_202      : return "BURGERS_202"; 
  case BURGERS_022      : return "BURGERS_022"; 
  case BURGERS_311      : return "BURGERS_311"; 
  case BURGERS_131     : return "BURGERS_131"; 
  case BURGERS_113    : return "BURGERS_113"; 
  case BURGERS_222    : return "BURGERS_222"; 
  case BURGERS_004     : return "BURGERS_004"; 
  default: return "UNKNOWN CODE"; 
  }
}

string ArmTypeNames(int atype) {
  switch (atype) {
  case ARM_EMPTY         : return "ARM_EMPTY"; 
  case ARM_UNKNOWN        : return "ARM_UNKNOWN"; 
  case ARM_UNINTERESTING  : return "ARM_UNINTERESTING"; 
  case ARM_LOOP           : return "ARM_LOOP"; 
  case ARM_MM_111          : return "ARM_MM_111"; 
  case ARM_MN_111         : return "ARM_MN_111"; 
  case ARM_NN_111          : return "ARM_NN_111"; 
  case ARM_MM_200         : return "ARM_MM_200"; 
  case ARM_MN_200         : return "ARM_MN_200"; 
  case ARM_NN_200         : return "ARM_NN_200"; 
  case ARM_SHORT_NN_111   : return "ARM_SHORT_NN_111"; 
  case ARM_SHORT_NN_200   : return "ARM_SHORT_NN_200"; 
  default: return "UNKNOWN ARMTYPE"; 
  }
}

string MetaArmTypeNames(int mtype) {
  switch (mtype) {
  case METAARM_UNKNOWN     : return "METAARM_UNKNOWN"; 
  case METAARM_111         : return "METAARM_111"; 
  case METAARM_LOOP_111    : return "METAARM_LOOP_111"; 
  case METAARM_LOOP_200    : return "METAARM_LOOP_200"; 
  default                  : return "METAARM ERROR"; 
  }
}; 

int BurgersCategory(float burgval) {
  int code=burgval/0.577350; 
  if (code < -1 ) code *= -1; 
  if (abs(code) > 4) {
    dbprintf(1, "\n\n********************************\n");
    dbprintf(1, "WARNING: Weird value %g encountered in Category\n", burgval);
    dbprintf(1, "\n********************************\n\n");
  }
  
  return code; 
}

int InterpretBurgersType(float burg[3]) {
  int burgersType = BURGERS_UNKNOWN;
  
  int catarray[3] = 
    {BurgersCategory(burg[0]), BurgersCategory(burg[1]), BurgersCategory(burg[2])};
  if (abs(catarray[0]) == 2 && catarray[1] == 0 && catarray[2] == 0)
    burgersType = BURGERS_200;
  else if (catarray[0] == 0 && abs(catarray[1]) == 2 && catarray[2] == 0)
    burgersType = BURGERS_020;
  else if (catarray[0] == 0 && catarray[1] == 0 && abs(catarray[2]) == 2)
    burgersType = BURGERS_002;
  else if ((catarray[0] == 1 && catarray[1] == 1 && catarray[2] == 1) ||
           (catarray[0] == -1 && catarray[1] == -1 && catarray[2] == -1))
    burgersType = BURGERS_PPP;
  else if ((catarray[0] == 1 && catarray[1] == 1 && catarray[2] == -1) ||
           (catarray[0] == -1 && catarray[1] == -1 && catarray[2] == 1))
    burgersType = BURGERS_PPM;
  else if ((catarray[0] == 1 && catarray[1] == -1 && catarray[2] == 1) ||
           (catarray[0] == -1 && catarray[1] == 1 && catarray[2] == -1))
    burgersType = BURGERS_PMP;
  else if ((catarray[0] == 1 && catarray[1] == -1 && catarray[2] == -1) ||
           (catarray[0] == -1 && catarray[1] == 1 && catarray[2] == 1))
    burgersType = BURGERS_PMM;
  else if (abs(catarray[0]) == 2 && abs(catarray[1]) == 2 && catarray[2] == 0)
    burgersType = BURGERS_220;
  else if (abs(catarray[0]) == 2 && catarray[1] == 0 && abs(catarray[2]) == 2)
    burgersType = BURGERS_202;
  else if (catarray[0] == 0 && abs(catarray[1]) == 2 && abs(catarray[2]) == 2)
    burgersType = BURGERS_022;
  else if (abs(catarray[0]) == 2 && abs(catarray[1]) == 2 && abs(catarray[2]) == 2)
    burgersType = BURGERS_222;
  else if (catarray[0] == 3 && abs(catarray[1]) == 1 && abs(catarray[2]) == 1) 
    burgersType = BURGERS_311;
  else if (abs(catarray[0]) == 1 && catarray[1] == 3 && abs(catarray[2]) == 1)
    burgersType = BURGERS_131;
  else if (abs(catarray[0]) == 1 && abs(catarray[1]) == 1 && catarray[2] == 3)
    burgersType = BURGERS_113;
  else if ((abs(catarray[0]) == 0 && abs(catarray[1]) == 0 && catarray[2] == 4) ||
           (abs(catarray[0]) == 0 && catarray[1] == 4 && abs(catarray[2]) == 0) ||
           (catarray[0] == 4 && abs(catarray[1]) == 0 && abs(catarray[2]) == 0) )  
    burgersType = BURGERS_004;
  else {
    burgersType = BURGERS_UNKNOWN;
    dbprintf(3, "\n\n********************************\n");
    dbprintf(3, "Warning: segment has unknown type: burgers = (%f, %f, %f), categories=(%d, %d, %d)\n", burg[0], burg[1], burg[2], catarray[0], catarray[1], catarray[2]); 
    dbprintf(3, "\n********************************\n\n");
  }
  return burgersType; 
}

string doctext = "ParaDIS data is a set of disconnected undirected graphs, which may contain cycles.  \n"
"All nodes in a paraDIS data set have at least one neighbor given in the paraDIS output file.  The connection, or neighbor relation, between two adjacent nodes is called an \"arm segment.\"  A node is considered an \"interior node\" if it has exactly two neighbors, else it is considered a \"terminal node.\"  The only exception to this is that loops always have one terminal node, which may have two neighbors.  \n"
"An \"arm\" is a sequence of interior nodes terminated by one or two terminal nodes.  A loop is an arm that has only one terminal node.  \n"
"Every node is part of at least one arm.  If it's not an interior node, we say the node \"has arms.\"  The number of arms equals the number of neighbors, even if some are loops.  E.g., a terminal node with three neighbors has three arms by definition, even if two of the neighbors are part of the same loop, i.e., two of the arms might be the same arm.\n"
"\n"
"Segments and arms have Burgers vectors associated with them.  \n"
"  //  Segment BURGERS TYPES: (P = plus(+) and M = minus(-))\n"
"// These are valued in order of increasing energy levels, corresponding to the sum of the square of the components of the burgers vector.  \n"
"#define BURGERS_DECOMPOSED  -2  // for segments that are decomposed\n"
"#define BURGERS_UNKNOWN     -1  // analysis failed\n"
"#define BURGERS_NONE        0   // no analysis done yet\n"
"#define BURGERS_PPP         10  // +++  BEGIN ENERY LEVEL 1\n"
"#define BURGERS_PPM         11  // ++-\n"
"#define BURGERS_PMP         12  // +-+\n"
"#define BURGERS_PMM         13  // +--\n"
"#define BURGERS_200         20  // BEGIN ENERGY LEVEL 2\n"
"#define BURGERS_020         21  \n"
"#define BURGERS_002         22\n"
"#define BURGERS_220         30  // BEGIN ENERGY LEVEL 3\n"
"#define BURGERS_202         31\n"
"#define BURGERS_022         32\n"
"#define BURGERS_311         40  // BEGIN ENERGY LEVEL 4\n"
"#define BURGERS_131         41\n"
"#define BURGERS_113         42\n"
"#define BURGERS_222         50  // BEGIN ENERGY LEVEL 5\n"
"#define BURGERS_004         60  // BEGIN ENERGY LEVEL 6\n"
"NODE TYPES and MONSTER NODES: \n"
"Every node has a node type.  The vast majority of nodes are simply have NodeType = number of neighboring nodes.  But some nodes get negative types, and these are known as Monster Nodes.  Oooh, scary.  \n"
"Only a terminal node may be a \"monster node\" A.K.A. \"M Type node.\" If a node is not an \"M\" then it is an \"N\" for \"non-monster\" or \"normal.\"  Interior nodes are always type \"N\" if anyone cares to ask, but this is usually ignored.  \n" 
"A monster is any node where all four basic 111 type arms are neighbors of the node.  Thus, monster nodes always have 4 or more arms. \n" 
"\n" 
"\n"
"\n"
"Arms also have \"arm type,\" which mostly indicates whether its endpoints are monsters or not or whether they are looped arms.  \n"
"  // Arm MN types:\n"
"#define ARM_EMPTY        -1 //marked for deletion after decomposition step\n"
"#define ARM_UNKNOWN       0 \n"
"#define ARM_UNINTERESTING 1\n"
"#define ARM_LOOP          2\n"
"#define ARM_MM_111        3 \n"
"#define ARM_MN_111        4\n"
"#define ARM_NN_111        5 \n"
"#define ARM_MM_200        6\n"
"#define ARM_MN_200        7\n"
"#define ARM_NN_200        8\n"
"#define ARM_SHORT_NN_111  9\n"
"#define ARM_SHORT_NN_200  10\n"
"\n"
"MetaArms are collections of arms.  There are three main types, plus an \"unknonwn\" category for misfit arms:\n"
"#define METAARM_UNKNOWN     0  // Not defined, error, or some other odd state\n"
"#define METAARM_111         1  // Entirely composed of type 111 arms of the same burgers vector.   Does not include loops. \n"
"#define METAARM_LOOP_111    2  // Contains a loop, composed entirely of type 111 arms.\n"
"#define METAARM_LOOP_200    3  // Contains a loop, composed entirely of type 200 arms.\n"
;
 
using namespace RC_Math; 
using namespace std; 
namespace paraDIS {
  
  //===========================================================================
#ifdef DEBUG
  int32_t ArmSegment::mNextID = 0; 
  int32_t Arm::mNextID = 0;

  rclib::Point<float> FullNode::mBoundsMin, FullNode::mBoundsMax, FullNode::mBoundsSize; 
#endif
  double Arm::mLongestLength = 0.0; 
  double gSegLen = 0 ;
  uint32_t gNumClassified = 0, gNumWrapped = 0, gNumArmSegmentsMeasured=0; 

  double Arm::mThreshold = -1; 
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
                  " index " + intToString(mNodeIndex) + 
#endif
                  " type " + (intToString(mNodeType) + string("\n")) + 
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
        s += "neighbor " + intToString(i) + ": "; 
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
          if (btypes[btype] || btype == BURGERS_UNKNOWN) {
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
    mBurgersType = BURGERS_UNKNOWN;

    int valarray[3] = 
      {Category(burg[0]), Category(burg[1]), Category(burg[2])};
    if (valarray[0] == 1 && valarray[1] == 0 && valarray[2] == 0)
      mBurgersType = BURGERS_100;
    else if (valarray[0] == 0 && valarray[1] == 1 && valarray[2] == 0)
      mBurgersType = BURGERS_010;
    else if (valarray[0] == 0 && valarray[1] == 0 && valarray[2] == 1)
      mBurgersType = BURGERS_001;
    else if ((valarray[0] == 2 && valarray[1] == 2 && valarray[2] == 2) ||
             (valarray[0] == 3 && valarray[1] == 3 && valarray[2] == 3))
      mBurgersType = BURGERS_PPP;
    else if ((valarray[0] == 2 && valarray[1] == 2 && valarray[2] == 3) ||
             (valarray[0] == 3 && valarray[1] == 3 && valarray[2] == 2))
      mBurgersType = BURGERS_PPM;
    else if ((valarray[0] == 2 && valarray[1] == 3 && valarray[2] == 2) ||
             (valarray[0] == 3 && valarray[1] == 2 && valarray[2] == 3))
      mBurgersType = BURGERS_PMP;
    else if ((valarray[0] == 2 && valarray[1] == 3 && valarray[2] == 3) ||
             (valarray[0] == 3 && valarray[1] == 2 && valarray[2] == 2))
      mBurgersType = BURGERS_PMM;
    else {
      mBurgersType = BURGERS_UNKNOWN;
      dbprintf(3, "\n\n********************************\n");
      dbprintf(3, "Warning: segment has odd value/category: value = %d, burg = (%f, %f, %f), valarray=(%d, %d, %d)\n", mBurgersType, burg[0], burg[1], burg[2], valarray[0], valarray[1], valarray[2]); 
      dbprintf(3, "\n********************************\n\n");
    }
    return; 
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
      if (dataSize[i]/2.0 < dist) {
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

#if LINKED_LOOPS
  //===========================================================================
  void Arm::CheckForLinkedLoops(void) {
    int err = 0;
    if (mCheckedForLinkedLoop) return; 
    mCheckedForLinkedLoop = true; 
    
    /*!
      Iff we have one terminal node with four arms and one neighbor, or two terminal nodes with three arms and num neighbors == 2, then we are part of a linked loop.   
      Note that it's not true that if you have two terminal nodes with N arms and number of arms on each terminal node == N-1 that it's necessarily a linked loop, although it certainly is an interesting beast.  
    */     
    
    bool notPartOfLoop = false; //set for early termination
    int numTerminalNodeArms = 0; // all terminal nodes must have the same number of arms. 
    // Count the number of distinct neighboring arms to this one
    vector<Arm*> myNeighbors; 
    vector<FullNode *>::iterator termNode = mTerminalNodes.begin(), endNode = mTerminalNodes.end();  
    for (; !notPartOfLoop && termNode != endNode; termNode++) {
      int neighborNum = (*termNode)->GetNumNeighbors(); 
      if (!numTerminalNodeArms) {
        numTerminalNodeArms = neighborNum; 
      } else if (neighborNum != numTerminalNodeArms) {
        notPartOfLoop = true; 
        break; 
      }
      
      while (neighborNum --) {
        Arm *neighbor = (*termNode)->GetNeighbor(neighborNum)->mParentArm; 
        bool knownNeighbor = (neighbor == this); 
        if (!knownNeighbor && 
            neighbor->mCheckedForLinkedLoop) {
   
          /*!
            Since this neighbor has been checked, we cannot be 
            part of a linked loop, or we would already be marked as such.
            Just a quick reality check to confirm it and we're done.
          */
  
    if (neighbor->mPartOfLinkedLoop) {
            dbprintf(0, "Impossible -- neighbor is part of linked loop but we are not!\n"); 
            errexit; 
          }
          notPartOfLoop = true; 
          break; 
        }
        int myNeighborNum = myNeighbors.size(); 
        while (!knownNeighbor && myNeighborNum--) {
          if (myNeighbors[myNeighborNum] == neighbor) {
            // duplicates are ok, and for linked loops, actually expected
            knownNeighbor = true; 
          }
        }
        if (!knownNeighbor) { 
          // Aha!  We have a new neighbor of us
          myNeighbors.push_back(neighbor);         
        }// found a new neighbor
      }// looking at all neighbors of terminal node
    } // looking at both terminal nodes
  
    /*! 
      Are we part of a linked loop? See definition above -- it's tricky.
    */ 
     
       if (!notPartOfLoop) {
      if ((myNeighbors.size() ==  2 && mTerminalNodes.size() == 2 && numTerminalNodeArms == 3) ||  
          (myNeighbors.size() == 1 && mTerminalNodes.size() == 1 && numTerminalNodeArms == 4)) {
        mPartOfLinkedLoop = true; 
      } 
    }
    
    /*!
      If we are not part of a linked loop, none of our neighbors are, 
      and if we are part of a linked loop, all of our neighbors are.
      So mark this arm and all neighbors of both terminal nodes appropriately.
    */ 
    int neighborNum = myNeighbors.size(); 
    while (neighborNum --) {
      myNeighbors[neighborNum]->mCheckedForLinkedLoop = true; 
      myNeighbors[neighborNum]->mPartOfLinkedLoop = mPartOfLinkedLoop; 
    }
    return; 
    
  }
#endif // LINKED_LOOPS
  
  //===========================================================================
  void Arm::ComputeLength(void) {
    mArmLength = 0; 
    // first figure out how to iterate.
    ArmSegment *startSegment = const_cast<ArmSegment*>(mTerminalSegments[0]);
    FullNode *startNode = NULL;  
    if (startSegment->GetEndpoint(0) == mTerminalNodes[0] ||
        startSegment->GetEndpoint(1) == mTerminalNodes[0]) {
      startNode = mTerminalNodes[0];
    } else if (mTerminalNodes.size() == 2 && 
               ( startSegment->GetEndpoint(0) == mTerminalNodes[1] ||
                 startSegment->GetEndpoint(1) == mTerminalNodes[1])) {
      startNode = mTerminalNodes[1];
    } else {
      throw string("Cannot find matching terminal node in arm for either segment endpoint"); 
    } 

    else {
      mArmLength = 0; 
      vector<ArmSegment*> segments = GetSegments(); 
      vector<ArmSegment*>::iterator segpos = segments.begin(), endseg = segments.end(); 
      
      while (segpos != endseg) {
        //dbprintf(5, "Adding length for %s\n", (*segpos)->Stringify(0).c_str()); 
        mArmLength += (*segpos)->GetLength(true); 
        ++segpos; 
      }
      if (mArmLength > mLongestLength) mLongestLength = mArmLength; 
      dbprintf(5, "ComputeLength(): arm %d has length %g\n", mArmID, mArmLength); 
    }

    return mArmLength; 
  }
  
  //===========================================================================
  vector<ArmSegment*>Arm::GetSegments(FullNode *startNode) {

    vector<ArmSegment*> err; 
    // Find the start segment that matches the given start node:
    ArmSegment *startSegment = NULL;
    vector<ArmSegment*> segments; 
    int segnum = -1;
    int nodenum = mTerminalNodes.size()-1; 
    if (startNode) nodenum = -1;
    do {
      segnum = mTerminalSegments.size(); 
      int segnum = mTerminalSegments.size(); 
      if (nodenum > -1) startNode = mTerminalNodes[nodenum];  
      while (!startSegment && segnum--) {
        if (mTerminalSegments[segnum]->GetEndpoint(0) == startNode ||
            mTerminalSegments[segnum]->GetEndpoint(1) == startNode) {
          startSegment = mTerminalSegments[segnum];
        }
      } 
      nodenum--; 
    } while (nodenum > -1 && !startSegment);

    if (!startSegment) {
      string s = string("GetSegments(): Cannot find matching terminal segment in arm for given start node"); 
      dbprintf(0, s.c_str()); 
      errexit1; 
    } 
    ArmSegment *lastSegment = NULL; 
    if (mTerminalSegments.size() == 1)  lastSegment = startSegment; 
    else lastSegment = const_cast<ArmSegment*>(mTerminalSegments[1]); 
    
    // now compute the length of the arm
    FullNode *currentNode = startNode; 
    ArmSegment *currentSegment = startSegment; 
    while (true) {
      dbprintf(5, "Adding length for %s\n", currentSegment->Stringify().c_str()); 
      mArmLength += currentSegment->GetLength(true); 
      if (currentSegment == lastSegment) {
        break; 
      }
      currentNode = currentSegment->GetOtherEndpoint(currentNode); 
      currentSegment = currentNode->GetOtherNeighbor(currentSegment);       
    }
    if (mArmLength > mLongestLength) mLongestLength = mArmLength; 
    return; 
  }
    
  
  //===========================================================================
  void Arm::Classify(void) {
    int err = -1; 
#if LINKED_LOOPS
    CheckForLinkedLoops(); 
#endif

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
      else if (btype != BURGERS_PPP && btype != BURGERS_PPM && 
               btype != BURGERS_PMP && btype != BURGERS_PMM) {
        dbprintf(0, "Error:  All arms should be type 111 now.\n"); 
        errexit; 
      }
    }
    
    /*!
      Now compute the length of the arm to enable "short arm" marking, then mark all segments with their MN type. 
    */ 
    // first figure out how to iterate.
    ArmSegment *startSegment = const_cast<ArmSegment*>(mTerminalSegments[0]);
    FullNode *startNode = NULL;  
    if (startSegment->GetEndpoint(0) == mTerminalNodes[0] ||
        startSegment->GetEndpoint(1) == mTerminalNodes[0]) {
      startNode = mTerminalNodes[0];
    } else if (mTerminalNodes.size() == 2 && 
               ( startSegment->GetEndpoint(0) == mTerminalNodes[1] ||
                 startSegment->GetEndpoint(1) == mTerminalNodes[1])) {
      startNode = mTerminalNodes[1];
    } else {
      throw string("Cannot find matching terminal node in arm for either segment endpoint"); 
    } 
    ArmSegment *lastSegment = NULL; 
    uint32_t numSeen = 0; 
    if (mTerminalSegments.size() == 1)  lastSegment = startSegment; 
    else lastSegment = const_cast<ArmSegment*>(mTerminalSegments[1]); 

    // now compute the length of the arm
    FullNode *currentNode = startNode; 
    ArmSegment *currentSegment = startSegment; 
    while (true) {
      dbprintf(5, "Adding length for %s\n", currentSegment->Stringify().c_str()); 
      mArmLength += currentSegment->GetLength(); 
      if (currentSegment == lastSegment) {
        break; 
      }
      currentNode = currentSegment->GetOtherEndpoint(currentNode); 
      currentSegment = currentNode->GetOtherNeighbor(currentSegment);       
    }
    
    /*!
      Now set every segment in this arm to the same type
    */ 
    currentNode = startNode; 
    currentSegment = startSegment; 
    if (mThreshold > 0 && mArmLength < mThreshold) {
      if (mArmType == ARM_NN_111) mArmType = ARM_SHORT_NN_111; 
      if (mArmType == ARM_NN_100) mArmType = ARM_SHORT_NN_100; 
    }
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
      string s = string("Error in Arm ")+intToString(mArmID)+":  classified "+intToString(numSeen)+" segments, but expected "+ intToString(mNumSegments); 
      dbprintf(0, s.c_str()); 
      errexit;
    }
#endif
    return; 
  }
  
  //===========================================================================
  void Arm::ExtendByArm(Arm *sourceArm, FullNode *sharedNode) {
    // identify the shared terminal node in the neighbor arm:     
    dbprintf(5, "\n======================================\n   ExtendByArm(): Extending arm: %s\n", Stringify(0, false).c_str()); 
    int sharedNodeNum = mTerminalNodes.size(); 
    while (sharedNodeNum-- ) {
      if (mTerminalNodes[sharedNodeNum] == sharedNode) break; 
    }
    if (sharedNodeNum == -1) {
      dbprintf(0, "ExtendByArm(): Error:  cannot find shared terminal node for extended arm!\n");
      errexit; 
    }    
    int sourceSharedNodeNum = sourceArm->mTerminalNodes.size(); 
    while (sourceSharedNodeNum-- ) {
        if (sourceArm->mTerminalNodes[sourceSharedNodeNum] == sharedNode) break; 
    }
    if (sourceSharedNodeNum == -1) {
      dbprintf(0, "ExtendByArm(): Error:  cannot find shared terminal node for source arm!\n");
      errexit; 
    }   

    bool isLoop = false;  
    ArmSegment * otherSharedSegment = NULL; 
    if (mTerminalNodes.size() == 1) {
      if (mTerminalSegments.size() == 1) {
        dbprintf(4, "ExtendByArm(): We are extending a looped arm with a single terminal segment.  So let's treat it as a non-looped arm and duplicate the shared node and other segment.\n"); 
        isLoop = true; 
        // we need to find our other terminal segment
        vector<ArmSegment *> segments = GetSegments(sharedNode); 
        otherSharedSegment = segments[segments.size()-1]; 
        if (otherSharedSegment == mTerminalSegments[0]) {
          //dbprintf(5, "ExtendByArm(): Changing otherSharedSegment to  mTerminalSegments[0]\n"); 
          otherSharedSegment = segments[0]; 
        }
        if (!otherSharedSegment->HasEndpoint(sharedNode)) {
          dbprintf(0, "ExtendByArm(): Error:  found a looped arm where one of the terminal segments does not have the shared node as an endpoint.\n");
          errexit; 
        }
        mTerminalSegments.push_back(otherSharedSegment); 
        //dbprintf(5, "ExtendByArm(): After pushing back otherSharedSegment, we look like this: %s\n", Stringify(0, mArmID==130704).c_str()); 
      }   
      mTerminalNodes.push_back(sharedNode); 
    }
      
    if (mNumSegments == 1 && mTerminalSegments.size() == 1) {
      //  dbprintf(5, "ExtendByArm(): Single segment arm:  duplicate our terminal segment.\n"); 
      mTerminalSegments.push_back(mTerminalSegments[0]); 
    }

    if (mTerminalSegments.size() != 2 || mTerminalNodes.size() != 2) {
      dbprintf(0, "ExtendByArm(): Error: arm with a single terminal segment or node cannot be properly extended at this point.\n");  
      errexit; 
    }

    // int nonSharedNum = 1-sharedNodeNum; 
    
    /*!
      first check node 0 against node 1, then node 1 against node 0
    */ 
    int sharedSegmentNum = mTerminalSegments.size(); 
    while(sharedSegmentNum--) {
      if (mTerminalSegments[sharedSegmentNum]->HasEndpoint(sharedNode)) {
        sharedNode->RemoveNeighbor(mTerminalSegments[sharedSegmentNum]); 
        sharedNode->RemoveNeighbor(this); 
        mTerminalSegments[sharedSegmentNum]->ReplaceEndpoint(sharedNode, newNode, false);
        newNode->AddNeighbor(mTerminalSegments[sharedSegmentNum]);
        // dbprintf(5, "Replaced the sharedNode %d as endpoint of terminal segment %d with newNode %d\n", 
        //        sharedNode->GetIndex(), mTerminalSegments[sharedSegmentNum]->mSegmentID, newNode->GetIndex()); 
       break; 
      }
    }
    if (sharedSegmentNum == -1) {
      dbprintf(0, "Error:  could not find terminal segment that has the old shared node as an endpoint.\n");
      errexit; 
    }
    FullNode *sourceNonSharedNode = sourceArm->mTerminalNodes[sourceNonSharedNum];
    // dbprintf(5, "ExtendByArm():  sharedNode = %d, sharedSegment = %d, sourceNonSharedNode = %d\n", sharedNode->GetIndex(),  mTerminalSegments[sharedSegmentNum]->mSegmentID, sourceNonSharedNode->GetIndex()); 
    // Now iterate over all segments and create duplicate internal nodes and segments.  
    int btype = GetBurgersType(); 
    vector<ArmSegment*> sourceSegments = sourceArm->GetSegments(sharedNode); 
    uint32_t seg = 0; 
    FullNode *sourceNode = sharedNode; // for iterating through source arm 
    ArmSegment *interiorSegment = NULL;
    while (seg < sourceSegments.size()) {
      sourceNode = sourceSegments[seg]->GetOtherEndpoint(sourceNode);
      
      int firstNodeType = firstNode->GetNodeType(), 
        otherNodeType =otherNode->GetNodeType();
      
      if (firstNodeType == 3 && otherNodeType ==  -44) {
        /*! 
          type3node is a "special butterfly"  
          Note -- we don't check if firstNodeType is -33 -- would be redundant.
          We don't check if firstNodeType is -3, because -33 "loses" to -3. 
        */ 
        firstNode->SetNodeType(-33); 
        
    if (mTerminalNodes.size() == 2 && mTerminalNodes[0] == mTerminalNodes[1]) {
      // dbprintf(5, "ExtendByArm(): After extending the arm it now forms a loop.  Consolidating terminal nodes.\n"); 
      mTerminalNodes.erase(++mTerminalNodes.begin(), mTerminalNodes.end()); 
    }
    
    {
      // this is intensive, for debugging, take this out later:
      vector<ArmSegment *>segments = GetSegments(); 
      if (segments.size() != mNumSegments) {
        dbprintf(0, "ExtendByArm(): Error:  segments.size() %d  != mNumSegments %d\n", 
                 segments.size(), mNumSegments); 
        dbprintf(5, "ExtendByArm() line %d: Arm 130704 segments: %s \n",
                 __LINE__, GetSegmentsAsString().c_str()); 
        errexit; 
      }
    }
    
    dbprintf(5, "\nExtendByArm(): After extension the arm looks like this: %s", Stringify(0,false).c_str()); 
    /* if (mArmID == 130704) {
      dbprintf(5, "ExtendByArm() at end of function line %d: Arm 130704 segments: \n%s \n",
               __LINE__, GetSegmentsAsString().c_str()); 
               }*/ 
    return; 
  }

  //===========================================================================
  /*!
    Decompose an arm by absorbing its nodes and segments into lower-energy neighbors 
  */
  bool Arm::Decompose(int energy) {
    int8_t burgtype = GetBurgersType();
    if (burgtype/10 != energy) 
      return false; // not yet

    // ComputeLength(); 

    // Find which terminal node to use in decomposing ourself.  
    // We prefer the terminal node that decomposes us into the smallest number of arms. 
    // This will be the one with the least number of neighbors.  If two terminal nodes have the same number of neighbors, use the one that has the lowest maximum energy level.  
    // I believe this greedy algorithm results in the global minimum decomposition too.  

    int numTermNodes = mTerminalNodes.size();
    vector<int> maxEnergies; // we'll analyze this later. 
    vector<int> numneighbors; 
    vector<int> extendedArmIDs; 
    if (numTermNodes == 0 || numTermNodes == 1) {
      // We don't expect this!  I don't expect type 200 or higher arms to form loops
      dbprintf(0, "Looped arm, will not decompose: %s\n", Stringify(0).c_str()); 
      return false;  
    }

    vector<int> allNeighborArmIDs; 
    dbprintf(5, "\n================================================================\n Found arm %d to decompose: %s\n", mArmID, Stringify(0, false).c_str());

    int sharedNodeNum = -1; 
    int termnode = 0; 
    while (termnode < numTermNodes) {
      numneighbors.push_back(mTerminalNodes[termnode]->mNeighborArms.size()); 
      maxEnergies.push_back(0); 
      int neighbor = 0;
      while (neighbor < numneighbors[termnode]) {
        Arm *neighborArm = mTerminalNodes[termnode]->mNeighborArms[neighbor]; 
        if (neighborArm != this && 
            neighborArm->GetBurgersType()/10 > maxEnergies[termnode]) {
         allNeighborArmIDs.push_back(neighborArm->mArmID); 
         maxEnergies[termnode] = neighborArm->GetBurgersType()/10;
        }
      }
    }
    return ;
  }/* end CheckForButterfly()*/ 

  //===========================================================================
  std::string Arm::Stringify(void) const {
    std::string s  = string("(arm): ") + 
#ifdef DEBUG
      "number " + intToString(mArmID) + 
      ", numSegments = " +intToString(mNumSegments) +
      ", length = " +doubleToString(GetLength()) +
#endif
      ", Type " +  intToString(mArmType);
#if LINKED_LOOPS
    if (mPartOfLinkedLoop) {
      s += ", is part of linked loop.\n"; 
    } else {
      s += ", is NOT part of linked loop.\n"; 
    }
#endif 

    int num = 0, max = mTerminalNodes.size(); 
    while (num < max) {
      s+= "Terminal Node " + intToString(num) + string(": ");
      if (mTerminalNodes[num]) {
        s += mTerminalNodes[num]->Stringify() + string("\n"); 
      } else {
        s += "(NULL)\n"; //deleted because it was useless.  
      }
      ++num; 
    }
    max = mTerminalSegments.size(); num = 0; 
    while (num < max) {
      s+= "Terminal Segment " + intToString(num) + string(": "); 
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
  string MetaArm::Stringify(int indent) {
    int atype = mMetaArmType;
    string s = INDENT(indent) + str(boost::format("(MetaArm): mLength: %1%, mMetaArmType: %2% (%3%), mMetaArmID: %4%\n\n")%mLength % atype % MetaArmTypeNames(mMetaArmType) % mMetaArmID); 
    uint32_t i = 0; 
    while (i<mTerminalNodes.size()) {
      string s2 = mTerminalNodes[i]->Stringify(indent+1);
      s += INDENT(indent+1);
      s+=  str(boost::format("Terminal Node %1%: %2%\n") % i % s2);
      ++i; 
    } 
    s += "\n"; 
    i = 0; 
    while (i<mTerminalArms.size()) {
      s += INDENT(indent+1) + str(boost::format("Terminal Arm %1%: %2%") % i % mTerminalArms[i]->Stringify(indent + 1));
      ++i; 
    }
    return s; 
  } 
   
  //===========================================================================
  // MetaArm::FindEndpoint()
  // recursive function, called by FindEndpoints())
  // Given a type 111 arm as seed, the node connecting the candidate to the previous arm in the chain, and a candidate, see if it can extend to an endpoint.  Return false when it fails, and true when it succeeds.  
  //  previous == node connecting the candidate to the last examined arm in the calling chain.  
  //  On first call, candidate != seed, but if we see the seed again, we have looped.  This is guaranteed because the seed is the last thing ever checked of any neighbor of an arm. 
  // 
  //===========================================================================
  bool MetaArm::FindEndpoint(Arm *seed, FullNode *previous, Arm* candidate) { 
    bool err = false; 
 
    dbprintf(4, "\n-------------------------------------------  \n");
    dbprintf(4, "FindEndpoint: seed = %s\nprevious = %s\n\ncandidate = %s\n", 
             seed->Stringify(0).c_str(), previous->Stringify(0).c_str(), candidate->Stringify(0, false).c_str()); 

    if (candidate->mArmType == ARM_EMPTY) {
      dbprintf(4, "FindEndpoint: candidate arm is empty.  This happens when decomposing high energy arms.  Skipping.\n"); 
      return false; 
    } 

    if (candidate->mSeenInMeta) {
       dbprintf(4, "FindEndpoint: candidate arm is already seen in this arm -- return false\n"); 
       return false; 
    }

    if (candidate->mSeen) {
      dbprintf(4, "FindEndpoint: candidate arm is type111 and already seen somewhere  -- return false\n"); 
      return false; 
    }
    if ( candidate->GetBurgersType() != seed->GetBurgersType()) {
      dbprintf(4, "FindEndpoint: candidate arm burgers (%s) does not match seed burgers (%s)-- return false\n", BurgersTypeNames(candidate->GetBurgersType()).c_str(), BurgersTypeNames(seed->GetBurgersType()).c_str()); 
      return false; 
    }
    
    if (candidate->isTypeMM()) {
      dbprintf(4, "FindEndpoint: Error: candidate arm is type MM -- This should never happen.\n"); 
      return false; 
    }
    
    if (candidate->mArmType == ARM_LOOP) {
      dbprintf(4, "FindEndpoint: Candidate is a loop. Do not explore.\n"); 
      return false; 
    }

    candidate->mSeen = true; 
    candidate->mSeenInMeta = true; 

    if (candidate->isType111()) {
      dbprintf(4, "Candidate is type 111 and we shall try to recurse..\n");
      mFound111 = true; 
    }
    else {
      dbprintf(4, "Candidate is type 200 and we shall try to recurse..\n");
    }
    uint32_t nodenum = candidate->mTerminalNodes.size(); 
    if (nodenum < 2) {
      dbprintf(0, "Error:  Found candidate with %d terminal node(s), but we already tested for loops. \n", nodenum); 
      errexit1; 
     }

    FullNode * otherNode = NULL; 
    while (nodenum --) {
      FullNode * node = candidate->mTerminalNodes[nodenum]; 
      if (node == previous) {
        continue; 
      }
      otherNode = node; 
      if (node->GetNodeType() < 0) {
        dbprintf(4, "FindEndpoint: Candidate has M node on other end.  Terminating and returning true.\n"); 
        AddTerminalNode(node); 
        AddTerminalArm(candidate); 
        return true; 
      }

      uint32_t neighbornum = node->mNeighborArms.size(); 
      dbprintf(4, "FindEndpoint: Candidate has N node on other end. Recursing on %d neighbors.\n", neighbornum); 
      Arm *foundseed = NULL; 
      while (neighbornum--) {
        Arm *arm = node->mNeighborArms[neighbornum]; 
        if (arm == candidate) {
          continue; // no need to examine ourselves.  
        }
        if (arm == seed) {
          dbprintf(4, "FindEndpoint: Hey!  Neighbor %d is the seed arm.  Are we a loop? Defer.\n", neighbornum); 
          foundseed = seed; 
          continue; 
        }
        dbprintf(4, "FindEndpoint: Recursing on candidate neighbor arm...\n"); 
        if (FindEndpoint(seed, node, node->mNeighborArms[neighbornum])) {
          AddArm(candidate); 
          return true; 
        }
      }
      dbprintf(4, "FindEndpoint: Done Checking candidate neighbor arms.\n"); 
      
      if (foundseed) {
        dbprintf(4, "FindEndpoint: We found the seed after all other neighbors are exhausted.  Thus we are in a looped arm.\n"); 
        AddArm(candidate); 
        AddTerminalArm(seed); 
        AddTerminalNode(node); 
        mMetaArmType = METAARM_LOOP_111; 
        return true;
      }
    }
    dbprintf(4, "FindEndpoint: Done recursing on neighbors.\n"); 
    
    if (candidate->isType111()) {
      dbprintf(4, "FindEndpoint:  Candidate is a type111 arm with no way to extend beyond its type N node.  Terminate and mark as METAARM_UNKNOWN.\n"); 
      mMetaArmType = METAARM_UNKNOWN; 
      AddTerminalNode(otherNode); 
      AddTerminalArm(candidate); 
      return true; 
    }

    dbprintf(4, "FindEndpoint:  Candidate is a dead end.\n");   
    mDeadEnds.push_back(candidate); // so you can mark them as not seen later.  
    return false; 
  }
   

  //===========================================================================
  void MetaArm::FindEndpoints(Arm *seed) {      

    dbprintf(4, "=======================================================\n", seed->Stringify(0).c_str()); 
    dbprintf(4, "FindEndpoints called with seed %s\n", seed->Stringify(0, false).c_str()); 
    
    seed->mSeen = true; 
    // mLength = seed->GetLength(); 
    if (seed->mArmType == ARM_LOOP) {
      if (seed->isType200()) {
        dbprintf(4, "FindEndpoints: Seed arm is Type 200 LOOP arm, so this is METAARM_LOOP_200.\n"); 
        mMetaArmType = METAARM_LOOP_200; 
      }
      else {
        dbprintf(4, "FindEndpoints: Seed arm is Type 111 LOOP arm, so this is METAARM_LOOP_111.\n"); 
        mMetaArmType = METAARM_LOOP_111; 
      }
      dbprintf(0, "FindEndpoints: Seed arm is a loop. Adding as its own metaArm.\n");
      AddTerminalArm(seed); 
      mTerminalNodes = seed->mTerminalNodes; 
      CapturePath(false); 
      return;
    }

    mLength = 0; 
    if (seed->isTypeMM()) {
      if (seed->isType200()) {
        dbprintf(4, "FindEndpoints: Seed arm is Type 200 MM arm.  This is METAARM_UNKNOWN, since all non-loop type 200 arms should have been decomposed.\n"); 
        mMetaArmType = METAARM_UNKNOWN; 
      }               
      else if (seed->isType111()){
        dbprintf(4, "FindEndpoints: Seed arm is Type 111 MM arm, so this is METAARM_111.\n"); 
        mMetaArmType = METAARM_111; 
      }
      else {
        dbprintf(0, "FindEndpoints: Seed arm is Type %d MM arm.  This is supposed to be impossible.  Aborting.\n", seed->mArmType); 
        errexit; 
      }       
      AddTerminalArm(seed); 
      mTerminalNodes = seed->mTerminalNodes; 
      CapturePath(false); 
      return;
    }
    
    if (seed->isType200() || seed->isTypeUnknown()) {
      dbprintf(4, "FindEndpoints: Seed arm is Type 200 or Unknown with at least one type N node.  This is a bad choice as seed as it can lead to ambiguities, and it will be included in at least one other meta-arm as a terminal arm.  Giving up and trying new seed.\n"); 
      mMetaArmType = METAARM_UNKNOWN; 
      return; 
    }
    
    // consider this to be a completely self contained metaamr. 
    // Best algorithm: 
    
    // A metaarm can continue in up to two directions, call them paths.  
    //uint32_t pathsTaken = 0; 
    
    // First, check each terminal node of the arm.  If it's a monster, add it as a meta arm endpoint and mark off a path.  If it's not, then recurse on it to extend the arm.  
    dbprintf(4, "FindEndpoints: Checking terminal nodes of seed arm.\n"); 
    uint32_t nodenum = 0; // seed->mTerminalNodes.size(); 
    bool seedIsTerminal = false; 
    while (nodenum <  seed->mTerminalNodes.size()) {
      FullNode * node = seed->mTerminalNodes[nodenum]; 
      if (node->GetNodeType() < 0) {
        dbprintf(4, "FindEndpoints: Terminal node %d is a monster node. Add seed as terminal arm and node as terminal node.\n", nodenum); 
        // Monster node
        if (!mTerminalArms.size() || mTerminalArms[0] != seed) {
          AddTerminalArm(seed); 
          seedIsTerminal = true; 
        }
        AddTerminalNode(node); 
      }
      else {
        dbprintf(4, "FindEndpoints: Terminal node %d is not a monster node. Recurse on its neighbors.\n", nodenum); 
        int neighborNum = node->mNeighborArms.size(); 
        while (neighborNum--) {
          Arm *arm = node->mNeighborArms[neighborNum]; 
          if (arm == seed) {
            dbprintf(4, "FindEndpoints: Node neighbor %d is seed.  Ignore.\n", nodenum); 
            continue; 
          }
          dbprintf(4, "FindEndpoints: call FindEndpoint on neighbor %d...\n", nodenum); 
          if (FindEndpoint(seed, node,  arm)) {            
            dbprintf(4, "FindEndpoints: Node neighbor %d  resulted in valid endpoint.\n", nodenum);
            if (nodenum == seed->mTerminalNodes.size()-1) {
              if (!seedIsTerminal) {
                dbprintf(4, "Adding seed as it is not a terminal arm and we are on the final terminal node.\n"); 
                AddArm(seed); 
              }
              else {
                dbprintf(4, "We are on the final terminal node but we are not adding the seed as it is already a terminal arm.\n");
              }
            }
            break; // only one path allowed per terminal node.  
          }
        }
      }
        
      CapturePath(nodenum); // Arms are collected in reverse, so reverse that for second node 
      ++nodenum; 
    }
    // we must be METAARM_111; 
    if (mMetaArmType == METAARM_UNKNOWN) 
      mMetaArmType = METAARM_111; 

    
    int armnum = mAllArms.size(); 
    while (armnum--) {
      mAllArms[armnum]->mSeenInMeta = false; 
    }
    armnum = mTerminalArms.size(); 
    while (armnum--) {
      mTerminalArms[armnum]->mSeenInMeta = false; 
    }
    armnum = mDeadEnds.size(); 
    while (armnum--) {
      mDeadEnds[armnum]->mSeenInMeta = false; 
    }
    int atype = mMetaArmType;
    dbprintf(4, "MetaArm is type %d, and found111 is %d\n", atype, (int)mFound111); 

    return;

  }
 
  //===========================================================================
  // see also DebugPrintArms() and PrintArmFile() -- this just tabulates a summary
  void DataSet::PrintArmStats(FILE *thefile) {
    //if (!dbg_isverbose()) return;
    //dbprintf(3, "Beginning PrintArmStats()"); 
    double armLengths[11] = {0}, totalArmLength=0; 
    
   
    uint32_t numArms[11] = {0};  // number of arms of each type
    uint32_t totalArms=0;
#if LINKED_LOOPS
    double linkedLoopLength = 0; 
    uint32_t numLinkedLoops = 0; 
#endif
    
    double *armLengthBins = NULL; 
    long *armBins = NULL; 
    if (mNumBins) {
      armLengthBins = (double*)calloc(mNumBins, sizeof(double)); 
      armBins = (long*)calloc(mNumBins, sizeof(long));  
    }
    //NN types corresponding to burgers values of the NN arms:    
    const char *armTypes[7] = {
      "NN_100", 
      "NN_010", 
      "NN_001", 
      "NN_+++",
      "NN_++-",
      "NN_+-+",
      "NN_-++"
    };
    
    double shortLengths[7] = {0}, longLengths[7]={0}; 
    uint32_t numShortArms[7]={0}, numLongArms[7]={0}; 

    vector<Arm>::iterator armpos = mArms.begin(), armend = mArms.end(); 
    while (armpos != armend) { 
      double length = armpos->GetLength(); 
      int armType = armpos->mArmType; 
      if (mThreshold >= 0) {
        int8_t btype = armpos->GetBurgersType(); 
        if (!btype) {
          printf("Error:  armpos has no terminal segments!\n"); 
        }
        if (armType == ARM_SHORT_NN_111 || armType == ARM_SHORT_NN_100 ) {
          numShortArms[btype-1]++;
          shortLengths[btype-1] += length; 
        }
        else {
          numLongArms[btype-1]++;
          longLengths[btype-1] += length; 
        }       
      }
      if (mNumBins) {
        int binNum = length/Arm::mLongestLength * mNumBins; 
        if (binNum == mNumBins) binNum = mNumBins-1; 
        if (binNum > mNumBins || binNum < 0) {
          printf("Error:  binNum %d is invalid (num bins is %d)\n", binNum, mNumBins); 
        } else {
          armLengthBins[binNum] += length; 
          armBins[binNum]++; 
        } 
      }
      armLengths[armType] += length;
      totalArmLength += length; 
      numArms[armType]++;       
      
      totalArms++; 
#if LINKED_LOOPS
      if (armpos->mPartOfLinkedLoop) {
        numLinkedLoops ++; 
        linkedLoopLength += length; 
      }
#endif
      ++armpos; 
    }
    
    const char *armTypeNames[11] = 
      { "ARM_UNKNOWN",
        "ARM_UNINTERESTING",
        "ARM_LOOP",
        "ARM_MM_111" ,
        "ARM_MN_111",
        "ARM_NN_111" ,
        "ARM_MM_100",
        "ARM_MN_100",
        "ARM_NN_100",
        "ARM_SHORT_NN_111",
        "ARM_SHORT_NN_100"
      };
    printf("\n"); 
    printf("===========================================\n"); 
    printf("total Number of arms: %d\n", totalArms); 
    printf("total length of all arms: %.2f\n", totalArmLength); 
    printf("Number of segments classified in arm: %d\n", gNumClassified); 
    printf("Number of segments measured in arm: %d\n", gNumArmSegmentsMeasured); 
    printf("Number of segments wrapped: %d\n", gNumWrapped); 
    printf("===========================================\n"); 
    int i = 0; for (i=0; i<11; i++) {
      printf("%s: number of arms = %d\n", armTypeNames[i], numArms[i]);
      printf("%s: total length of arms = %.2f\n", armTypeNames[i], armLengths[i]); 
      printf("----------------------\n"); 
    }
#if LINKED_LOOPS
    printf("LINKED LOOPS: total number of arms = %d\n", numLinkedLoops); 
    printf("LINKED LOOPS: total length of arms = %.2f\n", linkedLoopLength); 
    printf("----------------------\n"); 
#endif

    // write a row of arm lengths to make analysis via spreadsheet easier
    printf("Key: UNKNOWN\tUNINTRSTNG\tLOOP\tMM_111\tMN_111\tNN_111\tMM_100\tMN_100\tNN_100\tSHORT_NN_111\tSHORT_NN_100\n"); 
    int n = 0; 
    while (n<11) {
      printf("%.2f\t",  armLengths[n]); 
      ++n;
    }

           
    
    if (mThreshold >= 0.0) {
      printf("\n\n----------------------\n"); 
      printf("THRESHOLD data.  Threshold = %.2f\n", mThreshold); 
      int n = 0; 
      for (n=0; n<7; n++) {       
        printf("----------------------\n"); 
        printf("Total number of %s arms: %d\n", armTypes[n], numShortArms[n] + numLongArms[n]); 
        printf("Total length of %s arms: %.2f\n", armTypes[n], shortLengths[n] + longLengths[n]); 
        printf("Number of %s arms SHORTER than threshold = %d\n", armTypes[n], numShortArms[n]); 
        printf("Total length of %s arms shorter than threshold = %.2f\n", armTypes[n], shortLengths[n]); 
        printf("Number of %s arms LONGER than threshold = %d\n", armTypes[n], numLongArms[n]); 
        printf("Total length of %s arms longer than threshold = %.2f\n", armTypes[n], longLengths[n]); 
        printf("\n"); 
      }
    }
    
    // write a row of arm lengths to make analysis via spreadsheet easier
    printf("----------------------\n"); 
    printf("Key: NN_100\tNN_010\tNN_001\tNN_+++\tNN_++-\tNN_+-+\tNN_-++\n"); 
    printf("SHORT ARM LENGTHS:\n"); 
    n=0; while (n<7) {
      printf("%.2f\t",  shortLengths[n]); 
      ++n;
    }
    printf("\nLONG ARM LENGTHS:\n"); 
    n = 0; while (n<7) {
      printf("%.2f\t",  longLengths[n]); 
      ++n;
    }
    printf("\n"); 

    printf("----------------------\n\n\n"); 

    if (mNumBins) {
      // print a row of bin values
      printf("BINS: \n");
      printf("max length = %.3f\n", Arm::mLongestLength); 
      
      long totalArms = 0; // reality check
      double totalLength = 0;  // reality check
      string line; 
      int binNum = 0; 
      printf("%-12s%-12s%-12s\n", "Bin", "Arms", "Lengths"); 
      for (binNum = 0; binNum < mNumBins; ++binNum) {
        printf("%-12d%-12ld%-12.3f\n", binNum, armBins[binNum], armLengthBins[binNum]); 
        totalArms += armBins[binNum];
        totalLength += armLengthBins[binNum];
      }
      printf("%-12s%-12ld%-12.3f\n", "TOTAL", totalArms, totalLength); 
    }
    free(armLengthBins);
    free(armBins);

#ifdef DEBUG_SEGMENTS
    // check against segment lengths: 
    uint32_t numSegments[11] = {0}, totalSegments=0, culledSegments=0;  // number of arms of each type
    double segmentLengths[11] = {0}, totalSegmentLength=0, culledLength=0; 
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
    for (i=0; i<11; i++) {
      printf("%s: number of segs = %d\n", armTypeNames[i], numArms[i]);
      printf("%s: total length of segments = %.2f\n", armTypeNames[i], armLengths[i]); 
      printf("----------------------\n"); 
    }
    
    printf("CULLED segments = %d\n", culledSegments); 
    printf("CULLED length = %.2f\n", culledLength); 
    
    printf("Wrapped lengths: %.2f\n", gSegLen); 
    printf("----------------------\n\n\n"); 
#endif
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
        throw string("error reading neighbor number ") + intToString(neighbornum);
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
      throw string("Error in GetNodes while reading node ") + intToString(nodenum) +" at line " + intToString(lineno) + ":\n" + err; 
    }
    return; 
    dbprintf(2, "CreateMinimalNodes ended...\n"); 
  }

  //===========================================================================
  bool DataSet::Mkdir(const char *dirname) {
    dbprintf(3, "Mkdir(%s)",dirname); 
#ifndef WIN32
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
#else
    if (_mkdir(dirname) != 0)
    {
        string errmsg = string("Warning:  could not create debug output directory: ") + mDebugOutputDir+ ". Stats will not be written." ; 
        cerr << errmsg << endl; 
        dbprintf(1, errmsg.c_str()); 
        return false; 
    }
#endif
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
      err = string("Error in ClassifyMinimalNodes, node ") + intToString(nodenum) + ": " + err; 
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
                throw string("error reading line ")+ intToString(linenum) + string(" of neighbor ") + intToString(neighborNum);
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
          
#ifdef __GNUC__
          ArmSegmentSet::iterator foundSegment = mQuickFindArmSegments.find(currentSegment); 
          if (foundSegment == mQuickFindArmSegments.end()) {
            currentSegment->SetID(); 
            mQuickFindArmSegments.insert(currentSegment); 
            foundSegment = mQuickFindArmSegments.find(currentSegment); 
          } else {
            (*foundSegment)->ReplacePlaceholder(fullNode);             
          }
          fullNode->AddNeighbor((*foundSegment)); 
#endif
         
          ++neighbornum; 
        }; // done reading neighbor information
      } catch (string err) {
        throw string("Error in DataSet::ReadFullNode reading neighbor ")+intToString(neighbornum)+":" + err; 
      }
    } catch (string err) {
      throw string("Error trying to read full node info corresponding to ")+theNode.Stringify(true) + string("numskipped is ")+intToString(numskipped) + string("\n") + err;
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
      throw string("Error in DataSet::CreateFullNodes while reading node ") + intToString(nodenum) +":\n" + err; 
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
  void DataSet::DebugPrintFullNodes(const char *name) {
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
  void DataSet::FindEndOfArm(FullNodeIterator &iStartNode, FullNode **oEndNode,  ArmSegment *iStartSegment,  ArmSegment *&oEndSegment
#ifdef DEBUG
, Arm &theArm
#endif
) {
    //FullNodeIterator currentNode = iStartNode, otherEnd; 
    FullNode *currentNode = *iStartNode, *otherEnd; 
    ArmSegment *currentSegment = iStartSegment; 
    /* loop, don't recurse */ 
    while(true) {
#ifdef DEBUG
      if (!currentSegment->Seen()) {
        ++theArm.mNumSegments; 
        dbprintf(5, "Arm %d: adding segment %s\n", theArm.mArmID, currentSegment->Stringify().c_str());
      }
#endif
      currentSegment->SetSeen(true);  
      
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
        currentSegment = const_cast<ArmSegment*>(otherEnd->GetNeighbor(1)); 
      } else {
        currentSegment = const_cast<ArmSegment*>(otherEnd->GetNeighbor(0)); 
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
        ArmSegment *endSegment0 = NULL, *endSegment1 = NULL, 
          *startSegment0 = NULL, *startSegment1 = NULL; 
        if (numneighbors == 2) {
          startSegment0 = const_cast< ArmSegment*>((*nodepos)->GetNeighbor(0));
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

            startSegment1 = const_cast< ArmSegment*>((*nodepos)->GetNeighbor(1));
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
              const_cast< ArmSegment*>((*nodepos)->GetNeighbor(neighbornum));
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
      throw string("Arm #")+intToString(armnum)+": "+err;
    }
#if LINKED_LOOPS
    int armNum = mArms.size(); 
    while (armNum--) {
      mArms[armNum].SetSegmentBackPointers(); 
    } 
#endif
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
      ++pos; 
    }
    debugfile << "Number of Metaarms: " << mMetaArms.size()<< endl; 
    debugfile << "Total memory used by arms: " << mMetaArms.size() * sizeof(MetaArm) << endl; 
    
    cerr<< endl << "Wrote MetaArm details to debug file "<< filename<< endl; 
    return; 
  }

  //===========================================================================
  // Print out all arms in vtk file format in a file for analysis, just for Meijie
  void DataSet::WriteMetaArmVTK(char *vtkfilename) {
    return; 
    
  }

  //===========================================================================
  // Print out all MetaArms in a simple format in a file for analysis, just for Meijie
  void DataSet::PrintMetaArmFile(void) {
    if (mMetaArmFile == "") return;
    dbprintf(0, "Writing metaarms to metaarm file %s\n", mMetaArmFile.c_str()); 
    FILE *armfile = fopen (mMetaArmFile.c_str(), "w"); 
    if (!armfile) {
      cerr << "ERROR:  Cannot open output file to write out arms" << endl;
      return;
    }
    fprintf(armfile, "DISCUSSION: \n%s\n", doctext.c_str()); 
    
    vector<boost::shared_ptr<MetaArm> >::iterator pos = mMetaArms.begin(), endpos = mMetaArms.end(); 
    uint32_t armnum = 0, metaarmcounts[7]={0}; 
    double metaarmtypelengths[7] = {0.0}; 
    uint32_t numarms = 0; 
    while (pos != endpos) {
      metaarmcounts[(*pos)->mMetaArmType]++; 
      metaarmtypelengths[(*pos)->mMetaArmType] += (*pos)->mLength; 
      numarms += (*pos)->mAllArms.size(); 
      vector<Arm*>::iterator armpos = (*pos)->mAllArms.begin(), armend = (*pos)->mAllArms.end(); 
      while (armpos != armend) {
        if ((*armpos)->mSeen) {
          fprintf(stderr, "Error in PrintMetaArmFile: arm %d in metaarm %d has mSeen == true", (*armpos)->mArmID, (*pos)->mMetaArmID); 
          errexit; 
        }
        (*armpos)->mSeen = true; 
        ++armpos;
      } 
      ++pos; 
    }
    vector<Arm*>::iterator armpos = mArms.begin(), endarm = mArms.end(); 
    armnum = 0; 
    while (armpos != endarm) {   
      if ((*armpos)->mArmType != ARM_EMPTY && !(*armpos)->mSeen) {
        cerr << "Error: arm " << armnum << " has not been seen in a metaarm." << endl; 
        errexit; 
      }
      ++ armnum;
      ++armpos; 
    }
    dbprintf(2, "FindButterflies ended.\n");
    return; 
  }

  //===========================================================================
  void DataSet::ClassifyArms(void) {
    dbprintf(2, "ClassifyArms starting...\n");
    vector<Arm>::iterator armpos = mArms.begin(), armend = mArms.end(); 
    while (armpos != armend) {      
      armpos->Classify(); 
      ++armpos; 
    }
    dbprintf(2, "ClassifyArms ended.\n");
    return; 
  }
  
  //===========================================================================
  void DataSet::ComputeArmLengths(void) {
    dbprintf(2, "ComputeArmLengths starting...\n");
    vector<Arm>::iterator armpos = mArms.begin(), armend = mArms.end(); 
    while (armpos != armend) {      
      armpos->ComputeLength(); 
      ++armpos; 
    }
    dbprintf(2, "ComputeArmLengths ended.\n");
    return; 
  }
  
  //==========================================================================
  /*!
    Unary predicate for using STL to remove all useless nodes in a range
  */ 
   bool NodeIsUseless(FullNode *node) {
     return false; 
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
    /*    while (nodepos != nodeend) {      
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
    */
    /*!
      Next find all arms that have a useless node as either endpoint.  Delete them.  
    */ 
    /*  dbprintf(2, "Identifying and deleting useless arms...\n"); 
    numdeleted = mArms.size(); 
    vector<Arm>::iterator armpos = mArms.begin(), armend = mArms.end(); 
    armpos = remove_if(armpos, armend, ArmIsUseless); 
    mArms.erase(armpos, mArms.end()); 
    numdeleted -= mArms.size(); 
    dbprintf(2, "Deleted %d arms.\n", numdeleted); 
    */
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
          string err = string("Error: bad type ") + intToString( nodetype) +  " in node: " + (*nodepos)->Stringify();
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
      /*  We can now compute arm lengths properly */
      ComputeArmLengths(); 
 
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
  
  
  //===========================================================================
  void DataSet::FindMetaArms(void){
    vector<Arm*>::iterator currentArm = mArms.begin(), endArm = mArms.end(); 
    while (currentArm != endArm) {
      (*currentArm)->mSeen = false; 
      ++ currentArm;       
    }
    currentArm = mArms.begin(), endArm = mArms.end(); 
    uint32_t numMetaArms = 0, numArms = 0, totalArms = mArms.size(); 
    STARTPROGRESS(); 
    dbprintf(4, "FindMetaArms: %s\n", datestring()); 
    while (currentArm != endArm) {
      if ( ! (*currentArm)->mSeen  && 
           ! (*currentArm)->isTypeUnknown() && 
           (*currentArm)->mArmType != ARM_EMPTY
        ) {
        boost::shared_ptr<MetaArm> metaArmSPtr = boost::make_shared<MetaArm>(); 
        metaArmSPtr->FindEndpoints(*currentArm); 
        if (metaArmSPtr->mMetaArmType != METAARM_UNKNOWN) {
          dbprintf(4, "Adding meta arm %d with seed %d\n", mMetaArms.size(), (*currentArm)->mArmID); 
          metaArmSPtr->mMetaArmID = mMetaArms.size();
          mMetaArms.push_back(metaArmSPtr); 
          numMetaArms++;           
        }
      } else {
        dbprintf(4, "Skipping currentArm %d (type %d) as it was either seen (%d) and was type 111 or an unknown arm, or else FindEndpoints could not handle it.\n", (*currentArm)->mArmID, (*currentArm)->mArmType, (*currentArm)->mSeen);  
      }
      numArms++; 
      ++ currentArm; 
      UPDATEPROGRESS(numArms, totalArms, str(boost::format("FindMetaArms: %1% MetaArms created")%numMetaArms)); 
    }
    currentArm = mArms.begin();
    while (currentArm != endArm) {
      if (!(*currentArm)->mSeen && (*currentArm)->mArmType != ARM_EMPTY) {
        uint32_t id = (*currentArm)->mArmID;
        dbprintf(0, "\n\nError: arm %d has not been seen!\n", id); 
        errexit; 
      }
      (*currentArm)->mSeen = false; 
      ++currentArm; 
    }
    
    return; 
  }

  
  
} // end namespace paraDIS 


