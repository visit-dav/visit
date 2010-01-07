/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

// ************************************************************************* //
//                            avtGadgetFileFormat.C                          //
// ************************************************************************* //

#include <avtGadgetFileFormat.h>

#include <string>
#include <vtkVertex.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include <byteswap.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>
#include <avtIntervalTree.h>


//#define MY_DEBUG


#define SKIP  {if(my_fread(&blksize,sizeof(int),1,fd)==0)return -1; swap_Nbyte((char*)&blksize,1,4);}

/*---------------------- Basic routine to read data from a file ---------------*/
size_t avtGadgetFileFormat::my_fread(void *ptr, size_t size, size_t nmemb, FILE * stream)
{
  size_t nread;

  if((nread = fread(ptr, size, nmemb, stream)) != nmemb)
    {
      if(!feof(stream))
        fprintf(stderr,"I/O error (fread) !\n");
      return 0;
      //      exit(3);
    }
  return nread;
}

/*-----------------------------------------------------------------------------*/
/*---------------------- Routine to swap ENDIAN -------------------------------*/
/*-------- char *data:    Pointer to the data ---------------------------------*/
/*-------- int n:         Number of elements to swap --------------------------*/
/*-------- int m:         Size of single element to swap ----------------------*/
/*--------                int,float = 4 ---------------------------------------*/
/*--------                double    = 8 ---------------------------------------*/
/*-----------------------------------------------------------------------------*/
void avtGadgetFileFormat::swap_Nbyte(char *data,int n,int m)
{
  int i,j;
  char old_data[16];

  if(swap>0)
    {
      for(j=0;j<n;j++)
        {
          memcpy(&old_data[0],&data[j*m],m);
          for(i=0;i<m;i++)
            {
              data[j*m+i]=old_data[m-i-1];
            }
        }
    }
}


using     std::string;

// Modifications
//   Jeremy Meredith, Thu Jan  7 12:08:10 EST 2010
//   Throw exception with obviously incorrect format.
//
int avtGadgetFileFormat::get_block_names(FILE *fd, char **labels, int *vflag, int *numblocks)
{
  rewind(fd);
  *numblocks=0;
  int4bytes blocksize=0;
  while(blocksize == 0)
    {
       SKIP;
       if(blksize == 134217728)
         {
#ifdef MY_DEBUG
           printf("Enable ENDIAN swapping !\n");
#endif
           swap=1-swap;
           swap_Nbyte((char*)&blksize,1,4);
         }
       if(blksize != 8)
         {

           printf("incorrect format (blksize=%d)!\n",blksize);
           EXCEPTION1(InvalidFilesException, "unknown");
           //exit(1);
         }
       else
         {
           labels[*numblocks]=new char[5];
           if( my_fread(labels[*numblocks], 4*sizeof(char), 1, fd)==0 ) return(-1);
           labels[*numblocks][4]='\0';
           if( my_fread(&blocksize, sizeof(int4bytes), 1, fd)==0 ) return(-1);
           swap_Nbyte((char*)&blocksize,1,4);
#ifdef MY_DEBUG
           printf("get names: Found Block <%s> with %d bytes\n",labels[*numblocks],blocksize);
#endif
           SKIP;
           if(blocksize/(sizeof(float)*ntot)==3)
             vflag[*numblocks]=1;
           else
             vflag[*numblocks]=0;
#ifdef MY_DEBUG
           fprintf(stderr,"vector %d   %d   %d   %d\n",vflag[*numblocks],blocksize,ntot,blocksize/(sizeof(float)*ntot));
#endif
           fseek(fd,blocksize,SEEK_CUR);
           (*numblocks)++;
           blocksize=0;

         }
    }
  return(blocksize-8);
}


