// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtPLOT3DFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_PLOT3D_FILE_FORMAT_H
#define AVT_PLOT3D_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vector>
#include <string>


class     DBOptionsAttributes;
class     vtkPLOT3DReader;


// ****************************************************************************
//  Class: avtPLOT3DFileFormat
//
//  Purpose:
//      A file format reader for PLOT3D files.
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
//  Modifications:
//    Kathleen Biagas, Thu Apr 23 10:36:09 PDT 2015
//    Added 'haveSolutionFile' flag.
//
//    Kathleen Biagas, Fri Jun 26 10:24:26 PDT 2015
//    Change this from type STMD to MTMD.
//    Add solutionFiles, times, haveReadMetaFile, haveProcessedQ, previousTS.
//
//    Kathleen Biagas, Thu Aug 27 12:32:14 PDT 2015
//    Use GetTime instead of GetTimes. Add solutionHasValidTime.
//
// ****************************************************************************

class avtPLOT3DFileFormat : public avtMTMDFileFormat
{
  public:
                          avtPLOT3DFileFormat(const char *, DBOptionsAttributes *);
    virtual              ~avtPLOT3DFileFormat();
    
    virtual int            GetNTimesteps(void);

    virtual const char   *GetType(void) { return "PLOT3D File Format"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

    virtual void           ActivateTimestep(int ts);
    using avtMTMDFileFormat::ActivateTimestep;
  protected:
    vtkPLOT3DReader *reader;
    std::string           visitMetaFile;
    std::string           xFileName;
    std::string           qFileName;
    std::string           solutionRoot;
    std::vector<std::string> solutionFiles;
    bool                  haveSolutionFile;
    bool                  haveReadMetaFile;
    bool                  haveProcessedQ;
    bool                  solutionHasValidTime;
    int                   previousTS;


    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
    virtual double        GetTime(int);


  private:
    bool                  ReadVisItMetaFile(void);
    bool                  ProcessQForTimeSeries(void);
    void                  SetTimeStep(int timeState);
    double                time;
};

#endif


