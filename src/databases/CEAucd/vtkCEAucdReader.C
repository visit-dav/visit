/*=========================================================================

Program:   Visualization Toolkit
Module:    $RCSfile: vtkCEAucdReader.cxx,v $

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
// Thanks to Guenole Harel and Emmanuel Colin (Supelec engineering school,
// France) and Jean M. Favre (CSCS, Switzerland) who co-developed this class.
// Thanks to Isabelle Surin (isabelle.surin at cea.fr, CEA-DAM, France) who
// supervised the internship of the first two authors.  Thanks to Daniel
// Aguilera (daniel.aguilera at cea.fr, CEA-DAM, France) who contributed code
// und advice.  Please address all comments to Jean Favre (jfavre at cscs.ch)

#include <fcntl.h>
#include <ctype.h>

#include "vtkCEAucdReader.h"
#include <vtkDataArraySelection.h>
#include <vtkErrorCode.h>
#include <vtkUnstructuredGrid.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkFieldData.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkByteSwap.h>
#include <vtkIdTypeArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkByteSwap.h>
#include <vtkCellArray.h>
#include <vtkExecutive.h>

#include <list>

vtkCxxRevisionMacro(vtkCEAucdReader, "$Revision: 1.26 $");
vtkStandardNewMacro(vtkCEAucdReader);

#ifdef VERBOSE_DEBUG
#undef vtkDebugMacro
#define vtkDebugMacro(x) std::cout<<"" x; std::cout.flush()
#endif


static int UCD2VTK[8] =
{
   VTK_VERTEX,     // vtkCEAucdReader::PT
   VTK_LINE,       // vtkCEAucdReader::LINE
   VTK_TRIANGLE,   // vtkCEAucdReader::TRI
   VTK_QUAD,       // vtkCEAucdReader::QUAD
   VTK_TETRA,      // vtkCEAucdReader::TET
   VTK_PYRAMID,    // vtkCEAucdReader::PYR
   VTK_WEDGE,      // vtkCEAucdReader::PRISM
   VTK_HEXAHEDRON, // vtkCEAucdReader::HEX
};

//----------------------------------------------------------------------------
vtkCEAucdReader::vtkCEAucdReader()
{
   this->FileName  = NULL;
   this->GlobalOffset = 0;
   this->FileSize = 0;
   this->ByteOrder = FILE_BIG_ENDIAN;
   this->BinaryFile = 0;
   this->OwnStream = true;
   this->NumberOfNodeFields = 0;
   this->NumberOfCellFields = 0;
   this->NumberOfFields = 0;
   this->NumberOfNodeComponents = 0;
   this->NumberOfCellComponents = 0;
   this->FileStream = NULL;
   this->DecrementNodeIds = 0;
   this->NumberOfNodes = 0;
   this->NumberOfCells = 0;

   this->ActiveCellArray = NULL;
   this->ActivePointArray = NULL;

   this->NodeDataInfo = NULL;
   this->CellDataInfo = NULL;
   this->PointDataArraySelection = vtkDataArraySelection::New();
   this->CellDataArraySelection = vtkDataArraySelection::New();
   this->PerMaterialOutput = 0;
   this->NumberOfMaterials = 1;
   this->CellMaterialId = vtkIntArray::New();
   this->CellMaterialId->SetNumberOfComponents(1);
   this->CellMaterialId->SetName("Material Id");
   this->CellsInMaterial=NULL;
   this->PointsInMaterial=NULL;
   this->CellIndexInMaterial=NULL;

  // initialize output cache
   this->CachedFileName = (this->FileName!=0) ? strdup( this->FileName ) : 0;
   this->CachedGlobalOffset = this->GlobalOffset;
   this->CachedFileStream = this->FileStream;
   this->CachedPerMaterialOutput = this->PerMaterialOutput;

   this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
vtkCEAucdReader::~vtkCEAucdReader()
{
   ClearCache();

   if (this->FileStream!=0 && this->OwnStream)
   {
      delete this->FileStream;
   }

   if (this->FileName)
   {
      delete [] this->FileName;
   }
   if (this->NodeDataInfo)
   {
      delete [] this->NodeDataInfo;
   }
   if (this->CellDataInfo)
   {
      delete [] this->CellDataInfo;
   }
   if(this->CellsInMaterial)
   {
      delete [] this->CellsInMaterial;
   }
   if(this->PointsInMaterial)
   {
      delete [] this->PointsInMaterial;
   }
   if(this->CellIndexInMaterial)
   {
      delete [] this->CellIndexInMaterial;
   }
   this->CellDataArraySelection->Delete();
   this->PointDataArraySelection->Delete();
   this->CellMaterialId->Delete();
}


//----------------------------------------------------------------------------
void vtkCEAucdReader::SetByteOrderToBigEndian()
{
   this->ByteOrder = FILE_BIG_ENDIAN;
}


//----------------------------------------------------------------------------
void vtkCEAucdReader::SetByteOrderToLittleEndian()
{
   this->ByteOrder = FILE_LITTLE_ENDIAN;
}

//----------------------------------------------------------------------------
const char *vtkCEAucdReader::GetByteOrderAsString()
{
   if ( this->ByteOrder ==  FILE_LITTLE_ENDIAN)
   {
      return "LittleEndian";
   }
   else
   {
      return "BigEndian";
   }
}

void vtkCEAucdReader::SetFileStream( ifstream* istr )
{
   if( istr == this->FileStream ) return;

   this->OwnStream = (istr == 0);

   if( this->OwnStream )
   {
      if( this->FileStream != 0 )
      {
	 delete this->FileStream;
	 this->FileStream = 0;
      }
   }

   this->FileStream = istr;
   vtkDebugMacro(<<"OwnStream = "<<this->OwnStream<<"\n");
}

//----------------------------------------------------------------------------
int vtkCEAucdReader::RequestData(
   vtkInformation *vtkNotUsed(request),
   vtkInformationVector **vtkNotUsed(inputVector),
   vtkInformationVector *outputVector)
{
   vtkDebugMacro( << "Reading CEA UCD file\n");

   this->PruneCache();

  // If ExecuteInformation() failed FileStream will be NULL and
  // ExecuteInformation() will have spit out an error.
   if ( this->FileStream )
   {
      this->ReadFile(outputVector);
   }

   this->CacheOutputs();

   return 1;
}

//----------------------------------------------------------------------------
void vtkCEAucdReader::PrintSelf(ostream& os, vtkIndent indent)
{
   this->Superclass::PrintSelf(os,indent);

   os << indent << "File Name: " 
      << (this->FileName ? this->FileName : "(none)") << "\n";

   os << indent << "Number Of Nodes: " << this->NumberOfNodes << endl;
   os << indent << "Number Of Node Fields: " 
      << this->NumberOfNodeFields << endl;
   os << indent << "Number Of Node Components: " 
      << this->NumberOfNodeComponents << endl;

   os << indent << "Number Of Cells: " << this->NumberOfCells << endl;
   os << indent << "Number Of Cell Fields: " 
      << this->NumberOfCellFields << endl;
   os << indent << "Number Of Cell Components: " 
      << this->NumberOfCellComponents << endl;
  
   os << indent << "Byte Order: " << this->ByteOrder << endl;
   os << indent << "Binary File: " << (this->BinaryFile ? "True\n" : "False\n");
   os << indent << "Number of Fields: " << this->NumberOfFields << endl;
}


//----------------------------------------------------------------------------
void vtkCEAucdReader::ReadFile(vtkInformationVector *outputVector)
{
   this->ReadGeometry(outputVector);
   this->UpdateProgress(0.5);
   
   vtkInformation *outInfo = outputVector->GetInformationObject(0);
   vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

   if(this->NumberOfNodeFields)
   {
      this->ReadNodeData(output);
   }
   
   this->UpdateProgress(0.75);
   
   if(this->NumberOfCellFields)
   {
      this->ReadCellData(output); 
   }

   if(this->NumberOfMaterials == 1)
   {
      output->GetCellData()->AddArray(this->CellMaterialId);
      if (!output->GetCellData()->GetScalars())
      {
	 output->GetCellData()->SetScalars(this->CellMaterialId);
      }
   }

   if( this->OwnStream )
   {
      delete this->FileStream;
      this->FileStream = NULL;
   }
   this->UpdateProgress(1.0);
}

//----------------------------------------------------------------------------
int vtkCEAucdReader::RequestInformation(
   vtkInformation *vtkNotUsed(request),
   vtkInformationVector **vtkNotUsed(inputVector),
   vtkInformationVector *vtkNotUsed(outputVector))
{
   char magic_number='\0';
   long trueFileLength;
   int i, j, k, *ncomp_list;
   
   this->PruneCache();

   if( ! this->CachedOutputs.empty() )
   {
      vtkDebugMacro(<<"Bypass vtkCEAucdReader::RequestInformation\n");
      return 1;
   }

  // first open file in binary mode to check the first byte.
   if ( !this->FileName )
   {
      this->NumberOfNodes = 0;
      this->NumberOfCells = 0;
      this->NumberOfNodeFields = 0;
      this->NumberOfCellFields = 0;
      this->NumberOfFields = 0;
      vtkErrorMacro("No filename specified");
      return 0;
   }
   
   if( this->OwnStream )
   {
      vtkDebugMacro(<<"Open file "<<this->FileName<<"\n");
#ifdef _WIN32
      this->FileStream = new ifstream(this->FileName, ios::in | ios::binary);
#else
      this->FileStream = new ifstream(this->FileName, ios::in);
#endif
   }

   if (this->FileStream->fail())
   {
      this->SetErrorCode(vtkErrorCode::FileNotFoundError);
      if( this->OwnStream )
      {
	 delete this->FileStream;
	 this->FileStream = NULL;
      }
      vtkErrorMacro("Specified filename not found");
      return 0;
   }
  
   this->FileStreamSeek( 0 );
   this->FileStream->get(magic_number);
   this->FileStream->putback(magic_number);
   if(magic_number != 7)
   { // most likely an ASCII file
      this->BinaryFile = 0;
      if( this->OwnStream )
      {
	 vtkDebugMacro(<<"Re-open file "<<this->FileName<<" in ASCII mode");
	 delete this->FileStream; // close file to reopen it later
	 this->FileStream = NULL;
	 this->FileStream = new ifstream(this->FileName, ios::in);
      }
      
      char c='\0', buf[100];
      while(this->FileStream->get(c) && c == '#')
      {
	 this->FileStream->get(buf, 100, '\n'); this->FileStream->get(c);
      }
      this->FileStream->putback(c);
      
      *(this->FileStream) >> this->NumberOfNodes;
      *(this->FileStream) >> this->NumberOfCells;
      *(this->FileStream) >> this->NumberOfNodeFields;
      *(this->FileStream) >> this->NumberOfCellFields;
      *(this->FileStream) >> this->NumberOfFields;
   }
   else
   {
      this->BinaryFile = 1;
      
     // Here we first need to check if the file is little-endian or big-endian
     // We will read the variable once, with the given endian-ness set up in
     // the class constructor. If trueFileLength does not match
     // calculatedFileLength, then we will toggle the endian-ness and re-swap
     // the variables
      if (this->FileSize != 0)
      {
	 trueFileLength = this->FileSize;
      }
      else 
      {
	 this->FileStreamSeekEnd();
	 trueFileLength = this->FileStream->tellg();
      }
      long calculatedFileLength = 0; // unknown yet
      bool condition = true;
      int numberOfSwap = 0;
      while(condition)
      {
	// restart at beginning of file
	 this->FileStreamSeek( 0 );
	 
	 this->FileStream->read(&magic_number, 1);
	 
	 this->ReadIntBlock(1, &this->NumberOfNodes);
	 this->ReadIntBlock(1, &this->NumberOfCells);
	 this->ReadIntBlock(1, &this->NumberOfNodeFields);
	 this->ReadIntBlock(1, &this->NumberOfCellFields);
	 this->ReadIntBlock(1, &this->NumberOfFields);
	 this->ReadIntBlock(1, &this->NlistNodes);
	 
	 vtkDebugMacro( << this->NumberOfNodes << " "
			<< this->NumberOfCells << " "
			<< this->NumberOfNodeFields << " "
			<< this->NumberOfCellFields << " "
			<< this->NumberOfFields << " "
			<< this->NlistNodes << endl);
	 
	 calculatedFileLength  = 1 + 6*4;
	 calculatedFileLength += 16 * this->NumberOfCells + 4 * this->NlistNodes;
	 calculatedFileLength += 3*4 * this->NumberOfNodes;
	 if(this->NumberOfNodeFields)
	 {
	    calculatedFileLength += 2052 +
	       this->NumberOfNodeFields*(12 + 4 * this->NumberOfNodes + 4);
	 }
	 
	 if(this->NumberOfCellFields)
	 {
	    calculatedFileLength += 2052 + 
	       this->NumberOfCellFields*(12 + 4 * this->NumberOfCells + 4);
	 }
	 
	 if(this->NumberOfFields)
	 {
	    calculatedFileLength += 2052 + this->NumberOfFields*(4 * 5);
	 }
	 
	 vtkDebugMacro( << "TFL = " << trueFileLength 
			<< "\tCFL = " << calculatedFileLength << endl );
	 
	 double rapport = (double) (trueFileLength) / (double) (calculatedFileLength);
	 condition = (rapport < 0.99 || rapport > 1.01) && numberOfSwap < 2;
	 if(condition)
	 {
	    numberOfSwap++;
	   // switch to opposite of what previously set in constructor
	    if(this->ByteOrder == FILE_LITTLE_ENDIAN)
	    {
	       this->ByteOrder = FILE_BIG_ENDIAN; 
	    }
	    else if(this->ByteOrder == FILE_BIG_ENDIAN)
	    {
	       this->ByteOrder = FILE_LITTLE_ENDIAN;
	    }
	 }
	 
      } // end of while loop
      
      const long base_offset = 1 + 6*4;
      char buffer1[1024], buffer2[1024], label[32];
      
      long offset = base_offset + 16 * this->NumberOfCells + 
	 4 * this->NlistNodes + 3 * 4 * this->NumberOfNodes;
      
      if(this->NumberOfNodeFields)
      {
	//this->FileStream->seekg(offset,ios::beg);
	 this->FileStreamSeek( offset );
	 this->FileStream->read(buffer1, sizeof(buffer1));
	 this->FileStream->read(buffer2, sizeof(buffer2)); // read 2nd array of 1024 bytes
	 this->ReadIntBlock(1, &this->NumberOfNodeComponents);
	 
	 ncomp_list = new int[this->NumberOfNodeFields];
	 this->ReadIntBlock(this->NumberOfNodeFields, ncomp_list);
	 
	 this->NodeDataInfo = new DataInfo[this->NumberOfNodeComponents];
	 
	 float *mx = new float[this->NumberOfNodeFields];
	// read now the minimums for node_data
	 this->ReadFloatBlock(this->NumberOfNodeFields, mx);
	 k=0;
	 for(i=0; i < this->NumberOfNodeComponents; i++)
	 {
	    for(j=0; j < ncomp_list[i]; j++)
	    {
	       this->NodeDataInfo[i].min[j] = mx[k];
	    }
	    k++;
	 }
	// read now the maximums for node_data
	 this->ReadFloatBlock(this->NumberOfNodeFields, mx);
	 k=0;
	 for(i=0; i < this->NumberOfNodeComponents; i++)
	 {
	    for(j=0; j < ncomp_list[i]; j++)
	    {
	       this->NodeDataInfo[i].max[j] = mx[k];
	    }
	    k++;
	 }
	 delete [] mx;
	 
	 offset +=  1024 + 1024 + 4 + 3 * 4 * this->NumberOfNodeFields;
	 
	 k = 0;
	 for(i=0; i < this->NumberOfNodeComponents; i++)
	 {
	    this->GetLabel(buffer1, i, label);
	   /*vtkDebugMacro( << i+1 << " :found ND label = " << label 
	   << " [" << ncomp_list[i] << "]" <<endl); */
	    this->PointDataArraySelection->AddArray(label);
	    this->NodeDataInfo[i].foffset = offset + k * 4 * this->NumberOfNodes;
	    this->NodeDataInfo[i].veclen = ncomp_list[i];
	    k += ncomp_list[i];
	 }
	 delete [] ncomp_list;
      }
      
      if(this->NumberOfCellFields)
      {
	 offset += 4 * this->NumberOfNodes * this->NumberOfNodeFields + 
	    4 * this->NumberOfNodeFields;
	 this->FileStreamSeek( offset );
	 this->FileStream->read(buffer1, sizeof(buffer1));
	 
	 this->FileStream->read(buffer2, sizeof(buffer2)); // read 2nd array of 1024 bytes
	 this->ReadIntBlock(1, &this->NumberOfCellComponents);
	 
	 ncomp_list = new int[this->NumberOfCellFields];
	 this->ReadIntBlock(this->NumberOfCellFields, ncomp_list);
	 
	 if (this->CellDataInfo != 0) delete [] this->CellDataInfo;
	 this->CellDataInfo = new DataInfo[this->NumberOfCellComponents];
	 
	 float *mx = new float[this->NumberOfCellFields];
	// read now the minimums for cell_data
	 this->ReadFloatBlock(this->NumberOfCellFields, mx);
	 k=0;
	 for(i=0; i < this->NumberOfCellFields; i++)
	 {
	    for(j=0; j < ncomp_list[i]; j++)
	    {
	       this->CellDataInfo[i].min[j] = mx[k];
	    };
	    k++;
	 }
	// read now the maximums for cell_data
	 this->ReadFloatBlock(this->NumberOfCellFields, mx);
	 k=0;
	 for(i=0; i < this->NumberOfCellFields; i++)
	 {
	    for(j=0; j < ncomp_list[i]; j++)
	    {
	       this->CellDataInfo[i].max[j] = mx[k];
	    }
	    k++;
	 }
	 delete [] mx;
	 
	 offset += 1024 + 1024 + 4 + 3 * 4 * this->NumberOfCellFields;
	 
	 k = 0;
	 for(i=0; i < this->NumberOfCellComponents; i++)
	 {
	    this->GetLabel(buffer1, i, label);
	   /*
	   vtkDebugMacro( << i+1 << " :found CD label = " << label << " [" 
	   << ncomp_list[i] << "]" << endl);
	   */
	    this->CellDataArraySelection->AddArray(label);
	    this->CellDataInfo[i].foffset = offset + k * 4 * this->NumberOfCells;
	    this->CellDataInfo[i].veclen = ncomp_list[i];
	    k += ncomp_list[i];
	 }
	 delete [] ncomp_list;
      }
      
      if(this->NumberOfFields)
      {
	 offset += 4 * this->NumberOfCells * this->NumberOfCellFields + 
	    4 * this->NumberOfCellFields;
	 this->FileStreamSeek( offset );
	 this->FileStream->read(buffer1, sizeof(buffer1));
	 vtkDebugMacro(<< buffer1 << endl);
	 
	//offset += 1024 + 1024 + 4 + 3 * 4 * this->NumberOfFields;
	 
	 for(i=0; i < this->NumberOfFields; i++)
	 {
	    this->GetLabel(buffer1, i, label);
	    vtkDebugMacro( << "found MD label = " << label << endl);
	 }
      }
   } // end of Binary part 

  /*
  for(i=0; i < this->NumberOfNodeComponents; i++)
  {
  vtkDebugMacro( << endl << this->PointDataArraySelection->GetArrayName(i) 
  << endl
  << "offset = " << this->NodeDataInfo[i].foffset << endl
  << "load = " << this->PointDataArraySelection->GetArraySetting(i) << endl
  << "veclen = " << this->NodeDataInfo[i].veclen);
  }
   
  for(i=0; i < this->NumberOfCellComponents; i++)
  {
  vtkDebugMacro( << endl << this->CellDataArraySelection->GetArrayName(i) 
  << endl
  << "offset = " << this->CellDataInfo[i].foffset << endl
  << "load = " << this->CellDataArraySelection->GetArraySetting(i) << endl
  << "veclen = " << this->CellDataInfo[i].veclen);
  }
  */

   vtkDebugMacro( << "end of ExecuteInformation\n");
   
   return 1;
}

