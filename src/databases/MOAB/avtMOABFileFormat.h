// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtMOABFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_MOAB_FILE_FORMAT_H
#define AVT_MOAB_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <set>
class DBOptionsAttributes;

namespace moab
{
  class Core;
  class ParallelComm;
}
struct mhdf_FileDesc ;

// ****************************************************************************
//  Class: avtMOABFileFormat
//
//  Purpose:
//      Reads in MOAB files as a plugin to VisIt.
//
//  Programmer: vijaysm -- generated by xml2avt
//  Creation:   Wed Jan 20 13:02:35 PST 2016
//
// ****************************************************************************

class avtMOABFileFormat : public avtSTMDFileFormat
{
  public:
                       avtMOABFileFormat(const char *, const DBOptionsAttributes *);
    virtual           ~avtMOABFileFormat() ;

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, int domain,
    //                                     const char *type, void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the cycle number, overload this function.
    // Otherwise, VisIt will make up a reasonable one for you.
    //
    // virtual int         GetCycle(void);
    //

    virtual const char    *GetType(void)   { return "MOAB"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

  protected:
    // DATA MEMBERS

    struct tagBasic {
      std::string nameTag;
      int size;
      void * defValue; // size 4 or 8 usually
      int type; // mhdf_INTEGER = 1,    /**< Integer type */
                // mhdf_FLOAT = 2,      /**< Floating point value */ (double)
    };
    struct compare1 {
      bool operator () (const tagBasic& lhs, const tagBasic& rhs) const
      {
        if (lhs.nameTag == rhs.nameTag)
          return (lhs.size < rhs.size);
        else
          return (lhs.nameTag < rhs.nameTag);
      }
    };


    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    void                   gatherMhdfInformation();
    vtkDataArray*          GetPartitionTagAsEnumScalar();
    vtkDataArray*          GetMaterialTagAsEnumScalar();
    vtkDataArray*          GetNeumannSetsVar();
    vtkDataArray*          GetDirichletSetsVar();
    vtkDataArray*          GetGeometrySetsVar();
    moab::Core*            mbCore;
    const char*            fileName;
    const DBOptionsAttributes *  readOptions;
    bool                   fileLoaded;
    struct mhdf_FileDesc *       file_descriptor;
    std::set<int>          materials;
    std::set<int>          neumannsets;
    std::set<int>          dirichsets;

    std::set<struct tagBasic, compare1>       elemTags;
    std::vector<struct tagBasic>    nodeTags;
    int                    rank, nProcs;
    int                    num_parts; // PARALLEL_PARTITIONs
    int                    num_mats; // MATERIAL_SETs
    int                    num_neumann; // NEUMANN_SETs
    int                    num_diri;    // DIRICHLET_SETs
    int                    num_geom;    // geometry dimension sets
    moab::ParallelComm*    pcomm;
};



#endif
