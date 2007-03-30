// ************************************************************************* //
//                                  avtSpecies.C                             //
// ************************************************************************* //

#include <stdio.h>         // for sprintf
#include <avtSpecies.h>

#include <BadDomainException.h>


using namespace std;


// ****************************************************************************
//  Method: avtSpecies constructor
//
//  Arguments:
//    nm        the number of materials
//    ns        the number of species for each material
//    ndims     the number of dimensions
//    dims      the dimensions
//    sl        the species speclist
//    ml        the mixlen of the material object
//    msl       the species mix_speclist array
//    nsmf      the length of the species mass fraction array
//    smf       the species mass fraction array
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
//  Modifications:
//    Hank Childs, Wed Dec 19 20:54:36 PST 2001
//    Added a vector resize.
//
// ****************************************************************************

avtSpecies::avtSpecies(int nm, const int *ns,
                       int ndims, const int *dims,
                       const int *sl, int ml, const int *msl,
                       int nsmf, const float *smf)
{
    vector<int>              numspecs;
    vector<vector<string> >  specnames;
    specnames.resize(nm);
    for (int m = 0; m < nm; m++)
    {
        numspecs.push_back(ns[m]);
        for (int s = 0 ; s < ns[m] ; s++)
        {
            char name[256];
            sprintf(name, "%d", s+1);
            specnames[m].push_back(name);
        }
    }

    //
    // ndims == 1   ==>  ucd mesh, dims[0] contains nz.
    // ndims == 2   ==>  structured mesh of dim 2, dims[0]*dims[1].
    // ndims == 3   ==>  structured mesh of dim 3, dims[0]*dims[1]*dims[2].
    //
    int  nz = 1;
    for (int i = 0 ; i < ndims ; i++)
    {
        nz *= dims[i];
    }

    Initialize(numspecs, specnames, nz, sl, ml, msl, nsmf, smf);
}


// ****************************************************************************
//  Method: avtSpecies constructor
//
//  Arguments:
//    ns        the number of species for each material
//    sn        the name of each species in each material
//    nz        the number of zones in the associated mesh
//    sl        the species speclist
//    ml        the mixlen of the material object
//    msl       the species mix_speclist array
//    nsmf      the length of the species mass fraction array
//    smf       the species mass fraction array
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
// ****************************************************************************

avtSpecies::avtSpecies(const vector<int> ns,
                       const vector<vector<string> > &sn,
                       int nz,
                       const int *sl, int ml, const int *msl,
                       int nsmf, const float *smf)
{
    Initialize(ns, sn, nz, sl, ml, msl, nsmf, smf);
}


// ****************************************************************************
//  Method: avtSpecies destructor
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
// ****************************************************************************

avtSpecies::~avtSpecies()
{
    if (speclist != NULL)
    {
        delete [] speclist;
    }
    if (mix_speclist != NULL)
    {
        delete [] mix_speclist;
    }
    if (species_mf != NULL)
    {
        delete [] species_mf;
    }
}


// ****************************************************************************
//  Method: avtSpecies::Destruct
//
//  Purpose:
//      A routine that a void_ref_ptr can call to cleanly destruct species.
//
//  Programmer: Hank Childs
//  Creation:   September 25, 2002
//
// ****************************************************************************

void
avtSpecies::Destruct(void *p)
{
    avtSpecies *sp = (avtSpecies *) p;
    delete sp;
}


// ****************************************************************************
//  Method: avtSpecies::Initialize
//
//  Arguments:
//    ns        the number of species for each material
//    sn        the name of each species in each material
//    nz        the number of zones in the associated mesh
//    sl        the species speclist
//    ml        the mixlen of the material object
//    msl       the species mix_speclist array
//    nsmf      the length of the species mass fraction array
//    smf       the species mass fraction array
//  
//  Purpose:
//      Acts as the real constructor.  The true constructors take in the 
//      arguments and put it in a forspec that this routine can handle.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
//  Modifications:
//    Hank Childs, Wed Dec 19 20:54:36 PST 2001
//    Fixed a typo.
//
// ****************************************************************************

void
avtSpecies::Initialize(const vector<int> ns,
                       const vector<vector<string> > &sn,
                       int nz,
                       const int *sl, int ml, const int *msl,
                       int nsmf, const float *smf)
{
    int   i;

    nSpecies  = ns;
    species   = sn;
    nZones    = nz;
    speclist  = new int[nZones];
    for (i = 0 ; i < nZones ; i++)
    {
        speclist[i] = sl[i];
    }
    mixlen       = ml;
    mix_speclist = new int[mixlen];
    for (i = 0 ; i < mixlen ; i++)
    {
        mix_speclist[i]  = msl[i];
    }
    nspecies_mf  = nsmf;
    species_mf   = new float[nspecies_mf];
    for (i = 0 ; i < nspecies_mf ; i++)
    {
        species_mf[i]  = smf[i];
    }
}


// ****************************************************************************
//  Method: avtMultiSpecies constructor
//
//  Arguments:
//      nD      The number of domains for the species.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
// ****************************************************************************

avtMultiSpecies::avtMultiSpecies(int nD)
{
    numDomains = nD;
    species  = new avtSpecies*[numDomains];
    for (int i = 0 ; i < numDomains ; i++)
    {
        species[i] = NULL;
    }
}


// ****************************************************************************
//  Method: avtMultiSpecies destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
// ****************************************************************************

avtMultiSpecies::~avtMultiSpecies()
{
    //
    // We don't own the species, just the array that holds the pointers.
    //
    if (species != NULL)
    {
        delete [] species;
    }
}


// ****************************************************************************
//  Method: avtMultiSpecies::GetDomain
//
//  Purpose:
//      Gets the species for a domain.
//
//  Arguments:
//      dom     The domain of interest.
//
//  Returns:    The species for dom.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
// ****************************************************************************

avtSpecies *
avtMultiSpecies::GetDomain(int dom)
{
    if (dom < 0 || dom >= numDomains)
    {
        EXCEPTION2(BadDomainException, dom, numDomains);
    }

    return species[dom];
}


// ****************************************************************************
//  Method: avtMultiSpecies::SetDomain
//
//  Purpose:
//      Sets a domain of a species.
//
//  Arguments:
//      spec         The species for this domain.
//      dom         The domain number for spec.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
// ****************************************************************************

void
avtMultiSpecies::SetDomain(avtSpecies *spec, int dom)
{
    if (dom < 0 || dom >= numDomains)
    {
        EXCEPTION2(BadDomainException, dom, numDomains);
    }

    species[dom] = spec;
}


