/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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


