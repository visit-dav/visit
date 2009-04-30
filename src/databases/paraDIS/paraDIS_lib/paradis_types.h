/* $Id: paradis_types.h,v 1.3 2009/04/30 01:23:41 rcook Exp $ 
 $File$
*/
#ifndef LIBPARADIS_TYPES_H 
#define LIBPARADIS_TYPES_H

#include <vector>
#include <string>
#include <math.h>
#include <HTSFile.h>
#include "RC_c_lib/debugutil.h"
#include "RC_cpp_lib/RangeList.h"

using namespace std;

#define HEADER_ID "$Revision: 1.3 $"
//===========================
#define FILTER_BUTTERFLIES (-1)
#define FILTER_MONSTERS (-2)
#define DOUBLE_UNDEFINED (-42.121867351)
namespace ParaDIS {

  //============================================================================


  //======================
  struct segment; 
  
  //=======
  typedef int8_t MonsterInfo; 
#define MONSTER_UNKNOWN -1 
#define MONSTER_YES -4 
#define MONSTER_MIXED -44 
#define MONSTER_NO 0
  
  typedef int8_t  ButterflyInfo; 
#define BUTTERFLY_UNKNOWN -1
#define BUTTERFLY_YES -3
#define BUTTERFLY_MONSTER -33
#define BUTTERFLY_NO 0
  
  typedef int8_t superBoolean;
#define NO (0)
#define YES (1)
#define UNKNOWN (-1)
  
  typedef int8_t  BoundaryStatus; 
#define INBOUNDS (0)
#define OUTOFBOUNDS (1)
#define BOUNDARY (2)
  
  //=======
  struct node {
    node() { clear(); }
    
    void clear(void) {
      terminalNode = 0;
      numNeighbors=0;
      //id=0;
      loc[0] = loc[1] = loc[2] = 0.0;
      filtered = drawn = 0;
      butterflyEnd = BUTTERFLY_UNKNOWN; 
      fourArmedMonster = MONSTER_UNKNOWN;
      neighbors.clear();
      monstrosity = 1.0; //1.0 is the minimum
      distance_from_monster = 1000000;     
      _isButterflyOrMonster = UNKNOWN; 
    }
    bool InBounds(double min[3], double max[3]) {
      if (loc[0] < min[0] || loc[0] > max[0] ||
          loc[0] < min[1] || loc[1] > max[1] ||
          loc[0] < min[2] || loc[2] > max[2] ) {
        return false;
      }
      return true; 
    }
    MonsterInfo CheckFourArmedMonster(void);
    void ComputeButterflyOrMonster(void);
    void MarkTerminal(void) {if (!terminalNode) {
        _numTerminalNodes++;
        terminalNode = 1; 
      }
      return; 
    }
    long NumTerminalNodes(void) { return _numTerminalNodes;}
    u_int32_t //id, //global ID -- not useful -- forget it
    domain, domainID; //which domain and which node within that domain
    //wrap the node to newloc, then find the distance between us and newloc
    double distance(node *other, float newloc[3], float boundsize[3]);
    void dbprint(int dblevel);
    
    char terminalNode; 
    int16_t numNeighbors;
    BoundaryStatus _boundaryStatus; 
    ButterflyInfo butterflyEnd;//3-neighbor node at end of butterfly bodies
    MonsterInfo  fourArmedMonster; //four armed monster, each arm has unique burgers
    superBoolean _isButterflyOrMonster; 
    float loc[3];
    float monstrosity;
    float distance_from_monster;
    vector<segment *> neighbors;
    static u_int32_t _numTerminalNodes; 
  };
  //======================================


  struct arm;  // forward declaration
  struct segment {
    segment(){
      endpoints[0] = endpoints[1] = NULL;
      parentArm = NULL; 
      segnum = numsegs++; 
      burgersValue = 0;
      length = 0;
#ifdef DEBUG
      nx=ny=nz= 0; 
#endif
      drawn = filtered = wrapped = 0;
      monstrosity=1.0;
      distance_from_monster = 1000000; 
    }
    
    int Category(float num) {
      if (!num)
        return 0;
      else if (fabs(num) > 1.15 && fabs(num) < 1.16)
        return 1;
      else if (num > 0.577 && num < 0.578)
        return 2;
      else if (num < -0.577 && num > -0.578)
        return 3;
      dbprintf(2, "\n\n********************************\n");
      dbprintf(2, "WARNING: Weird value %g encountered in Category\n", num);
      dbprintf(2, "\n********************************\n\n");
      return 4;
    }
    
    void ComputeBurger(float burg[3]) {
      burgersValue = 0;
      // type 100 nodes are valued 1-3, and type 111 nodes are valued 4-7
      int valarray[3] = 
        {Category(burg[0]), Category(burg[1]), Category(burg[2])};
      if (valarray[0] == 1 && valarray[1] == 0 && valarray[2] == 0)
        burgersValue = 1;
      else if (valarray[0] == 0 && valarray[1] == 1 && valarray[2] == 0)
        burgersValue = 2;
      else if (valarray[0] == 0 && valarray[1] == 0 && valarray[2] == 1)
        burgersValue = 3;
      else if ((valarray[0] == 2 && valarray[1] == 2 && valarray[2] == 2) ||
               (valarray[0] == 3 && valarray[1] == 3 && valarray[2] == 3))
        burgersValue = 4;
      else if ((valarray[0] == 2 && valarray[1] == 2 && valarray[2] == 3) ||
               (valarray[0] == 3 && valarray[1] == 3 && valarray[2] == 2))
        burgersValue = 5;
      else if ((valarray[0] == 2 && valarray[1] == 3 && valarray[2] == 2) ||
               (valarray[0] == 3 && valarray[1] == 2 && valarray[2] == 3))
        burgersValue = 6;
      else if ((valarray[0] == 2 && valarray[1] == 3 && valarray[2] == 3) ||
               (valarray[0] == 3 && valarray[1] == 2 && valarray[2] == 2))
        burgersValue = 7;
      else {
        burgersValue = 8;
        dbprintf(3, "\n\n********************************\n");
        dbprintf(3, "Warning: segment %d has odd value/category: value = %d, burg = (%f, %f, %f)\n", segnum, burgersValue, burg[0], burg[1], burg[2]); 
        dbprintf(3, "\n********************************\n\n");
      }
    }
    
    
    void dbprint(int dblevel);     
    
