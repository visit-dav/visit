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

#include <stdio.h>
#include <silo.h>
#include <string>
#include <vector>
#include <math.h>

//
// Template function for memory reallocation.
//

template <class T>
T *remake(T *ptr, int oldsize, int size)
{
    T *retval = new T[size];
    T *iptr = retval;
    for(int i = 0; i < oldsize; ++i)
        *iptr++ = ptr[i];
    delete [] ptr;
    return retval;
}

// ****************************************************************************
// Class: MaterialList
//
// Purpose:
//   Keeps track of mixed material information.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 21 13:53:35 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Mar 17 16:43:34 PST 2004
//   I fixed an off by one error.
//
// ****************************************************************************

class MaterialList
{
public:
    MaterialList() : matNames()
    {
        have_mixed = false;
        mix_zone = NULL;
        mix_mat = NULL;
        mix_vf = NULL;
        mix_next = NULL;
        matlist = NULL;

        // initialize private members.
        _array_size = 0;
        _array_index = 1;
        _array_growth = 1000;
    }

    ~MaterialList()
    {
       delete [] matlist;
       if (have_mixed)
       {
           delete [] mix_zone;   
           delete [] mix_mat;    
           delete [] mix_vf;      
           delete [] mix_next;    
       }
    }

    void AddMaterial(const std::string &mat)
    {
        matNames.push_back(mat);
    }

    void AddClean(int siloZone, int matNumber)
    {
        matlist[siloZone] = matNumber;
    }

    void AddMixed(int siloZone, int *matNumbers, double *matVf, int nMats)
    {
        int i;

        /* Grow the arrays if they will not fit nMats materials. */
        Resize(nMats);

        /* Record the mixed zone as a negative offset into the mix arrays. */
        matlist[siloZone] = -_array_index;

        /* Update the mix arrays. */
        for(i = 0; i < nMats; ++i)
        {
            int index = _array_index - 1;

            mix_zone[index] = siloZone;
            mix_mat[index]  = matNumbers[i];
            mix_vf[index]   = matVf[i];

            if(i < nMats - 1)
                mix_next[index] = index + 2;
            else
                mix_next[index] = 0;

            ++(_array_index);
        }

        /* indicate that we have mixed materials. */
        have_mixed = true;
    }

    void AllocClean(int nZones)
    {
        matlist = new int[nZones];
    }

    int GetMixedSize() const { return _array_index - 1; };

    void WriteMaterial(DBfile *db, const char *matvarname,
         const char *meshName, int nx, int ny, int nz) const
    {
        int i, mdims[3];
        mdims[0] = nx;
        mdims[1] = ny;
        mdims[2] = nz;

        /* Create a 1..nTotalMaterials material number array. */
        int *allmats = new int[matNames.size()];
        for(i = 0; i < matNames.size(); ++i)
            allmats[i] = i + 1;

        DBoptlist *optList = DBMakeOptlist(2);
        // Add material names.
        char **matnames = new char *[matNames.size()];
        for(i = 0; i < matNames.size(); ++i)
            matnames[i] = (char *)matNames[i].c_str();
        DBAddOption(optList, DBOPT_MATNAMES, matnames);

        if (have_mixed)
        {
            DBPutMaterial(db, (char *)matvarname, (char *)meshName,
                          matNames.size(), allmats,
                          matlist, mdims, 1, mix_next,
                          mix_mat, mix_zone,
                          (float*)mix_vf , GetMixedSize(),
                          DB_DOUBLE, optList);
        }
        else
        {         
            DBPutMaterial(db, (char *)matvarname, (char *)meshName,
                          matNames.size(), allmats,
                          matlist, mdims, 1, NULL,
                          NULL, NULL, NULL , 0,
                          DB_INT, optList);
        }

        DBFreeOptlist(optList);
        delete [] matnames;
        delete [] allmats;
    }
private:
    void Resize(int nMats)
    { 
        if(_array_index + nMats >= _array_size)
        {
            int new_size = _array_size + _array_growth;

            if(_array_size == 0)
            {
                /* Reallocate arrays in large increments. */
                mix_zone = new int[new_size];
                mix_mat  = new int[new_size];
                mix_vf   = new double[new_size];
                mix_next = new int[new_size];
            }
            else
            {
                /* Reallocate arrays in large increments. */
                mix_zone = remake(mix_zone, _array_size, new_size);
                mix_mat  = remake(mix_mat, _array_size,  new_size);
                mix_vf   = remake(mix_vf, _array_size, new_size);
                mix_next = remake(mix_next, _array_size, new_size);
            }

            _array_size = new_size;
        }
    }