//----------------------------------------------------------------------------
void vtkCEAucdReader::GetCellDataRange(int cellComp, int index, float *min, float *max)
{
  // cet update est la pour forcer la lecture de la donnee ou sont
  // recalcules les min et max pour corriger les cas ou ils sont
  // foireux dans le fichier
   this->Update();
   if (index >= this->CellDataInfo[cellComp].veclen || index < 0)
   {
      index = 0;  // if wrong index, set it to zero
   }
   *min = this->CellDataInfo[cellComp].min[index];
   *max = this->CellDataInfo[cellComp].max[index];
}

//----------------------------------------------------------------------------
void vtkCEAucdReader::GetNodeDataRange(int nodeComp, int index, float *min, float *max)
{
  // cet update est la pour forcer la lecture de la donnee ou sont
  // recalcules les min et max pour corriger les cas ou ils sont
  // foireux dans le fichier
   this->Update();
   if (index >= this->NodeDataInfo[nodeComp].veclen || index < 0)
   {
      index = 0;  // if wrong index, set it to zero
   }
   *min = this->NodeDataInfo[nodeComp].min[index];
   *max = this->NodeDataInfo[nodeComp].max[index];
}

//----------------------------------------------------------------------------
void vtkCEAucdReader::ReadGeometry(vtkInformationVector *outputVector)
{
   int i;
   vtkFloatArray *coords;
   vtkInformation *outInfo = outputVector->GetInformationObject(0);
   vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

   if (this->BinaryFile)
   {

     // use cached data if available (but only for binary files)
      if( ! this->CachedOutputs.empty() )
      {
	 vtkDebugMacro(<<"Rebuild geometry from cache\n");

	 this->NumberOfMaterials = this->CachedOutputs.size();
	 int numberOfOutputPorts = this->GetNumberOfOutputPorts ();
	 this->SetNumberOfOutputPorts (this->NumberOfMaterials);
	 for(i=numberOfOutputPorts; i<this->NumberOfMaterials; i++)
	 {
	    vtkUnstructuredGrid * ug = vtkUnstructuredGrid::New();
	    ug->Initialize();
	    this->GetExecutive()->SetOutputData (i, ug);
	    ug->SetMaximumNumberOfPieces (-1);
	    ug->Delete();
	 }     

	 for(i=0;i<this->NumberOfMaterials;i++)
	 {
	    this->GetOutput(i)->Initialize();
	    this->GetOutput(i)->Allocate();
	    this->GetOutput(i)->GetCells()->Initialize();
	    
	    vtkPoints * points = vtkPoints::New();
	    this->GetOutput(i)->SetPoints(points);
	    points->Delete();
	    this->GetOutput(i)->GetPoints()->SetData( this->GetPointsDataFromCache(i) );

	    vtkUnsignedCharArray* cellTypes = 0;
	    vtkIdTypeArray* cellLocations = 0;
	    vtkCellArray* cells = 0;
	    this->GetCellsFromCache( i, cellTypes, cellLocations, cells );
	    this->GetOutput(i)->SetCells( cellTypes, cellLocations, cells );

	   //vtkDebugMacro(<<"Output "<<i<<" :\n");
	   //this->GetOutput(i)->PrintSelf(std::cout,4);
	 }
	 return ;
      }

     // otherwize, read from file
      int * ctypes;

     // read material ids and build material tables
      this->CellMaterialId->SetNumberOfTuples(this->NumberOfCells);
      ctypes = new int[4*this->NumberOfCells];
      this->ReadBinaryCellTypes( ctypes, this->PerMaterialOutput );

      this->UpdateProgress(0.15);

      vtkDebugMacro(<<"NumberOfMaterials*="<<this->NumberOfMaterials<<"\n");
      if( !this->PerMaterialOutput || this->NumberOfMaterials<=1 )
      {
	 this->NumberOfMaterials = 1;
	 if(this->CellsInMaterial)
	 {
	    delete [] this->CellsInMaterial;
	    this->CellsInMaterial = NULL;
	 }
	 if(this->PointsInMaterial)
	 {
	    delete [] this->PointsInMaterial;
	    this->PointsInMaterial = NULL;
	 }
	 if(this->CellIndexInMaterial)
	 {
	    delete [] this->CellIndexInMaterial;
	    this->CellIndexInMaterial = NULL;
	 }
      }
      vtkDebugMacro(<<"NumberOfMaterials="<<this->NumberOfMaterials<<"\n");

     // create supplementary outputs if necessary
      int numberOfOutputPorts = this->GetNumberOfOutputPorts ();
      this->SetNumberOfOutputPorts (NumberOfMaterials);
      for(i=numberOfOutputPorts; i<this->NumberOfMaterials; i++)
      {
	 vtkUnstructuredGrid * ug = vtkUnstructuredGrid::New();
	 ug->Initialize();
	 this->GetExecutive()->SetOutputData (i, ug);
	 ug->SetMaximumNumberOfPieces (-1);
	 ug->Delete();
      }

     // pre-allocate outputs memory
      if(this->NumberOfMaterials==1)
      {
	 output->Initialize();
	 vtkDebugMacro( <<"output->Allocate("<<this->NumberOfCells<<")\n" );
	 output->Allocate( this->NumberOfCells );
	 output->GetCells()->Initialize();
	 vtkDebugMacro( <<"output->GetCells()->Allocate("<<this->NumberOfCells + this->NlistNodes<<")\n" );
	 output->GetCells()->Allocate( this->NumberOfCells + this->NlistNodes );
      }
      else
      {
	 for(i=0;i<this->NumberOfMaterials;i++)
	 {
	    this->GetOutput(i)->Initialize();
	    vtkDebugMacro( << "this->GetOutput("<<i<<")->Allocate("<<this->CellsInMaterial[i]<<")\n" );
	    this->GetOutput(i)->Allocate( this->CellsInMaterial[i] );
	    this->GetOutput(i)->GetCells()->Initialize();
	    vtkDebugMacro( << "this->GetOutput("<<i<<")->GetCells()->Allocate("<<this->CellsInMaterial[i] + this->PointsInMaterial[i]<<")\n" );
	    this->GetOutput(i)->GetCells()->Allocate( this->CellsInMaterial[i] + this->PointsInMaterial[i] );
	 }
      }

      this->UpdateProgress(0.20);

     // read cell topology
      this->ReadBinaryCellTopology(ctypes);
      delete [] ctypes;

      this->UpdateProgress(0.35);

     // Read node coordinates
      coords = vtkFloatArray::New();
      coords->SetNumberOfComponents(3);
      coords->SetNumberOfTuples(this->NumberOfNodes);
      this->ReadXYZCoords(coords);

      this->UpdateProgress(0.45);
   }
   else
   {
     // Read node coordinates
      coords = vtkFloatArray::New();
      coords->SetNumberOfComponents(3);
      coords->SetNumberOfTuples(this->NumberOfNodes);
      this->ReadXYZCoords(coords);
      this->UpdateProgress(0.20);

     // Read cell connectivity
      this->ReadASCIICellTopology(this->CellMaterialId, output);
      this->UpdateProgress(0.40);
   }

   vtkPoints *points = vtkPoints::New();
   points->SetData(coords);
   coords->Delete();
  
   if(this->NumberOfMaterials == 1)
   {
      output->SetPoints(points);
      points->Delete();
   }
   else
   {
      for(i=0;i<this->NumberOfMaterials;i++)
      {
	 this->GetOutput(i)->SetPoints(points);
      }
      points->Delete();
   }

   this->UpdateProgress(0.50);
}