/*-----------------------------------------------------------------------------*/
/*---------------------- Routine find a block in a snapfile -------------------*/
/*-------- FILE *fd:      File handle -----------------------------------------*/
/*-------- char *label:   4 byte identifyer for block -------------------------*/
/*-------- returns length of block found, -------------------------------------*/
/*-------- the file fd points to starting point of block ----------------------*/
/*-----------------------------------------------------------------------------*/
// Modifications
//   Jeremy Meredith, Thu Jan  7 12:08:10 EST 2010
//   Throw exception with obviously incorrect format.
//
int avtGadgetFileFormat::find_block(FILE *fd,const char *label)
{
  int4bytes blocksize=0;
  char blocklabel[5]={"    "};
#ifdef MY_DEBUG
           printf("Searching <%s>\n",label);
#endif
 
  rewind(fd);

  while(!feof(fd) && blocksize == 0)
    {
       SKIP;
       if(blksize == 134217728)
         {
#ifdef MY_DEBUG
           printf("Enable ENDIAN swapping !\n");
#endif
           swap=1-swap;
           swap_Nbyte((char*)&blksize,1,4);
         }
       if(blksize != 8)
         {
           printf("incorrect format (blksize=%d)!\n",blksize);
           EXCEPTION1(InvalidFilesException, "unknown");
           //exit(1);
         }
       else
         {
           my_fread(blocklabel, 4*sizeof(char), 1, fd);
           my_fread(&blocksize, sizeof(int4bytes), 1, fd);
           swap_Nbyte((char*)&blocksize,1,4);
#ifdef MY_DEBUG
           printf("Found Block <%s> with %d bytes\n",blocklabel,blocksize);
#endif
           SKIP;
           if(strcmp(label,blocklabel)!=0)
             { 
                fseek(fd,blocksize,1);
                blocksize=0;
             }
         }
    }
  return(blocksize-8);
}


/*-----------------------------------------------------------------------------*/
/*---------------------- Routine to read a 3D float array ---------------------*/
/*-------- int *data:     Pointer where the data are stored to ----------------*/
/*-------- char *label:   Identifyer for the datafield to read ----------------*/
/*-------- FILE *fd:      File handle -----------------------------------------*/
/*-------- returns length of dataarray ----------------------------------------*/
/*-----------------------------------------------------------------------------*/
int avtGadgetFileFormat::read_gadget_float3(float *data,const char *label,FILE *fd)
{
  int blocksize = find_block(fd,label);
  if(blocksize <= 0)
    {
      printf("Block <%s> not fond !\n",label);
      //exit(5);
    }
  else
    {
#ifdef MY_DEBUG
       printf("Reding %d bytes of data from <%s>...\n",blocksize,label);
#endif
       SKIP;
       my_fread(data,blocksize, 1, fd);
       swap_Nbyte((char*)data,blocksize/sizeof(float),4);
       SKIP;
    }
  return(blocksize/sizeof(float)/3);
}

/*-----------------------------------------------------------------------------*/
/*---------------------- Routine to read a 1D float array ---------------------*/
/*-------- int *data:     Pointer where the data are stored to ----------------*/
/*-------- char *label:   Identifyer for the datafield to read ----------------*/
/*-------- FILE *fd:      File handle -----------------------------------------*/
/*-------- returns length of dataarray ----------------------------------------*/
/*-----------------------------------------------------------------------------*/
int avtGadgetFileFormat::read_gadget_float(float *data,const char *label,FILE *fd)
{
  int blocksize;

  blocksize = find_block(fd,label);
  if(blocksize <= 0)
    {
      printf("Block <%s> not fond !\n",label);
      //      exit(5);
    }
  else
    {
#ifdef MY_DEBUG
       printf("Reading %d bytes of data from <%s>...\n",blocksize,label);
#endif
       SKIP;
       my_fread(data,blocksize, 1, fd);
       swap_Nbyte((char*)data,blocksize/sizeof(float),4);
       SKIP;
    }
  return(blocksize/sizeof(float));
}