    int    have_mixed;
    int    *mix_zone;
    int    *mix_mat;
    double *mix_vf;
    int    *mix_next;
    int    *matlist;
    int    _array_size;
    int    _array_index;
    int    _array_growth;
    std::vector<std::string> matNames;
};

// ****************************************************************************
// Function: GetRandomNumber
//
// Purpose: 
//   Returns a random number.
//
// Returns:    A random number.
//
// Note:       This function returns numbers from a table because it was
//             reported that drand48 was not returning the same results on
//             some platforms, which resulted in a different noise database.
//             If more than 400 random numbers are required, disable the
//             PORTABLE_RANDOM_NUMBERS macro.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 2 11:48:41 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

//#define PORTABLE_RANDOM_NUMBERS
double GetRandomNumber()
{
#ifdef PORTABLE_RANDOM_NUMBERS
    static int random_counter = 0;
    static const double portable_random_data[] = {
        0.396464773760275, 0.840485369411425, 0.353336097245244, 0.446583434796544,
        0.318692772311881, 0.886428433223031, 0.0155828494083288,
        0.584090220317272, 0.159368626531805, 0.383715874807194,
        0.691004373382196, 0.0588589135927364, 0.899854306161604,
        0.163545950630365, 0.159071502581806, 0.533064714021855,
        0.604144189711239, 0.582699021207219, 0.269971117907016,
        0.390478195463409, 0.293400570118951, 0.742377406033981,
        0.298525606318119, 0.0755380785377824, 0.404982633583334,
        0.857377942708183, 0.941968323291899, 0.662830659789996,
        0.846475779930007, 0.00275508142688352, 0.462379245025485,
        0.532596024438298, 0.78787662089292, 0.265612234971371,
        0.98275226310103, 0.30678513061418, 0.600855136489105,
        0.608715653358658, 0.212438798201187, 0.885895130587606,
        0.304657101745793, 0.15185986406857, 0.337661902873531,
        0.387476950965358, 0.643609828900129, 0.753553275640016,
        0.603616098781568, 0.53162825175081, 0.459360316334315,
        0.652488446971034, 0.32718116385065, 0.946370485960081,
        0.368039867432817, 0.943890339354468, 0.00742826171906685,
        0.516599949702389, 0.272770952753351, 0.024299155634651,
        0.591954502437812, 0.2049635097516, 0.877693349889729,
        0.0593686933802502, 0.260842551926938, 0.302829184161332,
        0.891495219672155, 0.49819805913441, 0.710025580792159,
        0.286413993907622, 0.86492357739947, 0.675540671125631,
        0.458489973232272, 0.95963556238106, 0.774675406127844,
        0.376551280801323, 0.228639116426205, 0.354533877294422,
        0.300318248151815, 0.669765831680721, 0.718966572477935,
        0.56595450872225, 0.82446531320608, 0.390611909814908,
        0.818766311218223, 0.844008460045423, 0.180467770090349,
        0.943395886088908, 0.424886765414069, 0.520665778036708,
        0.065643754874575, 0.913508169204363, 0.882584572720003,
        0.761364126692378, 0.398922546078257, 0.688256841941055,
        0.761548303519756, 0.405008799190391, 0.125251137735066,
        0.484633904711558, 0.222462553152592, 0.873121166037272,
        0.528821897552877, 0.00141396199556709, 0.860513813395496,
        0.0186973975537157, 0.814891941959363, 0.242884021468967,
        0.314571843980655, 0.965732517843072, 0.935560105184653,
        0.80943169502747, 0.492109152867098, 0.220135507787283,
        0.576353738175104, 0.289029057945669, 0.321066976456063,
        0.26132333892145, 0.173987857556103, 0.00181729960961263,
        0.0447841404416316, 0.241174550751133, 0.415451187566848,
        0.701624649114535, 0.221845691021354, 0.503910343695601,
        0.0670302113764372, 0.393063444029124, 0.47947652725605,
        0.218142210865658, 0.21951128495812, 0.91620319329374,
        0.35022175096368, 0.192693936816291, 0.21123497924744,
        0.633681628042254, 0.0535653931591149, 0.783411412752308,
        0.0306739171400388, 0.444096599615705, 0.176412888209864,
        0.932180217909242, 0.909648273496249, 0.47284483467071,
        0.871695461067244, 0.695567358482201, 0.930189621212154,
        0.455090555576618, 0.398566781653198, 0.893304001032,
        0.693546747090654, 0.838734382742629, 0.739699136234915,
        0.651269549260768, 0.678154240201113, 0.577212316619637,
        0.273118280902604, 0.935388053918395, 0.661938885152541,
        0.0474125133813033, 0.373038389580241, 0.61819477787656,
        0.148627985722786, 0.377305582204912, 0.644591400844785,
        0.0256878245887222, 0.841380513085102, 0.0770188061447783,
        0.74263391670635, 0.255986140742344, 0.901844229953472,
        0.377647585319551, 0.319529924909983, 0.211434985403777,
        0.648532499829447, 0.251315083019463, 0.228644316759183,
        0.250922180856598, 0.943221161675812, 0.136715084880649,
        0.270060767431982, 0.548707187140856, 0.324021447266222,
        0.865080020970865, 0.296703347762357, 0.680059335513555,
        0.833147253665036, 0.876307512984511, 0.649664557992974,
        0.0731154781698606, 0.898546969277891, 0.253581256254577,
        0.611318627569684, 0.841889925089895, 0.832019301181166,
        0.372587091455188, 0.757048223143908, 0.109214906307621,
        0.850811976487268, 0.55932932619065, 0.857580462076012,
        0.343096981736231, 0.6917696038328, 0.345197294645459,
        0.893584653390956, 0.958707883428566, 0.121735195187956,
        0.981544356604459, 0.0550247948269735, 0.614776279301502,
        0.038126675578102, 0.376472597943376, 0.525604855838719,
        0.281923955409873, 0.5605379614823, 0.607242965637212,
        0.816439584668299, 0.446534988318398, 0.0271116801286304,
        0.471854013937502, 0.284898305380789, 0.292723682489203,
        0.195680144142489, 0.0175794171291983, 0.829599230575294,
        0.573165442377949, 0.104780601521615, 0.732976480693964,
        0.119097519828312, 0.223888397613173, 0.947252862210423,
        0.739051619278925, 0.821356975303129, 0.8261315647214,
        0.25097997212632, 0.256495729098447, 0.338193196867149,
        0.388221672854662, 0.527318475851974, 0.266409126781255,
        0.401220531998543, 0.870894566377494, 0.0458917928501172,
        0.294573818716259, 0.394246563234287, 0.560002386477102,
        0.310821141481274, 0.822576867147674, 0.47548149041361,
        0.0911435896732868, 0.262401092809885, 0.917044094123582,
        0.978435899004108, 0.332093345723855, 0.902074034670637,
        0.240634874411811, 0.37316437186217, 0.75228603344663,
        0.457827388234367, 0.900627876163043, 0.422894512375148,
        0.565923482910826, 0.97046215686176, 0.353870875923334,
        0.431506514559569, 0.179091358788433, 0.215319112863117,
        0.33717676507672, 0.454367752847968, 0.0447257049004079,
        0.683882454393814, 0.0621491404333625, 0.551375283486319,
        0.316171236642838, 0.268217904607642, 0.500765994969726,
        0.0625369631847654, 0.964826538534915, 0.72957579086405,
        0.806315558174489, 0.151974411416315, 0.705017961093425,
        0.729957445740386, 0.842565841557473, 0.617928583041344,
        0.955447777107601, 0.479388126336563, 0.527085486138574,
        0.0293865902502226, 0.607612114663588, 0.0475693663771644,
        0.59276964764976, 0.40264692966208, 0.904394395449408,
        0.330131069414808, 0.310837454255406, 0.854944744294666,
        0.734149534459185, 0.0986265335186864, 0.340458593324797,
        0.401739493528492, 0.985167063709561, 0.629594780693527,
        0.831962390675372, 0.437119729627849, 0.791226690481601,
        0.0594618604167785, 0.1351349002463, 0.543823899352059,
        0.930451652408131, 0.884899018605683, 0.389894586859256,
        0.414557516192215, 0.856862649939352, 0.286758583530943,
        0.107756722076772, 0.576675995301894, 0.777602183838702,
        0.142250931180048, 0.808677185678292, 0.748094221754794,
        0.410024863886342, 0.475115766840876, 0.344459535440109,
        0.416798845857908, 0.0236477473992593, 0.52581100972505,
        0.71789153822759, 0.236022679375846, 0.69486507501491,
        0.779950605780559, 0.762750408386637, 0.12156271618009,
        0.370517958929419, 0.928251432954529, 0.766006337556078,
        0.289579532526428, 0.983660707931783, 0.428106915559571,
        0.037813085813724, 0.598326448384523, 0.0155956066321394,
        0.756723842570995, 0.130679000016059, 0.374567164495513,
        0.217496165745068, 0.577797001137217, 0.205663393795078,
        0.78702094261676, 0.748369188658025, 0.455336403822049,
        0.285268327435531, 0.850220081227288, 0.450007519290402,
        0.835011918615127, 0.430246027271863, 0.330995768313198,
        0.950872109930351, 0.248872817045555, 0.356794045816386,
        0.417870679229249, 0.497053137824388, 0.885292950313929,
        0.56316534805876, 0.685500243541053, 0.967759368288945,
        0.182368758719225, 0.567805378761605, 0.0296646426917242,
        0.203961872513439, 0.157764928299631, 0.547594382618087,
        0.203990632807585, 0.211286622531027, 0.0672940430302056,
        0.193097376334592, 0.901521526459444, 0.782049466546162,
        0.302589853625339, 0.422007771555116, 0.989525426229175,
        0.79712059742716, 0.387471607480325, 0.181940282553754,
        0.224731393995793, 0.737382928197398, 0.533507071053922,
        0.664735985287706, 0.2018309754509, 0.468823843290416,
        0.566497238542599, 0.0974637872254647, 0.277023691980357,
        0.0173116656720467, 0.563884071527273, 0.886939682843376,
        0.0701900742072148, 0.0620209301768924, 0.253306765884634,
        0.307064570318531, 0.896639855680402, 0.134073245056804
    };
    int index = random_counter % 400;
    random_counter++;
    return portable_random_data[index];
#else
    return drand48();
#endif
}