void vtkCEAucdReader::FileStreamSeek( size_t pos )
{
   pos += this->GlobalOffset;
  //off_t delta = (off_t)pos - (off_t)this->FileStream->tellg();
   this->FileStream->seekg( pos , ios::beg );
  //cout<<"SEEK "<<pos<<" "<<delta<<endl;
}

void vtkCEAucdReader::FileStreamSeekEnd()
{
   this->FileStream->seekg( 0 , ios::end );
  //cout<<"SEEKEND "<<this->FileStream->tellg()<<endl;
}


//----------------------------------------------------------------------------
void vtkCEAucdReader::ReadBinaryCellTypes(int* ctype, bool buildTables)
{
   vtkIdType i;
   int nmat;
   int *mat = this->CellMaterialId->GetPointer(0);

   this->FileStreamSeek( 6*4 + 1 );
   this->ReadIntBlock(4 * this->NumberOfCells, ctype);

   this->UpdateProgress(0.10);

   nmat = 0;
   for(i=0; i < this->NumberOfCells; i++)
   {
      mat[i] = ctype[4*i+1];
      if(mat[i]>=nmat) nmat = mat[i]+1;
   }

   this->NumberOfMaterials = nmat;

   if(this->CellsInMaterial)
   {
      delete [] this->CellsInMaterial;
      this->CellsInMaterial = NULL;
   }
   if(this->PointsInMaterial)
   {
      delete [] this->PointsInMaterial;
      this->PointsInMaterial = NULL;
   }
   if(this->CellIndexInMaterial)
   {
      delete [] this->CellIndexInMaterial;
      this->CellIndexInMaterial = NULL;
   }

   if( ! buildTables ) return;

   this->CellsInMaterial = new vtkIdType [ nmat ];
   this->PointsInMaterial = new vtkIdType [ nmat ];
   this->CellIndexInMaterial = new vtkIdType [ this->NumberOfCells ];

   for(i=0; i < nmat; i++)
   {
      this->CellsInMaterial[i] = 0;
      this->PointsInMaterial[i] = 0;
   }
   for(i=0; i < this->NumberOfCells; i++)
   {
      int m = mat[i];
      this->CellIndexInMaterial[i] = this->CellsInMaterial[m];
      this->CellsInMaterial[m]++;
      this->PointsInMaterial[m] += ctype[4*i+2];
   }

}

