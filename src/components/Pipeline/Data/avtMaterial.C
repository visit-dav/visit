// ************************************************************************* //
//                                 avtMaterial.C                             //
// ************************************************************************* //

#include <avtMaterial.h>
#include <map>

#include <snprintf.h>

#include <avtCallback.h>

#include <BadDomainException.h>
#include <UnexpectedValueException.h>
#include <DebugStream.h>
#include <TimingsManager.h>


using std::string;
using std::vector;


// ****************************************************************************
//  Function: Renumber material numbers 0 --> N-1
//
//  Arguments:
//      nMats        The number of materials in mats.
//      mats         A list of material numbers.
//      ml           The material list.
//      mixl         The mix_len.
//      mixm         The mix_mat.
//      newml        Returned refrenced new material list filled in
//      newmixm      Returned refrenced new mix_mat list filled in
//      matUsed      Returned referenced vector of bools indicating which
//                   materials in mats are actually used
//      domain       The domain string (for printing error messages only).
//
//  Programmer: Hank Childs (re-factored by Mark Miller) 
//  Creation:   April 27, 2004 
//
//  Modifications:
//    Jeremy Meredith, Thu Nov  4 13:36:07 PST 2004
//    'maxMat' had a +1 from the actual highest used material number, but
//    checks for invalid numbers were using '> maxMat' instead of '>= maxMat'.
//    I change maxMat to be the actual highest used material number and
//    allocated an array of size maxMat+1 for the lut, because having maxMat
//    mean "highest used material number" seems clearer than changing the
//    invalid checks to be ">=".
//
//    Hank Childs, Sun Feb 13 13:28:52 PST 2005
//    Fix a bad test that assumes mixlen is > 0.
//
// ****************************************************************************
static void
RenumberMaterialsZeroToNminusOne(int nMats, const int *const mats,
                                 int nzon, const int *const ml,
                                 int mixl, const int *const mixm,
                                 int* &newml, int* &newmixm,
                                 vector<bool> &matUsed,
                                 const char *domain)
{
    int i;

    const char *dom = (domain != NULL ? domain : "<not available>");
    bool haveIssuedWarning = false;

    int maxMat = 0;
    for (i = 0 ; i < nMats ; i++)
    {
        if (mats[i] > maxMat)
        {
            maxMat = mats[i];
        }
    }

    //
    // Translate the arbitrarily numbered material lists to 0 -> n-1
    //
    if (maxMat < 100000)
    {
        //
        // It is faster to look up the material by lookup table.
        //
        int *lut = new int[maxMat+1];
        for (i = 0 ; i <= maxMat ; i++)
        {
            lut[i] = -1;
        }
        for (i = 0 ; i < nMats ; i++)
        {
            lut[mats[i]] = i;
        }
        for (i = 0 ; i < nzon ; i++)
        {
            //
            // If we have a valid index into the mixed portion, tag it and
            // skip along to the next zone.
            //
            if ((ml[i] < 0) && (ml[i] > -mixl))
            {
                newml[i] = ml[i];
                continue;
            }
            else if (ml[i] < 0 && ml[i] <= -mixl)
            {
                newml[i] = nMats;
                if (!haveIssuedWarning)
                {
                    char msg[1024];
                    SNPRINTF(msg, sizeof(msg),"Zone %d of %s has a bad entry in its "
                                 "mixed material structure", i, dom);
                    avtCallback::IssueWarning(msg);
                    haveIssuedWarning = true;
                }
            }
            else if (ml[i] > maxMat || lut[ml[i]] == -1)
            {
                if (!haveIssuedWarning)
                {
                    char msg[1024];
                    SNPRINTF(msg, sizeof(msg),"Zone %d of %s has an invalid material"
                                 " number -- %d", i, dom, ml[i]);
                    avtCallback::IssueWarning(msg);
                    haveIssuedWarning = true;
                }
                newml[i] = nMats;
            }
            else
            {
                newml[i] = lut[ml[i]];
            }
            matUsed[newml[i]] = true;
        }
        for (i = 0 ; i < mixl ; i++)
        {
            if (mixm[i] < 0 || mixm[i] > maxMat || lut[mixm[i]] == -1)
            {
                if (!haveIssuedWarning)
                {
                    char msg[1024];
                    SNPRINTF(msg, sizeof(msg),"Mixed mat entry %d of %s has an "
                                 "invalid material number -- %d", i, 
                                 dom, mixm[i]);
                    avtCallback::IssueWarning(msg);
                    haveIssuedWarning = true;
                }
                newmixm[i] = nMats;
            }
            else
            {
                newmixm[i] = lut[mixm[i]];
            }
            matUsed[newmixm[i]] = true;
        }
        delete [] lut;
    }
    else
    {
        //
        // The materials are being indexed funny -- just do it the slow way.
        //
        for (i = 0 ; i < nzon ; i++)
        {
            if (ml[i] < 0 && ml[i] > -mixl)
            {
                newml[i] = ml[i];
                continue;
            }

            newml[i] = nMats;
            int m;
            for (m = 0 ; m < nMats ; m++)
            {
                if (ml[i] == mats[m])
                {
                    newml[i] = m;
                    matUsed[m] = true;
                    break;
                }
            }
            if (m == nMats)
            {
                if (!haveIssuedWarning)
                {
                    char msg[1024];
                    SNPRINTF(msg, sizeof(msg),"Zone %d of %s has a bad entry in its "
                                 "material structure", i, dom);
                    avtCallback::IssueWarning(msg);
                    haveIssuedWarning = true;
                }
                matUsed[nMats] = true;
            }
        }
    
        for (i = 0 ; i < mixl ; i++)
        {
            newmixm[i] = nMats;
            int m;
            for (m = 0 ; m < nMats ; m++)
            {
                if (mixm[i] == mats[m])
                {
                    newmixm[i] = m;
                    matUsed[m] = true;
                    break;
                }
            }
            if (m == nMats)
            {
                if (!haveIssuedWarning)
                {
                    char msg[1024];
                    SNPRINTF(msg, sizeof(msg),"Mixed mat entry %d of %s has an "
                                 "invalid material number -- %d", i, 
                                 dom, mixm[i]);
                    avtCallback::IssueWarning(msg);
                    haveIssuedWarning = true;
                }
                matUsed[nMats] = true;
            }
        }
    }
}

