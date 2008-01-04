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

#include <string>
#include <vector>
#include <map>
#include <visitstream.h>

#include <visit-config.h>
#ifdef HAVE_PDB_PROPER
#include <pdb.h>
#else
#include <lite_pdb.h>
#endif

using std::string;
using std::vector;

class FieldWriter;

// ****************************************************************************
// Class: Field
//
// Purpose:
//   Base class for a set of values.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 24 11:47:14 PDT 2003
//
// Modifications:
//   
//   Mark C. Miller, Sat Feb  3 00:42:05 PST 2007
//   Added support for array variables with number of components (nc) > 1
// ****************************************************************************

class Field
{
public:
    Field(int nts_, int nc_, int nx_, int ny_, bool nodal_) : name()
    {
        nts = nts_;
        nx = nx_;
        ny = ny_;
        nc = nc_;
        nodal = nodal_;
        dynamic = false;
    }

    virtual ~Field()
    {
    }

    virtual void WriteData(FieldWriter &pdb, const string &var, int ts, int nts) = 0;

    void SetDynamic(bool val) { dynamic = val; }
    bool GetDynamic() const { return dynamic; }

    virtual int GetNX() const { return nx; }
    virtual int GetNY() const { return ny; }
    virtual int GetNTS() const { return nts; }
    virtual int GetNC() const { return nc; }
    bool GetNodal() const { return nodal; }

    void SetName(const string &n) { name = n; }
    const string &GetName() const { return name; }
protected:
    string name;
    bool dynamic;
    int  nx;
    int  ny;
    int  nts;
    int  nc;
    bool nodal;
};

// ****************************************************************************
// Class: FieldWithData
//
// Purpose:
//   Template class that contains data for a mesh that varies over time.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 24 11:48:20 PDT 2003
//
// Modifications:
//   
//   Mark C. Miller, Sat Feb  3 00:42:05 PST 2007
//   Added support for array variables with number of components (nc) > 1
//   
// ****************************************************************************

template <class T>
class FieldWithData : public Field
{
public:
    FieldWithData(int nts_, int nc_, int nx_, int ny_, T defaultVal, bool nodal_=false) :
        Field(nts_, nc_, nx_, ny_, nodal_)
    {
        int nvals = nx * ny * nts * nc;
        data = new T[nvals];
        T *tmp = data;
        for(int i = nvals; i > 0; --i)
            *tmp++ = defaultVal;
    }

    virtual ~FieldWithData()
    {
        if(data)
            delete [] data;
        data = 0;
    }

    void SetValue(int ts, int c, int x, int y, T val)
    {
        int nvals = nc * nx * ny;
        T *timeData = data + nvals * ts;
        timeData[c * nx * ny + y * nx + x] = val;
    }

    T GetValue(int ts, int c, int x, int y)
    {
        int nvals = nc * nx * ny;
        T *timeData = data + nvals * ts;
        return timeData[c * nx * ny + y * nx + x];
    }

    void CopyData(int destTime, int srcTime)
    {
        int nvals = nc * nx * ny;
        const T *src = data + srcTime * nvals;
        T *dest = data + destTime * nvals;
        for(int i = 0; i < nvals; ++i)
            dest[i] = src[i];
    }

    void LinearInterpolate(double t, int destTime, int t0, int t1)
    {
        int nvals = nc * nx * ny;
        double omt = 1. - t;
        const T *t0data = data + t0 * nvals;
        const T *t1data = data + t1 * nvals;
        T *dest = data + destTime * nvals;

        for(int i = 0; i < nvals; ++i)
            dest[i] = T(omt * t0data[i]) + T(t * t1data[i]);
    }

    virtual void WriteData(FieldWriter &pdb, const string &var, int ts, int nts);

protected:
    T *data;
};

// ****************************************************************************
// Class: FieldWithData1D
//
// Purpose:
//   Derived type that writes the data with 1D shape instead of 2D shape.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 9 10:25:51 PDT 2004
//
// Modifications:
//    Jeremy Meredith, Fri Aug 25 16:49:53 EDT 2006
//    Newer gcc's won't resolve unqualified members of a dependent base in 
//    templates.  See [temp.dep]/3 in the ANSI C++ Standard.  Using explicit
//    this-> fixes it (as would adding explicit "using" declaration).
//
//   Mark C. Miller, Sat Feb  3 00:42:05 PST 2007
//   Added support for array variables with number of components (nc) > 1
// ****************************************************************************

template <class T>
class FieldWithData1D : public FieldWithData<T>
{
public:
    FieldWithData1D(int nts_, int nx_, T defaultVal, bool nodal=false) : 
        FieldWithData<T>(nts_, 1, nx_, 1, defaultVal, nodal)
    {
    }

