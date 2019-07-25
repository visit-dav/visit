// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <visit-config.h>
#include <string.h>
#include <math.h>

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
using std::cout;
using std::cerr;
using std::endl;

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define MAKE_BOV_SUBDIRECTORY
#else
#define snprintf _snprintf
#endif

#define SPHERICAL_COORDINATES

#ifdef PARALLEL
#include <mpi.h>
#endif
const char *dirName = "mandelbulb_domains";

// ****************************************************************************
// Method: write_BOV_data
//
// Purpose:
//   Write a BOV data file.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 23 02:29:18 PDT 2014
//
// Modifications:
//
// ****************************************************************************

template <class T>
void
write_BOV_data(const char *filename, const T *data, int nels)
{
    bool done = false;
    while(!done)
    {
        FILE *fp = fopen(filename, "wb");
        if(fp != NULL)
        {
            size_t nWritten = 0, nTotal = (size_t)nels;
            const T *ptr = data;
            while(nWritten < nTotal)
            {
                size_t n = fwrite((void *)ptr, sizeof(T), nTotal - nWritten, fp);
                nWritten += n;
                ptr += n;
            }
            fclose(fp);
            done = true;
        }
        else
        {
            cerr << "Could not create: " << filename << ". Try again." << endl;
        }
    }
}

// ****************************************************************************
// Class: Triplex
//
// Purpose:
//   Hypercomplex number.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 23 02:29:33 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class Triplex
{
public:
    Triplex() { x = y = z = 0.f; }
    Triplex(float v) { x = y = z = v; }
    Triplex(float a, float b, float c) { x = a; y = b; z = c; }

    float mag2() const
    {
        return x*x + y*y + z*z;
    }

    float mag() const
    {
        return sqrt(mag2());
    }

    float x,y,z;
};

inline Triplex operator + (const Triplex &obj, float val)
{
    return Triplex(obj.x+val, obj.y+val, obj.z+val);
}

inline Triplex operator + (float val, const Triplex &obj)
{
    return Triplex(obj.x+val, obj.y+val, obj.z+val);
}

inline Triplex operator + (const Triplex &lhs, const Triplex &rhs)
{
    return Triplex(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z);
}

inline Triplex operator * (const Triplex &obj, float val)
{
    return Triplex(obj.x*val, obj.y*val, obj.z*val);
}

inline Triplex operator * (float val, const Triplex &obj)
{
    return Triplex(obj.x*val, obj.y*val, obj.z*val);
}

inline Triplex operator ^ (const Triplex &obj, float n)
{
    // http://www.bugman123.com/Hypercomplex/index.html
    float r = obj.mag();
    float theta = n * atan2(obj.y,obj.x);
    float phi = (r <= 0.) ? (n * asin(1)) : (n * asin(obj.z/r));
    float rn = pow(r,n);
    float newx = rn * cos(theta) * cos(phi);
    float newy = rn * sin(theta) * cos(phi);
    float newz = rn * -sin(phi);
    return Triplex(newx, newy, newz);
}

// ****************************************************************************
// Method: mandelbulb
//
// Purpose:
//   Calculate power 8 mandelbulb.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       Order 8 Mandelbulb
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 23 02:17:20 PDT 2014
//
// Modifications:
//
// ****************************************************************************
 
inline char
mandelbulb(const Triplex &C)
{
    const char MAXIT = 20;
    Triplex Z;
    for(char zit = 0; zit < MAXIT; ++zit)
    {
        Z = (Z ^ 8.f) + C;
        if(Z.mag2() > 4.f)
            return zit+1;
    }
    return 0;
}

// ****************************************************************************
// Method: DivideDomainList
//
// Purpose:
//   Divide the domain list among processors.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 23 02:18:01 PDT 2014
//
// Modifications:
//
// ****************************************************************************

