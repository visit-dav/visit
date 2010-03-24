/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <visitstream.h>
#include <ADIOSFileObject.h>
#include <avtParallel.h>
#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>
#include <DebugStream.h>

#include <vtkCharArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkShortArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkIntArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkLongArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkPoints.h>

static bool
SupportedVariable(ADIOS_VARINFO *avi);

static void
ConvertToFloat(float *data, int &n, ADIOS_DATATYPES &t, const void *readData);

// ****************************************************************************
//  Method: ADIOSFileObject::ADIOSFileObject
//
//  Purpose:
//      ADIOSFileObject constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 15:55:14 EST 2010
//
// ****************************************************************************

ADIOSFileObject::ADIOSFileObject(const char *fname)
{
    fileName = fname;
    fp = NULL;
    gps = NULL;
}

// ****************************************************************************
//  Method: ADIOSFileObject::ADIOSFileObject
//
//  Purpose:
//      ADIOSFileObject constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 15:55:14 EST 2010
//
// ****************************************************************************

ADIOSFileObject::ADIOSFileObject(const std::string &fname)
{
    fileName = fname;
    fp = NULL;
    gps = NULL;
}


// ****************************************************************************
//  Method: ADIOSFileObject::ADIOSFileObject
//
//  Purpose:
//      ADIOSFileObject dtor.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 15:55:14 EST 2010
//
// ****************************************************************************

ADIOSFileObject::~ADIOSFileObject()
{
    Close();
}

// ****************************************************************************
//  Method: ADIOSFileObject::NumTimeSteps
//
//  Purpose:
//      Return number of timesteps.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 15:55:14 EST 2010
//
// ****************************************************************************

int
ADIOSFileObject::NumTimeSteps()
{
    Open();
    return fp->ntimesteps;
}

// ****************************************************************************
//  Method: ADIOSFileObject::GetCycles
//
//  Purpose:
//      Return cycles.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 15:55:14 EST 2010
//
// ****************************************************************************

void
ADIOSFileObject::GetCycles(std::vector<int> &cycles)
{
    Open();
    
    cycles.resize(0);
    for (int i = 0; i < fp->ntimesteps; i++)
        cycles.push_back(fp->tidx_start + i);
}


// ****************************************************************************
//  Method: ADIOSFileObject::Open
//
//  Purpose:
//      Open a file.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 15:55:14 EST 2010
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar  9 12:40:15 EST 2010
//   Major overhaul. Added scalars, attributes, and reorganized the class.   
//
// ****************************************************************************