//----------------------------------------------------------------------------
void vtkCEAucdReader::ReadBinaryCellTopology(const int * ctype)
{
   vtkIdType pts[8];
   vtkIdType i, j, k;
   unsigned int t, np;
   int m, nmat = this->NumberOfMaterials;
   int* mat = this->CellMaterialId->GetPointer(0);
   int mask = (nmat>1) ? -1 : 0; // direct all cells to output 0 if not using multiple materials

   int *topology_list = new int[this->NlistNodes];
   if(topology_list == NULL)
   {
      vtkErrorMacro(<< "Error allocating topology_list memory");
   }

   this->FileStreamSeek( 6*4 + 1 + 4 * this->NumberOfCells * 4 );
   this->ReadIntBlock(this->NlistNodes, topology_list);
   this->UpdateProgress(0.25);

   for(i=0; i<this->NlistNodes; i++)
   {
      -- topology_list[i];
   }
   k = 0;
   for(i=0; i < this->NumberOfCells; i++)
   {
      m = mat[i] & mask;
      np = ctype[4*i+2];
      t = ctype[4*i+3];
      if( t >= 8 )
      {
	 vtkErrorMacro( << "cell type: " << t << " is not supported\n");
	 delete [] topology_list;
	 return;
      }
      t = UCD2VTK[ t ];
      if( t == VTK_PYRAMID )
      {
	//UCD ordering is 0,1,2,3,4 => VTK ordering is 1,2,3,4,0
	 pts[0] = topology_list[ k + 1 ];
	 pts[1] = topology_list[ k + 2 ];
	 pts[2] = topology_list[ k + 3 ];
	 pts[3] = topology_list[ k + 4 ];
	 pts[4] = topology_list[ k + 0 ];
	 k += np;
      }
      else
      {
	 for(j=0;j<np;j++) pts[j] = topology_list[k++];
      }
      this->GetOutput(m)->InsertNextCell( t , np , pts );
   }
  
   delete [] topology_list;
}


