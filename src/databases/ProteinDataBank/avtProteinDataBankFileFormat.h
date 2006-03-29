// ************************************************************************* //
//                            avtProteinDataBankFileFormat.h                 //
// ************************************************************************* //

#ifndef AVT_ProteinDataBank_FILE_FORMAT_H
#define AVT_ProteinDataBank_FILE_FORMAT_H

#include <avtMTSDFileFormat.h>

#include <vector>
#include <map>
#include <string>

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
// ****************************************************************************
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

    Atom(const char *line);
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
// ****************************************************************************

class avtProteinDataBankFileFormat : public avtMTSDFileFormat
{
  public:
                       avtProteinDataBankFileFormat(const char *, DBOptionsAttributes *);
    virtual           ~avtProteinDataBankFileFormat() {;};

    //
    // This is used to return unconventional data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, const char *type,
    //                                     int timestep, void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    // virtual void        GetCycles(std::vector<int> &);
    // virtual void        GetTimes(std::vector<double> &);
    //

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "ProteinDataBank"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    ifstream in;

    bool metadata_read;
    int  nmodels;
    std::vector< std::vector<Atom> > allatoms;
    std::vector<int>                 bonds[4];

    std::string filename;
    std::string dbTitle;
    void OpenFileAtBeginning();
    void ReadAllMetaData();
    void ReadAtomsForModel(int);
    void CreateBondsFromModel_Slow(int);
    void CreateBondsFromModel_Fast(int);
};


#endif