bool
ADIOSFileObject::Open()
{
    if (IsOpen())
        return true;

#ifdef PARALLEL
    fp = adios_fopen(fileName.c_str(), (MPI_Comm)VISIT_MPI_COMM);
#else
    fp = adios_fopen(fileName.c_str(), 0);
#endif
    
    char errmsg[1024];
    if (fp == NULL)
    {
        sprintf(errmsg, "Error opening bp file %s:\n%s", fileName.c_str(), adios_errmsg());
        EXCEPTION1(InvalidDBTypeException, errmsg);
    }

    
    ADIOS_VARINFO *avi;
    gps = (ADIOS_GROUP **) malloc(fp->groups_count * sizeof(ADIOS_GROUP *));
    if (gps == NULL)
        EXCEPTION1(InvalidDBTypeException, "The file could not be opened. Not enough memory");
    
    debug5 << "ADIOS BP file: " << fileName << endl;
    debug5 << "# of groups: " << fp->groups_count << endl;
    debug5 << "# of variables: " << fp->vars_count << endl;
    debug5 << "# of attributes:" << fp->attrs_count << endl;
    debug5 << "time steps: " << fp->ntimesteps << " from " << fp->tidx_start << endl;

    //Read in variables/scalars.
    variables.clear();
    scalars.clear();
    for (int gr=0; gr<fp->groups_count; gr++)
    {
        debug5 <<  "  group " << fp->group_namelist[gr] << ":" << endl;
        gps[gr] = adios_gopen_byid(fp, gr);
        if (gps[gr] == NULL)
        {
            sprintf(errmsg, "Error opening group %s in bp file %s:\n%s", fp->group_namelist[gr], fileName.c_str(), adios_errmsg());
            EXCEPTION1(InvalidDBTypeException, errmsg);
        }
        
        for (int vr=0; vr<gps[gr]->vars_count; vr++)
        {
            avi = adios_inq_var_byid(gps[gr], vr);
            if (avi == NULL)
            {
                sprintf(errmsg, "Error opening inquiring variable %s in group %s of bp file %s:\n%s", 
                        gps[gr]->var_namelist[vr], fp->group_namelist[gr], fileName.c_str(), adios_errmsg());
                EXCEPTION1(InvalidDBTypeException, errmsg);
            }

            if (SupportedVariable(avi))
            {
                //Scalar
                if (avi->ndim == 0)
                {
                    ADIOSScalar s(gps[gr]->var_namelist[vr], avi);
                    scalars[s.Name()] = s;
                    debug5<<"  added scalar "<<s<<endl;
                }
                //Variable
                else
                {
                    // add variable to map, map id = variable path without the '/' in the beginning
                    ADIOSVar v(gps[gr]->var_namelist[vr], gr, avi);
                    variables[v.name] = v;
                    debug5<<"  added variable "<< v.name<<endl;
                }
            }
            else
                debug5<<"Skipping variable: "<<gps[gr]->var_namelist[vr]<<" dim= "<<avi->ndim
                      <<" type= "<<adios_type_to_string(avi->type)<<endl;
            
            adios_free_varinfo(avi);
        }
        //Read in attributes.
        for (int a = 0; a < gps[gr]->attrs_count; a++)
        {
            int sz;
            void *data = NULL;
            ADIOS_DATATYPES attrType;

            if (adios_get_attr_byid(gps[gr], a, &attrType, &sz, &data) != 0)
            {
                debug5<<"Failed to get attr: "<<gps[gr]->attr_namelist[a]<<endl;
                continue;
            }
            
            ADIOSAttr attr(gps[gr]->attr_namelist[a], attrType, data);
            attributes[attr.Name()] = attr;
            free(data);
        }

        adios_gclose(gps[gr]);
        gps[gr] = NULL;
    }

    return true;
}

// ****************************************************************************
//  Method: ADIOSFileObject::Close
//
//  Purpose:
//      Close a file.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 15:55:14 EST 2010
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar  9 12:40:15 EST 2010
//   Major overhaul. Added scalars, attributes, and reorganized the class.   
//
// ****************************************************************************

void
ADIOSFileObject::Close()
{
    if (fp && gps)
    {
        for (int gr=0; gr<fp->groups_count; gr++)
            if (gps[gr] != NULL)
                adios_gclose(gps[gr]);
    }
    
    if (gps)
        free(gps);
    if (fp)
        adios_fclose(fp);
    
    fp = NULL;
    gps = NULL;
}


// ****************************************************************************
//  Method: ADIOSFileObject::OpenGroup
//
//  Purpose:
//      Open a group.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

void
ADIOSFileObject::OpenGroup(int grpIdx)
{
    if (!gps)
        return;
    if (gps[grpIdx] == NULL)
        gps[grpIdx] = adios_gopen_byid(fp, grpIdx);

    if (gps[grpIdx] == NULL)
    {
        std::string errmsg = "Error opening group "+std::string(fp->group_namelist[grpIdx])+" in " + fileName;
        EXCEPTION1(InvalidDBTypeException, errmsg.c_str());
    }
}

// ****************************************************************************
//  Method: ADIOSFileObject::CloseGroup
//
//  Purpose:
//      Close a group.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