#define NPOINTS 2500
#define MAXMATS 6
#define NDOMAINS 8

#define BLACK_HOLE      MAXMATS
#define BLACK_HOLE_MASS 1000000.
#define BH_CENTER_X     0.5
#define BH_CENTER_Y     0.5
#define BH_CENTER_Z     0.5

class StarData
{
public:
    StarData() : mats()
    {
        nstars = 0;

        static const char *matnames[] = {"Brown Dwarf", "White Dwarf",
           "Sun type star", "Red Giant", "Neutron star", "Black hole"
        };
        mats.AllocClean(NPOINTS);
        for(int i = 0; i < MAXMATS; ++i)
            mats.AddMaterial(std::string(matnames[i]));
    }

    virtual ~StarData()
    {
    }

    void SetupInitialConditions()
    {
        //
        // Set up the initial star configuration.
        //
        for(int i = 0; i < NPOINTS-1; ++i)
        {
            x[i] = GetRandomNumber();
            y[i] = GetRandomNumber();
            z[i] = GetRandomNumber();

            int matno = GetMaterialFromNumber(GetRandomNumber()) + 1;
            mats.AddClean(i, matno);
            matnos[i] = float(matno);
            mass[i] = GetMass(matno);
            float v[3];
            GetInitialV(i, v);
            vx[i] = v[0]; 
            vy[i] = v[1];
            vz[i] = v[2];
        }

        // Add a black hole
        x[NPOINTS-1] = BH_CENTER_X;
        y[NPOINTS-1] = BH_CENTER_Y;
        z[NPOINTS-1] = BH_CENTER_Z;
        mats.AddClean(NPOINTS-1, BLACK_HOLE);
        matnos[NPOINTS-1] = float(BLACK_HOLE);
        mass[NPOINTS-1] = BLACK_HOLE_MASS;
        vx[NPOINTS-1] = 0.;
        vy[NPOINTS-1] = 0.;
        vz[NPOINTS-1] = 0.;

        nstars = NPOINTS;
    }

