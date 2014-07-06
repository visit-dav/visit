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
#include "paradis_version.h"
//#include "visit_writer.h"

#ifndef WIN32
#  include <dirent.h>
#else
#  include <direct.h>
#endif
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

// GRRR.  Visit hooks are lame.  This is bad code but if I don't structure it like this, the SVN hooks complain. 
#ifdef  USE_ABORT
#define errexit abort()
#define errexit1 abort()
#else
#define errexit return 
#define errexit1 return err
#endif

using namespace rclib; 
using namespace RC_Math; 
using namespace std; 
  
#ifdef RC_CPP_VISIT_BUILD
void dbstream_printf(int level, const char *fmt, ...) {
  char buf[4096]; 
  va_list ap;                                   
  va_start(ap, fmt);
  vsnprintf(buf,4095,fmt,ap);
  va_end(ap);
  if (level == 5)  { debug5 << buf; debug5.flush(); }
  if (level == 4)  { debug4 << buf; debug4.flush(); }
  if (level == 3)  { debug3 << buf; debug3.flush(); }
  if (level == 2)  { debug2 << buf; debug2.flush(); }
  if (level < 2)  { debug1 << buf; debug1.flush(); }
  if (level == 0)  cout << buf; 
  return; 
}
#endif
#define dbecho(level, msg) cerr << msg; dbprintf(level, string(msg).c_str())

std::string GetLibraryVersionString(const char *progname) {
  return str(boost::format("%s: using paraDIS_lib version %s compiled on %s")% progname% RC_PARADIS_VERSION% RC_PARADIS_VERSION_DATE); 
}

std::string GetLibraryVersionNumberString(void) {
  return str(boost::format("%1%")% RC_PARADIS_VERSION); 
}

std::string INDENT(int i) {
  if (!i) return "";
  return str(boost::format(str(boost::format("%%1$%1%s")%(i*3)))%" "); 
}
#define STARTPROGRESS()                                       \
  timer theTimer;  theTimer.start();            \
  double theTime=theTimer.elapsed_time(), thePercent=0;   

#define UPDATEPROGRESS(count, total, description)                       \
  if (dbg_isverbose()) {                                                \
    Progress(theTimer, count, total, thePercent, 1, theTime, 1, description);  }
#define COMPLETEPROGRESS(total, description) \
  UPDATEPROGRESS(total,total,description); fprintf(stderr, "\n");    \

string BurgersTypeNames(int btype) {
  switch (btype) {
  case BURGERS_UNKNOWN  : return "UNKNOWN"; 
  case BURGERS_NONE   : return "NONE"; 
  case BURGERS_PPP     : return "PPP"; 
  case BURGERS_PPM    : return "PPM"; 
  case BURGERS_PMP   : return "PMP"; 
  case BURGERS_PMM   : return "PMM"; 
  case BURGERS_200     : return "200"; 
  case BURGERS_020      : return "020"; 
  case BURGERS_002    : return "002"; 
  case BURGERS_220     : return "220"; 
  case BURGERS_202      : return "202"; 
  case BURGERS_022      : return "022"; 
  case BURGERS_311      : return "311"; 
  case BURGERS_131     : return "131"; 
  case BURGERS_113    : return "113"; 
  case BURGERS_222    : return "222"; 
  case BURGERS_004     : return "004"; 
  default: return "UNKNOWN CODE"; 
  }
}

string ArmTypeNames(int atype) {
  switch (atype) {
  case ARM_EMPTY         : return "EMPTY"; 
  case ARM_UNKNOWN        : return "UNKNOWN"; 
  case ARM_UNINTERESTING  : return "UNINTERESTING"; 
  case ARM_LOOP           : return "LOOP"; 
  case ARM_MM_111          : return "MM_111"; 
  case ARM_MN_111         : return "MN_111"; 
  case ARM_NN_111          : return "NN_111"; 
  case ARM_SHORT_NN_111   : return "SHORT_NN_111"; 
  default: return "UNKNOWN ARMTYPE"; 
  }
}