void
ADIOSFileObject::CloseGroup(int grpIdx)
{
    if (!gps)
        return;
    if (gps[grpIdx] != NULL)
    {
        int val = adios_gclose(gps[grpIdx]);
        gps[grpIdx] = NULL;
        if (val != 0)
        {
            std::string errmsg = "Error closing group "+std::string(fp->group_namelist[grpIdx])+" in " + fileName;
            EXCEPTION1(InvalidDBTypeException, errmsg.c_str());
        }
    }
}

// ****************************************************************************
//  Method: ADIOSFileObject::GetIntScalar
//
//  Purpose:
//      Return an integer scalar
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

bool
ADIOSFileObject::GetIntScalar(const std::string &nm, int &val)
{
    Open();
    std::map<std::string, ADIOSScalar>::const_iterator s = scalars.find(nm);
    if (s == scalars.end() || !s->second.IsInt())
        return false;
    
    val = s->second.AsInt();
    return true;
}

// ****************************************************************************
//  Method: ADIOSFileObject::GetDoubleScalar
//
//  Purpose:
//      Return a double scalar
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

bool
ADIOSFileObject::GetDoubleScalar(const std::string &nm, double &val)
{
    Open();
    std::map<std::string, ADIOSScalar>::const_iterator s = scalars.find(nm);
    if (s == scalars.end() || !s->second.IsDouble())
        return false;
    
    val = s->second.AsDouble();
    return true;
}

// ****************************************************************************
//  Method: ADIOSFileObject::GetStringScalar
//
//  Purpose:
//      Return a string scalar
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 24 16:32:09 EDT 2010
//
// ****************************************************************************

bool
ADIOSFileObject::GetStringScalar(const std::string &nm, std::string &val)
{
    Open();
    std::map<std::string, ADIOSScalar>::const_iterator s = scalars.find(nm);
    if (s == scalars.end() || !s->second.IsString())
        return false;
    
    val = s->second.AsString();
    return true;
}

// ****************************************************************************
//  Method: ADIOSFileObject::GetIntAttr
//
//  Purpose:
//      Return integer attribute
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

bool
ADIOSFileObject::GetIntAttr(const std::string &nm, int &val)
{
    Open();
    std::map<std::string, ADIOSAttr>::const_iterator a = attributes.find(nm);
    if (a == attributes.end() || !a->second.IsInt())
        return false;

    val = a->second.AsInt();
    return true;
}

// ****************************************************************************
//  Method: ADIOSFileObject::GetStringAttr
//
//  Purpose:
//      Return string attribute
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

bool
ADIOSFileObject::GetStringAttr(const std::string &nm, std::string &val)
{
    Open();
    std::map<std::string, ADIOSAttr>::const_iterator a = attributes.find(nm);
    if (a == attributes.end() || !a->second.IsString())
        return false;

    val = a->second.AsString();
    return true;
}

// ****************************************************************************
//  Method: ADIOSFileObject::ReadCoordinates
//
//  Purpose:
//      Read vtkPoints.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

bool
ADIOSFileObject::ReadCoordinates(const std::string &nm,
                                 int ts,
                                 vtkPoints **pts)
{
    Open();
    
    varIter vi = variables.find(nm);
    if (vi == variables.end())
    {
        debug5<<"Variable "<<nm<<" not found."<<endl;
        return false;
    }
    ADIOSVar v = vi->second;
    
    *pts = ADIOSFileObject::AllocatePoints(v.type);

    int ntuples = 1;
    uint64_t start[4] = {0,0,0,0}, count[4] = {0,0,0,0};
    
    v.GetReadArrays(ts, start, count, &ntuples);
    ntuples /= v.dim;
    (*pts)->SetNumberOfPoints(ntuples);

    void *data = (*pts)->GetVoidPointer(0), *data2 = NULL;
    void *readData = data;
    
    if (v.dim < 3)
    {
        if (v.type == adios_real)
            data2 = malloc(ntuples*3*sizeof(float));
        else if (v.type == adios_double)
            data2 = malloc(ntuples*3*sizeof(double));
        else if (v.type == adios_integer)
            data2 = malloc(ntuples*3*sizeof(int));
        readData = data2;
    }
    
    OpenGroup(v.groupIdx);
    
    debug5<<"adios_read_var:"<<endl<<v<<endl;
    uint64_t retval = adios_read_var_byid(gps[v.groupIdx], v.varid, start, count, readData);
    if (retval > 0)
    {
        if (readData != data)
        {
            int i, j, n = ntuples*3;
            for (i=0, j=0; i < n; i += 3, j += v.dim)
            {
                int k;
                for (k = 0; k < v.dim; k++)
                    ((double *)data)[i+k] = ((double *)data2)[j+k];
                for ( ; k < 3; k++)
                    ((double *)data)[i+k] = 0.0;
            }
            free(data2);
        }
    }

    CloseGroup(v.groupIdx);    
    return (retval > 0);
}

