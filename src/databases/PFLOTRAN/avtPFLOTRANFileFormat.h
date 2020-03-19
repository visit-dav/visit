// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtPFLOTRANFileFormat.h                        //
// ************************************************************************* //

#ifndef AVT_PFLOTRAN_FILE_FORMAT_H
#define AVT_PFLOTRAN_FILE_FORMAT_H

// Define this symbol BEFORE including hdf5.h to indicate the HDF5 code
// in this file uses version 1.6 of the HDF5 API. This is harmless for
// versions of HDF5 before 1.8 and ensures correct compilation with
// version 1.8 and thereafter. When, and if, the HDF5 code in this file
// is explicitly upgraded to the 1.8 API, this symbol should be removed.
#define H5_USE_16_API
#include <hdf5.h>
#include <avtMTMDFileFormat.h>

#include <vector>
#include <map>
#include <string>


// ****************************************************************************
//  Class: avtPFLOTRANFileFormat
//
//  Purpose:
//      Reads in PFLOTRAN files as a plugin to VisIt.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Apr 24 14:00:58 PST 2008
//
//  Modifications:
//    Jeremy Meredith, Tue Jul  1 12:43:11 EDT 2008
//    Added support for automatic parallel decomposition and parallel I/O
//    via hyperslab reading.
//
//    Jeremy Meredith, Fri Apr  3 12:52:57 EDT 2009
//    Added support for zonal variables, and recentering old-style files
//    that used cell centers for the coordinate arrays.
//
//    Daniel Schep, Thu Aug 26 15:30:18 EDT 2010
//    Added support for vector and material data.
//
//    Jeremy Meredith, Tue Aug 23 17:25:48 EDT 2011
//    Added the virtual GetTimes method; the old way (where we set it in
//    the metadata directly) isn't supported when grouping MT files.
//
//    Jeremy Meredith, Wed Dec 19 13:26:18 EST 2012
//    Add unstructured grid support.
//
// ****************************************************************************

class avtPFLOTRANFileFormat : public avtMTMDFileFormat
{
  public:
                       avtPFLOTRANFileFormat(const char *);
    virtual           ~avtPFLOTRANFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    virtual void      *GetAuxiliaryData(const char *var, int timestep, 
                                         int domain, const char *type, void *args, 
                                         DestructorFunction &df);
    

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    // virtual void        GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "PFLOTRAN"; };
    virtual void           FreeUpResources(void); 


    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:
    hid_t fileID;
    bool unstructured;
    char *filename;
    bool opened;

    int nTime;
    std::vector< std::pair<float,std::string> > times;

    // this section of fields only applies to unstructured grids
    int  ucd_ncells;
    int  ucd_cellstride;
    int  ucd_nverts;
    int  ucd_vertdim;
    hid_t cellsID;
    hid_t vertsID;

    // this section only applies to structured grids
    int domainCount[3];
    int domainIndex[3];
    int globalDims[3];
    int domainGlobalStart[3];
    int domainGlobalCount[3];
    int localRealStart[3];
    int localRealCount[3];
    hid_t dimID[3];
    bool oldFileNeedingCoordFixup;

    void LoadFile(void);
    void AddGhostCellInfo(vtkDataSet *ds);
    void DoDomainDecomposition();

    //HDF5 helper functions.
    bool ReadAttribute( hid_t parentID, const char *attr, void *value );
    bool ReadStringAttribute( hid_t parentID, const char *attr, std::string *value );
    hid_t NormalizeH5Type( hid_t type );


    virtual void PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    // So that the reader can remember what the vector component arrays are
    // when asked for the vector data.
    std::map<std::string, std::vector<std::string> > vectors;
};


#endif