//----------------------------------------------------------------------------
void vtkCEAucdReader::ReadASCIICellTopology(vtkIntArray *materials, 
                                            vtkUnstructuredGrid *output)
{
   int i, k;
   vtkIdType list[8];
   int *mat = materials->GetPointer(0);
   char ctype[5];

   output->Allocate();
   for(i=0; i < this->NumberOfCells; i++)
   {
      int id;  // no check is done to see that they are monotonously increasing
      *(this->FileStream) >> id;
      *(this->FileStream) >> mat[i];
      *(this->FileStream) >> ctype;
      vtkDebugMacro( << mat[i] << ", " << ctype );
      if(!strcmp(ctype, "pt"))
      {
	 for(k=0; k < 1; k++)
	 {
	    *(this->FileStream) >> list[k];
	    if(this->DecrementNodeIds)
	    {
	       list[k]--;
	    }
	 }
	 output->InsertNextCell(VTK_VERTEX, 1, list);
      }
      else if(!strcmp(ctype, "line"))
      {
	 for(k=0; k < 2; k++)
	 {
	    *(this->FileStream) >> list[k];
	    if(this->DecrementNodeIds)
	    {
	       list[k]--;
	    }
	 }
	 output->InsertNextCell(VTK_LINE, 2, list);
      }
      else if(!strcmp(ctype, "tri"))
      {
	 for(k=0; k < 3; k++)
	 {
	    *(this->FileStream) >> list[k];
	    if(this->DecrementNodeIds)
	    {
	       list[k]--;
	    }
	 }
	 output->InsertNextCell(VTK_TRIANGLE, 3, list);
      }
      else if(!strcmp(ctype, "quad"))
      {
	 for(k=0; k < 4; k++)
	 {
	    *(this->FileStream) >> list[k];
	    if(this->DecrementNodeIds)
	    {
	       list[k]--;
	    }
	 }
	 output->InsertNextCell(VTK_QUAD, 4, list);
      }
      else if(!strcmp(ctype, "tet"))
      {
	 for(k=0; k < 4; k++)
	 {
	    *(this->FileStream) >> list[k];
	    if(this->DecrementNodeIds)
	    {
	       list[k]--;
	    }
	 }
	 output->InsertNextCell(VTK_TETRA, 4, list);
      }
      else if(!strcmp(ctype, "pyr"))
      {
	 for(k=0; k < 5; k++)
	 {
	    *(this->FileStream) >> list[k];
	    if(this->DecrementNodeIds)
	    {
	       list[k]--;
	    }
	 }
	 int tmp;
	 tmp = list[0];
	 list[0] = list[1]; list[1] = list[2]; list[2] = list[3];
	 list[3] = list[4]; list[4] = tmp;
	 output->InsertNextCell(VTK_PYRAMID, 5, list);
      }
      else if(!strcmp(ctype, "prism"))
      {
	 for(k=0; k < 6; k++)
	 {
	    *(this->FileStream) >> list[k];
	    if(this->DecrementNodeIds)
	    {
	       list[k]--;
	    }
	 }
	 output->InsertNextCell(VTK_WEDGE, 6, list);
      }
      else if(!strcmp(ctype, "hex"))
      {
	 for(k=0; k < 8; k++)
	 {
	    *(this->FileStream) >> list[k];
	    if(this->DecrementNodeIds)
	    {
	       list[k]--;
	    }
	 }
	 output->InsertNextCell(VTK_HEXAHEDRON, 8, list);
      }
      else
      {
	 vtkErrorMacro( << "cell type: " << ctype << " is not supported\n");
	 return;
      }
   }  // for all cell, read the indices 
}


//----------------------------------------------------------------------------
void vtkCEAucdReader::ReadXYZCoords(vtkFloatArray *coords)
{
   int i;
   float *ptr = coords->GetPointer(0);
   if (this->BinaryFile)
   {
      float *cs = new float[this->NumberOfNodes];

     // read X coordinates from file and stuff into coordinates array
      this->ReadFloatBlock(this->NumberOfNodes, cs);
      for(i=0; i < this->NumberOfNodes; i++)
      {
	 ptr[3*i] = cs[i];
      }

     // read Y coordinates from file and stuff into coordinates array
      this->ReadFloatBlock(this->NumberOfNodes, cs);
      for(i=0; i < this->NumberOfNodes; i++)
      {
	 ptr[3*i+1] = cs[i];
      }

     // read Z coordinates from file and stuff into coordinates array
      this->ReadFloatBlock(this->NumberOfNodes, cs);
      for(i=0; i < this->NumberOfNodes; i++)
      {
	 ptr[3*i+2] = cs[i];
      }
     // end of stuffing all coordinates
      delete [] cs;
   }  // end of binary read
   else
   {
      int id;  // no check is done to see that they are monotonously increasing
     // read here the first node and check if its id number is 0

      *(this->FileStream) >> id;
      i=0;
      *(this->FileStream) >> ptr[3*i] >> ptr[3*i+1] >> ptr[3*i+2];
      if(id)
      {
	 this->DecrementNodeIds = 1;
      }

      for(i=1; i < this->NumberOfNodes; i++)
      {
	 *(this->FileStream) >> id;
	 *(this->FileStream) >> ptr[3*i] >> ptr[3*i+1] >> ptr[3*i+2];
      }
   } // end of ASCII read
}


