/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                                  MemStream.C                              //
// ************************************************************************* //

#include <MemStream.h>
#include <vtkDataSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkDataArray.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIdTypeArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>

using namespace std;

static vtkFieldData* readFD(MemStream *buff);
static void readCD(vtkDataSet *ds, MemStream *buff);
static void readPD(vtkDataSet *ds, MemStream *buff);

static vtkDataArray* readARR(MemStream *buff);
static vtkRectilinearGrid* readRG(MemStream *buff);

static void writeFD(vtkFieldData *fd, MemStream *buff);
static void writeCD(vtkDataSet *ds, MemStream *buff);
static void writePD(vtkDataSet *ds, MemStream *buff);
static void writeARR(vtkDataArray *arr, MemStream *buff);
static void writeRG(vtkRectilinearGrid *ds, MemStream *buff);

// ****************************************************************************
//  Method: MemStream constructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

MemStream::MemStream(size_t sz0)
{
    _pos = 0;
    _len = 0;
    _maxLen = _len;
    _data = NULL;
    CheckSize(sz0);
}


// ****************************************************************************
//  Method: MemStream constructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

MemStream::MemStream(size_t sz, const unsigned char *buff)
{
    _pos = 0;
    _len = sz;
    _maxLen = _len;
    
    _data = new unsigned char[_len];
    memcpy(_data, buff, _len);
}

// ****************************************************************************
// Method:  MemStream constructor
//
// Programmer:  Dave Pugmire
// Creation:    September 10, 2010
//
// ****************************************************************************

MemStream::MemStream(const MemStream &s)
{
    _pos = 0;
    _len = s.len();
    _maxLen = _len;
    _data = new unsigned char[_len];
    memcpy(_data, s.data(), _len);
}


// ****************************************************************************
//  Method: MemStream destructor
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

MemStream::~MemStream()
{
    if (_data)
        delete [] _data;
    _pos = 0;
    _len = 0;
    _maxLen = 0;
    _data = NULL;
}

// ****************************************************************************
//  Method: MemStream::CheckSize
//
//  Purpose:
//      Checks to see if the current size is sufficent and changes the size
//      of the underlying array if not.
//
//  Programmer: Christoph Garth
//  Creation:   February 25, 2008
//
// ****************************************************************************

void
MemStream::CheckSize(size_t sz)
{
    size_t reqLen = _pos+sz;
    
    if (reqLen > _maxLen)
    {
        size_t newLen = 2*_maxLen; // double current size.
        if (newLen < reqLen)
            newLen = reqLen;
        
        unsigned char *newData = new unsigned char[newLen];
        
        if (_data)
        {
            memcpy(newData, _data, _len); // copy existing data to new buffer.
            delete [] _data;
        }
        _data = newData;
        _maxLen = newLen;
    }
}


//Tokens for vtkDataSet.
static unsigned char RECTILINEAR_GRID = 0;
static unsigned char STRUCTURED_GRID  = 1;
static unsigned char FIELD_DATA       = 2;
static unsigned char CELL_DATA        = 3;
static unsigned char POINT_DATA       = 4;
static unsigned char SCALARS          = 5;
static unsigned char VECTORS          = 6;
static unsigned char NULL_DATA        = 255;

void
MemStream::write(vtkDataSet *ds)
{
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
        writeRG(vtkRectilinearGrid::SafeDownCast(ds), this);

    vtkFieldData *fd = ds->GetFieldData();
    writeFD(fd, this);
    
    //Write out cell, point data.
    writeCD(ds, this);
    writePD(ds, this);
}

void
MemStream::read(vtkDataSet **ds)
{
    char dsType;
    read(dsType);
    
    if (dsType == RECTILINEAR_GRID)
        *ds = readRG(this);
    else
        EXCEPTION1(ImproperUseException, "INVALID TOKEN!!");
    
    vtkFieldData *fd = readFD(this);
    (*ds)->SetFieldData(fd);
    fd->Delete();
    
    readCD(*ds, this);
    readPD(*ds, this);
}

static void
writeRG(vtkRectilinearGrid *rg, MemStream *buff)
{
    buff->write(RECTILINEAR_GRID);
    int dim[3];
    rg->GetDimensions(dim);
    buff->write(dim[0]);
    buff->write(dim[1]);
    buff->write(dim[2]);
    
    writeARR(rg->GetXCoordinates(), buff);
    writeARR(rg->GetYCoordinates(), buff);
    writeARR(rg->GetZCoordinates(), buff);
}

