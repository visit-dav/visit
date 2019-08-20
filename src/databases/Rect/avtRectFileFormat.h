// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtRectFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_RECT_FILE_FORMAT_H
#define AVT_RECT_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>
#include <avtTypes.h>

#include <vector>
#include <string>
#include <visitstream.h>


class     vtkUnstructuredGrid;

typedef  struct origin_t {
    int x0;
    int y0;
    int z0;
} origin_t;


// ****************************************************************************
//  Class: avtRectFileFormat
//
//  Purpose:
//      A file format reader for multi-timestep, multi-domain files.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  4, 2003
//
//  Modifications:
// 
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg to PopulateDatabaseMetaData
//
//    Mark C. Miller, Wed Jan 16 17:25:43 PST 2008
//    Added support for multiple rect blocks
// ****************************************************************************

class avtRectFileFormat : public avtMTMDFileFormat
{
  public:
                          avtRectFileFormat(const char *);
    virtual              ~avtRectFileFormat();
    
    virtual const char   *GetType(void) { return "Rect File Format"; };
    
    virtual void          GetCycles(std::vector<int> &);
    virtual int           GetNTimesteps(void);
 
    virtual vtkDataSet   *ReadMesh(int, int, const char *);
    virtual vtkDataSet   *GetMesh(int, int, const char *);
    virtual vtkDataArray *GetVar(int, int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

  protected:
    vtkDataSet                        **cachedMeshes;

    std::string                         filename;
    std::string                         basename;
    std::string                         dirname;
    int                                 ndomains;
    int                                 ntimesteps;
    int                                 nvars;
    std::vector<std::string>            varnames;
    int                                 xsize,  ysize,  zsize;
    std::vector<int>                    dxsize, dysize, dzsize, numpts;
    std::vector<origin_t>               origins;
    avtMeshType                         gridType;

    void                                ReadVizFile(istream &);
    void                                SetUpDomainConnectivity();
};


#endif
