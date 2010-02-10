/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

//
// This code was contributed to the VisIt project by Corvid Technologies
// on February 10, 2010.
//

// ************************************************************************* //
//                            avtVelodyneFileFormat.C                           //
// ************************************************************************* //

#include <avtVelodyneFileFormat.h>

#include <string>

#include <vtkCellType.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>
#include <DebugStream.h>


using     std::string;

const string avtVelodyneFileFormat::node_name    ="Node";
const string avtVelodyneFileFormat::solid_name   ="Solid";
const string avtVelodyneFileFormat::shell_name   ="Shell";
const string avtVelodyneFileFormat::surface_name ="Surface";
const string avtVelodyneFileFormat::particle_name="Particle";
const string avtVelodyneFileFormat::tiednode_name="TiedNode";
const string avtVelodyneFileFormat::sph_name     ="SPH";
const string avtVelodyneFileFormat::invalid_name ="Invalid";


string avtVelodyneFileFormat::
composeName( const string& m, const string& v, const char app )
{
  return m+app+v;
}


void avtVelodyneFileFormat::
decomposeName( const string& s, string& m, string& v )
{
  if( s.compare( 0, solid_name.size(), solid_name )==0 )
    m = solid_name;
  else if( s.compare( 0, shell_name.size(), shell_name )==0 )
    m = shell_name;
  else if( s.compare( 0, surface_name.size(), surface_name )==0 )
    m = surface_name;
  else if( s.compare( 0, particle_name.size(), particle_name )==0 )
    m = particle_name;
  else if( s.compare( 0, node_name.size(), node_name )==0 )
    m = node_name;
  else if( s.compare( 0, tiednode_name.size(), tiednode_name )==0 )
    m = node_name;
  else if( s.compare( 0, sph_name.size(), sph_name )==0 )
    m = sph_name;
  else
    m = invalid_name;

  if( m!=invalid_name ) 
    v = s.substr( m.size()+1 );
  else
    v = s;
}


int avtVelodyneFileFormat::
getTypeId( const char* name )
{
  if( solid_name==name )
    return VelodyneReader::solid_type;
  else if( shell_name==name ) 
    return VelodyneReader::shell_type;
  else if( surface_name==name ) 
    return VelodyneReader::surface_type;
  else if( particle_name==name ) 
    return VelodyneReader::particle_type;
  else if( node_name==name ) 
    return VelodyneReader::node_type;
  else if( tiednode_name==name ) 
    return VelodyneReader::tiednode_type;
  else if( sph_name==name ) 
    return VelodyneReader::sph_type;
  else
    return -1;
}


const string& avtVelodyneFileFormat::
getTypeName( int type )
{
  switch( type ) {
  case VelodyneReader::node_type:
    return node_name;
  case VelodyneReader::solid_type:
    return solid_name;
  case VelodyneReader::shell_type:
    return shell_name;
  case VelodyneReader::surface_type:
    return surface_name;
  case VelodyneReader::particle_type:
    return particle_name;
  case VelodyneReader::tiednode_type:
    return tiednode_name;
  case VelodyneReader::sph_type:
    return sph_name;
  default:
    return invalid_name;
  }
}



int avtVelodyneFileFormat::
GetCycle()
{
  return reader_->GetNumCycles();
}


double avtVelodyneFileFormat::
GetTime()
{
  return reader_->GetSimuTime();
}



// ****************************************************************************
//  Method: avtVelodyneFileFormat constructor
//
//  Programmer: hpan -- generated by xml2avt
//  Creation:   Thu Aug 7 11:38:59 PDT 2008
//
// ****************************************************************************

avtVelodyneFileFormat::avtVelodyneFileFormat(const char *filename)
    : avtSTSDFileFormat(filename)
{
  debug2 << "Creating new avtVelodyneFileFormat ... filename=" << filename << "\n.";

  reader_ = new VelodyneReader();
  int ierr = reader_->open(filename);
  if( ierr==-1 ) {
    EXCEPTION1(InvalidDBTypeException,
           "The file could not be opened");
  }
  if( ierr==-2 ) {
    EXCEPTION1(InvalidDBTypeException,
           "The file does not contain valid groups");
  }
  if( ierr<0 ) {
    EXCEPTION1(InvalidDBTypeException,
           "The file does not have valid meshes");
  }

  idx_mn_=idx_mx_=-1;
  map_=NULL;
  crd_=NULL;
  nnvs_=0;
  pobj_ = vtkObjectBase::New();
}


avtVelodyneFileFormat::~avtVelodyneFileFormat()
{
  debug2 << "closed avtVelodyneFileFormat\n.";
  delete reader_;
  if( map_ )    { delete [] map_;     map_=NULL; }
  if( crd_ )    { crd_->Delete();     crd_=NULL; }
  for( int i=0; i<nnvs_; i++ ) {
    nvname_[i].clear();
    if( nvdata_[i] ) nvdata_[i]->Delete();
  }
  nnvs_=0;
  nvname_.clear();
  nvdata_.clear();
  pobj_->Delete();
}


