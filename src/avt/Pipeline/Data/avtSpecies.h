// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtSpecies.h                                //
// ************************************************************************* //

#ifndef AVT_SPECIES_H
#define AVT_SPECIES_H

#include <pipeline_exports.h>

#include <string>
#include <vector>

#include <void_ref_ptr.h>

class avtMaterial;

// ****************************************************************************
//  Class:  CellSpecInfo
//
//  Purpose:
//    Holds species info for a cell in response to a query.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 19, 2003
//
// ****************************************************************************
struct CellSpecInfo
{
    std::string name;
    float       mf;
    CellSpecInfo(std::string n="", float m=0.) : name(n), mf(m) { }
};

// ****************************************************************************
//  Class: avtSpecies
//
//  Purpose:
//      Holds a species.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtSpecies
{
  public:
                                     avtSpecies(int, const int*,
                                                int, const int*,
                                                const int*, int, const int*,
                                                int, const float *);
                                     avtSpecies(const std::vector<int>,
                                                const std::vector<
                                                  std::vector<std::string> > &,
                                                int, const int*, int,
                                                const int*,int, const float *);
    virtual                         ~avtSpecies();

    static void                      Destruct(void *);

    int                              GetNZones(void)      { return nZones; };
    const std::vector<int>          &GetNSpecies(void)    { return nSpecies; };
    const std::vector<std::vector<std::string> > &
                                     GetSpecies(void)     { return species; };
    const int                       *GetSpeclist(void)    { return speclist; };
    int                              GetMixlen(void)      { return mixlen; };
    const int                       *GetMixSpeclist(void) 
                                                      { return mix_speclist; };
    int                              GetNSpecMF(void)  { return nspecies_mf; };
    const float                     *GetSpecMF(void)   { return species_mf; };
    int                              GetNMat(void) { return static_cast<int>(nSpecies.size()); };
    
    std::vector<CellSpecInfo>        ExtractCellSpecInfo(int c, int m,
                                                         avtMaterial *);

  protected:
    std::vector<int>                         nSpecies;
    std::vector<std::vector<std::string> >   species;
    int                                      nZones;
    int                                     *speclist;
    int                                      mixlen;
    int                                     *mix_speclist;
    int                                      nspecies_mf;
    float                                   *species_mf;

    void             Initialize(const std::vector<int>,
                                const std::vector<std::vector<std::string> > &,
                                int, const int*, int, const int*, int,
                                const float *);
  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtSpecies(const avtSpecies &) {;};
    avtSpecies          &operator=(const avtSpecies &) { return *this; };
};


// ****************************************************************************
//  Class: avtMultiSpecies
//
//  Purpose:
//      Contatins species for many domains.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
// ****************************************************************************

class PIPELINE_API avtMultiSpecies
{
  public:
                                avtMultiSpecies(int);
    virtual                    ~avtMultiSpecies();

    void                        SetDomain(avtSpecies *, int);
    avtSpecies                 *GetDomain(int);

  protected:
    avtSpecies                **species;
    int                         numDomains;
};


#endif