// ****************************************************************************
//  Method: avtMaterial constructor
//
//  Arguments:
//      nMats        The number of materials in mats.
//      mats         A list of material numbers.
//      ndims        The number of entries in dims.
//      dims         The number of material entries in each direction.
//      major_order  The major order of the material list if multidimensional.
//      ml           The material list.
//      mixl         The mix_len.
//      mixm         The mix_mat.
//      mixz         The mix_zone.
//      mixv         The mix_vf.
//      dom          The domain string (for printing error messages only).
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Dec  5 15:43:41 PST 2000
//    Made it translate from arbitrary material numbers to a 0-origin index
//    in both the matlist (ml) and mix_mat (mixm).
//
//    Jeremy Meredith, Thu Dec 13 11:48:52 PST 2001
//    Made input arrays constants.
//
//    Sean Ahern, Fri Feb  8 10:26:05 PST 2002
//    Added support for Silo material names.
//
//    Eric Brugger, Thu May 23 14:39:07 PDT 2002
//    I added support for taking into account array ordering for material
//    lists from structured meshes.
//
//    Jeremy Meredith, Thu Aug 15 13:55:12 PDT 2002
//    Added code to figure out which materials were used and which were not.
//    This allows us to created a packed (i.e. non-sparse) representation.
//
//    Hank Childs, Tue Nov 12 11:51:54 PST 2002
//    Some codes put in material numbers that are not valid to show that they
//    are 'phoney zones'.  Account for this.
//
//    Hank Childs, Mon Nov 18 12:45:12 PST 2002
//    Fix dumb ABW.
//
//    Hank Childs, Tue Dec 10 16:08:48 PST 2002
//    Do not always include the 'phoney material', because it screws up
//    species.
//
//    Hank Childs, Thu Feb 12 13:57:28 PST 2004
//    Do a better job of checking for bad materials and also issue a warning
//    when they are encountered.
//
//    Hank Childs, Wed Feb 18 09:36:38 PST 2004
//    Added the "all materials" list to the Initialize call.
//
//    Hank Childs, Wed Apr 14 07:50:31 PDT 2004
//    Do not force names to have material numbers encoded in them.
//
//    Mark C. Miller, Thu Apr 29 12:14:37 PDT 2004
//    Re-factored section of code having to do with re-numbering materials to
//    0 to N-1 to a new function, RenumberMaterialsZeroToNminusOne
//
// ****************************************************************************

avtMaterial::avtMaterial(int nMats, const int *mats, char **names,
                         int ndims, const int *dims, int major_order,
                         const int *ml, int mixl, const int *mixm,
                         const int *mixn, const int *mixz, const float *mixv,
                         const char *domain)
{
    int timerHandle = visitTimer->StartTimer();
    int  i;

    vector<bool> matUsed(nMats+1, false);

    vector<string>  matnames;
    for (i = 0 ; i < nMats ; i++)
    {
        char name[256];
        if (names == NULL)
            SNPRINTF(name, sizeof(name), "%d", mats[i]);
        else
            SNPRINTF(name, sizeof(name), "%s", names[i]);
        matnames.push_back(name);
    }
    matnames.push_back("bad material");

    //
    // Use nzon to avoid name conflict.
    // ndims == 1   ==>  ucd mesh, dims[0] contains nzon.
    // ndims == 2   ==>  structured mesh of dim 2, dims[0]*dims[1].
    // ndims == 3   ==>  structured mesh of dim 3, dims[0]*dims[1]*dims[2].
    //
    int  nzon = 1;
    for (i = 0 ; i < ndims ; i++)
    {
        nzon *= dims[i];
    }

    //
    // Translate the arbitrarily numbered material lists to 0 -> n-1
    //
    int *newml   = new int[nzon];
    int *newmixm = new int[mixl];
    RenumberMaterialsZeroToNminusOne(nMats, mats,
                                     nzon, ml,
                                     mixl, mixm,
                                     newml, newmixm,
                                     matUsed,
                                     domain);

    int nMatsUsed = 0;
    for (i = 0 ; i < nMats ; i++)
    {
        if (matUsed[i])
        {
            nMatsUsed++;
        }
    }
    debug5 << "The number of materials actually used for this domain is "
           << nMatsUsed << " (out of " << nMats << ")." << endl;
    int nActualMats = nMats;
    if (matUsed[nMats] == true)
    {
        nActualMats = nMats+1;
    }
    Initialize(nActualMats, matnames, matnames, matUsed, nzon, ndims, dims, 
               major_order, newml, mixl, newmixm, mixn, mixz, mixv);
    delete[] newml;
    delete[] newmixm;

    visitTimer->StopTimer(timerHandle, "Constructing avtMaterial object");
}