// ****************************************************************************
//  Method: avtVelodyneFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: hpan -- generated by xml2avt
//  Creation:   Thu Aug 7 11:38:59 PDT 2008
//
// ****************************************************************************

void
avtVelodyneFileFormat::FreeUpResources(void)
{
  debug2 << "Free resources... " << "nnvs_= " << nnvs_ << "\n";
  // to save memory 
  if( map_ )    { delete [] map_;     map_=NULL; }
  if( crd_ )    { crd_->Delete();     crd_=NULL; }
  for( int i=0; i<nnvs_; i++ ) {
    if( nvdata_[i] ) {
      pobj_->UnRegister( nvdata_[i] );
      //nvdata_[i]->Delete();
      nvdata_[i]=NULL;
    }
  }
}


// ****************************************************************************
//  Method: avtVelodyneFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: hpan -- generated by xml2avt
//  Creation:   Thu Aug 7 11:38:59 PDT 2008
//
// ****************************************************************************

void
avtVelodyneFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
  int ierr;

  set<int> meshtypes;
  //meshtypes.insert( VelodyneReader::node_type );
  meshtypes.insert( VelodyneReader::solid_type );
  meshtypes.insert( VelodyneReader::shell_type );
  meshtypes.insert( VelodyneReader::surface_type );
  meshtypes.insert( VelodyneReader::particle_type );
  meshtypes.insert( VelodyneReader::tiednode_type );
  meshtypes.insert( VelodyneReader::sph_type );

  //
  // meshes
  //
  for( set<int>::const_iterator m=meshtypes.begin(); m!=meshtypes.end(); m++ ) {
    int num = reader_->GetMeshSize( *m );
    if( num>0 ) {
      int ntd;
      avtMeshType mtp;
      switch( *m ) {
      case VelodyneReader::solid_type:
    ntd = 3;
    mtp = AVT_UNSTRUCTURED_MESH;
    break;
      case VelodyneReader::shell_type:
      case VelodyneReader::surface_type:
    ntd = 2;
    mtp = AVT_UNSTRUCTURED_MESH;
    break;
      case VelodyneReader::tiednode_type:
//     ntd = 1;
//     mtp = AVT_UNSTRUCTURED_MESH;
//     break;
      case VelodyneReader::node_type:
      case VelodyneReader::particle_type:
      case VelodyneReader::sph_type:
    ntd = 0;
    mtp = AVT_POINT_MESH;
    break;
      }

      avtMeshMetaData *mmd = new avtMeshMetaData;
      mmd->name = getTypeName( *m );
      mmd->spatialDimension = 3;
      mmd->topologicalDimension = ntd;
      mmd->meshType = mtp;
      mmd->numBlocks = 1;
      md->Add(mmd);
    }}

  //
  // variables
  //
  for( set<int>::const_iterator m=meshtypes.begin(); m!=meshtypes.end(); m++ ) {
    int num = reader_->GetMeshSize( *m );
    if( num>0 ) {
      string meshname = getTypeName( *m );
      debug2 << "Searching meshname: " << meshname << "\n";

      char name[1024];
      int ndim=10;
      hsize_t dims[10];
      ierr = reader_->GetDataSetInfo( 0, *m, NULL, 0, NULL, NULL );

      while( ierr==0 ) {
    ndim = 10;
    ierr = reader_->GetDataSetInfo( 1, *m, name, 1024, dims, &ndim );
    if( ierr!=0 ) break;
    debug2 << "Find Dataset: name=" << name << "  ndim=" << ndim << "\n";

    if( ierr>0 ) break;
    if( ierr<0 ) continue;
    //if( strcmp( name, "Index" )==0 ) continue;
    if( strcmp( name, "Nodes" )==0 ) continue;
    if( strcmp( name, "Coordinate" )==0 ) continue;
    //if( strcmp( name, "Material" )==0 ) continue;

    avtCentering  nzc;
    switch( *m ) {
    case VelodyneReader::solid_type:
    case VelodyneReader::shell_type:
    case VelodyneReader::surface_type:
      nzc = AVT_ZONECENT;
      break;
    case VelodyneReader::node_type:
    case VelodyneReader::particle_type:
    case VelodyneReader::sph_type:
      nzc = AVT_NODECENT;
      break;
    }

    if( ndim==1 ) {
      if( strcmp( name, "NumberOfHistoryVariables" )!=0 && 
          strcmp( name, "HistoryVariable" )!=0 ) {
      avtScalarMetaData *smd = new avtScalarMetaData;
      smd->name = composeName( meshname, name);
      smd->meshName = meshname;
      smd->centering = nzc;
      smd->hasUnits = false;
      md->Add(smd);
      }
    }
    else if( ndim==2 ) {
      if( dims[1]<=3 ) {
        avtVectorMetaData *vmd = new avtVectorMetaData;
        vmd->name = composeName( meshname, name);
        vmd->meshName = meshname;
        vmd->centering = nzc;
        vmd->hasUnits = false;
        // ??? is it necessary
        //vmd->varDim = (int)dims[1];
        md->Add(vmd);

        for( int d=0;d<dims[1];d++ ) {
          char buf[100];
          sprintf(buf,"%d",d);
          Expression *exp= new Expression;
          exp->SetName( composeName(meshname,name)+"_"+buf );
          exp->SetDefinition( "array_decompose(<"+composeName(meshname,name)+">,"+buf+")");
          exp->SetType( Expression::ScalarMeshVar );
          exp->SetHidden( false );
          md->AddExpression(exp);
        }
      }
      else {
        avtTensorMetaData *tensor = new avtTensorMetaData();
        tensor->name = composeName( meshname, name);
        tensor->meshName = meshname;
        tensor->centering = nzc;
        tensor->hasUnits = false;
        //tensor->dim = 9; //(int)dims[1];
        md->Add(tensor);

        if( strcmp(name,"Stress")==0 ) {
          Expression *pre= new Expression;
          pre->SetName( composeName( meshname,"Pressure" ) );
          pre->SetDefinition( "-0.33333*trace(<" + composeName(meshname,name) + ">)" );
          pre->SetType( Expression::ScalarMeshVar );
          pre->SetHidden( false );
          md->AddExpression(pre);

          Expression *pst= new Expression;
          pst->SetName( composeName( meshname,"Principal_stress" ) );
          pst->SetDefinition( "principal_tensor(<" + composeName(meshname,name) + ">)" );
          pst->SetType( Expression::VectorMeshVar );
          pst->SetHidden( false );
          md->AddExpression(pst);

          Expression *vms= new Expression;
          vms->SetName( composeName( meshname,"von_Mises_Criterion" ) );
          vms->SetDefinition( "sqrt( 0.5*( (<" + 
                  composeName(meshname,"Principal_stress") + ">[0]-<" +
                  composeName(meshname,"Principal_stress") + ">[1])^2 + (<" +
                  composeName(meshname,"Principal_stress") + ">[1]-<" +
                  composeName(meshname,"Principal_stress") + ">[2])^2 + (<" +
                  composeName(meshname,"Principal_stress") + ">[2]-<" +
                  composeName(meshname,"Principal_stress") + ">[0])^2 ))");
          vms->SetType( Expression::ScalarMeshVar );
          vms->SetHidden( false );
          md->AddExpression(vms);
        }
      }
    }
    else {
      debug1 << "Only scalar and vector are supported.\n"
         << "Dataset " << name << " has number of dimensions "  << ndim  << ".\n";
    }
      }
    }}


  //
  // node-based variables 
  //
  if( reader_->GetMeshSize( VelodyneReader::node_type )>0 ) {
    char name[1024];
    hsize_t dims[10];
    int ndim=10;
    ierr = reader_->GetDataSetInfo( 0, VelodyneReader::node_type, NULL, 0, NULL, NULL );
    while( ierr==0 ) {
      ndim = 10;
      ierr = reader_->GetDataSetInfo( 1, VelodyneReader::node_type, name, 1024, dims, &ndim );
      if( ierr!=0 ) break;
      if( strcmp( name, "Index" )==0 ) continue; 
      if( strcmp( name, "Coordinate" )==0 ) continue;

      nvname_.push_back(name);
      nvdata_.push_back(NULL);
      nnvs_++;
      
      if( ndim==1 ) {
    if( reader_->GetMeshSize( VelodyneReader::solid_type )>0 ) {
      avtScalarMetaData *smd = new avtScalarMetaData;
      smd->name = composeName( solid_name, name);
      smd->meshName = solid_name;
      smd->centering = AVT_NODECENT;
      smd->hasUnits = false;
      md->Add(smd);
    }
    if( reader_->GetMeshSize( VelodyneReader::shell_type )>0 ) {
      avtScalarMetaData *smd = new avtScalarMetaData;
      smd->name = composeName( shell_name, name);
      smd->meshName = shell_name;
      smd->centering = AVT_NODECENT;
      smd->hasUnits = false;
      md->Add(smd);
    }
    if( reader_->GetMeshSize( VelodyneReader::surface_type )>0 ) {
      avtScalarMetaData *smd = new avtScalarMetaData;
      smd->name = composeName( surface_name, name);
      smd->meshName = surface_name;
      smd->centering = AVT_NODECENT;
      smd->hasUnits = false;
      md->Add(smd);
    }
      }
      else if( ndim==2 ) {
    if( dims[1]<=3 ) {
      if( reader_->GetMeshSize( VelodyneReader::solid_type )>0 ) {
        avtVectorMetaData *vmd = new avtVectorMetaData;
        vmd->name = composeName( solid_name, name);
        vmd->meshName = solid_name;
        vmd->centering = AVT_NODECENT;
        vmd->hasUnits = false;
        vmd->varDim = (int)dims[1];
        md->Add(vmd);
        for( int d=0;d<dims[1];d++ ) {
          char buf[100];
          sprintf(buf,"%d",d);
          Expression *exp= new Expression;
          exp->SetName( composeName(solid_name,name)+"_"+buf );
          exp->SetDefinition( "array_decompose(<"+composeName(solid_name,name)+">,"+buf+")");
          exp->SetType( Expression::ScalarMeshVar );
          exp->SetHidden( false );
          md->AddExpression(exp);
        }
      }
      if( reader_->GetMeshSize( VelodyneReader::shell_type )>0 ) {
        avtVectorMetaData *vmd = new avtVectorMetaData;
        vmd->name = composeName( shell_name, name);
        vmd->meshName = shell_name;
        vmd->centering = AVT_NODECENT;
        vmd->hasUnits = false;
        vmd->varDim = (int)dims[1];
        md->Add(vmd);
        for( int d=0;d<dims[1];d++ ) {
          char buf[100];
          sprintf(buf,"%d",d);
          Expression *exp= new Expression;
          exp->SetName( composeName(shell_name,name)+"_"+buf );
          exp->SetDefinition( "array_decompose(<"+composeName(shell_name,name)+">,"+buf+")");
          exp->SetType( Expression::ScalarMeshVar );
          exp->SetHidden( false );
          md->AddExpression(exp);
        }
      }
      if( reader_->GetMeshSize( VelodyneReader::surface_type )>0 ) {
        avtVectorMetaData *vmd = new avtVectorMetaData;
        vmd->name = composeName( surface_name, name);
        vmd->meshName = surface_name;
        vmd->centering = AVT_NODECENT;
        vmd->hasUnits = false;
        vmd->varDim = (int)dims[1];
        md->Add(vmd);
        for( int d=0;d<dims[1];d++ ) {
          char buf[100];
          sprintf(buf,"%d",d);
          Expression *exp= new Expression;
          exp->SetName( composeName(surface_name,name)+"_"+buf );
          exp->SetDefinition( "array_decompose(<"+composeName(surface_name,name)+">,"+buf+")");
          exp->SetType( Expression::ScalarMeshVar );
          exp->SetHidden( false );
          md->AddExpression(exp);
        }
      }
    }
    else {
      if( reader_->GetMeshSize( VelodyneReader::solid_type )>0 ) {
        avtTensorMetaData *tensor = new avtTensorMetaData();
        tensor->name = composeName( solid_name, name);
        tensor->meshName = solid_name;
        tensor->centering = AVT_NODECENT;
        tensor->hasUnits = false;
        tensor->dim = (int)dims[1];
        md->Add(tensor);
      }
      if( reader_->GetMeshSize( VelodyneReader::shell_type )>0 ) {
        avtTensorMetaData *tensor = new avtTensorMetaData();
        tensor->name = composeName( shell_name, name);
        tensor->meshName = shell_name;
        tensor->centering = AVT_NODECENT;
        tensor->hasUnits = false;
        tensor->dim = (int)dims[1];
        md->Add(tensor);
      }
      if( reader_->GetMeshSize( VelodyneReader::surface_type )>0 ) {
        avtTensorMetaData *tensor = new avtTensorMetaData();
        tensor->name = composeName( surface_name, name);
        tensor->meshName = surface_name;
        tensor->centering = AVT_NODECENT;
        tensor->hasUnits = false;
        tensor->dim = (int)dims[1];
        md->Add(tensor);
      }}
      }
      else {
    debug1 << "Unacceptable dataset(" << name << ") is found.\n"
           << "ndim = " << ndim << "dims= " << dims[0] << ", " << dims[1] << " ]\n";
      }
    }}

  //
  // history variables
  //
  for( set<int>::const_iterator m=meshtypes.begin(); m!=meshtypes.end(); m++ ) {
    if( *m != VelodyneReader::solid_type  && 
    *m != VelodyneReader::shell_type  && 
    *m != VelodyneReader::sph_type ) continue;

    int num = reader_->GetHVsize( *m );
    if( num==0 ) continue;

    string meshname = getTypeName( *m );
    char name[1024];
    for( int v=0; v<num; v++ ) {
      sprintf(name,"HV_%d",v+1);
      avtScalarMetaData *smd = new avtScalarMetaData;
      smd->name = composeName( meshname, name);
      smd->meshName = meshname;
      smd->centering = AVT_ZONECENT;;
      smd->hasUnits = false;
      md->Add(smd);
    }
  }

  //
  // material
  //
  for( set<int>::const_iterator m=meshtypes.begin(); m!=meshtypes.end(); m++ ) {
    if( *m == VelodyneReader::node_type ) continue;
    if( *m == VelodyneReader::tiednode_type ) continue;

    int num = reader_->GetMeshSize( *m );
    if( num>0 ) {
      string meshname = getTypeName( *m );

      ierr = reader_->readMeshMaterialSet( *m );
      if( ierr<0 ) {
    EXCEPTION1( InvalidVariableException, 
            "Failed to read in mesh Material Info."+meshname );
      }
      const set<int>& mset = reader_->GetMeshMatInfo( *m );

      avtMaterialMetaData *matmd = new avtMaterialMetaData;
      matmd->name = composeName( meshname, "mat", '_' );
      matmd->meshName = meshname;
      matmd->numMaterials = mset.size();
      char buf[1000];
      for( set<int>::const_iterator i=mset.begin(); i!=mset.end(); i++ ) {
    const string& mt = reader_->GetMaterialTitle(*i);
    strcpy( buf, mt.c_str() );
    matmd->materialNames.push_back(buf);
      }
      md->Add(matmd);
    }
  }
}


