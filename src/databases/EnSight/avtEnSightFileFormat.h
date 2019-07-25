// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtEnSightFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_EnSight_FILE_FORMAT_H
#define AVT_EnSight_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <string>
#include <vector>


class     vtkGenericEnSightReader;
class     vtkMultiBlockDataSet;


// ****************************************************************************
//  Class: avtEnSightFileFormat
//
//  Purpose:
//      A file format reader for EnSight files.
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Jul  9 07:37:46 PDT 2004
//    Make the reader be an MTMD file format.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg to PopulateDatabaseMetaData to satisfy new interface
//
//    Brad Whitlock, Tue Jun 27 10:07:36 PDT 2006
//    Added GetTimes method.
//
//    Hank Childs, Thu Feb 23 09:54:50 PST 2012
//    Add support for materials.
//
//    Kathleen Biagas, Thu Aug 10 14:49:13 MST 2017
//    Add support for particle blocks.
//
// ****************************************************************************

class avtEnSightFileFormat : public avtMTMDFileFormat
{
  public:
                          avtEnSightFileFormat(const char *);
    virtual              ~avtEnSightFileFormat();
    
    virtual const char   *GetType(void) { return "EnSight File Format"; };
    
    virtual vtkDataSet   *GetMesh(int, int, const char *);
    virtual vtkDataArray *GetVar(int, int, const char *);
    virtual vtkDataArray *GetVectorVar(int, int, const char *);

    virtual void         *GetAuxiliaryData(const char *var, int, int,
                                           const char *type, void *args,
                                           DestructorFunction &);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    virtual int           GetNTimesteps(void);
    virtual void          GetTimes(std::vector<double> &times);

    virtual void          RegisterVariableList(const char *,
                                               const std::vector<CharStrRef>&);

  protected:
    vtkGenericEnSightReader *reader;
    bool                  doneUpdate;
    std::vector<std::string> matnames;
    bool                  hasParticles;
    std::vector< int >    particleBlockIds;
    std::vector< int >    stdBlockIds;

    void                  PrepReader(int);
    void                  InstantiateReader(const char *);

    int                   ConvertDomainToBlock(int dom, const std::string &);
};


#endif