// ****************************************************************************
//  Method: avtMaterial constructor
//
//  Arguments:
//      nMats      The number of materials in mats.
//      mats       A list of material names.
//      ndims      The number of entries in dims.
//      dims       The number of material entries in each direction.
//      ml         The material list.
//      mixl       The mix_len.
//      mixm       The mix_mat.
//      mixz       The mix_zone.
//      mixv       The mix_vf.
//
//  Programmer: Jeremy Meredith
//  Creation:   November 26, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Dec 13 11:48:52 PST 2001
//    Made input arrays constants.
//
//    Eric Brugger, Thu May 23 14:39:07 PDT 2002
//    I added support for taking into account array ordering for material
//    lists from structured meshes.
//
//    Jeremy Meredith, Thu Aug 15 13:53:15 PDT 2002
//    Added code to figure out which materials were used and which were not.
//    This allows us to created a packed (i.e. non-sparse) representation.
//
//    Hank Childs, Fri Dec 20 12:33:45 PST 2002
//    Account for phoney materials.
//
//    Hank Childs, Wed Feb 18 09:36:38 PST 2004
//    Added the "all materials" list to the Initialize call.
//
// ****************************************************************************

avtMaterial::avtMaterial(int nMats, const vector<string> &mats, int nzon,
                         const int *ml, int mixl, const int *mixm,
                         const int *mixn, const int *mixz,
                         const float *mixv)
{
    int i;
    vector<bool> matUsed(nMats+1, false);
    for (i = 0 ; i < nzon ; i++)
    {
        if (ml[i] >= 0)
            matUsed[ml[i]] = true;
    }

    for (i = 0 ; i < mixl ; i++)
    {
        matUsed[mixm[i]] = true;
    }

    int nActualMats = (matUsed[nMats] ? nMats+1 : nMats);

    Initialize(nActualMats, mats, mats, matUsed, nzon, 1, &nzon, 0, ml, mixl,
               mixm, mixn, mixz, mixv);
}


// ****************************************************************************
//  Method: avtMaterial constructor for constructing materials from lists of
//          zones containing them cleanly and/or partially
//
//  Arguments:
//      nTotMats     The total number of materials over all domains
//      mats         The list of material numbers over all domains
//      names        The list of material names over all domains
//      matMap:      a list of MatZoneMap structs for each material indicating
//                   which zones contain the material cleanly, which zones
//                   contain it mixing and the associated volume fractions
//      ndims:       The number of entries in dims. 
//      dims:        The size of the list of zones in each logical dimension. 
//      major_order: 0==>row-major, 1==>col-major ordering for ndims>1 
//      domain:      for use in printing error messages, may be 0
//
//    
//  Programmer: Mark C. Miller 
//  Creation:   April 28, 2004 
//
//  Modifications:
//    Mark C. Miller, Wed May 19 21:31:28 PDT 2004
//    corrected off by one error in mixed traversals
//
// ****************************************************************************