//----------------------------------------------------------------------------
void vtkCEAucdReader::ReadNodeData(vtkUnstructuredGrid *output)
{
   int i, j, n;
   float *ptr;
   vtkDebugMacro( << "Begin of ReadNodeData()\n");
   if(this->BinaryFile)
   {

      int active = -1;
      std::list<int> arraySelection;
      for (i=0; i < this->NumberOfNodeComponents; i++)
      {
	 if( ActivePointArray==0 || strcmp(PointDataArraySelection->GetArrayName(i),ActivePointArray)!=0 )
	 {
	    if( this->PointDataArraySelection->GetArraySetting(i) )
	    {
	       arraySelection.push_back(i);
	    }
	 }
	 else
	 {
	    active = i;
	 }
      }
      if( active != -1 )
      {
	 arraySelection.push_front( active );
      }

      for (std::list<int>::iterator it=arraySelection.begin(); it!=arraySelection.end(); it++)
      {
	 i=*it;
	 if(this->PointDataArraySelection->GetArraySetting(i))
	 {
	    vtkDataArray* dataArray = this->GetPointDataArrayFromCache(0,this->PointDataArraySelection->GetArrayName(i)) ;
	    bool delArray = false;
	    if( dataArray == 0 )
	    {
	       vtkFloatArray *scalars = vtkFloatArray::New();
	       scalars->SetNumberOfComponents(this->NodeDataInfo[i].veclen);
	       scalars->SetNumberOfTuples(this->NumberOfNodes);
	       scalars->SetName(PointDataArraySelection->GetArrayName(i));
	       this->FileStreamSeek( this->NodeDataInfo[i].foffset );
	       if(1) // this->NodeDataInfo[i].veclen == 1)
	       {
		  ptr = scalars->GetPointer(0);
		  this->ReadFloatBlock(this->NumberOfNodes * 
				       this->NodeDataInfo[i].veclen, ptr);

		 // recompute min and max
		  for (int nc = 0; nc < NodeDataInfo[i].veclen; nc++)
		  {
		     float min = ptr[nc];
		     float max = ptr[nc];
		     for (int index = nc+1; index < this->NumberOfNodes * this->NodeDataInfo[i].veclen; index+= 1+nc)
		     {
			if (ptr[index] < min) min = ptr[index];
			if (ptr[index] > max) max = ptr[index];
		     }
		     this->NodeDataInfo[i].min[nc] = min;
		     this->NodeDataInfo[i].max[nc] = max;
		  }
	       }
	       else
	       {
		  ptr = new float[this->NodeDataInfo[i].veclen];
		  for(n=0; n < this->NumberOfNodes; n++)
		  {
		     this->ReadFloatBlock(this->NodeDataInfo[i].veclen, ptr);
		     for(j=0; j < this->NodeDataInfo[i].veclen; j++)
		     {
			scalars->SetComponent(n, j, ptr[j]);
		     }
		  }
		  delete [] ptr;
	       }
	       dataArray = scalars;
	       delArray = true;
	    }

	    for(j=0;j<this->NumberOfMaterials;j++)
	    {
	       this->GetOutput(j)->GetPointData()->AddArray(dataArray);
	       if ( !this->GetOutput(j)->GetPointData()->GetScalars() )
	       {
		  this->GetOutput(j)->GetPointData()->SetScalars(dataArray);
		  this->GetOutput(j)->GetPointData()->SetActiveScalars (dataArray->GetName());
	       }
	    }
	    if(delArray)
	    {
	       dataArray->Delete();
	    }
	    else 
	    {
	       vtkDebugMacro("Node data "<<dataArray->GetName()<<" retreived from cache\n");
	    }
	 }
	 this->UpdateProgress( 0.50 + i*0.25/this->NumberOfNodeComponents );
      }
     //
     // Don't know how to use the information below, so skip reading it 
     // int *node_active_list = new int[this->NumberOfNodeFields];
     // this->ReadIntArray(node_active_list, this->NumberOfNodeFields);
     // delete [] node_active_list;
     //
   } // end of binary read
   else
   {
      float value;
      int id;
      char buf1[128], c='\0', buf2[128];

      *(this->FileStream) >> this->NumberOfNodeComponents;
      if (this->NodeDataInfo != 0) delete [] this->NodeDataInfo;
      this->NodeDataInfo = new DataInfo[this->NumberOfNodeComponents];
      for(i=0; i < this->NumberOfNodeComponents; i++)
      {
	 *(this->FileStream) >> this->NodeDataInfo[i].veclen;
      }
      this->FileStream->get(c); // one more newline to catch

      vtkFloatArray **scalars = new 
	 vtkFloatArray * [this->NumberOfNodeComponents];
      for(i=0; i < this->NumberOfNodeComponents; i++)
      {
	 j=0;
	 while(this->FileStream->get(c) && c != ',')
	 {
	    buf1[j++] = c;
	 }
	 buf1[j] = '\0';
	// finish here to read the line
	 this->FileStream->get(buf2, 128, '\n'); this->FileStream->get(c);

	 scalars[i] = vtkFloatArray::New();
	 scalars[i]->SetNumberOfComponents(this->NodeDataInfo[i].veclen);
	 scalars[i]->SetNumberOfTuples(this->NumberOfNodes);
	 scalars[i]->SetName(buf1);
      }

      this->UpdateProgress( 0.60 );

      for(n=0; n < this->NumberOfNodes; n++)
      {
	 *(this->FileStream) >> id;
	 for(i=0; i < this->NumberOfNodeComponents; i++)
	 {
	    for(j=0; j < this->NodeDataInfo[i].veclen; j++)
	    {
	       *(this->FileStream) >> value;
	       scalars[i]->SetComponent(n, j, value);
	    }
	 }
      }

      this->UpdateProgress( 0.70 );

      for(i=0; i < this->NumberOfNodeComponents; i++)
      {
	 output->GetPointData()->AddArray(scalars[i]);
	 if (!output->GetPointData()->GetScalars())
	 {
	    output->GetPointData()->SetScalars(scalars[i]);
	 }
	 scalars[i]->Delete();
      }
      delete[] scalars;
   } // end of ASCII read
   vtkDebugMacro( << "End of ReadNodeData()\n");
}


