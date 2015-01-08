/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
* Originating Author:  Richard Cook 925-423-9605 c.2011-01-03
* This file encapsulates the old "dumpfile-based" code, which is capable of 
* very detailed analysis, but cannot work in parallel.
*****************************************************************************/
#include "Dumpfile.h"
#include <DebugStream.h>
#include <DBOptionsAttributes.h>
#include "debugutil.h"
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>
#include <boost/format.hpp>
#include "paradis.h"
using namespace std; 

int burgersTypes[18] = {-2,-1, 0, 10,11,12,13, 20,21,22, 30,31,32, 40,41,42, 50, 60};

Dumpfile::Dumpfile(const char* filename, DBOptionsAttributes *rdatts) {

  this->Clear(); 

  mFilename = filename; 

  paraDIS::DataSet * ds = reinterpret_cast<paraDIS::DataSet *>(paraDIS_init()); (void)ds;
  /*!
    Write out files
  */ 
  mVerbosity = (rdatts == NULL) ? 0: rdatts->GetInt(PARADIS_VERBOSITY); 
  char *cp = getenv("PARADIS_VERBOSITY"); 
  if (cp) mVerbosity=atoi(cp); 
  else cp = (char*)"unset"; 
  dbprintf(1, "PARADIS_VERBOSITY variable is %s, verbosity of dumpfile reader set to %d\n", cp, mVerbosity); 

  if (DebugStream::GetLevel() > mVerbosity) {
    mVerbosity = DebugStream::GetLevel(); 
    dbprintf(1, "User set -debug to %d\n", mVerbosity); 
  } 
  
  dbprintf(2, "Dumpfile::Dumpfile(%s, rdatts)\n", mFilename.c_str()); 
  if (mVerbosity) {
    mDebugFile = (rdatts == NULL) ? string("") : rdatts->GetString(PARADIS_DEBUG_FILE);
  }
  cp = getenv("PARADIS_DEBUG_FILE");
  if (cp) mDebugFile = cp; 
  else {
    if (mVerbosity < 2) {
      cp = (char*)"unset"; 
    }
    else {
      cp = (char*)"paradis_debug.out"; 
    }
  } 
  paraDIS_SetVerbosity(mVerbosity, mDebugFile.c_str()); 
  dbprintf(1, "PARADIS_DEBUG_FILE variable is %s\n", cp); 
  
  int enable = (rdatts == NULL) ? 0 : rdatts->GetBool(PARADIS_ENABLE_DEBUG_OUTPUT); 
  cp = getenv("PARADIS_ENABLE_DEBUG_OUTPUT"); 
  if (cp) enable = atoi(cp); 
  else  {
    if (mVerbosity < 2) {
      cp = (char*)"unset"; 
    }
    else {
      cp = (char*)"1"; 
      enable = atoi(cp); 
    }
  }
  dbprintf(1, "PARADIS_ENABLE_DEBUG_OUTPUT variable is %s, so enable is %d\n", cp, enable); 
  
  paraDIS_EnableDebugOutput(enable); 
  paraDIS_EnableStatsOutput(enable); 
  
  dbg_setverbose(mVerbosity); 
  
  if (rdatts) 
    paraDIS_SetThreshold(rdatts->GetDouble(PARADIS_NN_ARM_THRESHOLD)); 
  cp = getenv("PARADIS_NN_ARM_THRESHOLD"); 
  if (cp)  {
    paraDIS_SetThreshold(atof(cp));
  }
  else cp = (char*)"unset"; 
  dbprintf(1, "PARADIS_NN_ARM_THRESHOLD variable is %s\n", cp); 
  
  // Adding a material to the mesh, allows subsetting and discrete colors:
  int numtypes = sizeof(burgersTypes)/sizeof(burgersTypes[0]); 
  mSegmentBurgerTypes = vector<int> (burgersTypes, burgersTypes + numtypes); 
  int t = 0; 
  while (t<numtypes) {
    mSegmentBurgerTypeNames.push_back(BurgersTypeNames(mSegmentBurgerTypes[t])); 
    ++t; 
  }
  
  mNodeNeighborValues.clear(); 
  mNodeNeighborValues.push_back(string("0 neighbors"));    
  mNodeNeighborValues.push_back(string("1 neighbor"));    
  mNodeNeighborValues.push_back(string("2 neighbors"));    
  mNodeNeighborValues.push_back(string("3 neighbors"));    
  mNodeNeighborValues.push_back(string("4 neighbors"));    
  mNodeNeighborValues.push_back(string("5 neighbors"));    
  mNodeNeighborValues.push_back(string("6 neighbors"));    
  mNodeNeighborValues.push_back(string("7 neighbors"));    
  mNodeNeighborValues.push_back(string("8 neighbors"));    
  mNodeNeighborValues.push_back(string("more than 8 neighbors"));    
  
  mMetaArmTypes.clear(); 
  mMetaArmTypes.push_back("METAARM_UNKNOWN"); 
  mMetaArmTypes.push_back("METAARM_111"); 
  mMetaArmTypes.push_back("METAARM_LOOP_111"); 
  mMetaArmTypes.push_back("METAARM_LOOP_HIGH_ENERGY"); 
  
  // PARADIS METADATA
  paraDIS_SetDataFile(mFilename.c_str());    
  paraDIS_SetProcNum(0, 1); // always serial; processor 0 of 1
  paraDIS_GetBounds(mExtents); 
  
  return; 
}


