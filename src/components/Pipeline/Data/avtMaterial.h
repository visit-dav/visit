// ************************************************************************* //
//                              avtMaterial.h                                //
// ************************************************************************* //

#ifndef AVT_MATERIAL_H
#define AVT_MATERIAL_H

#include <pipeline_exports.h>

#include <string>
#include <vector>

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
                                                 const char *domain = NULL);
                                     avtMaterial(int, const
                                                     std::vector<std::string>&,
                                                 int, const int *, int,
                                                 const int *, const int *,
                                                 const int *,
                                                 const float *);

    avtMaterial                     *CreatePackedMaterial() const;

    virtual                         ~avtMaterial();
    static void                      Destruct(void *);

    int                              GetNZones(void)     { return nZones; };
    int                              GetNMaterials(void) { return nMaterials;};
    const std::vector<std::string>  &GetMaterials(void)  { return materials; };
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
    

  protected:
    int                        nMaterials;
    std::vector<std::string>   materials;
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

                               avtMaterial(const avtMaterial*, int,
                                           std::vector<int>,
                                           std::vector<int>);

    void                       Initialize(int,
                                          const std::vector<std::string> &,
                                          const std::vector<bool>&,
                                          int, int, const int *, int,
                                          const int *, int, const int *,
                                          const int *, const int *,
                                          const float *);
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