// ****************************************************************************
//  Method: ADIOSFileObject::ReadVariable
//
//  Purpose:
//      Read variable.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Feb 10 15:55:14 EST 2010
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar  9 12:40:15 EST 2010
//   Major overhaul. Added scalars, attributes, and reorganized the class.   
//
// ****************************************************************************

bool
ADIOSFileObject::ReadVariable(const std::string &nm,
                              int ts,
                              vtkDataArray **array)
{
    debug5<<"ADIOSFileObject::ReadVariable("<<nm<<")"<<endl;
    Open();

    varIter vi = variables.find(nm);
    if (vi == variables.end())
    {
        debug5<<"Variable "<<nm<<" not found."<<endl;
        return false;
    }
    ADIOSVar v = vi->second;

    int tupleSz = adios_type_size(v.type, NULL);
    *array = ADIOSFileObject::AllocateArray(v.type);

    int ntuples = 1;
    uint64_t start[4] = {0,0,0,0}, count[4] = {0,0,0,0};
    v.GetReadArrays(ts, start, count, &ntuples);
    
    (*array)->SetNumberOfTuples(ntuples);
    void *data = (*array)->GetVoidPointer(0);

    debug5<<"ARR: adios_read_var:"<<endl<<v<<endl;
    OpenGroup(v.groupIdx);

    uint64_t retval = adios_read_var_byid(gps[v.groupIdx], v.varid, start, count, data);
        
    CloseGroup(v.groupIdx);

    return (retval > 0);
}


// ****************************************************************************
//  Method: ADIOSFileObject::ReadVariable
//
//  Purpose:
//      Read variable.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
//
// ****************************************************************************

bool
ADIOSFileObject::ReadVariable(const std::string &nm,
                              int ts,
                              vtkFloatArray **array)
{
    debug5<<"ADIOSFileObject::ReadVariable("<<nm<<")"<<endl;
    Open();

    varIter vi = variables.find(nm);
    if (vi == variables.end())
    {
        debug5<<"Variable "<<nm<<" not found."<<endl;
        return false;
    }
    ADIOSVar v = vi->second;

    int tupleSz = adios_type_size(v.type, NULL);
    *array = vtkFloatArray::New();

    int ntuples = 1;
    uint64_t start[4] = {0,0,0,0}, count[4] = {0,0,0,0};
    v.GetReadArrays(ts, start, count, &ntuples);
    
    (*array)->SetNumberOfTuples(ntuples);
    float *data = (float *)(*array)->GetVoidPointer(0);
    void *readData = (void *)data;

    bool convertData = (v.type != adios_real);
    if (convertData)
        readData = malloc(ntuples*tupleSz);

    debug5<<"ARR: adios_read_var:"<<endl<<v<<endl;
    OpenGroup(v.groupIdx);

    uint64_t retval = adios_read_var_byid(gps[v.groupIdx], v.varid, start, count, readData);
        
    CloseGroup(v.groupIdx);

    if (retval > 0 && convertData)
    {
        ConvertToFloat(data, ntuples, v.type, readData);
        free(readData);
    }

    return (retval > 0);
}

// ****************************************************************************
//  Method: ADIOSFileObject::AllocatePoints
//
//  Purpose:
//      Create vtkPoints.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

