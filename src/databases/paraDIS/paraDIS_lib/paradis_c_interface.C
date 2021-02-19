/*!
  Implements the pure C interface to paraDIS lib, but is a C++ file.  
*/ 
#include "paradis.h"
#include "paradis_c_interface.h"
using namespace paraDIS; 
extern "C" {
  static paraDIS::DataSet *pci_gDataSet = NULL; 

#define MAX_METAARM_NODES 2048
  static float gNodeLocationBuffer[MAX_METAARM_NODES*3+20];   

  void *paraDIS_init(void *ds) {
    if (ds) {
      pci_gDataSet = reinterpret_cast<paraDIS::DataSet *>(ds); 
    }
    if (!pci_gDataSet)  {
      pci_gDataSet = new paraDIS::DataSet;
    }
    return (void*)pci_gDataSet; 
  }

  void paraDIS_close(void) {
    if (pci_gDataSet) delete pci_gDataSet; 
    pci_gDataSet = NULL;
  }

  void paraDIS_SetVerbosity(int level, const char *filename){
    paraDIS_init(NULL);    
    pci_gDataSet->SetVerbosity(level, filename); 
    return; 
  }
  
  void paraDIS_SetThreshold(double threshold){
    paraDIS_init(NULL);
    pci_gDataSet->SetThreshold(threshold); 
    return; 
  }

  void paraDIS_SetOutputDir(const char *dir){
    paraDIS_init(NULL);
    pci_gDataSet->SetOutputDir(dir); 
  }

  void paraDIS_EnableDebugOutput(int truth){
    paraDIS_init(NULL);
    pci_gDataSet->EnableDebugOutput(truth); 
    return; 
  }
  
  void paraDIS_EnableStatsOutput(int truth){
    paraDIS_init(NULL);
    pci_gDataSet->EnableStatsOutput(truth); 
    return; 
  }
  
  void paraDIS_EnableTagFileOutput(int truth){
    paraDIS_init(NULL);
    pci_gDataSet->EnableTagFileOutput(truth); 
    return; 
  }
  
  void paraDIS_EnableVTKFileOutput(int truth){
    paraDIS_init(NULL);
    pci_gDataSet->EnableVTKFileOutput(truth); 
    return; 
  }
  
  void paraDIS_SetDataFile(const char *filename){
    paraDIS_init(NULL);
    pci_gDataSet->SetDataFile(filename);    
    return; 
  } 
  
   int paraDIS_GetBounds(double xxyyzz[6]){
    paraDIS_init(NULL);
    rclib::Point<float> datamin, datamax;   
    try {
      pci_gDataSet->GetBounds(datamin, datamax); 
    } catch (string err) {
      dbprintf(1, "%s\n", err.c_str()); // << endl;     
      return 0;     
    }
    int i=3; while (i--) {
      xxyyzz[2*i] = datamin[i]; 
      xxyyzz[2*i+1] = datamax[i]; 
    }
    return 1; 
  }
   
  
  void paraDIS_SetSubspace(double xxyyzz[6]){
    paraDIS_init(NULL);
    rclib::Point<float> datamin, datamax;
    int i=3; while (i--) {
      datamin[i] = xxyyzz[2*i]; 
      datamax[i] = xxyyzz[2*i+1]; 
    }
    pci_gDataSet->SetSubspace(datamin, datamax); 
    return; 
  }
   
  
  void paraDIS_SetProcNum(int procnum, int numprocs){
    paraDIS_init(NULL);
    pci_gDataSet->SetProcNum(procnum, numprocs); 
    return; 
  }
  
  
  void paraDIS_ReadData(void){
    paraDIS_init(NULL);
    pci_gDataSet->ReadData("", true); 
    return; 
  }
  
  void paraDIS_PrintArmStats(void) {
    paraDIS_init(NULL); 
    pci_gDataSet->PrintArmStats(stderr); 
  }

  uint32_t paraDIS_GetNumNodes(void){
    paraDIS_init(NULL);
    return pci_gDataSet->GetNumNodes();  
  }
  
  int paraDIS_TestNode(uint32_t nodenum) {
    float location[3] ; 
    paraDIS_GetNodeLocation(nodenum, location); 
    float f = location[2]; 
    location[0] = f; // look for segfaults
    f=paraDIS_GetNodeHash(nodenum); 
    f=paraDIS_GetNodeType(nodenum); 
    f=paraDIS_NodeIsLoop(nodenum); 
    f=paraDIS_NodeIsTypeM(nodenum); 
    f=paraDIS_NodeIsTypeN(nodenum); 
    return pci_gDataSet->GetNode(nodenum)->Test();       
  }

  void paraDIS_printNodeVerbose(uint32_t nodenum){
    paraDIS_init(NULL);
    cout << pci_gDataSet->GetNode(nodenum)->Stringify(0) << endl; 
  }

  void paraDIS_GetNodeLocation(uint32_t nodenum, float loc[3]){    
    paraDIS_init(NULL);
    pci_gDataSet->GetNode(nodenum)->GetLocation(loc); 
    return; 
  }
  
  int8_t paraDIS_GetNodeType(uint32_t nodenum){
    paraDIS_init(NULL);
    return pci_gDataSet->GetNode(nodenum)->GetNodeType(); 
  }  
    
  int8_t paraDIS_NodeIsLoop(uint32_t nodenum){
    paraDIS_init(NULL);
    return pci_gDataSet->GetNode(nodenum)->IsLoopNode()?1:0; 
  }  
    
  int8_t paraDIS_NodeIsTypeM(uint32_t nodenum){
    paraDIS_init(NULL);
    return pci_gDataSet->GetNode(nodenum)->IsTypeM()?1:0; 
  }  
    
  int8_t paraDIS_NodeIsTypeN(uint32_t nodenum){
    paraDIS_init(NULL);
    return pci_gDataSet->GetNode(nodenum)->IsTypeN()?1:0; 
  }  
    
  int8_t paraDIS_GetNumNodeNeighbors(uint32_t nodenum){
    paraDIS_init(NULL);
    return pci_gDataSet->GetNode(nodenum)->GetNumNeighborSegments(); 
  }  

  boost::int64_t  paraDIS_GetNodeHash(uint32_t nodenum){
    paraDIS_init(NULL);
    return pci_gDataSet->GetNode(nodenum)->Hash(); 
  }  
  
  uint32_t paraDIS_GetNumArmSegments(void){
     paraDIS_init(NULL);
   return pci_gDataSet->GetNumArmSegments(); 
  }
  
  void paraDIS_printArmSegmentVerbose(uint32_t segnum){
    paraDIS_init(NULL);
    cout << pci_gDataSet->GetArmSegment(segnum)->Stringify(0) << endl; 
  }

  int8_t paraDIS_GetSegmentBurgersType(uint32_t segmentnum) {
    paraDIS_init(NULL);
    return pci_gDataSet->GetArmSegment(segmentnum)->GetBurgersType(); 
  }  
  
  int32_t paraDIS_GetSegmentSimulationIndex(uint32_t segmentnum) {
    return pci_gDataSet->GetArmSegment(segmentnum)->mSegmentID;
  }
  
  int32_t paraDIS_GetSegmentArmID(uint32_t segmentnum) {
    paraDIS_init(NULL);
    return pci_gDataSet->GetArmSegment(segmentnum)->GetArmID(); 
  }

  int32_t paraDIS_GetSegmentMetaArmID(uint32_t segmentnum) {
    paraDIS_init(NULL);
    return pci_gDataSet->GetArmSegment(segmentnum)->GetMetaArmID(); 
  }

  int8_t paraDIS_GetSegmentMetaArmType(uint32_t segmentnum) {
    paraDIS_init(NULL);
    return pci_gDataSet->GetArmSegment(segmentnum)->GetMetaArmType(); 
  }

  int8_t paraDIS_GetSegmentMNType(uint32_t segmentnum) {
    paraDIS_init(NULL);
    return pci_gDataSet->GetArmSegment(segmentnum)->GetMNType(); 
  }  
  
  int8_t paraDIS_GetSegmentDuplicates(uint32_t segmentnum) {
    return pci_gDataSet->GetArmSegment(segmentnum)->mNumDuplicates;
  }

  int paraDIS_TestSegment(uint32_t segnum) {
    int index0, index1;
    index0 = paraDIS_GetEndpointIndex(segnum, 0); 
    index1 = paraDIS_GetEndpointIndex(segnum, 1); 
    (void) index0; (void) index1;
    return true; 
  }

  int32_t paraDIS_GetEndpointIndex(uint32_t segmentnum, int endpointnum){
    paraDIS_init(NULL);
    return pci_gDataSet->GetArmSegment(segmentnum)->GetNodeIndex(endpointnum);
  }

  uint32_t paraDIS_GetNumMetaArms(void) {
    paraDIS_init(NULL);
    return pci_gDataSet->GetNumMetaArms(); 
  }
  
  int32_t paraDIS_GetMetaArmID(uint32_t metaArmNum) {
    paraDIS_init(NULL);
    return pci_gDataSet->GetMetaArm(metaArmNum)->mMetaArmID; 
  }

  int8_t paraDIS_GetMetaArmType(uint32_t metaArmNum) {
    paraDIS_init(NULL);
    return pci_gDataSet->GetMetaArm(metaArmNum)->GetMetaArmType(); 
  }  
  
  uint32_t paraDIS_GetMetaArmNumSegments(uint32_t metaArmNum, bool wrapEndpoints) {
    return pci_gDataSet->GetMetaArm(metaArmNum)->GetNumSegments(wrapEndpoints);
  }

  /*!
    Place a pointer to gNodeLocationBuffer in *opoints.  
    gNodeLocationBuffer is a string of floats.  Three floats in a row are
    a location, and two consecutive locations are assumed to be joined by 
    an ArmSegment.  Two special values are reserved: 
    WRAPPED_NODE == "wrapped node": do not connect previous node with next node
    END_OF_NODELIST == "END" of list.  
    Return number of points in buffer. 
  */ 
  float *paraDIS_GetMetaArmPoints(uint32_t metaArmNum, bool wrapEndpoints) {
    dbprintf(5, "paraDIS_GetMetaArmPoints(%d)\n", metaArmNum);
    paraDIS_init(NULL);
    float *bufp = gNodeLocationBuffer; 
    vector<rclib::Point<float> > points = pci_gDataSet->GetMetaArm(metaArmNum)->GetNodeLocations(wrapEndpoints); 
    uint32_t p = 0; 
    while (p < points.size()) {
      if (points[p] == paraDIS::MetaArm::mWrappedNode) {
        *bufp++ = WRAPPED_NODE; 
        dbprintf(5, "paraDIS_GetMetaArmPoints: pushed back points[%d] = WRAPPED_NODE\n", p);
      } else {
        for (int i=0; i<3; i++) 
          *bufp++ = points[p][i]; 
        dbprintf(5, "paraDIS_GetMetaArmPoints: pushed back points[%d] = (%f, %f, %f)\n", 
                 p, points[p][0], points[p][1], points[p][2] );
      }
      ++p; 
    }
    *((int*) bufp) = END_OF_NODELIST; 
    dbprintf(5, "paraDIS_GetMetaArmPoints: pushed back END_OF_NODELIST\n");
    return gNodeLocationBuffer;
  }  

  bool paraDIS_EndBuffCheck(float *bufp) {
    return *((uint32_t*) bufp) == END_OF_NODELIST; 
  }

}