// ****************************************************************************
void Dumpfile::Clear(void) {
  mVerbosity = 0; 
  mNumMetaArmSegments = 0; 
  mFilename = ""; 

  paraDIS_close(); 
  return; 
}

// ****************************************************************************
Dumpfile::~Dumpfile() {
  debug2 << "Dumpfile::~Dumpfile()" << endl;
  Clear(); 
  return; 
}



// ****************************************************************************
//  Method: Dumpfile::FileIsValid
//
//  Purpose:
//      Check to see if the file is paraDIS SERIAL format
//  Return true iff it's serial format data
//
//  Programmer: rcook
//  Creation:   Tue Jan 30 14:56:34 PST 2007
//
// ****************************************************************************
bool Dumpfile::FileIsValid(void) { 
  double fextents[6] = {0}; 
  if ( ! paraDIS_GetBounds(fextents)) {
    debug1 <<"Dumpfile::FileIsValid(): Could not get bounds.  The file is not a ParaDIS-style file" <<endl; 
    return false; 
  }
  debug1 <<"Dumpfile::FileIsValid(): The file is a ParaDIS-style file" <<endl; 
  
  return true; 
}

// ****************************************************************************
//  Method: Dumpfile::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: rcook 
//  Creation:   Tue Jan 30 14:56:34 PST 2007
//
// ****************************************************************************

