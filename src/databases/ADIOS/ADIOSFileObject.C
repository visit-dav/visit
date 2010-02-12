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
#include <vtkRectilinearGrid.h>

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

    //Read in variables.
    variables.clear();
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
            avi = adios_inq_var_byid (gps[gr], vr);
            if (avi == NULL)
            {
                sprintf(errmsg, "Error opening inquiring variable %s in group %s of bp file %s:\n%s", 
                        gps[gr]->var_namelist[vr], fp->group_namelist[gr], fileName.c_str(), adios_errmsg());
                EXCEPTION1(InvalidDBTypeException, errmsg);
            }

            // Skip scalars, or variables with > 3 real dimensions (besides time)
            if ( avi->ndim == 0 ||                        // scalar value in file
                 (avi->ndim == 1 && avi->timedim >= 0) ||  // scalar with time
                 (avi->ndim > 3 && avi->timedim == -1) ||  // >3D array with no time
                 (avi->ndim > 4 && avi->timedim >= 0))     // >3D array with time
            {
                debug5<<"Skipping variable"<<gps[gr]->var_namelist[vr]<<" of dimension: "<<avi->ndim<<endl;
                continue;
            }
            
            // Skip arrays whose type is not supported in VisIt            
            if (avi->type == adios_long_double ||
                avi->type == adios_complex || 
                avi->type == adios_double_complex) 
            {
                debug5<<"Skipping variable"<<gps[gr]->var_namelist[vr]<<" of unsupported type: "<<adios_type_to_string(avi->type)<<endl;
                continue;
            }

            // add variable to map, map id = variable path without the '/' in the beginning
            ADIOSVar v;
            if (gps[gr]->var_namelist[vr][0] == '/')
                v.name = gps[gr]->var_namelist[vr]+1;
            else
                v.name = gps[gr]->var_namelist[vr];

            v.type = avi->type;
            v.timedim = avi->timedim;
            v.groupIdx = gr;
            v.varid = avi->varid;
            
            // VisIt variable is max 3D and has no time dimension
            // ADIOS variable's dimensions include the time dimension 
            //  (which is the first dim in C/C++, i.e. avi->timedim == 0 or -1 if it has no time)
            if (avi->timedim == -1)
                v.dim = avi->ndim;
            else
                v.dim = avi->ndim - 1;
                    
            int i = 0; // avi's index
            int j = 0; // vi's index
            // 1. process dimensions before the time dimension
            // Note that this is empty loop with current ADIOS/C++ (timedim = 0 or -1)
            for (; i < std::min(avi->timedim,3); i++)
            {
                v.start[j] = 0;
                v.count[j] = 1;
                v.global[j] = 1;
                if (i<avi->ndim)
                    v.count[j] = v.global[j] = (int) avi->dims[i];
                j++;
            }
            // 2. skip time dimension if it has one
            if (avi->timedim >= 0)
                i++; 
            // 3. process dimensions after the time dimension
            for (; i < (avi->timedim == -1 ? 3 : 4); i++)
            {
                v.start[j] = 0;
                v.count[j] = 1;
                v.global[j] = 1;
                if (i<avi->ndim)
                    v.count[j] = v.global[j] = (int) avi->dims[i];
                j++;
            }

            v.SwapIndices();
            
            // add variable to map
            variables[v.name] = v;
            debug5<<"  added variable "<< v.name<<endl;

            // Note: avi has gmin and gmax / or value that we did not use here
            adios_free_varinfo(avi);
        }
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
// ****************************************************************************

void
ADIOSFileObject::Close()
{
    if (gps)
    {
        if (fp)
        {
            for (int gr=0; gr<fp->groups_count; gr++)
                adios_gclose(gps[gr]);
            
            adios_fclose(fp);
        }
        free(gps);
    }
    
    fp = NULL;
    gps = NULL;
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
// ****************************************************************************

bool
ADIOSFileObject::ReadVariable(const char *varname,
                              int ts,
                              vtkDataArray **array)
{
    debug5<<"ADIOSFileObject::ReadVariable("<<varname<<")"<<endl;

    std::map<std::string, ADIOSVar>::const_iterator v = variables.find(varname);
    if (v == variables.end())
        return false;
    
    int tupleSz = 0;
    
    switch (v->second.type)
    {        case adios_unsigned_byte:
             tupleSz = 1;
             *array = vtkCharArray::New(); 
             break;
        case adios_byte:
             tupleSz = 1;
             *array = vtkUnsignedCharArray::New(); 
             break;
        case adios_string:
             tupleSz = 1;
             *array = vtkCharArray::New(); 
             break;
               
        case adios_unsigned_short:
             tupleSz = 2;
             *array = vtkUnsignedShortArray::New(); 
             break;
        case adios_short:
             tupleSz = 2;
             *array = vtkShortArray::New(); 
             break;
 
        case adios_unsigned_integer:
             tupleSz = 4;
             *array = vtkUnsignedIntArray::New(); 
             break;
        case adios_integer:
             tupleSz = 4;
             *array = vtkIntArray::New(); 
             break;
    
        case adios_unsigned_long:
             tupleSz = 8;
             *array = vtkUnsignedLongArray::New(); 
             break;
        case adios_long:
             tupleSz = 8;
             *array = vtkLongArray::New(); 
             break;
    
        case adios_real:
             tupleSz = 4;
             *array = vtkFloatArray::New(); 
             break;

        case adios_double:
             tupleSz = 8;
             *array = vtkDoubleArray::New(); 
             break;
             
        case adios_long_double: // 16 bytes
        case adios_complex:     //  8 bytes
        case adios_double_complex: // 16 bytes
        default:
          return false;
          EXCEPTION1(InvalidVariableException, varname);
          break;
    }
    
    int ntuples = 1;
    uint64_t start[4] = {0,0,0,0}, count[4] = {0,0,0,0};
    int i=0;  // VisIt var dimension index
    int j=0;  // adios var dimension index
    // timedim=-1 for non-timed variables, 0..n for others
    // 1. up to time index, or max 3
    // This loop is empty with current ADIOS/C++ (timedim = -1 or 0)
    for (; i<std::min(v->second.timedim,3); i++)
    {
        ntuples *= (v->second.count[i]);
        start[j] = (uint64_t) v->second.start[i];
        count[j] = (uint64_t) v->second.count[i];
        j++;
    }
    // 2. handle time index if the variable has time
    if (v->second.timedim >= 0)
    {
        start[j] = ts;
        count[j] = 1;
        j++;
    }
    // 3. the rest of indices (all if no time dimension)
    for (; i<3; i++)
    {
        ntuples *= (v->second.count[i]);
        start[j] = (uint64_t) v->second.start[i];
        count[j] = (uint64_t) v->second.count[i];
        j++;
    }
    
    SwapIndices(v->second.dim, start);
    SwapIndices(v->second.dim, count);
    
    (*array)->SetNumberOfTuples(ntuples);
    void *data = (*array)->GetVoidPointer(0);
    
    debug5<<"s "<<start[0]<<" "<<start[1]<<" "<<start[2]<<" "<<start[3]<<endl;
    debug5<<"c "<<count[0]<<" "<<count[1]<<" "<<count[2]<<" "<<count[3]<<endl;
    debug5<<"adios_read_var ["<<start[0]<<"+"<<count[0]<<", "<<start[1]<<"+"<<count[1]<<", "<<start[2]<<"+"<<count[2]<<"] nt= "<<ntuples<<endl;
    
    if (adios_read_var_byid(gps[v->second.groupIdx], v->second.varid, start, count, data) < 0)
        return false;

    return true;
}
