// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtJMFileFormat.h                              //
// ************************************************************************* //

#ifndef AVT_JM_FILE_FORMAT_H
#define AVT_JM_FILE_FORMAT_H
#include <avtMTSDFileFormat.h>

#include <vectortypes.h>
#include <map>
#include <set>

#include <PDBFileObject.h>

class avtFileFormatInterface;

// ****************************************************************************
//  Class: avtJMFileFormat
//
//  Purpose:
//      Reads in JM files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Apr 30 15:50:23 PST 2009
//
// ****************************************************************************

class JMFileFormat : public avtMTSDFileFormat
{
public:
    static avtFileFormatInterface *CreateInterface(PDBFileObject *pdb,
         const char *const *filenames, int nList);

                       JMFileFormat(const char * const*);
                       JMFileFormat(const char * const*, PDBFileObject *);
    virtual           ~JMFileFormat();

    bool                   Identify();
    void                   OwnsPDB();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    virtual void          *GetAuxiliaryData(const char *var, int timestep, 
                                            const char *type, void *args, 
                                            DestructorFunction &);

    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);

    virtual void           ActivateTimestep(int);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "Jose Milovich"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

protected:
    struct VarItem
    {
        VarItem();
        ~VarItem();
        void  *DataForTime(int);
        size_t NumBytes() const;

        int       accesses;

        TypeEnum  type; 
        int      *dims;
        int       ndims;
        int       nTotalElements;
        void     *data;
    };

    PDBFileObject                   *pdb;
    bool                             ownsPDB;
    bool                             threeD;
    std::map<std::string, VarItem *> variableCache;
    std::set<std::string>            curveNames;

    void                   GetMaterialNames(stringVector &materialNames);
    VarItem *              ReadVariable(const std::string &var);
    VarItem *              ReadIREG();
    intVector              GetSize(const char *varName);
    void                   MakeRoom();
    size_t                 VariableCacheSize() const;

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
};


#endif
