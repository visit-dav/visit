/*=========================================================================

  Program:   Visualization Toolkit
  Module:    VisItAVSucdReader.cxx

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

#include <VisItAVSucdReader.h>
#include <vtkType.h>
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

#include <map>
#include <string>

using std::ifstream;
using std::string;

vtkStandardNewMacro(VisItAVSucdReader);

// Internal Classes/Structures
struct VisItAVSucdReader::idMapping : public std::map<vtkIdType, vtkIdType>
{};

void
rtrim(string &val)
{
    // trim trailing spaces & tabs
    size_t pos = val.find_last_not_of(" \t");
    if( string::npos != pos)
    {
        val = val.substr( 0, pos+1 );
    }
}

void
ltrim(string &val)
{
    // trim leading spaces & tabs
    size_t pos = val.find_first_not_of(" \t");
    if( string::npos != pos )
    {
        val = val.substr( pos );
    }
}

void
trim(string &val)
{
    rtrim(val);
    ltrim(val);
}

//----------------------------------------------------------------------------
VisItAVSucdReader::VisItAVSucdReader()
{
  this->FileName  = nullptr;
  this->ByteOrder = FILE_BIG_ENDIAN;
  this->BinaryFile = 0;
  this->NumberOfNodeFields = 0;
  this->NumberOfCellFields = 0;
  this->NumberOfFields = 0;
  this->NumberOfNodeComponents = 0;
  this->NumberOfCellComponents = 0;
  this->FileStream = nullptr;
  this->NumberOfNodes = 0;
  this->NumberOfCells = 0;

  this->NodeDataInfo = nullptr;
  this->CellDataInfo = nullptr;

  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
VisItAVSucdReader::~VisItAVSucdReader()
{
  delete [] this->FileName;
  delete [] this->NodeDataInfo;
  delete [] this->CellDataInfo;
}


//----------------------------------------------------------------------------
void VisItAVSucdReader::SetByteOrderToBigEndian()
{
  this->ByteOrder = FILE_BIG_ENDIAN;
}


//----------------------------------------------------------------------------
void VisItAVSucdReader::SetByteOrderToLittleEndian()
{
  this->ByteOrder = FILE_LITTLE_ENDIAN;
}

//----------------------------------------------------------------------------
const char *VisItAVSucdReader::GetByteOrderAsString()
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

//----------------------------------------------------------------------------
int VisItAVSucdReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkDebugMacro( << "Reading AVS UCD file");

  // If ExecuteInformation() failed FileStream will be nullptr and
  // ExecuteInformation() will have spit out an error.
  if ( this->FileStream )
  {
    this->ReadFile(output);
  }

  return 1;
}

//----------------------------------------------------------------------------
void VisItAVSucdReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";

  os << indent << "Number Of Nodes: " << this->NumberOfNodes << endl;
  os << indent << "Number Of Node Fields: "
     << this->NumberOfNodeFields << endl;
  os << indent << "Number Of Node Components: "
     << this->NumberOfNodeComponents << endl;

  for(int i=0; i < this->NumberOfNodeComponents; i++)
  {
    os << indent << indent
                 << "name = " << this->NodeDataInfo[i].name << endl;
    os << indent << indent
                   << "veclen = " << this->NodeDataInfo[i].veclen << endl;
    os << indent << indent
                   << "units = " << this->NodeDataInfo[i].units << endl;
  }

  os << indent << "Number Of Cells: " << this->NumberOfCells << endl;
  os << indent << "Number Of Cell Fields: "
     << this->NumberOfCellFields << endl;
  os << indent << "Number Of Cell Components: "
     << this->NumberOfCellComponents << endl;
  for(int i=0; i < this->NumberOfCellComponents; i++)
  {
    os << indent << indent
                 << "name = " << this->CellDataInfo[i].name << endl;
    os << indent << indent
                   << "veclen = " << this->CellDataInfo[i].veclen << endl;
    os << indent << indent
                   << "units = " << this->CellDataInfo[i].units << endl;
  }

  os << indent << "Byte Order: " << this->ByteOrder << endl;
  os << indent << "Binary File: " << (this->BinaryFile ? "True\n" : "False\n");
  os << indent << "Number of Fields: " << this->NumberOfFields << endl;
}


//----------------------------------------------------------------------------
void VisItAVSucdReader::ReadFile(vtkUnstructuredGrid *output)
{
  idMapping nodeMap, cellMap;

  this->ReadGeometry(output, nodeMap, cellMap);

  if(this->NumberOfNodeFields)
  {
    this->ReadNodeData(nodeMap);
  }

  if(this->NumberOfCellFields)
  {
    this->ReadCellData(cellMap);
  }

  delete this->FileStream;
  this->FileStream = nullptr;
}


//----------------------------------------------------------------------------
int VisItAVSucdReader::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *vtkNotUsed(outputVector))
{
  int i, j, k, *ncomp_list;

  // first open file in binary mode to check the first byte.
  if ( !this->FileName )
  {
    vtkErrorMacro("No filename specified");
    return 0;
  }

#ifdef _WIN32
    this->FileStream = new ifstream(this->FileName, ios::in | ios::binary);
#else
    this->FileStream = new ifstream(this->FileName, ios::in);
#endif
  if (this->FileStream->fail())
  {
    this->SetErrorCode(vtkErrorCode::FileNotFoundError);
    delete this->FileStream;
    this->FileStream = nullptr;
    vtkErrorMacro("Specified filename not found");
    return 0;
  }

  char magic_number='\0';
  this->FileStream->get(magic_number);
  this->FileStream->putback(magic_number);
  if(magic_number != 7)
  { // most likely an ASCII file
    this->BinaryFile = 0;
    delete this->FileStream; // close file to reopen it later
    this->FileStream = nullptr;

    this->FileStream = new ifstream(this->FileName, ios::in);
    char c='\0';
    while (!FileStream->eof())
    {
      // skip leading whitespace
      while(isspace(FileStream->peek()))
      {
        FileStream->get(c);
      }
      // skip comment lines
      if (FileStream->peek() == '#')
      {
        while(this->FileStream->get(c))
        {
          if (c == '\n')
          {
            break;
          }
        }
      }
      else
      {
        break;
      }
    }

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
    // We will read the variables once, with the given endian-ness set up in
    // the class constructor. If trueFileLength does not match
    // calculatedFileLength, then we will toggle the endian-ness and re-swap
    // the variables. We try at most twice, since there are only two endian-nesses.
    this->FileStream->seekg(0L, ios::end);
    vtkTypeUInt64 trueFileLength = this->FileStream->tellg();
    vtkTypeUInt64 calculatedFileLength = 0; // not known yet

    unsigned int attempts = 0;
    while (attempts < 2)
    {
      // restart at beginning of file
      this->FileStream->seekg(0L, ios::beg);

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

      // If we've guessed the wrong endianness, these values will be nonsense,
      // and the arithmetic below could easily caused (undefined) signed overflow,
      // so convert everything into uint64.
      vtkTypeUInt64 numNodes = this->NumberOfNodes;
      vtkTypeUInt64 numCells = this->NumberOfCells;
      vtkTypeUInt64 numNodeFields = this->NumberOfNodeFields;
      vtkTypeUInt64 numCellFields = this->NumberOfCellFields;
      vtkTypeUInt64 numFields = this->NumberOfFields;
      vtkTypeUInt64 numListNodes = this->NlistNodes;

      calculatedFileLength  = 1 + 6*4;
      calculatedFileLength += 16 * numCells + 4 * numListNodes;
      calculatedFileLength += 3*4 * numNodes;
      if(numNodeFields)
      {
        calculatedFileLength += 2052 + numNodeFields*(12 + 4 * numNodes + 4);
      }

      if(numCellFields)
      {
        calculatedFileLength += 2052 + numCellFields*(12 + 4 * numCells + 4);
      }

      if(numFields)
      {
        calculatedFileLength += 2052 + numFields*(4 * 5);
      }

      vtkDebugMacro( << "TFL = " << trueFileLength
                     << "\tCFL = " << calculatedFileLength << endl);

      // We tried. Count our trys.
      attempts++;

      if(trueFileLength == calculatedFileLength)
      {
        // Endianness assumption was correct.
        break;
      }
      else
      {
        // If the lengths don't match, then either:
        // we tried the wrong endian-ness or the file is corrupt.
        // Switch to opposite of what was previously set in constructor.
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

    if(trueFileLength != calculatedFileLength)
    {
      vtkErrorMacro("Calculated file length inconsistent with actual length; file corrupt?");
      return 0;
    }

    const long base_offset = 1 + 6*4;
    char buffer1[1024], buffer2[1024], label[32], units[32];

    long offset = base_offset + 16 * this->NumberOfCells +
      4 * this->NlistNodes + 3 * 4 * this->NumberOfNodes;

    if(this->NumberOfNodeFields)
    {
      this->FileStream->seekg(offset,ios::beg);
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
        this->GetLabel(buffer2, i, units);
        vtkDebugMacro( << i+1 << " :found ND label = " << label
                       << " [" << ncomp_list[i] << "]"
                       << " units: " << units << endl);
        this->NodeDataInfo[i].foffset = offset + k * 4 * this->NumberOfNodes;
        this->NodeDataInfo[i].veclen = ncomp_list[i];
        this->NodeDataInfo[i].name = label;
        this->NodeDataInfo[i].units = units;
        k += ncomp_list[i];
      }
      delete [] ncomp_list;
    }

    if(this->NumberOfCellFields)
    {
      offset += 4 * this->NumberOfNodes * this->NumberOfNodeFields +
        4 * this->NumberOfNodeFields;
      this->FileStream->seekg(offset,ios::beg);
      this->FileStream->read(buffer1, sizeof(buffer1));

      this->FileStream->read(buffer2, sizeof(buffer2)); // read 2nd array of 1024 bytes
      this->ReadIntBlock(1, &this->NumberOfCellComponents);

      ncomp_list = new int[this->NumberOfCellFields];
      this->ReadIntBlock(this->NumberOfCellFields, ncomp_list);

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
        this->GetLabel(buffer2, i, units);
        vtkDebugMacro( << i+1 << " :found CD label = " << label << " ["
                       << ncomp_list[i] << "] units: "<< units << endl);
        this->CellDataInfo[i].foffset = offset + k * 4 * this->NumberOfCells;
        this->CellDataInfo[i].veclen = ncomp_list[i];
        this->CellDataInfo[i].name = label;
        this->CellDataInfo[i].units = units;
        k += ncomp_list[i];
      }
      delete [] ncomp_list;
    }

    if(this->NumberOfFields)
    {
      offset += 4 * this->NumberOfCells * this->NumberOfCellFields +
        4 * this->NumberOfCellFields;
      this->FileStream->seekg(offset,ios::beg);
      this->FileStream->read(buffer1, sizeof(buffer1));
      vtkDebugMacro(<< buffer1 << endl);

      //offset += 1024 + 1024 + 4 + 3 * 4 * this->NumberOfFields;

      for(i=0; i < this->NumberOfFields; i++)
      {
        this->GetLabel(buffer1, i, label);
        vtkDebugMacro( << "found MD label = " << label << endl);
      }
    }
    for(i=0; i < this->NumberOfNodeComponents; i++)
    {
      vtkDebugMacro( << endl << this->NodeDataInfo[i].name
                     << endl
                     << "offset = " << this->NodeDataInfo[i].foffset << endl
                     << "veclen = " << this->NodeDataInfo[i].veclen
                     << "units = " << this->NodeDataInfo[i].units);
    }

    for(i=0; i < this->NumberOfCellComponents; i++)
    {
      vtkDebugMacro( << endl << this->CellDataInfo[i].name
                     << endl
                     << "offset = " << this->CellDataInfo[i].foffset << endl
                     << "veclen = " << this->CellDataInfo[i].veclen
                     << "units = " << this->CellDataInfo[i].units);
    }
  } // end of Binary part

  vtkDebugMacro( << "end of ExecuteInformation\n");

  return 1;
}

//----------------------------------------------------------------------------
void VisItAVSucdReader::GetCellDataRange(int cellComp, int index, float *min, float *max)
{
  if (index >= this->CellDataInfo[cellComp].veclen || index < 0)
  {
    index = 0;  // if wrong index, set it to zero
  }
  *min = this->CellDataInfo[cellComp].min[index];
  *max = this->CellDataInfo[cellComp].max[index];
}

//----------------------------------------------------------------------------
void VisItAVSucdReader::GetNodeDataRange(int nodeComp, int index, float *min, float *max)
{
  if (index >= this->NodeDataInfo[nodeComp].veclen || index < 0)
  {
    index = 0;  // if wrong index, set it to zero
  }
  *min = this->NodeDataInfo[nodeComp].min[index];
  *max = this->NodeDataInfo[nodeComp].max[index];
}

//----------------------------------------------------------------------------
void VisItAVSucdReader::ReadGeometry(vtkUnstructuredGrid *output,
                                   idMapping& nodeMap,
                                   idMapping& cellMap)
{
  // add a material array
  this->Materials->SetNumberOfTuples(this->NumberOfCells);
  this->Materials->SetName("Material Id");

  vtkFloatArray *coords = vtkFloatArray::New();
  coords->SetNumberOfComponents(3);
  coords->SetNumberOfTuples(this->NumberOfNodes);

  if (this->BinaryFile)
  {
    int *types = new int[this->NumberOfCells];
    if(types == nullptr)
    {
      vtkErrorMacro(<< "Error allocating types memory\n");
    }

    vtkIdTypeArray *listcells = vtkIdTypeArray::New();
    // this array contains a list of NumberOfCells tuples
    // each tuple is 1 integer, i.e. the number of indices following it (N)
    // followed by these N integers
    listcells->SetNumberOfValues(this->NumberOfCells + this->NlistNodes);

    this->ReadBinaryCellTopology(this->Materials, types, listcells);
    this->ReadXYZCoords(coords, nodeMap);

    vtkCellArray *cells = vtkCellArray::New();
    cells->SetCells(this->NumberOfCells, listcells);
    listcells->Delete();

    output->SetCells(types, cells);
    cells->Delete();
    delete [] types;
  }
  else
  {
    this->ReadXYZCoords(coords, nodeMap);
    this->ReadASCIICellTopology(this->Materials, output, nodeMap, cellMap);
  }

  vtkPoints *points = vtkPoints::New();
  points->SetData(coords);
  coords->Delete();

  output->SetPoints(points);
  points->Delete();
}


//----------------------------------------------------------------------------
void VisItAVSucdReader::ReadBinaryCellTopology(vtkIntArray *materials,
                                             int *types,
                                             vtkIdTypeArray *listcells)
{
  int i, j, k2=0;
  int *mat = materials->GetPointer(0);
  vtkIdType *list = listcells->GetPointer(0);
  int *ctype = new int[4 * this->NumberOfCells];
  if(ctype == nullptr)
  {
    vtkErrorMacro(<< "Error allocating ctype memory");
    return;
  }

  this->FileStream->seekg(6*4 + 1,ios::beg);
  this->ReadIntBlock(4 * this->NumberOfCells, ctype);

  int *topology_list = new int[this->NlistNodes];
  if(topology_list == nullptr)
  {
    vtkErrorMacro(<< "Error allocating topology_list memory");
    return;
  }

  this->ReadIntBlock(this->NlistNodes, topology_list);
  this->UpdateProgress(0.25);

  for(i=0; i < this->NumberOfCells; i++)
  {
    *list++ = ctype[4*i+2];
    if(ctype[4*i+3] == VisItAVSucdReader::PYR)
    { //UCD ordering is 0,1,2,3,4 => VTK ordering is 1,2,3,4,0
      *list++ = topology_list[++k2] - 1;
      *list++ = topology_list[++k2] - 1;
      *list++ = topology_list[++k2] - 1;
      *list++ = topology_list[++k2] - 1;
      *list++ = topology_list[k2-4] - 1;
      k2++;
    }
     else
     {
       for(j=0; j < ctype[4*i+2]; j++)
       {
         *list++ = topology_list[k2++] - 1;
       }
     }
  }

  delete [] topology_list;

  bool topo[4] = { false, false, false, false};
  for(i=0; i < this->NumberOfCells; i++)
  {
    *mat++ = ctype[4*i+1];
    switch(ctype[4*i+3])
    {
      case VisItAVSucdReader::PT:    types[i] = VTK_VERTEX;     topo[0] = true; break;
      case VisItAVSucdReader::LINE:  types[i] = VTK_LINE;       topo[1] = true; break;
      case VisItAVSucdReader::TRI:   types[i] = VTK_TRIANGLE;   topo[2] = true; break;
      case VisItAVSucdReader::QUAD:  types[i] = VTK_QUAD;       topo[2] = true; break;
      case VisItAVSucdReader::TET:   types[i] = VTK_TETRA;      topo[3] = true; break;
      case VisItAVSucdReader::PYR:   types[i] = VTK_PYRAMID;    topo[3] = true; break;
      case VisItAVSucdReader::PRISM: types[i] = VTK_WEDGE;      topo[3] = true; break;
      case VisItAVSucdReader::HEX:   types[i] = VTK_HEXAHEDRON; topo[3] = true; break;
      default:
        vtkErrorMacro( << "cell type: " << ctype[4*i+3] << "not supported\n");
        delete [] ctype;
        return;
    }
  }
  delete [] ctype;
  if (topo[3])
      this->TopologicalDimension = 3;
  else if (topo[2])
      this->TopologicalDimension = 2;
  else if (topo[1])
      this->TopologicalDimension = 1;
  else
      this->TopologicalDimension = 0;
}


//----------------------------------------------------------------------------
void VisItAVSucdReader::ReadASCIICellTopology(vtkIntArray *materials,
                                            vtkUnstructuredGrid *output,
                                            const idMapping& nodeMap,
                                            idMapping& cellMap)
{
  int i, k;
  vtkIdType list[8];
  int *mat = materials->GetPointer(0);
  std::string ctype;

  output->Allocate();

  bool topo[4] = { false, false, false, false};
  for(i=0; i < this->NumberOfCells; i++)
  {
    vtkIdType id;
    *(this->FileStream) >> id;
    cellMap.insert(std::make_pair(id, static_cast<vtkIdType>(i)));
    *(this->FileStream) >> mat[i];
    *(this->FileStream) >> ctype;
    vtkDebugMacro( << mat[i] << ", " << ctype );
    if(ctype == "pt")
    {
      for(k=0; k < 1; k++)
      {
        *(this->FileStream) >> id;
        list[k] = nodeMap.find(id)->second;
      }
      output->InsertNextCell(VTK_VERTEX, 1, list);
      topo[0] = true;
    }
    else if(ctype == "line")
    {
      for(k=0; k < 2; k++)
      {
        *(this->FileStream) >> id;
        list[k] = nodeMap.find(id)->second;
      }
      output->InsertNextCell(VTK_LINE, 2, list);
      topo[1] = true;
    }
    else if(ctype == "tri")
    {
      for(k=0; k < 3; k++)
      {
        *(this->FileStream) >> id;
        list[k] = nodeMap.find(id)->second;
      }
      output->InsertNextCell(VTK_TRIANGLE, 3, list);
      topo[2] = true;
    }
    else if(ctype == "quad")
    {
      for(k=0; k < 4; k++)
      {
        *(this->FileStream) >> id;
        list[k] = nodeMap.find(id)->second;
      }
      output->InsertNextCell(VTK_QUAD, 4, list);
      topo[2] = true;
    }
    else if(ctype == "tet")
    {
      for(k=0; k < 4; k++)
      {
        *(this->FileStream) >> id;
        list[k] = nodeMap.find(id)->second;
      }
      output->InsertNextCell(VTK_TETRA, 4, list);
      topo[3] = true;
    }
    else if(ctype == "pyr")
    {
      for(k=0; k < 5; k++)
      {
        *(this->FileStream) >> id;
        list[k] = nodeMap.find(id)->second;
      }
      int tmp;
      tmp = list[0];
      list[0] = list[1]; list[1] = list[2]; list[2] = list[3];
      list[3] = list[4]; list[4] = tmp;
      output->InsertNextCell(VTK_PYRAMID, 5, list);
      topo[3] = true;
    }
    else if(ctype == "prism")
    {
      for(k=0; k < 6; k++)
      {
        *(this->FileStream) >> id;
        list[k] = nodeMap.find(id)->second;
      }
      output->InsertNextCell(VTK_WEDGE, 6, list);
      topo[3] = true;
    }
    else if(ctype == "hex")
    {
      for(k=0; k < 8; k++)
      {
        *(this->FileStream) >> id;
        list[k] = nodeMap.find(id)->second;
      }
      output->InsertNextCell(VTK_HEXAHEDRON, 8, list);
      topo[3] = true;
    }
    else
    {
      vtkErrorMacro( << "cell type: " << ctype << " is not supported\n");
      return;
    }
  }  // for all cell, read the indices

  if (topo[3])
      this->TopologicalDimension = 3;
  else if (topo[2])
      this->TopologicalDimension = 2;
  else if (topo[1])
      this->TopologicalDimension = 1;
  else
      this->TopologicalDimension = 0;
}


//----------------------------------------------------------------------------
void VisItAVSucdReader::ReadXYZCoords(vtkFloatArray *coords, idMapping& nodeMap)
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
    vtkIdType id;
    for(i=0; i < this->NumberOfNodes; i++)
    {
      *(this->FileStream) >> id;
      *(this->FileStream) >> ptr[3*i] >> ptr[3*i+1] >> ptr[3*i+2];
      nodeMap.insert(std::make_pair(id, static_cast<vtkIdType>(i)));
    }
  } // end of ASCII read
}


//----------------------------------------------------------------------------
void VisItAVSucdReader::ReadNodeData(const idMapping& nodeMap)
{
  if(this->BinaryFile)
  {
    this->ReadBinaryData(this->NumberOfNodes, this->NumberOfNodeComponents,
                         this->NodeDataInfo);
  }
  else
  {
    this->ReadASCIIData(this->NumberOfNodes, this->NumberOfNodeComponents,
                        this->NodeDataInfo, nodeMap);
  }
}


//----------------------------------------------------------------------------
void VisItAVSucdReader::ReadCellData(const idMapping& cellMap)
{
  if(this->BinaryFile)
  {
    this->ReadBinaryData(this->NumberOfCells, this->NumberOfCellComponents,
                         this->CellDataInfo);
  }
  else
  {
    this->ReadASCIIData(this->NumberOfCells, this->NumberOfCellComponents,
                        this->CellDataInfo, cellMap);
  }
}

//----------------------------------------------------------------------------
void VisItAVSucdReader::ReadBinaryData(const int numTuples, const int numComp,
                                     DataInfo *&dataInfo)
{
  float *ptr;
  for (int i=0; i < numComp; ++i)
  {
    dataInfo[i].data->SetNumberOfComponents(dataInfo[i].veclen);
    dataInfo[i].data->SetNumberOfTuples(numTuples);
    dataInfo[i].data->SetName(dataInfo[i].name.c_str());
    this->FileStream->seekg(dataInfo[i].foffset, ios::beg);
    ptr = dataInfo[i].data->GetPointer(0);
    this->ReadFloatBlock(numTuples * dataInfo[i].veclen, ptr);
  }
}


//----------------------------------------------------------------------------
void VisItAVSucdReader::ReadASCIIData(const int numTuples, int &numComp,
                                      DataInfo *&dataInfo , const idMapping &idMap)
{
  vtkDebugMacro( << "Start of ReadASCIIData()\n");

  *(this->FileStream) >> numComp;
  dataInfo = new DataInfo[numComp];

  for(int i=0; i < numComp; ++i)
  {
    *(this->FileStream) >> dataInfo[i].veclen;
  }
  string name;
  string units;
  // one more newline to catch (use getline instead of get(c) as there may be whitespace)
  std::getline(*(this->FileStream), name);

  for(int i=0; i < numComp; ++i)
  {
    std::getline(*(this->FileStream), name, ',');
    std::getline(*(this->FileStream), units);
    trim(name);
    trim(units);

    dataInfo[i].data->SetNumberOfComponents(dataInfo[i].veclen);
    dataInfo[i].data->SetNumberOfTuples(numTuples);
    dataInfo[i].data->SetName(name.c_str());
    dataInfo[i].name = name;
    dataInfo[i].units = units;
  }

  float value;
  for(int n=0; n < numTuples; ++n)
  {
    vtkIdType id;
    *(this->FileStream) >> id;
    id = idMap.find(id)->second;
    for(int i=0; i < numComp; ++i)
    {
      for(int j=0; j < dataInfo[i].veclen; ++j)
      {
        *(this->FileStream) >> value;
        dataInfo[i].data->SetComponent(id, j, value);
      }
    }
  }
  vtkDebugMacro( << "End of ReadASCIIData()\n");
}


//----------------------------------------------------------------------------
const char* VisItAVSucdReader::GetPointArrayName(int index)
{
  if(this->NodeDataInfo && index >= 0 && index < this->NumberOfNodeComponents)
      return this->NodeDataInfo[index].name.c_str();
  return nullptr;
}

const char* VisItAVSucdReader::GetPointArrayUnits(int index)
{
  if(this->NodeDataInfo && index >= 0 && index < this->NumberOfNodeComponents)
      return this->NodeDataInfo[index].units.c_str();
  return nullptr;
}

int VisItAVSucdReader::GetPointArrayVecLength(int index)
{
  if(this->NodeDataInfo && index >= 0 && index < this->NumberOfNodeComponents)
      return this->NodeDataInfo[index].veclen;
  return 0;
}

vtkDataArray *
VisItAVSucdReader::GetPointArray(int index)
{
  if(this->NodeDataInfo && index >= 0 && index < this->NumberOfNodeComponents)
      return this->NodeDataInfo[index].data;
  return nullptr;
}

vtkDataArray *
VisItAVSucdReader::GetPointArray(const char *name)
{
  string sname(name);
  for (int i = 0; i <  this->NumberOfNodeComponents; ++i)
  {
      if (sname == this->NodeDataInfo[i].name)
          return (vtkDataArray*)this->NodeDataInfo[i].data;
  }
  return nullptr;
}



//----------------------------------------------------------------------------
const char* VisItAVSucdReader::GetCellArrayName(int index)
{
  if(this->CellDataInfo && index >= 0 && index < this->NumberOfCellComponents)
      return this->CellDataInfo[index].name.c_str();
  return nullptr;
}

const char* VisItAVSucdReader::GetCellArrayUnits(int index)
{
  if(this->CellDataInfo && index >= 0 && index < this->NumberOfCellComponents)
      return this->CellDataInfo[index].units.c_str();
  return nullptr;
}

int VisItAVSucdReader::GetCellArrayVecLength(int index)
{
  if(this->CellDataInfo && index >= 0 && index < this->NumberOfCellComponents)
      return this->CellDataInfo[index].veclen;
  return 0;
}

vtkDataArray *
VisItAVSucdReader::GetCellArray(int index)
{
  if(this->CellDataInfo && index >= 0 && index < this->NumberOfCellComponents)
      return this->CellDataInfo[index].data;
  return nullptr;
}

vtkDataArray *
VisItAVSucdReader::GetCellArray(const char *name)
{
  string sname(name);
  if (sname == "Material Id")
      return (vtkDataArray*)this->Materials;
  for (int i = 0; i <  this->NumberOfCellComponents; ++i)
  {
      if (sname == this->CellDataInfo[i].name)
          return (vtkDataArray*)this->CellDataInfo[i].data;
  }
  return nullptr;
}


//----------------------------------------------------------------------------
int VisItAVSucdReader::GetNumberOfCellArrays()
{
  return this->NumberOfCellComponents + 1;
}

//----------------------------------------------------------------------------
int VisItAVSucdReader::GetNumberOfPointArrays()
{
  return this->NumberOfNodeComponents;
}


//----------------------------------------------------------------------------
int VisItAVSucdReader::GetLabel(char *instring, int number, char *label)
{
  int   i, j, k, len;
  char  current;


  // check to make sure that structure is not nullptr
  if (instring == nullptr)
  {
    vtkErrorMacro( << "String is null");
    return 0;
  }

  // search for the appropriate label
  k = 0;
  len = static_cast<int>(strlen(instring));
  for(i = 0; i <= number; i++)
  {
    current = instring[k++];
    j = 0;
    while (current != '.')
    {
      // build the label character by character
      label[j++] = current;
      current = instring[k++];

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
int VisItAVSucdReader::ReadIntBlock(int n, int *block)
{
  if (this->BinaryFile)
  {
    this->FileStream->read((char *)block, n * sizeof(int));
    int retVal = static_cast<int>(this->FileStream->gcount()) / sizeof(int);

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
int VisItAVSucdReader::ReadFloatBlock(int n, float* block)
{
  if (this->BinaryFile)
  {
    this->FileStream->read((char *)block, n * sizeof(float));
    int retVal = static_cast<int>(this->FileStream->gcount()) / sizeof(int);
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
