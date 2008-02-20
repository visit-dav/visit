/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                           avtFileFormatInterface.h                        //
// ************************************************************************* //

#ifndef AVT_FILE_FORMAT_INTERFACE_H
#define AVT_FILE_FORMAT_INTERFACE_H

#include <database_exports.h>

#include <vector>

#include <void_ref_ptr.h>
#include <array_ref_ptr.h>
#include <avtDataSelection.h>

class    vtkDataSet;
class    vtkDataArray;

class    avtDatabaseMetaData;
class    avtFileFormat;
class    avtIOInformation;
class    avtVariableCache;


// ****************************************************************************
//  Class: avtFileFormatInterface
//
//  Purpose:
//      This class goes between a generic database and a file format.  It
//      serves two purposes.  One, it allows the interface of a file format to
//      not be too crowded.  Some function calls are not applicable for certain
//      file formats and this screens those functions out.  This is only
//      worthwhile because it is anticipated that many derived types of file
//      format will be written while only a few file format interfaces will be
//      written.  Two, this enables file formats to be used in ways that the
//      writers of the file format did not intend.  For example, the Silo file
//      format is a single timestep file format, but there are often multiple 
//      Silo files for multiple time steps.  This interface then would worry
//      about making those multiple time steps appear to the database while
//      the Silo files can worry only about one time step at a time.
//
//      MCM, Added 23Feb04
//
//      Two classes go hand-in-hand here; the FormatInterface classes and
//      the Format classes. Every FormatInterface HAS A (sometimes more than
//      one) Format object. avtFileFormatInterface is the base class for
//      for FormatInterface objects while avtFileFormat is the base class
//      for Format objects.
//
//      In VisIt, the abstract FormatInterface supports the notion that a mesh
//      is composed of various pieces (called domains) at various timesteps.
//      The general request for data from VisIt through the FormatInterface to a
//      plugin looks something like GetData(int domainNumber, int timeStep).
//      Of course, not all real file formats that Visit reads from actually
//      support either the notions of pieces of mesh or of time. Consequently,
//      there are four basic FormatInterface classes defined by VisIt derived
//      from avtFileFormatInterface. These four derived classes are used to
//      distinguish how the FormatInterface operates on one of many domains
//      and/or timesteps. The four FormatInterfaces are...
//
//         1) single-timestep, single-domain (STSD)
//         2) single-timestep, multiple-domain (STMD)
//         3) multiple-timestep, single-domain (MTSD)
//         4) multiple-timestep, multiple-domain (MTMD)
//
//     For example, an STSD FormatInterface supports the notion of only a single
//     timestep and a single domain. It will only ever recieve requests from VisIt
//     of the form GetData(). Note that the timeStep and domainNumber arguments
//     are not present because the FormatInterface cannot interpret them.
//     Note also that this DOES NOT IMPLY that one cannot handle time-varying
//     and/or multiple domain data through an STSD FormatInterface. Every
//     FormatInterface object HAS A Format object, one or more. An STSD 
//     FormatInterface in fact has many Format objects, one for each timestep
//     and each domain. Each of these Format objects handles one timestep
//     and one domain. The STSDFormatInterface is responsible for selecting
//     which among the many domains and which among the many timesteps for
//     a given operation. On the other hand, an MTMD FormatInterface has
//     a single Format object because all classes derived from an MTMD
//     type of FormatInterface support, natively, there own notions of
//     domains and timesteps.
//
//     Here are some rules of thumb to use in deciding where to implement
//     new functionality.
//
//     1) Don't define new funtions in any of the FormatInterface objects as
//        pure virtual, UNLESS ABSOLUTELY NECESSARY. The effect of introducing
//        pure virtual functions to the FormatInterface is that ALL derived
//        classes (e.g. the database plugins) ARE REQUIRED to implement them.
//        All the plugins would have to be updated.
//
//     2) Don't define new functions in any of the FormatInterface objects
//        as non-virtual because it means the derived classes (e.g. the
//        database plugins) won't be able to override the behavior if needed.
//        If there is functionality needed in the FormatInterface that
//        shall never be allowed to be overridden in the plugins, then it
//        should probably by implemented in the base classes for the Format
//        and FormatInterface.
//
//     3) If the interface to the new functionality is not FormatInterface
//        specific (e.g. it has no formal arguments for time or domain),
//        you can probably get by with implementing it in the base classes,
//        avtFileFormat and avtFileFormatInterface. Of course, it should still
//        be virtual so plugins can override it. Otherwise, you should
//        implement it in BOTH the bases classes and the FormatInterface
//        specific classes.
//
//  Programmer: Hank Childs
//  Creation:   February 22, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Aug 13 15:49:26 PDT 2001
//    Added GetType.
//
//    Hank Childs, Thu Sep 20 14:07:06 PDT 2001
//    Added SetCache.
//
//    Hank Childs, Fri Oct  5 16:37:03 PDT 2001
//    Added PerformsMaterialSelection and hooks for materials.
//
//    Hank Childs, Thu Oct 25 16:56:18 PDT 2001
//    Added CanDoDynamicLoadBalancing.
//
//    Hank Childs, Wed Jan  9 14:11:28 PST 2002 
//    Added virtual destructor.
//
//    Hank Childs, Tue Mar 12 11:25:42 PST 2002
//    Added GetFilename.
//
//    Kathleen Bonnell, Mon Mar 18 17:15:28 PST 2002 
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Kathleen Bonnell, Mon Mar 18 17:15:28 PST 2002 
//    vtkScalars and vtkVectors have been deprecated in VTK 4.0, 
//    use vtkDataArray instead.
//
//    Hank Childs, Fri Mar 14 17:15:00 PST 2003
//    Removed a number of pure virtual functions, since they could be
//    accomplished in the base class.
//
//    Brad Whitlock, Wed May 14 09:22:02 PDT 2003
//    I added an optional int argument to SetDatabaseMetaData.
//
//    Mark C. Miller, Wed Dec 10 10:31:52 PST 2003
//    I added method CanCacheVariable()
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
//    Mark C. Miller, Mon Feb  9 16:00:10 PST 2004
//    Added method, ActivateTimestep
//
//    Mark C. Miller, Mon Feb 23 20:38:47 PST 2004
//    Added ts argument to ActivateTimestep and made it pure virtual 
//
//    Mark C. Miller, Tue Mar 16 14:09:43 PST 2004
//    Added time step argument to PopulateIOInformation. Made it pure virtual
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added method, RegisterDataSelections
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added bool arg, forceReadAllCyclesAndTimes, to SetDatabaseMetaData
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added method SetCycleTimeInDatabaseMetaData
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
// ****************************************************************************

