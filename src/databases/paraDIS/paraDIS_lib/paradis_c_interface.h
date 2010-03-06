/*!
  \file paradis_c_interface.h
  Purpose:  provide a pure C interface for C++ codes to link to to potentially avoid linker and header conflicts by isolating all C++ data structures into a set of similarly compiled modules. 
*/ 

#ifndef PARADIS_C_INTERFACE_H
#define PARADIS_C_INTERFACE_H
#ifndef WIN32
#  include <stdint.h>
#else
#  include <msc_stdint.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif  
  /*! 
    purely for testing -- reduce the datamax by 0.5
  */ 
  void paraDIS_TestRestrictSubspace(void);
  
  void paraDIS_init(void); 
  
  void paraDIS_close(void); 

  void paraDIS_SetVerbosity(int level, const char *filename=0); 
  
  void paraDIS_EnableDebugOutput(int truth); 
  
  void paraDIS_SetDataFile(const char *filename); 
  
  void paraDIS_Clear(void); 
  
  /*!
    read the head of the data and return the bounding box. return 0 on failure, 1 on success
  */ 
  int paraDIS_GetBounds(double xxyyzz[6]);
 
  void paraDIS_SetSubspace(double xxyyzz[6]); 
  
  /*!
    procnum is zero-based
  */ 
  void paraDIS_SetProcNum(int procnum, int numprocs);
  
  void paraDIS_ReadData(void);
  
  void paraDIS_PrintArmStats(void); 

  uint32_t paraDIS_GetNumNodes(void);
  
  void paraDIS_printNodeVerbose(uint32_t nodenum);

  void paraDIS_GetNodeLocation(uint32_t nodenum, float loc[3]);
  
  int8_t paraDIS_GetNodeType(uint32_t nodenum);

  int8_t paraDIS_GetNumNodeNeighbors(uint32_t nodenum);

  /*!
    The domain refers to the simulation domain
  */ 
  int32_t  paraDIS_GetNodeSimulationDomain(uint32_t nodenum);
  
  /*! 
    Return the ID within the simulation ID for the node (not the index into the readers node array)
  */ 
  int32_t  paraDIS_GetNodeSimulationID(uint32_t nodenum);
  
  uint32_t paraDIS_GetNumArmSegments(void);
  
  void paraDIS_printArmSegmentVerbose(uint32_t segnum);

  int32_t paraDIS_GetEndpointIndex(uint32_t segmentnum, int endpointnum);

  int8_t paraDIS_GetSegmentBurgersType(uint32_t segmentnum);
  
  int8_t paraDIS_GetSegmentMNType(uint32_t segmentnum);
  

#ifdef __cplusplus
} /* end extern "C" */ 
#endif


#endif