vtkPoints *
ADIOSFileObject::AllocatePoints(ADIOS_DATATYPES &t)
{
    vtkPoints *pts = NULL;
    switch (t)
    {
      case adios_real:
        pts = vtkPoints::New(VTK_FLOAT);
        break;
      case adios_double:
        pts = vtkPoints::New(VTK_DOUBLE);
        break;
        
      default:
        std::string str = "Inavlid point type";
        EXCEPTION1(InvalidVariableException, str);
        break;
    }
    return pts;
}

// ****************************************************************************
//  Method: ADIOSFileObject::AllocateArray
//
//  Purpose:
//      Create vtkDataArray.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

vtkDataArray *
ADIOSFileObject::AllocateArray(ADIOS_DATATYPES &t)
{
    vtkDataArray *array = NULL;
    switch (t)
    {
      case adios_unsigned_byte:
        array = vtkCharArray::New();
        break;
      case adios_byte:
        array = vtkUnsignedCharArray::New();
        break;
      case adios_string:
        array = vtkCharArray::New();
        break;
        
      case adios_unsigned_short:
        array = vtkUnsignedShortArray::New();
        break;
      case adios_short:
        array = vtkShortArray::New();
        break;
        
      case adios_unsigned_integer:
        array = vtkUnsignedIntArray::New(); 
        break;
      case adios_integer:
        array = vtkIntArray::New(); 
        break;
        
      case adios_unsigned_long:
        array = vtkUnsignedLongArray::New(); 
        break;
      case adios_long:
        array = vtkLongArray::New(); 
        break;
        
      case adios_real:
        array = vtkFloatArray::New(); 
        break;
        
      case adios_double:
        array = vtkDoubleArray::New(); 
        break;
        
      case adios_long_double: // 16 bytes
      case adios_complex:     //  8 bytes
      case adios_double_complex: // 16 bytes
      default:
        std::string str = "Inavlid variable type";
        EXCEPTION1(InvalidVariableException, str);
        break;
    }

    return array;
}


// ****************************************************************************
//  Function: SupportedVariable
//
//  Purpose:
//      Determine if this variable is supported.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

static bool
SupportedVariable(ADIOS_VARINFO *avi)
{
    if ((avi->ndim == 1 && avi->timedim >= 0) ||  // scalar with time
        (avi->ndim > 3 && avi->timedim == -1) ||  // >3D array with no time
        (avi->ndim > 4 && avi->timedim >= 0)  ||  // >3D array with time
        avi->type == adios_long_double ||
        avi->type == adios_complex || 
        avi->type == adios_double_complex)
    {
        return false;
    }
    
    return true;
}

// ****************************************************************************
//  Method: ADIOSVar::ADIOSVar
//
//  Purpose:
//      Constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

ADIOSVar::ADIOSVar()
{
    start[0] = start[1] = start[2] = 0;
    count[0] = count[1] = count[2] = 0;
    global[0] = global[1] = global[2] = 0;
    dim = 0;
    type=adios_unknown; groupIdx=-1, varid=-1, timedim=-1;
    extents[0] = extents[1] = 0.0;
}

// ****************************************************************************
//  Method: ADIOSVar::ADIOSVar
//
//  Purpose:
//      Constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

