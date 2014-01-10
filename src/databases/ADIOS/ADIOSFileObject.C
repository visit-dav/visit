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
template<class T> static void
ConvertTo(T *data, int &n, ADIOS_DATATYPES &t, const void *readData);

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
    resetDimensionOrder = false;
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
    return numTimeSteps;
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
    for (int i = 0; i < numTimeSteps; i++)
        cycles.push_back(i);
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
    fp = adios_read_open_file(fileName.c_str(), ADIOS_READ_METHOD_BP, (MPI_Comm)VISIT_MPI_COMM);
#else
    MPI_Comm comm_dummy = 0;
    fp = adios_read_open_file(fileName.c_str(), ADIOS_READ_METHOD_BP, comm_dummy);
#endif

    if (resetDimensionOrder)
        ResetDimensionOrder();

    if (fp == NULL)
    {
        EXCEPTION1(InvalidDBTypeException, fileName.c_str());
    }
    numTimeSteps = -1;

    //Read vars.
    for (int i = 0; i < fp->nvars; i++)
    {
          ADIOS_VARINFO *avi = adios_inq_var_byid(fp, i);
          adios_inq_var_stat(fp, avi, 0, 0);

          if (!SupportedVariable(avi))
                {
                  debug5<<"Skipping variable: "<<fp->var_namelist[i]<<" dim= "<<avi->ndim<<endl;
                  cout<<"**Skipping variable: "<<fp->var_namelist[i]<<" dim= "<<avi->ndim<<endl;
                  continue;
                }
          
          if (avi->ndim == 0)
                { 
                  ADIOSScalar s(fp->var_namelist[i], avi);
                  scalars[s.Name()] = s;
                  debug5<<"  added scalar "<<s<<endl;
                  cout<<"  added scalar "<<s<<endl;
                }
          else
                {
                  ADIOSVar v(fp->var_namelist[i], avi);
                  variables[v.name] = v;
                  debug5<<"  added variable "<< v.name<<endl;
                  cout<<"  added variable "<<v<<" global= "<<avi->global<<endl;
                  cout<<"   ***** nblocks[0] "<<avi->nblocks[0]<<" "<<avi->sum_nblocks<<endl;
                  adios_inq_var_blockinfo(fp, avi);
                  /*
                  for (int kk = 0; kk < avi->nblocks[0]; kk++)
                  {
                      cout<<kk<<":";
                      cout<<"(";
                      for (int d = 0; d < avi->ndim; d++)
                          cout<<avi->blockinfo[kk].start[d]<<" ";
                      cout<<") (";
                      for (int d = 0; d < avi->ndim; d++)
                          cout<<avi->blockinfo[kk].count[d]<<" ";
                      cout<<")"<<endl;
                  }
                  */
                  if (numTimeSteps < 0)
                        numTimeSteps = avi->nsteps;
                  else if (numTimeSteps != avi->nsteps)
                        {
                          EXCEPTION1(InvalidDBTypeException,
                                                 "File contains variables with differening number of time steps.");
                        }
                }
          adios_free_varinfo(avi);
    }

    //Read attributes.
    for (int i = 0; i < fp->nattrs; i++)
    {
          ADIOS_DATATYPES attrType;
          int  asize;
          void *data;
          adios_get_attr_byid(fp, i, &attrType, &asize, &data);

          ADIOSAttr attr(fp->attr_namelist[i], attrType, data);
          attributes[attr.Name()] = attr;
          free(data);
          cout<<"  added attribute "<<attr<<endl;
    }
    return true;


#if 0
    
    int err;
    ADIOS_READ_METHOD read_method = ADIOS_READ_METHOD_BP;
    int timeoutSec = 0;
    
#ifdef PARALLEL
    err = adios_read_init_method(read_method, (MPI_Comm)VISIT_MPI_COMM, "");
    fp = adios_read_open_stream(fileName.c_str(), read_method, (MPI_Comm)VISIT_MPI_COMM, 
                                ADIOS_LOCKMODE_ALL, timeoutSec);

#else
    err = adios_read_init_method(read_method, 0, "");
    fp = adios_read_open_file(fileName.c_str(), read_method, 0);