static void
writeFD(vtkFieldData *fd, MemStream *buff)
{
    int attributeIndices[vtkDataSetAttributes::NUM_ATTRIBUTES];
    for (int i=0; i<vtkDataSetAttributes::NUM_ATTRIBUTES; i++)
        attributeIndices[i] = -1;
    vtkDataSetAttributes* dsa;
    if ((dsa=vtkDataSetAttributes::SafeDownCast(fd)))
        dsa->GetAttributeIndices(attributeIndices);
    int nArrs = fd->GetNumberOfArrays(), numActualArrays = 0;
    for (int i=0; i < nArrs; i++)
    {
        bool found = false;
        for (int j = 0; !found && j < vtkDataSetAttributes::NUM_ATTRIBUTES; j++)
            found = (i == attributeIndices[j]);
        if (!found)
            numActualArrays++;
    }

    buff->write(FIELD_DATA);
    buff->write(numActualArrays);
    
    for (int i = 0; i < nArrs; i++)
    {
        bool found = false;
        for (int j = 0; !found && j < vtkDataSetAttributes::NUM_ATTRIBUTES; j++)
            found = (i == attributeIndices[j]);

        if (!found)
            writeARR(fd->GetArray(i), buff);
    }
}

static void
writeCD(vtkDataSet *ds, MemStream *buff)
{
    vtkCellData *cd = ds->GetCellData();
    if (cd == NULL)
    {
        buff->write(NULL_DATA);
        return;
    }
    
    buff->write(CELL_DATA);
    buff->write(ds->GetNumberOfCells());
    vtkDataArray *arr = cd->GetScalars();
    if (arr && arr->GetNumberOfComponents() > 0)
    {
        buff->write(SCALARS);
        writeARR(arr, buff);
    }
    else
        buff->write(NULL_DATA);

    arr = cd->GetVectors();
    if (arr && arr->GetNumberOfComponents() > 0)
    {
        buff->write(VECTORS);
        writeARR(arr, buff);
    }
    else
        buff->write(NULL_DATA);
    
    vtkFieldData *fd = cd;
    writeFD(fd, buff);
}

static void
writePD(vtkDataSet *ds, MemStream *buff)
{
    vtkPointData *pd = ds->GetPointData();
    if (pd == NULL)
    {
        buff->write(NULL_DATA);
        return;
    }
    
    buff->write(POINT_DATA);
    buff->write(ds->GetNumberOfPoints());
    vtkDataArray *arr = pd->GetScalars();
    if (arr && arr->GetNumberOfComponents() > 0)
    {
        buff->write(SCALARS);
        writeARR(arr, buff);
    }
    else
        buff->write(NULL_DATA);

    arr = pd->GetVectors();
    if (arr && arr->GetNumberOfComponents() > 0)
    {
        buff->write(VECTORS);
        writeARR(arr, buff);
    }
    else
        buff->write(NULL_DATA);
    
    vtkFieldData *fd = pd;
    writeFD(fd, buff);
}

static void
writeARR(vtkDataArray *arr, MemStream *buff)
{
    int dataType = arr->GetDataType();
    int nTuples = arr->GetNumberOfTuples();
    int nComp = arr->GetNumberOfComponents();
    char *aa = arr->GetName();

    string nm;
    if (aa)
        nm = aa;
    
    buff->write(nTuples);
    buff->write(nComp);
    buff->write(dataType);
    buff->write(nm);
    
    unsigned char *data = NULL;
    int sz = nTuples*nComp;
    
    if (dataType == VTK_UNSIGNED_CHAR)
    {
        data = (unsigned char*) (((vtkUnsignedCharArray *)arr)->GetPointer(0));
        sz *= sizeof(unsigned char);
    }
    else if (dataType == VTK_INT)
    {
        data = (unsigned char*) (((vtkIntArray *)arr)->GetPointer(0));
        sz *= sizeof(int);
    }
    else if (dataType == VTK_FLOAT)
    {
        data = (unsigned char*) (((vtkFloatArray *)arr)->GetPointer(0));
        sz *= sizeof(float);
    }
    else if (dataType == VTK_DOUBLE)
    {
        data = (unsigned char*) (((vtkDoubleArray *)arr)->GetPointer(0));
        sz *= sizeof(double);
    }
    else if (dataType == VTK_ID_TYPE)
    {
        data = (unsigned char*) (((vtkIdTypeArray *)arr)->GetPointer(0));
        sz *= sizeof(vtkIdType);
    }
    else
        EXCEPTION1(ImproperUseException, "Datatype not handled.");

    if (data)
        buff->write(data, sz);
}

