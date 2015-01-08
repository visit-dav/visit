/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
#include <adios_selection.h>
#include <avtParallel.h>
#include <avtDatabase.h>
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
#include <algorithm>

using namespace std;

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
    resetDimensionOrder = false;
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
        
        if (ADIOSFileObject::SupportedVariable(avi))
        {
            if (avi->ndim == 0)
                scalars[fp->var_namelist[i]] = avi;
            else
            {
                adios_inq_var_stat(fp, avi, 0, 0);
                adios_inq_var_blockinfo(fp, avi);
                adios_inq_var_meshinfo(fp, avi);
                variables[fp->var_namelist[i]] = avi;

                //Get timestep, and check for consistency.
                if (numTimeSteps < 0)
                    numTimeSteps = avi->nsteps;
                else if (numTimeSteps != avi->nsteps)
                {
                    EXCEPTION1(InvalidDBTypeException,
                               "File contains variables with differening number of time steps.");
                }

                /*
                int N = avi->sum_nblocks;
                cout<<"var: "<<fp->var_namelist[i]<<" nBlocks= "<<N<<endl;
                if (N > 10)
                    N = 10;
                for (int i = 0; i < N; i++)
                {
                    cout<<"  "<<i<<": ";
                    cout<<"s[";
                    for (int j = 0; j < avi->ndim; j++)
                        cout<<avi->blockinfo[i].start[j]<<" ";
                    cout<<"] c[";
                    for (int j = 0; j < avi->ndim; j++)
                        cout<<avi->blockinfo[i].count[j]<<" ";
                    cout<<"]"<<endl;
                }
                */
            }
        }
    }

    //Read attributes.
    for (int i = 0; i < fp->nattrs; i++)
        attributes[fp->attr_namelist[i]] = i;
    
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
    map<string, ADIOS_VARINFO*>::iterator it;
    for (it = variables.begin(); it != variables.end(); it++)
        adios_free_varinfo(it->second);
    for (it = scalars.begin(); it != scalars.end(); it++)
        adios_free_varinfo(it->second);
    variables.clear();
    scalars.clear();
    attributes.clear();
    
    if (fp)
        adios_read_close(fp);
    fp = NULL;
}

// ****************************************************************************
//  Method: ADIOSFileObject::GetScalar
//
//  Purpose:
//      Return an integer scalar
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

bool
ADIOSFileObject::GetScalar(const std::string &nm, int &val)
{
    Open();
    std::map<std::string, ADIOS_VARINFO*>::const_iterator s = scalars.find(nm);
    if (s == scalars.end())
        return false;
    
    ADIOS_VARINFO *avi = s->second;
    if (avi->type != adios_integer)
        return false;
    
    val = *((int*)avi->value);
    return true;
}

// ****************************************************************************
//  Method: ADIOSFileObject::GetScalar
//
//  Purpose:
//      Return a double scalar
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

bool
ADIOSFileObject::GetScalar(const std::string &nm, double &val)
{
    Open();
    std::map<std::string, ADIOS_VARINFO*>::const_iterator s = scalars.find(nm);
    if (s == scalars.end())
        return false;
    
    ADIOS_VARINFO *avi = s->second;
    if (avi->type != adios_double)
        return false;
    
    val = *((double*)avi->value);
    return true;
}

// ****************************************************************************
//  Method: ADIOSFileObject::GetScalar
//
//  Purpose:
//      Return a string scalar
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 24 16:32:09 EDT 2010
//
// ****************************************************************************

bool
ADIOSFileObject::GetScalar(const std::string &nm, std::string &val)
{
    Open();
    std::map<std::string, ADIOS_VARINFO*>::const_iterator s = scalars.find(nm);
    if (s == scalars.end())
        return false;
    
    ADIOS_VARINFO *avi = s->second;
    if (avi->type != adios_string)
        return false;
    
    val = ((char*)avi->value);
    return true;
}

// ****************************************************************************
//  Method: ADIOSFileObject::GetAttr
//
//  Purpose:
//      Return integer attribute
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

bool
ADIOSFileObject::GetAttr(const std::string &nm, int &val)
{
    Open();
    map<string, int>::const_iterator it = attributes.find(nm);
    if (it == attributes.end())
        return false;
    
    ADIOS_DATATYPES attrType;
    int  asize;
    void *data;
    adios_get_attr_byid(fp, it->second, &attrType, &asize, &data);
    bool valid = (attrType == adios_integer);

    if (valid)
        val = *((int*)data);
    free(data);
    return valid;
}

// ****************************************************************************
//  Method: ADIOSFileObject::GetAttr
//
//  Purpose:
//      Return integer attribute
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