#endif
    
    char errmsg[1024];
    if (fp == NULL || adios_errno == err_file_not_found || adios_errno == err_end_of_stream)
    {
        sprintf(errmsg, "Error opening bp file %s:\n%s", fileName.c_str(), adios_errmsg());
        EXCEPTION1(InvalidDBTypeException, errmsg);
    }


    char **groupNames;
    int64_t gh;
    VarInfo *varinfo;
    ADIOS_VARINFO *v;

    while (adios_errno != err_end_of_stream)
    {
        adios_get_grouplist(fp, &groupNames);
        adios_declare_group(&gh, groupNames[0], "", adios_flag_yes);
        
        varinfo = (VarInfo *) malloc (sizeof(VarInfo) * fp->nvars);

        for (int i=0; i<fp->nvars; i++) 
        {
            //cout <<"Get info on variable "<<i<<" "<<fp->var_namelist[i]<<endl;
            varinfo[i].v = adios_inq_var_byid(fp, i);
            if (varinfo[i].v == NULL)
                THROW(eavlException, "ADIOS Importer: variable inquiry failed.");

            if (!SupportedVariable(varinfo[i].v))
                continue;
            
            // add variable to map, map id = variable path without the '/' in the beginning
            ADIOSVar v(fp->var_namelist[i], varinfo[i].v);
            variables[v.name] = v;
        }
        break;
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
                    cout<<"  added scalar "<<s<<endl;
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
                      <<" timedim= "<<avi->timedim
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
#endif

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
    if (fp)
        adios_read_close(fp);
    fp = NULL;
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
//  Method: ADIOSFileObject::GetDoubleAttr
//
//  Purpose:
//      Return integer attribute
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

bool
ADIOSFileObject::GetDoubleAttr(const std::string &nm, double &val)
{
    Open();
    std::map<std::string, ADIOSAttr>::const_iterator a = attributes.find(nm);
    if (a == attributes.end() || !a->second.IsDouble())
        return false;

    val = a->second.AsDouble();
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

#if 1
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
    debug5<<"ADIOSFileObject::ReadCoordinates"<<endl;
    Open();
    
    varIter vi = variables.find(nm);
    if (vi == variables.end())
    {
        debug5<<"Variable "<<nm<<" not found."<<endl;
        return false;
    }
    ADIOSVar v = vi->second;
    
    *pts = ADIOSFileObject::AllocatePoints(v.type);

    uint64_t start[4] = {0,0,0,0}, count[4] = {0,0,0,0};
    
    int sz;
    v.GetReadArrays(ts, start, count, &sz);
    int ntuples = sz/v.dim;
    (*pts)->SetNumberOfPoints(ntuples);
    cout<<"pts: "<<ntuples<<" "<<v.dim<<endl;


    //allocate read memory.
    int nBytes = sz;
    if (v.type == adios_real)
        nBytes *= sizeof(float);
    else if (v.type == adios_double)
        nBytes *= sizeof(double);
    else if (v.type == adios_integer)
        nBytes *= sizeof(int);
    
    void *buff = new unsigned char[nBytes];
    
    ADIOS_SELECTION *sel;
    sel = adios_selection_boundingbox(v.dim, start, count);
    adios_schedule_read_byid(fp, sel, v.varIdx, ts, 1, buff);
    int retval = adios_perform_reads(fp, 1);


    float pt[3];
    double *ptr = (double *)buff;
    for (int i = 0; i < ntuples; i++)
    {
        pt[0] = ptr[i*v.dim + 0];
        pt[1] = ptr[i*v.dim + 1];
        if (v.dim == 3)
            pt[2] = ptr[i*v.dim + 2];
        else
            pt[2] = 0.0;

        (*pts)->SetPoint(i, pt);
    }

    return true;

#if 0


















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
    
    debug5<<"adios_read_var:"<<endl<<v<<endl;
    cout<<"adios_read_var:"<<endl<<v<<endl;
    ADIOS_SELECTION *sel;
    sel = adios_selection_boundingbox(v.dim, start, count);
    adios_schedule_read_byid(fp, sel, v.varIdx, ts, 1, data);
    int retval = adios_perform_reads(fp, 1);
    cout<<"retval= "<<retval<<" data= "<<data<<endl;
    if (retval == 0)
    {
        cout<<__LINE__<<endl;
        double *p = (double *)data;
        for (int i = 0; i < 10; i++)
            cout<<i<<" "<<p[i]<<endl;
        cout<<__LINE__<<endl;
        if (readData != data)
        {
            cout<<__LINE__<<endl;
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

    return (retval > 0);
#endif
}
#endif

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
                              int dom,
                              vtkDataArray **array)
{
    debug5<<"ADIOSFileObject::ReadVariable("<<nm<<" time= "<<ts<<")"<<endl;
    //cout<<"ADIOSFileObject::ReadVariable("<<nm<<" time= "<<ts<<")"<<endl;
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
    ADIOS_SELECTION *sel;
    //cout<<nm<<" "<<v.dim<<" ("<<start[0]<<" "<<start[1]<<" "<<start[2]<<" "<<start[3]<<") ("<<count[0]<<" "<<count[1]<<" "<<count[2]<<" "<<count[3]<<")"<<endl;
    sel = adios_selection_boundingbox(v.dim, start, count);
    adios_schedule_read_byid(fp, sel, v.varIdx, ts, 1, data);
    int retval = adios_perform_reads(fp, 1);

    /*
    sel = adios_selection_writeblock(0);
    cout<<"Write Block "<<sel->type<<" "<<sel->u.block.index<<" ["<<sel->u.bb.start[0]<<" "<<sel->u.bb.start[1]<<"]"<<endl;
    */

    return (retval == 0);
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
                              int dom,
                              vtkFloatArray **array)
{
    vtkDataArray *arr = NULL;
    if (!ReadVariable(nm, ts, dom, &arr))
        return false;
    
    *array = vtkFloatArray::SafeDownCast(arr);
    arr->Delete();
    return true;
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
    if (avi->ndim > 3 ||
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
    type=adios_unknown;
    varIdx=-1;
    nTimeSteps = -1;
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

ADIOSVar::ADIOSVar(const std::string &nm, ADIOS_VARINFO *avi)
{
    if (nm[0] == '/')
        name = &nm[1];
    else
        name = nm;
    
    type = avi->type;
    double valMin = 0.0, valMax = 0.0;

    if (avi->statistics && avi->statistics->min && avi->statistics->max)
    {
        if (type == adios_integer)
        {
            valMin = (double)(*((int*)avi->statistics->min));
            valMax = (double)(*((int*)avi->statistics->max));
        }
        else if (type == adios_real)
        {
            valMin = (double)(*((float*)avi->statistics->min));
            valMax = (double)(*((float*)avi->statistics->max));
        }
        else if (type == adios_double)
        {
            valMin = (double)(*((double*)avi->statistics->min));
            valMax = (double)(*((double*)avi->statistics->max));
        }
    }

    extents[0] = valMin;
    extents[1] = valMax;
    varIdx = avi->varid;
    nTimeSteps = avi->nsteps;
    dim = avi->ndim;

    for (int i = 0; i < 3; i++)
    {
        start[i] = 0;
        count[i] = 1;
        global[i] = 1;
    }
    for (int i = 0; i < dim; i++)
    {
        count[i] = avi->dims[i];
        global[i] = count[i];
    }

    /*    
    //ADIOS is ZYX.
    if (dim == 3)
    {
        count[0] = avi->dims[idx+2];
        count[1] = avi->dims[idx+1];
        count[2] = avi->dims[idx+0];
    }
    else if (dim == 2)
    {
        count[0] = avi->dims[idx+1];
        count[1] = avi->dims[idx+0];
    }
    else if (dim == 1)
    {
        count[0] = avi->dims[0];
    }
    for (int i = 0; i < 3; i++)
        global[i] = count[i];
    */
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
    s[0] = s[1] = s[2] = s[3] = 0;
    c[0] = c[1] = c[2] = c[3] = 0;

    if (nTimeSteps > 1)
    {
        s[0] = ts;
        s[1] = start[0];
        s[2] = start[1];
        s[3] = start[2];
        
        c[0] = 1;
        c[1] = count[0];
        c[2] = 0;//count[1];
        c[3] = 0;//count[2];

        for (int i = 0; i < dim; i++)
            *ntuples *= count[i];
        return;
    }

    for (int i = 0; i < dim; i++)
    {
        s[i] = start[i];
        c[i] = count[i];
        *ntuples *= count[i];
    }
    
    /*
    int idx = 0;
    if (timedim >= 0)
    {
        s[idx] = ts;
        c[idx] = 1;
        idx++;
    }

    if (dim == 1)
    {
        s[idx] = start[0];
        c[idx] = count[0];
        idx++;
        *ntuples *= (int)count[0];
    }
    //ADIOS is ZYX.
    else if (dim == 2)
    {
        s[idx] = start[1];
        c[idx] = count[1];
        idx++;
        s[idx] = start[0];
        c[idx] = count[0];
        *ntuples *= (int)count[0];
        *ntuples *= (int)count[1];
    }
    else if (dim == 3)
    {
        s[idx] = start[2];
        c[idx] = count[2];
        idx++;
        s[idx] = start[1];
        c[idx] = count[1];
        idx++;
        s[idx] = start[0];
        c[idx] = count[0];
        *ntuples *= (int)count[0];
        *ntuples *= (int)count[1];
        *ntuples *= (int)count[2];
    }
    */
}


// ****************************************************************************
//  Method: ConvertTo
//
//  Purpose:
//      Convert arrays to different types.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
//
// ****************************************************************************

template<class T0, class T1> static void
CopyArray( T0 *inData, T1 *outData, int n)
{
    for (int i = 0; i < n; i++)
        outData[i] = (T1)inData[i];
}
template<class T> static void
ConvertTo(T *data, int &n, ADIOS_DATATYPES &t, const void *readData)
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