static vtkFieldData *
readFD(MemStream *buff)
{
    unsigned char v;
    buff->read(v);
    if (v != FIELD_DATA)
        EXCEPTION1(ImproperUseException, "INVALID TOKEN!!");
    
    int nArrays;
    buff->read(nArrays);
    vtkFieldData *fd = vtkFieldData::New();
    fd->AllocateArrays(nArrays);

    for (int i = 0; i < nArrays; i++)
    {
        vtkDataArray *arr = readARR(buff);
        fd->AddArray(arr);
        arr->Delete();
    }

    return fd;
}

static void
readCD(vtkDataSet *ds, MemStream *buff)
{
    unsigned char v;
    buff->read(v);
    if (v != CELL_DATA)
        return;

    int numCells;
    buff->read(numCells);

    buff->read(v);
    if (v == SCALARS)
    {
        vtkDataArray *scalars = readARR(buff);
        ds->GetCellData()->SetScalars(scalars);
        scalars->Delete();
    }
    
    buff->read(v);
    if (v == VECTORS)
    {
        vtkDataArray *vectors = readARR(buff);
        ds->GetCellData()->SetVectors(vectors);
        vectors->Delete();
    }
    
    vtkFieldData *fd = readFD(buff);
    for (int i = 0; i < fd->GetNumberOfArrays(); i++)
        ds->GetCellData()->AddArray(fd->GetArray(i));

    fd->Delete();
}

static void
readPD(vtkDataSet *ds, MemStream *buff)
{
    unsigned char v;
    buff->read(v);
    if (v != POINT_DATA)
        return;

    int nPts;
    buff->read(nPts);
    buff->read(v);
    if (v == SCALARS)
    {
        vtkDataArray *scalars = readARR(buff);
        ds->GetPointData()->SetScalars(scalars);
        scalars->Delete();
    }
    
    buff->read(v);
    if (v == VECTORS)
    {
        vtkDataArray *vectors = readARR(buff);
        ds->GetPointData()->SetVectors(vectors);
        vectors->Delete();
    }

    vtkFieldData *fd = readFD(buff);
    for (int i = 0; i < fd->GetNumberOfArrays(); i++)
        ds->GetPointData()->AddArray(fd->GetArray(i));

    fd->Delete();
}

static vtkDataArray *
readARR(MemStream *buff)
{
    int dataType, nTuples, nComp;
    string nm;

    buff->read(nTuples);
    buff->read(nComp);
    buff->read(dataType);
    buff->read(nm);

    vtkDataArray *array = NULL;
    size_t sz = nTuples*nComp;

    if (dataType == VTK_UNSIGNED_CHAR)
    {
        array = vtkUnsignedCharArray::New();
        sz *= sizeof(unsigned char);
    }
    else if (dataType == VTK_INT)
    {
        array = vtkIntArray::New();
        array->SetNumberOfComponents(nComp);
        //array->Allocate(nTuples);
        sz *= sizeof(int);
    }
    else if (dataType == VTK_FLOAT)
    {
        array = vtkFloatArray::New();
        array->SetNumberOfComponents(nComp);
        //array->Allocate(nTuples);
        sz *= sizeof(float);
    }
    else if (dataType == VTK_DOUBLE)
    {
        array = vtkDoubleArray::New();
        array->SetNumberOfComponents(nComp);
        //array->Allocate(nTuples);
        sz *= sizeof(double);
    }
    else
        EXCEPTION1(ImproperUseException, "Datatype not handled.");
    
    array->SetNumberOfComponents(nComp);
    array->SetNumberOfTuples(nTuples);
    array->SetName(nm.c_str());
    
    unsigned char *ptr = (unsigned char *)array->GetVoidPointer(0);
    buff->read(ptr, sz);
    return array;
}

static vtkRectilinearGrid *
readRG(MemStream *buff)
{
    vtkRectilinearGrid *rg = vtkRectilinearGrid::New();

    int dim[3];
    buff->read(dim[0]);
    buff->read(dim[1]);
    buff->read(dim[2]);
    rg->SetDimensions(dim);
    
    vtkDataArray *X = readARR(buff);
    vtkDataArray *Y = readARR(buff);
    vtkDataArray *Z = readARR(buff);

    rg->SetXCoordinates(X);
    rg->SetYCoordinates(Y);
    rg->SetZCoordinates(Z);
    
    X->Delete();
    Y->Delete();
    Z->Delete();

    return rg;
}