    virtual ~FieldWithData1D()
    {
    }

    void SetValue(int ts, int x, T val)
    {
        T *timeData = this->data + this->nx * ts;
        timeData[x] = val;
    }

    virtual void WriteData(FieldWriter &pdb, const string &var, int ts, int nts);
};

// ****************************************************************************
// Class: LinearArray
//
// Purpose:
//   Another kind of field but it is not defined on a mesh.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 24 11:48:50 PDT 2003
//
// Modifications:
//   
//   Mark C. Miller, Sat Feb  3 00:42:05 PST 2007
//   Added support for array variables with number of components (nc) > 1
// ****************************************************************************

template <class T>
class LinearArray : public Field
{
public:
    LinearArray() : Field(0,0,0,0,false), data()
    {
    }

    virtual ~LinearArray()
    {
    }

    void AddValue(T val)
    {
        data.push_back(val);
    }

    T GetValue(int index)
    {
        return data[index];
    }

    virtual int GetNX() const { return data.size(); }
    virtual int GetNY() const { return 1; }

    virtual void WriteData(FieldWriter &pdb, const string &var, int ts, int nts);
private:
    std::vector<T> data;
};

// ****************************************************************************
// Class: FieldWriter
//
// Purpose:
//   Takes care of writing fields to a PDB file.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 24 11:49:22 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Jul 23 14:20:32 PST 2004
//   I made it write out a database comment.
//
//   Brad Whitlock, Tue Dec 7 17:16:31 PST 2004
//   Added WriteFloatArray.
//
// ****************************************************************************

class FieldWriter
{
    typedef std::map<std::string, Field *> FieldMap;
public:
    FieldWriter() : cycles(), times(), databaseComment("   "), fields()
    {
        pdb = 0;
    }

    virtual ~FieldWriter()
    {
        Close();

        // Delete the fields.
        for(FieldMap::iterator vars = fields.begin();
            vars != fields.end();
            ++vars)
        {
            if(vars->second->GetDynamic())
                delete vars->second;
        }
    }

    void AddTimeStep(int c, double t)
    {
        cycles.push_back(c);
        times.push_back(t);
    }

    void SetDatabaseComment(const std::string &c)
    {
        databaseComment = c;
    }

    bool Open(const char *fileName)
    {
         pdb = PD_open((char *)fileName, "w");
         return pdb != 0;
    }

    void Close()
    {
        if(pdb)
            PD_close(pdb);
        pdb = 0;
    }

    void WriteData(const char *base, int kmax, int lmax, int tsPerFile, bool writemixedmats)
    {
        for(int ts = 0; ts < NumTimeSteps(); ts += tsPerFile)
        {
            string nodalString;

            // Determine how many time states will be in this file.
            int tsInFile = tsPerFile;
            if(ts + tsPerFile > NumTimeSteps())
                tsInFile = NumTimeSteps() - ts;

            // Create the filename.
            char fileName[1000];
            sprintf(fileName, "%s%02d.pdb", base, ts);

            if(Open(fileName))
            {
                if(tsInFile > 1)
                {
                    cout << "Writing timesteps:" << ts << "-"
                         << ts+tsInFile-1 << " to " << fileName << endl;
                }
                else
                {
                    cout << "Writing timestep:" << ts << " to " << fileName
                         << endl;
                }

                // Write the cycles and time.
                WriteString("cycle_variable@value", "cycles@history");
                WriteString("time_variable@value", "times@history");
                WriteIntArray("cycles@history", &cycles[ts], tsInFile);
                WriteDoubleArray("times@history", &times[ts], tsInFile);
                WriteString("idates@value", databaseComment);

                // Write the fields.
                bool sizeWritten = false;
                for(FieldMap::iterator vars = fields.begin();
                    vars != fields.end();
                    ++vars)
                {
                    if(!sizeWritten)
                    {
                        sizeWritten = true;
                        WriteInt("kmax@value", kmax);
                        WriteInt("lmax@value", lmax);
                    }

                    // Add the field's name to the nodal string if the
                    // field is nodal.
                    AddToNodalString(vars->first, nodalString);

                    // Write out the data.
                    vars->second->WriteData(*this, vars->second->GetName(), ts, tsInFile);
                }

                if(nodalString.size() > 0)
                    WriteString("pc_list@global", nodalString);

                if(writemixedmats)
                {
                    WriteInt("nszmmt@las", 1);
                    WriteInt("nreg@las", 4);
                }

                // Close the PDB file.
                Close();
            }
        }
    }