vtkDataSet *
Dumpfile::GetMesh(std::string meshname) {
  debug2 << "Dumpfile::GetMesh("<<meshname<<")"<<endl;
  try {
    //==================================================================
    /*! 
      Nodes, segments or metaarms
    */ 
    debug1 << "Starting Dumpfile::GetMesh(" <<meshname<<")"<< endl; 
    if (paraDIS_GetNumNodes() == 0) {
      paraDIS_SetDataFile(mFilename.c_str()); 
      paraDIS_ReadData();
      if (dbg_isverbose() > 1) {
        paraDIS_PrintArmStats(); 
      }
      debug2 << "Dumpfile::paradis created " << paraDIS_GetNumNodes() << " nodes, " << paraDIS_GetNumArmSegments() << " arm segments " << endl; 
    }
    /*!
      =============================================================
      Nodes, segments or metaarms  
      ===============================================================
    */ 
     if (meshname == "segments" || meshname == "nodes") {
       vtkPoints *points =  vtkPoints::New();
      /*! 
        The set of points in both segments and node meshes are the same.  This redundancy is the way our data is.
      */ 
      int numpoints = paraDIS_GetNumNodes(); 
      debug2 << "Setting number of points to  " <<numpoints << endl; 
      points->SetNumberOfPoints(numpoints); 
      
      debug2 << "Dumpfile::GetMesh: setting locations" << endl; 
      float location[3] ; 
      int nodenum = 0; 
      while (nodenum < numpoints) {
        paraDIS_GetNodeLocation(nodenum, location); 
        debug5 << "Setting point " << nodenum << " to (" << location[0] << ", " << location[1] << ", " << location[2] << ")" << endl; 
        points->SetPoint(nodenum, location); 
        ++nodenum; 
      }
      
      
      debug2 << "Dumpfile::GetMesh: *** creating "<< meshname << " mesh ***" << endl; 
      /*! 
        For segments, build the mesh from the ArmSegments
      */ 
      if (meshname == "segments") {
        
        int numsegments = paraDIS_GetNumArmSegments(); 
        
        vtkUnstructuredGrid *linemesh = vtkUnstructuredGrid::New(); // reference-counted "new"
        linemesh->SetPoints(points); 
        points->Delete();  // memory is not really freed here, I think due to ref counting. From now on, points are by index    
        
        /*!
          Associate node IDs to each segment.  
        */ 
        vtkIdType nodeIndices[2]; 
        int index0, index1, segnum = 0; 
        while (segnum < numsegments) {
          index0 = paraDIS_GetEndpointIndex(segnum, 0); 
          index1 = paraDIS_GetEndpointIndex(segnum, 1); 
          debug5 << "Dumpfile::GetMesh: Segment " << segnum << " node indices: (" << index0 << ", " << index1 << endl; 
          nodeIndices[0] = index0;
          nodeIndices[1] = index1;
          linemesh->InsertNextCell(VTK_LINE, 2, nodeIndices);
          ++segnum; 
        } 
        debug1 << "Dumpfile::GetMesh: Ending GetMesh(segments)" << endl; 
        return linemesh; 
      }/* End segments mesh */ 
      else {      
        vtkIdType nodeIndex[1]; //has to be array type
        vtkUnstructuredGrid *nodemesh = vtkUnstructuredGrid::New(); 
        nodemesh->SetPoints(points); 
        points->Delete(); 
        nodenum = 0; 
        while (nodenum != numpoints) {
          nodeIndex[0] = nodenum; 
          debug5 << "Inserting node index " << nodenum << " = " <<  nodeIndex[0] << endl;        
          nodemesh->InsertNextCell(VTK_VERTEX, 1, nodeIndex); 
          
          ++nodenum;
          
        }
        debug1 << "Dumpfile::GetMesh: Ending GetMesh(nodes)" << endl; 
        return nodemesh;         
      }/* end node mesh */ 
    }
    else if (meshname == "Meta Arms" || meshname == "Wrapped Meta Arms") {
      bool wrap = (meshname == "Wrapped Meta Arms"); 
      // For the meta arms, points and nodes are discovered per metaarm.  
      vtkPoints *points =  vtkPoints::New();
      vtkUnstructuredGrid *linemesh = vtkUnstructuredGrid::New(); // reference-counted "new"
      
      /*!
        Associate 0-based node IDs to each segment
      */ 
      uint32_t nodenum = 0;
      mNumMetaArmSegments = 0; // 0 based node IDs
      vtkIdType nodeIndices[2]; 
      uint32_t manum = 0, numMetaArms = paraDIS_GetNumMetaArms(); 
      while (manum < numMetaArms) {
        float *locations = paraDIS_GetMetaArmPoints(manum, wrap); 
        uint32_t segnum = 0, segsInArm = paraDIS_GetMetaArmNumSegments(manum, wrap); // sanity check
        while (!paraDIS_EndBuffCheck(locations)) {
          float *next = (locations+3); 
          if (*locations != WRAPPED_NODE && 
              *next != WRAPPED_NODE && 
              !paraDIS_EndBuffCheck(next) ) {
            nodeIndices[0] = nodenum;  
            nodeIndices[1] = nodenum+1; 
            dbprintf(5, "Dumpfile::GetMesh: MetaArm %d segment %d  node indices: (%d, %d)\n", manum, mNumMetaArmSegments,nodeIndices[0], nodeIndices[1]);             
            linemesh->InsertNextCell(VTK_LINE, 2, nodeIndices);
            mNumMetaArmSegments++; 
            segnum++; 
          }
          if (*locations == WRAPPED_NODE) {
            dbprintf(5, "Dumpfile::GetMesh: MetaArm %d: Skip location: WRAPPED_NODE\n", manum); 
            locations++; 
          } else { // not wrapped
            dbprintf(5, "Dumpfile::GetMesh: MetaArm %d points->InsertNextPoint %d at (%f, %f, %f);\n", manum, nodenum, locations[0], locations[1], locations[2]);  
            points->InsertNextPoint(locations); 
            locations += 3; 
            nodenum++; 
          }
          if (segnum > segsInArm) {
            dbprintf(0, "ERROR: metaarm %d: numsegs (%d) has exceeded number of segements from paraDIS_GetMetaArmPoints (%d)\n", manum, segnum, segsInArm); 
            
          }
        } 
        if (segnum != segsInArm) {
          throw str(boost::format("ERROR: metaarm %1%: number of segements from paraDIS_GetMetaArmPoints (%2%) does not match number of segments from paraDIS_GetMetaArmNumSegments() (%3%)\n")% manum% segnum% segsInArm);
          
        }
        dbprintf(5, "Done with metaarm %d\n\n", manum); 
        ++manum;
      }
      debug3 << mNumMetaArmSegments << " meta arm segments added to mesh" << endl;
      debug1 << "Dumpfile::GetMesh: Ending GetMesh(Meta Arms)" << endl; 
      linemesh->SetPoints(points); 
      points->Delete();  // memory is not really freed here, I think due to ref counting. From now on, points are by index    
      return linemesh; 
    }/* End MetaArm mesh */ 
  } catch (string err) {
    cerr << err << endl; 
    EXCEPTION1(VisItException, err.c_str());
  }   
  return NULL; ///TODO; check fix for non-void warning
}

