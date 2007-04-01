// ************************************************************************* //
//                                 avtMaterial.C                             //
// ************************************************************************* //

#include <avtMaterial.h>

#include <stdio.h>         // for sprintf

#include <avtCallback.h>

#include <BadDomainException.h>
#include <DebugStream.h>
#include <TimingsManager.h>


using std::string;
using std::vector;


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
// ****************************************************************************

avtMaterial::avtMaterial(int nMats, const int *mats, char **names,
                         int ndims, const int *dims, int major_order,
                         const int *ml, int mixl, const int *mixm,
                         const int *mixn, const int *mixz, const float *mixv,
                         const char *domain)
{
    int timerHandle = visitTimer->StartTimer();
    int  i;

    const char *dom = (domain != NULL ? domain : "<not available>");
    bool haveIssuedWarning = false;
    vector<bool> matUsed(nMats+1, false);

    vector<string>  matnames;
    for (i = 0 ; i < nMats ; i++)
    {
        char name[256];
        if (names == NULL)
            sprintf(name, "%d", mats[i]);
        else
            sprintf(name, "%s", names[i]);
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

    int maxMat = 0;
    for (i = 0 ; i < nMats ; i++)
    {
        if (mats[i] > maxMat)
        {
            maxMat = mats[i];
        }
    }
    maxMat += 1;

    //
    // Translate the arbitrarily numbered material lists to 0 -> n-1
    //
    int *newml   = new int[nzon];
    int *newmixm = new int[mixl];
    if (maxMat < 100000)
    {
        //
        // It is faster to look up the material by lookup table.
        //
        int *lut = new int[maxMat];
        for (i = 0 ; i < maxMat ; i++)
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

            if ((ml[i] <= -mixlen) && (ml[i] < 0))
            {
                newml[i] = nMats;
                if (!haveIssuedWarning)
                {
                    char msg[1024];
                    sprintf(msg, "Zone %d of %s has a bad entry in its "
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
                    sprintf(msg, "Zone %d of %s has an invalid material"
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
                    sprintf(msg, "Mixed mat entry %d of %s has an "
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
                    sprintf(msg, "Zone %d of %s has a bad entry in its "
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
                    sprintf(msg, "Mixed mat entry %d of %s has an "
                                 "invalid material number -- %d", i, 
                                 dom, mixm[i]);
                    avtCallback::IssueWarning(msg);
                    haveIssuedWarning = true;
                }
                matUsed[nMats] = true;
            }
        }
    }

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