// ****************************************************************************
//  Method: avtVelodyneFileFormat::GetMesh
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
//  Programmer: hpan -- generated by xml2avt
//  Creation:   Thu Aug 7 11:38:59 PDT 2008
//
// ****************************************************************************

vtkDataSet *
avtVelodyneFileFormat::GetMesh(const char *meshname)
{
  int ierr;
  string mname = meshname;
  debug2 << "Trying to GetMesh(\"" << meshname << "\")...\n";

  int meshtype = getTypeId( meshname );
  if( meshtype<0 ) {
      EXCEPTION1( InvalidVariableException, 
          "Failed to identify mesh type.\n" );
  }

  int meshsize = reader_->GetMeshSize( meshtype );

  // unstructured mesh
  if( meshtype == VelodyneReader::solid_type ||
      meshtype == VelodyneReader::shell_type ||
      meshtype == VelodyneReader::surface_type ||
      meshtype == VelodyneReader::tiednode_type ) {
    ierr = readNodeIndex();
    if( ierr!=0 ) {
      EXCEPTION1( InvalidVariableException, 
          "Failed to read in Node Index.\n" );
    }
    ierr = readNodeCoord();
    if( ierr!=0 ) {
      EXCEPTION1( InvalidVariableException, 
          "Failed to read in Node coordinates.\n" );
    }

    int nnd; // = meshtype==VelodyneReader::solid_type ? 8 : 4;
    int ctype; //  = meshtype==VelodyneReader::solid_type ? VTK_HEXAHEDRON : VTK_QUAD;
    switch( meshtype ) {
    case VelodyneReader::solid_type:
      nnd=8;      ctype=VTK_HEXAHEDRON;
      break;
    case VelodyneReader::shell_type:
    case VelodyneReader::surface_type:
      nnd=4;      ctype=VTK_QUAD;
      break;
    case VelodyneReader::tiednode_type:
      //      nnd=2;      ctype=VTK_LINE;
      nnd=1;      ctype=VTK_VERTEX;      meshsize*=2;
      break;
    }

    int *elmt = new int[ meshsize*nnd ];
    ierr = readdElements( meshtype, meshsize*nnd, elmt );
    if( ierr!=0 ) {
      EXCEPTION1( InvalidVariableException, 
          "Failed to read in elements of "+mname );
    }

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New(); 
    ugrid->SetPoints( crd_ );
    ugrid->Allocate(meshsize);

    vtkIdType verts[8];
    int* cnt=elmt;
    for( int i=0; i<meshsize; i++ ) {
      for( int j=0; j<nnd; j++ ) 
    verts[j] = *cnt++;
      ugrid->InsertNextCell( ctype, nnd, verts );
    }
    delete [] elmt;
    return ugrid;
  }
  // particle mesh
  else if( meshtype == VelodyneReader::particle_type ||
       meshtype == VelodyneReader::sph_type ||
       meshtype == VelodyneReader::node_type ) {
    vtkPoints *crd = vtkPoints::New();
    crd->SetNumberOfPoints(meshsize);
    float *pts = (float *) crd->GetVoidPointer(0);

    ierr = reader_->readMeshFltArray( meshtype, "Coordinate", meshsize*3, pts );
    if( ierr!=0 ) {
      crd->Delete();
      EXCEPTION1( InvalidVariableException, 
          "Failed to read in coordinates of "+mname );
    }

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New(); 
    ugrid->SetPoints( crd );
    crd->Delete();
    ugrid->Allocate(meshsize);
    vtkIdType vertex;
    for( int i=0; i<meshsize; i++ ) {
      vertex = i;
      ugrid->InsertNextCell(VTK_VERTEX, 1, &vertex);
    }
    return ugrid;
  }

  debug1 << "Unknown mesh name: " << meshname << ".\n";
  return NULL;
}


