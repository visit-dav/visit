// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtCHGCARFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_CHGCAR_FILE_FORMAT_H
#define AVT_CHGCAR_FILE_FORMAT_H

#include <avtFileFormatInterface.h>
#include <avtMTSDFileFormat.h>

class vtkDoubleArray;

// ****************************************************************************
//  Class: avtCHGCARFileFormat
//
//  Purpose:
//      Reads in CHGCAR files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Fri Apr 20 14:59:53 EDT 2007
//    Added a special case where axis-aligned unit cell vectors
//    construct a *true* rectilinear grid, not a transformed one.
//
//    Jeremy Meredith, Wed Jan  2 14:09:05 EST 2008
//    Support multiple concatenated CHGCAR's in a single file; now MTSD.
//
//    Jeremy Meredith, Tue Jul 15 15:41:07 EDT 2008
//    Added support for automatic domain decomposition.
//
//    Jeremy Meredith, Thu Oct 18 11:02:04 EDT 2012
//    Added support for atoms in the CHGCAR file.
//
// ****************************************************************************

class avtCHGCARFileFormat : public avtMTSDFileFormat
{
  public:
    static bool        Identify(const std::string&);
    static avtFileFormatInterface *CreateInterface(
                       const char *const *list, int nList, int nBlock);

                       avtCHGCARFileFormat(const char *filename);
    virtual           ~avtCHGCARFileFormat() {;};

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "CHGCAR"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    std::vector<istream::pos_type>   charge_file_positions;
    std::vector<istream::pos_type>   atom_file_positions;

    int globalZDims[3];
    int globalNDims[3];
    int domainCount[3];
    int domainIndex[3];
    int domainGlobalStart[3];
    int domainGlobalCount[3];
    int localRealStart[3];
    int localRealCount[3];

    double unitCell[3][3];
    bool is_rectilinear;
    ifstream in;
    std::string filename;
    bool metadata_read;
    int  atoms_read;
    int  values_read;
    int  values_per_line;
    vtkDoubleArray *values;
    int ntimesteps;
    int natoms;

    std::vector<float> ax;
    std::vector<float> ay;
    std::vector<float> az;
    std::vector<int>   as; // spoecies

    std::vector<int> species_counts;
    std::vector<int> element_map;

    void OpenFileAtBeginning();
    void ReadAllMetaData();
    void ReadValues(int);
    void AddGhostCellInfo(vtkDataSet *ds);
    void DoDomainDecomposition();
    void ReadAtomsForTimestep(int);
};


#endif