/*-----------------------------------------------------------------------------*/
/*---------------------- Routine to read the header information ---------------*/
/*-------- int *npart:    List of Particle numbers for spezies [0..5] ---------*/
/*-------- int *massarr:  List of masses for spezies [0..5] -------------------*/
/*-------- int *time:     Time of snapshot ------------------------------------*/
/*-------- int *massarr:  Redshift of snapshot --------------------------------*/
/*-------- FILE *fd:      File handle -----------------------------------------*/
/*-------- returns number of read bytes ---------------------------------------*/
/*-----------------------------------------------------------------------------*/
int avtGadgetFileFormat::read_gadget_head(int *npart,double *massarr,double *time,double *redshift,FILE *fd)
{
  int blocksize = find_block(fd,"HEAD");
  if(blocksize <= 0)
    {
      printf("Block <%s> not fond !\n","HEAD");
      //exit(5);
    }
  else
    {
       int dummysize=blocksize - 6 * sizeof(int) - 8 * sizeof(double);
       SKIP;
       my_fread(npart,6*sizeof(int), 1, fd);        swap_Nbyte((char*)npart,6,4);
       my_fread(massarr,6*sizeof(double), 1, fd);   swap_Nbyte((char*)massarr,6,8);
       my_fread(time,sizeof(double), 1, fd);        swap_Nbyte((char*)time,1,8);
       my_fread(redshift,sizeof(double), 1, fd);    swap_Nbyte((char*)redshift,1,8);
       fseek(fd,dummysize,1);
       SKIP;
    }
#ifdef MY_DEBUG
  fprintf(stderr,"done reading so far\n");
#endif
  return(blocksize);
}



// ****************************************************************************
//  Method: avtGadgetFileFormat constructor
//
//  Programmer: Ralph Bruckschen
//  Creation:   Wed Oct 22 12:47:04 PDT 2008
//
// ****************************************************************************

avtGadgetFileFormat::avtGadgetFileFormat(const char *filename)
    : avtSTSDFileFormat(filename)
{
  int i,n;
  int npart[6];
  fname=filename;
  swap=0;
  ntot=0;
#ifdef MY_DEBUG
  fprintf(stderr,"loading %s\n",filename);
#endif
  if(!(fd = fopen(filename,"r")))
    {
      EXCEPTION1(InvalidDBTypeException,"Cant open file\n");
    }  
  else
    {
      /*----------- RED HEADER TO GET GLOBAL PROPERTIES -------------*/
      n = read_gadget_head(npart,masstab,&time,&redshift,fd);
      
      ntot=0;
      for(i=0;i<6;i++)
        {
          ntot += npart[i];
        }
      //      pos=new float[3*npart[0]];
      //      n = read_gadget_float3((float*)pos,"POS ",fd);

    }
#ifdef MY_DEBUG

  fprintf(stderr,"finished constructor\n");
#endif
  // INITIALIZE DATA MEMBERS
}


// ****************************************************************************
//  Method: avtGadgetFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Ralph Bruckschen
//  Creation:   Wed Oct 22 12:47:04 PDT 2008
//
// ****************************************************************************

void
avtGadgetFileFormat::FreeUpResources(void)
{
  
}


// ****************************************************************************
//  Method: avtGadgetFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Ralph Bruckschen
//  Creation:   Wed Oct 22 12:47:04 PDT 2008
//
// ****************************************************************************

void
avtGadgetFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
#ifdef MY_DEBUG
  fprintf(stderr,"pop database\n");
#endif
    //
    // CODE TO ADD A MESH
    //
  char **labels=new char*[50];
  int numblocks;
  string meshname = "POS ";
  int nblocks = 1;//  <-- this must be 1 for STSD
//  int block_origin = 0;
  int spatial_dimension = 3;
  int topological_dimension = 0;
  int *vec_flag=new int [50];
  avtMeshMetaData *mmd = new avtMeshMetaData;
  mmd->name = "POS ";
  mmd->spatialDimension = spatial_dimension;
  mmd->topologicalDimension = topological_dimension;
  mmd->meshType = AVT_POINT_MESH;
  mmd->numBlocks = nblocks;
  md->Add(mmd);
#ifdef MY_DEBUG
  fprintf(stderr,"get block names\n");
#endif  
  get_block_names(fd, labels,vec_flag, &numblocks);
#ifdef MY_DEBUG
  fprintf(stderr,"got block labels\n");
