// ************************************************************************* //
//                               avtSpecies.h                                //
// ************************************************************************* //

#ifndef AVT_SPECIES_H
#define AVT_SPECIES_H

#include <pipeline_exports.h>

#include <string>
#include <vector>

#include <void_ref_ptr.h>


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
    int                              GetNMat(void) { return nSpecies.size(); };
    

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