std::vector<int>
DivideDomainList(int par_rank, int par_size, const int *nDomains)
{
    int ntotal = nDomains[0]*nDomains[1]*nDomains[2];
    int n_per_rank = ntotal / par_size;
    int *rank_count = new int[par_size];
    for(int i = 0; i < par_size; ++i)
        rank_count[i] = n_per_rank;
    int n_unassigned = ntotal - par_size*n_per_rank;
    for(int i = 0; i < n_unassigned; ++i)
        ++rank_count[i];

    std::vector<int> domainList;
    int offset = 0;
    for(int i = 0; i < par_size; ++i)
    {
        if(i == par_rank)
        {
            for(int j = 0; j < rank_count[i]; ++j)
                domainList.push_back(offset + j);
            break;
        }
        offset += rank_count[i];
    }
    delete [] rank_count;

    return domainList;
}

// ****************************************************************************
// Method: GetIJK
//
// Purpose:
//   Get the IJK of the domain.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 23 02:16:55 PDT 2014
//
// Modifications:
//
// ****************************************************************************

inline void
GetIJK(int idx, int nx, int ny, int nz, int &I, int &J, int &K)
{
    I = idx % nx;
    J = (idx % (nx*ny)) / nx;
    K = idx / (nx*ny);
}

// ****************************************************************************
// Method: CalculateDomain
//
// Purpose:
//   Calculate the mandelbulb for the specified domain.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 23 02:16:33 PDT 2014
//
// Modifications:
//
// ****************************************************************************

template <typename T>
void
CalculateDomain(int domain, const int *nDomains, const int *dims,
    const float *extents, T *data)
{
    // Figure out the domain IJK.
    int I,J,K;
    GetIJK(domain, nDomains[0], nDomains[1], nDomains[2], I,J,K);

    // Figure out the domain extents.
    float domDX = (extents[1] - extents[0]) / float(nDomains[0]);
    float domDY = (extents[3] - extents[2]) / float(nDomains[1]);
    float domDZ = (extents[5] - extents[4]) / float(nDomains[2]);
    float dExtents[6];
    dExtents[0] = extents[0] + domDX * float(I);
    dExtents[1] = dExtents[0] + domDX;
    dExtents[2] = extents[2] + domDY * float(J);
    dExtents[3] = dExtents[2] + domDY;
    dExtents[4] = extents[4] + domDZ * float(K);
    dExtents[5] = dExtents[4] + domDZ;

    // Generate the data for the domain
    T *ptr = data;
    for(int kk = 0; kk < dims[2]; ++kk)
    {
        float tz_val = float(kk) / float(dims[2]-1);
        float z = (1.-tz_val)*dExtents[4] + tz_val*dExtents[5];
        for(int jj = 0; jj < dims[1]; ++jj)
        {
            float ty_val = float(jj) / float(dims[1]-1);
            float y = (1.-ty_val)*dExtents[2] + ty_val*dExtents[3];
            for(int ii = 0; ii < dims[0]; ++ii)
            {
                float tx_val = float(ii) / float(dims[0]-1);
                float x = (1.-tx_val)*dExtents[0] + tx_val*dExtents[1];
#ifdef SPHERICAL_COORDINATES
                // If we're processing domains in spherical coordinates, turn
                // the result to cartesian before calculating the mandelbulb.
                float r = x;
                float theta = z;
                float phi = y;
                Triplex cartesian;
                cartesian.x = r * cos(theta) * sin(phi);
                cartesian.y = r * sin(theta) * sin(phi);
                cartesian.z = r * cos(phi);

                *ptr++ = mandelbulb(cartesian);
#else
                *ptr++ = mandelbulb(Triplex(x,y,z));
#endif
            }
        }
    }
}

