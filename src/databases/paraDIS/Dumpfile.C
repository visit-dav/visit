/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
#include "paradis_c_interface.h"
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
using namespace std; 

Dumpfile::Dumpfile(string filename, DBOptionsAttributes *rdatts): 
  mVerbosity(0),  mMaterialSetChoice(0), mFilename(filename) {

  paraDIS_init(); 
  // DebugStream::GetLevel doesn't exist in 1.12RC, so turning off
  // debugging as a work around.  It does exist in the trunk, so 
  // leave as follows in trunk.  Sigh.  
  // mVerbosity = DebugStream::GetLevel(); 
  /*!
    Write out files
  */ 
  mVerbosity = rdatts->GetInt(PARADIS_VERBOSITY); 
  debug3 << "paradis verbosity of dumpfile reader set to " << mVerbosity << endl; 
  if (mVerbosity) {
    mDebugFile = rdatts->GetString(PARADIS_DEBUG_FILE);
  }
  paraDIS_SetVerbosity(mVerbosity, mDebugFile.c_str()); 
  paraDIS_EnableDebugOutput(rdatts->GetBool(PARADIS_ENABLE_DEBUG_OUTPUT)); 

  dbg_setverbose(mVerbosity); 

  mMaterialSetChoice = rdatts->GetEnum(PARADIS_MATERIAL_SET_CHOICE); 
  paraDIS_SetThreshold(rdatts->GetDouble(PARADIS_NN_ARM_THRESHOLD)); 
 // Adding a material to the mesh, which is the correct thing to do for int-valued data (discrete) as it allows subsetting to work
  // our own data members. 
  mSegmentMNTypes.clear(); 
  mSegmentMNTypes.push_back(string("ARM_UNKNOWN"));    
  mSegmentMNTypes.push_back(string("ARM_UNINTERESTING"));
  mSegmentMNTypes.push_back(string("ARM_LOOP"));
  mSegmentMNTypes.push_back(string("ARM_MM_111"));
  mSegmentMNTypes.push_back(string("ARM_MN_111"));
  mSegmentMNTypes.push_back(string("ARM_NN_111"));
  mSegmentMNTypes.push_back(string("ARM_MM_100"));
  mSegmentMNTypes.push_back(string("ARM_MN_100"));
  mSegmentMNTypes.push_back(string("ARM_NN_100"));
  mSegmentMNTypes.push_back(string("ARM_SHORT_NN_111"));
  mSegmentMNTypes.push_back(string("ARM_SHORT_NN_100"));

  mBurgersTypes.push_back(string("100 arm type"));    
  mBurgersTypes.push_back(string("010 arm type"));
  mBurgersTypes.push_back(string("001 arm type"));
  mBurgersTypes.push_back(string("+++ arm type"));
  mBurgersTypes.push_back(string("++- arm type"));
  mBurgersTypes.push_back(string("+-+ arm type"));
  mBurgersTypes.push_back(string("-++ arm type"));
  mBurgersTypes.push_back(string("unknown arm type"));

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
  
  // PARADIS METADATA
  paraDIS_SetDataFile(mFilename.c_str());    
  paraDIS_SetProcNum(0, 1); // always serial; processor 0 of 1

  return; 
}

