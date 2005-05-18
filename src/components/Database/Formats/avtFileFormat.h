// ************************************************************************* //
//                               avtFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_FILE_FORMAT_H
#define AVT_FILE_FORMAT_H
#include <database_exports.h>

// For NULL
#include <stdlib.h>

#include <limits.h> // for INT_MAX
#include <float.h>  // for DBL_MAX

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

    virtual const char   *GetType(void) = 0;
    virtual const char   *GetFilename(void) = 0;

    virtual bool          PerformsMaterialSelection(void) { return false; };
    virtual bool          HasVarsDefinedOnSubMeshes(void) { return false; };
    virtual bool          HasInvariantMetaData(void) const { return true; };
    virtual bool          HasInvariantSIL(void) const      { return true; };
    virtual void          TurnMaterialSelectionOff(void);
    virtual void          TurnMaterialSelectionOn(const char *);

    virtual bool          CanCacheVariable(const char *) { return true; };

    bool                  CanDoDynamicLoadBalancing(void)
                              { return canDoDynamicLoadBalancing; };

    virtual void          RegisterVariableList(const char *,
                                          const std::vector<CharStrRef> &) {;};

    virtual void          RegisterDataSelections(
                              const std::vector<avtDataSelection_p>&,
                              std::vector<bool> *wasApplied) {;};

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

  protected:
    avtVariableCache     *cache;
    avtDatabaseMetaData  *metadata;
    bool                  doMaterialSelection;
    bool                  canDoDynamicLoadBalancing;
    bool                  closingFile;
    char                 *materialName;
    std::vector<int>      fileIndicesForDescriptorManager;

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
    virtual int           GetCycle(void) { return -INT_MAX; };
    virtual int           GetCycle(int) { return -INT_MAX; };
    virtual void          GetTimes(std::vector<double>&) { return; };
    virtual double        GetTime(void) { return -DBL_MAX; };
    virtual double        GetTime(int) { return -DBL_MAX; };

    //
    // These methods are designed so that we can distinguish between
    // the default implementation and one that is overridden in a plugin.
    // A plugin is expected to return -INT_MAX or -DBL_MAX for any
    // situation in which it cannot return what it thinks is a valid
    // cycle/time from a filename. The default methods return something
    // slightly different. The reason we do this is that our guesses are
    // NOT to be trusted, but a plugin's guesses are. So, we need to know
    // the difference.
    //
    virtual int           GetCycleFromFilename(const char *f) const
                              { if (f[0] == '\0') return -INT_MAX+1; 
                                return GuessCycle(f); };
    virtual double        GetTimeFromFilename(const char *f) const
                              { if (f[0] == '\0') return -DBL_MAX+1.0; 
                                return GuessTime(f); };

    void       AddMeshToMetaData(avtDatabaseMetaData *, std::string,
                                 avtMeshType, const float * = NULL, int = 1,
                                 int = 0, int = 3, int = 3);
    void       AddScalarVarToMetaData(avtDatabaseMetaData *, std::string,
                                      std::string, avtCentering,
                                      const float * = NULL);
    void       AddVectorVarToMetaData(avtDatabaseMetaData *, std::string,
                                      std::string, avtCentering, int = 3,
                                      const float * = NULL);
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

    int        GuessCycle(const char *fname) const
                   { double d = GuessCycleOrTime(fname, false);
                     if (d == -DBL_MAX) return -INT_MAX;
                     return (int) d;};
    double     GuessTime(const char *fname) const
                   { return GuessCycleOrTime(fname, true); };

    void          RegisterFile(int);
    void          UnregisterFile(int);
    void          UsedFile(int);
    virtual void  CloseFile(int);
    void          CloseFileDescriptor(int);

  private:
    double     GuessCycleOrTime(const char *, bool) const;

};


#endif


