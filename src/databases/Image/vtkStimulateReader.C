// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          vtkStimulateReader.C                             //
// ************************************************************************* //

#include <vtkStimulateReader.h>

#include <visit-config.h> // For LIB_VERSION_LE

#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkByteSwap.h>
#include <vtkPointData.h>
#include <FileFunctions.h>

#include <fstream>

#ifndef STRNCEQUAL
  #ifdef _WIN32
    #define STRNCEQUAL(a,b,c) strnicmp(a, b, c)
  #else
    #define STRNCEQUAL(a,b,c) strncasecmp(a, b, c)
  #endif
#endif
using std::ifstream;

vtkStandardNewMacro(vtkStimulateReader);

vtkStimulateReader::vtkStimulateReader()
{
  haveReadSPRFile = false;
  validSPRFile = false;
  dataType = FLOAT;
}
vtkStimulateReader::~vtkStimulateReader()
{
  ;
}


int vtkStimulateReader::RequestInformation(
    vtkInformation* vtkNotUsed(request),
    vtkInformationVector** vtkNotUsed(inInfoVec),
    vtkInformationVector* outInfoVec)
{

  char spr_name[1024];
  char sdt_name[1024];

  GetFilenames(this->FileName, spr_name, sdt_name);
  ReadSPRFile(spr_name);

  this->DataExtent[0] = 0;
  this->DataExtent[1] = dims[0]-1;
  this->DataExtent[2] = 0;
  this->DataExtent[3] = dims[1]-1;

  this->SetDataScalarTypeToFloat();

  SetDataByteOrderToLittleEndian();

  return this->vtkImageReader2::RequestInformation(0, 0, outInfoVec);
}


//    Mark C. Miller, Thu Mar 30 16:45:35 PST 2006
//    Made it use VisItStat instead of stat
int vtkStimulateReader::OpenFile(void)
{
  // Close file from any previous image
#if LIB_VERSION_LE(VTK,8,1,0)
  if (this->File)
    {
    this->File->close();
    delete this->File;
    this->File = NULL;
    }
#else
  this->CloseFile();
#endif

  char spr_name[1024];
  char sdt_name[1024];

  if (!GetFilenames(this->FileName, spr_name, sdt_name))
    {
    vtkErrorMacro(<<"Could not open file, bad filename: " << this->FileName);
    return 0;
    }

  FileFunctions::VisItStat_t fs;
  if ( !FileFunctions::VisItStat( sdt_name, &fs) )
    {
#ifdef _WIN32
    this->File = new ifstream(sdt_name, ios::in | ios::binary);
#else
    this->File = new ifstream(sdt_name, ios::in);
#endif
    }
  if (! this->File || this->File->fail())
    {
    vtkErrorMacro(<< "Initialize: Could not open file " << this->FileName);
    return 0;
    }

  vtkDebugMacro(<< "Able to sucessfully open file " << this->FileName);
  return 1;
}


bool vtkStimulateReader::GetFilenames(const char *one_file, char *spr_name, 
                                      char *sdt_name)
{
  if (one_file == NULL)
    return false;
  size_t len = strlen(one_file);
  if (len < 4)
    return false;

  bool isCaps = false;
  bool hasSPR = false;
  bool hasSDT = false;

  if (strcmp(one_file + (len-4), ".spr") == 0)
    hasSPR = true;
  if (strcmp(one_file + (len-4), ".SPR") == 0)
    {
    hasSPR = true;
    isCaps = true;
    }
  if (strcmp(one_file + (len-4), ".sdt") == 0)
      hasSDT = true;
  if (strcmp(one_file + (len-4), ".SDT") == 0)
    {
    hasSDT = true;
    isCaps = true;
    }

  if (!hasSPR && !hasSDT)
    {
    vtkErrorMacro(<< one_file << " cannot be a Stimulate file, "
                  << "no .spr or .sdt extension.");
    return false;
    }

  strcpy(spr_name, one_file);
  strcpy(sdt_name, one_file);
  if (hasSPR)
    if (isCaps)
      strcpy(sdt_name + (len-4), ".SDT");
    else
      strcpy(sdt_name + (len-4), ".sdt");
  else
    if (isCaps)
      strcpy(spr_name + (len-4), ".SPR");
    else
      strcpy(spr_name + (len-4), ".spr");

  return true;
}

