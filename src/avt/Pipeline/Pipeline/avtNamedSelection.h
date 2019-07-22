// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_NAMED_SELECTION_H
#define AVT_NAMED_SELECTION_H

#include <pipeline_exports.h>
#include <avtContract.h>
#include <avtVector.h>

#include <set>
#include <vector>

#include <visitstream.h>

#include <vtkDataArray.h>

class     avtDataSelection;
class     vtkDataSet;

typedef struct { int d; int z; } IntPair;
class PairCompare {
  public:
    bool operator()(const IntPair &x, const IntPair &y) const
        {
            if (x.d != y.d)
                return (x.d > y.d);
            if (x.z != y.z)
                return (x.z > y.z);
            return false;
        }
};


// ****************************************************************************
//  Class: avtNamedSelection
//
//  Purpose:
//      A selection of identifiers.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2009
//
//  Modifications:
//
//    Hank Childs, Mon Feb 23 10:09:26 PST 2009
//    Added support for setting up data selections for contracts.
//
//    Hank Childs, Mon Apr  6 16:31:10 PDT 2009
//    Add methods for creating condition strings.
//
//    Hank Childs, Mon Jul 13 17:16:00 PDT 2009
//    Added method for determining size of selection.
//
//    Brad Whitlock, Mon Nov  7 13:27:09 PST 2011
//    Lots of changes. Add Allocate, Append, Globalize, GetMatchingIds, etc.
//
//    Dave Pugmire, Thu Mar 15 10:55:22 EDT 2012
//    Support for location named selections. Added GetMatchingLocations,
//    GetMatchingIds, GetIDArray, CheckValid, changed signature for Append.
//
//    Burlen Loring, Mon Sep  7 06:02:27 PDT 2015
//    Use long long for size
//
// ****************************************************************************

class PIPELINE_API avtNamedSelection
{
  public:
                        avtNamedSelection(const std::string &);
    virtual            ~avtNamedSelection();
    
    typedef enum
    {
        ZONE_ID            = 0,
        FLOAT_ID,         /* 1 */
        LOCATIONS         /* 2 */
    } SELECTION_TYPE;

    const std::string  &GetName(void) { return name; };

    virtual void        Read(const std::string &) = 0;
    virtual void        Write(const std::string &) = 0;
    virtual long long   GetSize(void) = 0;
    virtual SELECTION_TYPE  GetType(void) = 0;

    virtual avtContract_p ModifyContract(avtContract_p c0) const = 0;
    virtual avtDataSelection *CreateSelection(void) { return NULL; };

    void SetIdVariable(const std::string &id);
    const std::string &GetIdVariable() const;

    virtual std::string CreateConditionString(void) { return ""; };

    virtual void        Allocate(size_t) = 0;
    virtual void        Append(vtkDataSet *ds) = 0;
    virtual bool        CheckValid(vtkDataSet *ds);

    virtual void        Globalize() = 0;
    virtual void        GetMatchingIds(vtkDataSet *, std::vector<vtkIdType> &) = 0;
    virtual void        GetMatchingLocations(std::vector<avtVector> &);

    static int          MaximumSelectionSize();
  protected:
    vtkDataArray *      GetIDArray(vtkDataSet *ds);

    std::string         name;
    std::string         idVar;
  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                           avtNamedSelection(const avtNamedSelection &) {;};
    avtNamedSelection     &operator=(const avtNamedSelection &) { return *this; };
};


class PIPELINE_API avtZoneIdNamedSelection : public avtNamedSelection
{
  public:
                  avtZoneIdNamedSelection(const std::string &);
    virtual      ~avtZoneIdNamedSelection();
    
    virtual void  Read(const std::string &);
    virtual void  Write(const std::string &);
    virtual long long GetSize(void) { return static_cast<long long>(zoneId.size()); };
    virtual SELECTION_TYPE  GetType(void) { return ZONE_ID; };

    virtual avtContract_p ModifyContract(avtContract_p c0) const;
    virtual avtDataSelection *CreateSelection(void);

    virtual void  Allocate(size_t);
    virtual void  Append(vtkDataSet *ds);

    virtual void  Globalize();
    virtual void  GetMatchingIds(vtkDataSet *, std::vector<vtkIdType> &);

    void SetIdentifiers(int nvals, const int *doms, const int *zones);
  protected:
    bool GetDomainList(std::vector<int> &) const;

    std::vector<int>  domId;
    std::vector<int>  zoneId;

    std::set<IntPair, PairCompare> lookupSet;
};


class PIPELINE_API avtFloatingPointIdNamedSelection : public avtNamedSelection
{
  public:
                  avtFloatingPointIdNamedSelection(const std::string &);
    virtual      ~avtFloatingPointIdNamedSelection();
    
    virtual void  Read(const std::string &);
    virtual void  Write(const std::string &);
    virtual long long GetSize(void) { return static_cast<long long>(ids.size()); }
    virtual SELECTION_TYPE    GetType(void) { return FLOAT_ID; };    

    virtual avtContract_p ModifyContract(avtContract_p c0) const;
    virtual avtDataSelection *CreateSelection(void);
    virtual std::string       CreateConditionString(void);

    virtual void  Allocate(size_t);
    virtual void  Append(vtkDataSet *ds);

    virtual void  Globalize();
    virtual void  GetMatchingIds(vtkDataSet *, std::vector<vtkIdType> &);

    void SetIdentifiers(const std::vector<double> &);
  protected:
    std::vector<double>  ids;
};

class PIPELINE_API avtLocationsNamedSelection : public avtNamedSelection
{
  public:
                  avtLocationsNamedSelection(const std::string &);
    virtual      ~avtLocationsNamedSelection();
    
    virtual void  Read(const std::string &);
    virtual void  Write(const std::string &);
    virtual long long GetSize(void) { return static_cast<long long>(locations.size()); };
    virtual SELECTION_TYPE    GetType(void) { return LOCATIONS; };

    virtual avtContract_p ModifyContract(avtContract_p c0) const;

    virtual void  Allocate(size_t);
    virtual void  Append(vtkDataSet *ds);
    virtual bool  CheckValid(vtkDataSet *ds);

    virtual void  Globalize();
    virtual void  GetMatchingIds(vtkDataSet *, std::vector<vtkIdType> &);
    virtual void  GetMatchingLocations(std::vector<avtVector> &);

  protected:
    
    std::vector<avtVector> locations;
};


#endif