    int NumTimeSteps() const
    {
        return cycles.size();
    }

    void AddField(const string &key, const string &var, Field *f)
    {
        f->SetName(var);
        fields[key] = f;
    }

    void AddField(const string &key, const string &var, Field *f, bool dynamic)
    {
        f->SetName(var);
        f->SetDynamic(dynamic);
        fields[key] = f;
    }

    PDBfile *FilePointer()
    {
        return pdb;
    }

    void WriteInt(const string &var, int val)
    {
        PD_write(pdb, (char *)var.c_str(), "integer", (void*)&val);
    }

    void WriteIntArray(const string &var, const int *vals, int nvals)
    {
        char formatString[100];
        if(nvals > 1)
            sprintf(formatString, "%s(%d)", var.c_str(), nvals);
        else
            strcpy(formatString, var.c_str());
        PD_write(pdb, formatString, "integer", (void*)vals);
    }

    void WriteFloatArray(const string &var, const float *vals, int nvals)
    {
        char formatString[100];
        if(nvals > 1)
            sprintf(formatString, "%s(%d,1)", var.c_str(), nvals);
        else
            strcpy(formatString, var.c_str());
        PD_write(pdb, formatString, "float", (void*)vals);
    }

    void WriteDoubleArray(const string &var, const double *vals, int nvals)
    {
        char formatString[100];
        if(nvals > 1)
            sprintf(formatString, "%s(%d,1)", var.c_str(), nvals);
        else
            strcpy(formatString, var.c_str());
        PD_write(pdb, formatString, "double", (void*)vals);
    }

    void WriteString(const string &var, const string &val)
    {
        char formatString[100];
        sprintf(formatString, "%s(%d)", var.c_str(), val.size() + 1);
        PD_write(pdb, formatString, "char", (void*)val.c_str());
    }

protected:
    void AddToNodalString(const string &var, string &nodalString) const
    {
        FieldMap::const_iterator t = fields.find(var);
        if(t != fields.end())
        {
            // If the field is nodal, add its name to the nodal string.
            if(t->second->GetNodal())
            {
                if(var.size() == 1)
                    nodalString += (var + " ");
                else if(var.size() == 2)
                    nodalString += var;
                else
                {
                    cerr << "Can't add " << var
                         << " as a nodal string because the name is too long."
                         << endl;
                }
            }
        }
    }

private:
    PDBfile         *pdb;
    vector<int>      cycles;
    vector<double>   times;
    std::string      databaseComment;
    FieldMap         fields;
};

// ****************************************************************************
// Method: FieldWithData<>::WriteData
//
// Purpose: 
//   Template specialization functions that write field data to the data file.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 24 11:51:35 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Dec 7 17:14:33 PST 2004
//   Added a float specialization.
//
//   Mark C. Miller, Sat Feb  3 00:42:05 PST 2007
//   Added support for array variables with number of components (nc) > 1
//
//   Thomas R. Treadway, Thu Feb  8 14:35:45 PST 2007
//   Corrected template syntax, required for gcc-4.x support
//
// ****************************************************************************

template <> void
FieldWithData<int>::WriteData(FieldWriter &pdb, const string &var, int ts, int nts)
{
/*    cerr << "Writing the int data for " << var
         << " ts="<<ts<<", nts=" << nts << endl;
*/
    char formatString[100];
    if(nts > 1)
    {
        if (nc > 1)
            sprintf(formatString, "%s(%d,%d,%d,%d)", var.c_str(), nts, nc, nx, ny);
        else
            sprintf(formatString, "%s(%d,%d,%d)", var.c_str(), nts, nx, ny);
    }
    else
    {
        if (nc > 1)
            sprintf(formatString, "%s(%d,%d,%d)", var.c_str(), nc, nx, ny);
        else
            sprintf(formatString, "%s(%d,%d)", var.c_str(), nx, ny);
    }
    int *timeData = data + (nx * ny * nc * ts);
    PD_write(pdb.FilePointer(), formatString, "integer", (void*)timeData);
}

template <> void
FieldWithData<double>::WriteData(FieldWriter &pdb, const string &var, int ts, int nts)
{
/*    cerr << "Writing the double data for " << var
         << " ts="<<ts<<", nts=" << nts << endl;
*/
    char formatString[100];
    if(nts > 1)
    {
        if (nc > 1)
            sprintf(formatString, "%s(%d,%d,%d,%d)", var.c_str(), nts, nc, nx, ny);
        else
            sprintf(formatString, "%s(%d,%d,%d)", var.c_str(), nts, nx, ny);
    }
    else
    {
        if (nc > 1)
            sprintf(formatString, "%s(%d,%d,%d)", var.c_str(), nc, nx, ny);
        else
            sprintf(formatString, "%s(%d,%d)", var.c_str(), nx, ny);
    }

    double *timeData = data + (nx * ny * nc * ts);
    PD_write(pdb.FilePointer(), formatString, "double", (void*)timeData);
}