    void AdvanceTime();
    void Write(int cycle, int driver) const;
private:
    void GetInitialV(int i, float *v)
    {
        float p0[3] = {BH_CENTER_X, BH_CENTER_Y, BH_CENTER_Z};
        float p1[3] = {x[i], y[i], z[i]};
        float M = mass[i];

        float v0[3] = {p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]};
        float v0m = sqrt(v0[0]*v0[0] + v0[1]*v0[1] + v0[2]*v0[2]);
        if(v0m > 0.)
        {
            v0[0] /= v0m;
            v0[1] /= v0m;
            v0[2] /= v0m;
        }

        // up vector
        float v1[3] = {0., 1., 0.};

        // general motion vector. perpendicular to v0.
        float gmv[3];
        gmv[0] = v1[1] * v0[2] - v1[2] * v0[1];
        gmv[1] = v1[2] * v0[0] - v1[0] * v0[2];
        gmv[2] = v1[0] * v0[1] - v1[1] * v0[0];

        // perturb gmv a little, depending on mass
        float maxperturb = 0.0001 * (log10(M) / log10(BLACK_HOLE_MASS));
        float pgmv[3];
        float S = 0.001;
        pgmv[0] = gmv[0] * S + maxperturb * (GetRandomNumber()-0.5);
        pgmv[1] = gmv[1] * S + maxperturb * (GetRandomNumber()-0.5);
        pgmv[2] = gmv[2] * S + maxperturb * (GetRandomNumber()-0.5);

