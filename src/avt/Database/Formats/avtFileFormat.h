/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                               avtFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_FILE_FORMAT_H
#define AVT_FILE_FORMAT_H

#include <database_exports.h>

// For NULL
#include <stdlib.h>

#include <string>
#include <vector>

#include <array_ref_ptr.h>

#include <avtDataSelection.h>
#include <avtTypes.h>


class     avtDatabaseMetaData;
class     avtIOInformation;
class     avtVariableCache;


// ****************************************************************************
//  Class:  avtFileFormat
//
//  Purpose:
//      This defines an interfaces that all file formats must conform to.
//
//  Programmer: Hank Childs
//  Creation:   February 22, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Sep 20 14:10:18 PDT 2001
//    Added SetCache.
//
//    Hank Childs, Mon Oct  8 08:50:45 PDT 2001
//    Added hooks for material selection.
//
//    Hank Childs, Thu Mar 21 13:43:14 PST 2002
//    Added mechanisms for closing files for file descriptor management.
//
//    Hank Childs, Sat Sep 20 09:04:49 PDT 2003
//    Added support for tensors.
//
//    Mark C. Miller, 30Sep03, Added support for time varying sil/metadata 
//
//    Mark C. Miller, Mon Feb  9 16:10:16 PST 2004
//    Added method, ActivateTimestep
//
//    Mark C. Miller, Tue Sep 28 19:57:42 PDT 2004
//    Added method, RegisterDataSelections
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added slew of methods related to getting cycle(s)/time(s) from files
//    See notes below for details
//    Added methods to GetCycle/Time from a filename
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added const members for invalid cycles/times. Changed class to use
//    these symbols instead of -INT_MAX and -DBL_MAX
//
//    Kathleen Bonnell, Wed Jul 13 18:28:51 PDT 2005 
//    Added bool to AddScalarVarToMetaData, in order to specify whether
//    the var should be treated as ascii (default -- false). 
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Moved implementations of GuessCycle/GuessTime to .C file. Added
//    optional regular expression to guide the guessing.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Fri Apr  3 23:39:26 CDT 2009
//    Add data member, resultMustBeProducedOnlyOnThisProcessor, which is
//    for telling dynamic decomposition formats when its processor is
//    operating in isolation and thus cannot assume other processors will
//    read some of its data.
//
//    Jeremy Meredith, Fri Jan  8 16:15:02 EST 2010
//    Added ability to turn on stricter file format error checking.
//
// ****************************************************************************

class DATABASE_API avtFileFormat
{
    friend void           FileFormatCloseFileCallback(void *, int);

  public:
                          avtFileFormat();
    virtual              ~avtFileFormat();


    virtual void          ActivateTimestep(void);

    virtual void          FreeUpResources(void);
    void                  RegisterDatabaseMetaData(avtDatabaseMetaData *);
    virtual void          PopulateIOInformation(avtIOInformation &);
    void                  SetCache(avtVariableCache *);

    void                  SetStrictMode(bool v) { strictMode = v;    }
    bool                  GetStrictMode()       { return strictMode; }

    virtual const char   *GetType(void) = 0;
    virtual const char   *GetFilename(void) = 0;

    virtual bool          PerformsMaterialSelection(void) { return false; };
    virtual bool          HasVarsDefinedOnSubMeshes(void) { return false; };
    virtual bool          HasInvariantMetaData(void) const { return true; };
    virtual bool          HasInvariantSIL(void) const      { return true; };
    virtual void          TurnMaterialSelectionOff(void);
    virtual void          TurnMaterialSelectionOn(const char *);

    virtual bool          CanCacheVariable(const char *) { return true; };

    bool                  CanDoStreaming(void)
                              { return canDoStreaming; };

    virtual void          RegisterVariableList(const char *,
                                          const std::vector<CharStrRef> &) {;};

    virtual void          RegisterDataSelections(
                              const std::vector<avtDataSelection_p>&,
                              std::vector<bool> *wasApplied) {;};

    void                  SetResultMustBeProducedOnlyOnThisProcessor(bool b)
                            { resultMustBeProducedOnlyOnThisProcessor = b; };

    //
    // MCM-07Apr05: Below this line, a new design pattern is being introduced
    // in order to make a more uniform interface boundary between format
    // plugins and VisIt. Eventually, I exect to overhaul the entire class
    // according to this new design pattern. The gist of the pattern is that
    // VisIt *always* calls non-virtual, public, wrapper methods that
    // simply turn around and call their virtual, protected counterparts
    // in the class. Plugins override *only* the virtual protected parts
    // of the class. The non-virtual, public versions VisIt calls down to are
    // always named 'FormatXYX' while the virtual, protected counterparts
    // are named 'XYZ'. In this way the plugin developers see only the
    // 'XYZ' symbol. This pattern will also make it easier to collapse
    // the FormatInterface classes into the Format classes in the future.
    //
    //----------------------------------------------------------------------