ADIOSVar::ADIOSVar(const std::string &nm, int grpIdx, ADIOS_VARINFO *avi)
{
    name = nm;
    type = avi->type;
    double valMin = 0.0, valMax = 0.0;

    if (avi->gmin && avi->gmax)
    {
        if (type == adios_integer)
        {
            valMin = (double)(*((int*)avi->gmin));
            valMax = (double)(*((int*)avi->gmax));
        }
        else if (type == adios_real)
        {
            valMin = (double)(*((float*)avi->gmin));
            valMax = (double)(*((float*)avi->gmax));
        }
        else if (type == adios_double)
        {
            valMin = (double)(*((double*)avi->gmin));
            valMax = (double)(*((double*)avi->gmax));
        }
    }

    extents[0] = valMin;
    extents[1] = valMax;
    timedim = avi->timedim;
    groupIdx = grpIdx;
    varid = avi->varid;
    if (avi->timedim == -1)
        dim = avi->ndim;
    else
        dim = avi->ndim - 1;
    int i = 0; // avi's index
    int j = 0; // vi's index
    // 1. process dimensions before the time dimension
    // Note that this is empty loop with current ADIOS/C++ (timedim = 0 or -1)
    for (; i < std::min(avi->timedim,3); i++)
    {
        start[j] = 0;
        count[j] = 1;
        global[j] = 1;
        if (i<avi->ndim)
            count[j] = global[j] = avi->dims[i];
        j++;
    }
    // 2. skip time dimension if it has one
    if (avi->timedim >= 0)
        i++; 
    // 3. process dimensions after the time dimension
    for (; i < (avi->timedim == -1 ? 3 : 4); i++)
    {
        start[j] = 0;
        count[j] = 1;
        global[j] = 1;
        if (i<avi->ndim)
            count[j] = global[j] = avi->dims[i];
        j++;
    }
    
    SwapIndices();
}

// ****************************************************************************
//  Method: ADIOSVar::SwapIndices
//
//  Purpose:
//      Swap indices.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

void ADIOSVar::SwapIndices()
{
    ::SwapIndices(dim, start);
    ::SwapIndices(dim, count);
    ::SwapIndices(dim, global);
}

// ****************************************************************************
//  Method: ADIOSVar::GetReadArrays
//
//  Purpose:
//      Fill in start/count arrays for adios API.
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

void
ADIOSVar::GetReadArrays(int ts, uint64_t *s, uint64_t *c, int *ntuples)
{
    *ntuples = 1;
    
    int i=0;  // VisIt var dimension index
    int j=0;  // adios var dimension index
    // timedim=-1 for non-timed variables, 0..n for others
    // 1. up to time index, or max 3
    // This loop is empty with current ADIOS/C++ (timedim = -1 or 0)
    for (; i<std::min(timedim,3); i++)
    {
        *ntuples *= (int)count[i];
        s[j] = start[i];
        c[j] = count[i];
        j++;
    }
    // 2. handle time index if the variable has time
    if (timedim >= 0)
    {
        s[j] = ts;
        c[j] = 1;
        j++;
    }
    // 3. the rest of indices (all if no time dimension)
    for (; i<3; i++)
    {
        *ntuples *= (int)count[i];
        s[j] = start[i];
        c[j] = count[i];
        j++;
    }
    
    ::SwapIndices(dim, s);
    ::SwapIndices(dim, c);
}


// ****************************************************************************
//  Method: ConvertToFloat
//
//  Purpose:
//      Convert array to floats.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
//
// ****************************************************************************

template<class T> static void
CopyArray( T readData, float *data, int n )
{
    T p1 = (T)readData;
    for (int i = 0; i < n; i++)
        data[i] = (float)p1[i];
}

static void
ConvertToFloat(float *data, int &n, ADIOS_DATATYPES &t, const void *readData)
{
    switch(t)
    {
      case adios_unsigned_byte:
        CopyArray((const unsigned char *)readData, data, n);
        break;
      case adios_byte:
        CopyArray((const char *)readData, data, n);
        break;
      case adios_unsigned_short:
        CopyArray((const unsigned short *)readData, data, n);
        break;
      case adios_short:
        CopyArray((const short *)readData, data, n);
        break;
      case adios_unsigned_integer:
        CopyArray((const unsigned int *)readData, data, n);
        break;
      case adios_integer:
        CopyArray((const int *)readData, data, n);
        break;
      case adios_unsigned_long:
        CopyArray((const unsigned long *)readData, data, n);
        break;
      case adios_long:
        CopyArray((const long *)readData, data, n);
        break;
      case adios_double:
        CopyArray((const double *)readData, data, n);
        break;
      default:
        std::string str = "Inavlid variable type";
        EXCEPTION1(InvalidVariableException, str);
        break;        
    }
}