// ****************************************************************************
//  Method: avtVelodyneFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: hpan -- generated by xml2avt
//  Creation:   Thu Aug 7 11:38:59 PDT 2008
//
// ****************************************************************************

vtkDataArray *
avtVelodyneFileFormat::GetVar(const char *name)
{
  int ierr;
  string mesh,varname;
  decomposeName( name, mesh, varname );
  debug2 << "geting variable " << varname << " on mesh " << mesh << "...\n";

  ierr = readNodeBaseVariableNames();

  if( mesh != particle_name && mesh != sph_name ) {
  int ind = isNodeBasedVariable( varname );
  if( ind>=0 && ind<nnvs_ ) {
    if( nvdata_[ind] == NULL ) {
      int num = reader_->GetMeshSize( VelodyneReader::node_type );

      vtkFloatArray* var = vtkFloatArray::New();
      var->SetNumberOfTuples( num );
      float *val = (float *)var->GetVoidPointer(0);

      ierr = reader_->readMeshFltArray( VelodyneReader::node_type,
                    varname.c_str(), num, val );
      if( ierr!=0 ) {
    EXCEPTION1( InvalidVariableException, 
            "Failed to read in node-based variable "+varname );
      }
      nvdata_[ind] = var;
      pobj_->Register( var );
    }
    return nvdata_[ind];
  }}

  int meshtype = getTypeId( mesh.c_str() );
  if( meshtype<0 ) {
    EXCEPTION1( InvalidVariableException, 
        "Failed to recognize mesh "+mesh );
  }
  int meshsize = reader_->GetMeshSize( meshtype );
  if( meshsize<=0 ) {
    EXCEPTION1( InvalidVariableException, 
        "Found invalid mesh "+mesh );
  }

  vtkFloatArray* var = vtkFloatArray::New();
  var->SetNumberOfTuples( meshsize );
  float *val = (float *)var->GetVoidPointer(0);

  ierr = reader_->readMeshFltArray( meshtype, varname.c_str(), 
                    meshsize, val );
  if( ierr!=0 ) {
    EXCEPTION1( InvalidVariableException, 
        "Failed to read in variable "+composeName(mesh,varname) );
  }
  return var;
}


