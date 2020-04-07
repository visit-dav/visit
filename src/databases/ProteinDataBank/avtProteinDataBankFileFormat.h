// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtProteinDataBankFileFormat.h                 //
// ************************************************************************* //

#ifndef AVT_ProteinDataBank_FILE_FORMAT_H
#define AVT_ProteinDataBank_FILE_FORMAT_H

#include <avtSTSDFileFormat.h>

#include <vector>
#include <map>
#include <string>
#include <utility>

class DBOptionsAttributes;


// ****************************************************************************
//  Struct:  Atom
//
//  Purpose:
//    Encapsulates an Atom for a PDB file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
//  Modifications:
//    Jeremy Meredith, Wed Oct 17 11:27:10 EDT 2007
//    Added compound support.
//
// ****************************************************************************
namespace avtProteinDataBankNamespace
{
struct Atom
{
    int   serial;
    char  name[5];
    char  altloc;
    char  resname[4];
    char  chainid;
    int   resseq;
    char  icode;
    float x;
    float y;
    float z;
    float occupancy;
    float tempfactor;
    char  segid[5];
    char  element[3];
    char  charge[3];

    int   atomicnumber;
    int   residuenumber;
    bool  backbone;

    int   compound;

    Atom(const char *line, int compound=0);
    void Print(ostream &out);
};
}


struct ConnectRecord
{
    int a;
    int b[4];

    ConnectRecord(const char *line);
    void Print(ostream &out);
};

// ****************************************************************************
//  Class: avtProteinDataBankFileFormat
//
//  Purpose:
//      Reads in ProteinDataBank files as a plugin to VisIt.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 23, 2006
//
//  Modifications:
//    Brad Whitlock, Thu Mar 23 11:45:32 PDT 2006
//    Added support for PDB title information.
//
//    Jeremy Meredith, Mon Aug 28 17:42:26 EDT 2006
//    Changed to a STSD file format; models are now exposed through 
//    directories, and times require multiple grouped files.
//    Changed molecular data model so that bonds are line elements, not
//    a 4-component array.
//
//    Jeremy Meredith, Wed Oct 17 11:27:10 EDT 2007
//    Added compound support.
//
// ****************************************************************************

class avtProteinDataBankFileFormat : public avtSTSDFileFormat
{
  public:
                       avtProteinDataBankFileFormat(const char *, DBOptionsAttributes *);
    virtual           ~avtProteinDataBankFileFormat() {;};

    virtual const char    *GetType(void)   { return "ProteinDataBank"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);
    virtual vtkDataArray  *GetVectorVar(const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    ifstream in;

    bool metadata_read;
    int  nmodels;
    std::vector< std::vector<avtProteinDataBankNamespace::Atom> >    allatoms;
    std::vector< std::pair<int, int> >  bonds;

    std::vector<ConnectRecord>       connect;
    std::vector<std::string>         compoundNames;

    std::string filename;
    std::string dbTitle;
    void OpenFileAtBeginning();
    void ReadAllMetaData();
    void ReadAtomsForModel(int);
    void CreateBondsFromModel(int);
    void CreateBondsFromModel_Slow(int);
    void CreateBondsFromModel_Fast(int);
};


#endif