        v[0] = pgmv[0];
        v[1] = pgmv[1];
        v[2] = pgmv[2];
    }

    float GetMass(int matno)
    {
        if(matno == 1) // brown dwarf
        {
            float minR = 0.08;
            float maxR = 0.3;
            float range = maxR - minR;
            return minR + GetRandomNumber() * range;
        }
        else if(matno == 2) // white dwarf
        {
            float minR = 0.8;
            float maxR = 2.;
            float range = maxR - minR;
            return minR + GetRandomNumber() * range;
        }
        else if(matno == 3) // Sun type star
        {
            float minR = 0.7;
            float maxR = 5.;
            float range = maxR - minR;
            return minR + GetRandomNumber() * range;
        }
        else if(matno == 4) // Red Giant
        {
            float minR = 10.;
            float maxR = 500.;
            float range = maxR - minR;
            return minR + GetRandomNumber() * range;
        }
        else if(matno == 5) // Neutron star
        {
            float minR = 10000.;
            float maxR = 100000.;
            float range = maxR - minR;
            return minR + GetRandomNumber() * range;
        }
        else  // Black hole
        {
            return BLACK_HOLE_MASS;
        }
    }

    void AddStar(const StarData &sd, int index)
    {
        x[nstars] = sd.x[index];
        y[nstars] = sd.y[index];
        z[nstars] = sd.z[index];
        matnos[nstars] = sd.matnos[index];
        mass[nstars] = sd.mass[index];
        vx[nstars] = sd.vx[index];
        vy[nstars] = sd.vy[index];
        vz[nstars] = sd.vz[index];
        mats.AddClean(nstars, int(sd.matnos[index]));
        ++nstars;
    }

    bool StarInDomain(int index, int dom) const
    {
        bool val = false;

        switch(dom)
        {
        case 0:
            val = x[index] <= BH_CENTER_X &&
                  y[index] <= BH_CENTER_Y &&
                  z[index] <= BH_CENTER_Z;
            break;
        case 1:
            val = x[index] >= BH_CENTER_X &&
                  y[index] <= BH_CENTER_Y &&
                  z[index] <= BH_CENTER_Z;
            break;
        case 2:
            val = x[index] <= BH_CENTER_X &&
                  y[index] >= BH_CENTER_Y &&
                  z[index] <= BH_CENTER_Z;
            break;
        case 3:
            val = x[index] >= BH_CENTER_X &&
                  y[index] >= BH_CENTER_Y &&
                  z[index] <= BH_CENTER_Z;
            break;
        case 4:
            val = x[index] <= BH_CENTER_X &&
                  y[index] <= BH_CENTER_Y &&
                  z[index] >= BH_CENTER_Z;
            break;
        case 5:
            val = x[index] >= BH_CENTER_X &&
                  y[index] <= BH_CENTER_Y &&
                  z[index] >= BH_CENTER_Z;
            break;
        case 6:
            val = x[index] <= BH_CENTER_X &&
                  y[index] >= BH_CENTER_Y &&
                  z[index] >= BH_CENTER_Z;
            break;
        case 7:
            val = x[index] >= BH_CENTER_X &&
                  y[index] >= BH_CENTER_Y &&
                  z[index] >= BH_CENTER_Z;
            break;
        }

        return val;
    }

    int GetMaterialFromNumber(double val) const
    {
        if(val < 0.1)
            return 0;
        else if(val < 0.2)
            return 1;
        else if(val < 0.8)
            return 2;
        else if(val < 0.95)
            return 3;
        else
            return 4;
    }

    void WriteDomain(DBfile *) const;

    int          nstars;
    float        x[NPOINTS];
    float        y[NPOINTS];
    float        z[NPOINTS];
    float        matnos[NPOINTS];
    float        mass[NPOINTS];
    float        vx[NPOINTS];
    float        vy[NPOINTS];
    float        vz[NPOINTS];
    MaterialList mats;
};