// ****************************************************************************
//  Method: avtVelodyneFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: hpan -- generated by xml2avt
//  Creation:   Thu Aug 7 11:38:59 PDT 2008
//
// ****************************************************************************
//
// not support tensor yet, VisIt calls this function to read in both vector and tensor 
//
vtkDataArray *
avtVelodyneFileFormat::GetVectorVar(const char *name)
{
  int ierr;
  int ndim,dims[10];

  string mesh,varname;
  decomposeName( name, mesh, varname );
  debug2 << "geting vector/tensor variable " << varname << " on mesh " << mesh << "...\n";

  ierr = readNodeBaseVariableNames();

  // node-based variables
  if( mesh != particle_name && mesh != sph_name ) {
  int ind = isNodeBasedVariable( varname );
  if( ind>=0 && ind<nnvs_ ) {
    if( nvdata_[ind] == NULL ) {
      int num = reader_->GetMeshSize( VelodyneReader::node_type );
      int isTensor = isTensorVariable( VelodyneReader::node_type, varname.c_str() );
      int ncmp = isTensor ? 9 : 3;

      vtkFloatArray* var = vtkFloatArray::New();
      var->SetNumberOfComponents(ncmp);
      var->SetNumberOfTuples( num );
      float *val = (float *)var->GetVoidPointer(0);

      ierr = reader_->readMeshFltArray( VelodyneReader::node_type,
                    varname.c_str(), num*ncmp, val,
                    &ndim, dims );
      if( ierr!=0 ) {
    EXCEPTION1( InvalidVariableException, 
            "Failed to read in node-based vector variable "+varname );
      }
      if( isTensor ) 
    switch(dims[1]) {
    case 9: break; // full tensor
    case 6: convertSymTensorToFullTensor( num, val ); break; // symmetric tensor
    default:
      EXCEPTION1( InvalidVariableException, 
              "Unknow tensor format for "+composeName(mesh,varname) );
    }
      else 
    switch(dims[1]) {
    case 3: break;
    case 2:    convert2dVectorTo3dVector( num, val );    break;
    case 1:    convert1dVectorTo3dVector( num, val );  break;
    default:
      EXCEPTION1( InvalidVariableException, 
              "Unknow vector format for "+composeName(mesh,varname) );
    }
      nvdata_[ind] = var;
      pobj_->Register( var );
    }
    return nvdata_[ind];
  }}

  // mesh/particle based variables
  int meshtype = getTypeId( mesh.c_str() );
  if( meshtype<0 ) {
    EXCEPTION1( InvalidVariableException, 
        "Failed to recognize mesh "+mesh );
  }
  int meshsize = reader_->GetMeshSize( meshtype );
  if( meshsize<=0 ) {
    EXCEPTION1( InvalidVariableException, 
        "Found invalid mesh "+mesh );
  }
  int isTensor = isTensorVariable( meshtype, varname.c_str() );
  int ncmp = isTensor ? 9 : 3;

  vtkFloatArray* var = vtkFloatArray::New();
  var->SetNumberOfComponents(ncmp);
  var->SetNumberOfTuples( meshsize );
  float *val = (float *)var->GetVoidPointer(0);

  ierr = reader_->readMeshFltArray( meshtype, varname.c_str(), 
                    meshsize*ncmp, val,
                    &ndim, dims );
  if( ierr!=0 ) {
    EXCEPTION1( InvalidVariableException, 
        "Failed to read in vector variable "+composeName(mesh,varname) );
  }
  if( isTensor )
    switch(dims[1]) {
    case 9: break; // full tensor
    case 6: convertSymTensorToFullTensor( meshsize, val ); break; // symmetric tensor
    default:
      EXCEPTION1( InvalidVariableException, 
          "Unknow tensor format for "+composeName(mesh,varname) );
    }
  else 
    switch(dims[1]) {
    case 3: break;
    case 2: convert2dVectorTo3dVector( meshsize, val );  break;
    case 1: convert1dVectorTo3dVector( meshsize, val );  break;
    default:
      EXCEPTION1( InvalidVariableException, 
          "Unknow vector format for "+composeName(mesh,varname) );
    }
  return var;
}




