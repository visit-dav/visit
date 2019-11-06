// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtMTSDFileFormatInterface.h                       //
// ************************************************************************* //

#ifndef AVT_MTSD_FILE_FORMAT_INTERFACE_H
#define AVT_MTSD_FILE_FORMAT_INTERFACE_H

#include <database_exports.h>

#include <avtFileFormatInterface.h>
#include <avtMTSDFileFormat.h>
#include <vector>

class avtIOInformation;

// ****************************************************************************
//  Class: avtMTSDFileFormatInterface
//
//  Purpose:
//      This is an implementation of avtFileFormatInterface for file formats
//      that have Multiple Timesteps and a Single Domain.  It enables such
//      file formats to be treated by the database as if there were multiple
//      domains, but to be written as if there was only one domain.
//
//  Programmer: Hank Childs
//  Creation:   October 8, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars and vtkVectors havebeen deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Fri Mar 14 17:28:55 PST 2003
//    Removed calls that could be put in the base class.
//
//    Brad Whitlock, Wed May 14 09:24:22 PDT 2003
//    Added an optional timeState argument to SetDatabaseMetaData.
//
//    Mark C. Miller, Mon Feb 23 20:38:47 PST 2004
//    Added method, ActivateTimestep
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added bool, forceReadAllCyclesTimes, to SetDatabaseMetaData
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added method SetCycleTimeInDatabaseMetaData
//
//    Jeremy Meredith, Thu Jan 28 13:11:07 EST 2010
//    MTSD files can now be grouped not just into a faux MD format by having
//    more than one block, but also into a longer sequence of MT files,
//    each chunk with one or more timesteps.
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

class DATABASE_API avtMTSDFileFormatInterface : public avtFileFormatInterface
{
  public:
                    avtMTSDFileFormatInterface(avtMTSDFileFormat ***,
                                               int ntsgroups, int nblocks);
    virtual        ~avtMTSDFileFormatInterface();

    virtual vtkDataSet     *GetMesh(int, int, const char *);
    virtual vtkDataArray   *GetVar(int, int, const char *);
    virtual vtkDataArray   *GetVectorVar(int, int, const char *);
    virtual void           *GetAuxiliaryData(const char *var, int, int,
                                             const char *type, void *args,
                                             DestructorFunction &);
    virtual std::string     CreateCacheNameIncludingSelections(std::string,
                                                               int, int);

    virtual void            GetCycles(int, intVector &);
    virtual void            GetTimes(int, doubleVector &);

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
    avtMTSDFileFormat    ***chunks;
    int                     nTimestepGroups;
    int                     nBlocks;
    std::vector<int>        tsPerGroup;
    int                     nTotalTimesteps;

    virtual int             GetNumberOfFileFormats(void)
                              { return nTimestepGroups*nBlocks; };
    virtual avtFileFormat  *GetFormat(int n) const
                              { int block = n % nBlocks;
                                int tsg   = n / nBlocks;
                                return chunks[tsg][block]; };
    void                    GenerateTimestepCounts();
    int                     GetTimestepGroupForTimestep(int ts);
    int                     GetTimestepWithinGroup(int ts);
};


#endif