void StarData::AdvanceTime()
{
#ifdef MAKE_THIS_REAL_SOMEDAY
    // Figure out the distances^2 between all points
    const float G = 0.00000001;
    float *F = new float[NPOINTS * NPOINTS];

    for(int j = 0; j < NPOINTS; ++j)
    {
        for(int i = 0; i < j; ++i)
        {
            if(i == j)
                F[i * NPOINTS + j] = 0.;
            else
            {
                float dx = x[i] - x[j];
                float dy = y[i] - y[j];
                float dz = z[i] - z[j];
                float r2 = dx*dx + dy*dy + dz*dz;
                float f = (G * mass[i] * mass[j]) / r2;

                F[i * NPOINTS + j] = f;
                F[j * NPOINTS + i] = f;
            }
        }
    }

    for(int i = 0; i < NPOINTS; ++i)
    {
        // Take the unit vector of (x,y,z) to each other (x,y,z)
        // and multiply that unit vector by F[i][j] and add up
        // all of the vectors before finally dividing by NPOINTS.
        // Then add the components to vx,vy,vz.
        float Fx = 0.;
        float Fy = 0.;
        float Fz = 0.;
        for(int j = 0; j < NPOINTS; ++j)
        {
            if(i == j)
                continue;
 
            float dx = x[j] - x[i];
            float dy = y[j] - y[i];
            float dz = z[j] - z[i];
            float r = sqrt(dx*dx + dy*dy + dz*dz);
            
            float v[3];
            float weight = F[i * NPOINTS + j] / r / mass[j];
            Fx += (dx * weight);
            Fy += (dy * weight);
            Fz += (dz * weight);
        }
        vx[i] += (Fx);
        vy[i] += (Fy);
        vz[i] += (Fz);

        x[i] += vx[i];
        y[i] += vy[i];
        z[i] += vz[i];        
    }

    delete [] F;
#endif
}