// ****************************************************************************
//  Method: Dumpfile::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: rcook -- generated by xml2avt
//  Creation:   Tue Jan 30 14:56:34 PST 2007
//
// ****************************************************************************
vtkDataArray *Dumpfile::GetVar(std::string varname) {
  
  debug2 << "Beginning Dumpfile::GetVar(" << varname << ")" << endl; 
  int index = 0;
  float f=0.0; 
  vtkFloatArray *scalars = vtkFloatArray::New(); 

  int numsegs = paraDIS_GetNumArmSegments(), 
    numnodes = paraDIS_GetNumNodes();
  debug4 << "Dumpfile::GetVar: numsegs is " << numsegs << " and numnodes is " << numnodes << endl;
  if (varname == "Node-ID-Hash") {
    for (index=0; index<numnodes; index++) {
      f=paraDIS_GetNodeHash(index); 
      scalars->InsertTuple(index,&f); 
    }
  }  else if (varname == "Node-Engine") {
    for (index=0; index<numnodes; index++) {
      f= 0; 
      scalars->InsertTuple(index,&f); 
    }
  }   else if (varname == "Node-Index") {
    for (index=0; index<numnodes; index++) {
      f=index;
      scalars->InsertTuple(index,&f); 
    }
  }   else if (varname == "Node-Type") {
    for (index=0; index<numnodes; index++) {
      f=paraDIS_GetNodeType(index); 
      if (f < -7) f = -7; 
      if (f >  7) f = 7; 
      if (f>-4 && f<1) {
        dbprintf(0, "ERROR: Bad node type %d, changing to 0\n", (int)f); 
      }
      scalars->InsertTuple(index,&f); 
    }
  }   else if (varname == "Node-Is-Loop") {
    for (index=0; index<numnodes; index++) {
      f=paraDIS_NodeIsLoop(index); 
      scalars->InsertTuple(index,&f); 
    }
  }   else if (varname == "Node-Is-Type-M") {
    for (index=0; index<numnodes; index++) {
      f=paraDIS_NodeIsTypeM(index); 
      scalars->InsertTuple(index,&f); 
    }
  }   else if (varname == "Node-Is-Type-N") {
    for (index=0; index<numnodes; index++) {
      f=paraDIS_NodeIsTypeN(index); 
      scalars->InsertTuple(index,&f); 
    }
  }  else if (varname == "Segment-Index") {
    for (index=0; index<numsegs; index++) {
      f=paraDIS_GetSegmentSimulationIndex(index); // segment ID != index necessarily 
      scalars->InsertTuple(index,&f); 
    }
  } else if (varname == "Segment-Engine") {
    for (index=0; index<numsegs; index++) {
      f= 0; 
      scalars->InsertTuple(index,&f);
    }
  }
  else if (varname == "Segment-Duplicates") {
    for (index=0; index<numsegs; index++) {
      f = paraDIS_GetSegmentDuplicates(index); 
      scalars->InsertTuple(index,&f);
    }
  }  else if (varname == "Segment-Parent-Arm-ID") {
    for (index=0; index<numsegs; index++) {
      f= paraDIS_GetSegmentArmID(index); 
      scalars->InsertTuple(index,&f);
    }
  }  else if (varname == "Segment-Parent-MetaArm-ID") {
    for (index=0; index<numsegs; index++) {
      f= paraDIS_GetSegmentMetaArmID(index); 
      scalars->InsertTuple(index,&f);
    }
  }  else if (varname == "Segment-Parent-MetaArm-Type") {
    for (index=0; index<numsegs; index++) {
      f= paraDIS_GetSegmentMetaArmType(index); 
      scalars->InsertTuple(index,&f);
    }
  }  else if (varname == "MetaArm-ID" || varname == "Wrapped-MetaArm-ID") {
    bool wrap = (varname == "Wrapped-MetaArm-ID");
    uint32_t manum = 0, numMetaArms = paraDIS_GetNumMetaArms(); 
    uint32_t totalSegs = 0; 
    while (manum < numMetaArms) {
      f= paraDIS_GetMetaArmID(manum); 
      uint32_t segnum = 0, numsegs = paraDIS_GetMetaArmNumSegments(manum,wrap);
      while (segnum++ < numsegs) {          
        scalars->InsertTuple(totalSegs++,&f);
      }
      ++manum;       
    }  
  }
  debug1 << "done with Dumpfile::GetVar"<<endl;
  return scalars;
}

