// ************************************************************************* //
//                            avtFVCOMReader.C                               //
// ************************************************************************* //

#include <avtFVCOMReader.h>
#include <map>
    
#include <string>

#include <vtkFloatArray.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkObject.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>

#include <avtDatabaseMetaData.h>
#include <avtVariableCache.h>

// Removed materials for now, 1.5.4
//#include <avtMaterial.h>

#include <DebugStream.h>
#include <InvalidVariableException.h>
#include <ImproperUseException.h>

#include <NETCDFFileObject.h>
#include <netcdf.h>
#include <Expression.h>

using     std::string;

// ****************************************************************************
// Method: avtFVCOMReader::Identify
//
// Purpose:
//   This method checks to see if the file is an FVCOM file.
//
// Arguments:
//   fileObject : The file to check.
//
// Returns:    True if the file is a particle file; False otherwise.
//
// Note:
//
// Programmer: David Stuebe
// Creation:   Thu May 4 16:18:57 PST 2006
//
// Modifications:
//
// ****************************************************************************

bool
avtFVCOMReader::Identify(NETCDFFileObject *fileObject)
{
  bool isFVCOM = false;

  // Simple statement to identify FVCOM files:
  // Do not change source statement in mod_ncdio !!!

  std::string source;
  if(fileObject->ReadStringAttribute("source", source))
    {
      isFVCOM = strncmp("FVCOM",source.c_str(),5)==0;
    }

  return isFVCOM;
}

// ****************************************************************************
//  Method: avtFVCOM constructor
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************


avtFVCOMReader::avtFVCOMReader(const char *filename)
{
  fileObject = new NETCDFFileObject(filename);
  alloc_fileObject=true;

  NeedDimensions=true;
  NeedGridVariables=true;

  alloc_dimSizes=false;
  alloc_xvals=false;
  alloc_yvals=false;
  alloc_zvals=false;
  alloc_SigLayers=false;
  alloc_SigLevels=false;
  alloc_nvvals=false;
}

avtFVCOMReader::avtFVCOMReader(const char *filename, NETCDFFileObject *f)
{
  fileObject = f;
  alloc_fileObject=false;

  NeedDimensions=true;
  NeedGridVariables=true;

  alloc_dimSizes=false;
  alloc_xvals=false;
  alloc_yvals=false;
  alloc_zvals=false;
  alloc_SigLayers=false;
  alloc_SigLevels=false;
  alloc_nvvals=false;
}

// ****************************************************************************
// Method: avtFVCOMReader::~avtFVCOMReader
//
// Purpose: 
//   Destructor for the avtFVCOMReader class.
//
// Programmer: David Stuebe
// Creation:   Thu May 4 16:18:57 PST 2006
//
// Modifications:
//   
// ****************************************************************************

avtFVCOMReader::~avtFVCOMReader()
{
  debug4 << "avtFVCOMReader::~avtFVCOMReader: destroying Reader." << endl;

  //  debug4 << "fileObject: " << fileObject << endl;

  if (alloc_fileObject)  
    {
      debug4 << "fileObject: " << fileObject << endl;
      delete fileObject;
      alloc_fileObject=false;
    }
  debug4 << "avtFVCOMReader::~avtFVCOMReader: end" << endl;
}

// ****************************************************************************
// Method: avtFVCOMReader::FreeUpResources
//
// Purpose: 
//   Frees up the resources that the file is using.
//
// Programmer: David Stuebe
// Creation:   Tue Jun 20 14:22:37 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
avtFVCOMReader::FreeUpResources()
{
  debug4 << "avtFVCOMReader::FreeUpResources: closing file." << endl;


  if (alloc_xvals)
    {
      delete [] xvals;
      debug4 << "delete xvals" << xvals << endl;
      alloc_xvals=false;
    }

  if (alloc_yvals)
    {
      delete [] yvals;
      debug4 << "delete yvals" << yvals << endl;
      alloc_yvals=false;
    }

  if (alloc_zvals)
    {
      delete [] zvals;
      debug4 << "delete zvals" << zvals << endl;
      alloc_zvals=false;
    }

  if (alloc_SigLayers)
    {
      delete [] SigLayers;
      debug4 << "delete SigLayers" << SigLayers << endl;
      alloc_SigLayers=false;
    }

  if (alloc_SigLevels)
    {
      delete [] SigLevels;
      debug4 << "delete Siglevels" << SigLevels << endl;
      alloc_SigLevels=false;
    }

  if(alloc_nvvals)
    {
      delete [] nvvals;
      debug4 << "delete nvvals" << nvvals << endl;
      alloc_nvvals=false;
    }


  if (alloc_dimSizes)
    {
      delete [] dimSizes;
      debug4 << "delete dimSizes" << dimSizes << endl;
      alloc_dimSizes=false;
    }

  debug4 << "done delete" << endl;
  NeedDimensions=true;
  NeedGridVariables=true;
  
  fileObject->Close();

  debug4 << "avtFVCOMReader::FreeUpResources: end" << endl;
  

}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************

int
avtFVCOMReader::GetNTimesteps(void)
{
  const char *mName = "avtFVCOMReader::GetNTimesteps: ";
  debug4 << mName << endl;


  if (NeedDimensions) GetDimensions();


  //    size_t ntimesteps;
  //  int status, time_id, ncid;

  //    ncid=fileObject->GetFileHandle();
      
  //    status = nc_inq_dimid(ncid, "time", &time_id);
  //    if (status != NC_NOERR) fileObject-> HandleError(status);
  //    status = nc_inq_dimlen(ncid, time_id, &ntimesteps);
  //    if (status != NC_NOERR) fileObject-> HandleError(status);


  debug4 << mName << "ntimesteps=" << nTime << endl;
  debug4 << mName << "end" << endl;


  return nTime;
}

// ****************************************************************************
//  Method: avtFVCOMReader::GetFVCOMDimensions
//
//  Purpose: Tell the rest of the code the dimension of the data in this file
//          
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************

void
avtFVCOMReader::GetDimensions()
{

  const char *mName = "avtFVCOMReader::GetFVCOMDimensions: ";
  debug4 << mName << endl;

  ncid = fileObject->GetFileHandle();
  
  if(ncid == -1)
    {
      std::string msg("Could not get file handle in avtFVCOMReader::GetDimensions");
      EXCEPTION1(ImproperUseException, msg);
    }
  
  //  size_t  tScalar, tNode, tElem, tSiglay, tSiglev, tThree, tFour, tMaxnode, 
  //  tMaxelem, tTime, tdimSizes;

  status = nc_inq(ncid, &nDims, &nVars, &nGlobalAtts, &unlimitedDimension);
  if(status != NC_NOERR) fileObject-> HandleError(status);
    

  dimSizes = new int[nDims];
  alloc_dimSizes=true;

  size_t  tScalar, tNode, tElem, tSiglay, tSiglev,
    tThree, tFour, tMaxnode, tMaxelem, tTime;


  debug4 << "All dims=[" ; 
  for(int i = 0; i < nDims; ++i)
    {
      size_t tSizes;
      status = nc_inq_dim(ncid, i, DimName, &tSizes);
      if (status != NC_NOERR) fileObject-> HandleError(status);
      dimSizes[i]=tSizes;
      debug4<< dimSizes[i] << " ";
    }
  debug4<< "]" << endl;
    

  debug4 << "Try to get all known dimensions used in FVCOM" << endl;
  debug4 << "Some may return unknown: that is okay!" << endl;

  // Get dimsizes for known dim names!
  // SCALAR

  status = nc_inq_dimid(ncid, "scalar", &nScalarID);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  status = nc_inq_dimlen(ncid, nScalarID, &tScalar);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  nScalar=tScalar;
 

  // NODE
  status = nc_inq_dimid(ncid, "node", &nNodeID);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  status = nc_inq_dimlen(ncid, nNodeID, &tNode);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  nNode=tNode;
  
  // NELE
  status = nc_inq_dimid(ncid, "nele", &nElemID);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  status = nc_inq_dimlen(ncid, nElemID, &tElem);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  nElem=tElem;

  //SIGLAY
  status = nc_inq_dimid(ncid, "siglay", &nSiglayID);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  status = nc_inq_dimlen(ncid, nSiglayID, &tSiglay);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  nSiglay=tSiglay;

  //SIGLEV
  status = nc_inq_dimid(ncid, "siglev", &nSiglevID);
  if (status != NC_NOERR) fileObject->HandleError(status);
  status = nc_inq_dimlen(ncid, nSiglevID, &tSiglev);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  nSiglev=tSiglev;

  //THREE
  status = nc_inq_dimid(ncid, "three", &nThreeID);
  if (status != NC_NOERR) fileObject->HandleError(status);
  status = nc_inq_dimlen(ncid, nThreeID, &tThree);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  nThree=tThree;

  //FOUR
  status = nc_inq_dimid(ncid, "four", &nFourID);
  if (status != NC_NOERR) fileObject->HandleError(status);
  status = nc_inq_dimlen(ncid, nFourID, &tFour);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  nFour=tFour;

  //MAXNODE
  status = nc_inq_dimid(ncid, "maxnode", &nMaxnodeID);
  if (status != NC_NOERR) fileObject->HandleError(status);
  status = nc_inq_dimlen(ncid, nMaxnodeID, &tMaxnode);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  nMaxnode=tMaxnode;

  //MAXELEM
  status = nc_inq_dimid(ncid, "maxelem", &nMaxelemID);
  if (status != NC_NOERR) fileObject->HandleError(status);
  status = nc_inq_dimlen(ncid, nMaxelemID, &tMaxelem);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  nMaxelem=tMaxelem;


  //TIME
  status = nc_inq_dimid(ncid, "time", &nTimeID);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  status = nc_inq_dimlen(ncid, nTimeID, &tTime);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  nTime=tTime;

  


  NeedGridVariables=true;

  NeedDimensions=false;


  debug4 << mName <<"end" << endl;

  return;

}

// ****************************************************************************
// Method: avtFVCOMReader::GetTimes
//
// Purpose: 
//   Returns the times in the file.
//
// Arguments:
//   t : The times to be returned.
//
// Programmer: David Stuebe
// Creation:   Thu May 18 08:39:01 PDT 2006
//
// Modifications: David Stuebe
//                Made code smart about reading in time, convert seconds to days
//                Wed NOV 22, 2006
//   
//   
// ****************************************************************************

void
avtFVCOMReader::GetTimes(doubleVector &t)
{
  const char *mName = "avtFVCOMReader::GetTimes: ";
  debug4 << mName << endl;

  if (NeedDimensions) GetDimensions();
  
  
  std::string timeunits;
  if(fileObject->ReadStringAttribute("time", "units", timeunits))
    {
      debug4<< "timeunits: " << timeunits << endl;
    }
  else
    debug4<< "timeunits+++ could not get" << endl;
    
  double convert=1;
  if (strncmp(timeunits.c_str(), "seconds",7)==0)
    convert=convert/double(60*60*24);



  if (nTime == 1)
    {
      float tf=-1;
      fileObject->ReadVariableInto("time", FLOATARRAY_TYPE, &tf);
      t.push_back(double(tf)*convert);
      
    }
  else if (nTime >1)
    {
      float t1=-1, t2=-1, tend=-1;
      
      const int s1[1]={0};
      const int c1[1]={1};

      fileObject->ReadVariableInto("time",FLOATARRAY_TYPE,s1,c1,&t1);

      const int s2[1]={1};
      const int c2[1]={1};

      fileObject->ReadVariableInto("time",FLOATARRAY_TYPE,s2,c2,&t2);

      const int send[1]={nTime-1};
      const int cend[1]={1};

      fileObject->ReadVariableInto("time",FLOATARRAY_TYPE,send,cend,&tend);


      float dti=t2-t1;

      if (tend == t1+(nTime-1)*dti)
        {
          for (int n=0; n < nTime; ++n)
            t.push_back(double(t1+dti*n)*convert);
        }
      else 
        {
          debug4<< "Something is fishy with the time: read it directly" << endl;

          float *tf = new float[nTime];
          fileObject->ReadVariableInto("time", FLOATARRAY_TYPE, tf);
          for(int n=0; n<nTime; ++n)
            t.push_back(double(tf[n])*convert);
          
          delete [] tf;
        }

      
    }
  
  debug4 << mName << "Got Times" << endl;
  debug4 << mName << "end" << endl;
}


// ****************************************************************************
// Method: avtFVCOMReader::GetCycles
//
// Purpose: 
//   Returns the time cycle in the file.
//
// Arguments:
//   cyc : The times cycle to be returned.
//
// Programmer: David Stuebe
// Creation:   Thu May 18 08:39:01 PDT 2006
//
// Modifications: David Stuebe
//                Made code smart about reading in cycles, faster!
//                Wed NOV 22, 2006
//   
// ****************************************************************************


