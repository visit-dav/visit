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
#include <avtMaterial.h>

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
//  Method: avtFVCOMReadeer::constructor
//
//  Programmer: David Stuebe
//  Creation:   Thu May 18 08:39:01 PDT 2006
//
// ****************************************************************************


avtFVCOMReader::avtFVCOMReader(const char *filename)
{
  fileObject = new NETCDFFileObject(filename);

  InitializeReader();

}

avtFVCOMReader::avtFVCOMReader(const char *filename, NETCDFFileObject *f)
{
  fileObject = f;

  InitializeReader();

}


// ****************************************************************************
// Method: avtFVCOMReader::InitializeReader
//
// Purpose: 
//   Explicitely initialize all variables used in the reader.
//
// Programmer: David Stuebe
// Creation:   Wed March 28 16:18:57 PST 2007
//
// Modifications:
//   
// ****************************************************************************
void
avtFVCOMReader::InitializeReader()
{
  debug4<< "avtFVCOMReader::InitializeReader;"<< endl;

  //Initialize dynamic memory variables
  xvals=0;
  yvals=0;
  zvals=0;
  SigLayers=0;
  SigLevels=0;
  latvals=0;
  lonvals=0;
  nvvals=0;
  egid=0;
  ngid=0;
  dimSizes=0;
  
  //Initialize boolen variables
  NeedDimensions=true;
  NeedGridVariables=true;
  IsGeoRef=false;
  xstate=false;
  ystate=false;
  hstate=false;
  zstate=false;
  latstate=false;
  lonstate=false;
  nodestate=false;
  elemstate=false;
  siglaystate=false;
  siglevstate=false;
  mesh1=false;
  mesh2=false;
  mesh3=false;
  mesh4=false;
  mesh5=false;
  
  
  
  //Initialize Integer variables
  status=0;
  ncid=0;
  nDims=0;
  nVars=0;
  nGlobalAtts=0;
  unlimitedDimension=0;
  nScalarID=0;
  nNodeID=0;
  nElemID=0;
  nSiglayID=0;
  nSiglevID=0;
  nThreeID=0;
  nFourID=0;
  nMaxnodeID=0;
  nMaxelemID=0;
  nTimeID=0;
  dimID=0;
  nScalar=0;
  nNode=0;
  nElem=0;
  nSiglay=0;
  nSiglev=0;
  nThree=0;
  nFour=0;
  nMaxnode=0;
  nMaxelem=0;
  nTime=0;
  VarnDims=0;
  VarnAtts=0;
  for(int i = 0; i < NC_MAX_VAR_DIMS; ++i)
    VarDimIDs[i]=0;
  VarID=0;
  
  //Initialize strings
  SigLayCoordType=' ';
  SigLevCoordType=' ';
  
  
  
  debug4<< "avtFVCOMReader::InitializeReader: end;"<< endl;
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

  if (fileObject!=0)  
    {
      debug4 << "fileObject: " << fileObject << endl;
      delete fileObject;
      fileObject=0;
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


  if (xvals != 0)
    {
      debug4 << "delete xvals:" << xvals << endl;
      delete [] xvals;
      xvals=0;
    }

  if (yvals != 0)
    {
      debug4 << "delete yvals:" << yvals << endl;
      delete [] yvals;
      yvals=0;
    }

  if (zvals != 0)
    {
      debug4 << "delete zvals:" << zvals << endl;
      delete [] zvals;
      zvals=0;
    }

  if (SigLayers != 0)
    {
      debug4 << "delete SigLayers:" << SigLayers << endl;
      delete [] SigLayers;
      SigLayers=0;
    }

  if (SigLevels != 0)
    {
      debug4 << "delete Siglevels:" << SigLevels << endl;
      delete [] SigLevels;
      SigLevels=0;
    }

  if(nvvals != 0)
    {
      debug4 << "delete nvvals:" << nvvals << endl;
      delete [] nvvals;
      nvvals=0;
    }

  if(egid != 0)
    {
      debug4 << "delete egid:" << egid << endl;
      delete [] egid;
      egid=0;
    }

  if(ngid != 0)
    {
      debug4 << "delete ngid:" << ngid << endl;
      delete [] ngid;
      ngid=0;
    }


  if (dimSizes != 0)
    {
      debug4 << "delete dimSizes:" << dimSizes << endl;
      delete [] dimSizes;
      dimSizes=0;
    }

   if (latvals != 0)
    {
      debug4 << "delete latvals:" << latvals << endl;
      delete [] latvals;
      latvals=0;
    }

 if (lonvals != 0)
    {
      debug4 << "delete lonvals:" << lonvals << endl;
      delete [] lonvals;
      lonvals=0;
    }



  debug4 << "done delete" << endl;
  NeedDimensions=true;
  NeedGridVariables=true;
  
  fileObject->Close();

  debug4 << "avtFVCOMReader::FreeUpResources: end" << endl;
  

}


// ****************************************************************************
//  Method: avtFVCOMReader::GetNTimesteps
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
// I have upgraded this method to do most of the logical checking about what 
// is in the file. That way it is not written twice, once in the metadata server
// and once for the engine.
//
// Could add test for the right dimensions on each variable?
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
  else
    {
      status = nc_inq_dimlen(ncid, nNodeID, &tNode);
      if (status != NC_NOERR) fileObject-> HandleError(status);
      nNode=tNode;
      nodestate=true;
    }
  
  // NELE
  status = nc_inq_dimid(ncid, "nele", &nElemID);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  else
    {
      status = nc_inq_dimlen(ncid, nElemID, &tElem);
      if (status != NC_NOERR) fileObject-> HandleError(status);
      nElem=tElem;
      elemstate=true;
    }

  if(!elemstate | !nodestate)
    {
      debug4<<"Must have both of the dimensions: 'node' and 'nele'" << endl;
      std::string msg("FVCOM file must have one of the dimensions: 'node' or 'nele'");      
      EXCEPTION1(ImproperUseException, msg);
    }
      
  //SIGLAY
  status = nc_inq_dimid(ncid, "siglay", &nSiglayID);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  else
    {
      status = nc_inq_dimlen(ncid, nSiglayID, &tSiglay);
      if (status != NC_NOERR) fileObject-> HandleError(status);
      nSiglay=tSiglay;
      siglaystate=true;
    }

  if(!siglaystate) debug4<<"NO SIGMA LAYERS"<< endl;

  //SIGLEV
  status = nc_inq_dimid(ncid, "siglev", &nSiglevID);
  if (status != NC_NOERR) fileObject->HandleError(status);
  else
    {
      status = nc_inq_dimlen(ncid, nSiglevID, &tSiglev);
      if (status != NC_NOERR) fileObject-> HandleError(status);
      nSiglev=tSiglev;
      siglevstate=true;
    }

  if(!siglevstate) debug4<<"NO SIGMA LEVELS"<< endl;


  //THREE
  status = nc_inq_dimid(ncid, "three", &nThreeID);
  if (status != NC_NOERR)
    {
      fileObject->HandleError(status);
      debug4<<"Must have the dimension 'three' for the connectivity" << endl;
      std::string msg("FVCOM file must have the dimension 'three' for the connectivity");
      EXCEPTION1(ImproperUseException, msg);
    }
  else
    {
      status = nc_inq_dimlen(ncid, nThreeID, &tThree);
      if (status != NC_NOERR) fileObject-> HandleError(status);
      nThree=tThree;
    }

  //FOUR
  status = nc_inq_dimid(ncid, "four", &nFourID);
  if (status != NC_NOERR) fileObject->HandleError(status);
  else
    {
      status = nc_inq_dimlen(ncid, nFourID, &tFour);
      if (status != NC_NOERR) fileObject-> HandleError(status);
      nFour=tFour;
    }

  //MAXNODE
  status = nc_inq_dimid(ncid, "maxnode", &nMaxnodeID);
  if (status != NC_NOERR) fileObject->HandleError(status);
  else
    {
      status = nc_inq_dimlen(ncid, nMaxnodeID, &tMaxnode);
      if (status != NC_NOERR) fileObject-> HandleError(status);
      nMaxnode=tMaxnode;
    }

  //MAXELEM
  status = nc_inq_dimid(ncid, "maxelem", &nMaxelemID);
  if (status != NC_NOERR) fileObject->HandleError(status);
  else
    {
      status = nc_inq_dimlen(ncid, nMaxelemID, &tMaxelem);
      if (status != NC_NOERR) fileObject-> HandleError(status);
      nMaxelem=tMaxelem;
    }

  //TIME
  status = nc_inq_dimid(ncid, "time", &nTimeID);
  if (status != NC_NOERR) 
    {
      debug4<<"FVCOM file does not have dimension time"<< endl;
      debug4<<"Assuming one timestep and continue"<< endl; 
      nTime=1;
      fileObject-> HandleError(status);
    }
  else
    {
      status = nc_inq_dimlen(ncid, nTimeID, &tTime);
      if (status != NC_NOERR) fileObject-> HandleError(status);
      nTime=tTime;
    }


  // Geo reference Coordinate system stuff
  IsGeoRef=false;
  std::string CoordSys;
  if(fileObject->ReadStringAttribute("CoordinateSystem", CoordSys))
    if (strcmp(CoordSys.c_str(),"GeoReferenced")==0) IsGeoRef=true;
  

  // Check and make sure we have lon and lat if data is supposed to be georef
  status = nc_inq_varid (ncid, "lon", &VarID);
  if (status != NC_NOERR) 
    {
      fileObject-> HandleError(status);
      if (IsGeoRef)
        {
          debug4 << "Critical Grid Coordinates could not be loaded" << endl;
          debug4<<"Spherical coordinates require: lon" << endl;
          std::string msg("Critical Grid Coordinates could not be loaded: lon");      
          EXCEPTION1(ImproperUseException, msg);
        }
    }
  else lonstate=true;

  status = nc_inq_varid (ncid, "lat", &VarID);
  if (status != NC_NOERR) 
    {
      fileObject-> HandleError(status);
      if (IsGeoRef)
        {
          debug4 << "Critical Grid Coordinates could not be loaded" << endl;
          debug4<<"Spherical coordinates require: lat" << endl;
          std::string msg("Critical Grid Coordinates could not be loaded: lat");      
          EXCEPTION1(ImproperUseException, msg);
        }
    }
  else latstate=true;

  

  // State variable used to check if variables which are 
  // neccissary for the grid are included in the file
  int siglevID;
  if (siglevstate)
    {
      status = nc_inq_varid (ncid, "siglev", &siglevID);
      if (status != NC_NOERR)
        {
          fileObject-> HandleError(status);
          siglevstate=false;
          debug4<< "found 'siglev' dimension but no variable" << endl;
        }
    }

  int siglayID;
  if (siglaystate)
    {
      status = nc_inq_varid (ncid, "siglay", &siglayID);
      if (status != NC_NOERR)
        {
          fileObject-> HandleError(status);
          siglaystate=false;
          debug4<< "found 'siglay' dimension but no variable" << endl;
        }
    }

  status = nc_inq_varid (ncid, "y", &VarID);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  else ystate=true;


  status = nc_inq_varid (ncid, "x", &VarID);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  else xstate=true;

  status = nc_inq_varid (ncid, "zeta", &VarID);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  else zstate=true; 

  status = nc_inq_varid (ncid, "h", &VarID);
  if (status != NC_NOERR) fileObject-> HandleError(status);
  else hstate=true;

  // if we have sigma coordinates but 
  // no surface elevation or depth throw an exception
  if(siglaystate | siglevstate)
    if ( !zstate | !hstate)
      {
        debug4<<"Sigma coordinate require surface elevation variable 'zeta'" << endl;
        std::string msg("Sigma coordinate require surface elevation variable 'zeta'");
        EXCEPTION1(ImproperUseException, msg);
      }
  
  int ndims;
  // SigLayCoordType is already declared in the header file
  if(siglaystate)
    if(!fileObject->ReadStringAttribute("siglay", "standard_name",SigLayCoordType)) 
      {
        status=nc_inq_varndims(ncid,siglayID,&ndims);
          if(ndims==1) SigLayCoordType="Ocean_Sigma_Coordinates";
          else if (ndims==2) SigLayCoordType="ocean_sigma/general_coordinate";
          else 
            {
              debug4<<"Sigma Layer variable has no standard name and bad dimensions" << endl;
              std::string msg("Sigma Layer variable has no standard name and bad dimensions");
              EXCEPTION1(ImproperUseException, msg);
              
            }
      }
  
  // SigLevCoordType is already declared in the header file
  if(siglevstate)
    if(!fileObject->ReadStringAttribute("siglev", "standard_name",SigLevCoordType))
      {
        status=nc_inq_varndims(ncid,siglevID,&ndims);
          if(ndims==1) SigLevCoordType="Ocean_Sigma_Coordinates";
          else if (ndims==2) SigLevCoordType="ocean_sigma/general_coordinate";
          else 
            {
              debug4<<"Sigma Layer variable has no standard name and bad dimensions" << endl;
              std::string msg("Sigma Layer variable has no standard name and bad dimensions");
              EXCEPTION1(ImproperUseException, msg);
            }
      }


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
  
  status = nc_inq_varid (fileObject->GetFileHandle(), "time", &VarID);
  if (status != NC_NOERR) 
    {
      debug4<< "No variable 'time', returning zero" << endl;
      fileObject-> HandleError(status);
      t.push_back(double(0));
      return;
    }


  
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

  status = nc_inq_varid (fileObject->GetFileHandle(), "iint", &VarID);
  if (status != NC_NOERR) 
    {
      debug4<< "No variable 'iint', returning one" << endl;
      fileObject-> HandleError(status);
      cyc.push_back(1);
      return;
    }


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
              
          fileObject->ReadVariableInto("iint",FLOATARRAY_TYPE,s1,c1,&cyc1);
              
          const int s2[1]={1};
          const int c2[1]={1};
              
          fileObject->ReadVariableInto("iint",FLOATARRAY_TYPE,s2,c2,&cyc2);
              
          const int send[1]={nTime-1};
          const int cend[1]={1};
              
          fileObject->ReadVariableInto("iint",FLOATARRAY_TYPE,send,cend,&cycend);
              
              
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
    history, references, conventions, CoordSys;


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
      if(fileObject->ReadStringAttribute("CoordinateSystem", CoordSys))
        comment += (std::string(", CoordinateSystem=") + CoordSys);

      if(siglaystate)
        comment += (std::string(", SigmaLayerCoordinates=") + SigLayCoordType);
      if(siglevstate)
        comment += (std::string(", SigmaLevelCoordinates=") + SigLevCoordType);

      md->SetDatabaseComment(comment);
    }

  debug4 << mName <<"Finished SetDatabaseComments" << endl;
  
  
  std::string xUnits, xLabel, yUnits, yLabel, zUnits,zLabel, hUnits,hLabel, latUnits,latLabel, lonUnits,lonLabel; 
  
  // If you can't return units or labels put "none" 
  if(! fileObject->ReadStringAttribute("lon", "units", lonUnits)) lonUnits="none";
  if(! fileObject->ReadStringAttribute("lon", "long_name", lonLabel)) lonLabel="none";
  
  if(! fileObject->ReadStringAttribute("lat", "units", latUnits)) latUnits="none";
  if(! fileObject->ReadStringAttribute("lat", "long_name", latLabel)) latLabel="none";
  
  if(! fileObject->ReadStringAttribute("y", "units", yUnits)) yUnits="none";
  if(! fileObject->ReadStringAttribute("y", "long_name", yLabel)) yLabel="none";
  
  if(! fileObject->ReadStringAttribute("x", "units", xUnits)) xUnits="none";
  if(! fileObject->ReadStringAttribute("x", "long_name", xLabel)) xLabel+"none";
  
  if(! fileObject->ReadStringAttribute("h", "units", hUnits)) hUnits="none";
  if(! fileObject->ReadStringAttribute("h", "long_name", hLabel)) hLabel="none";
  
  if(! fileObject->ReadStringAttribute("zeta", "units", zUnits)) zUnits="none";
  if(! fileObject->ReadStringAttribute("zeta", "long_name", zLabel)) zLabel="none";
    

  // Not sure what to do about units and labels... strings are loaded for future use

  if(IsGeoRef)
    {
      xUnits="meters";
      yUnits="meters";
      hUnits="meters";
      zUnits="meters";

      xLabel="X";
      yLabel="Y";
      zLabel="Z";
      hLabel="Z";

      // Bathymetry
      mesh1= hstate * lonstate * latstate;
      // SSH
      mesh2= lonstate * latstate * zstate;
      // TWOD
      mesh3= lonstate * latstate;
      // Sigma Layers
      mesh4= lonstate * latstate * siglaystate * hstate * zstate;
      // Sigma Levels
      mesh5= lonstate * latstate * siglevstate * hstate * zstate;

    }
  else if(! IsGeoRef)
    {
      mesh1= hstate * xstate * ystate;
      mesh2= xstate * ystate * zstate;
      mesh3= xstate * ystate;
      mesh4= xstate * ystate * siglaystate * hstate * zstate;
      mesh5= xstate * ystate * siglevstate * hstate * zstate;
    }


  // Add mesh exists statement to check if mesh loaded!
  std::map<std::string, bool> meshExists;

  // Define mesh names:

  std::string Bathymetry_Mesh("Bathymetry_Mesh");
  std::string SSH_Mesh("SSH_Mesh");
  std::string TWOD_Mesh("TWOD_Mesh");
  std::string SigmaLayer_Mesh("SigmaLayer_Mesh");
  std::string SigmaLevel_Mesh("SigmaLevel_Mesh");


  //--------------------------------------------------------------------
  //---------------------------------------------------------------
  // Add the Bathymetry_Mesh.
  //
  //  Get the units for the mesh.
    
  if (mesh1==true)
    {
      avtMeshMetaData *md_BM = new avtMeshMetaData(Bathymetry_Mesh, 
                                                   1, 1, 1, 0, 3, 2, 
                                                   AVT_UNSTRUCTURED_MESH);
      md_BM->xUnits = xUnits;
      md_BM->xLabel = xLabel;
      md_BM->yUnits = yUnits;
      md_BM->yLabel = yLabel;
      md_BM->zUnits = hUnits;
      md_BM->zLabel = hLabel;
      md->Add(md_BM);
      meshExists["Bathymetry_Mesh"] = true;
        
      debug4 << mName << "Added Bathymetry Mesh to MetaData" << endl;
    }

  //-------------------------------------------------------------------
  //
  // Add the SSH_Mesh.
  //

  if (mesh2==true)
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
  if (mesh3==true)
    {          
      
      avtMeshMetaData *md_2D;
      
      if(IsGeoRef)
        md_2D = new avtMeshMetaData(TWOD_Mesh, 
                                    1, 1, 1, 0, 3, 2, 
                                    AVT_UNSTRUCTURED_MESH);
      else if(!IsGeoRef)
        md_2D = new avtMeshMetaData(TWOD_Mesh, 
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
  if (mesh4==true)
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


  if (mesh5==true)
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
      std::string Var_Mesh("none");
      if(!fileObject->ReadStringAttribute(VarName,"grid", Var_Mesh)) 
        Var_Mesh=std::string("none");

      debug4<< "Grid attribute: "<< Var_Mesh << endl;


      int vnd=VarnDims-1;

      // The Grid on which to load data should be determined from the file meta data. This allows for more flexability in loading the data. The old method is kept for backward compatiablity.

//       debug4 << "Var_Mesh: " << Var_Mesh << endl;
//       debug4 << "Bathymetry_Mesh: " << Bathymetry_Mesh << endl;
//       debug4 << "TWOD_Mesh: " << TWOD_Mesh << endl;
//       debug4 << "SSH_Mesh: " << SSH_Mesh << endl;
//       debug4 << "SigmaLayer_Mesh: " << SigmaLayer_Mesh << endl;
//       debug4 << "SigmaLevel_Mesh: " << SigmaLevel_Mesh << endl;

      if (meshExists.find( Var_Mesh.c_str() ) != meshExists.end())
        {
          
          debug4<< "Grid is named: use attribute to set meta data!" << endl;
          
      
      // BATHYMETRY MESH
      if (strcmp(Var_Mesh.c_str(),Bathymetry_Mesh.c_str())==0 &&
          VarDimIDs[vnd] == nNodeID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,Bathymetry_Mesh, AVT_NODECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }

      else if (strcmp(Var_Mesh.c_str(),Bathymetry_Mesh.c_str())==0 &&
               VarDimIDs[vnd] == nElemID )
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
               VarDimIDs[vnd] == nNodeID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,TWOD_Mesh, AVT_NODECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      else if (strcmp(Var_Mesh.c_str(),TWOD_Mesh.c_str())==0 &&
               VarDimIDs[vnd] == nElemID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,TWOD_Mesh, AVT_ZONECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      // SSH MESH
      else if (strcmp(Var_Mesh.c_str(),SSH_Mesh.c_str())==0 &&
               VarDimIDs[vnd] == nNodeID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,SSH_Mesh, AVT_NODECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      else if (strcmp(Var_Mesh.c_str(),SSH_Mesh.c_str())==0 &&
               VarDimIDs[vnd] == nElemID )
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
               VarDimIDs[vnd] == nNodeID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,SigmaLayer_Mesh, AVT_NODECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      else if (strcmp(Var_Mesh.c_str(),SigmaLayer_Mesh.c_str())==0 &&
               VarDimIDs[vnd] == nElemID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,SigmaLayer_Mesh, AVT_ZONECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;

          // This catches all the usual scalars and puts them on cellcent!
          std::string ccvarname("CellCent_");
          ccvarname+= VarName;
          avtScalarMetaData *ccsmd = 
            new avtScalarMetaData(ccvarname.c_str(),SigmaLevel_Mesh, AVT_ZONECENT);
          ccsmd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(ccsmd);
          componentExists[ccvarname] = true;

        }
      // SIMGALEVEL MESH
      else if (strcmp(Var_Mesh.c_str(),SigmaLevel_Mesh.c_str())==0 &&
               VarDimIDs[vnd] == nNodeID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,SigmaLevel_Mesh, AVT_NODECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      else if (strcmp(Var_Mesh.c_str(),SigmaLevel_Mesh.c_str())==0 &&
               VarDimIDs[vnd] == nElemID )
        {
          avtScalarMetaData *smd = 
            new avtScalarMetaData(VarName,SigmaLevel_Mesh, AVT_ZONECENT);
          smd->hasUnits = fileObject->
            ReadStringAttribute(VarName,"units", smd->units);
          md->Add(smd);
          componentExists[VarName] = true;
        }
      
        } // End if it has a specified mesh!

      else // GO BACK TO THE OLD METHOD!
        {
      //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      if (VarnDims==1)
        {
          if (VarDimIDs[0] == nNodeID )
            {
              if (strcmp(VarName, "h")==0)
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

              // This catches all the usual scalars and puts them on cellcent!
              std::string ccvarname("CellCent_");
              ccvarname+= VarName;
              avtScalarMetaData *ccsmd = 
                new avtScalarMetaData(ccvarname.c_str(),SigmaLevel_Mesh, AVT_ZONECENT);
              ccsmd->hasUnits = fileObject->
                ReadStringAttribute(VarName,"units", smd->units);
              md->Add(ccsmd);
              componentExists[ccvarname] = true;

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
      //debug4 << mName << "Velocity is 3D" << endl;
      avtVectorMetaData *smd = new avtVectorMetaData("3DVEL",
                                                     SigmaLevel_Mesh, AVT_ZONECENT,3);
      smd->hasUnits = fileObject->ReadStringAttribute("u", 
                                                      "units", smd->units);
      md->Add(smd);    
    }

  if(componentExists.find("u") != componentExists.end() &&
     componentExists.find("v") != componentExists.end())
    {
      //debug4 << mName <<"Velocity is 2D" << endl;
      avtVectorMetaData *smd = new avtVectorMetaData("2DVEL",
                                                     SigmaLevel_Mesh, AVT_ZONECENT,3);
      smd->hasUnits = fileObject->ReadStringAttribute("u", 
                                                      "units", smd->units);
      md->Add(smd);
    }

  if(componentExists.find("uuice") != componentExists.end() &&
     componentExists.find("vvice") != componentExists.end())
    {
      // debug4 << mName <<"Velocity is 2D" << endl;
      avtVectorMetaData *smd = new avtVectorMetaData("Ice_Vel",
                                                     SSH_Mesh, AVT_ZONECENT,3);
      smd->hasUnits = fileObject->ReadStringAttribute("uuice", 
                                                      "units", smd->units);
      md->Add(smd);
    }

  if(componentExists.find("uuwind") != componentExists.end() &&
     componentExists.find("vvwind") != componentExists.end())
    {
      // debug4 << mName <<"Velocity is 2D" << endl;
      avtVectorMetaData *smd = new avtVectorMetaData("Wind_Vel",
                                                     SSH_Mesh, AVT_ZONECENT,3);
      smd->hasUnits = fileObject->ReadStringAttribute("uuwind", 
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


      // ADDED the variables again, interpolated to the sigmalevel mesh!
      avtScalarMetaData *dens3_mdl = new avtScalarMetaData("CellCent_Dens3{S,Theta,P}",
                                                          SigmaLevel_Mesh, AVT_ZONECENT);
      dens3_mdl->hasUnits = true;
      dens3_mdl->units = "kg/m3";
      md->Add(dens3_mdl);

      avtScalarMetaData *dens_mdl = new avtScalarMetaData("CellCent_Dens{S,T,P}",
                                                         SigmaLevel_Mesh, AVT_ZONECENT);
      dens_mdl->hasUnits = true;
      dens_mdl->units = "kg/m3";
      md->Add(dens_mdl);

      avtScalarMetaData *theta_mdl = new avtScalarMetaData("CellCent_Theta{S,T,P,0}",
                                                          SigmaLevel_Mesh, AVT_ZONECENT);
      theta_mdl->hasUnits = true;
      theta_mdl->units = "Degrees_C";
      md->Add(theta_mdl);
 
      // Removed for 1.5.4 release
      //      avtScalarMetaData *dens2_md = new avtScalarMetaData("Dens2(S,Theta,0)",
      //          SigmaLayer_Mesh, AVT_NODECENT);
      //      dens2_md->hasUnits = true;
      //      dens2_md->units = "kg/m3";
      //      md->Add(dens2_md);
      //      componentExists["Dens2(S,Theta,0)"] = true;
    
    }

  //    debug4 << "meshExists state" << meshExists.find("SSH_MESH") << endl;


  if(componentExists.find("ngid") != componentExists.end())
    {
      if (meshExists.find("SigmaLayer_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smdn2 = new avtScalarMetaData("ngid_Layers",
                                                           SigmaLayer_Mesh, AVT_NODECENT);
          md->Add(smdn2);
        }
      
      if (meshExists.find("SigmaLevel_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smdn3 = new avtScalarMetaData("ngid_Levels",
                                                           SigmaLevel_Mesh, AVT_NODECENT);
          md->Add(smdn3);
        }
      
      if (meshExists.find("SSH_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smdn4 = new avtScalarMetaData("ngid_SSH",
                                                           SSH_Mesh, AVT_NODECENT);
          md->Add(smdn4);
        }
      
      if (meshExists.find("Bathymetry_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smdn5 = new avtScalarMetaData("ngid_Bathy",
                                                           Bathymetry_Mesh, AVT_NODECENT);
          md->Add(smdn5);
        }
    }
  else
    {
      avtScalarMetaData *smdn1 = new avtScalarMetaData("ngid",
                                                       TWOD_Mesh, AVT_NODECENT);
      md->Add(smdn1);
      
      if (meshExists.find("SigmaLayer_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smdn2 = new avtScalarMetaData("ngid_Layers",
                                                           SigmaLayer_Mesh, AVT_NODECENT);
          md->Add(smdn2);
        }
      
      if (meshExists.find("SigmaLevel_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smdn3 = new avtScalarMetaData("ngid_Levels",
                                                           SigmaLevel_Mesh, AVT_NODECENT);
          md->Add(smdn3);
        }
      
      if (meshExists.find("SSH_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smdn4 = new avtScalarMetaData("ngid_SSH",
                                                           SSH_Mesh, AVT_NODECENT);
          md->Add(smdn4);
        }
      
      if (meshExists.find("Bathymetry_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smdn5 = new avtScalarMetaData("ngid_Bathy",
                                                           Bathymetry_Mesh, AVT_NODECENT);
          md->Add(smdn5);
        }
    }


  if(componentExists.find("egid") != componentExists.end())
    {
      if (meshExists.find("SigmaLayer_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smde2 = new avtScalarMetaData("egid_Layers",
                                                           SigmaLayer_Mesh, AVT_ZONECENT);
          md->Add(smde2);
        }
      if (meshExists.find("SigmaLevel_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smde3 = new avtScalarMetaData("egid_Levels",
                                                           SigmaLevel_Mesh, AVT_ZONECENT);
          md->Add(smde3);
        }

      if (meshExists.find("SSH_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smde4 = new avtScalarMetaData("egid_SSH",
                                                           SSH_Mesh, AVT_ZONECENT);
          md->Add(smde4);
        }
      if (meshExists.find("Bathymetry_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smde5 = new avtScalarMetaData("egid_Bathy",
                                                           Bathymetry_Mesh, AVT_ZONECENT);
          md->Add(smde5);
        }
    }
  else
    {
      avtScalarMetaData *smde1 = new avtScalarMetaData("egid",
                                                          TWOD_Mesh, AVT_ZONECENT);
      md->Add(smde1);

      if (meshExists.find("SigmaLayer_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smde2 = new avtScalarMetaData("egid_Layers",
                                                           SigmaLayer_Mesh, AVT_ZONECENT);
          md->Add(smde2);
        }
      if (meshExists.find("SigmaLevel_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smde3 = new avtScalarMetaData("egid_Levels",
                                                           SigmaLevel_Mesh, AVT_ZONECENT);
          md->Add(smde3);
        }

      if (meshExists.find("SSH_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smde4 = new avtScalarMetaData("egid_SSH",
                                                           SSH_Mesh, AVT_ZONECENT);
          md->Add(smde4);
        }
      if (meshExists.find("Bathymetry_Mesh") != meshExists.end() )
        {
          avtScalarMetaData *smde5 = new avtScalarMetaData("egid_Bathy",
                                                           Bathymetry_Mesh, AVT_ZONECENT);
          md->Add(smde5);
        }
    }





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


  if(IsGeoRef & meshExists.find("SigmaLevel_Mesh") != meshExists.end() )
    {
      avtScalarMetaData *depth_lev_md = new avtScalarMetaData("Depth_on_Levels",
                                                          SigmaLevel_Mesh, AVT_NODECENT);
      depth_lev_md->hasUnits = true;
      depth_lev_md->units = "Meters";
      md->Add(depth_lev_md);
      componentExists["depth"] = true;
    }
  if(IsGeoRef & meshExists.find("SigmaLayer_Mesh") != meshExists.end() )
    {
      avtScalarMetaData *depth_lay_md = new avtScalarMetaData("Depth_on_Layers",
                                                          SigmaLayer_Mesh, AVT_NODECENT);
      depth_lay_md->hasUnits = true;
      depth_lay_md->units = "Meters";
      md->Add(depth_lay_md);
      componentExists["depth"] = true;
    }


  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // ALL Variables added to META DATA


  if(mesh4=true)
    {
      //Add Material MetaData for: SigmaLayer_Mesh
      avtMaterialMetaData *matmd_lay = new avtMaterialMetaData;
      matmd_lay->name = "Sigma_Layers";
      matmd_lay->meshName = "SigmaLayer_Mesh";
      matmd_lay->numMaterials=(nSiglay-1);
      
      for (int i=0; i < (nSiglay-1); ++i)
        {
          
          char buffer[50];
          int n;
          n=sprintf(buffer, "Layer %d",(i+1));
          matmd_lay->materialNames.push_back(buffer);
        }
      
      
      md->Add(matmd_lay);
    }

  if(mesh5==true)
    {
      //Add Material MetaData for: SigmaLevel_Mesh
      avtMaterialMetaData *matmd_lev = new avtMaterialMetaData;
      matmd_lev->name = "Sigma_Levels";
      matmd_lev->meshName = "SigmaLevel_Mesh";
      matmd_lev->numMaterials=(nSiglev-1);
      
      for (int i=0; i < (nSiglev-1); ++i)
        {
          char buffer [50];
          int n;
          n=sprintf(buffer, "Layer %d",(i+1));
          matmd_lev->materialNames.push_back(buffer);
          
        }
      
      md->Add(matmd_lev);
      
    }
  

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
  fileObject->ReadVariableInto("x", FLOATARRAY_TYPE, xvals);
  
  yvals = new float[nNode];
  fileObject->ReadVariableInto("y", FLOATARRAY_TYPE, yvals);


  // Get Node Connectivity
  nvvals= new int[nThree * nElem];
  fileObject->ReadVariableInto("nv", INTEGERARRAY_TYPE, nvvals);


  // READ or GET the global element and global node info
  egid = new int[nElem];
  status = nc_inq_varid (ncid, "egid", &VarID);
  if (status != NC_NOERR) 
    {
      fileObject-> HandleError(status);
      for (int n=0; n < nElem; ++n)
        egid[n]=n+1;
    }
  else
    fileObject->ReadVariableInto("egid", INTEGERARRAY_TYPE, egid);

  ngid = new int[nNode];
  status = nc_inq_varid (ncid, "ngid", &VarID);
  if (status != NC_NOERR) 
    {
      fileObject-> HandleError(status);
      for (int n=0; n < nNode; ++n)
        ngid[n]=n+1;
    }
  else
    fileObject->ReadVariableInto("ngid", INTEGERARRAY_TYPE, ngid);


  if(IsGeoRef)
    {

      debug4<< "Reading spherical coordinates variables, lat and lon"<< endl;
      latvals = new float[nNode];
      fileObject->ReadVariableInto("lat", FLOATARRAY_TYPE, latvals);

      lonvals = new float[nNode];
      fileObject->ReadVariableInto("lon", FLOATARRAY_TYPE, lonvals);
    }


  // Bathymetry
  zvals = new float[nNode];
  state*=fileObject->ReadVariableInto("h", FLOATARRAY_TYPE, zvals);

  
  if(siglaystate)
    if (strcmp("ocean_sigma/general_coordinate",SigLayCoordType.c_str())==0)
      {
        debug4<< "Reading General ocean coordinates: sigma layers"<<endl;
        SigLayers = new float[nSiglay*nNode];
        fileObject->ReadVariableInto("siglay", FLOATARRAY_TYPE, SigLayers);
      }
    else if (strcmp("ocean_sigma_coordinate",SigLayCoordType.c_str())==0)
      {

        debug4<< "Reading sigma ocean coordinates: sigma layers"<<endl;
        SigLayers = new float[nSiglay];
        fileObject->ReadVariableInto("siglay", FLOATARRAY_TYPE, SigLayers);
      }
  
  if (siglevstate)
    if (strcmp("ocean_sigma/general_coordinate",SigLevCoordType.c_str())==0)
      {
        debug4<< "Reading General ocean coordinates: sigma levels"<<endl;
        SigLevels = new float[nSiglev*nNode];
        fileObject->ReadVariableInto("siglev", FLOATARRAY_TYPE, SigLevels);
      }
    else if (strcmp("ocean_sigma_coordinate",SigLevCoordType.c_str())==0)
      {
        debug4<< "Reading sigma ocean coordinates: sigma levels"<<endl;
        SigLevels = new float[nSiglev];
        fileObject->ReadVariableInto("siglev", FLOATARRAY_TYPE, SigLevels); 
      }

  
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
      debug4<< "Reading a time invariant grid, set time state to zero!" << endl;
      timestate=0; // bathymetry is not time varying in FVCOM!
    }
  else  if(strcmp(mesh, "TWOD_Mesh") == 0)
    {
      debug4<< "Reading a time invariant grid, set time state to zero!" << endl;
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
  // Visit must count from 0 to nNodes-1
  // ***************************************************************

  debug4 << mName << "Looking to see if Mesh is in cache" << endl;
  vtkObject *obj =0;
  
  std::string cachekey= mesh;
  cachekey +=keysuffix;
  debug4 << "cachekey: " << cachekey << endl;

  int domain=CacheDomainIndex; //  Set domain for MTMD!!!
  const char *matname = "all";
  obj = cache->GetVTKObject(cachekey.c_str(), avtVariableCache::DATASET_NAME,
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

      if(IsGeoRef)
        {
          for(int i= 0; i< nNode; ++i)
            {
              float pt[3]={lonvals[i], latvals[i],-zvals[i]};
              float *xi=pt;
              Sphere2Cart(xi);
              pts->InsertNextPoint(pt);
            }
          
        }
      else
        {
          // insert nodes into mesh pts object
          for(int i= 0; i< nNode; ++i)
            {
              float pt[3]={xvals[i], yvals[i],-zvals[i]};
              pts->InsertNextPoint(pt);
            }
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
      if (IsGeoRef)
        {
          for(int i= 0; i< nNode; ++i)
            {
              float pt[3]={lonvals[i], latvals[i],0};
              float *xi=pt;
              Sphere2Cart(xi);
              pts->InsertNextPoint(pt);
            }
        }
      else
        {
          for(int i= 0; i< nNode; ++i)
            {
              float pt[3]={xvals[i], yvals[i],0};
              pts->InsertNextPoint(pt);
            }
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
    } // End if TWOD_MESH
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

//       size_t starts[]={timestate,0};
//       size_t counts[]={1, nNode};
//       ptrdiff_t stride[]={1,1};
//       float *sshvals = new float[nNode];
//       int var_id;
//       status = nc_inq_varid (fileObject->GetFileHandle(), "zeta", &var_id);
//       if (status != NC_NOERR) fileObject-> HandleError(status);

//       status = nc_get_vars_float(fileObject->GetFileHandle(),var_id,
//                                  starts, counts, stride, sshvals);
//       if (status != NC_NOERR) fileObject-> HandleError(status);

      debug4 << mName << "Loading: zeta"<< endl;
      vtkDataArray *SSH = GetVar(timestate,"zeta",cache);
      float sshval;
      // insert nodes into mesh pts object
      if(IsGeoRef)
        {
          for(int i= 0; i< nNode; ++i)
            {
              sshval=SSH->GetComponent(i,0);
              float pt[3]={xvals[i], yvals[i], sshval};
              float *xi=pt;
              Sphere2Cart(xi);
              pts->InsertNextPoint(pt);
            } // end for nNodesPerLayer
        }
      else
        {
          for(int i= 0; i< nNode; ++i)
            {
              sshval=SSH->GetComponent(i,0);
              float pt[3]={xvals[i], yvals[i], sshval};
              pts->InsertNextPoint(pt);
            } // end for nNodesPerLayer
        }

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


      SSH->Delete();

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

//       size_t starts[]={timestate,0};
//       size_t counts[]={1, nNode};
//       ptrdiff_t stride[]={1,1};
//       float *sshvals = new float[nNode];
//       int var_id;
//       status = nc_inq_varid (fileObject->GetFileHandle(), "zeta", &var_id);
//       if (status != NC_NOERR) fileObject-> HandleError(status);
       
//       // SSH
//       status = nc_get_vars_float(fileObject->GetFileHandle(),var_id,
//                                  starts, counts, stride, sshvals);
//       if (status != NC_NOERR) fileObject-> HandleError(status);

      
      debug4 << mName << "Loading: zeta"<< endl;
      vtkDataArray *SSH = GetVar(timestate,"zeta",cache);

      if (strcmp("ocean_sigma/general_coordinate",SigLayCoordType.c_str())==0)
        {

          debug4 << mName; 
          debug4 << "General Coordinates!"<< endl;

          // insert nodes into mesh pts object
          if(IsGeoRef)
            {
              float depth, sshval;
              int index;
              for(int lay = 0; lay< nSiglay; ++lay) // order! surface -> bottom
                {
                  for(int node= 0; node< nNode; ++node)
                    {
                      sshval=SSH->GetComponent(node,0);
                      index = lay*nNode + node;
                      depth= SigLayers[index]*(zvals[node]+sshval)+sshval;
                      
                      float pt[3]={lonvals[node], latvals[node], depth};
                      float *xi=pt;
                      Sphere2Cart(xi);
                      pts->InsertNextPoint(pt);
                    }// end for nNodesPerLayer
                }// end for nSigLevels
            }
          else
            {
              float depth, sshval;
              int index;
              for(int lay = 0; lay< nSiglay; ++lay) // order! surface -> bottom
                {
                  for(int node= 0; node< nNode; ++node)
                    {
                      sshval=SSH->GetComponent(node,0);
                      index = lay*nNode + node;
                      depth= SigLayers[index]*(zvals[node]+sshval)+sshval;
                      
                      float pt[3]={xvals[node], yvals[node], depth};
                      pts->InsertNextPoint(pt);
                    }// end for nNodesPerLayer
                }// end for nSigLevels
            }
          debug4 << "Inserted points to General Coord Layer Mesh" << endl;
          
        }
      else if (strcmp("ocean_sigma_coordinate",SigLayCoordType.c_str())==0) 
        // USE OCEAN SIGMA COORDINATE
        {
        
          debug4 << mName; 
          debug4 << "Sigma Coordinates!"<< endl;

          // insert nodes into mesh pts object
          if(IsGeoRef)
            {
              float depth, sshval;
              for(int lay = 0; lay< nSiglay; ++lay) // order! surface -> bottom
                {
                  for(int node= 0; node< nNode; ++node)
                    {
                      sshval=SSH->GetComponent(node,0);
                      depth= SigLayers[lay]*(zvals[node]+sshval)+sshval;
                      float pt[3]={lonvals[node], latvals[node], depth};
                      float *xi=pt;
                      Sphere2Cart(xi);
                      pts->InsertNextPoint(pt);
                    }// end for nNodesPerLayer
                }// end for nSigLevels
            }
          else
            {
              float depth, sshval;
              for(int lay = 0; lay< nSiglay; ++lay) // order! surface -> bottom
                {
                  for(int node= 0; node< nNode; ++node)
                    {
                      sshval=SSH->GetComponent(node,0);
                      depth= SigLayers[lay]*(zvals[node]+sshval)+sshval;
                      float pt[3]={xvals[node], yvals[node], depth};
                      pts->InsertNextPoint(pt);
                    }// end for nNodesPerLayer
                }// end for nSigLevels
            }
              debug4 << "Inserted points to Sigma Coord. Layer Mesh" << endl;
        } // end sigma coordinate type...


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

      SSH->Delete();

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

//       size_t starts[]={timestate,0};
//       size_t counts[]={1, nNode};
//       ptrdiff_t stride[]={1,1};
//       float *sshvals = new float[nNode];
//       int var_id;
//       status = nc_inq_varid (fileObject->GetFileHandle(), "zeta", &var_id);
//       if (status != NC_NOERR) fileObject-> HandleError(status);

//       // SSH
//       status = nc_get_vars_float(fileObject->GetFileHandle(),var_id,
//                                  starts, counts, stride, sshvals);
//       if (status != NC_NOERR) fileObject-> HandleError(status);
      debug4 << mName << "Loading: zeta"<< endl;
      vtkDataArray *SSH = GetVar(timestate,"zeta",cache);


      if (strcmp("ocean_sigma/general_coordinate",SigLevCoordType.c_str())==0)
        {
          debug4 << mName; 
          debug4 << "General Coordinates"<< endl;

          // insert nodes into mesh pts object

          if (IsGeoRef)
            {
              float depth, sshval;
              int index;
              for(int lev = 0; lev< nSiglev; ++lev) // order! surface -> bottom
                {
                  for(int node= 0; node< nNode; ++node)
                    { 
                      sshval=SSH->GetComponent(node,0);
                      index = lev*nNode + node;
                      depth= SigLevels[index]*(zvals[node]+sshval)+sshval;
                      float pt[3]={lonvals[node], latvals[node], depth};
                      float *xi=pt;
                      Sphere2Cart(xi);
                      pts->InsertNextPoint(pt);
                    }// end for nNodesPerLayer
                }// end for nSigLevels
            }
          else
            {
              float depth, sshval;
              int index;
              for(int lev = 0; lev< nSiglev; ++lev) // order! surface -> bottom
                {
                  for(int node= 0; node< nNode; ++node)
                    { 
                      sshval=SSH->GetComponent(node,0);
                      index = lev*nNode + node;
                      depth= SigLevels[index]*(zvals[node]+sshval)+sshval;
                      float pt[3]={xvals[node], yvals[node], depth};
                      pts->InsertNextPoint(pt);
                    }// end for nNodesPerLayer
                }// end for nSigLevels
            }
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
          if (IsGeoRef)
            {
              float depth, sshval;
              for(int lev = 0; lev< nSiglev; ++lev) // order! surface -> bottom
                {
                  for(int node= 0; node< nNode; ++node)
                    { 
                      sshval=SSH->GetComponent(node,0);
                      depth= SigLevels[lev]*(zvals[node]+sshval)+sshval;
                      float pt[3]={lonvals[node], latvals[node], depth};
                      float *xi=pt;
                      Sphere2Cart(xi);
                      pts->InsertNextPoint(pt);
                    }// end for nNodesPerLayer
                }// end for nSigLevels
            }
          else
            {
              float depth, sshval;
              for(int lev = 0; lev< nSiglev; ++lev) // order! surface -> bottom
                {
                  for(int node= 0; node< nNode; ++node)
                    { 
                      sshval=SSH->GetComponent(node,0);
                      depth= SigLevels[lev]*(zvals[node]+sshval)+sshval;
                      float pt[3]={xvals[node], yvals[node], depth};
                      pts->InsertNextPoint(pt);
                    }// end for nNodesPerLayer
                }// end for nSigLevels
              
            }
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


      SSH->Delete();;

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
  cache->CacheVTKObject(cachekey.c_str(), avtVariableCache::DATASET_NAME, timestate, domain,
                        matname, retval);


  debug4 << mName << "END" << endl;

  return retval;
}

// ****************************************************************************
// Method: avtFVCOMReaderFileFormat::Sphere2Cart
//
// Purpose: 
//   change the position data from longitude, latitude and depth
//   to cartesian coordinates relative to earth center
// Arguments: float pt[3]
//
// Returns: void
//
// Note:       
//
// Programmer: David Stuebe
// Creation:   Mon Jul 17 2006
//
// Modifications:
//   
// ****************************************************************************


void 
avtFVCOMReader::Sphere2Cart(float* xi)
{
  //  const char *mName = "avtFVCOMReader::Sphere2Cart: ";
  //  debug4 << mName << "running" << xi[0]<<":"<< xi[1]<<":"<< xi[2] << endl;
  
  // xi[0] is longitude
  // xi[1] is latitude
  // xi[2] is height reference to the geoid

  
  //  float rad=6000000.0;
  float rad=100000.0;
  float d2r=3.14159/180.0;

  //======================================================================
  // Do not use elliptical earth. Velocity transformation is too complex!
  //=====================================================================
  
  // Elliptical approximation of the earths Geoid
  //  g= 978.032(1+0.00528 cos^2(theta)+2.3*10^-5*cos^4(theta)
  // theta is colatitude

  //  float c1=0.00528;
  //  float c1=0.528; // to make very elliptical
  //  float c2=2.3*pow(10,-5);
  //  rad=rad*(1+
  //           c1*pow(cos(xi[1]*d2r),2.0)+
  //           c2*pow(cos(xi[1]*d2r),4.0) );

  float x = (rad+xi[2])*cos(xi[0]*d2r)*cos(xi[1]*d2r);
  float y = (rad+xi[2])*sin(xi[0]*d2r)*cos(xi[1]*d2r);
  float z = (rad+xi[2])*sin(xi[1]*d2r);

  xi[0]=x;
  xi[1]=y;
  xi[2]=z;
  
  //  debug4 << mName << "finished" << xi[0]<<":"<< xi[1]<<":"<< xi[2] << endl;

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


  if (NeedDimensions) GetDimensions();

  if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
      int nMats;
      if(strcmp(var, "Sigma_Levels") == 0)
        {
          nMats=nSiglev-1;
        }
      else if(strcmp(var, "Sigma_Layers") == 0)
        {
          nMats=nSiglay-1;
        }

      int nzones=(nMats)*nElem; // there are nSigLayers
      // in the levels mesh. There are nSigLayers-1 in the Sigma_Layers
      // mesh because its nodes are at the edge centers of the original 
      // mesh.
 
      // Create matnos and names arrays so we can create an avtMaterial.
      int *matnos = new int[nMats];
      char **names = new char *[nMats];
      for(int i = 0; i < nMats; ++i)
        {
          matnos[i] = i + 1;
          char *buffer=new char[20];
          int n;
          n=sprintf(buffer, "Layer %d",matnos[i]);
          names[i] = (char *)buffer;
        }
    

      int *matlist =new int[nzones];
      int mod;
      int this_mat=0; // start count from zero: mod(zero,x) is zero!
      for (int i =0; i< nzones; ++i)
        {
          mod = i % nElem;
          //          if(mod==0) matlist[i]=++this_mat;
          //          else matlist[i]=this_mat;
          matlist[i]= (mod==0) ? ++this_mat : this_mat;
        }
    
    
      // Create the avtMaterial.
      int dims[3]= {1,1,1};
      dims[0] = nzones;
      int ndims=1;

      avtMaterial *mat= new avtMaterial(
                                        nMats,
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
      for (int i = 0; i < nMats; ++i)
        delete [] names[i];
      delete [] names;
      retval =(void *)mat;
      df = avtMaterial::Destruct;
    }


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
//  Modifications:
//    Kathleen Bonnell, Mon Apr  2 16:46:18 PDT 2007
//    Change creation of vnm to fix compile error on Win32.
//
// ****************************************************************************

vtkDataArray *
avtFVCOMReader::GetVar(int timestate, const char *Variable, avtVariableCache *cache)
{
  const char *mName = "avtFVCOMReader::GetVar: ";
  debug4 << mName << "timestate=" << timestate
         << ", Variable=" << Variable << endl;

  // Variables saved in NETCDF OUT
  // Bail out if we can't get the file handle.
  int ncid = fileObject->GetFileHandle();
  if(ncid == -1)
    {
      EXCEPTION1(InvalidVariableException, Variable);
    }


  if (NeedDimensions) GetDimensions();
  if(NeedGridVariables) GetStaticGridVariables();


  debug4 << mName << "Looking to see if Variable is in cache" << endl;
  vtkObject *obj =0;

  std::string cachekey= Variable;
  cachekey +=keysuffix;
  debug4 << "cachekey: " << cachekey << endl;


  int domain=CacheDomainIndex; //  Set domain for MTMD!!!
  const char *matname = "all";
  obj = cache->GetVTKObject(cachekey.c_str(), avtVariableCache::DATASET_NAME,
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
      debug4 << mName << "Add Variable to cache!" << endl;
      cache->CacheVTKObject(cachekey.c_str(), avtVariableCache::DATASET_NAME, timestate, domain,
                        matname, rv);
      return rv;
    }  
  else if (strcmp("Dens{S,T,P}", Variable)==0)
    {
      debug4 << mName << "Variable is Dens"<< endl;
      vtkDataArray *rv =  DENS(timestate, cache);
      cache->CacheVTKObject(cachekey.c_str(), avtVariableCache::DATASET_NAME, timestate, domain,
                            matname, rv);
      return rv;
    }  
  else if (strcmp("Theta{S,T,P,0}", Variable)==0)
    {
      debug4 << mName << "Variable is Theta"<< endl;
      vtkDataArray *rv =  THETA(timestate, cache);
      cache->CacheVTKObject(cachekey.c_str(), avtVariableCache::DATASET_NAME, timestate, domain,
                            matname, rv);
      return rv;
    }  
  // add again, interpolated to sigma level mesh zones
  else if (strncmp("CellCent_", Variable,9)==0)
    {
      std::string strvarname(Variable);
      int namelength;
      namelength = strlen(Variable);
      size_t tmplen=namelength-9;
      std::string vnm = strvarname.substr(tmplen, 9);

      debug4 << mName << "Variable: "<< vnm << endl;
      vtkDataArray *tmp =  GetVar(timestate,vnm.c_str(), cache);
      int ntuples = nElem * (nSiglev-1);
      vtkFloatArray *rv = vtkFloatArray::New();
      rv->SetNumberOfTuples(ntuples);
      float rval;
      for(int i = 0; i < (nSiglev-1) ; i++)
        for(int j = 0; j < nElem ; j++)
          {
            rval=0;
            for(int k = 0; k < 3 ; k++)
              {
                int ind = nvvals[j+k*nElem] -1;
                rval += tmp->GetComponent(ind+nNode*i,0);
              }
            rval /=3.0;
            rv->SetTuple1(j+i*nElem, rval);
          }
      tmp->Delete();
      debug4 << mName << "Returning variable: " << Variable << endl;
      return rv;
    }  


  //=================================================
  //=================================================
  //=================================================
  // Make return variable for all other types of data!
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
  else if (strcmp("Depth_on_Levels", Variable)==0)
    {

      debug4 << mName << "Loading: zeta"<< endl;
      vtkDataArray *SSH = GetVar(timestate,"zeta",cache);
        
      int ntuples = nNode*nSiglev;  
      
      // Put the data into the vtkFloatArray   
      rv->SetNumberOfTuples(ntuples);
      
      if (strcmp("ocean_sigma/general_coordinate",SigLevCoordType.c_str())==0)
        {
          debug4 << mName; 
          debug4 << "General Coordinates"<< endl; 
          int count =0;
          float depth, sshval;
          int index;
          for(int lev = 0; lev< nSiglev; ++lev) // order! surface -> bottom
            {
              for(int node= 0; node< nNode; ++node)
                { 
                  sshval=SSH->GetComponent(node,0);
                  index = lev*nNode + node;
                  depth= -SigLevels[index]*(zvals[node]+sshval);
                  rv->SetTuple1(count, depth);
                  count++;
                }// end for nNodesPerLayer
            }// end for nSigLevels
        }
      else if (strcmp("ocean_sigma_coordinate",SigLevCoordType.c_str())==0) 
        {
          debug4 << mName; 
          debug4 << "Sigma Coordinates!"<< endl;
          int count =0;
          float depth, sshval;
          for(int lev = 0; lev< nSiglev; ++lev) // order! surface -> bottom
            {
              for(int node= 0; node< nNode; ++node)
                { 
                  sshval=SSH->GetComponent(node,0);
                  depth= -SigLevels[lev]*(zvals[node]+sshval);
                  rv->SetTuple1(count, depth);
                  count++;
                }// end for nNodesPerLayer
            }// end for nSigLevels  
        }

      SSH->Delete();

      return rv;
    }  
  else if (strcmp("Depth_on_Layers", Variable)==0)
    {
      debug4 << mName << "Loading: zeta"<< endl;
      vtkDataArray *SSH = GetVar(timestate,"zeta",cache);
      

      int ntuples = nNode*nSiglay;  
      
      // Put the data into the vtkFloatArray   
      rv->SetNumberOfTuples(ntuples);
      
      if (strcmp("ocean_sigma/general_coordinate",SigLayCoordType.c_str())==0)
        {
          debug4 << mName; 
          debug4 << "General Coordinates"<< endl; 
          int count =0;
          float depth, sshval;
          int index;
          for(int lay = 0; lay< nSiglay; ++lay) // order! surface -> bottom
            {
              for(int node= 0; node< nNode; ++node)
                { 
                  sshval=SSH->GetComponent(node,0);
                  index = lay*nNode + node;
                  depth= -SigLayers[index]*(zvals[node]+sshval);
                  rv->SetTuple1(count, depth);
                  count++;
                }// end for nNodesPerLayer
            }// end for nSigLevels
        }
      else if (strcmp("ocean_sigma_coordinate",SigLayCoordType.c_str())==0) 
        {
          debug4 << mName; 
          debug4 << "Sigma Coordinates!"<< endl;
          int count =0;
          float depth, sshval;
          for(int lay = 0; lay< nSiglay; ++lay) // order! surface -> bottom
            {
              for(int node= 0; node< nNode; ++node)
                { 
                  sshval=SSH->GetComponent(node,0);
                  depth= -SigLayers[lay]*(zvals[node]+sshval);
                  rv->SetTuple1(count, depth);
                  count++;
                }// end for nNodesPerLayer
            }// end for nSigLevels  
        }
      
      SSH->Delete();

      return rv;
    }
  else if (strncmp("ngid",Variable,4)==0) 
    {
      int clt, count=0;
      if (strcmp("ngid_Layers",Variable)==0)
        clt=nSiglay;
      else if (strcmp("ngid_Levels",Variable)==0)
        clt=nSiglev;
      else
        clt=1;

      int ntuples = nNode*clt;
      rv->SetNumberOfTuples(ntuples);

      for(int lay = 0; lay< clt; ++lay) // order! surface -> bottom
        {
          for(int node= 0; node< nNode; ++node)
            { 
              rv->SetTuple1(count, ngid[node]);
              count++;
            }// end for nNodesPerLayer
        }// end for nSigLevels  
      return rv;
    }  
    else if (strncmp("egid",Variable,4)==0) 
    {
      int clt, count=0;
      if (strcmp("egid_Layers",Variable)==0)
        clt=nSiglay-1;
      else if (strcmp("egid_Levels",Variable)==0)
        clt=nSiglev-1;
      else
        clt=1;

      int ntuples = nElem*clt;
      rv->SetNumberOfTuples(ntuples);

      for(int lay = 0; lay< clt; ++lay) // order! surface -> bottom
        {
          for(int elem= 0; elem< nElem; ++elem)
            { 
              rv->SetTuple1(count, egid[elem]);
              count++;
            }// end for nNodesPerLayer
        }// end for nSigLevels  
      return rv;
    }  

  
  nc_type VarType;
  
  
  status=nc_inq_varid(ncid,Variable,&VarID);
  if(status != NC_NOERR)
   {
     fileObject-> HandleError(status);
     EXCEPTION1(InvalidVariableException, Variable);
   }


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
  cache->CacheVTKObject(cachekey.c_str(), avtVariableCache::DATASET_NAME, timestate, domain,
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
//  Modifications:
//    Kathleen Bonnell, Mon Apr  2 16:46:18 PDT 2007
//    Made ncomps const to fix compile error on Win32.
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
      // use three even for the 2d Vel- setting ncomps=2 is buggy...
      rv->SetNumberOfComponents(ncomps);
      rv->SetNumberOfTuples(ntuples);
      float one_entry[ncomps];

      if (IsGeoRef)
        {

          debug4<< "running georef vectors"<< endl;
          for (int i = 0 ; i < ntuples ; i++)
            {
              one_entry[0]=uvel->GetComponent(i,0);
              one_entry[1]=vvel->GetComponent(i,0);
              one_entry[2]=0;
              int mod = i % nElem;
              //if (i==0) debug4<<"i="<<i<<"mod="<<mod<<endl;
              //else if (i==nElem) debug4<<"i="<<i<<"mod="<<mod<<endl;
              SphereVel2Cart(one_entry,mod);
              rv->SetTuple(i, one_entry); 
            }            
        }
      else
        for (int i = 0 ; i < ntuples ; i++)
          {
            one_entry[0]=uvel->GetComponent(i,0);
            one_entry[1]=vvel->GetComponent(i,0);
            one_entry[2]=0;
            rv->SetTuple(i, one_entry); 
          }
      
      uvel->Delete();
      vvel->Delete();
      debug4<< mName << "Returned Velocity data" << endl;
      

    }
  else if (strcmp("Ice_Vel", var)==0) 
    {
      
      debug4 << mName << "Loading: uuice" << endl;
      vtkDataArray *uvel = GetVar(timestate,"uuice",cache);

      debug4 << mName << "Loading: vvice" << endl;
      vtkDataArray *vvel = GetVar(timestate,"vvice",cache);

      debug4 << "Setting ntuples" << endl;
      int ntuples = uvel->GetNumberOfTuples();
      debug4 << "ntuples=" << ntuples << endl;

      const int ncomps = 3;  // This is the rank of the vector
      // use three even for the 2d Vel- setting ncomps=2 is buggy...
      rv->SetNumberOfComponents(ncomps);
      rv->SetNumberOfTuples(ntuples);
      float one_entry[ncomps];

      if (IsGeoRef)
        {

          debug4<< "running georef vectors"<< endl;
          for (int i = 0 ; i < ntuples ; i++)
            {
              one_entry[0]=uvel->GetComponent(i,0);
              one_entry[1]=vvel->GetComponent(i,0);
              one_entry[2]=0;
              int mod = i % nElem;
              //if (i==0) debug4<<"i="<<i<<"mod="<<mod<<endl;
              //else if (i==nElem) debug4<<"i="<<i<<"mod="<<mod<<endl;
              SphereVel2Cart(one_entry,mod);
              rv->SetTuple(i, one_entry); 
            }            
        }
      else
        for (int i = 0 ; i < ntuples ; i++)
          {
            one_entry[0]=uvel->GetComponent(i,0);
            one_entry[1]=vvel->GetComponent(i,0);
            one_entry[2]=0;
            rv->SetTuple(i, one_entry); 
          }
      
      uvel->Delete();
      vvel->Delete();
      debug4<< mName << "Returned Velocity data" << endl;
      

    }
  else if (strcmp("Wind_Vel", var)==0) 
    {
      
      debug4 << mName << "Loading: uuwind" << endl;
      vtkDataArray *uvel = GetVar(timestate,"uuwind",cache);

      debug4 << mName << "Loading: vvice" << endl;
      vtkDataArray *vvel = GetVar(timestate,"vvwind",cache);

      debug4 << "Setting ntuples" << endl;
      int ntuples = uvel->GetNumberOfTuples();
      debug4 << "ntuples=" << ntuples << endl;

      const int ncomps = 3;  // This is the rank of the vector
      // use three even for the 2d Vel- setting ncomps=2 is buggy...
      rv->SetNumberOfComponents(ncomps);
      rv->SetNumberOfTuples(ntuples);
      float one_entry[ncomps];

      if (IsGeoRef)
        {

          debug4<< "running georef vectors"<< endl;
          for (int i = 0 ; i < ntuples ; i++)
            {
              one_entry[0]=uvel->GetComponent(i,0);
              one_entry[1]=vvel->GetComponent(i,0);
              one_entry[2]=0;
              int mod = i % nElem;
              SphereVel2Cart(one_entry,mod);
              rv->SetTuple(i, one_entry); 
            }            
        }
      else
        for (int i = 0 ; i < ntuples ; i++)
          {
            one_entry[0]=uvel->GetComponent(i,0);
            one_entry[1]=vvel->GetComponent(i,0);
            one_entry[2]=0;
            rv->SetTuple(i, one_entry); 
          }
      
      uvel->Delete();
      vvel->Delete();
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

      if (IsGeoRef)
        for (int i = 0 ; i < ntuples ; i++)
          {
            one_entry[0]=uvel->GetComponent(i,0);
            one_entry[1]=vvel->GetComponent(i,0);
            one_entry[2]=wwvel->GetComponent(i,0);
            int mod = i % nElem;
            SphereVel2Cart(one_entry,mod);
            rv->SetTuple(i, one_entry); 
          }
      else
        for (int i = 0 ; i < ntuples ; i++)
          {
            one_entry[0]=uvel->GetComponent(i,0);
            one_entry[1]=vvel->GetComponent(i,0);
            one_entry[2]=wwvel->GetComponent(i,0);
            rv->SetTuple(i, one_entry); 
          }

      uvel->Delete();
      vvel->Delete();
      wwvel->Delete();
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
//  Method: avtFVCOMReader::SphereVel2Cart
//
//  Purpose:
//          Transform eastward, northward and local upward velocity to 
//          Cartesian coordinates relative to earth center
//
//  Arguments:
//
//
//  Modified:
//
//  Programmer: David Stuebe
//  Creation:   Thu DEC 19 08:39:01 EST 2006
//
// ****************************************************************************

void 
avtFVCOMReader::SphereVel2Cart(float* vel,int cell)
{

  //Conversion from degrees to radians
  float d2r=3.14159/180.0;

  float lon=0.0;
  float lat=0.0;

  float sinlon=0.0;
  float coslon=0.0;

  float sinlat=0.0;
  float coslat=0.0;

  float tmp1,tmp2,tmp3,tmp4;
  tmp1=0.0;
  tmp2=0.0;
  tmp3=0.0;
  tmp4=0.0;

  for (int i=0; i<3; ++i)
  {
  int nd=nvvals[cell]-1;
  lon=lonvals[nd];
  lat=latvals[nd];

  sinlon=sin(d2r*lon);
  tmp1=tmp1+sinlon;

  coslon=cos(d2r*lon);
  tmp2=tmp2+coslon;
  
  sinlat=sin(d2r*lat);
  tmp3=tmp3+sinlat;

  coslat=cos(d2r*lat);
  tmp4=tmp4+coslat;
  }

  sinlon=tmp1/3.0;
  coslon=tmp2/3.0;

  sinlat=tmp3/3.0;
  coslat=tmp4/3.0;


  float DeltaLon[3]; // The longitude unit vector writen in terms of x,y,z 
  // mag =1
  DeltaLon[0]=-sinlon; // x component
  DeltaLon[1]=coslon;  // y component
  DeltaLon[2]=0.0;       // z component

  float DeltaLat[3]; // The latitude unit vector writen in terms of x,y,z 
  //  float mag=sqrt(pow(sinlat,2)+1);
  DeltaLat[0]=-sinlat*coslon; // x component
  DeltaLat[1]=-sinlat*sinlon; // y component
  DeltaLat[2]=coslat; // z component

  float DeltaR[3]; // The Radius unit vector writen in terms of x,y,z 
  //mag=1
  DeltaR[0]=coslon*coslat; // x component
  DeltaR[1]=sinlon*coslat; // y component
  DeltaR[2]=sinlat;        // z component

  float u=0.0;
  float v=0.0;
  float w=0.0;
  

  u=vel[0]*DeltaLon[0] + vel[1]*DeltaLat[0] + vel[2]*DeltaR[0];

  v=vel[0]*DeltaLon[1] + vel[1]*DeltaLat[1] + vel[2]*DeltaR[1];

  w=vel[0]*DeltaLon[2] + vel[1]*DeltaLat[2] + vel[2]*DeltaR[2];

  vel[0]=u;
  vel[1]=v;
  vel[2]=w;
    

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
  vtkDataArray *SALT = GetVar(timestate,"salinity",cache);

  debug4 << mName << "Loading: Theta{S,T,P,0}" << endl;
  vtkDataArray *PTMP = GetVar(timestate,"Theta{S,T,P,0}",cache);

  int ntuples = SALT->GetNumberOfTuples();;
  debug4 << "ntuples= "<< ntuples << endl;


  vtkFloatArray *rv = vtkFloatArray::New();
  rv->SetNumberOfTuples(ntuples);

  double GRAV=9.81;
  double sval;
  double tval;
  double pbar;
  
  
  if (IsGeoRef)
    {
      
      debug4 << mName << "Using global coordinates method!" << endl;
      debug4 << mName << "Loading: Depth_on_Layers" << endl;
      vtkDataArray *DEPTH = GetVar(timestate,"Depth_on_Layers",cache);
      

      double dval;
      // ADDED CHECK VAL TO DEBUG!!! when i=ntuples do check val!
      for (int i = 0 ; i < ntuples+1 ; i++) 
        if (i<ntuples)
          {
            tval= PTMP->GetComponent(i,0); // must be (i,0) not (i,1)
            sval= SALT->GetComponent(i,0);
            dval=DEPTH->GetComponent(i,0);
            // This returns xyz location of each point
            pbar = GRAV *1.025 * (dval) * 0.01; 
            // Calculate pressure in Bar at depth z in meters
            //bar = 9.81 *ave_dens * depth_m /100
            
            // Compute density (kg/m3) at standard one atmosphere pressure    
            // Loaded Theta above! Converted from temp:
            double rho = SVAN(sval,tval,pbar);
            rv->SetTuple1(i, rho);
          }
        else if (i==ntuples)
          {
            //  Check Values go here!
            tval=40;
            sval=40;
            pbar = 1000; 
            
            // Compute density (kg/m3) at standard one atmosphere pressure    
            // Loaded Theta above! Converted from temp:
            double rho = SVAN(sval,tval,pbar);
            
            debug4 << "Dens_Fofonoff_Millard CHECK VAL!" << endl
                   << "check Values: (T=40 C, S=40 PSU, 1000 bar)" << endl
                   << "RHO1 = 59.82037     (kg/m3)" << endl
                   << "VALUE=" << rho << endl;
          }      
      PTMP->Delete();
      SALT->Delete();
      DEPTH->Delete();
      debug4 << mName << "end" << endl;
      
    }
  else
    {
      debug4 << mName << "Loading: SigmaLayer_Mesh" << endl;
      vtkDataSet *MESH =  GetMesh(timestate,"SigmaLayer_Mesh", cache);

      debug4 << mName << "Loading: zeta"<< endl;
      vtkDataArray *SSH = GetVar(timestate,"zeta",cache);

                  
      double xyz[3], sshval;

      // ADDED CHECK VAL TO DEBUG!!! when i=ntuples do check val!
      for (int i = 0 ; i < ntuples+1 ; i++) 
        if (i<ntuples)
          {
            tval= PTMP->GetComponent(i,0); // must be (i,0) not (i,1)
            sval= SALT->GetComponent(i,0);
            MESH->GetPoint(i,xyz); 

            int mod= i % nNode;
            sshval=SSH->GetComponent(mod,0);

            // This returns xyz location of each point
            pbar = GRAV *1.025 * (sshval-xyz[2]) * 0.01; 
            // Calculate pressure in Bar at depth z in meters
            //bar = 9.81 *ave_dens * depth_m /100
            
            // Compute density (kg/m3) at standard one atmosphere pressure    
            // Loaded Theta above! Converted from temp:

            // debug4<< "sval:"<<sval<< " tval:"<<tval<<" pbar:"<<pbar<<endl;
            double rho = SVAN(sval,tval,pbar);
            rv->SetTuple1(i, rho);
          }
        else if (i==ntuples)
          {
            //  Check Values go here!
            tval=40;
            sval=40;
            pbar = 1000; 
            
            // Compute density (kg/m3) at standard one atmosphere pressure    
            // Loaded Theta above! Converted from temp:
            double rho = SVAN(sval,tval,pbar);
            
            debug4 << "Dens_Fofonoff_Millard CHECK VAL!" << endl
                   << "check Values: (T=40 C, S=40 PSU, 1000 bar)" << endl
                   << "RHO1 = 59.82037     (kg/m3)" << endl
                   << "VALUE=" << rho << endl;
          }      
      PTMP->Delete();
      SALT->Delete();
      MESH->Delete();
      SSH->Delete();
      debug4 << mName << "end" << endl;
      
    }
  
  
  
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
  vtkDataArray *SALT = GetVar(timestate,"salinity",cache);

  debug4 << mName << "Loading Potential Temperature variable: temp;" << endl;
  vtkDataArray *PTMP = GetVar(timestate,"temp",cache);
  

  int ntuples = SALT->GetNumberOfTuples();;
  debug4 << "ntuples= "<< ntuples << endl;
    
  vtkFloatArray *rv = vtkFloatArray::New();
  rv->SetNumberOfTuples(ntuples);
    
  double GRAV=9.81;
  double xyz[3];
  double tval;
  double sval;
  double PBAR;
  double RHOF;
    
  if (IsGeoRef)
    {
      debug4 << mName << "Using global coordinates method!" << endl;
      debug4 << mName << "Loading: Depth_on_Layers" << endl;
      vtkDataArray *DEPTH = GetVar(timestate,"Depth_on_Layers",cache);
      
      double dval;
      // ADDED CHECK VAL TO DEBUG!!! when i=ntuples do check val!
      for (int i = 0 ; i < ntuples+1 ; i++) 
        {
          
          if (i<ntuples)
            {
              tval= PTMP->GetComponent(i,0); // must be (i,0) not (i,1)
              sval= SALT->GetComponent(i,0);
              dval= DEPTH->GetComponent(i,0);
              // This returns xyz location of each point
              PBAR = GRAV *1.025 * dval * 0.01; 
              // Calculate pressure in Bar at depth z in meters
              //Bar = 9.81 *ave_dens * depth_m /100
              RHOF = Dens3helper(sval,tval,PBAR);
              rv->SetTuple1(i, RHOF);
              
              
            }
          else if (i==ntuples)
            {
              tval=3;
              sval=35.5;
              PBAR = 300; 
              
              RHOF = Dens3helper(sval,tval,PBAR);
              
              debug4 << "Dens_Jackett_McDougall CHECK VAL!" << endl
                     << "check Values: (T=3 C, S=35.5 PSU, 300 bar)" << endl
                     << "RHOF = 1041.83267     (kg/m3)" << endl
                     << "VALUE=" << RHOF << endl;
              
            }      
          
        }
      
      SALT->Delete();
      PTMP->Delete();
      DEPTH->Delete();
    }
  else
    {
      
      debug4 << mName << "Loading: zeta"<< endl;
      vtkDataArray *SSH = GetVar(timestate,"zeta",cache);
      
      debug4 << mName << "Loading: SigmaLayer_Mesh" << endl;
      vtkDataSet *MESH =  GetMesh(timestate,"SigmaLayer_Mesh", cache);
      // ADDED CHECK VAL TO DEBUG!!! when i=ntuples do check val!
      double sshval;
      for (int i = 0 ; i < ntuples+1 ; i++) 
        {
          if (i<ntuples)
            {
              tval= PTMP->GetComponent(i,0); // must be (i,0) not (i,1)
              sval= SALT->GetComponent(i,0);
              MESH->GetPoint(i,xyz); 
              int mod= i % nNode;
              sshval=SSH->GetComponent(mod,0);
              // This returns xyz location of each point
              PBAR = GRAV *1.025 *(sshval-xyz[2]) * 0.01; 
              // Calculate pressure in Bar at depth z in meters
              //Bar = 9.81 *ave_dens * depth_m /100
              RHOF = Dens3helper(sval,tval,PBAR);
              rv->SetTuple1(i, RHOF);
            }
          else if (i==ntuples)
            {
              tval=3;
              sval=35.5;
              PBAR = 300; 
              
              RHOF = Dens3helper(sval,tval,PBAR);
              
              debug4 << "Dens_Jackett_McDougall CHECK VAL!" << endl
                     << "check Values: (T=3 C, S=35.5 PSU, 300 bar)" << endl
                     << "RHOF = 1041.83267     (kg/m3)" << endl
                     << "VALUE=" << RHOF << endl;
            }      
        }
      SALT->Delete();
      PTMP->Delete();
      MESH->Delete();
      SSH->Delete();
    }

 debug4 << mName << "end" << endl;
  return rv;
}// end DENS3

//==============================================================================|
// Do the business of the Dens3 Jackett and McDougall method...
//==============================================================================|

double
avtFVCOMReader::Dens3helper(double sval, double tval, double pbar)
{
  
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


  double TEMP[10]; // This will be temporary storage for intermediate steps!

  double sqrtsval=sqrt(sval);

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
  double BULK = BULK0 + pbar * (BULK1 + pbar * BULK2);
  
  //  Compute "in situ" density anomaly (kg/m3)
  double RHOF=(DEN1*BULK)/(BULK-pbar);
  
  
  return RHOF;
}




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
  vtkDataArray *SALT = GetVar(timestate,"salinity",cache);

  debug4 << mName << "Loading insitu temperature variable: temp;" << endl;
  vtkDataArray *TMP = GetVar(timestate,"temp",cache);
    
  int ntuples = SALT->GetNumberOfTuples();;
  debug4 << "ntuples= "<< ntuples << endl;
    
  vtkFloatArray *rv = vtkFloatArray::New();
  rv->SetNumberOfTuples(ntuples);
  double GRAV = 9.81;   
  double tval;
  double sval;
  double pbar;
  double ptmp;

  if(IsGeoRef)
    {
      debug4 << mName << "Using global coordinates method!" << endl;
      debug4 << mName << "Loading: Depth_on_Layers" << endl;
      vtkDataArray *DEPTH = GetVar(timestate,"Depth_on_Layers",cache);
      
      double dval;
      // ADDED CHECK VAL TO DEBUG!!! when i=ntuples do check val!
      for (int i = 0 ; i < ntuples+1 ; i++) 
        {
          
          if (i<ntuples)
            {
              tval= TMP->GetComponent(i,0); // must be (i,0) not (i,1)
              sval= SALT->GetComponent(i,0);
              dval= DEPTH->GetComponent(i,0);
              // This returns xyz location of each point
              pbar = GRAV *1.025 * dval * 0.01; 
              // Calculate pressure in Bar at depth z in meters
              //Bar = 9.81 *ave_dens * depth_m /100
              
              ptmp =Thetahelper(sval,tval,pbar*10.0);//use pressure in decibar!
              rv->SetTuple1(i, ptmp);
              
            }
          else if (i==ntuples)
            {
              tval=40;
              sval=40;
              pbar = 1000; 
              
              ptmp =Thetahelper(sval,tval,pbar*10.0);//use pressure in decibar!
              
              debug4 << "THETA CHECK VAL!" << endl
                     << "check Values: (T=40 C, S=40 (ipss-78), 1000 bar)" << endl
                     << "RHOF = 36.89073   (c)" << endl
                     << "VALUE=" << ptmp << endl;
            }     
        } 
      
      SALT->Delete();
      TMP->Delete();
      DEPTH->Delete();
    }
  else
    {

      debug4 << mName << "Loading: zeta"<< endl;
      vtkDataArray *SSH = GetVar(timestate,"zeta",cache);
      
      debug4 << mName << "Loading: SigmaLayer_Mesh" << endl;
      vtkDataSet *MESH =  GetMesh(timestate,"SigmaLayer_Mesh", cache);
      
      double xyz[3];
      double sshval;
      // ADDED CHECK VAL TO DEBUG!!! when i=ntuples do check val!
      for (int i = 0 ; i < ntuples+1 ; i++) 
        {
          
          if (i<ntuples)
            {
              tval= TMP->GetComponent(i,0); // must be (i,0) not (i,1)
              sval= SALT->GetComponent(i,0);
              MESH->GetPoint(i,xyz); 
              
              int mod= i % nNode;
              sshval=SSH->GetComponent(mod,0);
              // This returns xyz location of each point
              pbar = GRAV *1.025 * (sshval-xyz[2]) * 0.01; 
              // Calculate pressure in Bar at depth z in meters
              //Bar = 9.81 *ave_dens * depth_m /100
              
              ptmp =Thetahelper(sval,tval,pbar*10.0);//use pressure in decibar!
              rv->SetTuple1(i, ptmp);
              
            }
          else if (i==ntuples)
            {
              tval=40;
              sval=40;
              pbar = 1000; 
              
              ptmp =Thetahelper(sval,tval,pbar*10);//use pressure in decibar!
              
              debug4 << "THETA CHECK VAL!" << endl
                     << "check Values: (T=40 C, S=40 (ipss-78), 1000 bar)" << endl
                     << "Theta = 36.89073   (c)" << endl
                     << "VALUE=" << ptmp << endl;
            }     
        } 
      
      SALT->Delete();
      TMP->Delete();
      MESH->Delete();
      SSH->Delete();
    }  
  debug4 << mName << "end" << endl;
  return rv;
}// end THETA

//==============================================================================|
// Do the business of the conversion from insitu to potential temperature.
//==============================================================================|

double
avtFVCOMReader::Thetahelper(double S4, double T4, double P4)
{
  //  double S4 = sval;
  //  double P4 = pbar*10.0; // pressure in decibar!
  //  double T4 = tval;
  double PR = 0.0;
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
  return ptmp;
}


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
                +((2.7759e-12*T4-1.1351e-10)*ds+
                  ((-5.4481e-14*T4+8.733e-12)*T4-6.7795e-10)*T4+1.8741e-8))*P4
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
  double A4,B4,C4,D4,E4,AA1,BB1,AW,BW,K0,KW,K35,SVA;
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