void* avtVelodyneFileFormat::
GetAuxiliaryData(const char *var, const char *type,
         void *args, DestructorFunction &df )
{
  int ierr;
  void* retval=NULL;

  if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0) {
    debug2 << "GetAuxiliaryData() : var=" << var << " type=" << type << ".\n";

    string mesh,varname;
    decomposeName( var, mesh, varname );
    int grpId = getTypeId( mesh.c_str() );
    if( grpId<0 ) {
      EXCEPTION1( InvalidVariableException, 
          "Failed to find mehs type id"+mesh );
    }
    ierr = reader_->readMeshMaterialSet( grpId );
    if( ierr!=0 ) {
      EXCEPTION1( InvalidVariableException, 
          "Failed to find material for mesh "+mesh );
    }
    const set<int>& mset = reader_->GetMeshMatInfo( grpId );
    int  nmats = mset.size();
    int* matnos= new int[nmats];
    int m=0;
    for( set<int>::const_iterator i=mset.begin(); i!=mset.end(); i++ ) 
      matnos[m++] = *i;

    char** mnames= new char*[nmats];
    char buf[1000];
    for( int i=0; i<nmats; i++ ) {
      const string& mt = reader_->GetMaterialTitle(matnos[i]);
      mnames[i] = new char[ mt.length()+1 ];
      strcpy( mnames[i], mt.c_str() );
    }

    int num = reader_->GetMeshSize( grpId );
    int*mvl = new int[num];

    ierr = reader_->readMeshIntArray( grpId, "Material", num, mvl );
    if( ierr<0 ) {
      EXCEPTION1( InvalidVariableException, 
          "Failed to read in material for mesh "+mesh );
    }

    int ndims=1;
    int dims[3]; dims[0]=num;
    avtMaterial *mat = new avtMaterial(
        nmats,
            matnos,
            mnames,
            ndims,
            dims,
            0,
            mvl,
            0, // length of mix arrays
            0, // mix_mat array
            0, // mix_next array
            0, // mix_zone array
            0  // mix_vf array
            );


    delete [] mvl;
    for( int i=0; i<nmats; i++ ) 
      delete [] mnames[i];
    delete [] mnames;
    delete [] matnos;

    retval = mat;
    df = avtMaterial::Destruct;
  }
  return retval;
}