avtMaterial::avtMaterial(int nTotMats, const int *mats, const char **names,
                         const vector<MatZoneMap> &matMap,
                         int ndims, const int *dims,
                         int major_order, const char *domain)
{
    int timerHandle = visitTimer->StartTimer();
    int i,j;
    const int notSet = INT_MAX;
    bool haveIssuedWarning = false;

    //
    // build material names vector
    //
    vector<string>  matnames;
    for (i = 0 ; i < nTotMats ; i++)
    {
        char name[256];
        if (names == NULL)
            SNPRINTF(name, sizeof(name), "%d", mats[i]);
        else
            SNPRINTF(name, sizeof(name), "%s", names[i]);
        matnames.push_back(name);
    }
    matnames.push_back("bad material");

    //
    // compute total number of zones and allocate mat list
    //
    int numZones = 1;
    for (i = 0; i < ndims; i++)
        numZones *= dims[i];
    int *ml = new int[numZones];

    //
    // determine one larger than max material number
    //
    int matnoMax = mats[0];
    for (i = 1; i < nTotMats; i++)
    {
        if (mats[i] > matnoMax)
            matnoMax = mats[i]; 
    }
    matnoMax++;

    //
    // determine how big the mix arrays will be
    //
    int mixl = 0;
    for (i = 0; i < matMap.size(); i++)
        mixl += matMap[i].numMixed;

    // set up the mix arrays 
    float *mixv = 0;
    int *mixm = 0;
    int *mixn = 0;
    int *mixz = 0;
    if (mixl)
    {
        mixv = new float[mixl];
        mixm = new int[mixl];
        mixn = new int[mixl];
        mixz = new int[mixl];
    }

    //
    // fill ml with a value we can recognize as having not been visited
    //
    for (i = 0; i < numZones; i++)
        ml[i] = notSet;

    //
    // Process each material in the map, clean part first, then mixed
    //
    int mlindex = 0;
    bool usedBadMaterial = false;
    vector<bool> matUsed(nTotMats+1, false);
    for (i = 0; i < matMap.size(); i++)
    {
        int matno = matMap[i].matno;

        // set associated matUsed flag 
        for (j = 0; j < nTotMats; j++)
        {
            if (matno == mats[j])
            {
                matUsed[j] = true;
                break;
            }
        }

        //
        // fill the clean zones with matno
        //
        int  nc = matMap[i].numClean;
        int *cz = matMap[i].cleanZones;
        for (j = 0; j < nc; j++)
        {
            int zoneNum = cz[j];

            if (!haveIssuedWarning && ((zoneNum < 0) || (zoneNum >= numZones)))
            {
                char msg[1024];
                SNPRINTF(msg, sizeof(msg),
                    "Clean zone index %d of %s is out of range [0,%d] "
                    "in material %s", zoneNum, domain, numZones,
                    matnames[matno].c_str());
                avtCallback::IssueWarning(msg);
                haveIssuedWarning = true;
            }
            else if (!haveIssuedWarning && (ml[zoneNum] != notSet))
            {
                char msg[1024];
                SNPRINTF(msg, sizeof(msg),
                    "Zone %d of %s is indicated clean in multiple "
                    "material maps, %s and %s", zoneNum, domain,
                    matnames[matno].c_str(), matnames[ml[zoneNum]].c_str());
                avtCallback::IssueWarning(msg);
                haveIssuedWarning = true;
                ml[zoneNum] = matnoMax;
                usedBadMaterial = true;
            }
            else
            {
                ml[zoneNum] = matno; 
            }
        }

        //
        // fill the mixed zones with matno and vfracs 
        //
        int  nm = matMap[i].numMixed;
        int *mz = matMap[i].mixedZones;
        float *vf = matMap[i].volFracs;
        for (j = 0; j < nm; j++)
        {
            int zoneNum = mz[j];

            if (!haveIssuedWarning && ((zoneNum < 0) || (zoneNum >= numZones)))
            {
                char msg[1024];
                SNPRINTF(msg, sizeof(msg),
                    "Clean zone index %d of %s is out of range [0,%d] "
                    "in material %s", zoneNum, domain, numZones,
                    matnames[matno].c_str());
                avtCallback::IssueWarning(msg);
                haveIssuedWarning = true;
            }
            else if (!haveIssuedWarning &&
                     (ml[zoneNum] != notSet) && (ml[zoneNum] >= 0))
            {
                char msg[1024];
                SNPRINTF(msg, sizeof(msg),
                    "Zone %d of %s is indicated clean in material %s "
                    "and mixed in material %s", zoneNum, domain,
                    matnames[ml[zoneNum]].c_str(), matnames[matno].c_str());
                avtCallback::IssueWarning(msg);
                haveIssuedWarning = true;
                ml[zoneNum] = matnoMax;
                usedBadMaterial = true;
            }
            else
            {
                if (ml[zoneNum] == notSet)
                {

                    //
                    // if we haven't visited this zone yet, just set it
                    //
                    ml[zoneNum] = -(mlindex+1);

                    mixm[mlindex] = matno;
                    mixz[mlindex] = zoneNum;
                    mixv[mlindex] = vf[j]; 
                    mixn[mlindex] = 0;
                }
                else
                {
                    //
                    // Since we've already visited this zone,
                    // walk forward through the list for this zone
                    //
                    int k = -(ml[zoneNum]+1);
                    while (true)
                    {
                        if (mixn[k] == 0)
                            break;
                        k = mixn[k]-1;
                    }
 
                    // link up last entry in list with the new entry
                    mixn[k] = mlindex+1;
 
                    // put in the new entry
                    mixm[mlindex] = matno;
                    mixz[mlindex] = zoneNum;
                    mixv[mlindex] = vf[j];
                    mixn[mlindex] = 0;
                }
                mlindex++;
            }
        }
    }

    //
    // If we encountered a problem, above, we may have notSet values in
    // the material list. So, we scan for them and replace with matnoMax, the
    // bad material id.
    //
    if (haveIssuedWarning)
    {
        for (i = 0; i < numZones; i++)
        {
            if (ml[i] == notSet)
            {
                ml[i] = matnoMax;
                usedBadMaterial = true;
            }
        }
    }

    //
    // see if the materials are arbitrarily numbered
    //
    bool arbNumbering = false; 
    for (i = 0; i < nTotMats; i++)
    {
        if (mats[i] != i)
        {
            arbNumbering = true;
            break;
        }
    }

    int addOne = 0;
    if (usedBadMaterial)
        addOne = 1;

    if (!arbNumbering)
    {
        if (usedBadMaterial)
            matUsed[nTotMats] = true;

        Initialize(nTotMats+addOne, matnames, matnames, matUsed, numZones,
                   ndims, dims, major_order, ml, mixl, mixm, mixn, mixz, mixv);
    }
    else
    {

        //
        // Translate the arbitrarily numbered material lists to 0 -> n-1
        //
        int *newml   = new int[numZones];
        int *newmixm = new int[mixl];
        int *newmats = new int[nTotMats+addOne];
        newmats[nTotMats+addOne-1] = matnoMax;
        for (i = 0; i < nTotMats; i++)
            newmats[i] = mats[i];
        vector<bool> newmatUsed(nTotMats+addOne, false);
        RenumberMaterialsZeroToNminusOne(nTotMats+addOne, newmats,
                                         numZones, ml,
                                         mixl, mixm,
                                         newml, newmixm,
                                         newmatUsed,
                                         domain);

        Initialize(nTotMats+addOne, matnames, matnames, newmatUsed, numZones,
            ndims, dims, major_order, newml, mixl, newmixm, mixn, mixz, mixv);

        delete [] newml;
        delete [] newmixm;
        delete [] newmats;

    }

    visitTimer->StopTimer(timerHandle, "Constructing avtMaterial object from "
                                       "material map");

}

