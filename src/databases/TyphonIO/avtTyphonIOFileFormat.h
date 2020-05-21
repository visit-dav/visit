// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//                            avtTyphonIOFileFormat.h
// ****************************************************************************

#ifndef AVT_TyphonIO_FILE_FORMAT_H
#define AVT_TyphonIO_FILE_FORMAT_H

#include <typhonio_viz.h>

#include <avtMTMDFileFormat.h>

#include <vector>

class vtkRectilinearGrid;
class vtkStructuredGrid;
class vtkUnstructuredGrid;

class avtMaterial;

// ****************************************************************************
//  Class: avtTyphonIOFileFormat
//
//  Purpose:
//      Reads in TyphonIO files as a plugin to VisIt.
//
//  Programmer: Paul Selby
//  Creation:   February 11, 2015
//
//  Modifications:
//    Paul Selby, Tue 10 Mar 15:16:25 GMT 2015
//    Added ActivateTimestep & Initialize methods along with initialized member
//
//    Paul Selby, Wed 18 Mar 15:36:05 GMT 2015
//    Added Destructor & GetPoint/QuadColinearMesh methods with various file &
//    state members
//
//    Paul Selby, Tue 24 Mar 15:07:51 GMT 2015
//    Enabled GetCycles, GetTimes via GetCyclesAndTimes and stateInfo cache
//
//    Paul Selby, Thu 26 Mar 13:13:59 GMT 2015
//    Added GetPointVar method
//
//    Paul Selby, Thu 18 Jun 13:25:35 BST 2015
//    Added GetAuxiliaryData & GetPointMat methods
//
//    Paul Selby, Thu 18 Jun 16:58:41 BST 2015
//    Added GetQuadVar method
//
//    Paul Selby, Tue 23 Jun 15:50:34 BST 2015
//    Added GetQuadNonColinearMesh method
//
// ****************************************************************************

class avtTyphonIOFileFormat : public avtMTMDFileFormat
{
  public:
                       avtTyphonIOFileFormat(const char *);
    virtual           ~avtTyphonIOFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    virtual void      *GetAuxiliaryData(const char *, int, int, const char *,
                                        void *, DestructorFunction &);

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "TyphonIO"; };
    virtual void           FreeUpResources(void); 
    virtual void           ActivateTimestep(int ts);

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:
    typedef struct
    {
        TIO_Step_t step;
        TIO_Time_t time;
    } TyphonIO_stateInfo;

    // DATA MEMBERS
    bool initialized;
    TIO_File_t fileId;
    std::string filecomment;
    TIO_Size_t nstates;
    TIO_Object_t stateId;
    int currentTimestep;
    std::vector<TyphonIO_stateInfo> stateInfo;

    void                   Initialize();
    void                   PopulateStateInfo();
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    vtkRectilinearGrid    *GetQuadColinearMesh(TIO_Object_t, TIO_Size_t);
    vtkStructuredGrid     *GetQuadNonColinearMesh(TIO_Object_t, TIO_Size_t);
    vtkUnstructuredGrid   *GetPointMesh(TIO_Object_t, TIO_Size_t);
    vtkDataArray          *GetQuadVar(TIO_Object_t, TIO_Size_t, TIO_Centre_t);
    vtkDataArray          *GetPointVar(TIO_Object_t, TIO_Size_t, TIO_Centre_t);
    avtMaterial           *GetPointMat(TIO_Object_t, TIO_Size_t);
};


#endif