void
StarData::Write(int cycle, int driver) const
{
    int i;
    int pointsInDom[] = {NPOINTS-1, NPOINTS, NPOINTS, NPOINTS,
                         NPOINTS, NPOINTS, NPOINTS, NPOINTS};
    float domXOffset[] = {0., 0.6, 0., 0.6, 0., 0.6, 0., 0.6};
    float domYOffset[] = {0., 0., 0.6, 0.6, 0., 0., 0.6, 0.6};
    float domZOffset[] = {0., 0., 0., 0., 0.6, 0.6, 0.6, 0.6};

    char *multiMesh[NDOMAINS];
    int  multiMeshTypes[NDOMAINS];
    char *multiMat[NDOMAINS];
    char *multiMatnos[NDOMAINS];
    char *multiMass[NDOMAINS];
    char *multivx[NDOMAINS];
    char *multivy[NDOMAINS];
    char *multivz[NDOMAINS];
    int  multiVarTypes[NDOMAINS];

    char name[100];
    sprintf(name, "galaxy%04d.silo", cycle);
    DBfile *db = DBCreate(name, DB_CLOBBER, DB_LOCAL,
        "VisIt galaxy dataset", driver);

    int dom;
    for(dom = 0; dom < NDOMAINS; ++dom)
    {
        char domainName[100];
        sprintf(domainName, "domain%d", dom);
        multiMesh[dom] = new char[100];
        sprintf(multiMesh[dom], "/%s/StarMesh", domainName);
        multiMat[dom] = new char[100];
        sprintf(multiMat[dom], "/%s/Stars", domainName);
        multiMatnos[dom] = new char[100];
        sprintf(multiMatnos[dom], "/%s/Matnos", domainName);
        multiMass[dom] = new char[100];
        sprintf(multiMass[dom], "/%s/Mass", domainName);
        multivx[dom] = new char[100];
        sprintf(multivx[dom], "/%s/vx", domainName);
        multivy[dom] = new char[100];
        sprintf(multivy[dom], "/%s/vy", domainName);
        multivz[dom] = new char[100];
        sprintf(multivz[dom], "/%s/vz", domainName);


        multiMeshTypes[dom] = DB_POINTMESH;
        multiVarTypes[dom] = DB_POINTVAR;

        DBMkDir(db, domainName);
        DBSetDir(db, domainName);

        StarData domain;
        for(int i = 0; i < nstars; ++i)
        {
            if(StarInDomain(i, dom))
                domain.AddStar(*this, i);
        }
        domain.WriteDomain(db);

        DBSetDir(db, "..");
    }

    // Now that individual meshes, etc have been created, 
    DBoptlist *optlist = DBMakeOptlist(6);
    DBAddOption(optlist, DBOPT_XLABEL, (void*)"Width");
    DBAddOption(optlist, DBOPT_YLABEL, (void*)"Height");
    DBAddOption(optlist, DBOPT_ZLABEL, (void*)"Depth");
    DBAddOption(optlist, DBOPT_XUNITS, (void *)"parsec");
    DBAddOption(optlist, DBOPT_YUNITS, (void *)"parsec");
    DBAddOption(optlist, DBOPT_ZUNITS, (void *)"parsec");
    DBPutMultimesh(db, "StarMesh", NDOMAINS, (char **)multiMesh, multiMeshTypes, optlist);
    DBFreeOptlist(optlist);

    optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_UNITS, (void *)"Material number");
    DBPutMultivar(db, "Matnos", NDOMAINS, (char **)multiMatnos, multiVarTypes, optlist);
    DBFreeOptlist(optlist);

    optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_UNITS, (void *)"Solar masses");
    DBPutMultivar(db, "Mass", NDOMAINS, (char **)multiMass, multiVarTypes, optlist);
    DBFreeOptlist(optlist);

    optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_UNITS, (void *)"Km/s");
    DBPutMultivar(db, "vx", NDOMAINS, (char **)multivx, multiVarTypes, optlist);
    DBFreeOptlist(optlist);

    optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_UNITS, (void *)"Km/s");
    DBPutMultivar(db, "vy", NDOMAINS, (char **)multivy, multiVarTypes, optlist);
    DBFreeOptlist(optlist);

    optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_UNITS, (void *)"Km/s");
    DBPutMultivar(db, "vz", NDOMAINS, (char **)multivz, multiVarTypes, optlist);
    DBFreeOptlist(optlist);

    DBPutMultimat(db, "stars", NDOMAINS, (char **)multiMat, NULL);

    for(dom = 0; dom < NDOMAINS; ++dom)
    {
        delete [] multiMesh[dom];
        delete [] multiMat[dom];
        delete [] multiMatnos[dom];
        delete [] multiMass[dom];
        delete [] multivx[dom];
        delete [] multivy[dom];
        delete [] multivz[dom];
    }

    DBClose(db);
}