Dumpfile::~Dumpfile() {
  paraDIS_Clear(); 
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
    debug1 <<"Could not get bounds.  The file is not a ParaDIS-style file" <<endl; 
    return false; 
  }
  debug1 <<"The file is a ParaDIS-style file" <<endl; 
 
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
      Nodes or segments
    */ 
    debug1 << "Starting Dumpfile::GetMesh(" <<meshname<<")"<< endl; 
    //return TestGetMesh(); 
    if (paraDIS_GetNumNodes() == 0) {
      paraDIS_SetDataFile(mFilename.c_str()); 
#ifdef TEST_SUBSPACE
      /*! 
        Set a subspace to exercise culling code
      */       
      paraDIS_TestRestrictSubspace(); 
#endif
      paraDIS_ReadData();
      if (dbg_isverbose() > 1) {
        paraDIS_PrintArmStats(); 
      }
      debug2 << "Dumpfile::GetMesh: created " << paraDIS_GetNumNodes() << " nodes  and " << paraDIS_GetNumArmSegments() << " arm segments." << endl; 
    }
    
    //=============================================================
    /* nodes or segments */ 
    //===============================================================
    
    /*! 
      The set of points in both segments and node meshes are the same.  This redundancy is the way our data is.
    */ 
    vtkPoints *points =  vtkPoints::New();
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
    
    
    /*! 
      For segments, build the mesh from the ArmSegments
    */ 
    if (meshname == "segments") {
      debug2 << "Dumpfile::GetMesh: *** creating segment mesh ***" << endl; 
      
      
      //vtkCellArray *lines = vtkCellArray::New(); 
      int numsegments = paraDIS_GetNumArmSegments(); 
      //lines->Allocate(3*numsegments); 
      
      vtkUnstructuredGrid *linemesh = vtkUnstructuredGrid::New(); // reference-counted "new"
      linemesh->SetPoints(points); 
      points->Delete();  // memory is not really freed here, I think due to ref counting. From now on, points are by index    
      
      /*!
        Associate node IDs to each segment
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
      /*!
        Assign lines to mesh
      */ 
      //linemesh->SetLines(lines);
      //lines->Delete(); // actually a deref, not deleted...
      debug1 << "Dumpfile::GetMesh: Ending GetMesh" << endl; 
      return linemesh; 
    }/* End segments mesh */ 
    else {      
      debug2 << "Dumpfile::GetMesh: *** creating node mesh ***" << endl; 
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
      debug1 << "Dumpfile::GetMesh: Ending GetMesh" << endl; 
      return nodemesh;         
    }/* end node mesh */ 
  } catch (string err) {
    cerr << err << endl; 
    EXCEPTION1(VisItException, err.c_str());
  }   
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
  if (varname == "segmentIndex") {
    for (index=0; index<numsegs; index++) {
      f=index; 
      scalars->InsertTuple(index,&f); 
    }
  } else if (varname == "segmentEngine") {
    for (index=0; index<numsegs; index++) {
      f= 0; 
      scalars->InsertTuple(index,&f);
    }
  } else if (varname == "burgersType") {
    for (index=0; index<numsegs; index++) {
      f= paraDIS_GetSegmentBurgersType(index); 
      scalars->InsertTuple(index,&f);
    }
  }   else if (varname == "simulationDomain") {
    for (index=0; index<numnodes; index++) {
      f=paraDIS_GetNodeSimulationDomain(index); 
      scalars->InsertTuple(index,&f); 
    }
  }   else if (varname == "simulationID") {
    for (index=0; index<numnodes; index++) {
      f=paraDIS_GetNodeSimulationID(index); 
      scalars->InsertTuple(index,&f); 
    }
  }   else if (varname == "nodeEngine") {
    for (index=0; index<numnodes; index++) {
      f= 0; 
      scalars->InsertTuple(index,&f); 
    }
  }   else if (varname == "nodeIndex") {
    for (index=0; index<numnodes; index++) {
      f=index;
      scalars->InsertTuple(index,&f); //value(ith element) == i
    }
  } 
  debug1 << "done with Dumpfile::GetVar"<<endl;
  return scalars;
    //
    // If you have a file format where variables don't apply (for example a
    // strictly polygonal format like the STL (Stereo Lithography) format,
    // then uncomment the code below.
    //
    // EXCEPTION1(InvalidVariableException, varname);
    //

    //
    // If you do have a scalar variable, here is some code that may be helpful.
    //
    // int ntuples = XXX; // this is the number of entries in the variable.
    // vtkFloatArray *rv = vtkFloatArray::New();
    // rv->SetNumberOfTuples(ntuples);
    // for (int i = 0 ; i < ntuples ; i++)
    // {
    //      rv->SetTuple1(i, VAL);  // you must determine value for ith entry.
    // }
    //
    // return rv;
    //
}

void *
Dumpfile::GetAuxiliaryData(const char *var, const char *type,
                             DestructorFunction &df) {
  debug3 << "Dumpfile::GetAuxiliaryData(" << var << ", " << type << ")" << endl; 
  if (strcmp(type, AUXILIARY_DATA_MATERIAL) != 0) {
    return NULL;
  }

  avtMaterial *mat = NULL;
  
  if (string(var).substr(0,8) == "Segment_") {
    int index; 
    int numsegs = paraDIS_GetNumArmSegments(); 
    int *matId = new int[numsegs];
    int *matptr = matId; 
    if (string(var) == "Segment_Burgers_Type") {
      //---------------------------------------------
      for (index=0; index<numsegs; index++) {
        *(matptr++)=paraDIS_GetSegmentBurgersType(index)-1; 
      }    
      debug3 << "mBurgersTypes.size(): " << mBurgersTypes.size() << endl;
      int i=0; 
      while (i < mBurgersTypes.size()) {
        debug3 << "mBurgersTypes["<<i<<"] = "<<mBurgersTypes[i]<< endl; 
        ++i; 
      }
      mat = new avtMaterial(mBurgersTypes.size(), mBurgersTypes, 
                            numsegs, matId, 0, NULL, NULL, NULL, NULL);
      //---------------------------------------------      
    }     else if (string(var) == "Segment_MN_Type")  {
      //---------------------------------------------      
      for (index=0; index<numsegs; index++) {
        *(matptr++)=paraDIS_GetSegmentMNType(index); 
      }    
      mat = new avtMaterial(mSegmentMNTypes.size(), mSegmentMNTypes, 
                            numsegs, matId, 0, NULL, NULL, NULL, NULL);
      //---------------------------------------------
    } else {
      string err = string("Error: unknown variable: ") + var; 
      EXCEPTION1(VisItException, err.c_str());
    }
    
  }
  else if (string(var) == "Node_Num_Neighbors") {
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
  } else {
    /* 
       string err = string("Error: unknown variable: ") + var; 
       EXCEPTION1(VisItException, err.c_str());
    */
    return NULL; 
  }
  df = avtMaterial::Destruct;
  
  return mat;
}
      