string MetaArmTypeNames(int mtype) {
  switch (mtype) {
  case METAARM_UNKNOWN             : return "METAARM_UNKNOWN"; 
  case METAARM_111                 : return "METAARM_111"; 
  case METAARM_LOOP_111            : return "METAARM_LOOP_111"; 
  case METAARM_LOOP_HIGH_ENERGY    : return "METAARM_LOOP_HIGH_ENERGY"; 
  default                          : return "METAARM ERROR"; 
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

// =====================================================================
/*!
  This prints out a list of arms with their depths to a text file 
  and a set of VTK files.  
  File created: basename.txt, basename.vtk
*/ 
namespace paraDIS {

  //===========================================================================
  vector<MinimalNode> MinimalNode::mMinimalNodes; 

  rclib::Point<float> FullNode::mBoundsMin, FullNode::mBoundsMax, FullNode::mBoundsSize; 
  string FullNode::mTraceFileBasename; 
  vector<uint32_t> FullNode::mTraceNodes; 
  std::map<uint32_t, FullNode *>  FullNode::mFullNodes;  
  vector<FullNode*> FullNode::mFullNodeVector; 
  uint32_t FullNode::mNextNodeID = 0; 

  std::map<uint32_t, ArmSegment *> ArmSegment::mArmSegments; 
  vector<ArmSegment*> ArmSegment::mArmSegmentVector; 

  uint32_t ArmSegment::mNextSegmentID = 0; 
  double ArmSegment::mSegLen = 0 ;
  uint32_t ArmSegment::mNumClassified = 0; 
  uint32_t ArmSegment::mNumWrapped = 0;
  uint32_t ArmSegment::mNumArmSegmentsMeasured=0; 
  
  vector<Arm *> Arm::mArms;
  vector<int32_t> Arm::mTraceArms; 
  uint8_t Arm::mTraceDepth; 
  string Arm::mTraceFileBasename; 
  double Arm::mLongestLength = 0.0; 
  double Arm::mDecomposedLength = 0.0; 
  vector<int32_t> Arm::mNumDecomposed(7, 0); // statistics
  int32_t Arm::mNumDestroyedInDetachment = 0; 
  double Arm::mTotalArmLengthBeforeDecomposition = 0.0;
  double Arm::mTotalArmLengthAfterDecomposition = 0.0;  
  double Arm::mThreshold = -1; 

  rclib::Point<float> MetaArm::mWrappedNode(-424242.42, 424242.42, -424242.42);

  set<ArmSegment *, CompareSegPtrs> DataSet::mQuickFindArmSegments; 
  rclib::Point<float> DataSet::mDataMin, DataSet::mDataMax, DataSet::mDataSize;

  //===========================================================================

  void WriteTraceFiles(string filebase, string description, vector<Arm*>&arms, vector<uint32_t> &armdepths, vector<int> &action) {
    // ===================================
    // FIRST, PRINT THE TEXT FILE
    string filename = filebase + ".txt"; 
    std::ofstream textfile(filename.c_str());
    if (!textfile) {
      string errmsg = str(boost::format("\nWarning:    cannot open text file %1%: %2%.")%filename%strerror(errno));
      cerr << errmsg << endl; 
      dbprintf(1, "%s\n", errmsg.c_str()); 
      return; 
    }
    textfile << "Tracefile for " << description << ", written by paraDIS_lib" << endl; 
    textfile.flush(); 
    uint32_t level = 0; 
    for (uint32_t arm = 0; arm < arms.size(); arm++) {
      if (armdepths[arm] == level) { // this is weird, don't worry
        textfile << str(boost::format("\n****************\nARM LEVEL %1%\n****************\n\n")%level); 
        textfile.flush(); 
        level++; 
      }
      textfile << arms[arm]->Stringify(0) ; 
    }
    string msg = str(boost::format("\n*** Wrote text tracefile %s for %s\n") % filename % description);
    dbprintf(0, msg.c_str()); 
    cerr << msg; 
    textfile.flush(); 
    textfile.close(); 
  
    

    // ===================================
    // NEXT, PRINT THE VTK FILE FOR ARM SEGMENTS
    filename = filebase+"-segments.vtk";
    std::ofstream vtkfile(filename.c_str());
    if (!vtkfile) {
      string errmsg = str(boost::format("\n*** Warning:    cannot open vtk file %1%: %2%.")%filename%strerror(errno));
      cerr << errmsg << endl; 
      dbprintf(1, "%s\n", errmsg.c_str()); 
      return; 
    }
  
    // First, the header
    vtkfile << "# vtk DataFile Version 3.0" << endl;
    vtkfile << filename + ", written by paraDIS_lib. If action = 2, that's the arm being decomposed.  If action = 1, then it's the arm being extended. Else action = 0.  mBounds are " << FullNode::mBoundsSize.Stringify() << endl;
    vtkfile << "ASCII" << endl;
    vtkfile << "DATASET POLYDATA" << endl;
  
    // next, the points
    vector<FullNode *>nodes; 
    vector<uint32_t>numArmNodes; 
    for (uint32_t arm = 0; arm < arms.size(); arm++) {
      vector<FullNode *> armnodes = arms[arm]->GetNodes(); 
      numArmNodes.push_back(armnodes.size()); 
      nodes.insert(nodes.end(), armnodes.begin(), armnodes.end()); 
    }
    uint32_t armnum = 0, armnode = 0; 
    vector<float> previous; 
    vtkfile << str(boost::format("POINTS %d float") % nodes.size()) << endl;
    for (uint32_t point = 0; point < nodes.size(); point++) {
      if (armnode == numArmNodes[armnum]) {
        armnode = 0;         
        armnum++; 
      } 
      vector<float> xyz = nodes[point]->GetLocation(); 
      if (point > 0) {
        for (uint32_t i=0; i<3; i++) {
          if (xyz[i] - previous[i] > FullNode::mBoundsSize[i]/2){
            xyz[i] -= FullNode::mBoundsSize[i]; 
          } 
          else if (previous[i] - xyz[i] > FullNode::mBoundsSize[i]/2){
            xyz[i] += FullNode::mBoundsSize[i]; 
          } 
        }
      }
      armnode++; 
      previous = xyz; 
      string note = ""; 
      /*if (armnode == numArmNodes[armnum]) {
        note = str(boost::format("# End arm %1%")%armnum); 
        } */     
      vtkfile << str(boost::format("%1% %2% %3% %4%") % xyz[0] % xyz[1] % xyz[2] % note) << endl; 
    }  
    vtkfile << endl; 
  
    // next the lines
    int numlines = nodes.size() - arms.size(); 
    vtkfile << str(boost::format("LINES %d %d") % numlines % (3*numlines)) << endl;
    uint32_t currentIndex = 0; 
    for (uint32_t arm = 0; arm < arms.size(); arm++) {
      if (numArmNodes[arm]) {
        for (uint32_t nodenum = 0; nodenum < numArmNodes[arm]-1; nodenum++) {
          vtkfile << str(boost::format("2 %d %d")% currentIndex % (currentIndex+1)) << endl;
          currentIndex++; 
        }
        currentIndex ++; 
      }
    }
    vtkfile << endl << endl; 
  
    // next the arm numbers for each line
    vtkfile << str(boost::format("CELL_DATA %d") % numlines) << endl;
    vtkfile << "SCALARS armnum int" << endl;
    vtkfile << "LOOKUP_TABLE default" << endl;
    vtkfile.flush(); 
    for (uint32_t arm = 0; arm < arms.size(); arm++) {
      if (!numArmNodes[arm]) {
        continue;
      }
      uint32_t armnum = arms[arm]->mArmID;
      for (uint32_t nodenum = 0; nodenum < numArmNodes[arm]-1; nodenum++) {
        vtkfile << armnum << " " ; 
        vtkfile.flush(); 
      }        
    }
    vtkfile << endl << endl; 
  
    // next the burgers type for each line
    vtkfile << "SCALARS burgers_type int" << endl;
    vtkfile << "LOOKUP_TABLE default" << endl;
    for (uint32_t arm = 0; arm < arms.size(); arm++) {
      if (!numArmNodes[arm]) {
        continue;
      }
      uint32_t armtype = arms[arm]->GetBurgersType(); 
      for (uint32_t nodenum = 0; nodenum < numArmNodes[arm]-1; nodenum++) {
        vtkfile << armtype << " " ; 
        vtkfile.flush(); 
      }
    }
    vtkfile << endl << endl; 
    vtkfile.flush(); 
  
    // next the arm depths for each line
    vtkfile << "SCALARS BFS_depth int" << endl;
    vtkfile << "LOOKUP_TABLE default" << endl;
    for (uint32_t arm = 0; arm < arms.size(); arm++) {
      if (!numArmNodes[arm]) {
        continue;
      }
      for (uint32_t nodenum = 0; nodenum < numArmNodes[arm]-1; nodenum++) {
        vtkfile << armdepths[arm] << " " ; 
      }
    }
    vtkfile << endl << endl; 
  
    // next the "action" for each line
    vtkfile << "SCALARS action int" << endl;
    vtkfile << "LOOKUP_TABLE default" << endl;
    for (uint32_t arm = 0; arm < arms.size(); arm++) {
      if (!numArmNodes[arm]) {
        continue;
      }
      for (uint32_t nodenum = 0; nodenum < numArmNodes[arm]-1; nodenum++) {
        vtkfile << action[arm] << " " ; 
      }
    }
    vtkfile << endl << endl; 
    vtkfile.flush(); 
    vtkfile.close(); 
  
    msg = str(boost::format("*** Wrote VTK arm segment tracefile %s for %s\n")% filename % description);
    dbprintf(0, msg.c_str()); 
    cerr << msg; 
  
    // ===================================
    // NEXT, PRINT THE VTK FILE FOR ARM NODES
    filename = filebase+"-nodes.vtk";
    vtkfile.open(filename.c_str());
    if (!vtkfile) {
      string errmsg = str(boost::format("\n*** Warning:    cannot open vtk file %1%: %2%.")%filename%strerror(errno));
      cerr << errmsg << endl; 
      dbprintf(1, "%s\n", errmsg.c_str()); 
      return; 
    }
  
    // First, the header
    vtkfile << "# vtk DataFile Version 3.0" << endl;
    vtkfile << filename + ", written by paraDIS_lib" << endl;
    vtkfile << "ASCII" << endl;
    vtkfile << "DATASET POLYDATA" << endl;
    vtkfile.flush(); 
  
    // next, the points
    armnum = 0; armnode = 0; 
    vtkfile << str(boost::format("POINTS %d float") % nodes.size()) << endl;
    for (uint32_t point = 0; point < nodes.size(); point++) {
      if (armnode == numArmNodes[armnum]) {
        armnode = 0;         
        armnum++; 
      } 
      vector<float> xyz = nodes[point]->GetLocation(); 
      if (point > 0) {
        for (uint32_t i=0; i<3; i++) {
          if (xyz[i] - previous[i] > FullNode::mBoundsSize[i]/2){
            xyz[i] -= FullNode::mBoundsSize[i]; 
          } 
          else if (previous[i] - xyz[i] > FullNode::mBoundsSize[i]/2){
            xyz[i] += FullNode::mBoundsSize[i]; 
          } 
        }
      }
      armnode++; 
      previous = xyz; 
      string note = ""; 
      /*if (armnode == numArmNodes[armnum]) {
        note = str(boost::format("# End arm %1%")%armnum); 
        } */     
      vtkfile << str(boost::format("%1% %2% %3% %4%") % xyz[0] % xyz[1] % xyz[2] % note) << endl; 
    }  
    vtkfile << endl; 
  
    // now node vertices to allow node plotting
    vtkfile << str(boost::format("VERTICES %d %d") % nodes.size() % (2*nodes.size())) << endl;
    for (uint32_t point = 0; point < nodes.size(); point++) {
      vtkfile << "1 " << point << endl; 
      vtkfile.flush(); 
    } 
    vtkfile << endl; 
  
    // now node indices from analyzeParaDIS
    vtkfile << str(boost::format("CELL_DATA %d") % nodes.size()) << endl;
    vtkfile << "SCALARS node_index int" << endl;
    vtkfile << "LOOKUP_TABLE default" << endl;
    for (uint32_t point = 0; point < nodes.size(); point++) {
      uint32_t nodeID = nodes[point]->GetIndex(); 
      vtkfile <<  nodeID << " "; 
      vtkfile.flush(); 
    } 
    vtkfile << endl << endl; 
    vtkfile.close(); 
  
    msg = str(boost::format("*** Wrote VTK arm node tracefile %s for %s\n")% filename % description);
    dbprintf(0, msg.c_str()); 
    cerr << msg; 
    return; 
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
 
  //===========================================================================
  std::string Node::Stringify(int indent) const {
    std::string s = INDENT(indent) + std::string("(Node): ") + 
                  mID.Stringify(0) + string("\n");
    s += string("(node): In bounds: ") + (mInBounds?"true":"false");   
    return s; 
  }
  
  //===========================================================================
  bool FullNode::Test(void) {
    if (0 == mLocation[0] && mLocation[0] == mLocation[1] && mLocation[1] == mLocation[2]) {
      dbprintf(0, "FullNode::Test() WARNING: mLocation[0] == mLocation[1] == mLocation[2] == 0, node %s \n", Stringify(0).c_str()); 
    }
    if (!mNodeType) {
      dbprintf(0, "FullNode::Test() ERROR: mNodeType = %d, node %s\n", mNodeType, Stringify(0).c_str());
      return false; 
    }
    int segnum = 0; 
    for (vector<ArmSegment*>::iterator seg = mNeighborSegments.begin(); seg != mNeighborSegments.end(); ++seg, ++segnum) {
      if (!*seg) {
        dbprintf(0, "FullNode::Test() ERROR: NULL neighbor seg %d for node: %s\n", segnum, Stringify(0).c_str());
        return false; 
      }
      (*seg)->GetLength(); 
    }
    return true; 
  }
  
  //===========================================================================
  void FullNode::PrintAllNodeTraces(string stepname) {
    for (vector<uint32_t>::iterator nodeid = FullNode::mTraceNodes.begin(); 
         nodeid != mTraceNodes.end(); nodeid++) {
      if (FullNode::mFullNodes.find(*nodeid) == FullNode::mFullNodes.end()) {
         if (*nodeid < FullNode::mFullNodes.size()) {
           cerr << "Warning: cannot print trace for requested node " << *nodeid <<  "as the node with that ID has been deleted." << endl; 
         } 
         else {
           cerr << "Warning: cannot print trace for requested node " << *nodeid <<  "as it has not yet been created." << endl; 
         } 
      }
      else {         
        FullNode::mFullNodes[*nodeid] -> WriteTraceFiles(stepname); 
      }
    }
    return; 
  }
  //===========================================================================
  /* 
     For each node that we want to trace, we write one set of files out before any decompositions, then one for decomposition step of each arm containing this node.
   */ 
  void FullNode::WriteTraceFiles(string stepname) {
    dbprintf(5, "Node::WriteTraceFiles(node %d)\n", mNodeIndex); 
    string filebase = str(boost::format("%s-%s-Node_%d") % mTraceFileBasename % stepname % mNodeIndex); 
    vector<Arm *> arms; 
    vector<uint32_t> armdepths; 
    vector<int> action; 
    
    if (mNeighborArms.size() ) {
      for (vector<Arm*>::iterator arm = mNeighborArms.begin(); arm != mNeighborArms.end(); arm++) {
        dbprintf(2, "Node::WriteTraceFiles(node %d) adding BFS neighbors for arm %d\n", mNodeIndex, (*arm)->mArmID); 
        (*arm)->FindBFSNeighbors(arms, armdepths, action); 
      }
    } 
    else {
      if (!mNeighborSegments.size()) {
        dbprintf(0, "Warning: requested trace node %d has no neighbor segments and thus is not a member of any arms", mNodeIndex);; 
        return; 
      } 
      dbprintf(2, "Node::WriteTraceFiles(node %d) adding BFS neighbors for arm %d\n", mNodeIndex, mNeighborSegments[0]->mParentArm->mArmID);       
      mNeighborSegments[0]->mParentArm -> FindBFSNeighbors(arms, armdepths, action); 
    }
    string description = str(boost::format("node %1%")%mNodeIndex); 
    paraDIS::WriteTraceFiles(filebase, description, arms, armdepths, action); 
    return; 
  

  }
  //===========================================================================
  /*!
    Replace a neighbor of this node with a new neighbor -- done when wrapping nodes
  */ 
  void FullNode::ReplaceNeighbor(ArmSegment *oldseg,  ArmSegment *newseg) {
    int segnum = mNeighborSegments.size(); 
    while (segnum--) {
      if (oldseg == mNeighborSegments[segnum]) {
        mNeighborSegments[segnum] = newseg;
        return; 
      }
    }
    string err = str(boost::format("Error: segment %1%  to replace is not a neighbor of node %2%\n")%newseg->Stringify(0)%Stringify(0)); 
    throw err; 
  }
  
  //===========================================================================
  ArmSegment *FullNode::GetOtherNeighbor(const ArmSegment* n, bool usePointers) {
    if (mNeighborSegments.size() > 2) {
      throw string("GetOtherNeighbor called, but mNeighborSegments.size() is greater than 2: ")+Stringify(0); 
    }
    if (usePointers) {
      if (mNeighborSegments.size() != 2) {
        throw string("GetOtherNeighbor called with userPointers = true and mNeighborSegments.size() != 2"); 
      }
      if (mNeighborSegments[0] == n) return mNeighborSegments[1]; 
      if (mNeighborSegments[1] == n) return mNeighborSegments[0];
      return NULL; 
    }
    if (*mNeighborSegments[0] == *n) {
      if (mNeighborSegments.size() == 1) {
        return NULL; 
      }
      return mNeighborSegments[1];
    }
    // else mNeighborSegments[0] != n
    if (mNeighborSegments.size() == 1 || *mNeighborSegments[1] != *n) {
      throw string("GetOtherNeighbor: given segment is not a neighbor of this segment.  This: ")+this->Stringify(0)+", given: "+n->Stringify(0);
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
  std::string FullNode::Stringify(int indent, bool shortform) const {
    int ntype = mNodeType ;
    vector<int> idarray = GetNeighborArmIDs(); 
    string armids = arrayToString(idarray); 
    std::string s = INDENT(indent) + 
      str(boost::format("FullNode index %1%, %2%, type %3%, neighbor arms: %4%, %5% neighbor segments, location: (%6% %7% %8%), address %9%")
          % mNodeIndex % mID.Stringify(0) % ntype % armids % mNeighborSegments.size() % mLocation[0] % mLocation[1] % mLocation[2] % this); 
      s += "\n"; 
      s+= INDENT(indent) + "Neighbor segments:"; 

    if (shortform) {
      if (! mNeighborSegments.size()) {
        s += "<NONE>"; 
      } else {
       s += "<"; 
       uint32_t i=0; while (i < mNeighborSegments.size()) {
         if (mNeighborSegments[i]) {
           s += intToString(mNeighborSegments[i]->GetID()); 
         } else {
           s += "NULL"; 
         }
         if (i == mNeighborSegments.size()-1) {
           s+= ">\n"; 
         } else {
           s+= ", "; 
         }
         ++i; 
       }
      }
    } else {
       s += "\n"; 
     uint32_t i=0; while (i < mNeighborSegments.size()) {
        s += INDENT(indent) + "neighbor " + intToString(i) + ": "; 
        if (mNeighborSegments[i]) {
          s+= mNeighborSegments[i]->Stringify(indent+1);
        } else { 
          s += INDENT(indent+1) + "(NULL)\n"; 
        } 
        ++i; 
      }
    }
    return s; 
  }
  //===========================================================================
  void FullNode::ComputeNodeType(void) {
    /*!
      set my own type according to what I know -- this covers all nodes except butterfly ends. 
    */ 
    mNodeType = mNeighborSegments.size();  // true for vast majority of nodes. 
    
    if (mNodeType >= 4) {//four-armed is forewarned!  Oh, I'm funny. 
      /*!
        check for a monster or special monster 
      */ 
      int neighbor = mNodeType; 
      /*!
        btypes:  used to check for duplicates -- if any two arms are the same type, it's not a monster. There are only 8 types, but they are 1-based, so allocate 9 slots. 
      */           
      bool btypes[60] = {false}; 
      while (neighbor--) {
        const ArmSegment *theSegment = 
          dynamic_cast<const ArmSegment *>(mNeighborSegments[neighbor]);
        int8_t btype =  theSegment->GetBurgersType(); 
        if (btypes[btype] || btype < 0) {
          continue; //not a monster, we're done
        }
        btypes[btype] = true; 
        
      }
      /*! 
        We have four uniquely valued arms.  Are they of the right mix? 
      */  
      if (btypes[ BURGERS_PPP] && btypes[ BURGERS_PPM] && 
          btypes[ BURGERS_PMP] && btypes[ BURGERS_PMM]) {
        mNodeType = -mNodeType;  // FOUND MONSTER NODE
      } 
    }// end if four-armed or higher
 
    return; 
  }/* end SetNodeType */ 

  
  //===========================================================================
  const std::vector< int> FullNode::GetNeighborArmIDs(void) const {
    vector<int> ids; 
    vector<Arm*>::const_iterator armpos = mNeighborArms.begin(); 
    while (armpos != mNeighborArms.end()) {
      ids.push_back((*armpos)->mArmID); 
      ++armpos; 
    }
    return ids; 
  }
  

  //===========================================================================
  /*!
    Identify all non-loop crossing arms.  
    Crossing arms, when removed, must not cause the node to have less than 
    three neighbors.  
  */ 
  vector<vector<Arm *> >FullNode::IdentifyCrossArms(void) {
    vector <vector<Arm *> >crossarms;
    if (mNeighborSegments.size() < 4) 
      return crossarms; 

    vector<Arm*> armpair(2, (Arm*) NULL); 
    vector<bool> matched(mNeighborSegments.size(), false); 
    // first, identify looped cross arms
    for (uint32_t segnum = 0; segnum < mNeighborSegments.size()-1; segnum++) {
      if (!matched[segnum] &&
          mNeighborSegments[segnum]->mParentArm->mTerminalNodes.size() == 1) {
        dbprintf(4, str(boost::format("FullNode::IdentifyCrossArms(): node(%1%): found crossing loop arm %2%.\n") % GetNodeIDString() % mNeighborSegments[segnum]->mParentArm->mArmID).c_str()); 
        armpair[0] = NULL;
        for (uint32_t match = segnum+1; match < mNeighborSegments.size(); ++match) {
          if (!matched[match] && 
              mNeighborSegments[match]->mParentArm == mNeighborSegments[segnum]->mParentArm ) {
            matched[segnum] = true; 
            matched[match] = true; 
            armpair[0] = mNeighborSegments[segnum]->mParentArm; 
            armpair[0]->mExtendOrDetach = true; 
            crossarms.push_back(armpair); 
            if (mNeighborSegments.size()- 2*crossarms.size() < 4) 
              return crossarms; 
         }            
        }
        if (armpair[0] == NULL) {
          dbprintf(0, "************************************************************************************\n");
          dbprintf(0, str(boost::format("FullNode::IdentifyCrossArms(): node(%1%): ERROR! Could not find matching arm for crossing loop arm %2%.\n") % GetNodeIDString() % mNeighborSegments[segnum]->mParentArm->mArmID).c_str());
          dbprintf(0, "************************************************************************************\n");
        }
      }
    }
    
    
    // next, identify non-looped cross arms
    for (uint32_t segnum = 0; segnum < mNeighborSegments.size()-1; ++segnum) {
      int8_t btype = mNeighborSegments[segnum]->GetBurgersType(); 
      if (!matched[segnum]) {
        for (uint32_t match = segnum+1; match < mNeighborSegments.size(); ++match) {
          if (!matched[match] && 
              mNeighborSegments[match]->GetBurgersType() == btype) {
            matched[match] = matched[segnum] = true; 
            armpair[0] = mNeighborSegments[segnum]->mParentArm; 
            armpair[1] = mNeighborSegments[match]->mParentArm; 
            armpair[0]->mExtendOrDetach = armpair[1]->mExtendOrDetach = true; 
            crossarms.push_back(armpair); 
            if (mNeighborSegments.size()- 2*crossarms.size() < 4) 
              return crossarms; 
          }
        }/* match < mNeighborSegments.size(); */
      } /* (!matched[segnum]) */ 
    } /* segnum < mNeighborSegments.size()-1 */
    return crossarms; 
  }

  //===========================================================================
  /*!
    Identify arms which cross over this node and glue them together. 
    Simplifies decomposition of arms.  
    Unmarks cross arms after detaching them.  
  */ 
  void FullNode::DetachCrossArms(void) {
    
    vector <vector<Arm *> >crossarms = IdentifyCrossArms(); 
    for (uint32_t armpair = 0; armpair < crossarms.size(); ++armpair) {
      crossarms[armpair][0]->mExtendOrDetach = false; 
      if (crossarms[armpair][1] == NULL) {
        crossarms[armpair][0]->DetachLoopFromNode(this); 
      } else {
        crossarms[armpair][0]->DetachAndFuse(this, crossarms[armpair][1]);
        crossarms[armpair][1]->mExtendOrDetach = false; 
     } 
    }
    return ; 
  }
     
  //===========================================================================
  std::string ArmSegment::Stringify(int indent) const {
    string s = str(boost::format("ArmSegment at %1%")%this);  
    s +=  " number " + intToString(mSegmentID); 
    int btype = mBurgersType; 
    s += str(boost::format(", %1%, parentArm ") %BurgersTypeNames(btype)); 
    if (mParentArm) {
      s += str(boost::format("%1%\n") % mParentArm->mArmID);
    } else {
      s += "(NONE)\n"; 
    }
    uint32_t epnum = 0; 
    while (epnum < 2) {
      s+= INDENT(indent+1) + "ep "+intToString(epnum)+": "; 
      if (mEndpoints[epnum]) s+= mEndpoints[epnum]->Stringify(0); 
      else s+= "(NULL)"; 
      s+= "\n"; 
      epnum++; 
    }
    epnum = 0; 
    while (epnum < mGhostEndpoints.size()) {
      s+= INDENT(indent+1) + "GHOST ep "+intToString(epnum)+": ";
      if (mGhostEndpoints[epnum]) {
        s += mGhostEndpoints[epnum]->Stringify(0); 
      } else {
        s += "(NULL)";
      } 
      s+= "\n"; 
      ++epnum; 
    }
    return INDENT(indent) + s; 
  }

  //===========================================================================
  void ArmSegment::ComputeBurgersType(float burg[3]) {
    mBurgersType = InterpretBurgersType(burg); 
    return; 
  }
  
  //===========================================================================
  bool ArmSegment::Wrap(const rclib::Point<float> &dataSize, 
                        ArmSegment *&oNewSegment, 
                        FullNode *&oWrapped0, FullNode *&oWrapped1) {
    if (mBurgersType == BURGERS_DECOMPOSED) {
      // do not wrap these
      return false; 
    }

    float loc0[3], loc1[3]; 
    mEndpoints[0]->GetLocation(loc0), 
    mEndpoints[1]->GetLocation(loc1);     

    bool wrap = DataSet::Wrap(loc0, loc1); 

    if (!wrap) {
      oWrapped0 = NULL; oWrapped1 = NULL; oNewSegment = NULL; 
      dbprintf(5, "ArmSegment::Wrap: Not wrapped: %s\n", Stringify(0).c_str());
      mSegLen += GetLength(); 
      return false; 
    }
    
    dbprintf(5, "--------------------------------------------\n");
    dbprintf(5, "ArmSegment::Wrap: Before wrapping: %s\n", Stringify(0).c_str());
    mNumWrapped++; 
    oWrapped0 = new FullNode(*mEndpoints[0], true); // WHY? skip neighbors
    oWrapped1 = new FullNode(*mEndpoints[1], true); // WHY? skip neighbors
    
    oNewSegment = new ArmSegment(*this); 
    oNewSegment->mWrapped = true; 
    oWrapped0->SetLocation(loc0); 
    oWrapped1->SetLocation(loc1); 

    oWrapped0->SetInBounds(); 
    oWrapped1->SetInBounds(); 
    
    mEndpoints[1]->SetInBounds(); 
    mEndpoints[0]->SetInBounds(); 
    
    /*! 
      insert the new segment between endpoint 1 and the new node 0
    */ 
    oNewSegment->ReplaceEndpoint(mEndpoints[0], oWrapped0, true); 
    oWrapped0->AddNeighbor(oNewSegment);
    mEndpoints[1]->ReplaceNeighbor(this,oNewSegment); 
    

    /*!
      Insert *this between endpoint 0 and the new node 1
    */ 
    oWrapped1->AddNeighbor(this); 
    this->ReplaceEndpoint(mEndpoints[1], oWrapped1, true); 
        
    mParentArm->mNumWrappedSegments ++; 
    mParentArm->mNumSegments ++; 
    
    mSegLen += GetLength(); 
    dbprintf(5, "ArmSegment::Wrap: Wrapped: %s\n", Stringify(0).c_str()); 
    dbprintf(5, "--------------------------------------------\n");
    return true; 
  }


  //===========================================================================
  ArmSegment *ArmSegment::SwitchToWrappedDouble(FullNode *originalNode, FullNode **wrappedOriginal, FullNode **wrappedOtherEnd) const {
    //FullNode *originalNode = *wrappedNode, *otherNode = *wrappedGhost; 
    *wrappedOriginal = NULL;
    if (wrappedOtherEnd) {
      *wrappedOtherEnd = NULL; 
    }
    dbprintf(6, "ArmSegment::SwitchToWrappedDouble(segment %d): originalNode = %d\n", mSegmentID, originalNode->GetIndex()); 

    if (originalNode->GetNumNeighborSegments() != 1) {
      dbprintf(0, "SwitchToWrappedDouble(): ERROR:  wrappedNode should have only one neighbor but doesn't.\n"); 
      return NULL; 
    }
    if (!mGhostEndpoints.size()) {
      dbprintf(0, "SwitchToWrappedDouble(): ERROR:  no ghost endpoints in segment.\n"); 
      return NULL; 
    }
    if (! (*originalNode == *mGhostEndpoints[0])) {
      dbprintf(0, "SwitchToWrappedDouble(): ERROR: ghost node does not come from the given original node.\n"); 
      return NULL; 
    }
    *wrappedOriginal = mGhostEndpoints[0]; 
    FullNode *myOtherEnd = GetOtherEndpoint(originalNode);
    int ne = 0 ;
    int numGhostNeighbors = (*wrappedOriginal)->GetNumNeighborSegments(); 
    while (ne < numGhostNeighbors) {
      ArmSegment *neighbor = (*wrappedOriginal)->GetNeighborSegment(ne);   
      FullNode * neighborghost = neighbor->GetGhostEndpoint(); 
      if (neighborghost) {
        if (*neighborghost == *myOtherEnd) {
          if (neighbor->mParentArm != mParentArm) {
            dbprintf(5, "SwitchToWrappedDouble(): rejecting wrapped double as it has wrong parent arm: %s\n", neighbor->Stringify(0).c_str()); 
          }
          else {
            dbprintf(5, "SwitchToWrappedDouble(): found wrapped double: %s\n", neighbor->Stringify(0).c_str()); 
            if (wrappedOtherEnd) {
              *wrappedOtherEnd = neighbor->GetOtherEndpoint(*wrappedOriginal); 
            }
            return neighbor; 
          }
        }  
      }
      ++ne; 
    }
    dbprintf(0, "SwitchToWrappedDouble(): ERROR: could not find wrapped double in my ghost node's neighbors.\n");    
    return NULL; 
  }


 //===========================================================================
  int8_t ArmSegment::GetMNType(void) const { 
    return mParentArm->mArmType; 
  } 

 //===========================================================================
  uint32_t ArmSegment::GetArmID(void) {    
    return mParentArm->mArmID;
  }

  //===========================================================================
  uint32_t ArmSegment::GetMetaArmID(void) {
    if (!mParentArm) {
      dbprintf(0, "ERROR: GetMetaArmID() called on parentless segment.\n"); 
      return METAARM_UNKNOWN; 
    }
    return mParentArm->GetMetaArmID(); 
  }

  //===========================================================================
  uint8_t ArmSegment::GetMetaArmType(void) {
    if (!mParentArm) {
      dbprintf(0, "ERROR: GetMetaArmType() called on parentless segment.\n"); 
      return METAARM_UNKNOWN; 
    }
    return mParentArm->GetMetaArmType(); 
  }


  //===========================================================================
  /*!
     This prints out an arm and its neighboring arms using BFS order
     to the given depth to a text file and a VTK file.  
     File created: basename.txt, basename.vtk
   */ 
  void Arm::WriteTraceFiles(string stepname) {
    bool trace = false; 
    for (uint32_t n = 0; n<mTraceArms.size(); n++) {
      if (mArmID == mTraceArms[n]) 
        trace = true; 
    }
    if (!trace || !mTerminalNodes.size()) return; 

    dbprintf(5, "Arm::WriteTraceFiles(arm %d)\n", mArmID); 
    string filebase = str(boost::format("%s-%s-arm_%d") % mTraceFileBasename % stepname % mArmID); 
    
    vector<Arm *> arms; 
    vector<uint32_t> armdepths; 
    vector<int> action; 
    FindBFSNeighbors(arms, armdepths, action); 
    string description = str(boost::format("arm %1%")%mArmID); 
    paraDIS::WriteTraceFiles(filebase, description, arms, armdepths, action); 
    return; 
  }

  //===========================================================================
  void Arm::FindBFSNeighbors(vector<Arm *> &arms, vector<uint32_t> &armdepths, vector<int> &action) {
    arms.push_back(this); 
    armdepths.push_back(0);
    action.push_back(2); 
    
    // first, collect the list of arms in BFS order:
    uint32_t armnum = 0;
    uint32_t depth = 0; 
    while (depth++ < mTraceDepth && armnum < arms.size()) {
      uint32_t lastInLevel = arms.size(); 
      for (; armnum < lastInLevel; armnum++) {
       // append arm's neighbors onto arms
        Arm *arm = arms[armnum]; 
        arm->mSeen = true; 
        uint32_t numneighbors = arm->GetNumNeighborArms(); 
        dbprintf(5, "Arm::WriteTraceFiles(arm %d): arm %d, depth %d: looking at %d neighbor arms\n", mArmID, arm->mArmID, depth, numneighbors); 
       while (numneighbors--) {
          Arm *nei = arm->GetNeighborArm(numneighbors); 
          if (!nei->mSeen){
            arms.push_back(nei); 
            dbprintf(5, "Arm::WriteTraceFiles(arm %d): arm %d, depth %d: pushed back arm %s\n", mArmID, arm->mArmID, depth, nei->Stringify(0).c_str()); 
            armdepths.push_back(depth); 
            if (arm->mExtendOrDetach) {
              action.push_back(1); 
            } else {
              action.push_back(0); 
            }              
            nei->mSeen = true; 
          }
        }
      }
    } 
    for (uint32_t arm = 0; arm < arms.size(); arm++) {
      arms[arm]->mSeen = false; 
    }
    return ;
  }
    

  //===========================================================================
  /* Returns true if the arm has four unique type "111" burgers vectors represented in the exterior arms, but no type "200" burgers vectors types.  Assumes arm has at least one 3-armed endpoint, and the other must be either 3 or 5 armed. */
  /* Changes:  2012-06-08 Allow 5 armed endpoints to be included in the check.  But we still do not allow type 200 arms to be included.
   */ 
  bool Arm::HaveFourUniqueType111ExternalArms(void) {

    if (mTerminalNodes.size() != 2) return false;

    // enforce "at least one 3 armed endpoint" condition
    if (mTerminalNodes[0]->GetNumNeighborSegments() != 3 && mTerminalNodes[1]->GetNumNeighborSegments() != 3 ) return false;

    int btypes[13] = {false}; 
    int nodenum = 2; 
    while (nodenum--) {
      FullNode *thisNode = mTerminalNodes[nodenum]; 
      int neighbornum = thisNode->GetNumNeighborSegments(); 

      // enforce "endpoint must have 3 arms or 5 arms" condition. 
      if (neighbornum != 3 && neighbornum != 5) return false; 

      while (neighbornum--) {
        const ArmSegment *thisSegment = thisNode->GetNeighborSegment(neighbornum);
        /*!
          Only check exterior segments (those which don't belong to *this).
        */ 
        if (thisSegment == mTerminalSegments[0] || 
            thisSegment == mTerminalSegments[1]) continue; 

        int btype = thisSegment->GetBurgersType(); 
        // if (btype <= 3 /*  || btype == 8 || btypes[btype]*/ ) return false; 
        btypes[btype] = true; 
      }
    }
    
    /*! 
      At this point, have we seen four unique type 111 arms?
    */ 
    return btypes[4] && btypes[5] && btypes[6] && btypes[7];  
  }/* end HaveFourUniqueType111ExternalArms */ 

  //===========================================================================
  string Arm::StringifyExternalArms(int indent) const {
    if (mTerminalNodes.size() == 0) return "(no terminal nodes)";
    string s1 = INDENT(indent)+"<"; // , s2 = ", <"; 
    int nodenum = mTerminalNodes.size(); 
    while (nodenum--) {
      FullNode *thisNode = mTerminalNodes[nodenum]; 
      int neighbornum = thisNode->mNeighborArms.size(); 
      
      while (neighbornum--) {
        const Arm *arm = thisNode->GetNeighborArm(neighbornum);
        /*!
          Only check exterior segments (those which don't belong to *this).
        */ 
        if (arm == this) continue; 
         s1 += str(boost::format("%1% %2% ")
                   % arm->mArmID % BurgersTypeNames(arm->GetBurgersType())); 
      }
    }
    return s1 + ">"; 
  }
  
  //===========================================================================
  bool Arm::HaveTwoMatchingType111ExternalArms(void) {
    int btypes[13] = {false };
    if (mTerminalNodes.size() != 2) return false;
    int nodenum = 2; 
    while (nodenum--) {
      FullNode *thisNode = mTerminalNodes[nodenum]; 
      int neighbornum = thisNode->GetNumNeighborSegments(); 
      
      while (neighbornum--) {
        const ArmSegment *thisSegment = thisNode->GetNeighborSegment(neighbornum);
        /*!
          Only check exterior segments (those which don't belong to *this).
        */ 
        if (thisSegment == mTerminalSegments[0] || 
            thisSegment == mTerminalSegments[1]) continue; 
        
        int btype = thisSegment->GetBurgersType(); 
        if (btype > 3 && btype < 8 && btypes[btype]) {
          return true; 
        } 
        
        btypes[btype] = true; 
        
      }
    }
    
    /*! 
      At this point, we know that none of our 111 neighbors are matched to each other
    */ 
    return false;  
  }/* end  HaveTwoMatchingType111ExternalArms */ 
    

 //===========================================================================
  uint32_t Arm::GetMetaArmID(void) {
    if (!mParentMetaArm) {
      dbprintf(0, "Error: GetMetaArmID() called on parentless arm.\n");
      return METAARM_UNKNOWN; 
    }
    return mParentMetaArm->GetMetaArmID(); 
  }

 //===========================================================================
  uint8_t Arm::GetMetaArmType(void) {
    if (!mParentMetaArm) {
      dbprintf(0, "Error: GetMetaArmType() called on parentless arm.\n");
      return METAARM_UNKNOWN; 
    }
    return mParentMetaArm->GetMetaArmType(); 
  }

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
      int neighborNum = (*termNode)->GetNumNeighborSegments(); 
      if (!numTerminalNodeArms) {
        numTerminalNodeArms = neighborNum; 
      } else if (neighborNum != numTerminalNodeArms) {
        notPartOfLoop = true; 
        break; 
      }
      
      while (neighborNum --) {
        Arm *neighbor = (*termNode)->GetNeighborArm(neighborNum); 
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
  double Arm::ComputeLength(void) {
    if (!mNumSegments) {
      mArmLength = 0; 
    } 

    else {
      double savedLength = mArmLength; 
      mArmLength = 0; 
      vector<ArmSegment*> segments = GetSegments(); 
      vector<ArmSegment*>::iterator segpos = segments.begin(), endseg = segments.end(); 
      
      while (segpos != endseg) {
        mArmLength += (*segpos)->GetLength(true); 
        ++segpos; 
      }
      if (mArmLength > mLongestLength) mLongestLength = mArmLength; 
      if (fabs(1.0 - mArmLength/ savedLength) > 0.0001) {
        dbprintf(5, "ComputeLength(): arm %d computed different length %g than saved length %g\n", mArmID, mArmLength, savedLength); 
      } else {
        dbprintf(5, "ComputeLength(): arm %d has length %g and saved length %g\n", mArmID, mArmLength, savedLength);         
      }
    }
   return mArmLength; 
  }
  
   //===========================================================================
  vector<FullNode*>Arm::GetNodes(FullNode*startNode) const {
    vector<FullNode*> err; 
    // Find the start segment that matches the given start node:
    ArmSegment *startSegment = NULL;
    vector<FullNode*> nodes; 
    int segnum = mTerminalSegments.size(); 
    if (!segnum) return nodes; 

    if (!startNode) {
      startNode = mTerminalNodes[0];       
    }

    uint32_t startnodenum = 0; 
    for (startnodenum = 0; startnodenum < mTerminalNodes.size(); startnodenum++) {
      if (startNode == mTerminalNodes[startnodenum]) {
        break;
      }
    }
    if (startnodenum == mTerminalNodes.size()) {
      dbprintf(0, "ERROR: Arm::GetNodes(arm %d): start node is not in mTerminalNodes array\n\n", mArmID); 
      return nodes; 
    }
    
    FullNode *lastNode = mTerminalNodes[mTerminalNodes.size()-1-startnodenum]; 
      
    
    while (!startSegment && segnum--) {
      int ep = 2; 
      while (ep--) {
        if (*(mTerminalSegments[segnum]->GetEndpoint(ep)) == *startNode)  {
          startSegment = mTerminalSegments[segnum];    
          startNode = mTerminalSegments[segnum]->GetEndpoint(ep); // get rid of aliasing to avoid returning nodes that are not part of this arm but are wrapped
        }
      }
    }    
 
    if (!startSegment) {
      dbprintf(0, "Arm::GetNodes(arm %d): Cannot find matching terminal segment in arm for known good start node!\n", mArmID); 
      errexit1; 
    } 

    dbprintf(6, "Arm::GetNodes(arm %d): Found start segment %s and startNode %s\n", mArmID, startSegment->Stringify(0).c_str(), startNode->Stringify(0).c_str()); 
    if (startNode->GetNumNeighborSegments() == 1) {
      if (mDecomposing) {
        dbprintf(6, "Arm::GetNodes(arm %d): startNode has only one neighbor and we are decomposing the arm and there are no ghost endpoints, so this is assumed to be the result of decomposition and this is in fact a legitimate endpoint.\n", mArmID);
      } else {
        /* 
           This is a special case.  Here the initial segment is wrapped and we are starting on the non-wrapped side.  We do want to draw the wrapped double, so we will make our start node be the ghost instead. 
        */ 
        if (startSegment->mGhostEndpoints.size() != 1) {
          dbprintf(0, "Arm::GetNodes(arm %d): ERROR: We have an endpoint with only one neighbor on a segment with %d ghost nodes.\n"); 
          return nodes; 
        }
        
        startNode = startSegment->mGhostEndpoints[0]; 
        dbprintf(6, "Arm::GetNodes(arm %d): We had a node with only one neighbor as the startNode.  This would leave an undrawn segment, which is its ghost neighbor.  We must instead start with the ghost node of the start segment and corresponding parent segment and let them get wrapped naturally later.\n", mArmID);
        
        startSegment = NULL; 
        segnum = startNode->GetNumNeighborSegments(); 
        while (!startSegment && segnum--) {
          if (startNode->GetNeighborSegment(segnum)->mParentArm == this) 
            startSegment =  startNode->GetNeighborSegment(segnum);
          // do not switch node though.           
        }
        
        if (!startSegment) {
          dbprintf(0, "GetNodes(): Cannot find matching terminal segment in arm %d for known good start node!\n", mArmID); 
          errexit1; 
        }        
      }
    }
    
    if (mArmID == 538) {
      dbprintf(6, "Arm::GetNodes(arm %d): Arm is %s\n", mArmID, Stringify(0).c_str()); 
    }
    FullNode *currentNode = startNode; 
    nodes.push_back(currentNode); 
    dbprintf(6, "Arm::GetNodes(arm %d): pushed back startNode: %s\n", mArmID, currentNode->Stringify(0,true).c_str()); 
    int numghosts = 0; 
    const ArmSegment *currentSegment = startSegment; 
    while (true) {
      dbprintf(6, "Arm::GetNodes(arm %d): currentSegment: %s\n", mArmID, currentSegment->Stringify(0).c_str() ); 
      currentNode = currentSegment->GetOtherEndpoint(currentNode); 
      
      nodes.push_back(currentNode); 
      dbprintf(6, "Arm::GetNodes(arm %d): pushed back node %d: %s\n", mArmID, nodes.size(), currentNode->Stringify(0,true).c_str()); 
      
      if (currentNode->GetNumNeighborSegments() == 1) {
          
        FullNode *wrappedPrevious = NULL; 
        const ArmSegment *wDouble =  currentSegment->SwitchToWrappedDouble(currentNode, &currentNode, &wrappedPrevious); 
        if (!wDouble) {
          if (mDecomposing) {
            dbprintf(6, "Arm::GetNodes(arm %d): currentNode has only one neighbor and no matching ghost node from current Segment.  We are decomposing, so this is the end of the arm, we assume.\n", mArmID);
            break; 
          }
          dbprintf(0, "Arm::GetNodes(arm %d): ERROR:  We have a node with only one neighbor which is not a ghost node.  Things are going to get bad from here.\n", mArmID);           
        }
        nodes.push_back(NULL); 
        nodes.push_back(wrappedPrevious); 
        ++numghosts; 
        dbprintf(6, "Arm::GetNodes(arm %d): found a wrapped node.  Pushed back NULL %d followed by ghost node %d: %s.\n", mArmID, nodes.size()-1, nodes.size(), wrappedPrevious->Stringify(0).c_str() );
        currentSegment = wDouble; 
        dbprintf(6, "Arm::GetNodes(arm %d): New wrapped currentSegment: %s\n", mArmID, currentSegment->Stringify(0).c_str() ); 
        nodes.push_back(currentNode); 
        dbprintf(6, "Arm::GetNodes(arm %d): pushed back neighbor to ghost node %d: %s\n", mArmID, nodes.size(),  currentNode->Stringify(0,true).c_str());        
      }
      if (currentNode == startNode) {
        dbprintf(6, "Arm::GetNodes(arm %d): currentNode == startNode, we are at the end of the arm\n", mArmID); 
        break; 
      } 
      else if (currentNode == lastNode) {
        dbprintf(6, "Arm::GetNodes(arm %d): currentNode == lastNode, we are at the end of the arm\n", mArmID); 
        break; 
      } 
      else if (currentNode->GetNumNeighborSegments() > 2) {
        dbprintf(6, "Arm::GetNodes(arm %d): currentNode has %d neighbors; we are at the end of the arm\n", mArmID, currentNode->GetNumNeighborSegments()); 
        break; 
      } 
      currentSegment = currentNode->GetOtherNeighbor(currentSegment, true);  
    }
    dbprintf(5, "Arm::GetNodes(arm %d) got %d nodes\n\n", mArmID, nodes.size()); 
    if (nodes.size() != mNumSegments + 1 + mNumWrappedSegments*2) {
      dbecho(0, str(boost::format("ERROR: Arm::GetNodes(arm %d) got %d nodes but should have gotten %d\n\n")% mArmID% (nodes.size())% (mNumSegments+1 + mNumWrappedSegments*2))); 
    }
      
    return nodes;
  }

  //===========================================================================
  vector<ArmSegment*>Arm::GetSegments(FullNode *startNode) const {
    vector<ArmSegment*> err; 
    // Find the start segment that matches the given start node:
    ArmSegment *startSegment = NULL;
    vector<ArmSegment*> segments; 
    if (!mTerminalNodes.size()) return segments; 
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
      dbprintf(0, "Arm::GetSegments(%d): Cannot find matching terminal segment in arm for given start node %s\n", mArmID, startNode->GetNodeIDString().c_str()); 
      errexit1; 
    } 
    ArmSegment *lastSegment = startSegment; 
    if (mTerminalSegments.size() > 1)  
      lastSegment = mTerminalSegments[1-segnum]; 
    
    FullNode *currentNode = startNode; 
    
    dbprintf(6, "Arm::GetSegments(%d): startNode: %s\n", mArmID, startNode->Stringify(0,true).c_str()); 

    ArmSegment *currentSegment = startSegment; 
    while (true) {
      segments.push_back(currentSegment); 
      dbprintf(6, "Arm::GetSegments(%d): pushed back segment: %s\n", mArmID, currentSegment->Stringify(0).c_str() ); 
      if (currentSegment == lastSegment) {
        break; 
      }
      currentNode = currentSegment->GetOtherEndpoint(currentNode); 
      if (currentNode->GetNumNeighborSegments() == 1) {
        ArmSegment *wDouble =  currentSegment->SwitchToWrappedDouble(currentNode, &currentNode, NULL); 
        if (!wDouble) {
          dbprintf(0, "Arm::GetSegments(%d): ERROR:  We have a node with only one neighbor which is not a ghost node.  Things are going to get bad from here.\n"); 
        }
        dbprintf(6, "Arm::GetSegments(%d): found a wrapped node and successfully switched to new node.\n", mArmID); 
        segments.push_back(wDouble); 
        currentSegment = wDouble; 
      }
      
      dbprintf(6, "Arm::GetSegments(%d): next node: %s\n", mArmID, currentNode->Stringify(0,true).c_str()); 

      if (currentNode->GetNumNeighborSegments() != 2 || currentNode == startNode) {
        break; 
      }
      currentSegment = currentNode->GetOtherNeighbor(currentSegment, true);       
    } 
    dbprintf(5, "Arm::GetSegments(%d): Found %d segments\n\n", mArmID, segments.size()); 
    return segments;
  }

  //===========================================================================
  void Arm::Classify(void) {
#ifdef RC_CPP_VISIT_BUILD
    int err = -1; (void) err;
#endif 
    dbprintf(5, "Arm::Classify(%d) called for arm %s\n", mArmID, Stringify(0).c_str()); 
#if LINKED_LOOPS
    CheckForLinkedLoops(); 
#endif
    if (!mNumSegments) {
      dbprintf(5, "Arm::Classify(%d): no segments in arm.\n", mArmID); 
      return; 
    }
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
            
      // This changes _111 to _200 by definition
      int btype = mTerminalSegments[0]->GetBurgersType(); 
      if (btype == BURGERS_NONE || btype == BURGERS_UNKNOWN) {
        mArmType = ARM_UNKNOWN; 
      }
      else if (btype != BURGERS_PPP && btype != BURGERS_PPM && 
               btype != BURGERS_PMP && btype != BURGERS_PMM) {
        dbprintf(0, "Error:  All non-loop arms should be type 111 now.\n"); 
        errexit; 
      }
    }
    
    if (mThreshold > 0 && mArmLength < mThreshold) {
      if (mArmType == ARM_NN_111) mArmType = ARM_SHORT_NN_111; 
    }
    dbprintf(5, "Arm::Classify(%d): gave arm type %d.\n", mArmID, mArmType); 
     return; 
  }
  
  //===========================================================================
  void Arm::DetachAndReplaceNode(FullNode* node, FullNode *replacement) {
    node->RemoveNeighbor(this, true); 
    replacement->AddNeighbor(this); 
    int segnum = mTerminalSegments.size(); 
    while(segnum--) {
      ArmSegment *segment = mTerminalSegments[segnum]; 
      if (segment->HasEndpoint(node)) {
        node->RemoveNeighbor(segment, true); 
        segment->ReplaceEndpoint(node, replacement, false); 
        replacement->AddNeighbor(segment); 
      }
    }
    return;
  }
  
  //===========================================================================
  void Arm::DetachLoopFromNode(FullNode *node) {
    dbprintf(4, str(boost::format("Arm::DetachLoopFromNode(arm %1%, node %2%)\n")%mArmID% node->GetNodeIDString()).c_str());

    if (node->GetNumNeighborArms() == 1) {
      dbprintf(4, "Arm::DetachLoopFromNode(arm %d): the node to detach from only has us as a neighbor -- not detaching \n", mArmID); 
      return;
    }
    
    dbprintf(5, str(boost::format("Arm::DetachAndFuse(arm %1%): Before detaching, the looped arm looks like this: %2%\n") % mArmID % Stringify(0, false)).c_str());    
    FullNode *replacement = new FullNode(*node, true); // skip neighbors   
    dbprintf(4, "Arm::DetachLoopFromNode(arm %d): Created a duplicate node to serve as a new interior node for the fused arm. \n", mArmID); 
    

    DetachAndReplaceNode(node, replacement); 
    mTerminalNodes[0] = replacement; 
    dbprintf(5, str(boost::format("Arm::DetachAndFuse(arm %1%): After detaching, the looped arm looks like this: %2%\n") % mArmID % Stringify(0, false)).c_str());

    return; 
  }

  //===========================================================================
  void Arm::DetachAndFuse(FullNode *node, Arm *other) {
    dbprintf(4, str(boost::format("Arm::DetachAndFuse(arm %1%, node %2%, other %3%)\n")% mArmID% node->GetNodeIDString()% other->mArmID).c_str());

    dbprintf(5, str(boost::format("Arm::DetachAndFuse(arm %1%): Before fusing, this arm looks like this: %2%\n") % mArmID % Stringify(0, false)).c_str());
    dbprintf(5, str(boost::format("Arm::DetachAndFuse(arm %1%): Before fusing, the other arm looks like this: %2%\n") % mArmID % other->Stringify(0, false)).c_str());

    // OK, now we just have to absorb the other arm: 
    vector<ArmSegment*> segs = other->GetSegments(node); 
    int snum = segs.size(); 
    while (snum--) {
      segs[snum]->mParentArm = this; 
    }

    int nodenum = 2; 
    while (nodenum--) {
      if (mTerminalNodes[nodenum] == node) {
        break;
      } 
    } 
    int othernodenum = 2; 
    while (othernodenum-- ) {
      if (other->mTerminalNodes[othernodenum] == node) {
        break;
      }
    }

    if (nodenum < 0) {
      dbprintf(0, str(boost::format("Arm::DetachAndFuse(arm %1%): ERROR: cannot find terminal node that matches the node %2% this is supposed to detach from!\n")% mArmID % node->GetNodeIDString()).c_str()); 
      return; 
    }
    if (othernodenum < 0) {
       dbprintf(0, str(boost::format("Arm::DetachAndFuse(arm %1%): ERROR: cannot find terminal node in other arm %2% that matches the node %3% it is supposed to detach from!\n")% mArmID % other->mArmID % node->GetNodeIDString()).c_str());  
      return; 
    }
    // now find the associated  terminal segments
    int segnum =  mTerminalSegments.size() ;
    while (segnum-- ) {
      if (mTerminalSegments[segnum]->HasEndpoint(node)) {
        break; 
      }
    }
    int othersegnum = other->mTerminalSegments.size(); 
    while (othersegnum-- ) {
      if (other->mTerminalSegments[othersegnum]->HasEndpoint(node)) {
        break; 
      }
    }
    if (segnum < 0) {
      dbprintf(0, str(boost::format("Arm::DetachAndFuse(arm %1%): ERROR: cannot find terminal segment that has the node %2% this is supposed to detach from!\n")% mArmID % node->GetNodeIDString()).c_str()); 
      return; 
    }
    if (othersegnum < 0) {
       dbprintf(0, str(boost::format("Arm::DetachAndFuse(arm %1%): ERROR: cannot find terminal segment in other arm %2% that has the node %3% it is supposed to detach from!\n")% mArmID % other->mArmID % node->GetNodeIDString()).c_str());  
      return; 
    }
          
    FullNode *discrete = NULL; 
    if (node->GetNumNeighborSegments() > 2) {      
      discrete = new FullNode(*node, true); // skip neighbors   
      dbprintf(4, "Arm::DetachAndFuse(arm %d): Node %s is not an interior node; created a duplicate node to serve as a new interior node for the fused arm. \n", mArmID, node->GetNodeIDString().c_str()); 
      DetachAndReplaceNode(node, discrete); 
      other->DetachAndReplaceNode(node,discrete);  
    }  else {
      dbprintf(4, "Arm::DetachAndFuse(arm %d): Node %s is an interior node; remove 'other' as node neighbor only. \n", mArmID, node->GetNodeIDString().c_str()); 
      node->RemoveNeighbor(other); 
    }
    
    ArmSegment *replacementSeg = other->mTerminalSegments[othersegnum];
    if (other->mTerminalSegments.size() == 2) {
      replacementSeg = other->mTerminalSegments[1-othersegnum];
    }  
    if (mTerminalSegments.size() == 2) {
      mTerminalSegments[segnum] = replacementSeg; 
    } else {
      mTerminalSegments.push_back(replacementSeg); 
    }

    other->mTerminalSegments.clear(); 

    mTerminalNodes[nodenum] = other->mTerminalNodes[1-othernodenum];
    mTerminalNodes[nodenum]->RemoveNeighbor(other); 
    mTerminalNodes[nodenum]->AddNeighbor(this); 
    
    if (mTerminalNodes.size() == 2 && mTerminalNodes[0] == mTerminalNodes[1]) {
      dbprintf(4, "Arm::DetachAndFuse(arm %d): After fusing, this arm has two identical terminal nodes, and therefore is a loop.  Need to remove one terminal node. \n", mArmID); 
      mTerminalNodes.pop_back(); 
    }
     
    other->mTerminalNodes.clear(); 
    
    MakeAncestor(other); 

    mArmLength += other->mArmLength; 
    other->mArmLength = 0; 

    mNumSegments += other->mNumSegments; 
    other->mNumSegments = 0; 
    other->mNumWrappedSegments = 0; 

    other->mArmType = ARM_EMPTY;

    mNumDestroyedInDetachment++; 
    
    dbprintf(5, str(boost::format("Arm::DetachAndFuse(arm %1%): After fusing, this arm looks like this: %2%\n") % mArmID % Stringify(0, false)).c_str());
    dbprintf(5, str(boost::format("Arm::DetachAndFuse(arm %1%): After fusing, the other destroyed arm looks like this: %2%\n") % mArmID % other->Stringify(0, false)).c_str());
    return; 
  }

  //===========================================================================
  void Arm::ExtendBySegments(Arm *sourceArm, vector<ArmSegment*> &sourceSegments, FullNode *sharedNode, int numDuplicates) {

    int sharedNodeNum = mTerminalNodes.size(); 
    while (sharedNodeNum-- ) {
      if (mTerminalNodes[sharedNodeNum] == sharedNode) break; 
    }
    if (sharedNodeNum == -1) {
      dbprintf(0, "ExtendBySegments(%d): Error:  cannot find shared terminal node for extended arm!\n", mArmID);
      errexit; 
    }    
    int sourceSharedNodeNum = sourceArm->mTerminalNodes.size(); 
    while (sourceSharedNodeNum-- ) {
      if (sourceArm->mTerminalNodes[sourceSharedNodeNum] == sharedNode) break; 
    }
    if (sourceSharedNodeNum == -1) {
      dbprintf(0, "ExtendBySegments(%d): Error:  cannot find shared terminal node for source arm!\n", mArmID);
      errexit; 
    }   
    int sourceNonSharedNum = -1; 
    if (sourceArm->mTerminalNodes.size() == 2) {
      sourceNonSharedNum = 1-sourceSharedNodeNum; 
    }    
    FullNode *sourceNonSharedNode = NULL; 
    if (sourceNonSharedNum != -1) {
      sourceNonSharedNode = sourceArm->mTerminalNodes[sourceNonSharedNum];
    }

   /*!
      Duplicate the shared node in all respects, except without neighbors 
    */ 
    FullNode *newNode = new FullNode(*sharedNode, true); 

    /*
      Find the terminal segment that has the shared node as an endpoint. 
      For loops, this would be both, so just work on the first one here.  
      Use our new interior node as its endpoint, replacing the shared node. 
    */ 
    int sharedSegmentNum = mTerminalSegments.size(); 
    while(sharedSegmentNum--) {
      if (mTerminalSegments[sharedSegmentNum]->HasEndpoint(sharedNode)) {
        sharedNode->RemoveNeighbor(mTerminalSegments[sharedSegmentNum]); 
        mTerminalSegments[sharedSegmentNum]->ReplaceEndpoint(sharedNode, newNode, false);
        newNode->AddNeighbor(mTerminalSegments[sharedSegmentNum]);
        dbprintf(6, "ExtendBySegments(%d): Replaced the sharedNode %d as endpoint of terminal segment %d with newNode %d\n", 
                 mArmID, sharedNode->GetIndex(), mTerminalSegments[sharedSegmentNum]->mSegmentID, newNode->GetIndex()); 
       break; 
      }
    }
    if (sharedSegmentNum == -1) {
      dbprintf(0, "ExtendBySegments(%d): Error:  could not find terminal segment that has the old shared node as an endpoint.\n", mArmID);
      errexit; 
    }
    if (mTerminalSegments.size() == 1) {
      // mTerminalSegments[0] is going to get replace later and we will then have two segments -- unless we form a loop. 
      mTerminalSegments.push_back(mTerminalSegments[0]); 
    }

    ArmSegment *interiorSegment = NULL;

    int btype = GetBurgersType(); 
    FullNode *sourceNode = sharedNode; // for iterating through source arm 
    dbprintf(6, "ExtendBySegments(%d): iterating through %d source segments\n", mArmID, sourceSegments.size()); 
    uint32_t seg = 0; 
    while (seg < sourceSegments.size()) {
      sourceNode = sourceSegments[seg]->GetOtherEndpoint(sourceNode);
      
      FullNode *previousNewNode = newNode; 
      if (seg == sourceSegments.size()-1) {
        newNode = sourceNonSharedNode;
        if (!newNode) newNode = sharedNode; // source is a loop

        dbprintf(6, "ExtendBySegments(%d): Arm segment #%d (last segment): Using source arm's nonshared terminal node %d as newNode.\n", mArmID, seg, newNode->GetIndex()); 
      } else {
        newNode = new FullNode(*sourceNode, true); 
         dbprintf(6, "ExtendBySegments(%d): Arm segment #%d: Created newNode %d by copying source node %d.\n", mArmID, seg, newNode->GetIndex(), sourceNode->GetIndex()); 
      }
      interiorSegment = new ArmSegment(*sourceSegments[seg]); 
      dbprintf(6, "ExtendBySegments(%d): Setting source segment %d as BURGERS_DECOMPOSED.\n", mArmID, sourceSegments[seg]->mSegmentID); 
      interiorSegment->mParentArm = this; 
      interiorSegment->mNumDuplicates = sourceSegments[seg]->mNumDuplicates + numDuplicates; 
      interiorSegment->SetBurgersType(btype); 
      interiorSegment->SetEndpoints(previousNewNode, newNode); 
      previousNewNode->AddNeighbor(interiorSegment); 
      newNode->AddNeighbor(interiorSegment); 
      ++mNumSegments; 
      mArmLength += interiorSegment->GetLength(true); 
      dbprintf(6, "ExtendBySegments(%d): Arm segment #%d: created segment %d by copying source segment %d, resulting in: %s\n", mArmID, seg, interiorSegment->mSegmentID, sourceSegments[seg]->mSegmentID, interiorSegment->Stringify(0).c_str()); 
      dbprintf(6, "ExtendBySegments(%d): Arm segment #%d: Added newNode: %s\n", mArmID, seg, newNode->Stringify(0).c_str()); 
      
      ++ seg; 
  
    }
    mTerminalSegments[sharedSegmentNum] = interiorSegment; 
    newNode->AddNeighbor(this); 

    mTerminalNodes[sharedNodeNum] = newNode;
    mNumWrappedSegments += sourceArm->mNumWrappedSegments; 

    dbprintf(5, "\nExtendBySegments(%d): After extension the arm looks like this: %s", mArmID, Stringify(0,false).c_str()); 
    return; 
  }

  //===========================================================================
  void Arm::ExtendByArm(Arm *sourceArm, vector<ArmSegment*> &sourceSegments, FullNode *sharedNode, int numDuplicates) {
    // identify the shared terminal node in the neighbor arm:     
    dbprintf(5, "\n======================================\n   ExtendByArm(): Extending arm: %s\n", Stringify(0, false).c_str()); 

    bool isLoop = false;  
    ArmSegment * otherSharedSegment = NULL; 
    if (mTerminalNodes.size() == 1) {
      isLoop = true; 
      if (mTerminalSegments.size() == 1) {
        dbprintf(4, "ExtendByArm(%d): Looped arm with only one terminal segment.  We need to duplicate our terminal segment so we can \"double extend\" the arm.\n", mArmID); 
        // we need to find our other terminal segment so we can "double extend"
        vector<ArmSegment *> segments = GetSegments(sharedNode); 
        otherSharedSegment = segments[segments.size()-1]; 
        if (otherSharedSegment == mTerminalSegments[0]) {
          //dbprintf(5, "ExtendByArm(%d): Changing otherSharedSegment to  mTerminalSegments[0]\n", mArmID); 
          otherSharedSegment = segments[0]; 
        }
        if (!otherSharedSegment->HasEndpoint(sharedNode)) {
          dbprintf(0, "ExtendByArm(%d): Error:  found a looped arm where one of the terminal segments does not have the shared node as an endpoint.\n", mArmID);
          errexit; 
        }
        mTerminalSegments.push_back(otherSharedSegment); 
        //dbprintf(5, "ExtendByArm(%d): After pushing back otherSharedSegment, we look like this: %s\n", mArmID, Stringify(0, mArmID==130704).c_str()); 
      }
      dbprintf(4, "ExtendByArm(%d): We are extending a looped arm, so have to duplicate our terminal node for the algorithm to proceed correctly.\n", mArmID);
      mTerminalNodes.push_back(sharedNode); 
    }
     
    /*if (mNumSegments == 1 && mTerminalSegments.size() == 1) {
      dbprintf(5, "ExtendByArm(%d): Single segment arm:  duplicate our terminal segment.\n", mArmID); 
      mTerminalSegments.push_back(mTerminalSegments[0]); 
      }*/ 

    if (/* mTerminalSegments.size() != 2 || */ mTerminalNodes.size() != 2) {
      dbprintf(0, "ExtendByArm(%d): Error: arm with a single terminal node should not be possible at this point.\n", mArmID);  
      errexit; 
    }

    bool sourceIsLoop = (sourceArm->mTerminalNodes.size() == 1);
    if (sourceIsLoop) {
      dbprintf(0, "ExtendByArm(%d): ERROR: Source should never be a loop\n", mArmID); 
      errexit; 
    }      
    if (!isLoop /* && !sourceIsLoop */ ) {
      dbprintf(5, "ExtendByArm(%d): CASE 1: no loops: extend by source once.\n", mArmID); 
      ExtendBySegments(sourceArm, sourceSegments, sharedNode, numDuplicates); 
    } 
    else /* if (isLoop && !sourceIsLoop) */  {
      dbprintf(5, "ExtendByArm(%d): CASE 2: Extending a loop by a non-loop: extend by source twice.\n", mArmID); 
      ExtendBySegments(sourceArm, sourceSegments, sharedNode, numDuplicates); 
      ExtendBySegments(sourceArm, sourceSegments, sharedNode, numDuplicates); 
    }   

    sharedNode->RemoveNeighbor(this); 

    if (mTerminalNodes.size() == 2 && mTerminalNodes[0] == mTerminalNodes[1]) {
      dbprintf(5, "ExtendByArm(%d): After extending the arm it now forms a loop.  Consolidating terminal nodes.\n", mArmID); 
      mTerminalNodes.erase(++mTerminalNodes.begin(), mTerminalNodes.end()); 
    }

    MakeAncestor(sourceArm); 
    
    dbprintf(5, "\nExtendByArm(%d): After extension the source arm looks like this: %s", sourceArm->mArmID, sourceArm->Stringify(0,false).c_str()); 
    dbprintf(5, "\nExtendByArm(%d): After extension the extended arm looks like this: %s", mArmID, Stringify(0,false).c_str()); 
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

    if (energy == 1) {
      WriteTraceFiles("0-low-energy-no-decomposition"); 
      return false; 
    }

    mDecomposing = true; 
    
    // Find which terminal node to use in decomposing ourself.  
    // We prefer the terminal node that decomposes us into the smallest number of arms. 
    // This will be the one with the least number of neighbors.  If two terminal nodes have the same number of neighbors, use the one that has the lowest maximum energy level.  
    // I believe this greedy algorithm results in the global minimum decomposition too.  

    uint32_t numTermNodes = mTerminalNodes.size();
    vector<int> maxEnergies; // we'll analyze this later. 
    vector<int> numneighbors; 
    vector<int> extendedArmIDs; 

    if (numTermNodes == 0 || numTermNodes == 1) {
      dbprintf(4, "Arm::Decompose(arm %d): Looped arm, will not decompose\n", mArmID); 
      WriteTraceFiles("0-loop-no-decomposition"); 
      return false;  
    }

    vector<int> allNeighborArmIDs; 
    dbprintf(5, "\n================================================================\n Arm::Decompose(arm %d): Found arm : %s\n", mArmID, Stringify(0, false).c_str());

    // First, mark cross arms for debug output
    mTerminalNodes[0]->IdentifyCrossArms(); 
    if (mTerminalNodes.size() == 2) {
      mTerminalNodes[1]->IdentifyCrossArms();
    }

    WriteTraceFiles("1-before-detachment"); 
      
    // Actually perform detachment
    for (uint32_t n = 0; n<mTerminalNodes.size(); n++) {
      mTerminalNodes[n]->DetachCrossArms(); 
    }

    // Note that cross-arm detachment might cause this arm to become invalid.  
    if (!mTerminalNodes.size()) {
       dbprintf(4, "Arm::Decompose(arm %d): This arm was eliminated by DetachCrossArms, cannot decompose.\n", mArmID); 
       return false; 
    }     

    WriteTraceFiles("2-before-decomposition"); 
    int sharedNodeNum = -1; 
    for (uint32_t termnode = 0; termnode < numTermNodes; termnode++) {
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
        ++neighbor; 
      }
      dbprintf(5, "Arm::Decompose(arm %d): numneighbors[%d] = %d, maxEnergies[%d] = %d\n", mArmID, termnode, numneighbors[termnode], termnode, maxEnergies[termnode]); 
    }
    if (numneighbors[0] < numneighbors[1]) {
      dbprintf(5, "Arm::Decompose(arm %d): Choosing terminal node 0 because it has fewer neighbors.\n", mArmID);
      sharedNodeNum = 0; 
    } else if  (numneighbors[1] < numneighbors[0]) {
      dbprintf(5, "Arm::Decompose(arm %d): Choosing terminal node 1 because it has fewer neighbors.\n", mArmID);
      sharedNodeNum = 1; 
    } else {
      // have to look at max energy levels now.  :-( 
      if (maxEnergies[0] < maxEnergies[1]) {
        dbprintf(5, "Arm::Decompose(arm %d): Choosing terminal node 0 because it has lower max energy.\n", mArmID);
        sharedNodeNum = 0;
      } else {
        dbprintf(5, "Arm::Decompose(arm %d): Choosing terminal node 1 because it has lower max energy.\n", mArmID);
        sharedNodeNum = 1;
      }      
    }
    //dbprintf(5, "mTerminalNodes[%d] is %s\n",sharedNodeNum,  mTerminalNodes[sharedNodeNum]->Stringify(0).c_str()); 
    FullNode *sharedNode = mTerminalNodes[sharedNodeNum];

   /* 
       Extend all neighbor arms along our length.  
       Formerly terminal nodes will become internal to multiple arms.  
    */ 
    // copy our neighbor vector before it changes... 
    vector <Arm*> neighborArms = sharedNode->mNeighborArms; 

    // uniquify list, to prevent arms from being extended twice during decomposition, leading to invalid states
    sort(neighborArms.begin(), neighborArms.end());
    neighborArms.erase(unique(neighborArms.begin(), neighborArms.end()), neighborArms.end()); 
    int neighbornum = neighborArms.size(), numDuplicates = 0; 
    while (neighbornum--) {
      Arm *neighborArm = neighborArms[neighbornum]; 
      if (neighborArm != this) { 
        numDuplicates++; 
        if (neighborArm->mTerminalNodes.size() == 1) {
          numDuplicates++; // loops get double-copied
        }
      }
    }
    double decomposedLength = 0; 
    neighbornum = neighborArms.size();
    vector<ArmSegment*> sourceSegments = this->GetSegments(sharedNode); 
    uint32_t decomposed = 0;
    while (neighbornum--) {
      Arm *neighborArm = neighborArms[neighbornum]; 
      if (neighborArm != this) { 
        dbprintf(5, "Arm::Decompose(arm %d): Adding %d segments from self to arm %d.\n", mArmID, sourceSegments.size(), neighborArm->mArmID, Stringify(0).c_str());
        neighborArm->ExtendByArm(this, sourceSegments, sharedNode, numDuplicates);
        decomposedLength += mArmLength; 
        extendedArmIDs.push_back(neighborArm->mArmID); 
        neighborArm->mExtendOrDetach = true; 
        WriteTraceFiles(str(boost::format("%d-after-decompose-%d_ID-%d")%(2+decomposed)%decomposed%neighborArm->mArmID));  
        neighborArm->mExtendOrDetach = false; 
        ++decomposed; 
      }
    }
    if (decomposedLength > 0) {
      decomposedLength -= mArmLength; // because we delete ourself once
    }
    mDecomposedLength += decomposedLength; 
    vector<FullNode *> deleteme; 
    for (vector<ArmSegment*> ::iterator segpos =  sourceSegments.begin(); segpos != sourceSegments.end(); segpos++) {
      for (int i=0; i<2; i++) {
        FullNode *ep = (*segpos)->GetEndpoint(i); 
        if (ep && ep->GetNumNeighborSegments() < 3) {
          deleteme.push_back(ep); 
        }
      }
      delete (*segpos);
    }
    sort(deleteme.begin(), deleteme.end()); 
    vector<FullNode *>::iterator endpos = unique(deleteme.begin(), deleteme.end()); 
    for (vector<FullNode *>::iterator node = deleteme.begin(); node != endpos; node++) {
      delete *node; 
    }
    
   /*
      Now remove our terminal segments from our terminal nodes' neighbor lists. 
    */     
    int termNodeNum = mTerminalNodes.size();     
    while (termNodeNum--) {
      int termSegmentNum = mTerminalSegments.size(); 
      while (termSegmentNum--) {
        mTerminalNodes[termNodeNum]->RemoveNeighbor(mTerminalSegments[termSegmentNum], true); 
      }
      mTerminalNodes[termNodeNum]->RemoveNeighbor(this, true); 
    }
    mTerminalNodes.clear(); 
    mTerminalSegments.clear(); 
    mArmLength = 0; 
    mNumSegments = 0; 
    mNumWrappedSegments = 0; 
    mArmType = ARM_EMPTY;

    mNumDecomposed[energy]++; 
    dbprintf(4, "Arm::Decompose(arm %d): Arm decomposition complete. After arm decomposition, the arm looks like this: %s", mArmID, Stringify(0,false).c_str()); 
    return true;    
  }
  
  //===========================================================================
  std::string Arm::Stringify(int indent, bool shortform) const {
    string seen = "false"; 
    if (mSeen) seen = "true"; 
    string seenMeta = "false"; 
    if (mSeenInMeta) seenMeta = "true"; 
    int btype = GetBurgersType();
    int atype = mArmType; 
    string btypestring = BurgersTypeNames(GetBurgersType()); 
    string armtypestring = ArmTypeNames(mArmType);
    std::string s  = INDENT(indent) + 
      str(boost::format("(arm): number %1%, mSeen = %2%, mSeenInMeta = %3%, numSegments = %4%, numWrappedSegments = %13%, length = %5%, Burgers = %6% (%7%), numTermNodes = %8%, Type = %9% (%10%), ExternalArms = %11%, AncestorArms = %12%") 
          % mArmID % seen % seenMeta % mNumSegments % GetLength() % btype % btypestring % mTerminalNodes.size()  % atype % armtypestring  % StringifyExternalArms(0) % arrayToString(mAncestorArms) % ((int)mNumWrappedSegments)); 

#if LINKED_LOOPS
    if (mPartOfLinkedLoop) {
      s += ", is part of linked loop."; 
    } else {
      s += ", is NOT part of linked loop."; 
    }
#else 
    s += "\n";
#endif 

    if (shortform) 
      return s; 

    int num = 0, max = mTerminalNodes.size(); 
    while (num < max) {
      s+= INDENT(indent+1) + "Terminal Node " + intToString(num) + string(": ");
      if (mTerminalNodes[num]) {
        s += mTerminalNodes[num]->Stringify(0) + string("\n"); 
      } else {
        s += "(NULL)\n"; //deleted because it was useless.  
      }
      ++num; 
    }
    max = mTerminalSegments.size(); num = 0; 
    while (num < max) {
      s+= INDENT(indent+1) + "Terminal Segment " + intToString(num) + string(": "); 
      if (mTerminalSegments[num]) {
        s += mTerminalSegments[num]->Stringify(indent + 1) + string("\n"); 
      } else {
        s+= "(NULL)\n"; 
      }
      ++num; 
    }
    //s += StringifyExternalArmTypes(indent+1); 
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
#ifndef  USE_ABORT
    bool err = false; 
#endif
 
    dbprintf(4, "\n-------------------------------------------  \n");
    dbprintf(4, "FindEndpoint(metaarm %d): seed = %d, previous node = %d, candidate = %d\n", mMetaArmID, seed->mArmID, previous->GetIndex(), candidate->mArmID); 

    if (candidate->mArmType == ARM_EMPTY) {
      dbprintf(4, "FindEndpoint(metaarm %d): candidate arm is empty.  This happens when decomposing high energy arms.  Skipping.\n", mMetaArmID); 
      return false; 
    } 

    if (candidate->mSeenInMeta) {
       dbprintf(4, "FindEndpoint(metaarm %d): candidate arm is already seen in this arm -- return false\n", mMetaArmID); 
       return false; 
    }

    if (candidate->mSeen) {
      dbprintf(4, "FindEndpoint(metaarm %d): candidate arm is type111 and already seen somewhere  -- return false\n", mMetaArmID); 
      return false; 
    }
    if ( candidate->GetBurgersType() != seed->GetBurgersType()) {
      dbprintf(4, "FindEndpoint(metaarm %d): candidate arm burgers (%s) does not match seed burgers (%s)-- return false\n", mMetaArmID, BurgersTypeNames(candidate->GetBurgersType()).c_str(), BurgersTypeNames(seed->GetBurgersType()).c_str()); 
      return false; 
    }
    
    if (candidate->isTypeMM()) {
      dbprintf(4, "FindEndpoint(metaarm %d): Error: candidate arm is type MM -- This should never happen.\n", mMetaArmID); 
      return false; 
    }
    
    if (candidate->mArmType == ARM_LOOP) {
      dbprintf(4, "FindEndpoint(metaarm %d): Candidate is a loop. Do not explore.\n", mMetaArmID); 
      return false; 
    }

    candidate->mSeen = true; 
    candidate->mSeenInMeta = true; 

    if (candidate->isType111()) {
      dbprintf(4, "Candidate is type 111 and we shall try to recurse..\n", mMetaArmID);
      mFound111 = true; 
    }
    else {
      dbprintf(4, "Candidate is type 200 and we shall try to recurse..\n", mMetaArmID);
    }
    uint32_t nodenum = candidate->mTerminalNodes.size(); 
    if (nodenum < 2) {
      dbprintf(0, "Error:  Found candidate with %d terminal node(s), but we already tested for loops. \n", mMetaArmID, nodenum); 
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
        dbprintf(4, "FindEndpoint(metaarm %d): Candidate has M node on other end.  Terminating and returning true.\n", mMetaArmID); 
        AddTerminalNode(node); 
        AddTerminalArm(candidate); 
        return true; 
      }

      uint32_t neighbornum = node->mNeighborArms.size(); 
      dbprintf(4, "FindEndpoint(metaarm %d): Candidate has N node on other end. Recursing on %d neighbors.\n", mMetaArmID, neighbornum); 
      Arm *foundseed = NULL; 
      while (neighbornum--) {
        Arm *arm = node->mNeighborArms[neighbornum]; 
        if (arm == candidate) {
          continue; // no need to examine ourselves.  
        }
        if (arm == seed) {
          dbprintf(4, "FindEndpoint(metaarm %d): Hey!  Neighbor %d is the seed arm.  Are we a loop? Defer.\n", mMetaArmID, neighbornum); 
          foundseed = seed; 
          continue; 
        }
        dbprintf(4, "FindEndpoint(metaarm %d): Recursing on candidate neighbor arm...\n", mMetaArmID); 
        if (FindEndpoint(seed, node, node->mNeighborArms[neighbornum])) {
          AddArm(candidate); 
          return true; 
        }
      }
      dbprintf(4, "FindEndpoint(metaarm %d): Done Checking candidate neighbor arms.\n", mMetaArmID); 
      
      if (foundseed) {
        dbprintf(4, "FindEndpoint(metaarm %d): We found the seed after all other neighbors are exhausted.  Thus we are in a looped arm.\n", mMetaArmID); 
        AddArm(candidate); 
        AddTerminalNode(node); 
        mMetaArmType = METAARM_LOOP_111; 
        return true;
      }
    }
    dbprintf(4, "FindEndpoint(metaarm %d): Done recursing on neighbors.\n", mMetaArmID); 
    
    if (candidate->isType111()) {
      dbprintf(4, "FindEndpoint(metaarm %d):  Candidate is a type111 arm with no way to extend beyond its type N node.  Terminate and mark as METAARM_UNKNOWN.\n", mMetaArmID); 
      mMetaArmType = METAARM_UNKNOWN; 
      AddTerminalNode(otherNode); 
      AddTerminalArm(candidate); 
      return true; 
    }

    dbprintf(4, "FindEndpoint(metaarm %d):  Candidate is a dead end.\n", mMetaArmID);   
    mDeadEnds.push_back(candidate); // so you can mark them as not seen later.  
    return false; 
  }
   

  //===========================================================================
  void MetaArm::FindEndpoints(Arm *seed) {      

    dbprintf(4, "=======================================================\n", seed->Stringify(0).c_str()); 
    dbprintf(4, "FindEndpoints(metaarm %d) called with seed %s\n", mMetaArmID, seed->Stringify(0, false).c_str()); 
    
    mLength = 0; 
    mMetaArmType = METAARM_UNKNOWN; 
    
    // FIRST, CHECK FOR A SIMPLE LOOP SITUATION. 
    seed->mSeen = true; 
    // mLength = seed->GetLength(); 
    if (seed->mArmType == ARM_LOOP) {
      if (seed->isHighEnergy()) {
        dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): Seed is HIGH_ENERGY LOOP arm, so this is METAARM_LOOP_HIGH_ENERGY.\n", mMetaArmID, seed->mArmID); 
        mMetaArmType = METAARM_LOOP_HIGH_ENERGY; 
      }
      else {
        dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): Seed arm is Type 111 LOOP arm, so this is METAARM_LOOP_111.\n", mMetaArmID, seed->mArmID); 
        mMetaArmType = METAARM_LOOP_111; 
      }
      dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): Seed arm is a loop. Adding as its own metaArm. Seed arm: %s\n", mMetaArmID, seed->mArmID, seed->Stringify(0).c_str());
      AddTerminalArm(seed); 
      mTerminalNodes = seed->mTerminalNodes; 
      CapturePath(false); 
      return;
    }
    
    // Another special case:  the metaarm is composed of a single MM arm
    if (seed->isTypeMM()) {
      if (seed->isHighEnergy()) {
        dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): Seed arm is Type 200 MM arm.  This is METAARM_UNKNOWN, since all non-loop type 200 arms should have been decomposed.\n", mMetaArmID, seed->mArmID); 
        mMetaArmType = METAARM_UNKNOWN; 
      }               
      else if (seed->isType111()){
        dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): Seed arm is Type 111 MM arm, so this is METAARM_111.\n", mMetaArmID, seed->mArmID); 
        mMetaArmType = METAARM_111; 
      }
      else {
        dbprintf(0, "FindEndpoints(metaarm %d) (seed %d): Seed arm is Type %d MM arm.  This is supposed to be impossible.  Aborting.\n", mMetaArmID, seed->mArmID, seed->mArmType); 
        errexit; 
      }   
      
      AddTerminalArm(seed); 
      mTerminalNodes = seed->mTerminalNodes; 
      CapturePath(false); 
      return;
    } 
    
    
    if (seed->isTypeUnknown()) {
      dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): ERROR: Seed arm is Type Unknown.  Giving up and trying new seed.\n", mMetaArmID, seed->mArmID); 
      return; 
    }
    
    
    // consider this to be a completely self contained metaarm. 
    // Best algorithm: 
    
    // A metaarm can continue in up to two directions, call them paths.  
    //uint32_t pathsTaken = 0; 
    
    // First, check each terminal node of the arm.  If it's a monster, add it as a meta arm terminal node and mark off a path.  If it's not, then recurse on it to extend the arm.  
    dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): Checking terminal nodes of seed arm.\n", mMetaArmID, seed->mArmID); 
    uint32_t nodenum = 0; // seed->mTerminalNodes.size(); 
    bool seedIsTerminal = false; 
    while (nodenum <  seed->mTerminalNodes.size() && mMetaArmType != METAARM_LOOP_111) {
      FullNode * node = seed->mTerminalNodes[nodenum]; 
      if (node->GetNodeType() < 0) {
        dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): Terminal node %d is a monster node. Add seed as terminal arm and node as terminal node.\n", mMetaArmID, seed->mArmID, nodenum); 
        // Monster node
        if (!mTerminalArms.size() || mTerminalArms[0] != seed) {
          AddTerminalArm(seed); 
          seedIsTerminal = true; 
        }
        AddTerminalNode(node); 
      }
      else {
        dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): Terminal node %d is not a monster node. Recurse on its neighbors.\n", mMetaArmID, seed->mArmID, nodenum); 
        int neighborNum = node->mNeighborArms.size(); 
        while (neighborNum--) {
          Arm *arm = node->mNeighborArms[neighborNum]; 
          if (arm == seed) {
            dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): Node neighbor %d is seed.  Ignore.\n", seed->mArmID, nodenum); 
            continue; 
          }
          dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): call FindEndpoint on neighbor %d...\n", mMetaArmID, seed->mArmID, nodenum); 
          if (FindEndpoint(seed, node,  arm)) {            
            dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): Node neighbor %d  resulted in valid endpoint.\n", mMetaArmID, seed->mArmID, nodenum);
            if (nodenum == seed->mTerminalNodes.size()-1) {
              if (!seedIsTerminal) {
                dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): Adding seed as it is not a terminal arm and we are on the final terminal node.\n", mMetaArmID, seed->mArmID); 
                AddArm(seed); 
              }
              else {
                dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): We are on the final terminal node but we are not adding the seed as it is already a terminal arm.\n", mMetaArmID, seed->mArmID);
              }
            }
            break; // only one path allowed per terminal node.  
          }
        }
      }
      if (mMetaArmType == METAARM_LOOP_111) {
        AddTerminalArm(seed); 
      }
      CapturePath(nodenum); // Arms are collected in reverse, so reverse that for last node 
      ++nodenum; 
    }
    if (mTerminalNodes.size() == 1 || mTerminalArms.size() == 1) {
      // we are a loop
      if (seed->isHighEnergy()) {
        dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): Seed is Type 200 LOOP arm, so this is METAARM_LOOP_HIGH_ENERGY.\n", mMetaArmID, seed->mArmID); 
        mMetaArmType = METAARM_LOOP_HIGH_ENERGY; 
      }
      else {
        dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): Seed arm is Type 111 LOOP arm, so this is METAARM_LOOP_111.\n", mMetaArmID, seed->mArmID); 
        mMetaArmType = METAARM_LOOP_111; 
      }
    }
       
    // we must be METAARM_111 if we are not a loop here 
    if (mMetaArmType == METAARM_UNKNOWN) {
      mMetaArmType = METAARM_111; 
    }
    
    
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
    dbprintf(4, "FindEndpoints(metaarm %d) (seed %d): MetaArm is type %d, and found111 is %d\n", mMetaArmID, seed->mArmID, atype, (int)mFound111); 

    return;

  }
 
  //===========================================================================
  /*!
    Return a list of nodes in the metaArm.  If the MetaArm
    contains any wrapped segments, then the interruption will be notated with 
    mWrappedNode.
  */ 
  vector<FullNode * > MetaArm::GetNodes(void) {
    vector<FullNode*> nodes;
    mNumSegments = 0; 
    if (mMetaArmType == METAARM_LOOP_111 || mMetaArmType == METAARM_LOOP_HIGH_ENERGY) {
      dbprintf(5, "MetaArm::GetNodes(MetaArm %d): Get all loop node locations for all member arms.\n", mMetaArmID); 
      
      vector<Arm*>::iterator armpos = mAllArms.begin(), endpos = mAllArms.end(); 
      FullNode *lastNode = mTerminalNodes[0]; 
      
      while (armpos != endpos) {
        Arm *currentArm = *armpos; 
        vector<FullNode*> newnodes = currentArm->GetNodes(lastNode); 
        dbprintf(5, "MetaArm::GetNodes(MetaArm %d): Got %d nodes from new armID %d\n", mMetaArmID, newnodes.size(), currentArm->mArmID); 
        if (!newnodes.size()) {
          dbprintf(5, "MetaArm::GetNodes(MetaArm %d): ERROR! Failed to get nodes from arm!\n", mMetaArmID); 
          return nodes; 
        }
        if (nodes.size() && *(nodes.end()-1) == newnodes[0]) {
          dbprintf(5, "MetaArm::GetNodes(MetaArm %d): deleting duplicate end node %s.\n", mMetaArmID, (*(nodes.end()-1))->Stringify(0).c_str()); 
          nodes.erase(nodes.end()-1); 
        } 
        nodes.insert(nodes.end(), newnodes.begin(), newnodes.end());  
        lastNode = *(nodes.end()-1);
        dbprintf(5, "MetaArm::GetNodes(MetaArm %d): lastNode is %d\n", mMetaArmID, lastNode->GetIndex());         
        ++armpos; 
      }


      //#define SANITY_CHECK 0
#if (SANITY_CHECK==1)
      int nsize = nodes.size(), ssize = mTerminalArms[0]->GetSegments().size();
      if (nsize != ssize+1) {
        int verbsave = dbg_isverbose(); 
        dbg_setverbose(5); 
        dbprintf(0, "MetaArm::GetNodes(MetaArm %d): WARNING: nodes.size() (%d) != GetSegments(startNode).size()+1 (%d) for arm %d\n", mMetaArmID, nsize, ssize+1, mTerminalArms[0]->mArmID);     
        mTerminalArms[0]->GetSegments(); 
        mTerminalArms[0]->GetNodes(); 
        dbg_setverbose(verbsave);        
      }     
    
      if (mTerminalArms[0]->mTerminalNodes.size() > 1) {
        dbprintf(5, "MetaArm::GetNodes(MetaArm %d): Checking with reverse search for nodes and segments.\n"); 
        vector<FullNode*> nodes2 = mTerminalArms[0]->GetNodesReversed(); 
        int ssize2 = mTerminalArms[0]->GetSegmentsReversed().size();
        if (nodes2.size() != nodes.size() || ssize != ssize2) {
          dbprintf(0, "MetaArm::GetNodes(MetaArm %d): ERROR: GetNodes() != GetNodesReversed.size() or ssize != ssize2\n", mMetaArmID); 
          return nodes; 
        }
      }
      dbprintf(4, "MetaArm::GetNodes(MetaArm %d): All checks seem consistent.\n", mMetaArmID); 
#endif // SANITY_CHECK
      dbprintf(5, "MetaArm::GetNodes(MetaArm %d): returning %d nodes.\n", mMetaArmID, nodes.size());  
      return nodes; 
      
    }
    else {
      dbprintf(5, "MetaArm::GetNodes(MetaArm %d): returning 2 terminal nodes.\n", mMetaArmID, nodes.size());  
      return mTerminalNodes; 
    }
  }

  //===========================================================================
  /*!
    Return a list of locations for the nodes in the metaArm if it's a loop, 
    otherwise just the endpoints.  The endpoints can be wrapped if desired.  
    If the MetaArm
    contains any wrapped segments, then the interruption will be notated with 
    mWrappedNode.  
  */ 
  vector<rclib::Point<float> > MetaArm::GetNodeLocations(bool wrapEndpoints) {
    vector<rclib::Point<float> > points; 
    if (mMetaArmType == METAARM_LOOP_111 || mMetaArmType == METAARM_LOOP_HIGH_ENERGY) {
      vector<FullNode *>nodes = GetNodes(); 
      uint32_t  p = 0; 
      while (p < nodes.size()-1) {
        vector<float> f = nodes[p]->GetLocation();
        points.push_back(f);
        dbprintf(5, "MetaArm::GetNodeLocations(), MetaArm %d: push back node (%f, %f, %f).\n", mMetaArmID, f[0], f[1], f[2]); 
        mNumNodes ++; 
        if (!nodes[p+1]) {
          // wrapped nodes always have the same NodeID. 
          points.push_back(mWrappedNode); 
          ++p; 
          dbprintf(5, "MetaArm::GetNodeLocations(), MetaArm %d: push back wrapped node\n", mMetaArmID);
        } else {
          mNumSegments++; 
        }
        ++p; 
      } 
      vector<float> f = nodes[p]->GetLocation();
      points.push_back(f);
      dbprintf(5, "MetaArm::GetNodeLocations(), MetaArm %d: push back %dth node (%f, %f, %f).\n", mMetaArmID, ++p, f[0], f[1], f[2]); 
      dbprintf(5, "MetaArm::GetNodeLocations(), MetaArm %d: mNumSegments is %d;\n", mMetaArmID, mNumSegments); 
      
   } else {
      mNumSegments = 0;
      dbprintf(5, "MetaArm::GetNodeLocations(), MetaArm %d Not a loop -- we can simply return the two terminal nodes.\n", mMetaArmID); 
      if (mTerminalNodes.size() != 2) {
        dbprintf(0, "MetaArm::GetNodeLocations(), MetaArm %d ERROR: We are in a non-loop meta arm that has %d terminal nodes.\n", mMetaArmID, mTerminalNodes.size()); 
        return points; 
      }      
      float loc1[3], loc1new[3], loc2[3], loc2new[3]; 
      mTerminalNodes[0]->GetLocation(loc1); 
      mTerminalNodes[0]->GetLocation(loc1new); 
      mTerminalNodes[1]->GetLocation(loc2); 
      mTerminalNodes[1]->GetLocation(loc2new); 
      dbprintf(5, "MetaArm::GetNodeLocations(),MetaArm %d:  non-loop: pushing back 2 nodes: (%f, %f, %f) and (%f, %f, %f).\n", mMetaArmID, loc1[0], loc1[1], loc1[2], loc2[0], loc2[1], loc2[2]); 

      if (wrapEndpoints && DataSet::Wrap(loc1new, loc2new)) {
        dbprintf(5, "MetaArm::GetNodeLocations() Wrapped terminal nodes, so there will appear to be four.\n"); 
        dbprintf(5, "MetaArm::GetNodeLocations(), non-loop: wrapped nodes to new locations: (%f, %f, %f) -> (%f, %f, %f) and (%f, %f, %f) -> (%f, %f, %f).\n", loc1[0], loc1[1], loc1[2], loc1new[0], loc1new[1], loc1new[2], loc2[0], loc2[1], loc2[2], loc2new[0], loc2new[1], loc2new[2]); 
        points.push_back(loc1); 
        points.push_back(loc2new);    
        points.push_back(mWrappedNode); 
        points.push_back(loc2); 
        points.push_back(loc1new);    
        mNumSegments = 2; 
      }
      else {
        points.push_back(loc1); 
        points.push_back(loc2); 
        mNumSegments = 1; 
      }      
    }
    dbprintf(5, "MetaArm::GetNodeLocations(), MetaArm %d:  COMPLETE returning %d points\n", mMetaArmID, points.size()); 
    return points; 
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
      "NN_200", 
      "NN_020", 
      "NN_002", 
      "NN_+++",
      "NN_++-",
      "NN_+-+",
      "NN_-++"
    };
    
    double shortLengths[16] = {0}, longLengths[16]={0}; 
    uint32_t numShortArms[16]={0}, numLongArms[16]={0}; 

    vector<Arm *>::iterator armpos = Arm::mArms.begin(), armend = Arm::mArms.end(); 
    while (armpos != armend) { 
      double length = (*armpos)->GetLength(); 
      int armType = (*armpos)->mArmType; 
      if (armType == ARM_EMPTY) {
        ++armpos; 
        continue; 
      }
      if (mThreshold >= 0) {
        int8_t btype = (*armpos)->GetBurgersType(); 
        if (!btype) {
          printf("Error:  armpos has no terminal segments!\n"); 
        }
        if (armType == ARM_SHORT_NN_111) {
          numShortArms[btype-1]++;
          shortLengths[btype-1] += length; 
        }
        else {
          numLongArms[btype-1]++;
          longLengths[btype-1] += length; 
        }       
      }
      if (mNumBins) {
        int binNum = (int)((double)length/Arm::mLongestLength * mNumBins); 
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
      if ((*armpos)->mPartOfLinkedLoop) {
        numLinkedLoops ++; 
        linkedLoopLength += length; 
      }
#endif
      ++armpos; 
    }
    
    fprintf(thefile, "\n"); 
    fprintf(thefile, "===========================================\n"); 
    fprintf(thefile, "total Number of non-empty arms: %d\n", totalArms); 
    fprintf(thefile, "total length of all arms before decomposition: %.2f\n", Arm::mTotalArmLengthBeforeDecomposition); 
    fprintf(thefile, "total length of all arms after decomposition: %.2f\n", Arm::mTotalArmLengthAfterDecomposition); 
    double delta = Arm::mTotalArmLengthAfterDecomposition - (Arm::mDecomposedLength + Arm::mTotalArmLengthBeforeDecomposition); 
    fprintf(thefile, "Total decomposed length (computed independently): %.2f (delta %f)\n", Arm::mDecomposedLength, delta); 
    double ratio = delta / Arm::mTotalArmLengthAfterDecomposition;
    if (Arm::mTotalArmLengthAfterDecomposition && fabs(ratio) > 0.00001 ) {
      string errmsg = str(boost::format("\n\nError:  mDecomposedLength %1% + mTotalArmLengthBeforeDecomposition %2% != mTotalArmLengthAfterDecomposition %3% (ratio is %4%)!\n\n\n")%Arm::mDecomposedLength%Arm::mTotalArmLengthBeforeDecomposition%Arm::mTotalArmLengthAfterDecomposition%ratio);
      cerr  << errmsg; 
      fprintf(thefile, "%s", errmsg.c_str()); 
    }
    fprintf(thefile, "Number of segments classified in arm: %d\n", ArmSegment::mNumClassified); 
    fprintf(thefile, "Number of segments measured in arm: %d\n", ArmSegment::mNumArmSegmentsMeasured); 
    fprintf(thefile, "Number of segments wrapped: %d\n", ArmSegment::mNumWrapped); 
    fprintf(thefile, "===========================================\n"); 
    int i = 0; for (i=0; i<11; i++) {
      fprintf(thefile, "%s: number of arms = %d\n", ArmTypeNames(i).c_str(), numArms[i]);
      fprintf(thefile, "%s: total length of arms = %.2f\n", ArmTypeNames(i).c_str(), armLengths[i]); 
      fprintf(thefile, "----------------------\n"); 
    }
#if LINKED_LOOPS
    fprintf(thefile, "LINKED LOOPS: total number of arms = %d\n", numLinkedLoops); 
    fprintf(thefile, "LINKED LOOPS: total length of arms = %.2f\n", linkedLoopLength); 
    fprintf(thefile, "----------------------\n"); 
#endif

    // write a row of arm lengths to make analysis via spreadsheet easier
    fprintf(thefile, "Key: \nUNKNOWN\tUNINTRSTNG\tLOOP\tMM_111\tMN_111\tNN_111\tMM_200\tMN_200\tNN_200\tSHORT_NN_111\tSHORT_NN_200\n"); 
    int n = 0; 
    while (n<11) {
      fprintf(thefile, "%.2f\t",  armLengths[n]); 
      ++n;
    }

           
    
    if (mThreshold >= 0.0) {
      fprintf(thefile, "\n\n----------------------\n"); 
      fprintf(thefile, "THRESHOLD data.  Threshold = %.2f\n", mThreshold); 
      int n = 0; 
      for (n=0; n<16; n++) {       
        fprintf(thefile, "----------------------\n"); 
        fprintf(thefile, "Total number of %s arms: %d\n", armTypes[n], numShortArms[n] + numLongArms[n]); 
        fprintf(thefile, "Total length of %s arms: %.2f\n", armTypes[n], shortLengths[n] + longLengths[n]); 
        fprintf(thefile, "Number of %s arms SHORTER than threshold = %d\n", armTypes[n], numShortArms[n]); 
        fprintf(thefile, "Total length of %s arms shorter than threshold = %.2f\n", armTypes[n], shortLengths[n]); 
        fprintf(thefile, "Number of %s arms LONGER than threshold = %d\n", armTypes[n], numLongArms[n]); 
        fprintf(thefile, "Total length of %s arms longer than threshold = %.2f\n", armTypes[n], longLengths[n]); 
        fprintf(thefile, "\n"); 
      }
    }
    
    // write a row of arm lengths to make analysis via spreadsheet easier
    fprintf(thefile, "----------------------\n"); 
    fprintf(thefile, "Key: NN_200\tNN_020\tNN_002\tNN_+++\tNN_++-\tNN_+-+\tNN_-++\n"); 
    fprintf(thefile, "SHORT ARM LENGTHS:\n"); 
    n=0; while (n<16) {
      fprintf(thefile, "%.2f\t",  shortLengths[n]); 
      ++n;
    }
    fprintf(thefile, "\nLONG ARM LENGTHS:\n"); 
    n = 0; while (n<16) {
      fprintf(thefile, "%.2f\t",  longLengths[n]); 
      ++n;
    }
    fprintf(thefile, "\n"); 

    fprintf(thefile, "----------------------\n\n\n"); 

    if (mNumBins) {
      // print a row of bin values
      fprintf(thefile, "BINS: \n");
      fprintf(thefile, "max length = %.3f\n", Arm::mLongestLength); 
      
      long totalArms = 0; // reality check
      double totalLength = 0;  // reality check
      string line; 
      int binNum = 0; 
      fprintf(thefile, "%-12s%-12s%-12s\n", "Bin", "Arms", "Lengths"); 
      for (binNum = 0; binNum < mNumBins; ++binNum) {
        fprintf(thefile, "%-12d%-12ld%-12.3f\n", binNum, armBins[binNum], armLengthBins[binNum]); 
        totalArms += armBins[binNum];
        totalLength += armLengthBins[binNum];
      }
      fprintf(thefile, "%-12s%-12ld%-12.3f\n", "TOTAL", totalArms, totalLength); 
    }

    fprintf(thefile, "\n\n========================================\n"); 
    fprintf(thefile, "DECOMPOSITION STATISTICS\n"); 
    fprintf(thefile, "----------------------\n"); 
    int energy = 0; 
    while (energy < 7) {
      fprintf(thefile, "Decomposed arms, energy level %d: %d\n", energy, Arm::mNumDecomposed[energy]); 
      energy++; 
    }

      fprintf(thefile, "Detached/absorbed arms: %d\n", Arm::mNumDestroyedInDetachment); 
    
    fprintf(thefile, "========================================\n\n\n"); 
#ifdef DEBUG_SEGMENTS
    // check against segment lengths: 
    uint32_t numSegments[11] = {0}, totalSegments=0, culledSegments=0;  // number of arms of each type
    double segmentLengths[11] = {0}, totalSegmentLength=0, culledLength=0; 
    for (std::map<uint32_t, ArmSegment *>::iterator segpos = ArmSegments.begin(); segpos != mArmSegments.end(); ++segpos) {
      ArmSegment *seg = segpos->second; 
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
    }
    
    fprintf(thefile, "===========================================\n"); 
    fprintf(thefile, "REALITY CHECK:  total length of all segments, skipping wrapped segments\n"); 
    fprintf(thefile, "total Number of segments: %d\n", totalSegments); 
    fprintf(thefile, "total length of all segments: %.2f\n", totalSegmentLength); 
    fprintf(thefile, "===========================================\n"); 
    for (i=0; i<11; i++) {
      fprintf(thefile, "%s: number of segs = %d\n", ArmTypeNames(i).c_str(), numArms[i]);
      fprintf(thefile, "%s: total length of segments = %.2f\n", ArmTypeNames(i).c_str(), armLengths[i]); 
      fprintf(thefile, "----------------------\n"); 
    }
    
    fprintf(thefile, "CULLED segments = %d\n", culledSegments); 
    fprintf(thefile, "CULLED length = %.2f\n", culledLength); 
    
    fprintf(thefile, "Wrapped lengths: %.2f\n", ArmSegment::mSegLen); 
    fprintf(thefile, "----------------------\n\n\n"); 
#endif
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
      dbprintf(2, "Looking for file version... "); 
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
  bool DataSet::Wrap(float *loc0, float *loc1) {
    float shift_amt[3] = {0}; 
    bool wrap = false;
    int i=3; while (i--) {
      float dist = fabs(loc0[i] - loc1[i]); 
      if (mDataSize[i]/2.0 < dist) {
        wrap = true; 
        shift_amt[i] = mDataSize[i]; 
      }
    }
    if (wrap) {
      i = 3; while (i--) {             
        if (loc0[i] < loc1[i]) {
          /*! 
            Wrap loc0 "forward" and wrap loc1 "backward" in space
          */ 
          loc0[i] += shift_amt[i]; 
          loc1[i] -= shift_amt[i]; 
          
        } else {
          /*! 
            Wrap loc0 "backward" and wrap loc1 "forward" in space
          */ 
          loc0[i] -= shift_amt[i]; 
          loc1[i] += shift_amt[i];    
          
        }
        
      }
    }
    return wrap; 
  }
  
  //====================================================================== 
  void DataSet::CopyNodeFromFile(uint32_t &lineno, map<uint64_t, FullNode*> &nodehash, std::ifstream &datafile, std::ofstream &tagfile) {
    // read the first line of the next node in the file: 
    char comma;
    int domainID, nodeID; 
    long old_id_junk, tag, numNeighbors; 
    double x,y,z; 
    if (mFileVersion > 0) {
      datafile >> domainID >> comma >> nodeID;
    }
    else { // old-style
      datafile >> nodeID >> old_id_junk;
    }
    if (!datafile.good()) {
      throw str(boost::format("DataSet::CopyNodeFromFile line %1%: error reading node")% __LINE__); 
    }
    //dbprintf(5, "Got node ID (%d, %d) at line %d\n", domainID, nodeID, lineno); 
    datafile >> x >> y >> z >> numNeighbors >> tag;    
    if (mFileVersion == 0){
      datafile  >> domainID >>  old_id_junk; 
    }
    if (!datafile.good()) {
      throw str(boost::format("DataSet::CopyNodeFromFile line %1%: error reading node")% __LINE__); 
    }
    //dbprintf(5, "Got loc (%f, %f, %f), neighbors %d, and tag %d at line %d\n", x,y,z,numNeighbors, tag, lineno);  
    ++lineno;
    NodeID theID(domainID, nodeID); 
    
    FullNode *matchNode = nodehash[theID.Hash()]; 
    if (!matchNode) {
      throw str(boost::format("DataSet::CopyNodeFromFile line %1%: cannot match NodeID %s in FullNodes hash")% __LINE__%(theID.Stringify(0))); 
    }
      
    // match the ID to our fullNode
    int nodetag = 0; 
    if (matchNode->IsLoopNode()) nodetag += 8192; 
    // if (fullnode->IsTypeM()) nodetag += 256; 
    // if (fullnode->IsTypeN()) nodetag += 512; 
    if (tag != nodetag) {
      dbprintf(5, "DataSet::CopyNodeFromFile: Node %d: Old tag %d --> New tag %d\n", matchNode->GetIndex(), tag, nodetag); 
      tag = nodetag;
    }        
 
    string linebuf; 
    // Write back out the first line of node information
    if (mFileVersion > 0) {
      linebuf = str(boost::format("%1%,%2% ")%domainID%nodeID); 
    }
    else { // old-style
      linebuf = str(boost::format("%1%,%2% ")%nodeID%old_id_junk);
    }
    linebuf +=  str(boost::format("%.8f %.8f %8f %d %d")%x%y%z%numNeighbors % tag); 
    if (mFileVersion == 0){
      linebuf +=  str(boost::format("%1% %2%")% domainID % old_id_junk); 
    }
    tagfile << linebuf << endl; 
 
   getline(datafile, linebuf); // clear the '\n' waiting in datafile. 
    int neighbornum = 0; 
    while (neighbornum < numNeighbors && datafile.good()) {
      // read and write two lines per neighbor
      getline(datafile, linebuf); 
      tagfile << linebuf << endl; 
      getline(datafile, linebuf); 
      tagfile << linebuf << endl; 
      if (!datafile.good()) {
        throw str(boost::format("DataSet::CopyNodeFromFile line %1%: error reading node index %1%, neighbor %2%")% __LINE__% matchNode->GetIndex()% neighbornum); 
      }      
      ++neighbornum;
      lineno += 2; 
    }
    return;
  }

  //====================================================================== 
  void DataSet::ReadMinimalNodeFromFile(uint32_t &lineno, std::ifstream &datafile) {
    MinimalNode theNode;
    char comma;
    int domainID, nodeID; 
    long old_id_junk, constraint_junk, numNeighbors; 
    double x,y,z, float_junk; 
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
      throw string("DataSet::ReadMinimalNodeFromFile: error reading node"); 
    }
    ++lineno;

    NodeID theID(domainID, nodeID); 
    //------------------------------------------------
    // done reading first line of node information
    if (rclib::InBounds(rclib::Point<float>(x,y,z), mSubspaceMin, mSubspaceMax ))
      theNode.InBounds(true); 

    theNode.SetNodeID(theID); 
    
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
        throw string("DataSet::ReadMinimalNodeFromFile: error reading neighbor number ") + intToString(neighbornum);
      }
      ++lineno; 
      ++neighbornum; 
    }; 
    
    //------------------------------------------------
    // done reading neighbor information
    theNode.SetFileOrderIndex(MinimalNode::mMinimalNodes.size()); 
    MinimalNode::mMinimalNodes.push_back(theNode); 
    // dbprintf(5, "pushed back new Minimal Node: %s\n", theNode.Stringify(0).c_str());
    return; 
  }
  
  //===========================================================================
  void DataSet::CreateMinimalNodes(void){
    dbprintf(2, "CreateMinimalNodes started...\n"); 
    STARTPROGRESS();
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
      
      while (datafile.good() && nodenum++ < mTotalDumpNodes) {
        UPDATEPROGRESS(nodenum, mTotalDumpNodes, "CreateMinimalNodes: Reading datafile"); 
        ReadMinimalNodeFromFile(lineno, datafile);  
        
      }
      COMPLETEPROGRESS(mTotalDumpNodes,"CreateMinimalNodes: Reading datafile"); 
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
    if (!dir)
      {
      if (mkdir(dirname, S_IRWXU | S_IRWXG | S_IRWXO )) {
        string errmsg = string("Warning:  could not create directory: ") + mOutputDir+ "." ; 
        cerr << errmsg << endl; 
        dbprintf(1, errmsg.c_str()); 
        return false; 
      } else {
        dbprintf(1, "Created directory %s\n", dirname);         
      }
    }else {
      closedir(dir); 
    }
#else
    if (_mkdir(dirname) != 0)
    {
        string errmsg = string("Warning:  could not create directory: ") + dirname+ "." ; 
        cerr << errmsg << endl; 
        dbprintf(1, errmsg.c_str()); 
        return false; 
    }
#endif
    return true; 
  }
  //===========================================================================
  void DataSet::DebugPrintMinimalNodes(void) {
    std::string filename = mOutputDir + "/" + mOutputBasename + "-MinimalNodes-debug.txt"; 
    dbecho(1, str(boost::format("Writing minimal nodes to debug file %1%... ")%filename)); 
    ofstream debugfile (filename.c_str()); 
    if (!debugfile) {
      string errmsg = string("Warning:  cannot open debug file ")+filename+". " + strerror(errno);
      cerr << errmsg << endl; 
      dbprintf(1, "%s\n", errmsg.c_str()); 
      return; 
    }
    debugfile << "Minimal Nodes file written on " << timestamp() << " by " << GetLibraryVersionString("DebugPrintMinimalNodes") << endl; 
    debugfile << "data bounds: " << mDataMin.Stringify() << ", " << mDataMax.Stringify() << endl; 
    debugfile << "subspace bounds: " << mSubspaceMin.Stringify() << ", " << mSubspaceMax.Stringify() << endl; 
    std::vector<MinimalNode>::iterator pos = MinimalNode::mMinimalNodes.begin(),
      endpos = MinimalNode::mMinimalNodes.end(); 
    uint32_t nodenum = 0; 
    while (pos != endpos) {
      debugfile << "MinimalNode " << nodenum++ <<":\n"<< pos->Stringify(false) << endl;
      debugfile << "***************************************************" << endl << endl; 
      ++pos; 
    }
    debugfile <<"Total minimal nodes: " << nodenum << endl; 
    debugfile << "Total memory for minimal nodes: " << nodenum * sizeof(MinimalNode) << endl; 
    dbecho(1, string("Done writing file.\n")); 
    return ;
  }
  //===========================================================================
  std::vector<MinimalNode *> DataSet::GetNeighborMinimalNodes(const MinimalNode &inode) { 
    std::vector<MinimalNode *> neighborNodes; 
    
    std::vector<const Neighbor *>::const_iterator neighbor_pos = inode.GetNeighbors().begin(), neighbor_endpos = inode.GetNeighbors().end(); 
    
    while (neighbor_pos != neighbor_endpos) {      
      std::vector<MinimalNode>::iterator otherEnd =  
        lower_bound(MinimalNode::mMinimalNodes.begin(), MinimalNode::mMinimalNodes.end(), 
                    MinimalNode((*neighbor_pos)->GetOtherEndpoint(inode.GetNodeID())));

      if (otherEnd == MinimalNode::mMinimalNodes.end()) 
        throw string("Error in DataSet::GetNeighborMinimalNodes -- cannot find other end of neighbor relation from node ")+inode.Stringify(false); 
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
    
    sort(MinimalNode::mMinimalNodes.begin(), MinimalNode::mMinimalNodes.end()); 
    std::vector<MinimalNode>::iterator node_pos = MinimalNode::mMinimalNodes.begin(), node_endpos = MinimalNode::mMinimalNodes.end(); 
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
      remove_if(MinimalNode::mMinimalNodes.begin(), MinimalNode::mMinimalNodes.end(), cullminimalnode); 
    MinimalNode::mMinimalNodes.erase(last, MinimalNode::mMinimalNodes.end()); 

    /*!
      Re-sort in file order to make finding full node counterparts go quicker
    */ 
    sort(MinimalNode::mMinimalNodes.begin(), MinimalNode::mMinimalNodes.end(), FileOrderPredicate); 
    
    dbprintf(2, "CullMinimalNodes ended...\n\n"); 
    return; 
  }



  //===========================================================================
  void DataSet::ReadFullNodeFromFile(std::ifstream &datafile, MinimalNode &theNode){
    FullNode *fullNode = NULL; //new FullNode; 
    char comma;
    long old_id_junk, constraint_junk, numNeighbors = 0;  ///TODO: check on uninitialized value
    float float_junk, location[3]; 
    int domainID, nodeID, neighborDomain, neighborID, numskipped=0;
    string junkstring;
    dbprintf(5, "ReadFullNodeFromFile\n"); 
    try {
      while (!fullNode) { 
        //-----------------------------------------------
        // read the first line of node information and see if it matches theNode
        if (mFileVersion > 0) {
          datafile >> domainID >> comma >> nodeID;
          if (!datafile.good()) {
            throw string("DataSet::ReadFullNodeFromFile: error reading domainID and nodeID of node"); 
          }
          dbprintf(5, "DataSet::ReadFullNodeFromFile: Got node id (%d,%d)\n", domainID, nodeID); 
        } else {
          datafile >> nodeID >> old_id_junk;
          if (!datafile.good()) {
            throw string("DataSet::ReadFullNodeFromFile: error reading nodeID"); 
          }
        }
        /*! 
          read location
        */ 
        int i=0; while (i < 3) datafile >> location[i++];
        if (!datafile.good()) {
          throw string("DataSet::ReadFullNodeFromFile: error reading location"); 
        }
        dbprintf(5, "DataSet::ReadFullNodeFromFile: read location (%f, %f, %f)\n", location[0], location[1], location[2]); 
        datafile >> numNeighbors >> constraint_junk;
        if (!datafile.good()) {
          throw string("DataSet::ReadFullNodeFromFile: error reading numNeighbors"); 
        }
        if (mFileVersion == 0){
          datafile  >> domainID >>  old_id_junk; 
          if (!datafile.good()) {
            throw string("DataSet::ReadFullNodeFromFile: error reading domainID"); 
          }
        }
        if (!datafile.good()) {
          throw string("DataSet::ReadFullNodeFromFile: error reading node"); 
        }
        
        if (theNode == NodeID(domainID, nodeID)) {
          fullNode = new FullNode;
          *fullNode = theNode; 
        }
        else {
          /*!
            read past the remaining node information for this node. 
          */ 
          //dbprintf(5, "skipping node: (%d, %d)\n", domainID, nodeID); 
          getline(datafile, junkstring); // finish the current line
          if (!datafile.good()) {
            throw string("DataSet::ReadFullNodeFromFile: error looking for end of line"); 
          }
          int neighborNum = 0; 
          while (neighborNum < numNeighbors) {
            int linenum = 0; while (linenum < 2) {
              getline(datafile, junkstring);  
              if (!datafile.good()) {
                throw string("DataSet::ReadFullNodeFromFile: error reading line ")+ intToString(linenum) + string(" of neighbor ") + intToString(neighborNum);
              }
              ++linenum;
            }
            ++neighborNum; 
          } 
          ++numskipped; 
        }      
      }
      dbprintf(5, "DataSet::ReadFullNodeFromFile: Found node info for minimal node (%d, %d)\n", domainID, nodeID); 
      //------------------------------------------------
      // done reading first line of node information    
      fullNode->SetLocation(location); 
      
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
              throw string("DataSet::ReadFullNodeFromFile: error reading neighborDomain and neighborID of neighbor"); 
            }
          }
          else {
            neighborDomain   = 0;
            datafile >> neighborID;
            if (!datafile.good()) {
              throw string("DataSet::ReadFullNodeFromFile: error reading neighborID"); 
            }
          }
          currentSegment->SetEndpoints(fullNode, neighborDomain, neighborID); 
          
          // read burgers value and discard "nx, ny, nz". 
          datafile >> burgers[0] >> burgers[1] >> burgers[2] >> float_junk >> float_junk >> float_junk;
          if (!datafile.good()) {
            throw string("DataSet::ReadFullNodeFromFile: error reading burgers values"); 
          }
          
          currentSegment->ComputeBurgersType(burgers); 
          
          //#ifdef __GNUC__
          ArmSegmentSet::iterator foundSegment = mQuickFindArmSegments.find(currentSegment); 
          if (foundSegment == mQuickFindArmSegments.end()) {
            mQuickFindArmSegments.insert(currentSegment); 
            foundSegment = mQuickFindArmSegments.find(currentSegment); 
          } else {
            (*foundSegment)->ReplacePlaceholder(fullNode);  
            delete currentSegment; 
          }
          fullNode->AddNeighbor((*foundSegment)); 
          //#endif
         
          ++neighbornum; 
        }; // done reading neighbor information
      } catch (string err) {
        throw string("DataSet::ReadFullNodeFromFile: Error in DataSet::ReadFullNode reading neighbor ")+intToString(neighbornum)+":" + err; 
      }
    } catch (string err) {
      throw string("DataSet::ReadFullNodeFromFile: Error trying to read full node info corresponding to ")+theNode.Stringify(false) + string("numskipped is ")+intToString(numskipped) + string("\n") + err;
    } 

    //fullNode->ComputeNodeType(); 
    dbprintf(5, "DataSet::ReadFullNodeFromFile: Done creating full node %s\n", fullNode->Stringify(false).c_str()); 

    return; 

  } /* end ReadFullNodeFromFile */ 
  //===========================================================================
  void DataSet::CreateFullNodesAndArmSegments(void){
    dbprintf(2, "CreateFullNodesAndArmSegments started...\n"); 
    reverse(MinimalNode::mMinimalNodes.begin(), MinimalNode::mMinimalNodes.end());
    timer theTimer; 
    theTimer.start(); 
    dbprintf(2, "Size of a full node is %d bytes, so expect to use %d megabytes\n",  sizeof(FullNode), MinimalNode::mMinimalNodes.size()*sizeof(FullNode)/1000000);
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
      STARTPROGRESS(); 
      uint32_t nodelimit = MinimalNode::mMinimalNodes.size(); 
      std::vector<MinimalNode>::reverse_iterator rpos = MinimalNode::mMinimalNodes.rbegin(), rend = MinimalNode::mMinimalNodes.rend(); 
      //dbprintf(2, "\n"); 
      while (datafile.good() && nodenum < nodelimit && rpos != rend) {
        UPDATEPROGRESS(nodenum, nodelimit,  "CreateFullNodesAndArmSegments: "); 
        ReadFullNodeFromFile(datafile, *rpos++);  
        /*!
          We have "used up" the last node in the vector, delete it to save memory
        */ 
        MinimalNode::mMinimalNodes.pop_back(); 
        ++nodenum; 
      }
      COMPLETEPROGRESS(nodelimit,  "CreateFullNodesAndArmSegments: "); 
      if (nodenum != nodelimit ) throw string("terminating before nodenum == nodelimit\n"); 
      if (rpos != rend) throw string("terminating before rpos == rend\n"); 
    } catch (string err) {
      throw string("Error in DataSet::CreateFullNodesAndArmSegments while reading node ") + intToString(nodenum) +":\n" + err; 
    }
    dbprintf(2, "CreateFullNodesAndArmSegments ended...\n"); 
    mQuickFindArmSegments.clear();
    return; 
  }

  
  //===========================================================================
  void DataSet::WrapBoundarySegments(void) {
    STARTPROGRESS();
    uint32_t segnum = 0, totalsegs = ArmSegment::mArmSegments.size(); 
    ArmSegment *newSegment = NULL; 
    FullNode *newnode0, *newnode1; 
    for (map <uint32_t, ArmSegment *>::const_iterator segpos = ArmSegment::mArmSegments.begin(); segpos != ArmSegment::mArmSegments.end(); segpos++, segnum++) {      
      ArmSegment *seg = segpos->second; 
      if (seg && seg->Wrap(mDataSize, newSegment, newnode0, newnode1)) {
        newnode0->SetWrappedCopy(true); 
        newnode1->SetWrappedCopy(true); 
        dbprintf(5, str(boost::format("\n***********\nWrapBoundarySegments: Created new wrapped node %1% at %2%\n")% (newnode0->GetNodeID().Stringify(0)) %newnode0).c_str()); 
        dbprintf(5, str(boost::format("\n***********\nWrapBoundarySegments: Created new wrapped node %1% at %2%\n")% (newnode1->GetNodeID().Stringify(0)) %newnode1).c_str()); 
        dbprintf(5, str(boost::format("\n***********\nWrapBoundarySegments: Created new wrapped segment %1% at %2%\n")% (newSegment->Stringify(0))%newSegment).c_str());        
      } 
      UPDATEPROGRESS(segnum, totalsegs, "WrapBoundarySegments"); 
    }
    COMPLETEPROGRESS(totalsegs, "WrapBoundarySegments"); 

    return; 
  }

  //===========================================================================
  string DataSet::GetMonsterNodeSummary(void) {
    vector<uint32_t> monsterTypes(1); 
    for (std::map<uint32_t, FullNode*>::iterator nodepos = FullNode::mFullNodes.begin(); nodepos !=  FullNode::mFullNodes.end(); nodepos++) {
      FullNode *node = nodepos->second; 
      if (node && node->GetNodeType() < 0 && !node->IsWrappedCopy()) {
        uint8_t theType = - node->GetNodeType(); 
        if (theType >= monsterTypes.size()) {
          monsterTypes.resize(theType+1); 
        }
        monsterTypes[theType]++;
        monsterTypes[0]++;
      }
    }    
    string s; 
    s += str(boost::format("Total monster nodes: %1%\n") %monsterTypes[0]); 
    for (uint8_t t = 1; t < monsterTypes.size(); t++) {
      if (monsterTypes[t]) {
        s += str(boost::format("Type -%1% monster nodes: %2%\n") % (int)t % monsterTypes[t]); 
      }
    }
    return s; 
  }
    
  //===========================================================================
  void DataSet::DebugPrintFullNodes(void) {
    std::string filename = mOutputDir + "/" + mOutputBasename + "-FullNodes-debug.txt";
    dbecho(1, str(boost::format("Writing full nodes to debug file %s... ")%filename)); 


    ofstream debugfile (filename.c_str()); 
    debugfile << endl << endl; 
    debugfile << "Full Nodes file written on " << timestamp() << " by " << GetLibraryVersionString("DebugPrintFullNodes") << endl; 
    debugfile << "NODE SUMMARY" << endl; 
    debugfile << "=================================================" << endl; 
    debugfile <<"Total full nodes: " << FullNode::mFullNodes.size() << endl; 

    debugfile << GetMonsterNodeSummary(); 

    debugfile <<"Total memory for nodes and their pointers: " << FullNode::mFullNodes.size() * (sizeof(FullNode) + sizeof(FullNode *)) << endl; 
    debugfile << "=================================================" << endl; 
    debugfile << endl << endl; 

    for (std::map<uint32_t, FullNode*>::iterator nodepos = FullNode::mFullNodes.begin(); nodepos != FullNode::mFullNodes.end(); nodepos++) {
      uint32_t key = nodepos->first; 
      FullNode *node = nodepos->second; 
      debugfile << "mFullNodes[" << key <<"]: " << node->Stringify(false ) << endl; 
      debugfile << "**************************************************************" << endl << endl; 
    }        
    dbecho(1, string("Wrote full nodes to debug file\n")); ; 
    return;  
  }
  
  //===========================================================================
  void DataSet::FindEndOfArm(FullNode *iStartNode, FullNode **oEndNode,  ArmSegment *iStartSegment,  ArmSegment *&oEndSegment, Arm *theArm
) {
    //FullNodeIterator currentNode = iStartNode, otherEnd; 
    FullNode *currentNode = iStartNode, *otherEnd; 
    ArmSegment *currentSegment = iStartSegment; 
    /* loop, don't recurse */ 
    while(true) {
      if (!currentSegment->Seen()) {
        ++(theArm->mNumSegments); 
        dbprintf(5, "Arm %d: adding segment %s\n", theArm->mArmID, currentSegment->Stringify(0).c_str());
        theArm->mArmLength += currentSegment->GetLength(true); 
      }
      currentSegment->SetSeen(true); 
      currentSegment->mParentArm = theArm; 
      otherEnd = currentSegment->GetOtherEndpoint(currentNode);
      if ((otherEnd)->Hash() == iStartNode->Hash() ||
          (otherEnd)->GetNumNeighborSegments() != 2) {
        if ((otherEnd)->GetNumNeighborSegments() == 2) {
          dbprintf(4, "Arm %d: LOOP detected\n", theArm->mArmID);
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
      if (*currentSegment == *(otherEnd)->GetNeighborSegment(0)) {
        currentSegment = const_cast<ArmSegment*>(otherEnd->GetNeighborSegment(1)); 
      } else {
        currentSegment = const_cast<ArmSegment*>(otherEnd->GetNeighborSegment(0)); 
      }     
      currentNode = otherEnd; 
    }
    return;     
  }

  //===========================================================================
  void DataSet::BuildArms(void) {
    dbprintf(2, "BuildArms started.\n"); 

    STARTPROGRESS(); 

    uint32_t armnum = 0; 
    /*! 
      For now, just look at every inbounds node and if it has not been looked at, make an arm out of it.  
    */ 
    
    int nodenum = 0, totalNodes = FullNode::mFullNodes.size(); 
    /*!
      If you start from an out of bounds node, you will often trace out arms that have no nodes in them!  That would be segfault fodder. 
    */ 
    try {
      for (map<uint32_t, FullNode*>::iterator nodepos = FullNode::mFullNodes.begin(); nodepos != FullNode::mFullNodes.end(); ++nodepos, ++nodenum) {
        FullNode *node = nodepos->second; 
        if (!node->InBounds()) {
          continue;
        }
        int neighbornum = 0, numneighbors = node->GetNumNeighborSegments();       
        FullNode *endNode0, *endNode1; 
        ArmSegment *endSegment0 = NULL, *endSegment1 = NULL, 
          *startSegment0 = NULL, *startSegment1 = NULL; 
        if (numneighbors == 2) {
          startSegment0 = const_cast< ArmSegment*>(node->GetNeighborSegment(0));
          if (!startSegment0->Seen()) {
            Arm *theArm = new Arm; 
            dbprintf(5, "DataSet::BuildArms(): Starting arm %d in middle of arm\n", theArm->mArmID); 
            FindEndOfArm(node, &endNode0, startSegment0, endSegment0, theArm ); 
            theArm->mTerminalNodes.push_back(endNode0); 
            endNode0->mNeighborArms.push_back(theArm); 
            theArm->mTerminalSegments.push_back(endSegment0); 

            startSegment1 = const_cast< ArmSegment*>(node->GetNeighborSegment(1));
            FindEndOfArm(node, &endNode1,startSegment1, endSegment1 , theArm ); 
            
            if (endNode0 != endNode1) {
              theArm->mTerminalNodes.push_back(endNode1); 
              endNode1->mNeighborArms.push_back(theArm); 
            }
            if (endSegment0 != endSegment1) {
              theArm->mTerminalSegments.push_back(endSegment1); 
            }
            dbprintf(5, "DataSet::BuildArms() (from middle of arm):  Pushing back arm %d: %s\n", armnum++, theArm->Stringify(0).c_str()); 

            Arm::mTotalArmLengthBeforeDecomposition += theArm->mArmLength; 

          }
        } else { // starting with a terminal node; create multiple arms
          while (neighbornum < numneighbors) {
            startSegment0 = 
              const_cast< ArmSegment*>(node->GetNeighborSegment(neighbornum));
            if (!startSegment0->Seen()) {
              Arm *theArm = new Arm; 
              dbprintf(5, "DataSet::BuildArms(): Starting arm %d at one end of arm\n", theArm->mArmID); 
              FindEndOfArm(node, &endNode0, startSegment0, endSegment0, theArm); 
              theArm->mTerminalNodes.push_back(node); 
              node->mNeighborArms.push_back(theArm); 
              if (endNode0 != node ) {
                theArm->mTerminalNodes.push_back(endNode0); 
                endNode0->mNeighborArms.push_back(theArm); 
              }
              theArm->mTerminalSegments.push_back(startSegment0); 
              if (endSegment0 != startSegment0) {                
                theArm->mTerminalSegments.push_back(endSegment0); 
              }
              dbprintf(5, "DataSet::BuildArms(): (from end of arm) Pushing back arm %d: %s\n", armnum++, theArm->Stringify(0).c_str()); 
               Arm::mTotalArmLengthBeforeDecomposition += theArm->mArmLength; 
            }
            ++neighbornum; 
          }
        }

        UPDATEPROGRESS(nodenum, totalNodes, str(boost::format("BuildArms: %1% arms created.")%armnum)); 
        
      }   
    } catch (string err) {
      throw string("Arm #")+intToString(armnum)+": "+err;
    }
    COMPLETEPROGRESS(totalNodes, str(boost::format("BuildArms: %1% arms created.")%armnum)); 
    return; 
  }


  //===========================================================================
  void DataSet::DecomposeArms(void) {    
    uint32_t armnum = 0; 
    vector<Arm*> newArms; 
    int energyLevel = 7, numarms=Arm::mArms.size();
    vector<int32_t> numDecomposed(7, 0); 
    while (energyLevel-- > 1) {
      if (energyLevel == 1 && !Arm::mTraceArms.size()) {
        // skip level 1; the only reason to do level 1 is to trace arms 
        continue; 
      }

      STARTPROGRESS();       
      vector<Arm*>::iterator pos = Arm::mArms.begin(), endpos = Arm::mArms.end(); 
      armnum = 0; 
      while (pos != endpos) {
        if ((*pos)->Decompose(energyLevel)) {
          numDecomposed[energyLevel]++;
        }
        ++pos; ++armnum;        
        UPDATEPROGRESS(armnum, numarms, str(boost::format("DecomposeArms: level %1% (%2%), arm %3%, %4% decomp.") % energyLevel % BurgersTypeNames(energyLevel*10) % armnum % numDecomposed[energyLevel]));         
      }
      COMPLETEPROGRESS(numarms, str(boost::format("DecomposeArms: level %1% (%2%), arm %3%, %4% decomp.") % energyLevel % BurgersTypeNames(energyLevel*10) % armnum % numDecomposed[energyLevel])); 
    }
    Arm::mTotalArmLengthAfterDecomposition = ComputeArmLengths(); 
    
  }


  //===========================================================================
  void DataSet::DebugPrintArms(const char *altname) {
    std::string filename = mOutputDir + "/" + mOutputBasename + "-Arms-debug.txt"; 
    if (altname) {
      string altstring = string("-") + altname; 
      filename = mOutputDir + string("/Arms-debug") + altstring + ".txt"; 
    }
    dbecho(1, str(boost::format("Writing arms to debug file %s... ")% filename)); 
    if (!Mkdir (mOutputDir.c_str())) {
      cerr << "Error: Cannot create output directory for arm file"; 
      return; 
    }

    ofstream debugfile (filename.c_str()); 
    
    debugfile << "Arms file written on " << timestamp() << " by " << GetLibraryVersionString("DebugPrintArms") << endl; 

    debugfile <<"There are " << Arm::mArms.size() << " arms." << endl; 
    vector<Arm*>::iterator pos = Arm::mArms.begin(), endpos = Arm::mArms.end(); 
    uint32_t armnum = 0, empty=0; 
    while (pos != endpos) {
      debugfile << "Arm #" << armnum << ": " << (*pos)->Stringify(0) << endl; 
      debugfile << "******************************************************" << endl << endl; 
      if ((*pos)->mArmType == ARM_EMPTY) 
        ++empty; 
      ++armnum; 
      ++pos; 
    }
    debugfile << "Number of arms: " << armnum<< endl; 
    debugfile << "Number of EMPTY arms: " << empty<< endl; 
    debugfile << "Number of non-EMPTY arms: " << armnum - empty<< endl; 
    debugfile << "Total memory used by arms: " << Arm::mArms.size() * sizeof(Arm) << endl; 
    debugfile << "Number of arm segments: " << ArmSegment::mArmSegments.size() << endl; 
    debugfile << "Memory used by arm segments and their pointers: " << ArmSegment::mArmSegments.size() * (sizeof(ArmSegment) + sizeof(ArmSegmentSetElement)) << endl;
    dbecho(1, string( "Done writing arm details to debug file \n")); 
    
    return; 
  }
  
  //===========================================================================
  void DataSet::DebugPrintMetaArms(void) {
    std::string filename = mOutputDir + "/" + mOutputBasename + "-metaarms-debug.txt"; 
    dbecho(1, str(boost::format("Writing MetaArms to debug file %s... ")% filename)); 
    if (!Mkdir (mOutputDir.c_str())) {
      cerr << "Error: Cannot create output directory for arm file"; 
      return; 
    }

    ofstream debugfile (filename.c_str()); 
    
    debugfile << "Meta-Arms file written on " << timestamp() << " by " << GetLibraryVersionString("DebugPrintMetaArms") << endl; 

    debugfile <<"There are " << mMetaArms.size() << " MetaArms." << endl; 
    vector<boost::shared_ptr<MetaArm> >::iterator pos = mMetaArms.begin(), endpos = mMetaArms.end(); 
    uint32_t armnum = 0; 
    while (pos != endpos) {
      debugfile << "MetaArm #" << armnum << ": " << (*pos)->Stringify(0) << endl; 
      debugfile.flush(); 
#if INSANE_DEBUG
      debugfile << (*pos)->mAllArms.size() << " arms included in metaarm " << armnum << ": " << endl;debugfile.flush(); 
      vector<Arm*>::iterator armpos = (*pos)->mAllArms.begin(), armendpos = (*pos)->mAllArms.end(); 
      while (armpos < armendpos) {
       debugfile << (*armpos)->Stringify(1); debugfile.flush(); 
        ++armpos; 
      } 
#endif
      debugfile << "******************************************************" << endl << endl; 
      ++armnum; 
      ++pos; 
    }
    debugfile << "Number of Metaarms: " << mMetaArms.size()<< endl; 
    debugfile << "Total memory used by arms: " << mMetaArms.size() * sizeof(MetaArm) << endl; 
    
    dbecho(1, "Finished writing MetaArm details to debug file\n"); 
    return; 
  }
  

  //===========================================================================
  void DataSet::TagNodes(void) {
    // First, have to tag all nodes.  Luckily, we only care about LOOPs.  
    int manum = 0, numMetaArms = mMetaArms.size(); 
    STARTPROGRESS();
    while (manum < numMetaArms) {
      vector<FullNode*> nodes = mMetaArms[manum]->GetNodes(); 
      if (mMetaArms[manum]->GetMetaArmType() == METAARM_LOOP_111 || mMetaArms[manum]->GetMetaArmType() == METAARM_LOOP_HIGH_ENERGY) {
        int n = nodes.size(); 
        while (n--) {
          if (nodes[n]) {
            nodes[n]->SetLoopNode(true); 
          }
        }
      }
      UPDATEPROGRESS(manum, numMetaArms, "TagNodes: tagging METAARM_LOOP nodes."); 
      ++manum;
    }
    COMPLETEPROGRESS(numMetaArms, "TagNodes: tagging METAARM_LOOP nodes."); 
    return;
  }
  
  //===========================================================================
  // Write a vtk fileset containing all nodes and segments. 
  void DataSet::WriteVTKFiles(void) {
    // =======================================================
    // SEGMENT FILES 
    // We go by arm by arm to save writing duplicate points everywhere. 
    uint32_t numsegfiles = ArmSegment::mArmSegments.size()/mElementsPerVTKFile + 1; 
    dbecho(0, str(boost::format("DataSet::WriteVTKFiles() will write %d segment files. \n")% numsegfiles));
    vector<FILE *> segfiles;
    vector<string> segfilenames; 
    for (uint32_t fileno = 0; fileno < numsegfiles; fileno++) {
      string filename = str(boost::format("%s/%s-segments-%04d.vtk")%mOutputDir%mOutputBasename%fileno); 
      FILE * fp = fopen(filename.c_str(), "w"); 
      if (!fp) {
        dbprintf(0, "DataSet::WriteVTKFiles(): ERROR: cannot open VTK file %s for writing\n", filename.c_str()); 
        return; 
      }
      segfilenames.push_back(filename); 
      segfiles.push_back(fp); 
    }      

    // ----------------------------------------------------------
    // WRITE THE SEGMENT FILES
    float armsperfile = (float)(Arm::mArms.size())/segfiles.size(); 
    for (uint32_t fileno = 0; fileno < segfiles.size(); fileno++) {
      FILE *segfile = segfiles[fileno]; 
      string filename = segfilenames[fileno]; 
      uint32_t firstarm =  armsperfile * fileno;
      uint32_t lastarm = armsperfile *(fileno + 1) - 1; 
      if (fileno == segfiles.size()-1) 
        lastarm = Arm::mArms.size() - 1; 
      uint32_t numarms = lastarm-firstarm+1; 
      // ----------------------------------------------------------
      // 1. Segment file: the header
      fprintf(segfile, "# vtk DataFile Version 3.0\n" );
      fprintf(segfile, "%s", str(boost::format("%s, written by paraDIS_lib. mBounds are %s, number of arms = %d\n")% filename % (FullNode::mBoundsSize.Stringify()) % (lastarm-firstarm+1)).c_str()); 
      fprintf(segfile, "ASCII\n" );
      fprintf(segfile, "DATASET POLYDATA\n" );
      
      // ----------------------------------------------------------
      // 2. Segment file: Points.  We iterate by arms because 
      //   going by segment would 
      //   cause us to write too many duplicate points.  This of course 
      //   comes at a minor expense in imperfect load balancing due to 
      //   unequal unpredictable arm lengths.  <Shrug>  
      uint32_t numnodes = 0, numsegs = 0;
      for (uint32_t armnum = 0; armnum < numarms; armnum++) {
        numnodes += Arm::mArms[armnum+firstarm]->GetNumNodes(); 
      }
      fprintf(segfile, "\nPOINTS %d float\n", numnodes);
      vector<uint32_t> nodecounts; 
      for (uint32_t armnum = 0; armnum < numarms; armnum++) {
        vector<FullNode*> nodes = Arm::mArms[armnum+firstarm]->GetNodes(); 
        nodecounts.push_back(0); 
        FullNode *previous = NULL; 
        int armsegs = 0; 
        for (vector<FullNode*>::iterator node = nodes.begin(); node != nodes.end(); node++) {
          if (*node) {
            fprintf(segfile, "%f %f %f\n", (*node)->GetLocation(0), (*node)->GetLocation(1), (*node)->GetLocation(2));
            nodecounts[armnum]++; 
            if (previous) {
              numsegs ++; 
              ++armsegs; 
            }
          }  
          previous = *node; 
        }
      }
      
      // ----------------------------------------------------------
      // 3. Segment file: Segments (connectivity)
      fprintf(segfile, "\nLINES %d %d\n", numsegs, 3*numsegs); 
      
      uint32_t nodenum = 0, previousnodenum = 0; 
      for (uint32_t armnum = 0; armnum < numarms; armnum++) {
        Arm *theArm = Arm::mArms[armnum+firstarm]; 
        if (!theArm->mNumSegments) continue; 

        uint32_t armsegs = 0; 
        if (theArm->mNumWrappedSegments) {          
          vector<FullNode *> nodes = theArm->GetNodes(); 
          FullNode *previous = NULL; 
          for (vector<FullNode *>::iterator node = nodes.begin(); node != nodes.end(); node++) {
            if (*node) {
              if (previous) {
                fprintf(segfile, "2 %d %d\n", nodenum-1, nodenum); 
                armsegs ++; 
              }
              ++nodenum; 
            }
            previous = *node;            
          }
          if (armsegs != theArm->mNumSegments) {
            dbecho(0, str(boost::format("DataSet::WriteVTKFiles(): ERROR! armsegs  %d != theArm->mNumSegments %d for arm %d\n")%armsegs%(theArm->mNumSegments)%(theArm->mArmID))); 
          }
          
        } 
        else {
          // no wrapped segments
          for (uint32_t segnum = 0; segnum < theArm->mNumSegments; segnum++) {
            fprintf(segfile, "2 %d %d\n", nodenum, nodenum+1); 
            ++nodenum; 
            armsegs ++; 
          }
          ++nodenum; 
        } 
        
        dbprintf(5, "DataSet::WriteVTKFiles(): For arm %d with %d segments, %d wrapped segments, wrote %d segments. Current node is now %d. \n", theArm->mArmID, theArm->mNumSegments, theArm->mNumWrappedSegments, armsegs, nodenum);
        if (nodenum - previousnodenum != nodecounts[armnum]) {
          dbecho(0, str(boost::format("DataSet::WriteVTKFiles(): ERROR! nodenum - previousnodenum + 1 != nodecounts[armnum] %d for arm %d\n") %(nodecounts[armnum])%(theArm->mArmID))); 
        }
        previousnodenum = nodenum; 
        
      }
      
      // ----------------------------------------------------------
      // 4. Segment file: Segment Arm numbers -- this should be much more straightforward as we do not need to reference any nodes now. 
      fprintf(segfile, "\nCELL_DATA %d\n", numsegs); 
      fprintf(segfile, "SCALARS armnum int\n"); 
      fprintf(segfile, "LOOKUP_TABLE default\n");   
      uint32_t testsegs = 0; 
      for (uint32_t armnum = 0; armnum < numarms; armnum++) {
        Arm *theArm = Arm::mArms[armnum+firstarm]; 
        for (uint32_t segnum = 0; segnum < theArm->mNumSegments; segnum++) {
          fprintf(segfile, "%d ", armnum); 
          ++testsegs;         
        }
      }
      fprintf(segfile, "\n"); 
      if (testsegs != numsegs) {
        dbecho(0, str(boost::format("DataSet::WriteVTKFiles(): ERROR!  testsegs %d != numsegs %d\n")%testsegs%numsegs)); 
      }
      // ----------------------------------------------------------
      // 5. Segment file: Segment Burgers type
      fprintf(segfile, "SCALARS burgers_type int\n"); 
      fprintf(segfile, "LOOKUP_TABLE default\n");   
      for (uint32_t armnum = 0; armnum < numarms; armnum++) {
        Arm *theArm = Arm::mArms[armnum+firstarm]; 
        uint32_t burgertype = theArm->GetBurgersType(); 
        for (uint32_t segnum = 0; segnum < theArm->mNumSegments; segnum++) {
          fprintf(segfile, "%d ", burgertype); 
          ++testsegs; 
        }        
      }
      fprintf(segfile, "\n"); 
    
      // ----------------------------------------------------------
      // 6. Segment file: Segment metaarm type
      // ----------------------------------------------------------
      fprintf(segfile, "SCALARS metaarm_type int\n"); 
      fprintf(segfile, "LOOKUP_TABLE default\n");   
      for (uint32_t armnum = 0; armnum < numarms; armnum++) {
        Arm *theArm = Arm::mArms[armnum+firstarm]; 
        uint32_t matype = theArm->GetMetaArmType(); 
        for (uint32_t segnum = 0; segnum < theArm->mNumSegments; segnum++) {
          fprintf(segfile, "%d ", matype); 
          ++testsegs; 
        }        
      }
      fprintf(segfile, "\n"); 

      // ----------------------------------------------------------
      // 7. Segment file: block (file) number
      // ----------------------------------------------------------
      fprintf(segfile, "SCALARS block_number int\n"); 
      fprintf(segfile, "LOOKUP_TABLE default\n");   
      for (uint32_t armnum = 0; armnum < numarms; armnum++) {
        Arm *theArm = Arm::mArms[armnum+firstarm]; 
        for (uint32_t segnum = 0; segnum < theArm->mNumSegments; segnum++) {
          fprintf(segfile, "%d ", fileno); 
          ++testsegs; 
        }        
      }
      fprintf(segfile, "\n"); 


      // ----------------------------------------------------------
      dbecho(1, str(boost::format("DataSet::WriteVTKFiles(): Wrote segment file %s\n")% filename)); 
      fclose(segfile); 
    }
    // ============================================================
    // Write out the VisIt file declaring the segfiles as blocks
    string visitsegfilename = str(boost::format("%s/%s-segments.visit")%mOutputDir%mOutputBasename); 
    FILE *visitsegfile = fopen(visitsegfilename.c_str(), "w"); 
    fprintf(visitsegfile, "!NBLOCKS %d\n", (int)segfiles.size()); 
    for (uint32_t fileno = 0; fileno < segfiles.size(); fileno++) {
      fprintf(visitsegfile, "%s\n", Basename(segfilenames[fileno]).c_str()); 
    }
    fclose(visitsegfile); 
    dbecho(1, str(boost::format("DataSet::WriteVTKFiles(): Wrote visit file %s\n")% visitsegfilename)); 

    
    // ============================================================
    // NODE FILES 
    // We use mElementsPerVTKFile to limit our node file size too.
    uint32_t numnodefiles = FullNode::mFullNodes.size()/mElementsPerVTKFile + 1; 
    dbecho(0, str(boost::format("DataSet::WriteVTKFiles() will write %d node files. \n")% numnodefiles));
    vector<FILE *> nodefiles;
    vector<string> nodefilenames; 
    for (uint32_t fileno = 0; fileno < numnodefiles; fileno++) {
      string filename = str(boost::format("%s/%s-nodes-%04d.vtk")%mOutputDir%mOutputBasename%fileno); 
      FILE * fp = fopen(filename.c_str(), "w"); 
      if (!fp) {
        dbprintf(0, "Error: cannot open VTK file %s for writing\n", filename.c_str()); 
        return; 
      }
      nodefilenames.push_back(filename); 
      nodefiles.push_back(fp); 
    }      
    
   // ----------------------------------------------------------
    // WRITE THE NODE FILES
    map<uint32_t, FullNode *>::iterator nodepos = FullNode::mFullNodes.begin(),
      nodeend =  FullNode::mFullNodes.end(); 
    float nodesperfile = (float)(FullNode::mFullNodes.size())/numnodefiles; 
    uint32_t firstnode = 0; 

    for (uint32_t fileno = 0; fileno < nodefiles.size(); fileno++) {
      FILE *nodefile = nodefiles[fileno]; 
      string filename = nodefilenames[fileno]; 
      uint32_t lastnode = nodesperfile *(fileno + 1.0) - 1; 
      if (fileno == nodefiles.size()-1) 
        lastnode = FullNode::mFullNodes.size()-1;
      uint32_t numnodes = lastnode - firstnode + 1; 
      
      map<uint32_t, FullNode *>::iterator firstnodepos = nodepos;  
      
      // ----------------------------------------------------------
      // 1. Node file: the header
      fprintf(nodefile, "# vtk DataFile Version 3.0\n" );
      fprintf(nodefile, "%s", str(boost::format("%s, written by paraDIS_lib. mBounds are %s\n")% filename % (FullNode::mBoundsSize.Stringify())).c_str()); 
      fprintf(nodefile, "ASCII\n" );
      fprintf(nodefile, "DATASET POLYDATA\n" );
      
      // ----------------------------------------------------------
      // 2. Node file: Points.  
      fprintf(nodefile, "\nPOINTS %d float\n", numnodes); 
      uint32_t nodecount = 0; 
      for (nodecount = 0; 
           nodecount < numnodes && nodepos != nodeend; 
           nodecount++, nodepos++) {
        fprintf(nodefile, "%f %f %f\n", nodepos->second->GetLocation(0), nodepos->second->GetLocation(1), nodepos->second->GetLocation(2));
        
      }
      if (nodecount < numnodes) {
        dbecho(0, str(boost::format("DataSet::WriteVTKFiles() PROGRAMMING ERROR: ran out of nodes.  \n")));
      }
      
      // ----------------------------------------------------------
      // 2. Node file: Vertices  
      for (uint32_t nodenum = 0; nodenum < numnodes; nodenum++) {
        fprintf(nodefile, "1 %d\n", nodenum); 
      }
      
      // ----------------------------------------------------------
      // Node file: Node Index
      nodepos = firstnodepos; 
      for (uint32_t nodenum = 0;  nodenum < numnodes; nodenum++, nodepos++) {
        fprintf(nodefile, "1 %d\n", nodepos->second->GetIndex()); 
      }
      // ----------------------------------------------------------
      // Node file: Node IsLoop 
      // ----------------------------------------------------------
      // Node file: Node Type
      // ----------------------------------------------------------
      // Node file: Node IsTypeM

      firstnode = lastnode + 1; 
      fclose(nodefile); 
    }



    return; 
  }

  //===========================================================================
  // Write a file containing all tagged nodes with their tag values. 
  void DataSet::WriteTagFile(void) {
    // First, create a fast searchable map for lookups, as our global map is ordered by global Node Index, which is not necessarily in file order any more at this point. 
    map<uint64_t, FullNode*> nodehash; 
    for (map<uint32_t, FullNode*>::iterator pos = FullNode::mFullNodes.begin(); 
         pos != FullNode::mFullNodes.end(); pos++) {
      nodehash[pos->second->Hash()] = pos->second; 
    }

    string tagfilename = mOutputDir + "/" + mOutputBasename + "-tagged.data";
    dbecho(1, str(boost::format("DataSet::WriteTagFile: Writing node tags to file %s... ")% tagfilename)); 

    
    uint32_t nodenum = 0;
    uint32_t lineno = 1; 
    char linebuf[2048]="";
    STARTPROGRESS();

    try {
      ofstream tagfile (tagfilename.c_str()); 
      if (!tagfile.good()){
        throw string("Error: cannot open tagfile for writing: ")+ tagfilename;
      }

      tagfile << "# paraDIS dump file with added tags, generated from datafile " << mDataFilename << " on " << timestamp() << " by " << GetLibraryVersionString("WriteTagFile") << endl; 

      
      ifstream datafile(mDataFilename.c_str()); 
      // =================================================
      /* Read in old header and copy to new file verbatim */ 
      while (datafile.good() && !strstr(linebuf, "Secondary lines:")) {
        datafile.getline(linebuf, 2048);  
        tagfile << linebuf << endl; 
        ++lineno; 
      }
      
      if (mFileVersion > 0 && mFileVersion < 4) {
        datafile.getline(linebuf, 2048); //should be "#"
        ++lineno; 
        if (!strstr(linebuf, "#")){
          dbprintf(0, "ERROR: Expected line:\n#\n but got:\n%s\n", linebuf);
          throw string("Error in file format"); 
        }
        tagfile << linebuf << endl; 
      }
      
       // the nodes in the file should correspond exactly to our FullNode list.  But let's be cautious the first time through and check it out. 
      while (datafile.good() && nodenum++ < mTotalDumpNodes) {
        UPDATEPROGRESS(nodenum, mTotalDumpNodes, "WriteTagFile: creating new tagged datafile"); 
        CopyNodeFromFile(lineno, nodehash, datafile, tagfile);          
      }
      COMPLETEPROGRESS(mTotalDumpNodes, "WriteTagFile: creating new tagged datafile"); 
    } catch (string err) {
      string outs = str(boost::format("Error in WriteTagFile while translating node %1% at line %2%: %3%\n") % nodenum % lineno %  err); 
      dbprintf(0, outs.c_str()); 
      throw outs; 
    }
    dbecho(1, "Successfully wrote tagged data file\n" ); 
    return;  
  }

  //===========================================================================
  // Print out all MetaArms in a simple format in a file for analysis, just for Meijie
  void DataSet::PrintMetaArmFile(void) {
    string metaArmFile = mOutputDir + "/" + mOutputBasename + ".metaarms"; 
    dbecho(0, str(boost::format("Writing metaarms to metaarm file %s... ")% metaArmFile));
    
    uint32_t a = Arm::mArms.size(); 
    while (a--) {
      Arm::mArms[a]->mSeen = false; 
    }

    FILE *armfile = fopen (metaArmFile.c_str(), "w"); 
    if (!armfile) {
      cerr << "ERROR:  Cannot open output file to write out arms" << endl;
      return;
    }
    fprintf(armfile, "DISCUSSION: \n%s\n", doctext.c_str()); 
    
    vector<boost::shared_ptr<MetaArm> >::iterator pos = mMetaArms.begin(), endpos = mMetaArms.end(); 
    uint32_t armnum = 0, metaarmcounts[7]={0}; 
    double metaarmtypelengths[7] = {0.0}, totalEPDist = 0.0; 
    uint32_t numarms = 0; 
    while (pos != endpos) {
      if ((*pos)->mTerminalNodes.size() == 2) {
        totalEPDist += (*pos)->mTerminalNodes[0]->Distance(*( (*pos)->mTerminalNodes[1]), true); 
      }
      metaarmcounts[(*pos)->mMetaArmType]++; 
      metaarmtypelengths[(*pos)->mMetaArmType] += (*pos)->mLength; 
      numarms += (*pos)->mAllArms.size(); 
      vector<Arm*>::iterator armpos = (*pos)->mAllArms.begin(), armend = (*pos)->mAllArms.end(); 
      while (armpos != armend) {
        if ((*armpos)->mSeen) {
          fprintf(stderr, "\nError in PrintMetaArmFile: arm %d in metaarm %d has mSeen == true\n", (*armpos)->mArmID, (*pos)->mMetaArmID); 
          // errexit; 
        }
        (*armpos)->mSeen = true; 
        ++armpos;
      } 
      ++pos; 
    }
    vector<Arm*>::iterator armpos = Arm::mArms.begin(), endarm = Arm::mArms.end(); 
    armnum = 0; 
    while (armpos != endarm) {   
      if ((*armpos)->mArmType != ARM_EMPTY && !(*armpos)->mSeen) {
        string msg = str(boost::format("\n******** \n   Error: arm %1% has not been seen in a metaarm.\n*********\n")%armnum); 
        cerr << msg; 
        dbprintf(0, msg.c_str()); 
        // errexit; 
      }
      ++ armnum;
      ++armpos; 
    }
    
    fprintf(armfile, "\n\n"); 
    fprintf(armfile, "METAARM SUMMARY STATISTICS \n"); 
    fprintf(armfile, "=========================================================================\n"); 
    fprintf(armfile, "%9s%28s%20s%20s\n", "TypeID", "MetaArmType", "NumMetaArms", "MetaArmLengths"); 
    int i=0; 
    while (i<4) {
      fprintf(armfile, "%9d%28s%20d%20.3f\n", 
              i, 
              MetaArmTypeNames(i).c_str(), 
              metaarmcounts[i],
              metaarmtypelengths[i]); 
      ++i; 
    }
    fprintf(armfile, "Total number of arms in metaarms: %d\n", numarms); 
    fprintf(armfile, "Total EP-Dist: %20.3f\n", totalEPDist); 
    fprintf(armfile, "=========================================================================\n"); 
    fprintf(armfile, "\n\n"); 
   
    fprintf(armfile, "\n\n"); 
    fprintf(armfile, "MONSTER NODE SUMMARY STATISTICS \n"); 
    fprintf(armfile, "=========================================================================\n"); 
    fprintf(armfile, "%s", GetMonsterNodeSummary().c_str()); 
    fprintf(armfile, "=========================================================================\n");     
    fprintf(armfile, "\n\n"); 
    
    fprintf(armfile, "\n\n"); 
    fprintf(armfile, "METAARM DETAILED STATISTICS\n"); 
    fprintf(armfile, " (for more, use the -debugfiles option and look in MetaArms-debug.txt )\n"); 
    fprintf(armfile, "=========================================================================\n"); 
    
    fprintf(armfile, "%-8s%-5s%-12s%-12s%-10s%-12s%-12s%-12s%-12s%-10s%-12s%-12s%-12s%-12s%-12s%s\n", 
            "ID", "Type", "Length", "EP-Dist", 
            "EP1-Type", "EP1-ID",   "EP1-X", "EP1-Y", "EP1-Z", 
            "EP2-Type", "EP2-ID",   "EP2-X", "EP2-Y", "EP2-Z", 
            "BurgersType","(NumArms):<Arm List>");
    pos = mMetaArms.begin();
    armnum = 0; 
    while (pos != endpos) {
      int numtermnodes = (*pos)->mTerminalNodes.size(), numNeighbors[2] = {0,0}; (void) numNeighbors;
      int numtermarms = (*pos)->mTerminalArms.size(); 
      double eplength = 0.0; 
      string ids[2]; ids[1] = "--"; 
      float loc[2][3]={{0}};
      int ntypes[2]={0}, armtypes[2] = {0,0}; (void) armtypes;
      if (numtermarms < 1 || numtermarms > 2) {
        dbprintf(0,  "WARNING: arm # %d has %d terminal arms\n", armnum, numtermarms);
        dbprintf(0,  (*pos)->Stringify(0).c_str()); 
      } else {
        armtypes[0] = (*pos)->mTerminalArms[0]->mArmType; 
        if ((*pos)->mTerminalArms.size() == 2) {
          armtypes[1] = (*pos)->mTerminalArms[1]->mArmType; 
        }
      }
      if (numtermnodes < 1 || numtermnodes > 2) {
        dbprintf(0,  "WARNING: arm # %d has %d terminal arms\n", armnum, numtermnodes);
        dbprintf(0,  (*pos)->Stringify(0).c_str()); 
      } else {
        uint32_t i = numtermnodes, numarms = (*pos)->mAllArms.size(); 
        while (i--) {
          (*pos)->mTerminalNodes[i]->GetLocation(loc[i]);
          ntypes[i] = (*pos)->mTerminalNodes[i]->GetNodeType();
          numNeighbors[i] = (*pos)->mTerminalNodes[i]->GetNumNeighborSegments();
          ids[i]  = str(boost::format("(%1%,%2%)") 
                        % (*pos)->mTerminalNodes[i]->GetNodeSimulationDomain()
                        % (*pos)->mTerminalNodes[i]->GetNodeSimulationID());  
        }
        if (numtermnodes == 2) {
          eplength = (*pos)->mTerminalNodes[0]->Distance(*( (*pos)->mTerminalNodes[1]), true); 
        }    
        string burgstring =  BurgersTypeNames((*pos)->mTerminalArms[0]->GetBurgersType()); 
        int matype = (*pos)->mMetaArmType; 
        fprintf(armfile, "%-8d%-5d%-12.3f%-12.3f%-10d%-12s%-12.3f%-12.3f%-12.3f%-10d%-12s%-12.3f%-12.3f%-12.3f%-12s(%d):< ",
                armnum, matype, (*pos)->mLength, eplength, 
                ntypes[0], ids[0].c_str(), loc[0][0],loc[0][1],loc[0][2],
                ntypes[1], ids[1].c_str(), loc[1][0],loc[1][1],loc[1][2],
                burgstring.c_str(), numarms); 
        if (!numarms) {
          fprintf(armfile, "NONE >\n"); 
        }
        else {
          i = 0; 
          while (i<numarms-1) {
            fprintf(armfile, "%d, ", (*pos)->mAllArms[i]->mArmID); 
            i++; 
          }
          fprintf(armfile, "%d >\n", (*pos)->mAllArms[i]->mArmID); 
        }
      }
      ++armnum; 
      ++pos; 
    }
    
    dbecho(1, "Successfully wrote meta arms\n");  
    return ;

  }

  //===========================================================================
  // Print out all arms in a simple format in a file for analysis, just for Meijie
  void DataSet::PrintArmFile(char *altname) {
    
    string armfilename = mOutputDir + "/" + mOutputBasename + ".arms";  
    if (altname) {
      armfilename = mOutputDir + "/" + mOutputBasename + "-" + altname + ".arms"; 
    } 
    dbecho(0, str(boost::format("Writing arms to arm file %s... ") % armfilename)); 

    FILE *armfile = fopen (armfilename.c_str(), "w"); 
    if (!armfile) {
      cerr << "ERROR:  Cannot open output file to write out arms" << endl;
      return;
    }

    fprintf(armfile, "This file is a printout of all arms.\n"); 
    fprintf(armfile, "DISCUSSION: \n%s\n", doctext.c_str()); // includes key for this file

    fprintf(armfile, "-----------------------------------------------------\n"); 

    PrintArmStats(armfile); 

    fprintf(armfile, "%-12s%-6s%-6s%-15s%-15s%-10s%-10s%-10s%-15s%-15s%-15s%-10s%-10s%-10s%-15s%-15s%-15s\n", 
            "Arm-ID", "Type", "Burg", "Length", "EP-Distance", 
            "EP1-ID", "EP1-Type", "EP1-Nbrs", "EP1-X", "EP1-Y", "EP1-Z", 
            "EP2-ID", "EP2-Type", "EP2-Nbrs", "EP2-X", "EP2-Y", "EP2-Z");
    vector<Arm*>::iterator pos = Arm::mArms.begin(), endpos = Arm::mArms.end(); 
    uint32_t armnum = 0; 
    while (pos != endpos) {   
      if ((*pos)->mArmType == ARM_EMPTY) {
        ++pos; 
        continue; 
      }
      int numtermnodes = (*pos)->mTerminalNodes.size(), numNeighbors[2] = {0};
      double eplength = 0.0; 
      float loc[2][3]={{0}};
      char ntypes[2]={'-','-'}; 
      string ids[2]; 
      
      if (numtermnodes == 2) {
        eplength = (*pos)->mTerminalNodes[0]->Distance(*( (*pos)->mTerminalNodes[1]), true); 
      }
      int i = numtermnodes; 
      while (i--) {
        (*pos)->mTerminalNodes[i]->GetLocation(loc[i]);
        ntypes[i] = (*pos)->mTerminalNodes[i]->IsTypeM()? 'M':'N';
        numNeighbors[i] = (*pos)->mTerminalNodes[i]->GetNumNeighborSegments();
        ids[i]  = str(boost::format("(%1%,%2%)") 
                      % (*pos)->mTerminalNodes[i]->GetNodeSimulationDomain()
                      % (*pos)->mTerminalNodes[i]->GetNodeSimulationID());  
      }
      
      fprintf(armfile, "%-12d%-6d%-6d%-15f%-15f%-10s%-10c%-10d%-15f%-15f%-15f%-10s%-10c%-10d%-15f%-15f%-15f\n",
              (*pos)->mArmID, (*pos)->mArmType, (*pos)->GetBurgersType(), (*pos)->mArmLength, eplength, ids[0].c_str(), ntypes[0], numNeighbors[0], loc[0][0], loc[0][1], loc[0][2], ids[1].c_str(), ntypes[1], numNeighbors[1], loc[1][0], loc[1][1], loc[1][2]); 
      
      
      ++armnum; 
      ++pos; 
    }    
    int s = Arm::mArms.size();
    fprintf(armfile, "Size of Arm::mArms: %d\n",  s); 
    fprintf(armfile, "Total printed non-EMPTY arms: %d\n",  armnum); 
    dbecho(0, "Wrote arms file \n"); 
    return; 
  }

  //===========================================================================
  void DataSet::ClassifyArms(void) {
    dbprintf(2, "ClassifyArms starting...\n");
    STARTPROGRESS()   ;
    int armNum=0, totalArms = Arm::mArms.size(); 
    vector<Arm*>::iterator armpos = Arm::mArms.begin(), armend = Arm::mArms.end(); 
    while (armpos != armend) {      
      (*armpos)->Classify(); 
      ++armpos; ++armNum; 
      UPDATEPROGRESS(armNum, totalArms, "Classify arms... "); 
    }
    COMPLETEPROGRESS(totalArms, "Classify arms... "); 
    
    return; 
  }
  
  //===========================================================================
  double DataSet::ComputeArmLengths(void) {
    dbprintf(2, "ComputeArmLengths starting...\n");
    STARTPROGRESS()   ;
    double totalLength = 0; 
    int armNum=0, totalArms = Arm::mArms.size(); 
    vector<Arm*>::iterator armpos = Arm::mArms.begin(), armend = Arm::mArms.end(); 
    while (armpos != armend) {      
      //totalLength += (*armpos)->ComputeLength(); 
      totalLength += (*armpos)->mArmLength; 
#ifdef DEBUG_ARMLENGTHS
      if (totalLength != (*armpos)->ComputeLength()) {
        dbprintf(0, "ERROR: DataSet::ComputeArmLengths(arm %d): totalLength != (*armpos)->ComputeLength()\n", (*armpos)->mArmID); 
      }
#endif
      ++armpos; ++armNum; 
     UPDATEPROGRESS(armNum, totalArms, "Compute arm lengths... "); 
    }
    COMPLETEPROGRESS(totalArms, "Compute arm lengths... "); 
    return totalLength; 
  }
  
  
  //==========================================================================
  /*! 
    To be used with for_each() to delete all nodes in the container
  */ 
  void DeleteNode(FullNode *node) {
    delete node; 
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
  void DataSet::CreateNodeSegmentVectors(void) {  
    uint32_t index = 0; 
    for (map<uint32_t, FullNode *>::iterator pos = FullNode::mFullNodes.begin(); pos != FullNode::mFullNodes.end(); pos++, index++) {
      pos->second->SetIndex(index); 
      FullNode::mFullNodeVector.push_back(pos->second); 
    }

    index = 0; 
    for (map<uint32_t, ArmSegment *>::iterator pos = ArmSegment::mArmSegments.begin(); pos != ArmSegment::mArmSegments.end(); pos++, index++) {
      pos->second->SetIndex(index); 
      ArmSegment::mArmSegmentVector.push_back(pos->second); 
    }
    
    return; 
  }

  //=========================================================================
  void DataSet::ComputeNodeTypes(void) {
    for (map<uint32_t, FullNode*>::iterator pos = FullNode::mFullNodes.begin(); pos !=  FullNode::mFullNodes.end(); pos++) {
      pos->second->ComputeNodeType(); 
    }
  }

  //===========================================================================
  void DataSet::FindMetaArms(void){
    for (vector<Arm*>::iterator currentArm = Arm::mArms.begin(); currentArm != Arm::mArms.end(); ++currentArm) {
      (*currentArm)->mSeen = false; 
    }

    uint32_t numMetaArms = 0, numArms = 0, totalArms = Arm::mArms.size(); 
    STARTPROGRESS(); 
    dbprintf(4, "FindMetaArms: %s\n", datestring()); 
    for (vector<Arm*>::iterator currentArm = Arm::mArms.begin(); currentArm != Arm::mArms.end(); ++currentArm, ++numArms) {
      if ( ! (*currentArm)->mSeen  && 
           ! (*currentArm)->isTypeUnknown() && 
           (*currentArm)->mArmType != ARM_EMPTY
        ) {
        boost::shared_ptr<MetaArm> metaArmSPtr = boost::make_shared<MetaArm>();
        metaArmSPtr->mMetaArmID = mMetaArms.size();
        metaArmSPtr->FindEndpoints(*currentArm); 
        if (metaArmSPtr->mMetaArmType != METAARM_UNKNOWN) {
          dbprintf(4, "Adding meta arm %d with seed %d, terminal arms <", mMetaArms.size(), (*currentArm)->mArmID); 
          int numterms = metaArmSPtr->mTerminalArms.size(); 
          int tnum = 0; 
          for ( tnum = 0; tnum < numterms; tnum++) {
            dbprintf(5, "%d",metaArmSPtr->mTerminalArms[tnum]->mArmID); 
            if (tnum < numterms-1) {
              dbprintf(5, ", "); 
            } 
          }
          if (!tnum) {
            dbprintf(5, "NONE"); 
          }
            
          dbprintf(4, ">, and arms <"); 
          int nummaarms = metaArmSPtr->mAllArms.size(); 
          int maarm = 0; 
          for ( maarm = 0; maarm < nummaarms; maarm++) {
            dbprintf(5, "%d",metaArmSPtr->mAllArms[maarm]->mArmID); 
            if (maarm < nummaarms-1) {
              dbprintf(5, ", "); 
            } 
          }
          if (!maarm) {
            dbprintf(5, "NONE"); 
          }
          dbprintf(5, ">\n"); 
          metaArmSPtr->mMetaArmID = mMetaArms.size();
          mMetaArms.push_back(metaArmSPtr); 
          numMetaArms++;      
        }      
      } else {
        dbprintf(4, "Skipping currentArm %d (type %d) as it was either seen (%d) and was type 111 or an unknown arm, or else FindEndpoints could not handle it.\n", (*currentArm)->mArmID, (*currentArm)->mArmType, (*currentArm)->mSeen);  
      }
      UPDATEPROGRESS(numArms, totalArms, str(boost::format("FindMetaArms: %1% MetaArms created")%numMetaArms)); 
    }
    COMPLETEPROGRESS(totalArms, str(boost::format("FindMetaArms: %1% MetaArms created")%numMetaArms)); 

    for (vector<Arm*>::iterator currentArm = Arm::mArms.begin(); currentArm != Arm::mArms.end(); ++currentArm) {
      if (!(*currentArm)->mSeen && (*currentArm)->mArmType != ARM_EMPTY) {
        uint32_t id = (*currentArm)->mArmID;
        dbprintf(0, "\n\nError: arm %d has not been seen!\n", id); 
        errexit; 
      }
      (*currentArm)->mSeen = false; 
    }
    
    return; 
  }

  //===========================================================================
  bool DataSet::Test(void) {
    dbprintf(0, "DataSet::Test(): called\n"); 
    
    vector<uint32_t>segsperarm(Arm::mArms.size(), 0), segspermetaarm(mMetaArms.size(), 0);
    
     //================================================================
   dbprintf(0, "DataSet::Test(): enumerating segsperarm\n"); 
   for (map<uint32_t, ArmSegment*>::iterator segpos = ArmSegment::mArmSegments.begin(); segpos != ArmSegment::mArmSegments.end(); ++segpos) {   
      ArmSegment * seg = segpos->second; 
      if (!seg->mParentArm) {
        dbprintf(0, "DataSet::Test(): seg %d has NULL parent \n", seg->mSegmentID); 
        return false; 
      }
      if (!seg->mParentArm->GetParentMetaArm()) {
        dbprintf(0, "DataSet::Test(): seg %d has NULL parent metaarm \n", seg->mSegmentID);  
        return false; 
      }
      segsperarm[seg->mParentArm->mArmID]++; 
      segspermetaarm[seg->mParentArm->GetParentMetaArm()->GetMetaArmID()]++; 
    }
    
    
    //================================================================
    dbprintf(0, "DataSet::Test(): checking arms \n"); 
    bool success = true; 
    uint32_t numArms = 0, numsegs = 0, numnodes = 0; 
    for (vector<Arm*>::iterator arm = Arm::mArms.begin(); arm != Arm::mArms.end(); ++arm, ++numArms) {
      vector<ArmSegment*> segs = (*arm)->GetSegments(); 
      numsegs += segs.size(); 
      if (segs.size() != segsperarm[numArms]) {
        dbprintf(0, "DataSet::Test(): numsegs %d != segsperarm[%d] (%d)\n", segs.size(), numArms, segsperarm[numArms]); 
        int segnum = 0; 
        if (segs.size() < 5){
          dbprintf(0, "Searching for missing segment ...\n"); 
          for (vector<ArmSegment*>::iterator seg = segs.begin(); seg != segs.end(); seg++, ++segnum) {
            dbprintf(0, "armseg %d: %s\n", segnum, (*seg)->Stringify(0).c_str()); 
          }
          for (map<uint32_t, ArmSegment*>::iterator seg = ArmSegment::mArmSegments.begin(); seg != ArmSegment::mArmSegments.end(); ++seg) {   
            if (seg->second->mParentArm == *arm) {
              vector<ArmSegment*>::iterator pos = find(segs.begin(), segs.end(), seg->second); 
              if (pos == segs.end()) {
                dbprintf(0, "Segment not in arm but in data set: %s\n", seg->second->Stringify(0).c_str()); 
              }
            }
          }
        }
        
        success = false;
      }

      vector<FullNode *> nodes = (*arm)->GetNodes(); 
      // avoid double-counting: 

      for (vector<FullNode *>::iterator node = nodes.begin(); node != nodes.end(); node++) {
        if ((*node) && !(*node)->mSeen) {
          numnodes++; 
          (*node)->mSeen = true; 
        }
      }      
    }
    if (!success) return false; 

    if (numsegs != ArmSegment::mArmSegments.size()) {
      dbprintf(0, "DataSet::Test(): numsegs in Arms %d != ArmSegment::mArmSegments.size() %d \n", numsegs, ArmSegment::mArmSegments.size()); 
      return false; 
    }

    //================================================================
    /*!
      Checking metaarm segments counts is wrong -- for VisIt, a 'MetaArm' always has only a single segment, connecting its endpoints.  The entire metaarm is returned only if it is a LOOP. 
    
    dbprintf(0, "DataSet::Test(): checking meta-arms \n"); 
     uint32_t manum = 0;
    numsegs = 0; 
    for (vector<boost::shared_ptr<MetaArm> >::iterator metaarm = mMetaArms.begin(); metaarm != mMetaArms.end(); ++metaarm, ++manum) {
      uint32_t masegs = (*metaarm)->GetNumSegments(false); 
      uint32_t shouldbe = segspermetaarm[(*metaarm)->GetMetaArmID()]; 
      if (masegs != shouldbe) {
        dbprintf(0, "DataSet::Test(): numsegs %d != segspermetaarm[%d] %d \n", masegs, (*metaarm)->GetMetaArmID(), shouldbe); 
        return false; 
      }
      numsegs += masegs; 
    }
     if (numsegs != ArmSegment::mNumArmSegments) {
      dbprintf(0, "DataSet::Test(): numsegs in meta MetaArms %d != ArmSegment::mArmSegments.size() %d \n", numsegs, ArmSegment::mArmSegments.size()); 
      return false;    
    }
    */
    
    //================================================================
    dbprintf(0, "DataSet::Test(): checking arm segments \n"); 
    if (numnodes != FullNode::mFullNodes.size()) {
      dbprintf(0, "DataSet::Test(): numnodes %d != FullNode::mFullNodes.size() %d.  Searching for extra node...\n", numnodes, FullNode::mFullNodes.size()); 
      for (map<uint32_t, FullNode*>::iterator node = FullNode::mFullNodes.begin(); node != FullNode::mFullNodes.end(); ++node) {           
        if (!node->second->mSeen) {
          dbprintf(0, "DataSet::Test(): unseen node: %s\n", node->second->Stringify(0, false).c_str());
        }
      } 
          
      return false; 
    }
      
    dbprintf(0, "\n============================================\n"); 
    dbprintf(0, "DataSet::Test(): All tests PASSED.\n"); 
    dbprintf(0, "============================================\n\n"); 
    return true; 
  }

  //===========================================================================
  void DataSet::ReadData(std::string filename, bool createVectors){
    dbprintf(1, "ReadData called with debug level %d\n", dbg_isverbose()); 
    try { 
      if (filename != "") {
        mDataFilename = filename; 
      }
      int dblevelSave = dbg_isverbose(); 
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
          cout << "mSubspaceMin: " << mSubspaceMin.Stringify(0) << endl; 
          cout << "mSubspaceMax: " << mSubspaceMax.Stringify(0) << endl; 
        }
        mNumProcs --; 
      }
#endif

      /*!
        Announce to the nodes what their bounds are for wrapping
      */ 
      FullNode::SetBounds(mSubspaceMin, mSubspaceMax); 
      
      if (0 && dblevelSave > 2) 
        SetVerbosity(2); 

      CreateMinimalNodes(); 

      ClassifyMinimalNodes(); 

      if (mDoDebugOutput) {
        DebugPrintMinimalNodes(); 
      }

      CullAndResortMinimalNodes(); 

      CreateFullNodesAndArmSegments(); 

      SetVerbosity(dblevelSave); 
      
      BuildArms(); 
 
      if (mDoStats && mDoDebugOutput) {
        PrintArmFile((char *)"debug-before-decomp"); 
      }

      FullNode::PrintAllNodeTraces("before-decomposition"); 

      DecomposeArms(); 

      ComputeNodeTypes(); 
 
      ClassifyArms(); // This also computes their lengths...


#if INSANE_DEBUG
      if (mDoDebugOutput) {
        DebugPrintFullNodes("NodesBeforeDeletion");       
      }
#endif
 
      WrapBoundarySegments();  
  
      FindMetaArms(); 
 
      TagNodes(); // for Meijie

      if (mDoDebugOutput) {
        DebugPrintMetaArms(); 
        DebugPrintArms(); 
        DebugPrintFullNodes(); 
      }

      // make nodes and segments index sequential so that they can be gotten by e.g. VisIt etc.  
      if (createVectors) {
        CreateNodeSegmentVectors(); 
      }

    } catch (string err) {
      cerr << "An exception occurred" << endl; 
      throw string("Error in DataSet::ReadData reading data from file ")+mDataFilename+":\n" + err; 
    }
    if (mDoStats) {
      PrintMetaArmFile(); 
      PrintArmFile(); 
    }

    if (mDoTagFile) {
      WriteTagFile();
    }

    if (mDoVTKFile) {
      WriteVTKFiles();
    }

    dbprintf(1, "ReadData complete\n"); 
    return; 
  } 
  
  

  
} // end namespace paraDIS 


