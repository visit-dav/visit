// ************************************************************************* //
//                          vtkStimulateReader.C                             //
// ************************************************************************* //

#include <vtkStimulateReader.h>

#include <vtkImageData.h>
#include <vtkObjectFactory.h>
#include <vtkByteSwap.h>
#include <vtkPointData.h>

#include <sys/stat.h>

vtkCxxRevisionMacro(vtkStimulateReader, "$Revision: 1.30 $");
vtkStandardNewMacro(vtkStimulateReader);

vtkStimulateReader::vtkStimulateReader()
{
    haveReadSPRFile = false;
    validSPRFile = false;
}
vtkStimulateReader::~vtkStimulateReader()
{
    ;
}


void vtkStimulateReader::ExecuteInformation()
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

  this->vtkImageReader2::ExecuteInformation();
}


int vtkStimulateReader::OpenFile(void)
{
  // Close file from any previous image
  if (this->File)
    {
    this->File->close();
    delete this->File;
    this->File = NULL;
    }

  char spr_name[1024];
  char sdt_name[1024];

  if (!GetFilenames(this->FileName, spr_name, sdt_name))
  {
      vtkErrorMacro(<<"Could not open file, bad filename: " << this->FileName);
      return 0;
  }

  struct stat fs;
  if ( !stat( sdt_name, &fs) )
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
    int len = strlen(one_file);
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

void vtkStimulateReader::ExecuteData(vtkDataObject *output)
{
  if (!OpenFile())
  {
      vtkErrorMacro(<<"Unable to open file");
      return;
  }

  // A lot of this work should be done by the base class.  But it doesn't
  // appear to be working/is very hard to interface to, so just do a little
  // of the heavy lifting ourselves.
  vtkImageData *data = AllocateOutputData(output);
  vtkDataArray *scalars = data->GetPointData()->GetScalars();
  scalars->SetNumberOfTuples(dims[0]*dims[1]);
  void *ptr = scalars->GetVoidPointer(0);

  File->read((char *)ptr, dims[0]*dims[1]*sizeof(float));
  if (GetSwapBytes())
  {
     vtkByteSwap::SwapVoidRange(ptr, dims[0]*dims[1], sizeof(float));
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

    validSPRFile = true;
    return validSPRFile;
}


