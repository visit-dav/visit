// ************************************************************************* //
//                             avtRectFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_RECT_FILE_FORMAT_H
#define AVT_RECT_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vector>
#include <string>
#include <visitstream.h>


class     vtkUnstructuredGrid;


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

    void                                ReadVizFile(ifstream &);
    void                                SetUpDomainConnectivity();
};


#endif