//    Mark C. Miller, Fri Apr 23 23:32:51 PDT 2010
//    Added support for sdt types.
void vtkStimulateReader::ExecuteDataWithInformation(
    vtkDataObject *output, vtkInformation* outInfo)
{
  if (!OpenFile())
    {
    vtkErrorMacro(<<"Unable to open file");
    return;
    }

  // A lot of this work should be done by the base class.  But it doesn't
  // appear to be working/is very hard to interface to, so just do a little
  // of the heavy lifting ourselves.
  vtkImageData *data = AllocateOutputData(output, outInfo);
  int size = sizeof(unsigned char); /// TODO: check fix for uninitialized pointer
  data->SetDimensions(dims[0],dims[1],1);
  switch (dataType)
    {
    case UCHAR: 
      data->AllocateScalars(VTK_UNSIGNED_CHAR, 1); 
      size=sizeof(unsigned char); 
      break;
    case SHORT: 
      data->AllocateScalars(VTK_SHORT, 1); 
      size=sizeof(short); 
      break;
    case INT: 
      data->AllocateScalars(VTK_INT, 1); 
      size=sizeof(int); 
      break;
    case FLOAT: 
      data->AllocateScalars(VTK_FLOAT, 1); 
      size=sizeof(float); 
      break;
   }
  void *ptr = data->GetScalarPointer();
  File->read((char *)ptr, dims[0]*dims[1]*size);

  if (GetSwapBytes())
    {
    vtkByteSwap::SwapVoidRange(ptr, dims[0]*dims[1], size);
    }
}


int vtkStimulateReader::CanReadFile(const char* fname)
{
  char spr_name[1024];
  char sdt_name[1024];

  if (GetFilenames(fname, spr_name, sdt_name))
    {
    vtkErrorMacro(<<"Cannot read file: invalid filename " << fname);
    return 0;
    }
  
  ifstream sdt_file(sdt_name);
  if (sdt_file.fail())
    {
    vtkErrorMacro(<<"Cannot read file: invalid sdt_file " << sdt_name);
    return 0;
    }

  return ReadSPRFile(spr_name);
}

//    Mark C. Miller, Fri Apr 23 23:33:16 PDT 2010
//    Added parsing of data type.
bool vtkStimulateReader::ReadSPRFile(const char *spr_name)
{
  if (haveReadSPRFile)
    return validSPRFile;

  haveReadSPRFile = true;
  validSPRFile = false;

  ifstream spr_file(spr_name);
  if (spr_file.fail())
    {
    vtkErrorMacro(<<"Unable to read SPR file " << spr_name << ": file "
                  << "does not exist");
    return false;
    }

  char line[1024];

  spr_file.getline(line, 1024);
  int ndims = atoi(line);
  if (ndims != 2)
    {
    vtkErrorMacro(<<"Unable to read SPR file, ndims =  " << ndims
                  << " and must equal 2.");
    return false;
    }

  spr_file.getline(line, 1024);
  dims[0] = atoi(line);
  if (dims[0] < 0)
    {
    vtkErrorMacro(<<"Unable to read SPR file, dims[0] =  " << dims[0]
                  << " and must be positive.");
    return false;
    }

  spr_file.getline(line, 1024);
  origin[0] = atof(line);
  spr_file.getline(line, 1024);
  step[0] = atof(line);
  if (step[0] < 0.)
    {
    vtkErrorMacro(<<"Unable to read SPR file step in X is negative");
    return false;
    }

  spr_file.getline(line, 1024);
  dims[1] = atoi(line);
  if (dims[1] < 0)
    {
    vtkErrorMacro(<<"Unable to read SPR file, dims[1] =  " << dims[1]
                  << " and must be positive.");
    return false;
    }

  spr_file.getline(line, 1024);
  origin[1] = atof(line);
  spr_file.getline(line, 1024);
  step[1] = atof(line);
  if (step[1] < 0.)
    {
    vtkErrorMacro(<<"Unable to read SPR file step in Y is negative");
    return false;
    }

  if (!spr_file.eof())
    {
    spr_file.getline(line, 1024);
    if (!STRNCEQUAL(line,"byte",4))
      dataType = UCHAR;
    else if (!STRNCEQUAL(line,"word",4))
      dataType = SHORT;
    else if (!STRNCEQUAL(line,"lword",5))
      dataType = INT;
    else if (!STRNCEQUAL(line,"real",4))
      dataType = FLOAT;
    else
      {
      vtkErrorMacro(<<"Unable to support dataType = \"" << line << "\"");
      return false;
      }
    }

  validSPRFile = true;
  return validSPRFile;
}