template <> void
FieldWithData<float>::WriteData(FieldWriter &pdb, const string &var, int ts, int nts)
{
/*    cerr << "Writing the float data for " << var
         << " ts="<<ts<<", nts=" << nts << endl;
*/
    char formatString[100];
    if(nts > 1)
    {
        if (nc > 1)
            sprintf(formatString, "%s(%d,%d,%d,%d)", var.c_str(), nts, nc, nx, ny);
        else
            sprintf(formatString, "%s(%d,%d,%d)", var.c_str(), nts, nx, ny);
    }
    else
    {
        if (nc > 1)
            sprintf(formatString, "%s(%d,%d,%d)", var.c_str(), nc, nx, ny);
        else
            sprintf(formatString, "%s(%d,%d)", var.c_str(), nx, ny);
    }

    float *timeData = data + (nx * ny * ts);
    PD_write(pdb.FilePointer(), formatString, "float", (void*)timeData);
}

// ****************************************************************************
// Method: FieldWithData1D<>::WriteData
//
// Purpose: 
//   Template specialization functions that write field data to the data file.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 9 10:27:06 PDT 2004
//
// Modifications:
//   Brad Whitlock, Tue Dec 7 17:15:04 PST 2004
//   Added a float specialization.
//
//   Thomas R. Treadway, Thu Feb  8 14:35:45 PST 2007
//   Corrected template syntax, required for gcc-4.x support
//
// ****************************************************************************

template <> void
FieldWithData1D<int>::WriteData(FieldWriter &pdb, const string &var, int ts, int nts)
{
/*    cerr << "Writing the int data for " << var
         << " ts="<<ts<<", nts=" << nts << endl;
*/
    char formatString[100];
    if(nts > 1)
        sprintf(formatString, "%s(%d,%d)", var.c_str(), nx * ny, nts);
    else
        sprintf(formatString, "%s(%d)", var.c_str(), nx * ny);

    int *timeData = data + (nx * ny * ts);
    PD_write(pdb.FilePointer(), formatString, "integer", (void*)timeData);
}

template <> void
FieldWithData1D<double>::WriteData(FieldWriter &pdb, const string &var, int ts, int nts)
{
/*    cerr << "Writing the double data for " << var
         << " ts="<<ts<<", nts=" << nts << endl;
*/
    char formatString[100];
    if(nts > 1)
        sprintf(formatString, "%s(%d,%d)", var.c_str(), nx * ny, nts);
    else
        sprintf(formatString, "%s(%d)", var.c_str(), nx * ny);

    double *timeData = data + (nx * ny * ts);
    PD_write(pdb.FilePointer(), formatString, "double", (void*)timeData);
}

template <> void
FieldWithData1D<float>::WriteData(FieldWriter &pdb, const string &var, int ts, int nts)
{
/*    cerr << "Writing the float data for " << var
         << " ts="<<ts<<", nts=" << nts << endl;
*/
    char formatString[100];
    if(nts > 1)
        sprintf(formatString, "%s(%d,%d)", var.c_str(), nx * ny, nts);
    else
        sprintf(formatString, "%s(%d)", var.c_str(), nx * ny);

    float *timeData = data + (nx * ny * ts);
    PD_write(pdb.FilePointer(), formatString, "float", (void*)timeData);
}

// ****************************************************************************
// Method: LinearArray<>::WriteData
//
// Purpose: 
//   Template specialization functions that write field data to the data file.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 24 11:51:35 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Dec 7 17:17:14 PST 2004
//   Added a float specialization.
//
//   Thomas R. Treadway, Thu Feb  8 14:35:45 PST 2007
//   Corrected template syntax, required for gcc-4.x support
//
// ****************************************************************************

template <> void
LinearArray<int>::WriteData(FieldWriter &pdb, const string &var, int ts, int nts)
{
    pdb.WriteIntArray(var, &data[0], data.size());
}

template <> void
LinearArray<double>::WriteData(FieldWriter &pdb, const string &var, int ts, int nts)
{
    pdb.WriteDoubleArray(var, &data[0], data.size());
}

template <> void
LinearArray<float>::WriteData(FieldWriter &pdb, const string &var, int ts, int nts)
{
    pdb.WriteFloatArray(var, &data[0], data.size());
}