void StarData::WriteDomain(DBfile *db) const
{
    // Write the point mesh
    DBoptlist *optlist = DBMakeOptlist(6);
    DBAddOption(optlist, DBOPT_XLABEL, (void*)"Width");
    DBAddOption(optlist, DBOPT_YLABEL, (void*)"Height");
    DBAddOption(optlist, DBOPT_ZLABEL, (void*)"Depth");
    DBAddOption(optlist, DBOPT_XUNITS, (void *)"parsec");
    DBAddOption(optlist, DBOPT_YUNITS, (void *)"parsec");
    DBAddOption(optlist, DBOPT_ZUNITS, (void *)"parsec");
    float *pcoords[3] = {(float *)x, (float *)y, (float *)z};
    DBPutPointmesh(db, "StarMesh", 3, pcoords, nstars, DB_FLOAT, optlist);
    DBFreeOptlist(optlist);

    // Write the point var
    optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_UNITS, (void *)"Material number");
    float *vals[1] = {(float *)matnos};
    DBPutPointvar(db, "Matnos", "StarMesh", 1, vals, nstars, DB_FLOAT, optlist);
    DBFreeOptlist(optlist);

    optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_UNITS, (void *)"Solar masses");
    vals[0] = (float *)mass;
    DBPutPointvar(db, "Mass", "StarMesh", 1, vals, nstars, DB_FLOAT, optlist);
    DBFreeOptlist(optlist);

    optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_UNITS, (void *)"Km/s");
    vals[0] = (float *)vx;
    DBPutPointvar(db, "vx", "StarMesh", 1, vals, nstars, DB_FLOAT, optlist);
    DBFreeOptlist(optlist);

    optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_UNITS, (void *)"Km/s");
    vals[0] = (float *)vy;
    DBPutPointvar(db, "vy", "StarMesh", 1, vals, nstars, DB_FLOAT, optlist);
    DBFreeOptlist(optlist);

    optlist = DBMakeOptlist(1);
    DBAddOption(optlist, DBOPT_UNITS, (void *)"Km/s");
    vals[0] = (float *)vz;
    DBPutPointvar(db, "vz", "StarMesh", 1, vals, nstars, DB_FLOAT, optlist);
    DBFreeOptlist(optlist);

    mats.WriteMaterial(db, "Stars", "StarMesh", nstars, 1, 1);
}

int
main(int argc, char *argv[])
{
    int driver = DB_PDB;

    // Look through command line args.
    for(int j = 1; j < argc; ++j)
    {
        if (strcmp(argv[j], "-driver") == 0)
        {
            j++;

            if (strcmp(argv[j], "DB_HDF5") == 0)
            {
                driver = DB_HDF5;
            }
            else if (strcmp(argv[j], "DB_PDB") == 0)
            {
                driver = DB_PDB;
            }
            else
            {
                fprintf(stderr,"Unrecognized driver name \"%s\"\n",
                    argv[j]);
            }
        }
    }

    StarData stars;
    stars.SetupInitialConditions();

#define NTIMES 1
    for(int c = 0; c < NTIMES; ++c)
    {
        printf("Writing time state: %d\n", c);
        stars.Write(c, driver);
        if(c < NTIMES-1)
            stars.AdvanceTime();
    }

    return 0;
}