#endif
  for(int i=0;i<numblocks;i++)
    {
#ifdef MY_DEBUG
      fprintf(stderr,"got %s\n",labels[i]);
#endif
    if(strcmp(labels[i],"POS ")!=0 && strcmp(labels[i],"HEAD")!=0)
      {
        if(vec_flag[i])
          {
            avtVectorMetaData *smd = new avtVectorMetaData;
            smd->name = labels[i];
            smd->meshName = "POS ";
            smd->centering = AVT_ZONECENT;
            md->Add(smd);
          }
        else
          {
            avtScalarMetaData *smd = new avtScalarMetaData;
            smd->name = labels[i];
            smd->meshName = "POS ";
            smd->centering = AVT_ZONECENT;
            md->Add(smd);
          }
      }
    }
#ifdef MY_DEBUG
  fprintf(stderr,"done creating\n");
#endif

}


// ****************************************************************************
//  Method: avtGadgetFileFormat::GetMesh
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
//  Programmer: Ralph Bruckschen
//  Creation:   Wed Oct 22 12:47:04 PDT 2008
//
// ****************************************************************************

vtkDataSet *
avtGadgetFileFormat::GetMesh(const char *meshname)
{
#ifdef MY_DEBUG
  fprintf(stderr,"reading mesh %d\n",ntot);
#endif
  vtkPoints *points = vtkPoints::New();
  points->SetNumberOfPoints(ntot);
  float *pts = (float *) points->GetVoidPointer(0);
  
  int n = read_gadget_float3(pts,meshname,fd);
#ifdef MY_DEBUG
  fprintf(stderr,"done reading mesh %d\n",n);
#endif
  vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
  ugrid->SetPoints(points);
  points->Delete();
  ugrid->Allocate(ntot);
  vtkIdType onevertex;
  for(int i = 0; i < (int)ntot; ++i)
    {
      onevertex = i;
      ugrid->InsertNextCell(VTK_VERTEX, 1, &onevertex);
    }
      return ugrid; 
}


// ****************************************************************************
//  Method: avtGadgetFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Ralph Bruckschen
//  Creation:   Wed Oct 22 12:47:04 PDT 2008
//
// ****************************************************************************

vtkDataArray *
avtGadgetFileFormat::GetVar(const char *varname)
{
#ifdef MY_DEBUG
  fprintf(stderr,"reading data %s\n",varname);
#endif
  if(!strcmp(varname,"ID  "))
    {
#ifdef MY_DEBUG
      fprintf(stderr,"got ID\n");
#endif
      vtkIntArray *rv=vtkIntArray::New();
      rv->SetNumberOfTuples(ntot);
      float *pts = (float *) rv->GetVoidPointer(0);
      int n = read_gadget_float(pts,varname,fd);
      return rv;
    }
    else
      {
        vtkFloatArray *rv = vtkFloatArray::New();
      
        rv->SetNumberOfTuples(ntot);
        float *pts = (float *) rv->GetVoidPointer(0);
        int n = read_gadget_float(pts,varname,fd);
#ifdef MY_DEBUG
        fprintf(stderr,"done reading data %s  %d\n",varname,n);
#endif
        return rv; 
      }

}


// ****************************************************************************
//  Method: avtGadgetFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Ralph Bruckschen
//  Creation:   Wed Oct 22 12:47:04 PDT 2008
//
// ****************************************************************************

vtkDataArray *
avtGadgetFileFormat::GetVectorVar(const char *varname)
{
  //YOU MUST IMPLEMENT THIS
#ifdef MY_DEBUG
  fprintf(stderr,"read vector %s\n",varname);
#endif
  vtkFloatArray *rv = vtkFloatArray::New();
  rv->SetNumberOfComponents(3);
  rv->SetNumberOfTuples(ntot);
  float *pts = (float *) rv->GetVoidPointer(0);
  int n = read_gadget_float3(pts,varname,fd);
#ifdef MY_DEBUG
  fprintf(stderr,"done reading data %s  %d\n",varname,n);
#endif
  return rv;
}

int avtGadgetFileFormat::GetCycle(void)
{
  return GetCycleFromFilename(fname);
}
double avtGadgetFileFormat::GetTime(void)
{
  return time;
}
