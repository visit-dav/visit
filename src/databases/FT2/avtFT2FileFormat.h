// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtFT2FileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_FT2_FILE_FORMAT_H
#define AVT_FT2_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>

#include <vector>


// ****************************************************************************
//  Class: avtFT2FileFormat
//
//  Purpose:
//      Reads in FT2 mass spectrometry files (converted by raxport from
//      Thermo Scientific raw files to FT2 format) as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   February  4, 2013
//
// ****************************************************************************
#define FT2_NVARS 5
class avtFT2FileFormat : public avtMTSDFileFormat
{
  public:
                       avtFT2FileFormat(const char *);
    virtual           ~avtFT2FileFormat() {;};

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "FT2"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);

  protected:
    bool                           getLimitsAcrossWholeFile;
    ifstream                       in;
    std::string                    filename;
    bool                           metaDataRead;
    std::vector<istream::pos_type> filePositions;
    std::vector<double>            times;
    std::vector<int>               cycles;
    float                          varmin[FT2_NVARS]; // across the WHOLE file
    float                          varmax[FT2_NVARS]; // across the WHOLE file

    int                 currentTimestep;
    std::vector<float>  vars[FT2_NVARS];

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
    virtual void           GetTimes(std::vector<double>&);
    virtual void           GetCycles(std::vector<int>&);

    void OpenFileAtBeginning();
    void ReadAllMetaData();
    void ReadTimeStep(int);
};


#endif