// ****************************************************************************
//  Method: avtMaterial constructor for constructing materials from full zonal 
//          arrays of per-material volume fractions 
//
//  Arguments:
//      nTotMats     The total number of materials over all domains
//      mats         The list of material numbers over all domains
//      names        The list of material names over all domains
//      vfracs       The per-material, per-zone volume fractions for THIS domain
//                   If vfracs[k] == 0, material k is NOT present on THIS domain
//      ndims:       The number of entries in dims. 
//      dims:        The number of zones in each of the ndims logical dimensions
//      major_order  The major order of the material list if multidimensional.
//      domain       The domain string (for printing error messages only).
//    
//  Programmer: Mark C. Miller 
//  Creation:   April 28, 2004 
//
//  Modifications:
//    Mark C. Miller, Wed May 19 21:31:28 PDT 2004
//    corrected off by one error in mixed traversals
//
// ****************************************************************************

avtMaterial::avtMaterial(int nTotMats, const int *mats, char **names,
                         int ndims, const int *dims, int major_order,
                         const float *const *vfracs, const char *domain)
{
    int timerHandle = visitTimer->StartTimer();
    const int notSet = INT_MAX;
    int i,m,z;
    bool haveIssuedWarning = false;

    //
    // build material names vector
    //
    vector<string>  matnames;
    for (i = 0 ; i < nTotMats ; i++)
    {
        char name[256];
        if (names == NULL)
            SNPRINTF(name, sizeof(name), "%d", mats[i]);
        else
            SNPRINTF(name, sizeof(name), "%s", names[i]);
        matnames.push_back(name);
    }
    matnames.push_back("bad material");

    //
    // compute total number of zones
    //
    int ncells = 1;
    for (i = 0; i < ndims; i++)
        ncells *= dims[i];

    //
    // determine one larger than max material number
    //
    int matnoMax = mats[0];
    for (i = 1; i < nTotMats; i++)
    {
        if (mats[i] > matnoMax)
            matnoMax = mats[i]; 
    }
    matnoMax++;

    // allocate and fill ml array with the 'notSet' value
    int *ml = new int[ncells];
    for (i = 0; i < ncells; i++)
        ml[i] = notSet;

    // pre-compute size of mix arrays
    int mixl = 0;
    for (m = 0; m < nTotMats; m++)
    {
        const float *frac = vfracs[m];

        // skip this material if it isn't present 
        if (frac == 0)
            continue;

        for (z = 0; z < ncells; z++)
        {
            if ((frac[z] > 0.0) &&
                (frac[z] < 1.0))
                mixl++;
        }
    }

    // allocate mix arrays
    float  *mixv = new float[mixl];
    int *mixm = new int[mixl];
    int *mixz = new int[mixl];
    int *mixn = new int[mixl];

    // loop over materials
    mixl = 0;
    bool usedBadMaterial = false;
    vector<bool> matUsed(nTotMats+1, false);
    for (m = 0; m < nTotMats; m++)
    {

        const float *frac = vfracs[m];

        // skip this material if it isn't present 
        if (frac == 0)
            continue;

        matUsed[m] = true;

        // loop over zones
        for (z = 0; z < ncells; z++)
        {
            if (frac[z] >= 1.0)
            {
                if (!haveIssuedWarning && (ml[z] != notSet))
                {
                    char msg[1024];
                    SNPRINTF(msg, sizeof(msg),
                        "Material %s has vol-frac >= 1.0 in zone %d of %s "
                        "which already contains another material",
                        matnames[m].c_str(), z, domain);
                    avtCallback::IssueWarning(msg);
                    haveIssuedWarning = true;
                    ml[z] = matnoMax;
                    usedBadMaterial = true;
                }
                else
                {
                    ml[z] = mats[m];
                }
            }
            else if ((1.0 > frac[z]) && (frac[z] > 0.0))
            {

                if (ml[z] == notSet)
                {
                    // put the first entry in the list for this zone
                    ml[z] = -(mixl+1); 
                    mixm[mixl] = mats[m]; 
                    mixv[mixl] = frac[z];
                    mixz[mixl] = z;
                    mixn[mixl] = 0;
                }
                else
                {
                    if (!haveIssuedWarning && (ml[z] >= 0))
                    {
                        char msg[1024];
                        SNPRINTF(msg, sizeof(msg),
                            "Zone %d of %s which was already determined to "
                            "be clean in material %s contains another material "
                            "%s in fractional amount %f",
                            z, domain, matnames[ml[z]].c_str(),
                            matnames[m].c_str(), frac[z]);
                        avtCallback::IssueWarning(msg);
                        haveIssuedWarning = true;
                        ml[z] = matnoMax;
                        usedBadMaterial = true;
                    }
                    else
                    {

                        // walk forward through the list for this zone
                        int j = -(ml[z]+1);
                        while (true)
                        {
                            if (mixn[j] == 0)
                                break;
                            j = mixn[j]-1;
                        }

                        // link up last entry in list with the new entry
                        mixn[j] = mixl+1;

                        // put in the new entry
                        mixm[mixl] = mats[m];
                        mixv[mixl] = frac[z];
                        mixz[mixl] = z;
                        mixn[mixl] = 0;
                    }
                }

                mixl++;
            }
            else if (frac[z] != 0.0)
            {
                char msg[1024];
                SNPRINTF(msg, sizeof(msg),
                    "Zone %d of %s contains material %s with a negative "
                    "volume fraction %f", z, domain, matnames[m].c_str(),
                    frac[z]);
                avtCallback::IssueWarning(msg);
                haveIssuedWarning = true;
                ml[z] = matnoMax;
                usedBadMaterial = true;
            }
        }
    }

    //
    // see if the materials are arbitrarily numbered
    //
    bool arbNumbering = false; 
    for (i = 0; i < nTotMats; i++)
    {
        if (mats[i] != i)
        {
            arbNumbering = true;
            break;
        }
    }

    int addOne = 0;
    if (usedBadMaterial)
        addOne = 1;

    if (!arbNumbering)
    {
        if (usedBadMaterial)
            matUsed[nTotMats] = true;

        Initialize(nTotMats+addOne, matnames, matnames, matUsed, ncells,
            ndims, dims, major_order, ml, mixl, mixm, mixn, mixz, mixv);
    }
    else
    {
        //
        // Translate the arbitrarily numbered material lists to 0 -> n-1
        //
        int *newml   = new int[ncells];
        int *newmixm = new int[mixl];
        int *newmats = new int[nTotMats+addOne];
        for (i = 0; i < nTotMats; i++)
            newmats[i] = mats[i];
        newmats[nTotMats] = matnoMax;
        vector<bool> newmatUsed(nTotMats+addOne, false);
        RenumberMaterialsZeroToNminusOne(nTotMats+addOne, newmats,
                                         ncells, ml,
                                         mixl, mixm,
                                         newml, newmixm,
                                         newmatUsed,
                                         domain);

        Initialize(nTotMats+addOne, matnames, matnames, newmatUsed, ncells,
            ndims, dims, major_order, newml, mixl, newmixm, mixn, mixz, mixv);

        delete [] newml;
        delete [] newmixm;
        delete [] newmats;
    }

    visitTimer->StopTimer(timerHandle, "Constructing avtMaterial object from "
                                       "dense vfrac arrays");

}