int avtVelodyneFileFormat::
readNodeIndex()
{
  debug2 << "begin to read in node Index...\n";
  if( map_!=NULL ) return 0;

  int nnode = reader_->GetMeshSize( VelodyneReader::node_type );
  if( nnode==0 ) return 0;

  int *idx = new int[nnode];
  int ierr = reader_->readMeshIntArray( VelodyneReader::node_type, "Index",
                    nnode, idx );
  if( ierr!=0 ) return ierr;

  // find node index range 
  idx_mn_ = idx_mx_ = idx[0];
  for( int i=1; i<nnode; i++ ) {
    if( idx_mn_>idx[i] )      idx_mn_ = idx[i];
    if( idx_mx_<idx[i] )      idx_mx_ = idx[i];
  }

  // create node index mapping
  int lmap = idx_mx_ - idx_mn_ + 1;
  map_ = new int[lmap];
  if( map_==NULL ) {
    debug1 << "Failed to allocate node index map[" << lmap << "].\n";
    return -10;
  }

  for( int i=0; i<lmap; i++ )   map_[i]=-1;
  for( int i=0; i<nnode; i++ )  map_[ idx[i]-idx_mn_ ] = i;
  delete [] idx;
  return 0;
}



int avtVelodyneFileFormat::
readNodeCoord()
{
  debug2 << "begin to read in node coordinates...\n";
  if( crd_!=NULL ) return 0;

  int nnode = reader_->GetMeshSize( VelodyneReader::node_type );
  if( nnode==0 ) return 0;

  crd_ = vtkPoints::New();
  crd_->SetNumberOfPoints(nnode);
  float *pts = (float *) crd_->GetVoidPointer(0);

  return reader_->readMeshFltArray( VelodyneReader::node_type, "Coordinate",
                    nnode*3, pts );
}