void
avtFVCOMReader::GetCycles(intVector &cyc)
{
  const char *mName = "avtFVCOMReader::GetCycles: ";
  debug4 << mName << endl;


  if (NeedDimensions) GetDimensions();

  TypeEnum type = NO_TYPE;

  int *dimlen;
  fileObject->InqVariable("iint", &type, &VarnDims, &dimlen);
  delete [] dimlen;

  // Now get variable based on type
  if(type == INTEGERARRAY_TYPE)
    {
      
      if (nTime == 1)
        {
          int ci=-1;
          fileObject->ReadVariableInto("iint", INTEGERARRAY_TYPE, &ci);
          cyc.push_back(ci);
          
        }
      else if (nTime >1)
        {
          int cyc1=-1, cyc2=-1, cycend=-1;
          
          const int s1[1]={0};
          const int c1[1]={1};
          
          fileObject->ReadVariableInto("iint",INTEGERARRAY_TYPE,s1,c1,&cyc1);
          
          const int s2[1]={1};
          const int c2[1]={1};
          
          fileObject->ReadVariableInto("iint",INTEGERARRAY_TYPE,s2,c2,&cyc2);
          
          const int send[1]={nTime-1};
          const int cend[1]={1};
          
          fileObject->ReadVariableInto("iint",INTEGERARRAY_TYPE,send,cend,&cycend);
          
          
          int dci=cyc2-cyc1;
          
          if (cycend == cyc1+(nTime-1)*dci)
            {
              for (int n=0; n < nTime; ++n)
                cyc.push_back(cyc1+n*dci);
            }
          else 
            {
              debug4<< "Something is fishy with iint: read it directly" << endl;
              debug4 << mName << "IINT returned to cyc as NC_INT" << endl;
              int *ci = new int[nTime];
              fileObject->ReadVariableInto("iint", INTEGERARRAY_TYPE, ci);
              for(int n=0; n<nTime; ++n)
                {
                  cyc.push_back(ci[n]);
                }
              delete [] ci;
              
            }
        }
    }
  else if(type == FLOATARRAY_TYPE)
    { 
      debug4 << mName << "IINT returned to cyc as NC_FLOAT: Convert to INT" << endl;
      
      if (nTime == 1)
        {
          float cf=-1;
          fileObject->ReadVariableInto("iint", FLOATARRAY_TYPE, &cf);
          cyc.push_back(int(cf));
              
        }
      else if (nTime >1)
        {
          float cyc1=-1, cyc2=-1, cycend=-1;
              
          const int s1[1]={0};
          const int c1[1]={1};
              
          fileObject->ReadVariableInto("iint",INTEGERARRAY_TYPE,s1,c1,&cyc1);
              
          const int s2[1]={1};
          const int c2[1]={1};
              
          fileObject->ReadVariableInto("iint",INTEGERARRAY_TYPE,s2,c2,&cyc2);
              
          const int send[1]={nTime-1};
          const int cend[1]={1};
              
          fileObject->ReadVariableInto("iint",INTEGERARRAY_TYPE,send,cend,&cycend);
              
              
          float dci=cyc2-cyc1;
              
          if (cycend == cyc1+(nTime-1)*dci)
            {
              for (int n=0; n < nTime; ++n)
                cyc.push_back(int(cyc1+n*dci));
            }
          else 
            {

              float *cf = new float[nTime];
              fileObject->ReadVariableInto("iint", FLOATARRAY_TYPE, cf);
                  
              // If float returned, change to double for VisIt   
              for(int n=0; n<nTime; ++n)
                {
                  cyc.push_back(int(cf[n]));
                }
              delete [] cf;
            }
        }        
    }
  else    
    {
      debug4 << mName << "Could not return cycles: Wrong variable type" << endl;
    }
      
  debug4 << mName << "Got cycles" << endl;
  debug4 << mName << "end" << endl;
}
  
// ****************************************************************************
//  Method: avtFVCOMReader::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************