bool
ADIOSFileObject::GetAttr(const std::string &nm, double &val)
{
    Open();
    map<string, int>::const_iterator it = attributes.find(nm);
    if (it == attributes.end())
        return false;
    
    ADIOS_DATATYPES attrType;
    int  asize;
    void *data;
    adios_get_attr_byid(fp, it->second, &attrType, &asize, &data);
    bool valid = (attrType == adios_double);

    if (valid)
        val = *((double*)data);
    free(data);
    
    return valid;
}

// ****************************************************************************
//  Method: ADIOSFileObject::GetAttr
//
//  Purpose:
//      Return string attribute
//
//  Programmer: Dave Pugmire
//  Creation:   Tue Mar  9 12:40:15 EST 2010
//
// ****************************************************************************

bool
ADIOSFileObject::GetAttr(const std::string &nm, std::string &val)
{
    Open();
    map<string, int>::const_iterator it = attributes.find(nm);
    if (it == attributes.end())
        return false;
    
    ADIOS_DATATYPES attrType;
    int  asize;
    void *data;
    adios_get_attr_byid(fp, it->second, &attrType, &asize, &data);
    bool valid = (attrType == adios_string);

    if (valid)
        val = ((char*)data);
    free(data);
    
    return valid;
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

bool
ADIOSFileObject::SupportedVariable(ADIOS_VARINFO *avi)
{
    if (avi->ndim > 3 ||
        avi->type == adios_long_double)
    {
        return false;
    }
    
    return true;
}

//****************************************************************************
// Method:  ADIOSFileObject::ReadScalarData
//
// Purpose:
//   
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************


bool
ADIOSFileObject::ReadScalarData(const std::string &nm, int ts, int block, vtkDataArray **arr)
{
    Open();
    map<string, ADIOS_VARINFO*>::const_iterator it = variables.find(nm);
    if (it == variables.end())
        return false;
    
    ADIOS_SELECTION *s = CreateSelection(it->second, block);
    ReadScalarData(nm, ts, s, arr);
    delete [] s->u.bb.start;
    delete [] s->u.bb.count;
    adios_selection_delete(s);
    return true;
}

//****************************************************************************
// Method:  ADIOSFileObject::ReadScalarData
//
// Purpose:
//   
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

bool
ADIOSFileObject::ReadScalarData(const std::string &nm, int ts, ADIOS_SELECTION *sel, vtkDataArray **arr)
{
    Open();
    map<string, ADIOS_VARINFO*>::const_iterator it = variables.find(nm);
    if (it == variables.end())
        return false;

    ADIOS_VARINFO *avi = it->second;
    ADIOS_SELECTION *s = sel;
    
    //If no selection specified, do the whole thing.
    if (sel == NULL)
    {
        uint64_t start[4] = {0,0,0,0}, count[4] = {0,0,0,0};
        for (int i = 0; i < avi->ndim; i++)
            count[i] = avi->dims[i];
        s = adios_selection_boundingbox(avi->ndim, start, count);
    }

    bool val = ReadScalarData(avi, ts, s, arr);
    
    if (sel == NULL)
        adios_selection_delete(s);
    
    return val;
}

//****************************************************************************
// Method:  ADIOSFileObject::ReadScalarData
//
// Purpose:
//   
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

bool
ADIOSFileObject::ReadScalarData(ADIOS_VARINFO *avi, int ts, ADIOS_SELECTION *sel, vtkDataArray **arr)
{
    Open();
    *arr = AllocateScalarArray(avi, sel);
    cout<<"SEL: "<<sel<<" "<<sel->type<<" "<<sel->u.bb.ndim<<" s["<<sel->u.bb.start[0]<<" "<<sel->u.bb.start[1]<<"] c["<<sel->u.bb.count[0]<<" "<<sel->u.bb.count[1]<<"]"<<endl;
    cout<<"adios_schedule_read_byid "<<avi->varid<<" "<<ts<<endl;
    adios_schedule_read_byid(fp, sel, avi->varid, ts, 1, (*arr)->GetVoidPointer(0));
    int retval = adios_perform_reads(fp, 1);
    return true;
}

//****************************************************************************
// Method:  ADIOSFileObject::ReadComplexData
//
// Purpose:
//   
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

bool
ADIOSFileObject::ReadComplexData(const std::string &nm, int ts,
                                 ADIOS_SELECTION *sel, vtkDataArray **arr, int idx)
{
    string::size_type ir = nm.rfind("_real");
    string::size_type ii = nm.rfind("_imag");

    string varnm;
    if (ir != string::npos)
        varnm = nm.substr(0,ir);
    else if (ii != string::npos)
        varnm = nm.substr(0,ii);
    else
        return false;

    vtkDataArray *complexArr = NULL;
    if (!ReadScalarData(varnm, ts, sel, &complexArr))
        return false;

    int nt = complexArr->GetNumberOfTuples();
    if (complexArr->IsA("vtkFloatArray"))
        *arr = vtkFloatArray::New();
    else if (complexArr->IsA("vtkDoubleArray"))
        *arr = vtkDoubleArray::New();
    else
        EXCEPTION1(InvalidVariableException, "Complex variable not of type float or double");

    (*arr)->SetNumberOfTuples(nt);
    for (int i = 0; i < nt; i++)
        (*arr)->SetTuple1(i, complexArr->GetComponent(i, idx));
    complexArr->Delete();
    
    return true;
}

//****************************************************************************
// Method:  ADIOSFileObject::AllocateTypedArray
//
// Purpose:
//   
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//   Dave Pugmire, Wed Aug 27 09:43:03 EDT 2014
//   Added missing return argument.
//
//****************************************************************************

vtkDataArray *
ADIOSFileObject::AllocateTypedArray(ADIOS_VARINFO *avi)
{
    vtkDataArray *array = NULL;
    switch (avi->type)
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
    case adios_complex:
        array = vtkFloatArray::New();
        break;
    case adios_double_complex:
        array = vtkDoubleArray::New();
        break;
        
    case adios_long_double: // 16 bytes
    default:
        std::string str = "Inavlid variable type";
        EXCEPTION1(InvalidVariableException, str);
        break;
    }
    
    return array;
}

