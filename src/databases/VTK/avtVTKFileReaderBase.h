// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  avtVTKFileReaderBase.h
// ****************************************************************************

#ifndef AVT_VTK_FILE_READER_BASE_H
#define AVT_VTK_FILE_READER_BASE_H

#include <array>
#include <map>
#include <string>
#include <vector>
#include <void_ref_ptr.h>

#include <ExpressionList.h>
#include <avtTypes.h>

class avtDatabaseMetaData;
class vtkDataArray;
class vtkDataSet;
class vtkDataSetAttributes;
class vtkFieldData;
class vtkRectilinearGrid;
class vtkStructuredPoints;
class vtkVisItXMLPDataReader;

class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtVTKFileReaderBase
//
//  Purpose:
//     Base class for various VTK file readers.
//
//  Notes:
//     Bulk of logic originally part of avtVTKFileReader.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0,
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Thu Mar 11 12:53:12 PST 2004
//    Added ConvertStructuredPointsToRGrid.
//
//    Hank Childs, Tue May 24 12:06:52 PDT 2005
//    Added argument to constructor for DB options.
//
//    Eric Brugger, Fri Aug 12 11:28:43 PDT 2005
//    Added GetCycleFromFilename.
//
//    Mark C. Miller, Thu Sep 15 19:45:51 PDT 2005
//    Added GetAuxiliaryData to support materials
//
//    Kathleen Bonnell, Thu Sep 22 15:37:13 PDT 2005
//    Added 'extension' to store file extension.
//
//    Kathleen Bonnell, Thu Jun 29 17:30:40 PDT 2006
//    Added GetTime method.
//
//    Hank Childs, Tue Sep 26 14:09:18 PDT 2006
//    Remove class qualification of method definition, which xlC dislikes.
//
//    Kathleen Bonnell, Wed Jul  9 18:13:50 PDT 2008
//    Added GetCycle method.
//
//    Brad Whitlock, Wed Oct 26 11:01:00 PDT 2011
//    I added vtkCurves.
//
//    Eric Brugger, Mon Jun 18 12:26:52 PDT 2012
//    I enhanced the reader so that it can read parallel VTK files.
//
//    Brad Whitlock, Mon Oct 22 16:35:15 PDT 2012
//    Make it a helper class for the real readers.
//
//    Eric Brugger, Tue Jul  9 09:33:28 PDT 2013
//    I modified the reading of pvti, pvtr and pvts files to handle the case
//    where the piece extent was a subset of the whole extent.
//
//    Kathleen Biagas, Fri Feb  6 06:09:01 PST 2015
//    Added vtk_meshname, to store MeshName provided in file (if present).
//
//    Kathleen Biagas, Thu Aug 13 17:25:03 PDT 2015
//    Add support for groups and block names.
//
//    Eric Brugger, Tue Jun 20 13:44:39 PDT 2017
//    Modified the STSD databases so that they get the database metadata
//    from the first non empty database for a collection of STSD databases
//    that have been grouped into a multi data version using a visit file.
//
//    Mark C. Miller, Mon Mar  9 19:52:43 PDT 2020
//    Add vtk_exprs to support expressions from VTK files.
//
//    Kathleen Biagas, Fri Aug 13, 2021
//    Add virtual tag to PopulateDatabaseMetaData, FreeUpResources and
//    ReadInFile so that avtPVDReader could override them.  Change pieceNames
//    type to vector<string>. Change pieceExtents to vectr<array<int, 6>>.
//
// ****************************************************************************

class avtVTKFileReaderBase
{
  public:
    avtVTKFileReaderBase();
    ~avtVTKFileReaderBase();


    virtual void   FreeUpResources(void);

    virtual double        GetTime(void) = 0;
    virtual int           GetCycle(void) = 0;

  protected:
    static int            INVALID_CYCLE;
    static double         INVALID_TIME;
    static const char    *MESHNAME;
    static const char    *VARNAME;

    std::string           matvarname;
    std::vector<int>      matnos;
    std::vector<std::string> matnames;
    double                vtk_time;
    int                   vtk_cycle;
    std::string           vtk_meshname;
    ExpressionList        vtk_exprs;

    std::map<std::string, vtkRectilinearGrid *> vtkCurves;

    // Helper methods that handle common checks for mesh and var retrieval
    vtkDataSet   *GetMeshFromDataset(vtkDataSet *, const char *);
    vtkDataArray *GetVarFromDataset(vtkDataSet *, const char *);
    void         *GetAuxiliaryDataFromDataset(vtkDataSet *, const char *var,
                     const char *type, DestructorFunction &df);

    // Read standard VTK dataset from filename.
    // ext is used to pass structured extents if available.
    // (eg from a meta file reader).
    vtkDataSet *ReadVTKDataset(const std::string &filename,
                               const int *ext=nullptr);

    // Helper methods for filling in Mesh, Variable, and Material MetaData.
    void        FillMeshMetaData(avtDatabaseMetaData *md, vtkDataSet *ds,
                    const std::string &meshName,
                    const int nGroups, const std::string &GroupPieceName,
                    const std::vector<std::string> &GroupNames,
                    const std::vector<int> &GroupIds,
                    const int nBlocks, const std::string &BlockPieceName,
                    const std::vector<std::string> &BlockNames);

    void        FillMaterialMetaData(avtDatabaseMetaData *md,
                    const std::string &meshName, const std::string &varName,
                    vtkDataArray *arr, vtkDataArray *materialIds);

    void        FillSingleVarMetaData(avtDatabaseMetaData *md,
                    const std::string &meshName, const std::string &varName,
                    int ncomp, int dataType, avtCentering center);

    void        FillVarsMetaData(avtDatabaseMetaData *md,
                    vtkDataSetAttributes *atts,
                    const std::string &meshName,
                    avtCentering center,
                    vtkFieldData *fieldData = nullptr);

private:

    void   CreateCurves(vtkRectilinearGrid *rgrid);

    // Borrowed from avtFileFormat.
    void   AddScalarVarToMetaData(avtDatabaseMetaData *, std::string,
                                  std::string, avtCentering,
                                  const double * = NULL,
                                  const bool = false);
    void   AddVectorVarToMetaData(avtDatabaseMetaData *, std::string,
                                  std::string, avtCentering, int = 3,
                                  const double * = NULL);
    void   AddTensorVarToMetaData(avtDatabaseMetaData *, std::string,
                                  std::string, avtCentering, int = 3);
    void   AddSymmetricTensorVarToMetaData(avtDatabaseMetaData *, std::string,
                                  std::string, avtCentering, int = 3);
    void   AddArrayVarToMetaData(avtDatabaseMetaData *, std::string, int,
                                 std::string, avtCentering);
};


#endif


