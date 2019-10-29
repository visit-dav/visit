// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtMTMDFileFormatInterface.h                       //
// ************************************************************************* //

#ifndef AVT_MTMD_FILE_FORMAT_INTERFACE_H
#define AVT_MTMD_FILE_FORMAT_INTERFACE_H

#include <database_exports.h>

#include <avtFileFormatInterface.h>
#include <avtMTMDFileFormat.h>

class avtIOInformation;

// ****************************************************************************
//  Class: avtMTMDFileFormatInterface
//
//  Purpose:
//      This is an implementation of avtFileFormatInterface for file formats
//      that have Multiple Timesteps and a Multiple Domain.  This is the basic
//      abstraction used by the generic database.  However, this class is
//      needed to enable those classes that don't use the basic abstraction
//      provided by the generic database to also fit in.
//
//  Programmer: Hank Childs
//  Creation:   April 4, 2003
//
//  Modifications:
//    Brad Whitlock, Wed May 14 09:24:22 PDT 2003
//    Added an optional timeState argument to SetDatabaseMetaData.
//
//    Mark C. Miller, Mon Feb 23 20:38:47 PST 2004
//    Added ActivateTimestep method
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added bool arg, forceReadAllCyclesTimes, to SetDatabaseMetaData
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added method SetCycleTimeInDatabaseMetaData
//
//    Jeremy Meredith, Thu Jan 28 15:49:05 EST 2010
//    MTMD files can now be grouped into longer sequences.
//
//    Hank Childs, Tue Dec 20 14:15:33 CST 2011
//    Add method CreateCacheNameIncludingSelections.
//
//    Brad Whitlock, Thu Jun 19 10:50:25 PDT 2014
//    Pass mesh name to PopulateIOInformation.
//
//    Alister Maguire, Tue Sep 24 11:15:10 MST 2019
//    Added GetCycles and GetTimes.
//
// ****************************************************************************

class DATABASE_API avtMTMDFileFormatInterface : public avtFileFormatInterface
{
  public:
                    avtMTMDFileFormatInterface(avtMTMDFileFormat **,
                                               int ntsgroups);
    virtual        ~avtMTMDFileFormatInterface();

    virtual vtkDataSet     *GetMesh(int, int, const char *);
    virtual vtkDataArray   *GetVar(int, int, const char *);
    virtual vtkDataArray   *GetVectorVar(int, int, const char *);
    virtual void           *GetAuxiliaryData(const char *var, int, int,
                                             const char *type, void *args,
                                             DestructorFunction &);

    virtual void            GetCycles(int, intVector &);
    virtual void            GetTimes(int, doubleVector &);

    virtual std::string     CreateCacheNameIncludingSelections(std::string,
                                                               int, int);


    virtual const char     *GetFilename(int);
    virtual void            SetDatabaseMetaData(avtDatabaseMetaData *md,
                                int timeState = 0,
                                bool forceReadAllCyclesTimes = false);
    virtual void            SetCycleTimeInDatabaseMetaData(
                                avtDatabaseMetaData *md, int ts);
    virtual void            FreeUpResources(int, int);

    virtual void            ActivateTimestep(int ts);

    virtual bool            PopulateIOInformation(int ts, const std::string &meshname,
                                                  avtIOInformation& ioInfo);

  protected:
    int                     nTimestepGroups;
    std::vector<int>        tsPerGroup;
    int                     nTotalTimesteps;
    avtMTMDFileFormat     **chunks;

    virtual int             GetNumberOfFileFormats(void)
                              { return nTimestepGroups; };
    virtual avtFileFormat  *GetFormat(int n) const { return chunks[n]; };

    void                    GenerateTimestepCounts();
    int                     GetTimestepGroupForTimestep(int ts);
    int                     GetTimestepWithinGroup(int ts);
};


#endif