//****************************************************************************
// Method:  ADIOSFileObject::AllocateScalarArray
//
// Purpose:
//   
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataArray *
ADIOSFileObject::AllocateScalarArray(ADIOS_VARINFO *avi, ADIOS_SELECTION *sel)
{
    vtkDataArray *array = AllocateTypedArray(avi);
    int nt = 0;
    if (sel->type == ADIOS_SELECTION_BOUNDINGBOX)
    {
        nt = 1;
        for (int i = 0; i < sel->u.bb.ndim; i++)
            nt *= sel->u.bb.count[i];
    }
    if (avi->type == adios_complex || avi->type == adios_double_complex)
        array->SetNumberOfComponents(2);
    else
        array->SetNumberOfComponents(1);
    array->SetNumberOfTuples(nt);
    return array;
}

//****************************************************************************
// Method:  ADIOSFileObject::CreateSelection
//
// Purpose:
//   
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************


ADIOS_SELECTION *
ADIOSFileObject::CreateSelection(ADIOS_VARINFO *avi, int block)
{
    if (block > avi->sum_nblocks)
        EXCEPTION1(ImproperUseException, "Block index out of range.");
    
    uint64_t *start = new uint64_t[4], *count = new uint64_t[4];
    for (int i = 0; i < 4; i++)
        start[i] = count[i] = 0;
    if (block < 0)
    {
        for (int i = 0; i < avi->ndim; i++)
            count[i] = avi->dims[i];
    }
    else
    {
        for (int i = 0; i < avi->ndim; i++)
        {
            start[i] = avi->blockinfo[block].start[i];
            count[i] = avi->blockinfo[block].count[i];
        }
    }
    
    ADIOS_SELECTION *sel = adios_selection_boundingbox(avi->ndim, start, count);
    //cout<<"SEL: "<<sel<<" "<<sel->type<<" "<<sel->u.bb.ndim<<" s["<<sel->u.bb.start[0]<<" "<<sel->u.bb.start[1]<<"] c["<<sel->u.bb.count[0]<<" "<<sel->u.bb.count[1]<<"]"<<endl;

    return sel;
}

//****************************************************************************
// Method:  ADIOSFileObject::GetMeshInfo
//
// Purpose:
//   
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

ADIOS_MESH *
ADIOSFileObject::GetMeshInfo(ADIOS_VARINFO *avi)
{
    if (avi->meshinfo == NULL)
        adios_inq_var_meshinfo(fp, avi);
    
    if (avi->meshinfo)
        return adios_inq_mesh_byid(fp, avi->meshinfo->meshid);
    return NULL;
}

//****************************************************************************
// Method:  ADIOSFileObject::ReadCoordinates
//
// Purpose:
//   
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//   Dave Pugmire, Tue Jun 17 13:24:30 EDT 2014
//   Bug fix to ADIOSFile interface.
//
//****************************************************************************

vtkPoints *
ADIOSFileObject::ReadCoordinates(const std::string &nm, int ts, int dim, int nPts)
{
    Open();
    vtkDataArray *coords = NULL;
    ReadScalarData(nm, ts, &coords);

    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(nPts);
    if (dim == 2)
    {
        for (int i = 0, j = 0; i < nPts; i++, j+=2)
        {
            pts->SetPoint(i,
                          coords->GetTuple1(j),
                          coords->GetTuple1(j+1),
                          0.0);
        }
    }
    else
    {
        for (int i = 0, j = 0; i < nPts; i++, j+=3)
        {
            pts->SetPoint(i,
                          coords->GetTuple1(j),
                          coords->GetTuple1(j+1),
                          coords->GetTuple1(j+2));
        }
    }
    
    coords->Delete();
    return pts;
}
