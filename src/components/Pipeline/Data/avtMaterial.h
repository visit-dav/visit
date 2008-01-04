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
//                              avtMaterial.h                                //
// ************************************************************************* //

#ifndef AVT_MATERIAL_H
#define AVT_MATERIAL_H

#include <pipeline_exports.h>

#include <visitstream.h>

#include <string>
#include <vector>


class avtMixedVariable;


// ****************************************************************************
//  Class:  CellMatInfo
//
//  Purpose:
//    Holds material info for a cell.
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 24, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Nov 19 14:42:09 PST 2003
//    Added "material number" to CellMatInfo.  This is more for internal
//    use than for displaying; for example, if one were to try to extract
//    species information for each material in this cell.
// ****************************************************************************
struct CellMatInfo
{
    std::string name;
    int         matno;
    float       vf;
    int         mix_index;
    CellMatInfo(std::string n="", int mn=-1, float v=0., int mi=-1)
                                 : name(n), matno(mn), vf(v), mix_index(mi) { }
};

// ****************************************************************************
//  Class:  MatZoneMap
//
//  Purpose:
//    For a given material, holds list of clean zones, a list of mixed zones
//    and for the list of mixed zones, the volume fractions
//
//  Programmer:  Mark C. Miller 
//  Creation:    April 28, 2004 
//
// ****************************************************************************
struct MatZoneMap 
{
    std::string name;          // the material name
    int         matno;         // the material number
    int         numClean;      // length of cleanZones
    int        *cleanZones;    // 0-origin array of clean zone numbers 
    int         numMixed;      // length of mixedZones
    int        *mixedZones;    // 0-origin array of mixed zone numbers
    float      *volFracs;      // volume fractions for each mixed zone
};