//----------------------------------------------------------------------------
void vtkCEAucdReader::ReadCellData(vtkUnstructuredGrid *output)
{
   vtkIdType i, j, n, m, p, cell;
   float *ptr;
   int* mat = this->CellMaterialId->GetPointer(0);
  
   vtkDebugMacro( << "Begin of ReadCellData()\n");
   if(this->BinaryFile)
   {

      int active = -1;
      std::list<int> arraySelection;
      for (i=0; i < this->NumberOfCellComponents; i++)
      {
	 if( ActiveCellArray==0 || strcmp(CellDataArraySelection->GetArrayName(i),ActiveCellArray)!=0 )
	 {
	    if( this->CellDataArraySelection->GetArraySetting(i) )
	    {
	       arraySelection.push_back(i);
	    }
	 }
	 else
	 {
	    active = i;
	 }
      }
      if( active != -1 )
      {
	 arraySelection.push_front( active );
      }

      for (std::list<int>::iterator it=arraySelection.begin(); it!=arraySelection.end(); it++)
      {
	 i=*it;
	 if(this->CellDataArraySelection->GetArraySetting(i))
	 {
	    bool hasCachedArray = ( this->GetCellDataArrayFromCache( 0, this->CellDataArraySelection->GetArrayName(i) ) != 0 );
	    if( hasCachedArray )
	    {
	       vtkDebugMacro( << "Retreiving cell array "<<this->CellDataArraySelection->GetArrayName(i)<<" from cache\n" );
	    }

	    if(this->NumberOfMaterials>1)
	    {
	       vtkDebugMacro( << "multi-material\n" );
	       vtkFloatArray **scalars = 0;
	       if( ! hasCachedArray )
	       {
		  ptr = new float[ this->NumberOfCells * this->CellDataInfo[i].veclen ];
		  this->FileStreamSeek( this->CellDataInfo[i].foffset );
		  this->ReadFloatBlock(this->NumberOfCells * this->CellDataInfo[i].veclen, ptr);

		 // recompute min and max
		  for (int nc = 0; nc < CellDataInfo[i].veclen; nc++)
		  {
		     float min = ptr[nc];
		     float max = ptr[nc];
		     for (int index = nc+1; index < this->NumberOfCells * this->CellDataInfo[i].veclen; index+= 1+nc)
		     {
			if (ptr[index] < min) min = ptr[index];
			if (ptr[index] > max) max = ptr[index];
		     }
		     this->CellDataInfo[i].min[nc] = min;
		     this->CellDataInfo[i].max[nc] = max;
		  }

		  scalars = new vtkFloatArray*[this->NumberOfMaterials];
		  for(m=0;m<this->NumberOfMaterials;m++)
		  {
		     vtkDebugMacro( << "material "<<m<<" comps="<<this->CellDataInfo[i].veclen<<" tuples="<<this->CellsInMaterial[m]<<" name="<<CellDataArraySelection->GetArrayName(i)<<"\n" );
		     scalars[m] = vtkFloatArray::New();
		     scalars[m]->SetName( CellDataArraySelection->GetArrayName(i) );
		     scalars[m]->SetNumberOfComponents(this->CellDataInfo[i].veclen);
		     scalars[m]->SetNumberOfTuples( this->CellsInMaterial[m] );
		  }
		  for(cell=0;cell<this->NumberOfCells;cell++)
		  {
		     m = mat[cell];
		     p = this->CellIndexInMaterial[cell];

		     if(1) // this->CellDataInfo[i].veclen == 1)
		     {
			scalars[m]->SetTuple( p , ptr+cell*this->CellDataInfo[i].veclen );
		     }
		     else
		     {
			for(int comp=0;comp<this->CellDataInfo[i].veclen;comp++)
			{
			   scalars[m]->SetComponent( p , comp , ptr[comp*this->NumberOfCells+cell] );
			}
		     }
		  }
		  delete [] ptr;
	       }

	       for(m=0;m<this->NumberOfMaterials;m++)
	       {
		  vtkDataArray* dataArray = hasCachedArray ? this->GetCellDataArrayFromCache( m, this->CellDataArraySelection->GetArrayName(i) ) : scalars[m];
		  if( dataArray != 0 )
		  {
		     this->GetOutput(m)->GetCellData()->AddArray(dataArray);
		     if (!this->GetOutput(m)->GetCellData()->GetScalars())
		     {
			this->GetOutput(m)->GetCellData()->SetScalars(dataArray);
			this->GetOutput(m)->GetCellData()->SetActiveScalars (dataArray->GetName());
		     }
		     if( !hasCachedArray ) dataArray->Delete(); // car GetCellDataArrayFromCache retourne un nouvel array, avec un tableau recycle
		  }
	       }
	       if( !hasCachedArray ) delete [] scalars;
	    }
	    else
	    {
	       vtkDebugMacro( << "mono-material\n" );
	       vtkFloatArray *scalars = 0;
	       if( ! hasCachedArray )
	       {		
		  scalars = vtkFloatArray::New();
		  scalars->SetName(CellDataArraySelection->GetArrayName(i));
		  scalars->SetNumberOfComponents(this->CellDataInfo[i].veclen);
		  scalars->SetNumberOfTuples(this->NumberOfCells);
		  if(1) // this->CellDataInfo[i].veclen == 1)
		  {
		     ptr = scalars->GetPointer(0);
		     this->FileStreamSeek( this->CellDataInfo[i].foffset );
		     this->ReadFloatBlock(this->NumberOfCells * 
					  this->CellDataInfo[i].veclen, ptr);
		  }
		  else
		  {
		     ptr = new float[this->NumberOfCells];
		     for(j=0; j < this->CellDataInfo[i].veclen; j++)
		     {
			this->FileStreamSeek( this->CellDataInfo[i].foffset + j*this->NumberOfCells );
			this->ReadFloatBlock(this->NumberOfCells, ptr);
	      
			for(n=0; n < this->NumberOfCells; n++)
			{
			   scalars->SetComponent(n, j, ptr[n]);
			}
		     }
		     delete [] ptr;
		  }
	       }
	       vtkDataArray* dataArray = hasCachedArray ? this->GetCellDataArrayFromCache(0, this->CellDataArraySelection->GetArrayName(i) ) : scalars ;
	       if( dataArray != 0 )
	       {
		  output->GetCellData()->AddArray(dataArray);
		  if (!output->GetCellData()->GetScalars())
		  {
		     output->GetCellData()->SetScalars(dataArray);
		     output->GetCellData()->SetActiveScalars (dataArray->GetName());
		  }
	       }
	       if( ! hasCachedArray ) scalars->Delete();
	    }
	 }
	 this->UpdateProgress( 0.75 + i*0.25/this->NumberOfCellComponents );
      }
   } // end of binary read
   else
   {
      float value;
      int id;
      char buf1[128], c='\0', buf2[128];

      *(this->FileStream) >> this->NumberOfCellComponents;
      if (this->CellDataInfo != 0) delete [] this->CellDataInfo;
      this->CellDataInfo = new DataInfo[this->NumberOfCellComponents];

      for(i=0; i < this->NumberOfCellComponents; i++)
      {
	 *(this->FileStream) >> this->CellDataInfo[i].veclen;
      }
      this->FileStream->get(c); // one more newline to catch

      vtkFloatArray **scalars = new 
	 vtkFloatArray * [this->NumberOfCellComponents];
      for(i=0; i < this->NumberOfCellComponents; i++)
      {
	 j=0;
	 while(this->FileStream->get(c) && c != ',')
	 {
	    buf1[j++] = c;
	 }
	 buf1[j] = '\0';
	// finish here to read the line
	 this->FileStream->get(buf2, 128, '\n'); this->FileStream->get(c);

	 scalars[i] = vtkFloatArray::New();
	 scalars[i]->SetNumberOfComponents(this->CellDataInfo[i].veclen);
	 scalars[i]->SetNumberOfTuples(this->NumberOfCells);
	 scalars[i]->SetName(buf1);
      }

      this->UpdateProgress( 0.85 );

      for(n=0; n < this->NumberOfCells; n++)
      {
	 *(this->FileStream) >> id;
	 for(i=0; i < this->NumberOfCellComponents; i++)
	 {
	    for(j=0; j < this->CellDataInfo[i].veclen; j++)
	    {
	       *(this->FileStream) >> value;
	       scalars[i]->SetComponent(n, j, value);
	    }
	 }
      }

      this->UpdateProgress( 0.95 );

      for(i=0; i < this->NumberOfCellComponents; i++)
      {
	 output->GetCellData()->AddArray(scalars[i]);
	 if (!output->GetCellData()->GetScalars())
	 {
	    output->GetCellData()->SetScalars(scalars[i]);
	 }
	 scalars[i]->Delete();
      }
      delete[] scalars;
   } // end of ASCII read
   vtkDebugMacro( << "End of ReadCellData()\n");
}

//----------------------------------------------------------------------------
const char* vtkCEAucdReader::GetPointArrayName(int index)
{
   return this->PointDataArraySelection->GetArrayName(index);
}

//----------------------------------------------------------------------------
int vtkCEAucdReader::GetPointArrayStatus(const char* name)
{
   return this->PointDataArraySelection->ArrayIsEnabled(name);
}

//----------------------------------------------------------------------------
void vtkCEAucdReader::SetPointArrayStatus(const char* name, int status)
{
   if(GetPointArrayStatus(name) != status)
   {
      if(status)
      {
	 this->PointDataArraySelection->EnableArray(name);
      }
      else
      {
	 this->PointDataArraySelection->DisableArray(name);
      }
      this->Modified();
   }
}

//----------------------------------------------------------------------------
const char* vtkCEAucdReader::GetCellArrayName(int index)
{
   return this->CellDataArraySelection->GetArrayName(index);
}

//----------------------------------------------------------------------------
int vtkCEAucdReader::GetCellArrayStatus(const char* name)
{
   return this->CellDataArraySelection->ArrayIsEnabled(name);
}


//----------------------------------------------------------------------------
void vtkCEAucdReader::SetCellArrayStatus(const char* name, int status)
{
   if(GetCellArrayStatus(name) != status)
   {
      if(status)
      {
	 this->CellDataArraySelection->EnableArray(name);
      }
      else
      {
	 this->CellDataArraySelection->DisableArray(name);
      }
      this->Modified();
   }
}

//----------------------------------------------------------------------------
int vtkCEAucdReader::GetNumberOfCellArrays()
{
   return this->CellDataArraySelection->GetNumberOfArrays();
}

//----------------------------------------------------------------------------
int vtkCEAucdReader::GetNumberOfPointArrays()
{
   return this->PointDataArraySelection->GetNumberOfArrays();
}

//----------------------------------------------------------------------------
void vtkCEAucdReader::EnableAllPointArrays()
{
   this->PointDataArraySelection->EnableAllArrays();
}

//----------------------------------------------------------------------------
void vtkCEAucdReader::DisableAllPointArrays()
{
   this->PointDataArraySelection->DisableAllArrays();
}

//----------------------------------------------------------------------------
void vtkCEAucdReader::EnableAllCellArrays()
{
   this->CellDataArraySelection->EnableAllArrays();
}

//----------------------------------------------------------------------------
void vtkCEAucdReader::DisableAllCellArrays()
{
   this->CellDataArraySelection->DisableAllArrays();
}

//----------------------------------------------------------------------------
int vtkCEAucdReader::GetLabel(char *string, int number, char *label)
{
   int   i, j, k, len;
   char  current;


  // check to make sure that structure is not NULL
   if (string == NULL)
   {
      vtkErrorMacro( << "String is null");
      return 0;
   }

  // search for the appropriate label
   k = 0;
   len = strlen (string);
   for(i = 0; i <= number; i++)
   {
      current = string[k++];
      j = 0;
      while (current != '.')
      {
	// build the label character by character
	 label[j++] = current;
	 current = string[k++];

	// the last character was found
	 if (k > len)
	 {
	   // the nth label was not found, where n = number
	    if (i < number)
	    {
	       return 0;
	    }
	    current = '.';
	 }
      }  // end while
      label[j] = '\0';
   }
  // a valid label was found
   return 1;
}