// ****************************************************************************
// Method: WriteDomainFiles
//
// Purpose:
//   Writes the domain files for this processor to BOV format.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 23 02:16:08 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
WriteDomainFiles(int par_rank, const std::string &output, const int *nDomains, 
    const int *dims, const float *extents, const std::vector<int> &domListThisProc)
{
#ifdef MAKE_BOV_SUBDIRECTORY
    int mkdir_ret = 0;
    if(par_rank == 0)
    {
        mkdir_ret = mkdir(dirName, S_IRUSR | S_IWUSR | S_IXUSR);
        if(mkdir_ret < 0)
        {
            if(errno == EEXIST)
            {
                mkdir_ret = 0;
                if(par_rank == 0)
                {
                    cerr << "WARNING: Directory " << dirName << " already exists." << endl;
                }
            }
            else
            {
                cerr << "ERROR: could not create " << dirName << " directory:"
                     << strerror(errno) << endl;
            }
        }
    }
#ifdef PARALLEL
    MPI_Bcast(&mkdir_ret, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
    if(mkdir_ret == 0)
    {   
        int chdir_ret = chdir(dirName);
        if(par_rank == 0 && chdir_ret < 0)
        {
            cerr << "ERROR: failed to cd to " << dirName << endl;
        }
    }
#endif

    char *data = new char[dims[0] * dims[1] * dims[2]];
    for(size_t idx = 0; idx < domListThisProc.size(); ++idx)
    {
        // Generate the data for the domain
        CalculateDomain(domListThisProc[idx], nDomains, dims, extents, data);

        // Write the data for the domain.
        char filename[100];
        snprintf(filename, 100, "%s%06d.values", output.c_str(), domListThisProc[idx]);
        write_BOV_data(filename, data, dims[0] * dims[1] * dims[2]);
    }
    delete [] data;

#ifdef MAKE_BOV_SUBDIRECTORY
    if (mkdir_ret == 0)
    {
        int chdir_ret = chdir("..");
        if(par_rank == 0 && chdir_ret < 0)
        {
            cerr << "ERROR: failed to cd .." << endl;
        }
    }
#endif
}

// ****************************************************************************
// Method: WriteMasterFile
//
// Purpose:
//   Writes the master BOV file.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 23 02:12:16 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
WriteMasterFile(int par_rank, const std::string &output, const int *nDomains,
    const int *dims, const float *extents, const std::vector<int> &domListThisProc)
{
    if(par_rank != 0)
        return;

    // Write the BOV header.
    FILE *fp = fopen(std::string(output + ".bov").c_str(), "wt");
    fprintf(fp, "TIME: 0\n");
#ifdef MAKE_BOV_SUBDIRECTORY
    fprintf(fp, "DATA_FILE: %s/%s%%06d.values\n", dirName, output.c_str());
#else
    fprintf(fp, "DATA_FILE: %s%%06d.values\n", output.c_str());
#endif
    fprintf(fp, "DATA_SIZE: %d %d %d\n", 
        nDomains[0] * dims[0], nDomains[1] * dims[1], nDomains[2] * dims[2]);
    fprintf(fp, "DATA_FORMAT: CHAR\n");
    fprintf(fp, "DATA_COMPONENTS: 1\n");
    fprintf(fp, "VARIABLE: mandelbulb\n");
#ifdef WORDS_BIGENDIAN
    fprintf(fp, "DATA_ENDIAN: BIG\n");
#else
    fprintf(fp, "DATA_ENDIAN: LITTLE\n");
#endif
    fprintf(fp, "CENTERING: nodal\n");
    fprintf(fp, "BRICK_ORIGIN: %g %g %g\n", extents[0], extents[2], extents[4]);
    fprintf(fp, "BRICK_SIZE: %g %g %g\n", 
            extents[1]-extents[0],
            extents[3]-extents[2],
            extents[5]-extents[4]);
    fprintf(fp, "DATA_BRICKLETS: %d %d %d\n", dims[0], dims[1], dims[2]);
    fclose(fp);
}

// ****************************************************************************
// Purpose: Print the usage information.
// 
// Programmer: Brad Whitlock
// Creation:   Thu Oct 23 01:21:19 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
PrintUsage(const char *argv0)
{
    cout << "Usage: " << argv0 << "[-output filename] ";
#ifdef SPHERICAL_COORDINATES
    cout << " [-nr number] [-nphi number] [-ntheta number]";
    cout << " [-rdoms number] [-phidoms number] [-thetadoms number]" << endl;
    cout << "==================================================================================" << endl;
    cout << "-nr number        Sets the number of nodes (per domain) along the radius dimension." << endl;
    cout << "-nphi number      Sets the number of nodes (per domain) along the phi dimension." << endl;
    cout << "-ntheta number    Sets the number of nodes (per domain) along the theta dimension." << endl;
    cout << "-rdoms number     Sets the number of domains along the radius dimension." << endl;
    cout << "-thetadoms number Sets the number of domains along the theta dimension." << endl;
    cout << "-phidoms number   Sets the number of domains along the phi dimension." << endl;

#else
    cout << " [-nx number] [-ny number] [-nz number]";
    cout << " [-xdoms number] [-ydoms number] [-zdoms number]" << endl;
#endif
}

// ****************************************************************************
// Purpose: This program writes out mandelbulb data.
// 
// Programmer: Brad Whitlock
// Creation:   Thu Oct 23 01:21:19 PDT 2014
//
// Modifications:
//
// ****************************************************************************

int
main(int argc, char *argv[])
{
    int par_rank = 0, par_size = 1;
#ifdef SPHERICAL_COORDINATES
    // r,phi,theta (to match VisIt)
    int nDomains[3] = {2,5,10};
    int dims[3] = {100, 100, 100};
    float extents[6] = {0.6, 1.2f,    // r
                        0, M_PI,      // phi
                        0, 2. * M_PI, // theta
                       };
#else
    int nDomains[3] = {2,3,4}; //20, 30, 40};
    int dims[3] = {100, 100, 100};
    float extents[6] = {-1.2f, 1.1f, -1.1f, 1.1f, -1.1f, 1.1f};
#endif
    std::string output("mandelbulb");

#ifdef PARALLEL
    // Init MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &par_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &par_size);
#endif

    bool okay = true;
    for(int i = 1; i < argc; ++i)
    {
        if((strcmp(argv[i], "-nx") == 0 || 
            strcmp(argv[i], "-nr") == 0) && (i+1) < argc)
        {
            dims[0] = std::max(1, atoi(argv[i+1]));
            ++i;
        }
        else if((strcmp(argv[i], "-xdoms") == 0 || 
                 strcmp(argv[i], "-rdoms") == 0) && (i+1) < argc)
        {
            nDomains[0] = std::max(1, atoi(argv[i+1]));
            ++i;
        }
        else if((strcmp(argv[i], "-ny") == 0 || 
                 strcmp(argv[i], "-nphi") == 0) && (i+1) < argc)
        {
            dims[1] = std::max(1, atoi(argv[i+1]));
            ++i;
        }
        else if((strcmp(argv[i], "-ydoms") == 0 || 
                 strcmp(argv[i], "-phidoms") == 0) && (i+1) < argc)
        {
            nDomains[1] = std::max(1, atoi(argv[i+1]));
            ++i;
        }
        else if((strcmp(argv[i], "-nz") == 0 || 
                 strcmp(argv[i], "-ntheta") == 0) && (i+1) < argc)
        {
            dims[2] = std::max(1, atoi(argv[i+1]));
            ++i;
        }
        else if((strcmp(argv[i], "-zdoms") == 0 || 
                 strcmp(argv[i], "-thetadoms") == 0) && (i+1) < argc)
        {
            nDomains[2] = std::max(1, atoi(argv[i+1]));
            ++i;
        }
        else if(strcmp(argv[i], "-output") == 0 && (i+1) < argc)
        {
            output = argv[i+1];
            ++i;
        }
        else
        {
            if(par_rank == 0)
                PrintUsage(argv[0]);
            okay = false;
        }
    }

    if(okay)
    {
        // Come up with the domains that this processor will handle.
        std::vector<int> domListThisProc = DivideDomainList(par_rank, par_size, nDomains);

        WriteDomainFiles(par_rank, output, nDomains, dims, extents, domListThisProc);
        WriteMasterFile(par_rank, output, nDomains, dims, extents, domListThisProc);
    }

#ifdef PARALLEL
    MPI_Finalize();
#endif

    return 0;
}