class DATABASE_API avtFileFormatInterface
{
  public:
    virtual                ~avtFileFormatInterface();

    virtual vtkDataSet     *GetMesh(int, int, const char *) = 0;
    virtual vtkDataArray   *GetVar(int, int, const char *) = 0;
    virtual vtkDataArray   *GetVectorVar(int, int, const char *) = 0;
    virtual void           *GetAuxiliaryData(const char *, int, int, 
                                             const char *, void *, 
                                             DestructorFunction &) = 0;

    virtual const char     *GetFilename(int) = 0;
    virtual void            SetDatabaseMetaData(avtDatabaseMetaData *md,
                                int timeState = 0,
                                bool forceReadAllCyclesTimes = false) = 0;
    virtual void            SetCycleTimeInDatabaseMetaData(
                                avtDatabaseMetaData *md, int ts) = 0;

    virtual void            FreeUpResources(int, int) = 0;
    virtual void            ActivateTimestep(int ts) = 0;
    virtual void            PopulateIOInformation(int ts, avtIOInformation &) = 0;

    bool                    HasInvariantMetaData(void);
    bool                    HasInvariantSIL(void);

    bool                    CanCacheVariable(const char *);

    bool                    CanDoStreaming(void);
    const char             *GetType(void);
    bool                    HasVarsDefinedOnSubMeshes(void);
    bool                    PerformsMaterialSelection(void);
    void                    RegisterVariableList(const char *,
                                              const std::vector<CharStrRef> &);
    void                    SetCache(avtVariableCache *);
    void                    TurnMaterialSelectionOff(void);
    void                    TurnMaterialSelectionOn(const char *);

    void                    RegisterDataSelections(
                                const std::vector<avtDataSelection_p>& selList,
                                std::vector<bool> *wasApplied);

  protected:
    virtual int             GetNumberOfFileFormats(void) = 0;
    virtual avtFileFormat  *GetFormat(int) const = 0;
};


#endif