// ****************************************************************************
//  Method: avtMaterial constructor for packing sparse materials
//
//  Arguments:
//      mat              the material to copy and pack
//      nUsedMats        the number of materials actually used
//      mapMatToUsedMat  a map from the original material numbers to compressed
//      mapUsedMatToMat  a map from the compressed material numbers to original
//
//  Programmer: Jeremy Meredith
//  Creation:   August 15, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Feb 18 09:36:38 PST 2004
//    Added the "all materials" list to the Initialize call.
//
// ****************************************************************************

avtMaterial::avtMaterial(const avtMaterial *mat,
                         int                nUsedMats,
                         vector<int>        mapMatToUsedMat,
                         vector<int>        mapUsedMatToMat)
{
    vector<bool> matUsed(nUsedMats, true);

    if (mat->nMaterials == nUsedMats)
    {
        // no packing can occur
        Initialize(mat->nMaterials, mat->materials, mat->materials, matUsed,
                   mat->nZones, 1, &mat->nZones, 0,
                   mat->matlist, mat->mixlen, mat->mix_mat,
                   mat->mix_next, mat->mix_zone, mat->mix_vf);
        return;
    }

    int i;
    int            *matlist = new int[mat->nZones];
    int            *mix_mat = new int[mat->mixlen];
    vector<string>  materials(nUsedMats, "");

    for (i=0; i<mat->nZones; i++)
    {
        int matno = mat->matlist[i];
        if (matno >= 0)
            matlist[i] = mapMatToUsedMat[matno];
        else
            matlist[i] = matno;
    }

    for (i=0; i<mat->mixlen; i++)
    {
        mix_mat[i] = mapMatToUsedMat[mat->mix_mat[i]];
    }

    for (i=0; i<nUsedMats; i++)
        materials[i] = mat->materials[mapUsedMatToMat[i]];

    Initialize(nUsedMats, materials, mat->materials, matUsed,
               mat->nZones, 1, &mat->nZones, 0,
               matlist, mat->mixlen, mix_mat,
               mat->mix_next, mat->mix_zone, mat->mix_vf);

    delete[] matlist;
    delete[] mix_mat;
}


// ****************************************************************************
//  Method:  CreatePackedMaterial
//
//  Purpose:
//    Create a clone of this material, but with the material numbers packed.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 15, 2002
//
//  Modifications:
//    Jeremy Meredith, Thu Aug 21 15:09:17 PDT 2003
//    Added a timer.
//
// ****************************************************************************

avtMaterial *
avtMaterial::CreatePackedMaterial() const
{
    int timerHandle = visitTimer->StartTimer();
    avtMaterial *m = new avtMaterial(this, nUsedMats,
                                     mapMatToUsedMat,
                                     mapUsedMatToMat);
    visitTimer->StopTimer(timerHandle, "Packing material");
    return m;
}

// ****************************************************************************
//  Method: avtMaterial destructor
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
// ****************************************************************************