int avtVelodyneFileFormat::
readdElements( int grp, int bufsz, int* elmt )
{
  debug2 << "begin to read in " << grp << "th mesh elements...\n";
  int ierr = reader_->readMeshIntArray( grp, "Nodes",
                    bufsz, elmt );
  for( int i=0; i<bufsz; i++ ) {
    int gi = elmt[i];
    if( gi<idx_mn_ || gi>idx_mx_ ) {
      debug1 << "Node index " << gi << " is outside index range [" << idx_mn_ << ", " << idx_mx_ << "].\n";
      return -1;
    }

    elmt[i]= map_[ gi-idx_mn_ ];
    if( elmt[i]<0 ) {
      debug1 << "Invalid node index occurs. Node index " << gi << " is not in given node list.\n";
      return -2;
    }
  }
  return 0;
}




int avtVelodyneFileFormat::
isNodeBasedVariable( const std::string& name )
{
  int i;
  if( nnvs_==0 ) return -1;

  for( i=0; i<nnvs_; i++ ) 
    if( nvname_[i] == name )
      return i;
  return -2;
}


// int avtVelodyneFileFormat::
// isVectorVariable( int grp, const char* varname )
// {
//   int ierr;
//   int ndim;
//   ierr = reader_->readMeshFltArray( grp, varname, 0, NULL, &ndim, NULL );
//   if( ierr<0 ) {
//     debug1 << "Failed to inquire type of variable " << varname << ".\n";
//     return 0;
//   }
//   if( ndim==2 )
//     return 1;
//   else
//     return 0;
// }


int avtVelodyneFileFormat::
isTensorVariable( int grp, const char* varname )
{
  int ierr;
  int ndim,dims[100];
  ierr = reader_->readMeshFltArray( grp, varname, 0, NULL, &ndim, dims );
  if( ierr<0 ) {
    debug1 << "Failed to inquire type of variable " << varname << ".\n";
    return 0;
  }
  if( ndim==2 && dims[1]>3 )
    return 1;
  else
    return 0;
}




void avtVelodyneFileFormat::
convert2dVectorTo3dVector( int num, float* val )
{
  for( int i=num-1; i>=0; i-- ) {
    val[ i*3+2 ] = 0.0;
    val[ i*3+1 ] = val[i*2+1 ];
    val[ i*3+0 ] = val[i*2+0 ];
  }
}


void avtVelodyneFileFormat::
convert1dVectorTo3dVector( int num, float* val )
{
  for( int i=num-1; i>=0; i-- ) {
    val[ i*3+2 ] = 0.0;
    val[ i*3+1 ] = 0.0;
    val[ i*3+0 ] = val[i];
  }
}



// sym tensor (xx,yy,zz,xy,yz,zx)
void avtVelodyneFileFormat::
convertSymTensorToFullTensor( int num, float* val )
{
  float c11,c22,c33,c23,c13,c12;
  for( int i=num-1; i>=0; i-- ) {
    int i6=i*6;
    c11 = val[ i6+0 ];
    c22 = val[ i6+1 ];
    c33 = val[ i6+2 ];
    c23 = val[ i6+3 ];
    c13 = val[ i6+4 ];
    c12 = val[ i6+5 ];

    int i9=i*9;
    val[ i9+0 ] = c11;
    val[ i9+1 ] = c12;
    val[ i9+2 ] = c13;

    val[ i9+3 ] = c12;
    val[ i9+4 ] = c22;
    val[ i9+5 ] = c23;

    val[ i9+6 ] = c13;
    val[ i9+7 ] = c23;
    val[ i9+8 ] = c33;
  }
}


int avtVelodyneFileFormat::
readNodeBaseVariableNames()
{
  char name[1024];
  hsize_t dims[10];
  int ndim=10;

  if( nnvs_>0 ) return 0;

  int ierr = reader_->GetDataSetInfo( 0, VelodyneReader::node_type, NULL, 0, NULL, NULL );
  while( ierr==0 ) {
    ndim = 10;
    ierr = reader_->GetDataSetInfo( 1, VelodyneReader::node_type, name, 1024, dims, &ndim );
    if( strcmp( name, "Index" )==0 ) continue; 
    if( strcmp( name, "Coordinate" )==0 ) continue;
    nvname_.push_back(name);
    nvdata_.push_back(NULL);
    nnvs_++;
  }
  return 0;
}