void *
Dumpfile::GetAuxiliaryData(const char *var, const char *type,
                             DestructorFunction &df) {
  debug3 << "Dumpfile::GetAuxiliaryData(" << var << ", " << type << ")" << endl; 
  if (strcmp(type, AUXILIARY_DATA_MATERIAL) != 0) {
    return NULL;
  }

  avtMaterial *mat = NULL;
   
  if (string(var).substr(0,8) == "Segment-") {
    int index; 
    int numsegs = paraDIS_GetNumArmSegments(); 
    int *matId = new int[numsegs];
    int *matptr = matId; 
    if (string(var) == "Segment-Burgers-Type") {
      //---------------------------------------------
      for (index=0; index<numsegs; index++) {
        *(matptr++)=BurgersTypeToIndex(paraDIS_GetSegmentBurgersType(index)); 
      }    
      mat = new avtMaterial(mSegmentBurgerTypeNames.size(), 
                            mSegmentBurgerTypeNames, 
                            numsegs, matId, 0, NULL, NULL, NULL, NULL);
      //---------------------------------------------      
    }     
    /*else if (string(var) == "Segment-MN-Type")  {
      //---------------------------------------------      
      for (index=0; index<numsegs; index++) {
        *(matptr++)=paraDIS_GetSegmentMNType(index); 
      }    
      mat = new avtMaterial(mSegmentMNTypes.size(), mSegmentMNTypes, 
                            numsegs, matId, 0, NULL, NULL, NULL, NULL);
      //---------------------------------------------
      }  */
    else {
      string err = string("Error: unknown variable: ") + var; 
      EXCEPTION1(VisItException, err.c_str());
    }
    
  }
  else if (string(var) == "Node Num Neighbors") {
    int index; 
    int numnodes = paraDIS_GetNumNodes(); 
    int *matId = new int[numnodes];
    int *matptr = matId; 
    for (index=0; index<numnodes; index++) {
      *(matptr++)=paraDIS_GetNumNodeNeighbors(index); 
    }    
    debug3 << "mNodeNeighborValues.size() = " << mNodeNeighborValues.size() << endl; 
    mat =  new avtMaterial(mNodeNeighborValues.size(), mNodeNeighborValues, 
                           numnodes, matId, 0, NULL, NULL, NULL, NULL);
  } else  if (string(var) == "MetaArm-Type" || 
              string(var) == "Wrapped-MetaArm-Type") {
    bool wrap = (string(var) == "Wrapped-MetaArm-Type"); 
    uint32_t manum = 0, numMetaArms = paraDIS_GetNumMetaArms(); 
    debug3 << "numMetaArms is " << numMetaArms << " and mNumMetaArmSegments is " << mNumMetaArmSegments << endl; 
    int *matId = new int[mNumMetaArmSegments];
    int *matptr = matId; 
    uint32_t totalSegs = 0;
    while (manum < numMetaArms) {
      uint8_t matype = paraDIS_GetMetaArmType(manum); 
      uint32_t segnum = 0, numsegs = paraDIS_GetMetaArmNumSegments(manum, wrap); 
      while (segnum < numsegs) {          
        *(matptr++)=matype;  
        ++ totalSegs;  
        ++segnum; 
      }
      ++manum; 
    }
    mat =  new avtMaterial(mMetaArmTypes.size(), mMetaArmTypes, 
                           mNumMetaArmSegments, matId, 0, NULL, NULL, NULL, NULL);
    debug4 << "total # values placed into material is " << totalSegs << endl; 
  }  else {
    return NULL; 
  }
  df = avtMaterial::Destruct;
  
  return mat;
}
      
uint8_t Dumpfile::BurgersTypeToIndex(int btype) {
  int numtypes = sizeof(burgersTypes)/sizeof(burgersTypes[0]); 
  int i=0; 
  for  (i=0; i<numtypes; i++) {
    if (btype == burgersTypes[i]) {
      return i;
    }
  }
  return 0; 
}