    void                  FormatGetCycles(std::vector<int>& cycles)
                              { GetCycles(cycles); };
    int                   FormatGetCycle(void) { return GetCycle(); };
    int                   FormatGetCycle(int ts) { return GetCycle(ts); };
    int                   FormatGetCycleFromFilename(const char *f) const
                              { return GetCycleFromFilename(f); };
    void                  FormatGetTimes(std::vector<double>& times)
                              { GetTimes(times); };
    double                FormatGetTime(void) { return GetTime(); };
    double                FormatGetTime(int ts) { return GetTime(ts); };
    double                FormatGetTimeFromFilename(const char *f) const
                              { return GetTimeFromFilename(f); };

    static const int      FORMAT_INVALID_CYCLE;
    static const double   FORMAT_INVALID_TIME;

    static const int      INVALID_CYCLE;
    static const double   INVALID_TIME;

    static int        GuessCycle(const char *fname, const char *re = 0);
    static double     GuessTime(const char *fname, const char *re = 0);

  protected:
    avtVariableCache     *cache;
    avtDatabaseMetaData  *metadata;
    bool                  doMaterialSelection;
    bool                  canDoStreaming;
    bool                  closingFile;
    char                 *materialName;
    std::vector<int>      fileIndicesForDescriptorManager;
    bool                  strictMode;

    // This data member is for file formats that do their 
    // own domain decomposition.
    bool                  resultMustBeProducedOnlyOnThisProcessor;

    //
    // These cannot be const because the format might have to do real work
    // to service these requests. It would probably make sense to make
    // these pure virtual. However, that would force all formats to have
    // to implment them (the desired goal) and invalidate any that currently
    // do not which is an undiserable side effect. And, if we ever did so,
    // we'd have to move their declaration to the STXX or MTXX classes so
    // that a STXX format would not have to implement the int arg versions
    // and MTXX the void versions.
    //
    virtual void          GetCycles(std::vector<int>&) { return; };
    virtual int           GetCycle(void) { return INVALID_CYCLE; };
    virtual int           GetCycle(int) { return INVALID_CYCLE; };
    virtual void          GetTimes(std::vector<double>&) { return; };
    virtual double        GetTime(void) { return INVALID_TIME; };
    virtual double        GetTime(int) { return INVALID_TIME; };

    //
    // These methods are designed so that we can distinguish between
    // the default implementation and one that is overridden in a plugin.
    // A plugin is expected to return INVALID_CYCLE or INVALID_TIME for any
    // situation in which it cannot return what it thinks is a valid
    // cycle/time from a filename. The default methods return something
    // slightly different. The reason we do this is that our guesses are
    // NOT to be trusted, but a plugin's guesses are. So, we need to know
    // the difference.
    //
    virtual int           GetCycleFromFilename(const char *f) const
                              { if (f[0] == '\0') return FORMAT_INVALID_CYCLE; 
                                return GuessCycle(f); };
    virtual double        GetTimeFromFilename(const char *f) const
                              { if (f[0] == '\0') return FORMAT_INVALID_TIME; 
                                return GuessTime(f); };

    void       AddMeshToMetaData(avtDatabaseMetaData *, std::string,
                                 avtMeshType, const double * = NULL, int = 1,
                                 int = 0, int = 3, int = 3);
    void       AddScalarVarToMetaData(avtDatabaseMetaData *, std::string,
                                      std::string, avtCentering,
                                      const double * = NULL,
                                      const bool = false);
    void       AddVectorVarToMetaData(avtDatabaseMetaData *, std::string,
                                      std::string, avtCentering, int = 3,
                                      const double * = NULL);
    void       AddTensorVarToMetaData(avtDatabaseMetaData *, std::string,
                                      std::string, avtCentering, int = 3);
    void       AddSymmetricTensorVarToMetaData(avtDatabaseMetaData *,
                              std::string, std::string, avtCentering, int = 3);
    void       AddMaterialToMetaData(avtDatabaseMetaData *, std::string,
                                     std::string,int,std::vector<std::string>);
    void       AddSpeciesToMetaData(avtDatabaseMetaData *, std::string,
                                    std::string, std::string, int,
                                    std::vector<int>,
                                    std::vector<std::vector<std::string> >);
    void       AddArrayVarToMetaData(avtDatabaseMetaData *,
                                     std::string, std::vector<std::string> &,
                                     std::string, avtCentering);
    void       AddArrayVarToMetaData(avtDatabaseMetaData *, std::string, int,
                                     std::string, avtCentering);

    void          RegisterFile(int);
    void          UnregisterFile(int);
    void          UsedFile(int);
    virtual void  CloseFile(int);
    void          CloseFileDescriptor(int);

  private:
    static double     GuessCycleOrTime(const char *, const char *re = 0);

};


#endif