avtMaterial::~avtMaterial()
{
    if (matlist != NULL)
    {
        delete [] matlist;
    }
    if (mix_mat != NULL)
    {
        delete [] mix_mat;
    }
    if (mix_next != NULL)
    {
        delete [] mix_next;
    }
    if (mix_zone != NULL)
    {
        delete [] mix_zone;
    }
    if (mix_vf != NULL)
    {
        delete [] mix_vf;
    }
}


// ****************************************************************************
//  Method: avtMaterial::Destruct
//
//  Purpose:
//      A routine that a void_ref_ptr can call to cleanly destruct a material.
//
//  Programmer: Hank Childs
//  Creation:   September 25, 2002
//
// ****************************************************************************

void
avtMaterial::Destruct(void *p)
{
    avtMaterial *mat = (avtMaterial *) p;
    delete mat;
}


// ****************************************************************************
//  Method: avtMaterial::Initialize
//
//  Arguments:
//      nMats        The number of material in matnames.
//      matnames     The names of the materials.
//      all_matnames The names of all the materials, including those discarded
//                   because they weren't present in this domain.
//      nzon         The number of zones in ml.
//      ndims        The number of entries in dims.
//      dims         The number of material entries in each direction.
//      major_order  The major order of the material list if multidimensional.
//      ml           A list of materials for each zone.
//      mixl         The mixlen.
//      mixm         The mix_mat.
//      minn         The mix_next.
//      mixz         The mix_zone.
//      mixv         The mix_vf.
//  
//  Purpose:
//      Acts as the real constructor.  The true constructors take in the 
//      arguments and put it in a format that this routine can handle.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Dec 13 11:48:52 PST 2001
//    Made input arrays constants.
//
//    Eric Brugger, Thu May 23 14:39:07 PDT 2002
//    I added support for taking into account array ordering for material
//    lists from structured meshes.
//
//    Jeremy Meredith, Thu Aug 15 13:53:15 PDT 2002
//    Added code to figure out which materials were used and to create
//    the mapping between the sparse material numbers and the packed ones.
//
//    Hank Childs, Mon Apr  7 09:26:23 PDT 2003
//    Do not assume mixz is non-NULL.
//
//    Hank Childs, Wed Feb 18 09:36:38 PST 2004
//    Add an argument for a list of all materials.
//
// ****************************************************************************

void
avtMaterial::Initialize(int nMats, const vector<string> &matnames,
                        const vector<string> &all_matnames, 
                        const vector<bool> &matUsed, int nzon,
                        int ndims, const int *dims, int major_order,
                        const int *ml, int mixl, const int *mixm,
                        const int *mixn, const int *mixz, const float *mixv)
{
    int   i;

    nUsedMats = 0;
    mapMatToUsedMat = vector<int>(nMats, -1);
    for (i=0; i<nMats; i++)
    {
        if (matUsed[i])
        {
            mapMatToUsedMat[i] = nUsedMats;
            mapUsedMatToMat.push_back(i);
            nUsedMats++;
        }
    }

    nMaterials = nMats;
    materials  = matnames;
    all_materials = all_matnames;
    nZones     = nzon;
    matlist    = new int[nZones];
    if (ndims <= 1 || major_order == 0)
    {
        for (i = 0 ; i < nZones ; i++)
        {
            matlist[i] = ml[i];
        }
    }
    else
    {
        int       i, j, k;
        int       nx, ny, nz, nxy, nyz;

        switch (ndims)
        {
          case 2:
            nx = dims[0];
            ny = dims[1];

            for (j = 0; j < ny; j++)
            {
                for (i = 0; i < nx; i++)
                {
                    matlist[j*nx + i] = ml[j + i*ny];
                }
            }

            break;

          case 3:
            nx = dims[0];
            ny = dims[1];
            nz = dims[2];
            nxy = dims[0] * dims[1];
            nyz = dims[1] * dims[2];

            for (k = 0; k < nz; k++)
            {
                for (j = 0; j < ny; j++)
                {
                    for (i = 0; i < nx; i++)
                    {
                        matlist[k*nxy + j*nx + i] = ml[k + j*nz + i*nyz];
                    }
                }
            }

            break;
        }
    }
    mixlen     = mixl;
    mix_mat    = new int[mixlen];
    mix_next   = new int[mixlen];
    if (mixz == NULL)
        mix_zone = NULL;
    else
        mix_zone   = new int[mixlen];
    mix_vf     = new float[mixlen];
    for (i = 0 ; i < mixlen ; i++)
    {
        mix_mat[i]  = mixm[i];
        mix_next[i] = mixn[i];
        if (mixz != NULL)
            mix_zone[i] = mixz[i];
        mix_vf[i]   = mixv[i];
    }
}


// ****************************************************************************
//  Method: ExtractCellMatInfo 
//
//  Purpose:
//    Extract the clean/mixed material info for a single cell.
//
//  Arguments:
//    c          the cell
//    mix_index  the output mixed index array
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 22, 2003
//
// ****************************************************************************
void
avtMaterial::ExtractCellMatInfo(int c, int *mix_index) const
{
    for (int m=0; m<nMaterials; m++)
    {
        mix_index[m] = -1;
    }

    if (matlist[c] < 0)
    {
        int  mixix  = -matlist[c] - 1;

        while (mixix >= 0)
        {
            mix_index[mix_mat[mixix]] = mixix;

            mixix = mix_next[mixix] - 1;
        }
    }
}


