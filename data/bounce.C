/*****************************************************************************
*
* Copyright (c) 2000 - 2013, Lawrence Livermore National Security, LLC
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

#include <vector>
#include <string>

#ifdef _WIN32
#define drand48() (((float) rand())/((float) RAND_MAX)) 
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <silo.h>

// Parameters
#define MIN_MASS                 1.
#define MAX_MASS                 1.5

#define MIN_RESTITUTION          0.5
#define MAX_RESTITUTION          0.8

#define DELTA_TIME               0.05
#define MAX_TIME                 15.

#define MAX_PARTICLES            50000

#define GRAVITY_CONSTANT         9.81

#define DEFAULT_ZVELOCITY        -1.

#define NX_DOMAINS               2
#define NY_DOMAINS               2
#define NZ_DOMAINS               3

// ****************************************************************************
// Class: vector3
//
// Purpose:
//   Simple 3D vector class.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 17 15:19:26 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

class vector3
{
public:
    vector3() : x(0.), y(0.), z(0.) { }
    vector3(const vector3 &obj) : x(obj.x), y(obj.y), z(obj.z) { }
    vector3(double X, double Y, double Z) : x(X), y(Y), z(Z) { }

    vector3 operator + (const vector3 &obj) const
    {
        return vector3(x + obj.x, y + obj.y, z + obj.z);
    }

    vector3 operator - (const vector3 &obj) const
    {
        return vector3(x - obj.x, y - obj.y, z - obj.z);
    }

    void operator += (const vector3 &obj)
    {
        x += obj.x;
        y += obj.y;
        z += obj.z;
    }

    vector3 operator * (double v) const
    {
        return vector3(v * x, v * y, v * z);
    }

    double magnitude() const
    {
        return sqrt(x*x + y*y + z*z);
    }

    void normalize()
    {
        double mag = magnitude();
        if(mag != 0.)
        {
            x /= mag;
            y /= mag;
            z /= mag;
        }
    }

    vector3 normalized() const
    {
        vector3 tmp(x,y,z);
        tmp.normalize();
        return tmp;
    }

    double x,y,z;
};

vector3 operator * (double v, const vector3 &obj)
{
    return vector3(v * obj.x, v * obj.y, v * obj.z);
}

double distance(const vector3 &A, const vector3 &B)
{
    double dX = A.x - B.x;
    double dY = A.y - B.y;
    double dZ = A.z - B.z;

    return sqrt(dX*dX + dY*dY + dZ*dZ);
}

////////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Class: particle
//
// Purpose:
//   This class holds particle attributes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 17 15:19:54 PDT 2011
//
// Modifications:
//   Brad Whitlock, Mon Nov  7 14:39:14 PST 2011
//   I added a contact variable.
//
// ****************************************************************************

class particle
{
public:
    particle() : ID(0), goTime(0.), mass(0.), restitution(0.),
        location(), velocity(), acceleration(), contact(false)
    {
    }
    particle(const particle &p) : ID(p.ID), goTime(p.goTime), mass(p.mass), 
        restitution(p.restitution), location(p.location),
        velocity(p.velocity), acceleration(p.acceleration), contact(p.contact)
    {
    }

    int     ID;
    double  goTime;

    double  mass;          // g
    double  restitution;
    vector3 location;      // m
    vector3 velocity;      // m/s
    vector3 acceleration;  // m/s/s
    bool    contact;
};

typedef std::vector<particle> particleVector;

////////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Class: ParticleDataSaver
//
// Purpose:
//   Abstract base class for an object that can save particle data.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 17 15:20:21 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

class ParticleDataSaver
{
public:
    ParticleDataSaver(const std::string &name) : fileBase(name), nDomains(0)
    {
    }

    void SetCycleAndTime(int c, double t)
    {
        cycle = c;
        time = t;
    }

    void SetNumDomains(int nd)
    {
        nDomains = nd; 
    }

    virtual void CreateDomainDirectory() { }

    virtual void WriteBoundariesFile(const int *, const double *) = 0;
    virtual void WriteMasterFile(const bool *domainsHaveData) = 0;
    virtual bool WriteDomainFile(int dom, const particleVector &P) = 0;
protected:
    std::string intToString05(int a) const
    {
        char tmp[10];
        sprintf(tmp, "%05d", a);
        return std::string(tmp);
    }
    std::string intToString(int a) const
    {
        char tmp[10];
        sprintf(tmp, "%d", a);
        return std::string(tmp);
    }

    std::string fileBase;
    int         nDomains;

    int         cycle;
    double      time;
};

// ****************************************************************************
// Class: SiloDataSaver
//
// Purpose:
//   Save particle data as a Silo file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 17 15:20:47 PDT 2011
//
// Modifications:
//   Brad Whitlock, Mon Nov  7 10:57:20 PST 2011
//   Add a mesh that shows the domain boundaries.
//
// ****************************************************************************

class SiloDataSaver : public ParticleDataSaver
{
public:
    SiloDataSaver(const std::string &name) : ParticleDataSaver(name), dirName()
    {
    }

    virtual void CreateDomainDirectory()
    { 
#ifndef _WIN32
        dirName = fileBase + intToString05(cycle);

        // Make a directory in which to store the domain files.
        mkdir(dirName.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);

        // Append a slash so we can use dirName in creating filenames
        dirName += "/";
#endif        
    }

    virtual void WriteMasterFile(const bool *domainsHaveData)
    {
        std::string filename(MasterFile());        
        DBfile *dbfile = DBCreate(filename.c_str(), DB_CLOBBER, DB_LOCAL,
                                  "3D point mesh", DB_HDF5);
        if(dbfile == NULL)
        {
            fprintf(stderr, "Could not create master Silo file!\n");
            return;
        }

        // Create an option list for saving cycle and time values.
        DBoptlist *optlist = DBMakeOptlist(2);
        DBAddOption(optlist, DBOPT_CYCLE, (void *)&cycle);
        DBAddOption(optlist, DBOPT_DTIME, (void *)&time);

        const char *snames[] = {"pointmesh", "vx", "vy", "vz", "restitution", "mass", "dom", "contact"};
        const int svartypes[] = {DB_POINTMESH, DB_POINTVAR, DB_POINTVAR, DB_POINTVAR, DB_POINTVAR,
                                               DB_POINTVAR, DB_POINTVAR, DB_POINTVAR};
        const bool isMesh[] = {true, false, false, false, false, false, false, false};
        for(int i = 0; i < (sizeof(snames)/sizeof(const char*)); ++i)
        {
            std::vector<std::string> names;
            for(int d = 0; d < nDomains; ++d)
            {
                std::string domName("EMPTY");
                if(domainsHaveData[d])
                    domName = (DomainFile(d) + ":") + snames[i];
                names.push_back(domName);
            }
            char **varnames = new char *[nDomains];
            int *vartypes = new int[nDomains];
            for(int d = 0; d < nDomains; ++d)
            {
                varnames[d] = (char*)names[d].c_str();
                vartypes[d] = svartypes[i];
            }

            if(isMesh[i])
                DBPutMultimesh(dbfile, snames[i], nDomains, varnames, vartypes, optlist);
            else
                DBPutMultivar(dbfile, snames[i], nDomains, varnames, vartypes, optlist);
        }

        const char *names[] = {"velocity", "speed", "ID"};
        const char *defs[] = {"{vx,vy,vz}", "magnitude(velocity)", "global_nodeid(pointmesh)"};
        int types[] = {DB_VARTYPE_VECTOR, DB_VARTYPE_SCALAR, DB_VARTYPE_SCALAR};
        DBPutDefvars(dbfile, "defvars", 3, (char**)names, types, (char**)defs, NULL);
        DBFreeOptlist(optlist);

        DBClose(dbfile);
    }

    virtual void WriteBoundariesFile(const int *divisions, const double *extents)
    {
        std::string filename(BoundariesFile());
        DBfile *dbfile = DBCreate(filename.c_str(), DB_CLOBBER, DB_LOCAL,
                                  "3D point mesh domain boundaries", DB_HDF5);
        if(dbfile == NULL)
        {
            fprintf(stderr, "Could not create Silo file!\n");
            return;
        }

        double deltaX = (extents[1] - extents[0]) / double(divisions[0]);
        double deltaY = (extents[3] - extents[2]) / double(divisions[1]);
        double deltaZ = (extents[5] - extents[4]) / double(divisions[2]);

        char domname[30];
        double z0 = extents[4];
        int dom = 0;
        std::vector<std::string> names;
        int nDomains = divisions[0]*divisions[1]*divisions[2];
        char **varnames = new char *[nDomains];
        int *vartypes = new int[nDomains];
        for(int dz = 0; dz < divisions[2]; ++dz)
        {
            double y0 = extents[2];
            for(int dy = 0; dy < divisions[1]; ++dy)
            {
                double x0 = extents[0];
                for(int dx = 0; dx < divisions[0]; ++dx, ++dom)
                {
                    sprintf(domname, "domain%d", dom);
                    DBMkDir(dbfile, domname);
                    DBSetDir(dbfile, domname);

                    sprintf(domname, "domain%d/bounds", dom);
                    names.push_back(domname);
                    varnames[dom] = (char *)names[dom].c_str();
                    vartypes[dom] = DB_QUADMESH;

                    double thisExtents[6];
                    thisExtents[0] = x0 + (dx)   * deltaX;
                    thisExtents[1] = x0 + (dx+1) * deltaX;
                    thisExtents[2] = y0 + (dy)   * deltaX;
                    thisExtents[3] = y0 + (dy+1) * deltaY;
                    thisExtents[4] = z0 + (dz)   * deltaZ;
                    thisExtents[5] = z0 + (dz+1) * deltaZ;

                    // Create a simple mesh that shows this domain's boundaries.
                    const int nvals = 4;
                    double x[nvals];
                    double y[nvals];
                    double z[nvals];
                    for(size_t i = 0; i < nvals; ++i)
                    {
                        double t = double(i) / double(nvals-1);
                        x[i] = (1.-t)*thisExtents[0] + t*thisExtents[1];
                        y[i] = (1.-t)*thisExtents[2] + t*thisExtents[3];
                        z[i] = (1.-t)*thisExtents[4] + t*thisExtents[5];
                    }
                    int dims[3] = {nvals, nvals, nvals};
                    int ndims = 3;
                    float *coords[3];
                    coords[0] = (float*)x;
                    coords[1] = (float*)y;
                    coords[2] = (float*)z;
                    DBPutQuadmesh(dbfile, "bounds", NULL, coords, dims, ndims,
                        DB_DOUBLE, DB_COLLINEAR, NULL);

                    DBSetDir(dbfile, "..");
                }
            }
        }
        DBPutMultimesh(dbfile, "bounds", nDomains, varnames, vartypes, NULL);

        delete [] varnames;
        delete [] vartypes;

        DBClose(dbfile);
    }

    virtual bool WriteDomainFile(int dom, const particleVector &P)
    {
        if(P.empty())
            return false;

        std::string filename(DomainFile(dom));
        DBfile *dbfile = DBCreate(filename.c_str(), DB_CLOBBER, DB_LOCAL,
                                  "3D point mesh", DB_HDF5);
        if(dbfile == NULL)
        {
            fprintf(stderr, "Could not create Silo file!\n");
            return false;
        }

        double *m, *x, *y, *z, *vx, *vy, *vz, *r;
        int *id, *doms, *contact;
        int dims[3];
        int ndims = 3;
        float *coords[3];
        x = new double[P.size()];
        y = new double[P.size()];
        z = new double[P.size()];
        vx = new double[P.size()];
        vy = new double[P.size()];
        vz = new double[P.size()];
        m = new double[P.size()];
        r = new double[P.size()];
        id = new int[P.size()];
        doms = new int[P.size()];
        contact = new int[P.size()];

        dims[0] = (int)P.size();
        dims[1] = (int)P.size();
        dims[2] = (int)P.size();
        coords[0] = (float*)x;
        coords[1] = (float*)y;
        coords[2] = (float*)z;

        for(size_t i = 0; i < P.size(); ++i)
        { 
            x[i] = P[i].location.x;
            y[i] = P[i].location.y;
            z[i] = P[i].location.z;

            vx[i] = P[i].velocity.x;
            vy[i] = P[i].velocity.y;
            vz[i] = P[i].velocity.z;

            m[i] = P[i].mass;
            r[i] = P[i].restitution;
            id[i] = P[i].ID;
            doms[i] = dom+1;
            contact[i] = P[i].contact ? 1 : 0;
        }

        // Create an option list for saving cycle and time values.
        DBoptlist *optlist = DBMakeOptlist(9);
        DBAddOption(optlist, DBOPT_CYCLE, (void *)&cycle);
        DBAddOption(optlist, DBOPT_DTIME, (void *)&time);
        DBAddOption(optlist, DBOPT_NODENUM, (void *)id);
        DBAddOption(optlist, DBOPT_XUNITS, (void *)"m");
        DBAddOption(optlist, DBOPT_YUNITS, (void *)"m");
        DBAddOption(optlist, DBOPT_ZUNITS, (void *)"m");
        DBAddOption(optlist, DBOPT_XLABEL, (void *)"Width");
        DBAddOption(optlist, DBOPT_YLABEL, (void *)"Height");
        DBAddOption(optlist, DBOPT_ZLABEL, (void *)"Depth");

        DBoptlist *voptlist = DBMakeOptlist(3);
        DBAddOption(voptlist, DBOPT_CYCLE, (void *)&cycle);
        DBAddOption(voptlist, DBOPT_DTIME, (void *)&time);

        // Write a point mesh.
        DBPutPointmesh(dbfile, "pointmesh", ndims, coords, P.size(),
                       DB_DOUBLE, optlist);

        // Write variables.
        DBPutPointvar1(dbfile, "vx", "pointmesh", vx, P.size(), DB_DOUBLE, voptlist);
        DBPutPointvar1(dbfile, "vy", "pointmesh", vy, P.size(), DB_DOUBLE, voptlist);
        DBPutPointvar1(dbfile, "vz", "pointmesh", vz, P.size(), DB_DOUBLE, voptlist);

        DBPutPointvar1(dbfile, "mass", "pointmesh", m, P.size(), DB_DOUBLE, voptlist);
        DBPutPointvar1(dbfile, "restitution", "pointmesh", r, P.size(), DB_DOUBLE, voptlist);
        DBPutPointvar1(dbfile, "dom", "pointmesh", doms, P.size(), DB_INT, voptlist);
        DBPutPointvar1(dbfile, "contact", "pointmesh", contact, P.size(), DB_INT, voptlist);

        delete [] x;
        delete [] y;
        delete [] z;
        delete [] vx;
        delete [] vy;
        delete [] vz;
        delete [] m;
        delete [] r;
        delete [] id;
        delete [] doms;
        delete [] contact;

        DBFreeOptlist(optlist);
        DBFreeOptlist(voptlist);
        DBClose(dbfile);

        return true;
    }

protected:
    std::string MasterFile() const 
    {
        return std::string(fileBase + intToString05(cycle) + ".silo");
    }

    std::string BoundariesFile() const 
    {
        return std::string(fileBase + ".boundaries.silo");
    }

    std::string DomainFile(int dom) const
    {
        return std::string(dirName + (fileBase + intToString05(cycle) + ".") + intToString(dom) + ".silo");
    }

    std::string dirName;
};

////////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Class: SimState
//
// Purpose:
//   Contains simulation state for a bouncing particle simulation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 17 15:15:57 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

struct SimState
{
    SimState();

    void   CreateParticles(int np);
    void   Advance();
    void   SaveData(ParticleDataSaver &saver);

    // State variables
    int            cycle;
    double         time;          // s

    int            maxParticles;
    particleVector particles;

    bool           createAll;
    bool           keepParticles;
    double         dT;            // s
    double         maxTime;       // s
    double         extents[6];    // m
    int            divisions[3];

    double NumberInRange(double v0, double v1);
    double Random01();
};

// ****************************************************************************
// Method: SimState::SimState
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 17 15:18:44 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

SimState::SimState() : cycle(0), time(0.), maxParticles(MAX_PARTICLES), particles(), 
    createAll(true), keepParticles(true),dT(DELTA_TIME), maxTime(MAX_TIME)
{
    extents[0] = -20.;
    extents[1] = 20.;
    extents[2] = -20.;
    extents[3] = 20.;
    extents[4] = 0.;
    extents[5] = 30.;

    divisions[0] = NX_DOMAINS;
    divisions[1] = NY_DOMAINS;
    divisions[2] = NZ_DOMAINS;
}

// ****************************************************************************
// Method: SimState::CreateParticles
//
// Purpose: 
//   Create new particles in an emitting disc and add them to the particle list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 17 15:18:15 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
SimState::CreateParticles(int numNewParticles)
{
    vector3 emitterCenter(0., 0., extents[5]);
    const double emitterRadius = 1.;
    const double emitterRadius2 = 2000000.;

    int newID = (int)particles.size();
    for(int i = 0; i < numNewParticles; ++i)
    {
        particle P;
        P.ID = newID++;

        P.goTime = createAll ? NumberInRange(0., maxTime*0.5) : 0.;

        P.mass = NumberInRange(MIN_MASS, MAX_MASS);
        P.restitution = NumberInRange(MIN_RESTITUTION, MAX_RESTITUTION);

        // location
        double a = NumberInRange(0, 2 * M_PI);
        double rad = NumberInRange(0., emitterRadius);
        P.location = vector3(
            emitterCenter.x + rad * cos(a),
            emitterCenter.y + rad * sin(a),
            emitterCenter.z - NumberInRange(0, 0.4*(extents[5]-extents[4])*dT)
            );

        // velocity
        vector3 A(P.location.x, P.location.y, 0.);
        vector3 A_norm(A.normalized());
        double normR = A.magnitude() / emitterRadius;
        P.velocity = vector3(2. * normR * A_norm.x, 2. * normR * A_norm.y, DEFAULT_ZVELOCITY);

        // acceleration
        P.acceleration = vector3(0.,0.,-GRAVITY_CONSTANT);

        particles.push_back(P);
    }
}

double
SimState::NumberInRange(double v0, double v1) 
{
    double t = Random01();
    return (1.-t)*v0 + t*v1;
}

double
SimState::Random01() 
{
    return drand48();
}

// ****************************************************************************
// Method: SimState::Advance
//
// Purpose: 
//   Advance the simulation one time step.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 17 15:17:43 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
SimState::Advance()
{
    // Move the particles we have.
    for(size_t i = 0; i < particles.size(); ++i)
    {
        if(particles[i].goTime <= time)
        {
            particles[i].velocity += particles[i].acceleration * dT;
            particles[i].location += particles[i].velocity * dT;
            particles[i].contact = false;

            if(particles[i].location.z < 0.)
            {
                // bounce.
                particles[i].location.z = 0.;
                particles[i].contact = true;

                particles[i].velocity = vector3(
                    particles[i].velocity.x,
                    particles[i].velocity.y,
                   -particles[i].velocity.z * particles[i].restitution
                    );
            }

            // If we're keeping particles trapped in the box so they don't
            // leave then make sure we bounce them off of the XY walls too.
            if(keepParticles)
            {
                if(particles[i].location.x < extents[0])
                {
                    particles[i].location.x = extents[0];
                    particles[i].velocity = vector3(
                       -particles[i].velocity.x * particles[i].restitution,
                        particles[i].velocity.y,
                        particles[i].velocity.z
                        );
                    particles[i].contact = true;
                }
                if(particles[i].location.x > extents[1])
                {
                    particles[i].location.x = extents[1];
                    particles[i].velocity = vector3(
                       -particles[i].velocity.x * particles[i].restitution,
                        particles[i].velocity.y,
                        particles[i].velocity.z
                        );
                    particles[i].contact = true;
                }
                if(particles[i].location.y < extents[2])
                {
                    particles[i].location.y = extents[2];
                    particles[i].velocity = vector3(
                        particles[i].velocity.x,
                       -particles[i].velocity.y * particles[i].restitution,
                        particles[i].velocity.z
                        );
                    particles[i].contact = true;
                }
                if(particles[i].location.y > extents[3])
                {
                    particles[i].location.y = extents[3];
                    particles[i].velocity = vector3(
                        particles[i].velocity.x,
                       -particles[i].velocity.y * particles[i].restitution,
                        particles[i].velocity.z
                        );
                    particles[i].contact = true;
                }
            }
        }
    }

    cycle++;
    time += dT;
}

// ****************************************************************************
// Method: SimState::SaveData
//
// Purpose: 
//   Save the particle simulation data into different domains using the saver.
//
// Arguments:
//   saver : An object that knows how to write domain and master files.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 17 15:16:52 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
SimState::SaveData(ParticleDataSaver &saver)
{
    // For each domain, figure out which particles are in it and save out
    // a file.
    int dom = 0;

    double deltaX = (extents[1] - extents[0]) / double(divisions[0]);
    double deltaY = (extents[3] - extents[2]) / double(divisions[1]);
    double deltaZ = (extents[5] - extents[4]) / double(divisions[2]);

    bool *domainsHaveData = new bool[divisions[0]*divisions[1]*divisions[2]];

    saver.SetCycleAndTime(cycle, time);
    saver.SetNumDomains(divisions[0] * divisions[1] * divisions[2]);
    saver.CreateDomainDirectory();

    double z0 = extents[4];
    for(int dz = 0; dz < divisions[2]; ++dz)
    {
        double y0 = extents[2];
        for(int dy = 0; dy < divisions[1]; ++dy)
        {

            double x0 = extents[0];
            for(int dx = 0; dx < divisions[0]; ++dx, ++dom)
            {
                particleVector P;

                // Select the particles in this domain's box.
                for(int i = 0; i < particles.size(); ++i)
                {
                    const vector3 &L = particles[i].location;
                    bool inX = (L.x >= x0 && L.x <= (x0 + deltaX));
                    bool inY = (L.y >= y0 && L.y <= (y0 + deltaY));
                    bool inZ = (dz==divisions[2]-1) ? (L.z >= z0) : (L.z >= z0 && L.z <= (z0 + deltaZ));
                    if(inX && inY && inZ)
                    {
                        P.push_back(particles[i]);
                    }
                }

                domainsHaveData[dom] = saver.WriteDomainFile(dom, P);

                x0 += deltaX;
            }

            y0 += deltaY;
        }

        z0 += deltaZ;
    }

    saver.WriteMasterFile(domainsHaveData);
    delete [] domainsHaveData;
}

// ****************************************************************************
// Method: main
//
// Purpose: 
//   The main function
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 17 15:15:37 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

int
main(int argc, char *argv[])
{
    SimState sim;
    const char *fileBase = "bounce";

    // Parse some command line arguments.
    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "-emit") == 0)
            sim.createAll = false;
        else if(strcmp(argv[i], "-keepParticles") == 0)
            sim.keepParticles = true;
        else if(strcmp(argv[i], "-noKeepParticles") == 0)
            sim.keepParticles = false;
        else if(strcmp(argv[i], "-o") == 0 && (i+1)<argc)
        {
            fileBase = argv[i+1];
            ++i;
        }
        else if(strcmp(argv[i], "-nparticles") == 0 && (i+1)<argc)
        {
            int n = atoi(argv[i+1]);
            if(n <= 0)
                printf("The number of particles must be greater than 0.\n");
            else
                sim.maxParticles = n;
            ++i;
        }
        else if(strcmp(argv[i], "-nx") == 0 && (i+1)<argc)
        {
            int n = atoi(argv[i+1]);
            if(n <= 0)
                printf("The number of divisions must be greater than 0.\n");
            else
                sim.divisions[0] = n;
            ++i;
        }
        else if(strcmp(argv[i], "-ny") == 0 && (i+1)<argc)
        {
            int n = atoi(argv[i+1]);
            if(n <= 0)
                printf("The number of divisions must be greater than 0.\n");
            else
                sim.divisions[1] = n;
            ++i;
        }
        else if(strcmp(argv[i], "-nz") == 0 && (i+1)<argc)
        {
            int n = atoi(argv[i+1]);
            if(n <= 0)
                printf("The number of divisions must be greater than 0.\n");
            else
                sim.divisions[2] = n;
            ++i;
        }
        else
        {
            printf("Usage: %s [-emit] [-keepParticles] [-noKeepParticles] [-o filebase] [-nparticles n]\n\n", argv[0]);
            printf("\t-nparticles n     Set the max number of particles if we're not emitting.\n");
            printf("\t-nx n             The number of domains in X.\n");
            printf("\t-ny n             The number of domains in Y.\n");
            printf("\t-nz n             The number of domains in Z.\n");
            printf("\t-emit             Do not create all particles upfront. Emit them as time advances.\n");
            printf("\t-keepParticles    Keep all particles in the box. If this is true then we bounce the\n");
            printf("\t                  particles off of the XY walls to keep them.\n");
            printf("\t-noKeepParticles  If particles venture outside the XY walls, let them escape\n");
            return -1;
        }
    }

    // Add particles at the corners so we don't get changing extents.
    particle P;
    P.ID = 0;
    P.location = vector3(sim.extents[0], sim.extents[2], sim.extents[4]);
    sim.particles.push_back(P);
    P.ID = 1;
    P.location = vector3(sim.extents[1], sim.extents[2], sim.extents[4]);
    sim.particles.push_back(P);
    P.ID = 2;
    P.location = vector3(sim.extents[1], sim.extents[3], sim.extents[4]);
    sim.particles.push_back(P);
    P.ID = 3;
    P.location = vector3(sim.extents[0], sim.extents[3], sim.extents[4]);
    sim.particles.push_back(P);
    P.ID = 4;
    P.location = vector3(sim.extents[0], sim.extents[2], sim.extents[5]);
    sim.particles.push_back(P);
    P.ID = 5;
    P.location = vector3(sim.extents[1], sim.extents[2], sim.extents[5]);
    sim.particles.push_back(P);
    P.ID = 6;
    P.location = vector3(sim.extents[1], sim.extents[3], sim.extents[5]);
    sim.particles.push_back(P);
    P.ID = 7;
    P.location = vector3(sim.extents[0], sim.extents[3], sim.extents[5]);
    sim.particles.push_back(P);

    // Create the initial particles. This will either be all of them or
    // just a few if we're going to emit.
    if(sim.createAll)
        sim.CreateParticles(sim.maxParticles - 8);
    else
        sim.CreateParticles(100);

    SiloDataSaver saver(fileBase);
    saver.WriteBoundariesFile(sim.divisions, sim.extents);

    // Advance through time.
    while(sim.time < sim.maxTime)
    {
        sim.SaveData(saver);
        printf("Saved cycle %d, time %lg\n", sim.cycle, sim.time);
        sim.Advance();

        // If we're emitting then create more particles.
        if(!sim.createAll && sim.time < sim.maxTime/2.)
            sim.CreateParticles(100);
    }
    sim.SaveData(saver);

    return 0;
}
