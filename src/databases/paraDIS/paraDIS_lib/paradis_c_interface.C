/*!
  Implements the pure C interface to paraDIS lib, but is a C++ file.  
*/ 
#include "paradis.h"
#include "paradis_c_interface.h"

extern "C" {
static paraDIS::DataSet *gDataSet = NULL; 

#define PARADIS_INIT if (!gDataSet)  gDataSet = new paraDIS::DataSet     

  void paraDIS_init(void) {
    PARADIS_INIT;
  }

  void paraDIS_close(void) {
    if (gDataSet) delete gDataSet; 
    gDataSet = NULL;
  }

  void paraDIS_SetVerbosity(int level, const char *filename){
    PARADIS_INIT;    
    gDataSet->SetVerbosity(level, filename); 
    return; 
  }
  
  void paraDIS_EnableDebugOutput(int truth){
    PARADIS_INIT;
    gDataSet->EnableDebugOutput(truth); 
    return; 
  }
  
  void paraDIS_SetDataFile(const char *filename){
    PARADIS_INIT;
    gDataSet->SetDataFile(filename);    
    return; 
  } 
  
  void paraDIS_Clear(void){
    PARADIS_INIT;
    gDataSet->Clear(); 
    return; 
  }
   
   int paraDIS_GetBounds(double xxyyzz[6]){
    PARADIS_INIT;
    rclib::Point<float> datamin, datamax;   
    try {
      gDataSet->GetBounds(datamin, datamax); 
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
    PARADIS_INIT;
    rclib::Point<float> datamin, datamax;
    int i=3; while (i--) {
      datamin[i] = xxyyzz[2*i]; 
      datamax[i] = xxyyzz[2*i+1]; 
    }
    gDataSet->SetSubspace(datamin, datamax); 
    return; 
  }
   
  void paraDIS_TestRestrictSubspace(void) {
    gDataSet->TestRestrictSubspace(); 
    return; 
  } 
     
  
  void paraDIS_SetProcNum(int procnum, int numprocs){
    PARADIS_INIT;
    gDataSet->SetProcNum(procnum, numprocs); 
    return; 
  }
  
  
  void paraDIS_ReadData(void){
    PARADIS_INIT;
    gDataSet->ReadData(); 
    return; 
  }
  
  void paraDIS_PrintArmStats(void) {
    PARADIS_INIT; 
    gDataSet->PrintArmStats(); 
  }

  uint32_t paraDIS_GetNumNodes(void){
    PARADIS_INIT;
    return gDataSet->GetNumNodes();  
  }
  
  void paraDIS_printNodeVerbose(uint32_t nodenum){
    PARADIS_INIT;
    cout << gDataSet->GetNode(nodenum)->Stringify() << endl; 
  }

  void paraDIS_GetNodeLocation(uint32_t nodenum, float loc[3]){    
    PARADIS_INIT;
    gDataSet->GetNode(nodenum)->GetLocation(loc); 
    return; 
  }
  
  int8_t paraDIS_GetNodeType(uint32_t nodenum){
    PARADIS_INIT;
    return gDataSet->GetNode(nodenum)->GetNodeType(); 
  }  
    
  int8_t paraDIS_GetNumNodeNeighbors(uint32_t nodenum){
    PARADIS_INIT;
    return gDataSet->GetNode(nodenum)->GetNumNeighbors(); 
  }  

  int32_t  paraDIS_GetNodeSimulationDomain(uint32_t nodenum){
    PARADIS_INIT;
    return gDataSet->GetNode(nodenum)->GetNodeSimulationDomain(); 
  }
  
  int32_t  paraDIS_GetNodeSimulationID(uint32_t nodenum){
    PARADIS_INIT;
    return gDataSet->GetNode(nodenum)->GetNodeSimulationID(); 
  }  
  
  uint32_t paraDIS_GetNumArmSegments(void){
     PARADIS_INIT;
   return gDataSet->GetNumArmSegments(); 
  }
  
  void paraDIS_printArmSegmentVerbose(uint32_t segnum){
    PARADIS_INIT;
    cout << gDataSet->GetArmSegment(segnum)->Stringify() << endl; 
  }

  int8_t paraDIS_GetSegmentBurgersType(uint32_t segmentnum) {
    PARADIS_INIT;
    return gDataSet->GetArmSegment(segmentnum)->GetBurgersType(); 
  }  
  
  int8_t paraDIS_GetSegmentMNType(uint32_t segmentnum) {
    PARADIS_INIT;
    return gDataSet->GetArmSegment(segmentnum)->GetMNType(); 
  }  
  
  int32_t paraDIS_GetEndpointIndex(uint32_t segmentnum, int endpointnum){
    PARADIS_INIT;
    return gDataSet->GetArmSegment(segmentnum)->GetNodeIndex(endpointnum);  
  }
  
  

}