// ****************************************************************************
//  Class: avtMaterial
//
//  Purpose:
//      Holds a material.
//
//  Programmer: Hank Childs
//  Creation:   November 6, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Dec 12 13:38:42 PST 2000
//    Added Get() functions for all the properties.
//
//    Jeremy Meredith, Thu Dec 13 11:48:52 PST 2001
//    Made input arrays constants.
//
//    Eric Brugger, Thu May 23 14:36:51 PDT 2002
//    I added support for taking into account array ordering for material
//    lists from structured meshes.
//
//    Jeremy Meredith, Mon Jun 24 13:36:49 PDT 2002
//    Added ExtractCellMatInfo.
//
//    Jeremy Meredith, Thu Aug 15 12:36:14 PDT 2002
//    Added helper functions to work with packing materials.
//
//    Jeremy Meredith, Fri Aug 22 10:33:10 PDT 2003
//    Added another helper function.
//
//    Hank Childs, Wed Feb 18 09:30:12 PST 2004
//    Keep around the original material names.
//
//    Mark C. Miller, Thu Apr 29 12:14:37 PDT 2004
//    Added new constructors for constucting from...
//        a) lists of elements containing each material
//        b) sparse volume fractions arrays
//
//    Hank Childs, Wed Aug 17 10:18:12 PDT 2005
//    Added SimplifyHeavilyMixedZones.
//
//    Thomas R. Treadway, Tue Aug 22 15:58:53 PDT 2006
//    Added allowmat0
//
//    Hank Childs, Thu Sep 20 12:58:17 PDT 2007
//    Added tracking of "reordering materials", which currently only happens
//    when we do simplify heavily mixed zones.  Also added methods for
//    doing the same reordering to mixed variables, since they will get out
//    of synch otherwise ['8082].
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtMaterial
{
  public:
                                     avtMaterial(int nMats,
                                                 const int *mats,
                                                 char **names, int ndims,
                                                 const int *dims,
                                                 int major_order,
                                                 const int *ml, int mixl,
                                                 const int *mixm,
                                                 const int *mixn,
                                                 const int *mixz,
                                                 const float *mixv, 
                                                 const char *domain = NULL,
                                                 int mat0 = 0);
                                     avtMaterial(int, 
                                              const std::vector<std::string>&,
                                              int, const int *, int,
                                              const int *, const int *,
                                              const int *,
                                              const float *);
                                     avtMaterial(int nTotMats,
                                                 const int *mats,
                                                 const char **names,
                                                 const std::vector<MatZoneMap>
                                                     &matMap,
                                                 int ndims, const int *dims,
                                                 int major_order,
                                                 const char *domain = NULL);
                                     avtMaterial(int nTotMats,
                                                 const int *mats,
                                                 char **names,
                                                 int ndims, const int *dims,
                                                 int major_order,
                                                 const float *const *vfracs,
                                                 const char *domain);

    avtMaterial                     *CreatePackedMaterial() const;
    avtMaterial                     *SimplifyHeavilyMixedZones(int) const;

    virtual                         ~avtMaterial();
    static void                      Destruct(void *);

    int                              GetNZones(void)     { return nZones; };
    int                              GetNMaterials(void) { return nMaterials;};
    const std::vector<std::string>  &GetMaterials(void)  { return materials; };
    const std::vector<std::string>  &GetCompleteMaterialList(void) 
                                                     { return all_materials; };
    const int                       *GetMatlist(void)    { return matlist; };
    int                              GetMixlen(void)     { return mixlen; };
    const int                       *GetMixMat(void)     { return mix_mat; };
    const int                       *GetMixZone(void)    { return mix_zone; };
    const int                       *GetMixNext(void)    { return mix_next; };
    const float                     *GetMixVF(void)      { return mix_vf; };

    std::vector<CellMatInfo>         ExtractCellMatInfo(int c) const;
    void                             ExtractCellMatInfo(int c, float *zone_vf,
                                                        int *mix_index) const;
    void                             ExtractCellMatInfo(int c,
                                                        int *mix_index) const;

    std::vector<int>           GetMapUsedMatToMat() { return mapUsedMatToMat; }
    std::vector<int>           GetMapMatToUsedMat() { return mapMatToUsedMat; }

    void                       SetOriginalMaterialOrdering(std::vector<int> &mo)
                                      { originalMaterialOrdering = mo; };
    bool                       ReorderedMaterials(void)
                                      { return (originalMaterialOrdering.size() > 0); } ;
    avtMixedVariable          *ReorderMixedVariable(avtMixedVariable *);

    static void                      Print(ostream &out, int numZones,
                                         const int *matlist,
                                         int mixlen,
                                         const int *mix_mat,
                                         const int *mix_zone,
                                         const float *mix_vf,
                                         const int *mix_next);
    
  protected:
    int                        nMaterials;
    std::vector<std::string>   materials;
    std::vector<std::string>   all_materials;
    int                        nZones;
    int                       *matlist;
    int                        mixlen;
    int                       *mix_mat;
    int                       *mix_next;
    int                       *mix_zone;
    float                     *mix_vf;

    int                        nUsedMats;
    std::vector<int>           mapMatToUsedMat;
    std::vector<int>           mapUsedMatToMat;

    // This is only used after SimplifyHeavilyMixedZones.
    // If it has length 0, then it should be ignored.
    // It will have length 0 most of the time.
    std::vector<int>           originalMaterialOrdering;

                               avtMaterial(const avtMaterial*, int,
                                           std::vector<int>,
                                           std::vector<int>);

    void                       Initialize(int,
                                          const std::vector<std::string> &,
                                          const std::vector<std::string> &,
                                          const std::vector<bool>&,
                                          int, int, const int *, int,
                                          const int *, int, const int *,
                                          const int *, const int *,
                                          const float *);
  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtMaterial(const avtMaterial &) {;};
    avtMaterial         &operator=(const avtMaterial &) { return *this; };
};



// ****************************************************************************
//  Class: avtMultiMaterial
//
//  Purpose:
//      Contatins materials for many domains.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
// ****************************************************************************

class PIPELINE_API avtMultiMaterial
{
  public:
                                avtMultiMaterial(int);
    virtual                    ~avtMultiMaterial();

    void                        SetDomain(avtMaterial *, int);
    avtMaterial                *GetDomain(int);

  protected:
    avtMaterial               **materials;
    int                         numDomains;
};


#endif


