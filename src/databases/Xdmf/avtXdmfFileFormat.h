// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtXdmfFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Xdmf_FILE_FORMAT_H
#define AVT_Xdmf_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vector>

#include <XdmfObject.h>

class XdmfArray;
class XdmfAttribute;
class XdmfDOM;
class XdmfElement;
class XdmfGrid;

class vtkDataArray;
class vtkImageData;
class vtkRectilinearGrid;
class vtkStructuredGrid;
class vtkUnstructuredGrid;

// ****************************************************************************
//  Class: avtXdmfFileFormat
//
//  Purpose:
//      Reads in Xdmf files as a plugin to VisIt.
//
//  Programmer: kleiter -- generated by xml2avt
//  Creation:   Mon Mar 29 15:43:05 PST 2010
//
//  Modifications:
//    Hank Childs, Thu Aug 18 17:22:19 PDT 2011
//    Change timesteps to double (from int).
//
//    Eric Brugger, Wed Dec 12 09:26:59 PST 2012
//    I added support for processing the BaseIndex and GhostOffsets properties
//    for strucutured grids.
//
//    Mark C. Miller, Wed Aug 28 14:43:31 EDT 2019
//    Add HasInvariantXXX methods to support time varying xdmf block count
// ****************************************************************************

class avtXdmfFileFormat: public avtMTMDFileFormat
{
    public:
        avtXdmfFileFormat(const char *);
        virtual ~avtXdmfFileFormat();

        virtual void        GetTimes(std::vector<double> &);

        virtual int GetNTimesteps(void);

        virtual const char *GetType(void)
        {
            return "Xdmf";
        }

        virtual void FreeUpResources(void);

        virtual vtkDataSet *GetMesh(int, int, const char *);
        virtual vtkDataArray *GetVar(int, int, const char *);
        virtual vtkDataArray *GetVectorVar(int, int, const char *);

        bool                  HasInvariantMetaData(void) const;
        bool                  HasInvariantSIL(void) const;


    protected:
        // DATA MEMBERS
        std::string filename;
        std::string firstGrid;
        XdmfGrid * currentGrid;
        XdmfDOM *dom;
        int Stride[3];
        int numGrids;
        std::vector<double> timesteps;

        void AddArrayExpressions(avtDatabaseMetaData *, std::string, std::vector<std::string> &);
        void AddTensorExpressions(avtDatabaseMetaData *, std::string, int, int);
        vtkDataArray* CopyXdmfArray(XdmfArray *, int, int);
        template<typename T> void CopyXdmfArray(XdmfArray *, vtkDataArray *, int, int);
        vtkDataArray * CopyXdmfArrayByPointer(XdmfArray *, int);
        XdmfAttribute * GetAttributeFromName(XdmfGrid *, const char *);
        std::vector<std::string> GetComponentNames(std::string, XdmfInt32, int);
        void GetDims(XdmfGrid *, int[3]);
        void GetDims(int[6], int[3]);
        std::string GetFormattedExpressionName(std::string &);
        XdmfGrid * GetGrid(int);
        int GetMeshDataType(XdmfGrid *);
        int GetNumberOfComponents(XdmfGrid *, XdmfAttribute *);
        long GetNumberOfCellComponents(XdmfGrid *, XdmfAttribute *);
        long GetNumberOfNodeComponents(XdmfGrid *, XdmfAttribute *);
        long GetNumberOfPoints(XdmfGrid *);
        int GetNumberOfSymmetricalTensorComponents(int);
        long GetNumberOfValues(XdmfElement *);
        int GetSpatialDimensions(XdmfInt32);
        int GetTopologicalDimensions(XdmfInt32);
        int GetVTKCellType(XdmfInt32);
        bool GetWholeExtent(XdmfGrid *, int[6]);
        virtual void PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
        vtkRectilinearGrid* ReadRectilinearGrid(XdmfGrid *);
        vtkStructuredGrid* ReadStructuredGrid(XdmfGrid *);
        vtkUnstructuredGrid* ReadUnstructuredGrid(XdmfGrid *);
        void GetStructuredGhostZones(int[3], int[6], vtkDataSet *);
        void ScaleExtents(int[6], int[6], int[3]);
        void SetCurrentGrid(int, const char *);
        XdmfGrid *FirstRealGrid(XdmfGrid *start);
};

#endif