void
avtFVCOMReader::PopulateDatabaseMetaData(avtDatabaseMetaData *md, 
                                         const int ts, const char *dbtype)
{
  const char *mName = "avtFVCOM::PopulateDatabaseMetaData: ";
  debug4 << mName << endl;

  if (NeedDimensions) GetDimensions();

  if(debug4_real)
    fileObject->PrintFileContents(debug4_real);

  // Assemble a database title.
  std::string comment(dbtype), titleString, source,
    history, references, conventions;
  if(fileObject->ReadStringAttribute("title", titleString))
    {
      comment += (std::string(" database: title=") + titleString);

      if(fileObject->ReadStringAttribute("source", source))
        comment += (std::string(", source=") + source);
 
      if(fileObject->ReadStringAttribute("history", history))
        comment += (std::string(", history=") + history);
      if(fileObject->ReadStringAttribute("references", references))
        comment += (std::string(", references=") + references);
      if(fileObject->ReadStringAttribute("Conventions", conventions))
        comment += (std::string(", Conventions=") + conventions);

      md->SetDatabaseComment(comment);
    }

  debug4 << mName <<"Finished SetDatabaseComments" << endl;


  // Declare state variable used to check if needed info is included 
  // in the netcdf file

  int state=1;

  // Add mesh exists statement to check if mesh loaded!
  //  std::map<std::string, bool> meshExists;

  //--------------------------------------------------------------------
  //---------------------------------------------------------------
  // Add the Bathymetry_Mesh.
  //
  std::string Bathymetry_Mesh("Bathymetry_Mesh");
  state=1;
  //  Get the units for the mesh.
  std::string xUnits, xLabel, yUnits, yLabel, zUnits,zLabel; 
  state*=fileObject->ReadStringAttribute("x", "units", xUnits);
  state*=fileObject->ReadStringAttribute("x", "long_name", xLabel);
  state*=fileObject->ReadStringAttribute("y", "units", yUnits);
  state*=fileObject->ReadStringAttribute("y", "long_name", yLabel);
  state*=fileObject->ReadStringAttribute("h", "units", zUnits);
  state*=fileObject->ReadStringAttribute("h", "long_name", zLabel);
    
  if (state==1)
    {
      avtMeshMetaData *md_BM = new avtMeshMetaData(Bathymetry_Mesh, 
                                                   1, 1, 1, 0, 3, 2, 
                                                   AVT_UNSTRUCTURED_MESH);

      md_BM->xUnits = xUnits;
      md_BM->xLabel = xLabel;
      md_BM->yUnits = yUnits;
      md_BM->yLabel = yLabel;
      md_BM->zUnits = zUnits;
      md_BM->zLabel = zLabel;
      md->Add(md_BM);
      meshExists["Bathymetry_Mesh"] = true;
        
      debug4 << mName << "Added Bathymetry Mesh to MetaData" << endl;
    }

  //-------------------------------------------------------------------
  //
  // Add the SSH_Mesh.
  //
  std::string SSH_Mesh("SSH_Mesh");
  state=1;
  // Get the units for the mesh.
  state*=fileObject->ReadStringAttribute("x", "units", xUnits);
  state*=fileObject->ReadStringAttribute("x", "long_name", xLabel);
  state*=fileObject->ReadStringAttribute("y", "units", yUnits);
  state*=fileObject->ReadStringAttribute("y", "long_name", yLabel);
  state*=fileObject->ReadStringAttribute("zeta", "units", zUnits);
  state*=fileObject->ReadStringAttribute("zeta", "long_name", zLabel);

  if (state==1)
    {
      avtMeshMetaData *md_SSH = new avtMeshMetaData(SSH_Mesh, 
                                                    1, 1, 1, 0, 3, 2, 
                                                    AVT_UNSTRUCTURED_MESH);
        
      md_SSH->xUnits = xUnits;
      md_SSH->xLabel = xLabel;
      md_SSH->yUnits = yUnits;
      md_SSH->yLabel = yLabel;
      md_SSH->zUnits = zUnits;
      md_SSH->zLabel = zLabel;
      md->Add(md_SSH);

      meshExists["SSH_Mesh"] = true;
      debug4 << mName << "Added SSH Mesh to MetaData" << endl;
    }
  //-------------------------------------------------------------------
  //
  // Add the TWOD_Mesh.
  //
  std::string TWOD_Mesh("TWOD_Mesh");
  state=1;

  // Get the units for the mesh.
  state*=fileObject->ReadStringAttribute("x", "units", xUnits);
  state*=fileObject->ReadStringAttribute("x", "long_name", xLabel);
  state*=fileObject->ReadStringAttribute("y", "units", yUnits);
  state*=fileObject->ReadStringAttribute("y", "long_name", yLabel);

  if (state==1)
    {          
      avtMeshMetaData *md_2D = new avtMeshMetaData(TWOD_Mesh, 
                                                   1, 1, 1, 0, 2, 2, 
                                                   AVT_UNSTRUCTURED_MESH);
        
      md_2D->xUnits = xUnits;
      md_2D->xLabel = xLabel;
      md_2D->yUnits = yUnits;
      md_2D->yLabel = yLabel;
      md->Add(md_2D);

      meshExists["TWOD_Mesh"] = true;
      debug4 << mName << "Added TWOD_Mesh to MetaData" << endl;
    }
  //-------------------------------------------------------------------
  //
  // Add the SigmaLayer_Mesh.
  //
  std::string SigmaLayer_Mesh("SigmaLayer_Mesh");
  state=1;

  // Get the units for the mesh:
  state*=fileObject->ReadStringAttribute("x", "units", xUnits);
  state*=fileObject->ReadStringAttribute("x", "long_name", xLabel);
  state*=fileObject->ReadStringAttribute("y", "units", yUnits);
  state*=fileObject->ReadStringAttribute("y", "long_name", yLabel);
  state*=fileObject->ReadStringAttribute("zeta", "units", zUnits);
  state*=fileObject->ReadStringAttribute("zeta", "long_name", zLabel);
  // Get Coord type here to make sure mesh exists
  state*=fileObject->ReadStringAttribute("siglay", "standard_name",SigLayCoordType);

  if (state==1)
    {
      avtMeshMetaData *md_LAY = new avtMeshMetaData(SigmaLayer_Mesh, 
                                                    1, 1, 1, 0, 3, 3, 
                                                    AVT_UNSTRUCTURED_MESH);
        
      md_LAY->xUnits = xUnits;
      md_LAY->xLabel = xLabel;
      md_LAY->yUnits = yUnits;
      md_LAY->yLabel = yLabel;
      md_LAY->zUnits = zUnits;
      md_LAY->zLabel = zLabel;
      md->Add(md_LAY);
      meshExists["SigmaLayer_Mesh"] = true;

      debug4 << mName << "Added Sigma Layer Mesh to MetaData" << endl;
    }
  //------------------------------------------------------------------
  // Add the SigmaLevel_Mesh.
  //
  std::string SigmaLevel_Mesh("SigmaLevel_Mesh");
  state=1;

  // Get the units for the mesh.
  state*=fileObject->ReadStringAttribute("x", "units", xUnits);
  state*=fileObject->ReadStringAttribute("x", "long_name", xLabel);
  state*=fileObject->ReadStringAttribute("y", "units", yUnits);
  state*=fileObject->ReadStringAttribute("y", "long_name", yLabel);
  state*=fileObject->ReadStringAttribute("zeta", "units", zUnits);
  state*=fileObject->ReadStringAttribute("zeta", "long_name", zLabel);
  // Get Coord type here to make sure mesh exists
  state*=fileObject->ReadStringAttribute("siglev", "standard_name",SigLevCoordType);

  if (state==1)
    {
      avtMeshMetaData *md_LEV = new avtMeshMetaData(SigmaLevel_Mesh, 
                                                    1, 1, 1, 0, 3, 3, 
                                                    AVT_UNSTRUCTURED_MESH);
        
      md_LEV->xUnits = xUnits;
      md_LEV->xLabel = xLabel;
      md_LEV->yUnits = yUnits;
      md_LEV->yLabel = yLabel;
      md_LEV->zUnits = zUnits;
      md_LEV->zLabel = zLabel;
      md->Add(md_LEV);
      meshExists["SigmaLevel_Mesh"] = true;

      debug4 << mName << "Added SigmaLevel Mesh to MetaData" << endl;
    }

  

  //-------------------------------------------------------
  //--------------------------------------------------------------------
  //--------------------------------------------------------------------
  // Add Variables!
  //
  debug4<< mName << "Add variables to meta data" << endl;


  std::map<std::string, bool> componentExists;

    
  for(int i = 0; i < nVars; ++i)
    {
      // First identify variable type!
      debug4 << "Examining variable#" << i << endl;
      
      
      nc_type VarType;
      status = nc_inq_var(ncid, i, VarName, &VarType, &VarnDims, 
                          VarDimIDs, &VarnAtts);
      if (status != NC_NOERR) fileObject-> HandleError(status);
      
      //  This just for debugging it is not needed to id vars!
      debug4 << "Variable name: " << VarName << endl;
      debug4 << "variable type";
      if(VarType == NC_BYTE)
        debug4 << "NC_BYTE";
      else if(VarType == NC_CHAR)
        debug4 << "NC_CHAR";
      else if(VarType == NC_SHORT)
        debug4 << "NC_SHORT";
      else if(VarType == NC_INT)
        debug4 << "NC_INT";
      else if(VarType == NC_FLOAT)
        debug4 << "NC_FLOAT";
      else if(VarType == NC_DOUBLE)
        debug4 << "NC_DOUBLE";
      else 
        {
          debug4 << "unknown type: WOW that is bad!";
          EXCEPTION1(InvalidVariableException, VarName);
        }
      debug4<< endl;
       
       
      debug4<< "Variable Dimensions=[";
       
      for (int vd=0; vd<VarnDims; ++vd)
        {
          debug4<< dimSizes[VarDimIDs[vd]] << " ";
        }
         
      debug4<< "]" << endl;
       
      // Add flexibility for a second mesh on which to load the data: how do we keep the names straight? We can't just use the variable name anymore!?!?!
      std::string Var_Mesh;
      fileObject->ReadStringAttribute(VarName,"Name_Mesh", Var_Mesh);


      // The Grid on which to load data should be determined from the file meta data. This allows for more flexability in loading the data. The old method is kept for backward compatiablity.

//       debug4 << "Var_Mesh: " << Var_Mesh << endl;
//       debug4 << "Bathymetry_Mesh: " << Bathymetry_Mesh << endl;
//       debug4 << "TWOD_Mesh: " << TWOD_Mesh << endl;
//       debug4 << "SSH_Mesh: " << SSH_Mesh << endl;
//       debug4 << "SigmaLayer_Mesh: " << SigmaLayer_Mesh << endl;
//       debug4 << "SigmaLevel_Mesh: " << SigmaLevel_Mesh << endl;
      
      // BATHYMETRY MESH
      if (strcmp(Var_Mesh.c_str(),Bathymetry_Mesh.c_str())==0 &&
          VarDimIDs[(VarnDims-1)] == nNodeID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,Bathymetry_Mesh, AVT_NODECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }

      else if (strcmp(Var_Mesh.c_str(),Bathymetry_Mesh.c_str())==0 &&
               VarDimIDs[(VarnDims-1)] == nElemID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,Bathymetry_Mesh, AVT_ZONECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      // TWOD MESH
      else if (strcmp(Var_Mesh.c_str(),TWOD_Mesh.c_str())==0 &&
               VarDimIDs[(VarnDims-1)] == nNodeID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,TWOD_Mesh, AVT_NODECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      else if (strcmp(Var_Mesh.c_str(),TWOD_Mesh.c_str())==0 &&
               VarDimIDs[(VarnDims-1)] == nElemID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,Bathymetry_Mesh, AVT_ZONECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      // SSH MESH
      else if (strcmp(Var_Mesh.c_str(),SSH_Mesh.c_str())==0 &&
               VarDimIDs[(VarnDims-1)] == nNodeID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,SSH_Mesh, AVT_NODECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      else if (strcmp(Var_Mesh.c_str(),SSH_Mesh.c_str())==0 &&
               VarDimIDs[(VarnDims-1)] == nElemID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,SSH_Mesh, AVT_ZONECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      // SIGMALAYER MESH
      else if (strcmp(Var_Mesh.c_str(),SigmaLayer_Mesh.c_str())==0 &&
               VarDimIDs[(VarnDims-1)] == nNodeID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,SigmaLayer_Mesh, AVT_NODECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      else if (strcmp(Var_Mesh.c_str(),SigmaLayer_Mesh.c_str())==0 &&
               VarDimIDs[(VarnDims-1)] == nElemID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,SigmaLayer_Mesh, AVT_ZONECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      // SIMGALEVEL MESH
      else if (strcmp(Var_Mesh.c_str(),SigmaLevel_Mesh.c_str())==0 &&
               VarDimIDs[(VarnDims-1)] == nNodeID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,SigmaLevel_Mesh, AVT_NODECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      else if (strcmp(Var_Mesh.c_str(),SigmaLevel_Mesh.c_str())==0 &&
               VarDimIDs[(VarnDims-1)] == nElemID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,SigmaLevel_Mesh, AVT_ZONECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      
      else // GO BACK TO THE OLD METHOD!
        {
      //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      if (VarnDims==1)
        {
          if (VarDimIDs[0] == nNodeID )
            {
              if (strncmp(VarName, "h",1)==0)
                {
                  // THIS GETS THE BATHYMETRY, WHICH HAS THE SAME DIMESIONS
                  // AS THE GRID VARIABLES.
                  avtScalarMetaData *smd = 
                    new avtScalarMetaData(VarName,Bathymetry_Mesh, AVT_NODECENT);
                  smd->hasUnits = fileObject->
                    ReadStringAttribute(VarName,"units", smd->units);
                  md->Add(smd);
                  componentExists[VarName] = true;
                }
              else
                {
                  // THIS ONE IS JUST FOR THE GRID VARS.
                  avtScalarMetaData *smd = 
                    new avtScalarMetaData(VarName,TWOD_Mesh, AVT_NODECENT);
                  smd->hasUnits = fileObject->
                    ReadStringAttribute(VarName,"units", smd->units);
                  md->Add(smd);
                  componentExists[VarName] = true;
                }
            } // end if:   (VarDimIDs[0] == nNodeID )
          else if  (VarDimIDs[0] == nElemID )
            {
              // THIS GETS THE PARALLEL PROCESSING DOMAINS
              avtScalarMetaData *smd = 
                new avtScalarMetaData(VarName,TWOD_Mesh, AVT_ZONECENT);
              smd->hasUnits = fileObject->
                ReadStringAttribute(VarName,"units", smd->units);
              md->Add(smd);
              componentExists[VarName] = true;
            } // end if:   (VarDimIDs[0] == nElem )
           
          // Add more 1D data here!
           
        } // end if (varndims ==1 )
       
       
      //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      else if (VarnDims==2)
        {
          if  (VarDimIDs[0] == nTimeID &&
               VarDimIDs[1] == nNodeID)
            {
              if(strncmp(VarName, "zeta",5)==0)
                {
                  // SEA SURFACE HEIGHT
                  avtScalarMetaData *smd = 
                    new avtScalarMetaData(VarName,SSH_Mesh, AVT_NODECENT);
                  smd->hasUnits = fileObject->
                    ReadStringAttribute(VarName,"units", smd->units);
                  md->Add(smd);
                  componentExists[VarName] = true;
                }
              else
                {
                  avtScalarMetaData *smd = 
                    new avtScalarMetaData(VarName,TWOD_Mesh, AVT_NODECENT);
                  smd->hasUnits = fileObject->
                    ReadStringAttribute(VarName,"units", smd->units);
                  md->Add(smd);
                  componentExists[VarName] = true;
                } 
            } // end if: (VarDimIDs[0] == nTimeID &&
          //              VarDimIDs[1] == nNodeID)
           
           
          // add any time varrying TWOD to the TWOD mesh
          else if (VarDimIDs[0] == nTimeID &&
                   VarDimIDs[1] == nElemID)
            {
              avtScalarMetaData *smd = 
                new avtScalarMetaData(VarName,TWOD_Mesh, AVT_ZONECENT);
              smd->hasUnits = fileObject->
                ReadStringAttribute(VarName,"units", smd->units);
              md->Add(smd);
              componentExists[VarName] = true;
            }
          else if (VarDimIDs[0] == nThreeID &&
                   VarDimIDs[1] == nElemID)
            {
              // Cell Angle arrays for gradient operator and NV
              //          avtArrayMetaData *smd = new avtArrayMetaData(VarName,
              //            TWOD_Mesh, AVT_ZONECENT, three);
              //              md->Add(smd);
              //          componentExists[VarName] = true;
            }
          else if (VarDimIDs[0] == nFourID &&
                   VarDimIDs[1] == nElemID)
            {
              // More Cell Angle arrays for gradient operator
              //          avtArrayMetaData *smd = new avtArrayMetaData(VarName,
              //            TWOD_Mesh, AVT_ZONECENT, four);
              //              md->Add(smd);
              //          componentExists[VarName] = true;
            }
          else if (VarDimIDs[0] == nMaxnodeID &&
                   VarDimIDs[1] == nNodeID)
            {
              // nbsn: nodes surrounding each node
              //          avtArrayMetaData *smd = new avtArrayMetaData(VarName,
              //            TWOD_Mesh, AVT_NODECENT, maxnode);
              //              md->Add(smd);
              //          componentExists[VarName] = true;
            }
            
          else if (VarDimIDs[0] == nMaxelemID &&
                   VarDimIDs[1] == nNodeID)

            {
              // nbve: Cells surrounding each node
              //          avtArrayMetaData *smd = new avtArrayMetaData(VarName,
              //            TWOD_Mesh, AVT_NODECENT, maxelem);
              //              md->Add(smd);
              //          componentExists[VarName] = true;
            }
            
          // add more 2D data here     
            
        }// end if: (varndims==2)
    
      //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      else if (VarnDims==3)
        {
          if  (VarDimIDs[0] == nTimeID &&
               VarDimIDs[1] == nSiglayID &&
               VarDimIDs[2] == nNodeID)
            {
              // This catches all the usual scalars!
              avtScalarMetaData *smd = 
                new avtScalarMetaData(VarName,SigmaLayer_Mesh, AVT_NODECENT);
              smd->hasUnits = fileObject->
                ReadStringAttribute(VarName,"units", smd->units);
              md->Add(smd);
              componentExists[VarName] = true;
            }
          else if (VarDimIDs[0] == nTimeID &&
                   VarDimIDs[1] == nSiglevID &&
                   VarDimIDs[2] == nNodeID)
            {
              // This catches KM in the new, correct location!
              avtScalarMetaData *smd = 
                new avtScalarMetaData(VarName,SigmaLevel_Mesh, AVT_NODECENT);
              smd->hasUnits = fileObject->
                ReadStringAttribute(VarName,"units", smd->units);
              md->Add(smd);
              componentExists[VarName] = true;
            }
          else if (VarDimIDs[0] == nTimeID &&
                   VarDimIDs[1] == nSiglayID &&
                   VarDimIDs[2] == nElemID)
            {
              // This catches KM in the old, incorrect location
              // This also catches u,v,w if they exist!
              avtScalarMetaData *smd = 
                new avtScalarMetaData(VarName,SigmaLevel_Mesh, AVT_ZONECENT);
              smd->hasUnits = fileObject->
                ReadStringAttribute(VarName,"units", smd->units);
              md->Add(smd);
              componentExists[VarName] = true;
            }
        
        } // end if: (VarnDims ==3)

        } // else use the old method!

    } // end for nvars !!!


  if(componentExists.find("u") != componentExists.end() &&
     componentExists.find("v") != componentExists.end() &&
     componentExists.find("ww") != componentExists.end())
    {
      debug4 << mName << "Velocity is 3D" << endl;
      avtVectorMetaData *smd = new avtVectorMetaData("3DVEL",
                                                     SigmaLevel_Mesh, AVT_ZONECENT,3);
      smd->hasUnits = fileObject->ReadStringAttribute("u", 
                                                      "units", smd->units);
      md->Add(smd);    
    }

  if(componentExists.find("u") != componentExists.end() &&
     componentExists.find("v") != componentExists.end())
    {
      debug4 << mName <<"Velocity is 2D" << endl;
      avtVectorMetaData *smd = new avtVectorMetaData("2DVEL",
                                                     SigmaLevel_Mesh, AVT_ZONECENT,3);
      smd->hasUnits = fileObject->ReadStringAttribute("u", 
                                                      "units", smd->units);
      md->Add(smd);
    }

  // Add conditional variables!
  // IF temperature and salinity variables exist calculate density
  // Three options are available, one using potential temperature
  //                              one using insitu temperature
  //                              one for shallow water
  if(componentExists.find("temp") != componentExists.end() &&
     componentExists.find("salinity") != componentExists.end())
    {
      debug4 << mName << "Adding Density Variables:" << endl;
    
      avtScalarMetaData *dens3_md = new avtScalarMetaData("Dens3{S,Theta,P}",
                                                          SigmaLayer_Mesh, AVT_NODECENT);
      dens3_md->hasUnits = true;
      dens3_md->units = "kg/m3";
      md->Add(dens3_md);
      componentExists["Dens3{S,Theta,P}"] = true;

      avtScalarMetaData *dens_md = new avtScalarMetaData("Dens{S,T,P}",
                                                         SigmaLayer_Mesh, AVT_NODECENT);
      dens_md->hasUnits = true;
      dens_md->units = "kg/m3";
      md->Add(dens_md);
      componentExists["Dens{S,T,P}"] = true;

      avtScalarMetaData *theta_md = new avtScalarMetaData("Theta{S,T,P,0}",
                                                          SigmaLayer_Mesh, AVT_NODECENT);
      theta_md->hasUnits = true;
      theta_md->units = "Degrees_C";
      md->Add(theta_md);
      componentExists["Theta{S,T,P,0}"] = true;

      // Removed for 1.5.4 release
      //      avtScalarMetaData *dens2_md = new avtScalarMetaData("Dens2(S,Theta,0)",
      //          SigmaLayer_Mesh, AVT_NODECENT);
      //      dens2_md->hasUnits = true;
      //      dens2_md->units = "kg/m3";
      //      md->Add(dens2_md);
      //      componentExists["Dens2(S,Theta,0)"] = true;
    
    }

  //    debug4 << "meshExists state" << meshExists.find("SSH_MESH") << endl;

  if (meshExists.find("SigmaLayer_Mesh") != meshExists.end() )
    {
      avtScalarMetaData *Laynodes_md = new avtScalarMetaData("Select Layer",
                                                             SigmaLayer_Mesh, AVT_NODECENT);
      Laynodes_md->hasUnits = false;
      md->Add(Laynodes_md);
    }

  if (meshExists.find("SigmaLevel_Mesh") != meshExists.end() )
    {
      avtScalarMetaData *Levnodes_md = new avtScalarMetaData("Select Level",
                                                             SigmaLevel_Mesh, AVT_NODECENT);
      Levnodes_md->hasUnits = false;
      md->Add(Levnodes_md);
    }



  debug4 << "All dims=[" ; 
  for(int i = 0; i < nDims; ++i)
    {
      debug4<< dimSizes[i] << " ";
    }
  debug4<< "]" << endl;



  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // ALL Variables added to META DATA

  // Removed Materials from 1.5.4
  // Not needed because ISOSURFACE works better!
# ifdef Materials_stuff
  //Add Material MetaData for: SigmaLayer_Mesh
  avtMaterialMetaData *matmd_1 = new avtMaterialMetaData;
  matmd_1->name = "Sigma_Layers";
  matmd_1->meshName = "SigmaLayer_Mesh";
  matmd_1->numMaterials=nSigLayers;

  for (int i=0; i < nSigLayers; ++i)
    {

      char buffer[50];
      int n;
      n=sprintf(buffer, "Layer %d",i);
      matmd_1->materialNames.push_back(buffer);
    }


  md->Add(matmd_1);


  //Add Material MetaData for: SigmaLevel_Mesh
  avtMaterialMetaData *matmd_2 = new avtMaterialMetaData;
  matmd_2->name = "Sigma_Levels";
  matmd_2->meshName = "SigmaLevel_Mesh";
  matmd_2->numMaterials=nSigLayers;

  for (int i=0; i < nSigLayers; ++i)
    {
      char buffer [50];
      int n;
      n=sprintf(buffer, "Layer %d",i);
      matmd_2->materialNames.push_back(buffer);

    }

  md->Add(matmd_2);
# endif


  debug4 << mName << "END" <<endl;
}

// ****************************************************************************
//  Method: avtFVCOMReader::GetStaticGridVariables
//
//  Purpose:
//          Read in and store grid variables which do not change 
//          with the time step. This prevents reading them repeatedly!
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************


void
avtFVCOMReader::GetStaticGridVariables(void)
{

  const char *mName = "avtFVCOMReader::GetStaticGridVariables: ";
  debug4 << mName << endl;

  int state=1;


  // Get Node locations  
  xvals = new float[nNode];
  state*=fileObject->ReadVariableInto("x", FLOATARRAY_TYPE, xvals);
  alloc_xvals=true;
  
  yvals = new float[nNode];
  state*=fileObject->ReadVariableInto("y", FLOATARRAY_TYPE, yvals);
  alloc_yvals=true;


  // Get Node Connectivity
  nvvals= new int[nThree * nElem];
  state*=fileObject->ReadVariableInto("nv", INTEGERARRAY_TYPE, nvvals);
  alloc_nvvals=true;

  if (state!=1)
    {
      debug4 << "Critical Grid Coordinates could not be loaded" << endl;
      std::string msg("Critical Grid Coordinates could not be loaded");      
      EXCEPTION1(ImproperUseException, msg);
    }


  // Bathymetry
  zvals = new float[nNode];
  state*=fileObject->ReadVariableInto("h", FLOATARRAY_TYPE, zvals);
  alloc_zvals=true;

  
  // Added these to MetaData method
  bool getsiglay=false;
  state*=fileObject->ReadStringAttribute("siglay", "standard_name",SigLayCoordType);
  if (state==1) getsiglay=true;
  

  bool getsiglev=false;
  state*=fileObject->ReadStringAttribute("siglev", "standard_name",SigLevCoordType);
  if (state==1) getsiglev=true;
    
  if (strcmp("ocean_sigma/general_coordinate",SigLayCoordType.c_str())==0)
    {
      SigLayers = new float[nSiglay*nNode];
      fileObject->ReadVariableInto("siglay", FLOATARRAY_TYPE, SigLayers);
      alloc_SigLayers=true;

    }
  else if (strcmp("ocean_sigma_coordinate",SigLayCoordType.c_str())==0)
    {
      debug4 << "here I am" << endl;
      SigLayers = new float[nSiglay];
      fileObject->ReadVariableInto("siglay", FLOATARRAY_TYPE, SigLayers);
      alloc_SigLayers=true;

      debug4 << "SigLayers[1]= " << SigLayers[1] << endl; 
      debug4 << "SigLayers[nSiglay]= " << SigLayers[nSiglay-1] << endl; 
    }
  else if (getsiglay==true) // only if the data exists but the file coord is unrecognized
    {
      debug4 << "Vertical coordinate type not recognized" << endl;
      debug4 << "siglay:standard_name returned:"<< SigLayCoordType << endl;
      debug4 << "Known options are:" << endl;
      debug4 << "ocean_sigma-coordinates OR ocean_sigma/general_coordinate" << endl;
      std::string msg("Unsupported principal vertical coordinate");
      EXCEPTION1(ImproperUseException, msg);
    }
  else debug4 << "No Sigma Layers variable found" << endl;


  if (strcmp("ocean_sigma/general_coordinate",SigLevCoordType.c_str())==0)
    {
      SigLevels = new float[nSiglev*nNode];
      fileObject->ReadVariableInto("siglev", FLOATARRAY_TYPE, SigLevels);
      alloc_SigLevels=true;

    }
  else if (strcmp("ocean_sigma_coordinate",SigLevCoordType.c_str())==0)
    {
      SigLevels = new float[nSiglev];
      fileObject->ReadVariableInto("siglev", FLOATARRAY_TYPE, SigLevels); 

      alloc_SigLevels=true;

      debug4 << "SigLevels[1]= " << SigLevels[1] << endl; 
      debug4 << "SigLevels[nsiglev]= " << SigLevels[nSiglev-1] << endl; 

    }
  else if (getsiglev==true)
    {
      debug4 << "Vertical coordinate type not recognized" << endl;
      debug4 << "siglay:standard_name returned:"<< SigLayCoordType << endl;
      debug4 << "Known options are:" << endl;
      debug4 << "ocean_sigma-coordinates OR ocean_sigma/general_coordinate" << endl;
      std::string msg("Unsupported principal vertical coordinate");
      EXCEPTION1(ImproperUseException, msg);
    }    
  else debug4 << "No Sigma Levels variable found" << endl;

  
  NeedGridVariables=false;


  debug4 << mName << "end" << endl;

  return;
}

// ****************************************************************************
//  Method: avtFVCOMReader::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************

vtkDataSet *
avtFVCOMReader::GetMesh(int timestate, const char *mesh, avtVariableCache *cache)
{
  const char *mName = "avtFVCOMReader::GetMesh: ";
  debug4 << mName << "meshname=" << mesh  << " timestate="
         << timestate << endl;

  if(strcmp(mesh, "Bathymetry_Mesh") == 0)
    {
      timestate=0; // bathymetry is not time varying in FVCOM!
    }
  else  if(strcmp(mesh, "TWOD_Mesh") == 0)
    {
      timestate=0; // bathymetry is not time varying in FVCOM!
    }


  if (NeedDimensions) GetDimensions();
  if(NeedGridVariables) GetStaticGridVariables();

  // note: GetStaticGrid Variables returns the connectivity variable nvvals
  // nvvals is  [3 * nElem] linear index to the node numbers that are the 
  // vertices of each cell. 
  // nvvals[i=0,i<nElem] are the first vertices of each cell
  // nvvals[i=nElem,i<2*nElem] are the second verticies of each cell
  // nvvals[i=2*nElem,i<3*nElem] are the third verticies of each cell
  // ***************************************************************
  // nvvals as stored from fvcom counts nodes from 1 to nNodes
  // Visit must cound from 0 to nNodes-1
  // ***************************************************************

  debug4 << mName << "Looking to see if Mesh is in cache" << endl;
  vtkObject *obj =0;
  int domain=CacheDomainIndex; //  Set domain for MTMD!!!
  const char *matname = "all";
  obj = cache->GetVTKObject(mesh, avtVariableCache::DATASET_NAME,
                            timestate, domain, matname);
  if(obj != 0)
    {
      debug4 << mName << "Getting mesh: " << mesh << " from Cache" << endl;

      vtkUnstructuredGrid *ugrid = (vtkUnstructuredGrid *)obj;
      ugrid->Register(NULL);
      return ugrid;
    }
  else
    {
      debug4 << mName << "Mesh Not in Cache. Load from data." << endl;
    }

  vtkDataSet *retval = 0;

  // Bail out if we can't get the file handle.
  ncid = fileObject->GetFileHandle();
  if(ncid == -1) // if the mesh does not exist!
    {
      EXCEPTION1(InvalidVariableException, mesh);
    }

  //-------------------------------------------------------------------------------
  //-------------------------------------------------------------------------------
  // MAKE THE MESH FOR EACH TYPE
  if(strcmp(mesh, "Bathymetry_Mesh") == 0)
    {
      debug4 << mName << "Getting Bathymetry_Mesh" << endl;
      // for Bathymetry mesh we only need one layer, only nNodesPerLayer nodes!
      vtkPoints *pts = vtkPoints::New();
      pts->Allocate(nNode);

      debug4 << "nPTS: " << nNode << endl;

      vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
      ugrid->SetPoints(pts);
      ugrid->Allocate(nElem);
      pts->Delete();

      debug4 << "nCell: " << nElem << endl;


      debug4 << "Allocated ugrid and pts" << endl;


      // insert nodes into mesh pts object
      for(int i= 0; i< nNode; ++i)
        {
          float pt[3]={xvals[i], yvals[i],-zvals[i]};
          pts->InsertNextPoint(pt);
        }
  

      // insert cells in to mesh cell object
      vtkIdType verts[3];
      for(int cell = 0; cell < nElem; ++cell)
        {
          for(int vrt =0; vrt <3; ++vrt)
            {
              verts[vrt] = nvvals[vrt * nElem + cell] -1;
            }
          ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
        }

      debug4 << mName;
      debug4 << "Success Returning VTK_TRIANCLE for Bathymetry" << endl;

      retval = ugrid;
    } // End if Bathymetry_Mesh
      //--------------------------------------------------------------------------
      //--------------------------------------------------------------------------
      //-------------------------------------------------------------------------------
      //-------------------------------------------------------------------------------
      //    Make the mesh for simple 2D data sets
  else if(strcmp(mesh, "TWOD_Mesh") == 0)
    {
      debug4 << mName << "Getting TWOD_Mesh" << endl;
      // for 2D mesh we only need one layer, only nNodesPerLayer nodes!
      vtkPoints *pts = vtkPoints::New();
      pts->Allocate(nNode);

      debug4 << "nPTS: " << nNode << endl;

      vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
      ugrid->SetPoints(pts);
      ugrid->Allocate(nElem);
      pts->Delete();

      debug4 << "nCell: " << nElem << endl;

      debug4 << "Allocated ugrid and pts" << endl;


      // insert nodes into mesh pts object
      for(int i= 0; i< nNode; ++i)
        {
          float pt[3]={xvals[i], yvals[i],0};
          pts->InsertNextPoint(pt);
        }
  

      // insert cells in to mesh cell object
      vtkIdType verts[3];
      for(int cell = 0; cell < nElem; ++cell)
        {
          for(int vrt =0; vrt <3; ++vrt)
            {
              verts[vrt] = nvvals[vrt * nElem + cell] -1;
            }
          ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
        }


      debug4 << mName;
      debug4 << "Success Returning VTK_TRIANCLE for TWOD_MESH" << endl;

      retval = ugrid;
    } // End if Bathymetry_Mesh
      //--------------------------------------------------------------------------
      //--------------------------------------------------------------------------
      //--------------------------------------------------------------------------
  else if(strcmp(mesh, "SSH_Mesh") == 0) // is mesh: SSH_Mesh
    {    
      debug4 << mName << "Getting SSH_Mesh"<< endl;
      // for SSH mesh we only need one layer, only nNodesPerLayer nodes!
      vtkPoints *pts = vtkPoints::New();
      pts->Allocate(nNode);

      debug4 << "nPTS: " << nNode << endl;

      vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
      ugrid->SetPoints(pts);
      ugrid->Allocate(nElem);
      pts->Delete();

      debug4 << "nCell: " << nElem << endl;

      debug4 << "Allocated ugrid and pts" << endl;
    
      // Need to specify time step to read SSH: timestate
      debug4 << "timestate=" << timestate << endl;

      size_t starts[]={timestate,0};
      size_t counts[]={1, nNode};
      ptrdiff_t stride[]={1,1};
      float *sshvals = new float[nNode];
      int var_id;
      status = nc_inq_varid (fileObject->GetFileHandle(), "zeta", &var_id);
      if (status != NC_NOERR) fileObject-> HandleError(status);

      status = nc_get_vars_float(fileObject->GetFileHandle(),var_id,
                                 starts, counts, stride, sshvals);
      if (status != NC_NOERR) fileObject-> HandleError(status);


      // insert nodes into mesh pts object
      for(int i= 0; i< nNode; ++i)
        {
          float pt[3]={xvals[i], yvals[i], sshvals[i]};
          pts->InsertNextPoint(pt);
        } // end for nNodesPerLayer

          // insert cells into mesh cell object
      vtkIdType verts[3];
      for(int cell = 0; cell < nElem; ++cell)
        {
          for(int vrt =0; vrt <3; ++vrt)
            {
              verts[vrt] = nvvals[vrt * nElem + cell] -1;
            }
          ugrid->InsertNextCell(VTK_TRIANGLE, 3, verts);
        } // End for nCellsPerLayer


      delete [] sshvals;

      debug4 << mName;
      debug4 << "Success Returning VTK_TRIANCLE for ssh mesh" << endl;

      retval = ugrid;
    } // End if SSH_Mesh

      //--------------------------------------------------------------------------
      //--------------------------------------------------------------------------
      //--------------------------------------------------------------------------
  else if(strcmp(mesh, "SigmaLayer_Mesh") == 0)
    { // SigmaLayer_Mesh
      // Plot scalars which are calculated at cell edge centers
      // On this grid these scalars will be on cell nodes
        
      debug4<< mName << "Getting SigmaLayer_Mesh" << endl;
      //Sigma layer mesh has one fewer layers and one fewer levels!
      int tNodes = nNode * (nSiglay);
      int tCells = nElem * (nSiglay-1);

      debug4 << "tNodes= "<< tNodes << endl;
        
      vtkPoints *pts = vtkPoints::New();
      pts->Allocate(tNodes);

      vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
      ugrid->SetPoints(pts);
      ugrid->Allocate(tCells);

      debug4 << "tCell: " << tCells << endl;

      pts->Delete();

      // Need to specify time step to read SSH: timestate
      debug4 << "timestate=" << timestate << endl;

      size_t starts[]={timestate,0};
      size_t counts[]={1, nNode};
      ptrdiff_t stride[]={1,1};
      float *sshvals = new float[nNode];
      int var_id;
      status = nc_inq_varid (fileObject->GetFileHandle(), "zeta", &var_id);
      if (status != NC_NOERR) fileObject-> HandleError(status);
       
      // SSH
      status = nc_get_vars_float(fileObject->GetFileHandle(),var_id,
                                 starts, counts, stride, sshvals);
      if (status != NC_NOERR) fileObject-> HandleError(status);
       

      // CHANGE TO ADD GENERAL OCEAN COORDINATES!

      // the coordinates are now read in GetStaticGridVariables method
      //      std::string CoordType;
      //      fileObject->ReadStringAttribute("siglay", "standard_name",CoordType);
      //        debug4 << "CoordType= " << CoordType << endl;

      //        bool isGeneralCoord=false;
      //isGeneralCoord=strcmp("ocean_sigma/general_coordinate",
      //                       CoordType.c_str())==0;

      //        debug4 << "isGeneralCoord= " << isGeneralCoord << endl;
        
      //        if (isGeneralCoord == true)
      if (strcmp("ocean_sigma/general_coordinate",SigLayCoordType.c_str())==0)
        {
          // sigmalayers
          //          float *SigLayers = new float[nSiglay*nNode];
          //          fileObject->ReadVariableInto("siglay", FLOATARRAY_TYPE, SigLayers); 

          debug4 << mName; 
          debug4 << "General Coordinates!"<< endl;


          // insert nodes into mesh pts object
          float depth;
          for(int lay = 0; lay< nSiglay; ++lay) // order! surface -> bottom
            {
              for(int node= 0; node< nNode; ++node)
                {
                  int index = lay*nNode + node;
                  depth= SigLayers[index]*(zvals[node]+sshvals[node])
                    +sshvals[node];

                  float pt[3]={xvals[node], yvals[node], depth};
                  pts->InsertNextPoint(pt);
                }// end for nNodesPerLayer
            }// end for nSigLevels
          debug4 << "Inserted points to General Coord Layer Mesh" << endl;

        }
      else if (strcmp("ocean_sigma_coordinate",SigLayCoordType.c_str())==0) 
        // USE OCEAN SIGMA COORDINATE
        {
          // sigmalayers
          //          float *SigLayers = new float[nSiglay];
          //          fileObject->ReadVariableInto("siglay", FLOATARRAY_TYPE, SigLayers); 
        
          debug4 << mName; 
          debug4 << "Sigma Coordinates!"<< endl;


          // insert nodes into mesh pts object
          float depth;
          for(int lay = 0; lay< nSiglay; ++lay) // order! surface -> bottom
            {
              for(int node= 0; node< nNode; ++node)
                {
                  depth= SigLayers[lay]*(zvals[node]+sshvals[node])+sshvals[node];
                  float pt[3]={xvals[node], yvals[node], depth};
                  pts->InsertNextPoint(pt);
                }// end for nNodesPerLayer
            }// end for nSigLevels
          debug4 << "Inserted points to Sigma Coord. Layer Mesh" << endl;

        }


      // insert cells into mesh cell object
      vtkIdType verts[6];

      for(int lay = 0; lay < (nSiglay-1); ++lay)
        {
          for(int cell = 0; cell < nElem; ++cell)
            {
              for(int TB=0; TB<2; ++TB) // TopOrBottom of cell!
                for(int vrt =0; vrt <3; ++vrt)
                  {
                    verts[vrt + TB*3] = lay*nNode + 
                      // offset each layer by the number of nodes
                        
                      (nvvals[vrt * nElem + cell]-1) + 
                      // offset each nvval by the number of cells per layer

                      TB*nNode;
                    // offset the bottom nodes from the top by nNodesPerLayer

                  } // End for verts per cell and TB

    
              ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
            } // End for nCellsPerLayer
        } // End For nSigLayers

      delete [] sshvals;

      debug4 << mName;
      debug4 << "Success Returning VTK_WEDGE: SigmaLayerMesh" << endl;

      retval = ugrid;
    } // end SigmaLayers Mesh!
      //--------------------------------------------------------------------------
      //--------------------------------------------------------------------------
      //--------------------------------------------------------------------------
  else if(strcmp(mesh, "SigmaLevel_Mesh") == 0)
    {   // is mesh: SigmaLevel_Mesh
      // plot turbulent quantities which occur at cell nodes (fvcom 2.5+)
      // plot turbulent quantities which occur at cell center (fvcom <2.5)
      // [Note cell center km is not at the right level!]

      // plot velocity at cell centers
      debug4 << mName << "Getting SigmaLevel Mesh" << endl;

      int tNodes = nNode * nSiglev;
      int tCells = nElem * (nSiglev-1);

      vtkPoints *pts = vtkPoints::New();
      pts->Allocate(tNodes);

     debug4 << "nPTS: " << tNodes << endl;


      vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
      ugrid->SetPoints(pts);
      ugrid->Allocate(tCells);
      pts->Delete();

      debug4 << "tCells: " << tCells << endl;

      debug4 << "Allocated ugrid and pts" << endl;

      // Need to specify time step to read SSH: timestate
      debug4 << "timestate=" << timestate << endl;

      size_t starts[]={timestate,0};
      size_t counts[]={1, nNode};
      ptrdiff_t stride[]={1,1};
      float *sshvals = new float[nNode];
      int var_id;
      status = nc_inq_varid (fileObject->GetFileHandle(), "zeta", &var_id);
      if (status != NC_NOERR) fileObject-> HandleError(status);

      // SSH
      status = nc_get_vars_float(fileObject->GetFileHandle(),var_id,
                                 starts, counts, stride, sshvals);
      if (status != NC_NOERR) fileObject-> HandleError(status);


      // CHANGE TO ADD GENERAL OCEAN COORDINATES!
      //      std::string CoordType;
      //      fileObject->ReadStringAttribute("siglev", "standard_name",CoordType);
      //        debug4 << "CoordType= " << CoordType << endl;

      //        bool isGeneralCoord=false;
      //isGeneralCoord=strcmp("ocean_sigma/general_coordinate",
      //                       CoordType.c_str())==0;

      //        debug4 << "isGeneralCoord= " << isGeneralCoord << endl;
      if (strcmp("ocean_sigma/general_coordinate",SigLevCoordType.c_str())==0)
        {
          // sigmalevels
          //          float *SigLevels = new float[nSiglev*nNode];
          //          fileObject->ReadVariableInto("siglev", FLOATARRAY_TYPE, SigLevels); 
         
          debug4 << mName; 
          debug4 << "General Coordinates"<< endl;

          // insert nodes into mesh pts object
          float depth;
          for(int lev = 0; lev< nSiglev; ++lev) // order! surface -> bottom
            {
              for(int node= 0; node< nNode; ++node)
                { 
                  int index = lev*nNode + node;
                  depth= SigLevels[index]*(zvals[node]+sshvals[node])+sshvals[node];
                  float pt[3]={xvals[node], yvals[node], depth};
                  pts->InsertNextPoint(pt);
                }// end for nNodesPerLayer
            }// end for nSigLevels

          debug4 << "Inserted points to Sigma Coord. Level Mesh" << endl;

        }
      else if (strcmp("ocean_sigma_coordinate",SigLevCoordType.c_str())==0) 
        {
          // sigmalevels
          //          float *SigLevels = new float[nSiglev];
          //          fileObject->ReadVariableInto("siglev", FLOATARRAY_TYPE, SigLevels); 
         
          debug4 << mName; 
          debug4 << "Sigma Coordinates!"<< endl;

          // insert nodes into mesh pts object
          float depth;
          for(int lev = 0; lev< nSiglev; ++lev) // order! surface -> bottom
            {
              for(int node= 0; node< nNode; ++node)
                { 
                  depth= SigLevels[lev]*(zvals[node]+sshvals[node])+sshvals[node];
                  float pt[3]={xvals[node], yvals[node], depth};
                  pts->InsertNextPoint(pt);
                }// end for nNodesPerLayer
            }// end for nSigLevels

          debug4 << "Inserted points to Sigma Coord. Level Mesh" << endl;

        }
      
      // insert cells into mesh cell object
      vtkIdType verts[6];
      for(int lay = 0; lay < nSiglay; ++lay)
        {
          for(int cell = 0; cell < nElem; ++cell)
            {
              for(int TB=0; TB<2; ++TB) // TopOrBottom of cell!
                for(int vrt =0; vrt <3; ++vrt)
                  {
                    verts[vrt + TB*3] = lay*nNode + 
                      // offset each layer by the number of nodes

                      (nvvals[vrt * nElem + cell]-1) + 
                      // offset each nvval by the number of cells per layer

                      TB*nNode;
                    // offset the bottom nodes from the top by nNodesPerLayer

                  } // End for verts per cell and TB
    
              ugrid->InsertNextCell(VTK_WEDGE, 6, verts);
            } // End for nCellsPerLayer
        } // End For nSigLayers


      delete [] sshvals;

      debug4 << mName;
      debug4 << "Success Returning VTK_WEDGE: SigmaLevels" << endl;

      retval = ugrid;
    } // end SigmaLevels Mesh!
  else // Can't match mesh!!!
    {
      EXCEPTION1(InvalidVariableException, mesh);
    }

  // Loaded variable succesfully, add to cache
  debug4 << mName << "Add mesh to cache!" << endl;
  cache->CacheVTKObject(mesh, avtVariableCache::DATASET_NAME, timestate, domain,
                        matname, retval);


  debug4 << "All dims=[" ; 
  for(int i = 0; i < nDims; ++i)
    {
      debug4<< dimSizes[i] << " ";
    }
  debug4<< "]" << endl;


  debug4 << mName << "END" << endl;

  return retval;
}




// ****************************************************************************
// Method: avtFVCOMReaderFileFormat::GetAuxiliaryData
//
// Purpose: 
//   Gets the material object for the particles.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: David Stuebe
// Creation:   Mon Jul 17 2006
//
// Modifications:
//   
// ****************************************************************************

void *
avtFVCOMReader::GetAuxiliaryData(const char *var, int ts,
                                 const char *type, void *args, DestructorFunction &df)
{
  void *retval = 0;


  const char *mName = "avtFVCOMReader::GetAuxiliaryData: ";
  debug4 << mName << "timestate=" << ts << endl
         << ", VarName=" << var << endl
         << "type=" << type << endl
         << "args=" << args << endl;

#ifdef materials_method
  // NOT WORKING PROPERLY>>>> compiles and runs with out error, 
  // UNEXPECTED BEHAVIOR: does not turn on and off the right cells?


  if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0 &&
     strcmp(var, "Sigma_Levels") == 0)
    {
      int status;
      int ncid;
      ncid=fileObject->GetFileHandle(); 

      // NELE
      size_t nCellsPerLayer;
      int nele_id;
      status = nc_inq_dimid(ncid, "nele", &nele_id);
      if (status != NC_NOERR) fileObject-> HandleError(status);
      status = nc_inq_dimlen(ncid, nele_id, &nCellsPerLayer);
      if (status != NC_NOERR) fileObject-> HandleError(status);

      //SIGLAY
      size_t nSigLayers;
      int siglay_id;
      status = nc_inq_dimid(ncid, "siglay", &siglay_id);
      if (status != NC_NOERR) fileObject-> HandleError(status);
      status = nc_inq_dimlen(ncid, siglay_id, &nSigLayers);
      if (status != NC_NOERR) fileObject-> HandleError(status);
    
      int nzones=nSigLayers*nCellsPerLayer; // there are nSigLayers
      // in the levels mesh. There are nSigLayers-1 in the Sigma_Layers
      // mesh because its nodes are at the edge centers of the original 
      // mesh.
 
      // Create matnos and names arrays so we can create an avtMaterial.
      int *matnos = new int[nSigLayers];
      char **names = new char *[nSigLayers];
      for(int i = 0; i < nSigLayers; ++i)
        {
          matnos[i] = i + 1;
          char buffer [50];
          int n;
          n=sprintf(buffer, "Layer %d",i);
          names[i] = (char *)buffer;
          debug4 <<"i="<< i << ", matnos=" << 
            matnos[i] << ", names=" << names[i] << endl;

        }
    

      int *matlist =new int[nzones];
      int mod;
      int this_mat=0;
      for (int i =0; i< nzones; ++i)
        {
          mod = i % nCellsPerLayer;
          if(mod==0) matlist[i]=++this_mat;
       
          else matlist[i]=this_mat;

          debug4 << "matlist=" << matlist[i] << endl;
        }
    
    
      // Create the avtMaterial.
      int dims[3]= {1,1,1};
      dims[0] = nzones;
      int ndims=3;
      avtMaterial *mat= new avtMaterial(
                                        nSigLayers,
                                        matnos,
                                        names,
                                        ndims,
                                        dims,
                                        0,
                                        matlist,
                                        0, // length of mix arrays
                                        0, // mix_mat array
                                        0, // mix_next array
                                        0, // mix_zone array
                                        0  // mix_vf array
                                        );
    
      delete [] matlist;
      delete [] matnos;
      //    for (int i = 0; i < nSigLayers; ++i)
      //  delete [] names[i];
      delete [] names;
      retval =(void *)mat;
      df = avtMaterial::Destruct;
    }
    

#endif
  debug4 << mName << "END" << endl;
    
  return retval;
}




// ****************************************************************************
//  Method: avtFVCOMReader::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      VarName    The name of the variable requested.
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************

vtkDataArray *
avtFVCOMReader::GetVar(int timestate, const char *Variable, avtVariableCache *cache)
{
  const char *mName = "avtFVCOMReader::GetVar: ";
  debug4 << mName << "timestate=" << timestate
         << ", Variable=" << Variable << endl;

  if (NeedDimensions) GetDimensions();
  if(NeedGridVariables) GetStaticGridVariables();




  debug4 << "All dims=[" ; 
  for(int i = 0; i < nDims; ++i)
    {
      debug4<< dimSizes[i] << " ";
    }
  debug4<< "]" << endl;


  debug4 << mName << "Looking to see if Variable is in cache" << endl;
  vtkObject *obj =0;
  int domain=CacheDomainIndex;
  // for now: Set domain in MTMD!!!
 
  const char *matname = "all";
  obj = cache->GetVTKObject(Variable, avtVariableCache::DATASET_NAME,
                            timestate, domain, matname);
  if(obj != 0)
    {
      debug4 << mName << "Getting variable: " << Variable << " from Cache" << endl;
      vtkDataArray *ds = (vtkDataArray *)obj;
      ds->Register(NULL);
      return ds;
    }
  else
    {
      debug4 << mName << "Variable Not in Cache. Load from data." << endl;
    }


  // Special variables created in Visit for FVCOM
  if (strcmp("Dens3{S,Theta,P}", Variable)==0)
    {
      debug4 << mName << "Variable is Dens3"<< endl;
      vtkDataArray *rv =  DENS3(timestate, cache);
      return rv;
    }  
  else if (strcmp("Dens{S,T,P}", Variable)==0)
    {
      debug4 << mName << "Variable is Dens"<< endl;
      vtkDataArray *rv =  DENS(timestate, cache);
      return rv;
    }  
  else if (strcmp("Theta{S,T,P,0}", Variable)==0)
    {
      debug4 << mName << "Variable is Theta"<< endl;
      vtkDataArray *rv =  THETA(timestate, cache);
      return rv;
    }  

  //    else if (strcmp("Dens2(S,Theta,0)", VarName)==0)
  //    {
  //        debug4 << mName << "Variable is Dens2"<< endl;
  //    vtkDataArray *rv =  DENS2(timestate, cache);
  //    return rv;
  //    }  


    
  // Variables saved in NETCDF OUT
  // Bail out if we can't get the file handle.
  int ncid = fileObject->GetFileHandle();
  if(ncid == -1)
    {
      EXCEPTION1(InvalidVariableException, Variable);
    }

    
  // Make return variable
  vtkFloatArray *rv = vtkFloatArray::New();


  // Get the threshold variables for ploting single layers
  if (strcmp("Select Layer", Variable)==0)
    {
      debug4 << mName << "Layer_Nodes"<< endl;
      int ntuples = nNode*nSiglay;  
    
      // Put the data into the vtkFloatArray   
      rv->SetNumberOfTuples(ntuples);
    
      int count =0;
      float val=0.9999999; // the first level should be less than one
      for (int i = 0 ; i < nSiglay ; i++)
        {
          for (int j = 0; j <nNode; j++) 
            {
              rv->SetTuple1(count, val);
              count++;
            }
          val=float(i+2); // add two because i starts at zero
        }
      return rv;
    }  
  else if (strcmp("Select Level", Variable)==0)
    {
      debug4 << mName << "Level_Nodes"<< endl;

      int ntuples = nNode*nSiglev;  
    
      // Put the data into the vtkFloatArray   
      rv->SetNumberOfTuples(ntuples);
    
      int count =0;
      float val=0.9999999; // The first layer should be less than one
      for (int i = 0 ; i < nSiglev ; i++)
        {
          for (int j = 0; j <nNode; j++) 
            {
              rv->SetTuple1(count, val);
              count++;
            }
          val=float(i+2); // add two because i starts at zero
        }


      return rv;
    }  


  nc_type VarType;

  status=nc_inq_varid(ncid,Variable,&VarID);

  status = nc_inq_var(ncid, VarID, VarName, &VarType, &VarnDims, 
                          VarDimIDs, &VarnAtts);


  debug4 << mName << ", VarName=" << VarName << endl
         << "; vartype=" << VarType << endl
         << "; varndims="<< VarnDims<< endl
         << "; Vardims: [" ;
  for(int i=0; i<VarnDims; ++i)
    {
      debug4 << dimSizes[VarDimIDs[i]] << ",";
    }

  debug4 << "]" << endl;


  // declare variables for vtk      
  int ntuples=0;
  if(VarnDims == 1)
    {
      debug4 << "Variable: " << VarName << "is 1d" << endl;
    
      if (VarType == NC_FLOAT)
        {
          ntuples = dimSizes[VarDimIDs[0]];  
          float *vals = new float[ntuples];
          fileObject->ReadVariableInto(VarName, FLOATARRAY_TYPE, vals);    

          // Put the data into the vtkFloatArray   
          rv->SetNumberOfTuples(ntuples);
          for (int i = 0 ; i < ntuples ; i++)
            {
              rv->SetTuple1(i, vals[i]);
            }
          delete [] vals;
        }
      else if ( VarType== NC_INT)
        {
          ntuples = dimSizes[VarDimIDs[0]];
          int *vals = new int[ntuples];
          fileObject->ReadVariableInto(VarName, INTEGERARRAY_TYPE, vals); 
        
          // Put the data into the vtkFloatArray   
          rv->SetNumberOfTuples(ntuples);
          for (int i = 0 ; i < ntuples ; i++)
            {
              rv->SetTuple1(i, vals[i]);
            }
          delete [] vals;
        }
    }
  else if(VarnDims == 2)    
    {
      debug4 << "Variable: " << VarName << "is 2d" << endl;

      if (VarType == NC_FLOAT &&
          VarDimIDs[0] == nTimeID)
        {
          ntuples = dimSizes[VarDimIDs[1]];    
          size_t starts[]={timestate,0};
          size_t counts[]={1, ntuples};
          ptrdiff_t stride[]={1,1};
          float *vals = new float[ntuples];
          int var_id;

          status = nc_get_vars_float(ncid,VarID,
                                     starts, counts, stride, vals);
          if (status != NC_NOERR) fileObject-> HandleError(status);

          // Put the data into the vtkFloatArray   
          rv->SetNumberOfTuples(ntuples);
          for (int i = 0 ; i < ntuples ; i++)
            {
              rv->SetTuple1(i, vals[i]);
            }
          delete [] vals;
        }

    }
  else if (VarnDims == 3)
    {
      debug4 << "Variable: " << VarName << "is 3d" << endl;

      if (VarType == NC_FLOAT &&
          VarDimIDs[0] == nTimeID)
        {
          ntuples = dimSizes[VarDimIDs[1]]*dimSizes[VarDimIDs[2]];    
          size_t starts[]={timestate,0,0};
          size_t counts[]={1, dimSizes[VarDimIDs[1]], dimSizes[VarDimIDs[2]]};
          ptrdiff_t stride[]={1,1,1};
          float *vals = new float[ntuples];
          int var_id;
    
          status = nc_get_vars_float(ncid,VarID,
                                     starts, counts, stride, vals);
          if (status != NC_NOERR) fileObject-> HandleError(status);

          // Put the data into the vtkFloatArray   
          rv->SetNumberOfTuples(ntuples);
          for (int i = 0 ; i < ntuples ; i++)
            {
              rv->SetTuple1(i, vals[i]);

            }

          delete [] vals;
        }
    }
  else
    {
      EXCEPTION1(InvalidVariableException, VarName);
    }


  debug4 << mName << "Add Variable to cache!" << endl;
  cache->CacheVTKObject(VarName, avtVariableCache::DATASET_NAME, timestate, domain,
                        matname, rv);


    
  debug4 << mName << "end" << endl;

  return rv;
}


// ****************************************************************************
//  Method: avtFVCOMReader::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      var        The name of the variable requested.
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************



vtkDataArray *
avtFVCOMReader::GetVectorVar(int timestate, const char *var, avtVariableCache *cache)
{
  const char *mName = "avtFVCOMReader::GetVectorVar: ";
  debug4 << mName << "timestate=" << timestate
         << ", VarName=" << var << endl;

  if (NeedDimensions) GetDimensions();
  if(NeedGridVariables) GetStaticGridVariables();

  // For Arrays, var, should be the var name idenfied in the metadata



  // Bail out if we can't get the file handle.
  int ncid = fileObject->GetFileHandle();
  if(ncid == -1)
    {
      EXCEPTION1(InvalidVariableException, var);
    }


  // Create return variable
  vtkFloatArray *rv = vtkFloatArray::New();


  if (strcmp("2DVEL", var)==0) 
    {
      
      debug4 << mName << "Loading: u" << endl;
      vtkDataArray *uvel = GetVar(timestate,"u",cache);

      debug4 << mName << "Loading: v" << endl;
      vtkDataArray *vvel = GetVar(timestate,"v",cache);

      debug4 << "Setting ntuples" << endl;
      int ntuples = uvel->GetNumberOfTuples();
      debug4 << "ntuples=" << ntuples << endl;

      const int ncomps = 3;  // This is the rank of the vector
      rv->SetNumberOfComponents(ncomps);
      rv->SetNumberOfTuples(ntuples);
      float one_entry[ncomps];
      for (int i = 0 ; i < ntuples ; i++)
        {
          one_entry[0]=uvel->GetComponent(i,0);
          one_entry[1]=vvel->GetComponent(i,0);
          one_entry[2]=0;
          rv->SetTuple(i, one_entry); 
        }
      debug4<< mName << "Returned Velocity data" << endl;


    }
  else if (strcmp("3DVEL",var)==0)
    {

      debug4 << mName << "Loading: u" << endl;
      vtkDataArray *uvel = GetVar(timestate,"u",cache);

      debug4 << mName << "Loading: v" << endl;
      vtkDataArray *vvel = GetVar(timestate,"v",cache);

      debug4 << mName << "Loading: ww" << endl;
      vtkDataArray *wwvel = GetVar(timestate,"ww",cache);

      debug4 << "Setting ntuples" << endl;
      int ntuples = uvel->GetNumberOfTuples();



      debug4 << "ntuples=" << ntuples << endl;

      const int ncomps = 3;  // This is the rank of the vector
      rv->SetNumberOfComponents(ncomps);
      rv->SetNumberOfTuples(ntuples);
      float one_entry[ncomps];
      for (int i = 0 ; i < ntuples ; i++)
        {
          one_entry[0]=uvel->GetComponent(i,0);
          one_entry[1]=vvel->GetComponent(i,0);
          one_entry[2]=wwvel->GetComponent(i,0);
          rv->SetTuple(i, one_entry); 
        }

      debug4<< mName << "Returned Velocity data" << endl;


       
    }
  else if ((status = nc_inq_varid(ncid, var, &VarID)) == NC_NOERR)
    {
      // Variable Appears to be an Array!
      debug4 << "Variable:" << var << ": Appears to be an array" << endl;
      TypeEnum t = NO_TYPE;
      int varndims = 0, *vardims;
      fileObject->InqVariable(var, &t, &varndims, &vardims);
      if  (status != NC_NOERR) fileObject-> HandleError(status);

      if (varndims==2)
        {
          int nvars = vardims[0]; // The number of columns in the array   
          int nvals = vardims[1]; // The number of rows in the array
          // nvals is also the number of elements for array variables!

          debug4<< "nvars=" << nvars << endl;
          debug4 << "nvals=" << nvals << endl;
          float *vals = new float[nvals*nvars];
          status = nc_get_var_float(ncid,VarID,vals);
          if  (status != NC_NOERR) fileObject-> HandleError(status);

          rv->SetNumberOfComponents(nvars);
          rv->SetNumberOfTuples(nvals);
          for (int i = 0 ; i < nvals ; i++)
            for (int j = 0 ; j < nvars ; j++)
              rv->SetComponent(i, j, vals[nvars*i+j]);

          delete [] vals;

          debug4<< mName << "Returned Array data" << endl;
        }

      //else if (varndims==3)
      //{
      //int nvars = vardims[1]; //Number of layers or levels 
      //int nvals = vardims[2]; //Number of zones or nodes    
      //size_t starts[]={timestate,0,0};
      //size_t counts[]={1, vardims[1], vardims[2]};
      //ptrdiff_t stride[]={1,1,1};
      //float *vals = new float[nvars*nvals];
      //
      //status = nc_get_vars_float(fileObject->GetFileHandle(),var_id,
      //      starts, counts, stride, vals);
      //  if (status != NC_NOERR) fileObject-> HandleError(status);
      //
      //
      //  rv->SetNumberOfComponents(nvars);
      //  rv->SetNumberOfTuples(nvals);
      //  for (int i = 0 ; i < nvals ; i++)
      //      for (int j = 0 ; j < nvars ; j++)
      //  rv->SetComponent(i, j, vals[nvars*i+j]);
      //
      //  delete [] vals;
      //
      //  debug4<< mName << "Returned Array data" << endl;
      //}
      delete [] vardims;

    }
  else 
    {
      fileObject-> HandleError(status);
      EXCEPTION1(InvalidVariableException, var);
    }

  return rv;    
}





// ****************************************************************************
//  Method: avtFVCOMReader::DENS
//
//  Purpose:
//          Calculate the density from the temperature, salinity and pressure 
//               :use Fofonoff and Millard equation of state
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//
//
//  Modified:  fixed bug in call to getComponent: caused bad values at 
//             domain boundaries 
//             getComponent(i,0)  // must be zero not one!
//             9/13/2006
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************


//==============================================================================|
//   Calculate Potential Density Based on Potential Temp and Salinity           |
//     Pressure effects are incorported (Can Model Fresh Water < 4 Deg C)       |
//     Ref:  algorithms for computation of fundamental properties of            |
//         seawater , Fofonoff and Millard.                        |
//                                                                              |
//  calculates: rho1(nnode) density at nodes                        |
//  calculates: rho (ncell) density at elements                        |
//==============================================================================|
   
vtkDataArray *
avtFVCOMReader::DENS(int timestate, avtVariableCache *cache)
{
  const char *mName = "avtFVCOMReader::DENS: ";
  debug4 << mName << "timestate=" << timestate << endl;


  // Bail out if we can't get the file handle.
  int ncid = fileObject->GetFileHandle();
  if(ncid == -1)
    {
      EXCEPTION1(InvalidVariableException, "Density");
    }

  debug4 << mName << "Loading: salinity" << endl;
  vtkDataArray *salt = GetVar(timestate,"salinity",cache);

  debug4 << mName << "Loading: Theta{S,T,P,0}" << endl;
  vtkDataArray *ptmp = GetVar(timestate,"Theta{S,T,P,0}",cache);

  debug4 << mName << "Loading: SigmaLayer_Mesh" << endl;
  vtkDataSet *mesh =  GetMesh(timestate,"SigmaLayer_Mesh", cache);
    
  int ntuples = mesh->GetNumberOfPoints();
       
  debug4 << "ntuples= "<< ntuples << endl;
    
  vtkFloatArray *rv = vtkFloatArray::New();
  rv->SetNumberOfTuples(ntuples);
    
  double GRAV=9.81;
  double xyz[3];
  double sval;
  double tval;
  double pbar;
  // ADDED CHECK VAL TO DEBUG!!! when i=ntuples do check val!
  for (int i = 0 ; i < ntuples+1 ; i++) 
    {
    
      if (i<ntuples)
        {

          tval= ptmp->GetComponent(i,0); // must be (i,0) not (i,1)

          sval= salt->GetComponent(i,0);

          

          mesh->GetPoint(i,xyz); 
          // This returns xyz location of each point
          pbar = GRAV *1.025 * fabs(xyz[2]) * 0.01; 

     
          // Calculate pressure in Bar at depth z in meters
          //dbar = 9.81 *ave_dens * depth_m /100
        }
      else if (i==ntuples)
        {
          //  Check Values go here!
          tval=40;
          sval=40;
          pbar = 1000; 
        }      

      // Compute density (kg/m3) at standard one atmosphere pressure

    
      // Loaded Theta above! Converted from temp:
      double rho = SVAN(sval,tval,pbar);

      if (i<ntuples)
        rv->SetTuple1(i, rho);

      else
        debug4 << "Dens_Fofonoff_Millard CHECK VAL!" << endl
               << "check Values: (T=40 C, S=40 PSU, 1000 bar)" << endl
               << "RHO1 = 59.82037     (kg/m3)" << endl
               << "VALUE=" << rho << endl;

    }


  debug4 << mName << "end" << endl;

  return rv;
} // END DENS


  // ****************************************************************************
  //  Method: avtFVCOMReader::DENS2
  //
  //  Purpose:
  //          Calculate the density from the potential temperature and salinity 
  //               :use Unknown equation of state equation of state
  //
  //  Arguments:
  //      timestate  The index of the timestate.  If GetNTimesteps returned
  //                 'N' time steps, this is guaranteed to be between 0 and N-1.
  //
  //  Modified:  fixed bug in call to getComponent: caused bad values at 
  //             domain boundaries 
  //             getComponent(i,0)  // must be zero not one!
  //             9/13/2006
  //
  //  Programmer: David Stuebe
  //  Creation:   Thu May 18 08:39:01 PDT 2006
  //
  // ****************************************************************************

   
  // REMOVED DENS 2

#ifdef DENS2_unknown


vtkDataArray *
avtFVCOMReader::DENS2(int timestate, avtVariableCache *cache)
{
  const char *mName = "avtFVCOMReader::DENS2: ";
  debug4 << mName << "timestate=" << timestate << endl;


  // Bail out if we can't get the file handle.
  int ncid = fileObject->GetFileHandle();
  if(ncid == -1)
    {
      EXCEPTION1(InvalidVariableException, "Density");
    }

  debug4 << mName << "Loading: salinity" << endl;
  vtkDataArray *salt = GetVar(timestate,"salinity",cache);

  debug4 << mName << "Loading Potential Temperature variable: temp;" << endl;
  vtkDataArray *ptmp = GetVar(timestate,"temp",cache);

  debug4 << mName << "Loading: SigmaLayer_Mesh" << endl;
  vtkDataSet *mesh =  GetMesh(timestate,"SigmaLayer_Mesh", cache);
    
  int ntuples = mesh->GetNumberOfPoints();
       
    
  vtkFloatArray *rv = vtkFloatArray::New();
  rv->SetNumberOfTuples(ntuples);
    
  double sval;
  double tval;
  double rho;
  double sval2;
  double sval3;
  double tval2;
  double tval3;

  //Constant coefficients of equation of state
  double a1 = +6.76786136E-6;
  double a2 = -4.8249614E-4;
  double a3 = +8.14876577E-1;
  double a4 = -0.225845860E0;

  double b1 = +1.667E-8;
  double b2 = -8.164E-7;
  double b3 = +1.803E-5;

  double c1 = -1.0843E-6;
  double c2 = +9.8185E-5;
  double c3 = -4.786E-3;

  double d1 = +6.7678613E-6;
  double d2 = -4.8249614E-4;
  double d3 = 8.14876577E-1;
  double d4 = 3.895414E-2;

  double e1 = -3.98;

  double f1 = 283;

  double g1 = +503.57;
  double g2 = +67.26;



  // ADDED CHECK VAL TO DEBUG!!! when i=ntuples do check val!
  for (int i = 0 ; i < ntuples+1 ; i++) 
    {
    
      if (i<ntuples)
        {
          tval= ptmp->GetComponent(i,0); // must be (i,0) not (i,1)
          sval= salt->GetComponent(i,0);

        }
      else if (i==ntuples)
        {
          //  Check Values go here!
          tval=40;
          sval=40;
        }      

      // Compute density (kg/m3)

      sval2= sval * sval;
      sval3 =  sval2 * sval;

      tval2 = tval *tval;
      tval3 =  tval2 * tval;

      rho = ( ( a1 * sval3 + a2 * sval2 + a3 * sval + a4 ) *
              ( b1 * tval3 + b2 * tval2 + b3 * tval ) + 
              c1 * tval3 + c2 * tval2 + c3 * tval + 1 ) *
        ( d1 +sval3 + d2 * sval2 + d3 * sval +d4 ) -
        (tval + e1) * (tval + e1) * (tval + f1 ) / ( g1 * tval + g2);



      if (i<ntuples)
        rv->SetTuple1(i, rho);

      else
        debug4 << "Don't know where this equation of state came from?" << endl
               << "check Values: ?)" << endl;
      // << "RHO1 = 59.82037     (kg/m3)" << endl
      //   << "VALUE=" << rho << endl;

    }


  debug4 << mName << "end" << endl;

  return rv;
} // END DENS2


#endif

// ****************************************************************************
//  Method: avtFVCOMReader::DENS3
//
//  Purpose:
//          Calculate the density from the temperature, salinity and pressure 
//               :use Jackett and McDougall equation of state
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//
//
//  Modified:  fixed bug in call to getComponent: caused bad values at 
//             domain boundaries 
//             getComponent(i,0)  // must be zero not one!
//             9/13/2006
//
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************

//==============================================================================|
//     COMPUTE IN SITU DENSITY - 1000  USING SALINITY, POTENTIAL TEMP,          |
//     AND PRESSURE FROM A POLYNOMIAL EXPRESSION (JACKETT & MCDOUGALL,          |
//     1995). IT ASSUMES  NO  PRESSURE  VARIATION  ALONG GEOPOTENTIAL           |
//     SURFACES, THAT IS, DEPTH (METERS; NEGATIVE) AND PRESSURE (DBAR           |
//     ASSUMED NEGATIVE HERE) ARE INTERCHANGEABLE.                              |
//                                                                              |
//     check Values: (T=3 C, S=35.5 PSU, Z=-5000 m)                             |
//        RHOF  = 1050.3639165364     (kg/m3)                                   |
//        DEN1  = 1028.2845117925     (kg/m3)                                   |
//                                                                              |
//  Reference:                                                                  |
//                                                                              |
//  Jackett, D. R. and T. J. McDougall, 1995, Minimal Adjustment of             |
//    Hydrostatic Profiles to Achieve Static Stability, J. of Atmos.            |
//    and Oceanic Techn., vol. 12, pp. 381-389.                                 |
//                                            | 
//    CALCULATES: RHO1(M) DENSITY AT NODES                                |
//    CALCULATES: RHO (N) DENSITY AT ELEMENTS                        |
//==============================================================================|



vtkDataArray *
avtFVCOMReader::DENS3(int timestate, avtVariableCache *cache)
{
  const char *mName = "avtFVCOMReader::DENS3: ";
  debug4 << mName << "timestate=" << timestate << endl;


  // Bail out if we can't get the file handle.
  int ncid = fileObject->GetFileHandle();
  if(ncid == -1)
    {
      EXCEPTION1(InvalidVariableException, "Density");
    }

  debug4 << mName << "Loading: salinity" << endl;
  vtkDataArray *salt = GetVar(timestate,"salinity",cache);

  debug4 << mName << "Loading Potential Temperature variable: temp;" << endl;
  vtkDataArray *ptmp = GetVar(timestate,"temp",cache);

  debug4 << mName << "Loading: SigmaLayer_Mesh" << endl;
  vtkDataSet *mesh =  GetMesh(timestate,"SigmaLayer_Mesh", cache);
    
  int ntuples = mesh->GetNumberOfPoints();
    
  vtkFloatArray *rv = vtkFloatArray::New();
  rv->SetNumberOfTuples(ntuples);
    
  double TEMP[10]; // This will be temporary storage for intermediate steps!
  double GRAV=9.81;
  double xyz[3];
  float tval;
  float sval;
  float sqrtsval;
  double PBAR;
    


    
  //==============================================================================|
  //  Polynomial  expansion  coefficients for the computation of in situ          |
  //  density  via  the  nonlinear  equation of state  for seawater as a          |
  //  function of potential temperature, salinity, and pressure (Jackett          |
  //  and McDougall, 1995).                                                       |
        
  double A00 = +1.965933e+04;
  double A01 = +1.444304e+02;
  double A02 = -1.706103e+00;
  double A03 = +9.648704e-03;
  double A04 = -4.190253e-05;
  double B00 = +5.284855e+01;
  double B01 = -3.101089e-01;
  double B02 = +6.283263e-03;
  double B03 = -5.084188e-05;
  double D00 = +3.886640e-01;
  double D01 = +9.085835e-03;
  double D02 = -4.619924e-04;
  double E00 = +3.186519e+00;
  double E01 = +2.212276e-02;
  double E02 = -2.984642e-04;
  double E03 = +1.956415e-06;
  double F00 = +6.704388e-03;
  double F01 = -1.847318e-04;
  double F02 = +2.059331e-07;
  double G00 = +1.480266e-04;
  double G01 = +2.102898e-04;
  double G02 = -1.202016e-05;
  double G03 = +1.394680e-07;
  double H00 = -2.040237e-06;
  double H01 = +6.128773e-08;
  double H02 = +6.207323e-10;
    
  double Q00 = +9.99842594e+02;
  double Q01 = +6.793952e-02;
  double Q02 = -9.095290e-03;
  double Q03 = +1.001685e-04;
  double Q04 = -1.120083e-06;
  double Q05 = +6.536332e-09;
  double U00 = +8.24493e-01;
  double U01 = -4.08990e-03;
  double U02 = +7.64380e-05;
  double U03 = -8.24670e-07;
  double U04 = +5.38750e-09;
  double V00 = -5.72466e-03;
  double V01 = +1.02270e-04;
  double V02 = -1.65460e-06;
  double W00 = +4.8314e-04;
    

  // ADDED CHECK VAL TO DEBUG!!! when i=ntuples do check val!
  for (int i = 0 ; i < ntuples+1 ; i++) 
    {
    
      if (i<ntuples)
        {
          tval= ptmp->GetComponent(i,0); // must be (i,0) not (i,1)
          sval= salt->GetComponent(i,0);
          sqrtsval = sqrt(sval);
          mesh->GetPoint(i,xyz); 
          // This returns xyz location of each point
          PBAR = GRAV *1.025 * fabs(xyz[2]) * 0.01; 
          // Calculate pressure in Bar at depth z in meters
          //Bar = 9.81 *ave_dens * depth_m /100
        }
      else if (i==ntuples)
        {
          tval=3;
          sval=35.5;
          sqrtsval = sqrt(sval);
          PBAR = 300; 
        }      

      // Compute density (kg/m3) at standard one atmosphere pressure
    
      TEMP[1]=Q00+tval*(Q01+tval*(Q02+tval*(Q03+tval*(Q04+tval*Q05))));
      TEMP[2]=U00+tval*(U01+tval*(U02+tval*(U03+tval*U04)));
      TEMP[3]=V00+tval*(V01+tval*V02);
      double DEN1=TEMP[1]+sval*(TEMP[2]+sqrtsval*TEMP[3]+sval*W00);

      // Compute secant bulk modulus (BULK = BULK0 + BULK1*PBAR + BULK2*PBAR*PBAR)
    
      TEMP[4]=A00+tval*(A01+tval*(A02+tval*(A03+tval*A04)));
      TEMP[5]=B00+tval*(B01+tval*(B02+tval*B03));
      TEMP[6]=D00+tval*(D01+tval*D02);
      TEMP[7]=E00+tval*(E01+tval*(E02+tval*E03));
      TEMP[8]=F00+tval*(F01+tval*F02);
      TEMP[9]=G01+tval*(G02+tval*G03);
      TEMP[10]=H00+tval*(H01+tval*H02);
    
      double BULK0=TEMP[4]+sval*(TEMP[5]+sqrtsval*TEMP[6]);
      double BULK1=TEMP[7]+sval*(TEMP[8]+sqrtsval*G00);
      double BULK2=TEMP[9]+sval*TEMP[10];
      double BULK = BULK0 + PBAR * (BULK1 + PBAR * BULK2);
    
      //  Compute "in situ" density anomaly (kg/m3)
      double RHOF=(DEN1*BULK)/(BULK-PBAR);
      if (i<ntuples)
        rv->SetTuple1(i, RHOF);
    
      else
        debug4 << "Dens_Jackett_McDougall CHECK VAL!" << endl
               << "check Values: (T=3 C, S=35.5 PSU, 300 bar)" << endl
               << "RHOF = 1041.83267     (kg/m3)" << endl
               << "VALUE=" << RHOF << endl;
              
    }


  debug4 << mName << "end" << endl;

  return rv;
}// end DENS3


// ****************************************************************************
//  Method: avtFVCOMReader::THETA
//
//  Purpose: Calculate potential temperature for insitu temperature, salinity, 
//           Pressure, and a reference pressure.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//
//
//  Modified:  fixed bug in call to getComponent: caused bad values at 
//             domain boundaries 
//             getComponent(i,0)  // must be zero not one!
//             9/13/2006
//
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************


//==============================================================================|
// to compute local potential temperature at pr using                           |
// bryden 1973 polynomial for adiabatic lapse rate and                          |
// runge-kutta 4th order integration algorithm.                                 |
// ref: bryden,h.,1973,deep-sea res.,20,401-408;                                |
// fofonoff,n.,1977,deep-sea res.,24,489-491                                    |
//                                                                              |
// units:                                                                       |
//       pressure        p04       decibars                                     |
//       temperature     t04       deg celsius (ipts-68)                        |
//       salinity         s4        (ipss-78)                                   |
//       reference prs    pr       decibars                                     |
//       potential tmp.  theta     deg celsius                                  |
// checkvalue:                                                                  |
//             theta= 36.89073 c,s=40 (ipss-78),                                |
//             t0=40 deg c,p0=10000 decibars,pr=0 decibars                      |
//                                                                              |
//==============================================================================|


vtkDataArray *
avtFVCOMReader::THETA(int timestate, avtVariableCache *cache)
{
  const char *mName = "avtFVCOMReader::THETA: ";
  debug4 << mName << "timestate=" << timestate << endl;


  // Bail out if we can't get the file handle.
  int ncid = fileObject->GetFileHandle();
  if(ncid == -1)
    {
      EXCEPTION1(InvalidVariableException, "Density");
    }

  debug4 << mName << "Loading: salinity" << endl;
  vtkDataArray *salt = GetVar(timestate,"salinity",cache);

  debug4 << mName << "Loading insitu temperature variable: temp;" << endl;
  vtkDataArray *tmp = GetVar(timestate,"temp",cache);

  debug4 << mName << "Loading: SigmaLayer_Mesh" << endl;
  vtkDataSet *mesh =  GetMesh(timestate,"SigmaLayer_Mesh", cache);
    
  int ntuples = mesh->GetNumberOfPoints();
    
  vtkFloatArray *rv = vtkFloatArray::New();
  rv->SetNumberOfTuples(ntuples);
  double GRAV = 9.81;   
  double xyz[3];
  float tval;
  float sval;
  double pbar;
    
  // Set Reference pressure here!
  double p_ref=0.0;
  // if you change this please change the name of the variable in the meta data
  // as well for clarity!!!!


  // ADDED CHECK VAL TO DEBUG!!! when i=ntuples do check val!
  for (int i = 0 ; i < ntuples+1 ; i++) 
    {
        
      if (i<ntuples)
        {
          tval= tmp->GetComponent(i,0); // must be (i,0) not (i,1)
          sval= salt->GetComponent(i,0);
          mesh->GetPoint(i,xyz); 
          // This returns xyz location of each point
          pbar = GRAV *1.025 * fabs(xyz[2]) * 0.01; 
          // Calculate pressure in Bar at depth z in meters
          //Bar = 9.81 *ave_dens * depth_m /100

        }
      else if (i==ntuples)
        {
          tval=40;
          sval=40;
          pbar = 1000; 
        }      
        
    
    
    
    
      //==============================================================================|

      double S4 = sval;
      double P4 = pbar*10.0; // pressure in decibar!
      double T4 = tval;
      double PR = p_ref;
      double H4;
      double XK;
      double Q4;
      H4 = PR - P4;
      XK = H4 * ATG(S4,T4,P4);
      T4 = T4 + 0.5*XK;
      Q4 = XK;
      P4 = P4 + 0.5*H4;
      XK = H4 * ATG(S4,T4,P4);
      T4 = T4 + 0.29289322*(XK-Q4);
      Q4 = 0.58578644*XK + 0.121320344*Q4;
      XK = H4 * ATG(S4,T4,P4);
      T4 = T4 + 1.707106781*(XK-Q4);
      Q4 = 3.414213562*XK - 4.121320344*Q4;
      P4 = P4 + 0.5*H4;
      XK = H4 * ATG(S4,T4,P4);
      double ptmp = T4 + (XK-2.0*Q4)/6.0;
    
    
      if (i<ntuples)
        {
          rv->SetTuple1(i, ptmp);
          debug4 << "================" << endl;
          debug4 << "tval: " << tval <<endl;
          debug4 << "sval: " << sval << endl;
          debug4 << "z(meters): " << fabs(xyz[2]) <<endl;
          debug4 << "pbar: " << pbar << endl;
          debug4 << "pref: " << p_ref << endl;
          debug4 << "ptmp: " << ptmp << endl;
          debug4 << "================" << endl;
        }

      else
        debug4 << "THETA CHECK VAL!" << endl
               << "check Values: (T=40 C, S=40 (ipss-78), 1000 bar)" << endl
               << "RHOF = 36.89073   (c)" << endl
               << "VALUE=" << ptmp << endl;
              
    }


  debug4 << mName << "end" << endl;

  return rv;
}// end THETA




//==============================================================================|
// adiabatic temperature gradient deg c per decibar                        |
// ref: bryden, h., 1973,deep-sea res.,20,401-408                               |
//                                                                              |
// units:                                                                       |
//       pressure        P4        decibars                                     |
//       temperature     T4        deg celsius(ipts-68)                         |
//       salinity        s4        (ipss-78)                                    |
//       adiabatic      atg        deg. c/decibar                               |
// checkvalue: atg=3.255976e-4 c/dbar for s=40 (ipss-78),                       |
// t=40 deg c,p0=10000 decibars                                                 |
//==============================================================================|

double
avtFVCOMReader::ATG(double S4, double T4, double P4)
{
  
  double ds  = S4 - 35.0;
  double atg = (((-2.1687e-16*T4+1.8676e-14)*T4-4.6206e-13)*P4
                +((2.7759e-12*T4-1.1351e-10)*ds+((-5.4481e-14*T4
                                                  +8.733e-12)*T4-6.7795e-10)*T4+1.8741e-8))*P4
    +(-4.2393e-8*T4+1.8932e-6)*ds 
    +((6.6228e-10*T4-6.836e-8)*T4+8.5258e-6)*T4+3.5803e-5;

  return atg;
}



//==============================================================================!
// specific volume anomaly (steric anomaly) based on 1980 equation              |
// of state for seawater and 1978 practerical salinity scale.                   |
// references:                                                                  |
// millero, et al (1980) deep-sea res.,27a,255-264                              |
// millero and poisson 1981,deep-sea res.,28a pp 625-629.                       |
// both above references are also found in unesco report 38 (1981)              |
//                                                                              |
// units:                                                                       |
//       pressure        p04       decibars                                     |
//       temperature     t4        deg celsius (ipts-68)                        |
//       salinity        s4        (ipss-78)                                    |
//       spec. vol. ana. svan     m**3/kg *1.0e-8                               |
//       density ana.    sigma    kg/m**3                                       |
//                                                                              |
// check value: svan=981.3021 e-8 m**3/kg. for s = 40 (ipss-78),                |
// t = 40 deg c, p0= 10000 decibars.                                            |
// check value: sigma = 59.82037  kg/m**3. for s = 40 (ipss-78) ,               |
// t = 40 deg c, p0= 10000 decibars.                                            |
//==============================================================================!

double
avtFVCOMReader::SVAN(double S4, double T4, double P4)
{

  // Declare storage variable here!
  double SIG,SR,RR1,RR2,RR3,V350P,DK;
  double A4,B4,C4,D4,E4,AA1,BB1,AW,BW,KK,K0,KW,K35,SVA;
  double GAM,PK,DVAN,DR35P, SVAN;
    
  // Return variable:
  double SIGMA;    

  double R3500 = 1028.1063;
  double RR4   = 4.8314E-4;
  double DR350 = 28.106331;
    
  // rr4 is refered to as  c  in millero and poisson 1981
  // pressure is bars and take square root salinity.
    
  //   P4=P04/10.0_SP  Input pressure is bars not dbar!!!
  SR  = sqrt(fabs(S4));
    
  // pure water density at atmospheric pressure
  //   bigg p.h.,(1967) br. j. applied physics 8 pp 521-537.
  //
    
  RR1 = ((((6.536332E-9*T4 - 1.120083E-6)*T4 + 1.001685E-4)*T4
          -9.095290E-3)*T4 + 6.793952E-2)*T4 - 28.263737;


  // seawater density atm press.
  //  coefficients involving salinity
  //  rr2 = a   in notation of millero and poisson 1981

  RR2 = (((5.3875E-9*T4 - 8.2467E-7)*T4 + 7.6438E-5)*T4
         -4.0899E-3)*T4 + 8.24493E-1;

  //  rr3 = b4  in notation of millero and poisson 1981

  RR3 = (-1.6546E-6*T4 + 1.0227E-4)*T4 - 5.72466E-3;

  //  international one-atmosphere equation of state of seawater

  SIG = (RR4*S4 + RR3*SR + RR2)*S4 + RR1;

  // specific volume at atmospheric pressure

  V350P = 1.0/R3500;
  SVA   = -SIG*V350P/(R3500 + SIG);
  SIGMA = SIG + DR350;

  //  scale specific vol. anamoly to normally reported units

  SVAN=SVA*1.0E+8;
  if(P4 == 0.0) return SIGMA;
    
  //-------------------------------------------------------------|
  //    new high pressure equation of sate for seawater          |
  //                                                             |
  //        millero, el al., 1980 dsr 27a, pp 255-264            |
  //        constant notation follows article                    |
  //-------------------------------------------------------------|
  // compute compression terms

  E4  = (9.1697E-10*T4 + 2.0816E-8)*T4 - 9.9348E-7;
  BW  = (5.2787E-8*T4 - 6.12293E-6)*T4 + 3.47718E-5;
  B4  = BW + E4*S4;
     
  D4  = 1.91075E-4;
  C4  = (-1.6078E-6*T4 - 1.0981E-5)*T4 + 2.2838E-3;
  AW  = ((-5.77905E-7*T4 + 1.16092E-4)*T4 + 1.43713E-3)*T4
    -0.1194975;
  A4  = (D4*SR + C4)*S4 + AW;
     
  BB1 = (-5.3009E-4*T4 + 1.6483E-2)*T4 + 7.944E-2;
  AA1 = ((-6.1670E-5*T4 + 1.09987E-2)*T4 - 0.603459)*T4 + 54.6746;
  KW  = (((-5.155288E-5*T4 + 1.360477E-2)*T4 - 2.327105)*T4
         +148.4206)*T4 - 1930.06;
  K0  = (BB1*SR + AA1)*S4 + KW;
     
  // evaluate pressure polynomial
  //-----------------------------------------------------|
  //   k equals the secant bulk modulus of seawater      |
  //   dk=k(s,t,p)-k(35,0,p)                             |
  //   k35=k(35,0,p)                                     |
  //-----------------------------------------------------|
    
  DK  = (B4*P4 + A4)*P4 + K0;
  K35 = (5.03217E-5*P4 + 3.359406)*P4 + 21582.27;
  GAM = P4/K35;
  PK  = 1.0 - GAM;
  SVA = SVA*PK + (V350P + SVA)*P4*DK/(K35*(K35+DK));

  //  scale specific vol. anamoly to normally reported units

  SVAN  = SVA*1.0E+8;
  V350P = V350P*PK;

  //----------------------------------------------------------|
  // compute density anamoly with respect to 1000.0 kg/m**3   |
  //  1) dr350: density anamoly at 35 (ipss-78),              |
  //                               0 deg. c and 0 decibars    |
  //  2) dr35p: density anamoly at 35 (ipss-78),              |
  //                               0 deg. c, pres. variation  |
  //  3) dvan : density anamoly variations involving specific |
  //            volume anamoly                                |
  //                                                          |
  // check values: sigma = 59.82037 kg/m**3                   |
  // for s = 40 (ipss-78), t = 40 deg c, p0= 10000 decibars.  |
  //----------------------------------------------------------|

  DR35P = GAM/V350P;
  DVAN  = SVA/(V350P*(V350P + SVA));
  SIGMA = DR350 + DR35P - DVAN;


  return SIGMA;
}