//----------------------------------------------------------------------------
// Read a block of ints (ascii or binary) and return number read.
int vtkCEAucdReader::ReadIntBlock(int n, int *block)
{
   if (this->BinaryFile)
   {
      this->FileStream->read((char *)block, n * sizeof(int));
      int retVal = this->FileStream->gcount() / sizeof(int);

      if (this->ByteOrder == FILE_LITTLE_ENDIAN)
      {
	 vtkByteSwap::Swap4LERange(block, n);
      }
      else
      {
	 vtkByteSwap::Swap4BERange(block, n);
      }
      return retVal;
   }
   else
   {
      int count = 0;
      for(int i=0; i<n; i++)
      {
	 if (*(this->FileStream) >> block[i])
	 {
	    count++;
	 }
	 else
	 {
	    return 0;
	 }
      }
      return count;
   }
}

//----------------------------------------------------------------------------
int vtkCEAucdReader::ReadFloatBlock(int n, float* block)
{
   if (this->BinaryFile)
   {
      this->FileStream->read((char *)block, n * sizeof(float));
      int retVal = this->FileStream->gcount() / sizeof(int);
      if (this->ByteOrder == FILE_LITTLE_ENDIAN)
      {
	 vtkByteSwap::Swap4LERange(block, n);
      }
      else
      {
	 vtkByteSwap::Swap4BERange(block, n);
      }
      return retVal;
   }
   else
   {
      int count = 0;
      for(int i=0; i<n; i++)
      {
	 if (*(this->FileStream) >> block[i])
	 {
	    count++;
	 }
	 else
	 {
	    return 0;
	 }
      }
    return count;
    }
}



// ------------------------- Data Cache -------------------------------

void vtkCEAucdReader::ClearCache()
{
   if( this->CachedFileName != 0 ) free( this->CachedFileName );
   this->CachedFileName = 0;
   this->CachedFileStream = 0;
   this->CachedGlobalOffset = 0;
   for(int i=0;i<CachedOutputs.size();i++)
   {
      if (this->CachedOutputs[i] != 0) this->CachedOutputs[i]->Delete();
   }
   this->CachedOutputs.clear();
}

void vtkCEAucdReader::PruneCache()
{
   if( OwnStream						||
       this->PerMaterialOutput != this->CachedPerMaterialOutput ||
       this->GlobalOffset != this->CachedGlobalOffset           ||
       strcmp(this->FileName,this->CachedFileName) != 0         ||
       this->FileStream != this->CachedFileStream)
   {
      this->ClearCache();
      vtkDebugMacro(<<"Cache pruned\n");
   }

  if( this->BinaryFile )
  {
     int no = this->CachedOutputs.size();
     for(int i=0;i<no;i++)
     {
	if(this->CachedOutputs[i] != 0 && this->CachedOutputs[i]->GetCellData()!=0) 
	{
	   int ncd = this->CachedOutputs[i]->GetCellData()->GetNumberOfArrays();
	   int npd = this->CachedOutputs[i]->GetPointData()->GetNumberOfArrays();
	   vtkDebugMacro( <<"reuse Output "<<i<<" : npd="<<npd<<", ncd="<<ncd<<"\n" );
	}
     }
  }
}

void vtkCEAucdReader::CacheOutputs()
{
   this->ClearCache();

  this->CachedFileName = (this->FileName!=0) ? strdup( this->FileName ) : 0;
  this->CachedGlobalOffset = this->GlobalOffset;
  this->CachedFileStream = this->FileStream;
  this->CachedPerMaterialOutput = this->PerMaterialOutput;

  if( this->BinaryFile )
  {
     int no = this->GetNumberOfOutputPorts();
     this->CachedOutputs.resize( no , 0 );
     for(int i=0;i<no;i++)
     {
	if (this->GetOutput(i)->GetPoints() != 0)
	{
	   this->CachedOutputs[i] = vtkUnstructuredGrid::New();
	   this->CachedOutputs[i]->Initialize();
	   this->CachedOutputs[i]->Allocate();
	   vtkPoints * points = vtkPoints::New();
	   this->CachedOutputs[i]->SetPoints(points);
	   points->Delete();
	   this->CachedOutputs[i]->GetPoints()->SetData( this->GetOutput(i)->GetPoints()->GetData() );
	   this->CachedOutputs[i]->SetCells( this->GetOutput(i)->GetCellTypesArray(), this->GetOutput(i)->GetCellLocationsArray(), this->GetOutput(i)->GetCells() );
	   int ncd = this->GetOutput(i)->GetCellData()->GetNumberOfArrays();
	   for(int j=0;j<ncd;j++)
	   {
	      vtkDataArray* dst = this->GetOutput(i)->GetCellData()->GetArray(j);
	      this->CachedOutputs[i]->GetCellData()->AddArray( dst );
	   }
	   int npd = this->GetOutput(i)->GetPointData()->GetNumberOfArrays();
	   for(int j=0;j<npd;j++)
	   {
	      this->CachedOutputs[i]->GetPointData()->AddArray( this->GetOutput(i)->GetCellData()->GetArray(j) );
	   }
	   ncd = this->CachedOutputs[i]->GetCellData()->GetNumberOfArrays();
	   npd = this->CachedOutputs[i]->GetPointData()->GetNumberOfArrays();
	   vtkDebugMacro( <<"cache Output "<<i<<" : npd="<<npd<<", ncd="<<ncd<<"\n" );
	}
	else
	{
	   this->CachedOutputs[i] = 0;
	}
     }
  }
}

vtkDataArray* vtkCEAucdReader::GetPointsDataFromCache(int outputPort)
{
   if( outputPort>=0 && outputPort<this->CachedOutputs.size() && this->CachedOutputs[outputPort]!=0 && this->CachedOutputs[outputPort]->GetPoints()!=0 )
   {
      return this->CachedOutputs[outputPort]->GetPoints()->GetData();
   }
   else
   {
      return 0;
   }
}

int vtkCEAucdReader::GetCellsFromCache( int outputPort, vtkUnsignedCharArray* &cellTypes, vtkIdTypeArray* &cellLocations, vtkCellArray* &cellArray )
{
   if( outputPort>=0 && outputPort<this->CachedOutputs.size() && this->CachedOutputs[outputPort]!=0 )
   {
      cellTypes = this->CachedOutputs[outputPort]->GetCellTypesArray();
      cellLocations = this->CachedOutputs[outputPort]->GetCellLocationsArray();
      cellArray = this->CachedOutputs[outputPort]->GetCells();
      return 1;
   }
   else
   {
      cellTypes = 0;
      cellLocations = 0;
      cellArray = 0;
      return 0;
   }
}


vtkDataArray* vtkCEAucdReader::GetCellDataArrayFromCache(int outputPort, const char* arrayName)
{
   if( outputPort>=0 && outputPort<this->CachedOutputs.size() && this->CachedOutputs[outputPort]!=0 && this->CachedOutputs[outputPort]->GetCellData()!=0 )
   {
      vtkDataArray* dst = this->CachedOutputs[outputPort]->GetCellData()->GetArray( arrayName );
     //cout<<"GetCellDataArrayFromCache("<<outputPort<<","<<arrayName<<") = "<<dst<<endl;
     //if(dst!=0) dst->PrintSelf(std::cout,4);
      return dst;
   }
   else
   {
      return 0;
   }   
}

vtkDataArray* vtkCEAucdReader::GetPointDataArrayFromCache(int outputPort, const char* arrayName)
{
   if( outputPort>=0 && outputPort<this->CachedOutputs.size() && this->CachedOutputs[outputPort]!=0 && this->CachedOutputs[outputPort]->GetPointData()!=0 )
   {
      return this->CachedOutputs[outputPort]->GetPointData()->GetArray( arrayName );
   }
   else
   {
      return 0;
   }   
}