// ****************************************************************************
//  Method: ExtractCellMatInfo 
//
//  Purpose:
//    Extract the clean/mixed material info for a single cell.
//
//  Arguments:
//    c          the cell
//    zone_vf    the output vf array
//    mix_index  the output mixed index array
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 24, 2002
//
// ****************************************************************************
void
avtMaterial::ExtractCellMatInfo(int c, float *zone_vf, int *mix_index) const
{
    for (int m=0; m<nMaterials; m++)
    {
        zone_vf[m]   = 0.;
        mix_index[m] = -1;
    }

    if (matlist[c] < 0)
    {
        int  mixix  = -matlist[c] - 1;

        while (mixix >= 0)
        {
            zone_vf[mix_mat[mixix]]   = mix_vf[mixix];
            mix_index[mix_mat[mixix]] = mixix;

            mixix = mix_next[mixix] - 1;
        }
    }
    else
    {
        zone_vf[matlist[c]] = 1.0;
    }
}


// ****************************************************************************
//  Method:  ExtractCellMatInfo
//
//  Purpose:
//    Extract the clean/mixed material info for a single cell.
//
//  Arguments:
//    c          the cell
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 24, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Nov 19 14:42:09 PST 2003
//    Added "material number" to CellMatInfo.  This is more for internal
//    use than for displaying; for example, if one were to try to extract
//    species information for each material in this cell.
//
// ****************************************************************************
vector<CellMatInfo>
avtMaterial::ExtractCellMatInfo(int c) const
{
    vector<CellMatInfo> info;

    if (matlist[c] < 0)
    {
        int  mixix  = -matlist[c] - 1;

        while (mixix >= 0)
        {
            info.push_back(CellMatInfo(materials[mix_mat[mixix]],
                                       mix_mat[mixix],
                                       mix_vf[mixix],
                                       mixix));

            mixix = mix_next[mixix] - 1;
        }
    }
    else
    {
        info.push_back(CellMatInfo(materials[matlist[c]], matlist[c], 1.0));
    }

    return info;
}


// ****************************************************************************
//  Method: Print material list and mix info passed in 
//
//  Programmer: Mark C. Miller 
//  Creation:   May 19, 2004 
//
// ****************************************************************************

void
avtMaterial::Print(ostream& out, int nzones , const int *matlist, int mixlen,
    const int *mix_mat, const int *mix_zone, const float *mix_vf,
    const int *mix_next)
{
    std::map<int,bool> matNumsUsed;

    int i;
    for (i = 0; i < nzones; i++)
    {
        if (matlist[i] >= 0)
        {
            out << "Zone " << i << " is clean in material " << matlist[i] << endl;
            matNumsUsed[matlist[i]] = true;
        }
        else
        {
            out << "Zone " << i << " is mixing as follows" << endl; 
            int mixidx = -(matlist[i]+1);
            float vfsum = 0.0;

            while (true)
            {
                out << mix_mat[mixidx] << " (" << (int) (100*mix_vf[mixidx]) << "%), ";
                matNumsUsed[mix_mat[mixidx]] = true;
                vfsum += mix_vf[mixidx];
                if (mix_next[mixidx] == 0)
                    break;
                mixidx = mix_next[mixidx]-1;
            }

            out << " vfrac sum = " << vfsum;

            if ((vfsum > 1 + 0.01) || (vfsum < 1 - 0.01))
                out << " *** NOT ~1.0 ***" << endl;
            else
                out << " OK" << endl;
        }
    }

    out << "In traversing the clean and mixed arrays, encountered the following "
           "material numbers" << endl;
    std::map<int,bool>::const_iterator pos;
    for (pos = matNumsUsed.begin(); pos != matNumsUsed.end(); pos++)
        out << ", " << pos->first;
    out << endl;

}

// ****************************************************************************
//  Method: avtMultiMaterial constructor
//
//  Arguments:
//      nD      The number of domains for the material.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
// ****************************************************************************

avtMultiMaterial::avtMultiMaterial(int nD)
{
    numDomains = nD;
    materials  = new avtMaterial*[numDomains];
    for (int i = 0 ; i < numDomains ; i++)
    {
         materials[i] = NULL;
    }
}


// ****************************************************************************
//  Method: avtMultiMaterial destructor
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
// ****************************************************************************

avtMultiMaterial::~avtMultiMaterial()
{
    //
    // We don't own the materials, just the array that holds the pointers.
    //
    if (materials != NULL)
    {
        delete [] materials;
    }
}


// ****************************************************************************
//  Method: avtMultiMaterial::GetDomain
//
//  Purpose:
//      Gets the material for a domain.
//
//  Arguments:
//      dom     The domain of interest.
//
//  Returns:    The material for dom.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
// ****************************************************************************

avtMaterial *
avtMultiMaterial::GetDomain(int dom)
{
    if (dom < 0 || dom >= numDomains)
    {
        EXCEPTION2(BadDomainException, dom, numDomains);
    }

    return materials[dom];
}


// ****************************************************************************
//  Method: avtMultiMaterial::SetDomain
//
//  Purpose:
//      Sets a domain of a material.
//
//  Arguments:
//      mat         The material for this domain.
//      dom         The domain number for mat.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
// ****************************************************************************

void
avtMultiMaterial::SetDomain(avtMaterial *mat, int dom)
{
    if (dom < 0 || dom >= numDomains)
    {
        EXCEPTION2(BadDomainException, dom, numDomains);
    }

    materials[dom] = mat;
}