    node *endpoints[2]; 
    arm *parentArm; 
    u_int32_t unk_domain, unk_domainID; //so we know when we see the next node
    u_int8_t filtered, drawn, wrapped; 
    u_int8_t burgersValue; 
    BoundaryStatus _boundaryStatus; 
    int segnum; // zero based
    //float burg[3], 
#ifdef DEBUG
    float nx, ny, nz;
#endif
    float  monstrosity; // 1.0 = right next to M node or far from N node, 8.0 = right next to N node
    float distance_from_monster; 
    float length; // note that this is the actual "wrapped" length
    
    static u_int32_t numsegs; 
  };
  
  //=======================================
  struct nodeDomain {
    nodeDomain() {nodes.resize(20);}
    vector<node*> nodes;
  };
  
  //===================================================
  typedef char mn_type; 
#define MN_UNKNOWN 0
#define MN_MONSTER_MONSTER 1 //connections between "strong" monsters == "M-M"
#define MN_MONSTER_NONMONSTER 2 //"M-N" (only one is monster or butterfly)
#define MN_NONMONSTER_NONMONSTER 3 // "N-N" (neither is butterfly or monster)
#define MN_NOTA 4 // None Of The Above
  
  typedef char arm_type;
#define ARM_UNKNOWN 0
#define ARM_UNINTERESTING 1
#define ARM_LOOP 2
#define ARM_MM_111 3 
#define ARM_MN_111 4
#define ARM_NN_111 5 
#define ARM_MM_100 6
#define ARM_MN_100 7
#define ARM_NN_100 8
  
  //forward declaration:  
  struct DataSet; 
  
  struct arm {
    public:
    arm(DataSet *df) { _dfp = df;  Clear(); }
    void SetFiltered(int filterval);
    void ComputeWeight(void);
    void ComputeArmType(void); 
    void CheckForButterflyBody(void);
    void PrintInfo(int dblevel);
    int BuildArm(segment *);
    segment *BuildArmPrivate(segment *theSegment, node *&ignore, segment *initial);
    
    void ComputeSegmentMonstrosities(void); 
    void ComputeLength(void);
    void ComputeMNType(void);
    void Clear(void) {
      _segments.clear(); _terminalSegments.clear(); 
      _terminalNodes.clear(); _interiorNodes.clear();     
      _butterflyBody=BUTTERFLY_UNKNOWN;
      _armtype=ARM_UNKNOWN;
      _mnType = MN_UNKNOWN; 
      _length = -1.00; 
      _armNumber = _burgers = -1;
      filtered=drawn = 0;
      _weight = -1; 
    }
    vector<segment *> _segments, _terminalSegments; 
    vector<node*> _terminalNodes; 
    vector<node*> _interiorNodes;
    int8_t filtered, drawn;
    int8_t _burgers; 
    BoundaryStatus _boundaryStatus; 
    ButterflyInfo _butterflyBody; 
    arm_type _armtype; 
    mn_type _mnType; 
    float _weight; 
    u_int32_t _armNumber;
    double _length; 
    private:
    static u_int32_t _numArms;
    static DataSet *_dfp; 
    private:
    void BuildArmPrivate(node *theNode, node *previous);
    void ComputeSegmentMonstrositiesHelper(segment *seg, node *previous, 
                                           int segnum, int numsegs, 
                                           double distance);
    
  };
  
  //==================
  struct DataSet {
    DataSet() {_nodes=NULL; _numnodes = 0; _reportStats = 1; }
    ~DataSet() { if (_nodes) delete[] _nodes; }
    
    void resize(u_int32_t numnodes) {
      if (numnodes == _numnodes)
        return;
      _numnodes = numnodes;
      if (_nodes)  {
        delete[] _nodes; 
        _nodes = NULL; 
      }
      if (!_numnodes) 
        return;
      _nodes = new node[_numnodes];
      if (!_nodes) throw string("Could not allocate _nodes in DataSet::resize"); 
    }
    void GetBounds(char *filename) ;
    void ParseDumpFile(char *filename) ;
    node *FindNodeByID(int id) { return &_nodes[id-1];}
    void CreateArms(void);
    void FilterArms(void);
    void UndrawArms(void);
    void WriteArmStats(char *armstatsfile);
    void ReportDrawnStats(void); 
    void DumpToPovray(char *outfile);
    void DumpToHTS(hts_flags &hflags) ;
    filter _filter; 
    float min[3], max[3], size[3];
    node *_nodes; 
    vector<segment *> _segments; 
    vector<arm *> theArms;
    u_int32_t _numnodes; 
    u_int32_t _reportStats; 
    u_int32_t _maxChunkTriangles; 
  };
  
  
} ; //end namespace ParaDIS 

#endif

