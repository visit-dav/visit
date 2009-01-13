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

// ************************************************************************* //
//                           avtTransformManager.C                           //
// ************************************************************************* //
#include <vtkCellData.h>
#include <vtkCSGGrid.h>
#include <vtkCharArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkShortArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkIntArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkLongArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkDoubleArray.h>
#include <vtkIdTypeArray.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFieldData.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkCellTypes.h>
#include <vtkUnstructuredGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtDatasetCollection.h>
#include <avtParallel.h>
#include <avtSourceFromDatabase.h>
#include <avtTransformManager.h>

#include <ImproperUseException.h>
#include <PointerNotInCacheException.h>

#include <DebugStream.h>
#include <Utility.h>
#include <snprintf.h>

#include <vectortypes.h>

#include <map>
#include <vector>

using std::vector;
using std::map;


// ****************************************************************************
//  Function: GetArrayTypeName 
//
//  Purpose: Given a vtk data array, return the a char * for its type name
//
//  Programmer: Mark C. Miller 
//  Creation:   April 4, 2005 
// 
//  Modifications:
//
//    Hank Childs, Fri Jun  9 14:03:55 PDT 2006
//    Add return statement for unmet cases.
//
//    Mark C. Miller, Wed Sep 13 09:06:07 PDT 2006
//    Moved here from avtGenericDatabase.C
//
// ****************************************************************************

static const char *DataArrayTypeName(vtkDataArray *arr)
{
    switch (arr->GetDataType())
    {
        case VTK_VOID:           return "void*"; 
        // case VTK_BIT: not sure what to do here
        case VTK_CHAR:           return "char"; 
        case VTK_UNSIGNED_CHAR:  return "unsigned char"; 
        case VTK_SHORT:          return "short";
        case VTK_UNSIGNED_SHORT: return "unsigned short";
        case VTK_INT:            return "int";
        case VTK_UNSIGNED_INT:   return "unsigned int";
        case VTK_LONG:           return "long";
        case VTK_UNSIGNED_LONG:  return "unsigned long";
        case VTK_FLOAT:          return "float";
        case VTK_DOUBLE:         return "double";
        case VTK_ID_TYPE:        return "vtkIdType";
    }

    return "<does not match any known type>";
}

// ****************************************************************************
//  Function: PrecisionInBytes
//
//  Purpose: Given a vtk data type, return its precision in bytes 
//
//  Programmer: Mark C. Miller 
//  Creation:   April 4, 2005 
// 
//  Modifications:
//     Mark C. Miller, Tue Sep 13 20:07:48 PDT 2005
//     Made it just take data type as arg instead of a vtkDataArray
//
//    Hank Childs, Fri Jun  9 14:03:55 PDT 2006
//    Add return statement for unmet cases.
//
//    Mark C. Miller, Wed Sep 13 09:06:07 PDT 2006
//    Moved here from avtGenericDatabase.C
//
// ****************************************************************************

static int
PrecisionInBytes(int dataType)
{
    switch (dataType)
    {
        case VTK_VOID:           return sizeof(void*);
        // case VTK_BIT: not sure what to do here
        case VTK_CHAR:           return sizeof(char);
        case VTK_UNSIGNED_CHAR:  return sizeof(unsigned char);
        case VTK_SHORT:          return sizeof(short);
        case VTK_UNSIGNED_SHORT: return sizeof(unsigned short);
        case VTK_INT:            return sizeof(int);
        case VTK_UNSIGNED_INT:   return sizeof(unsigned int);
        case VTK_LONG:           return sizeof(long);
        case VTK_UNSIGNED_LONG:  return sizeof(unsigned long);
        case VTK_FLOAT:          return sizeof(float);
        case VTK_DOUBLE:         return sizeof(double);
        case VTK_ID_TYPE:        return sizeof(vtkIdType);
    }

    return -1;
}

// ****************************************************************************
//  Function: PrecisionInBytes
//
//  Purpose: Given a vtk data array, return its precision in bytes 
//
//  Programmer: Mark C. Miller 
//  Creation:   August 10, 2005 
//
//  Modifications:
// 
//    Mark C. Miller, Wed Sep 13 09:06:07 PDT 2006
//    Moved here from avtGenericDatabase.C
//
// ****************************************************************************
static int
PrecisionInBytes(vtkDataArray *var)
{
    if (var)
        return PrecisionInBytes(var->GetDataType());
    else
        return PrecisionInBytes(VTK_FLOAT);
}

// ****************************************************************************
//  Function: IsAdmissibleDataType
//
//  Purpose: Given a vector of admissible types and a given type, return
//  whehter or not the given type is in the vector
//
//  Programmer: Mark C. Miller 
//  Creation:   April 4, 2005 
//
//  Modifications:
// 
//    Mark C. Miller, Wed Sep 13 09:06:07 PDT 2006
//    Moved here from avtGenericDatabase.C
//
// ****************************************************************************

static bool
IsAdmissibleDataType(const vector<int>& admissibleTypes, const int type)
{
    for (int i = 0; i < admissibleTypes.size(); i++)
    {
        if (admissibleTypes[i] == type)
            return true;
    }
    return false;
}

// ****************************************************************************
//  Template: ConvertToType
//
//  Purpose: Template for conversion, usually to float 
//
//  Programmer: Mark C. Miller 
//  Creation:   April 4, 2005 
//
//  Modifications:
//
//    Mark C. Miller, Wed Sep 13 09:06:07 PDT 2006
//    Moved here from avtGenericDatabase.C
// 
// ****************************************************************************
template <class oT, class iT>
static void ConvertToType(oT *obuf, const iT* ibuf, int n)
{
    for (int i = 0; i < n; i++)
        obuf[i] = (oT) ibuf[i];
}

// ****************************************************************************
//  Function: ConvertDataArrayToFloat 
//
//  Purpose: Given a vtk data array, make a copie of it that is converted
//  to float
//
//  Programmer: Mark C. Miller 
//  Creation:   April 4, 2005 
// 
//  Modifications:
//    Jeremy Meredith, Tue Apr  5 11:49:30 PDT 2005
//    Removed the templating.  It was causing compiler errors.
//
//    Mark C. Miller, Tue Aug 16 13:56:55 PDT 2005
//    Eliminated attempt to print array's name using GetName()
//
//    Mark C. Miller, Wed Sep 13 09:06:07 PDT 2006
//    Moved here from avtGenericDatabase.C
//
//    Kathleen Bonnell, Tue Apr  3 07:56:18 PDT 2007
//    Only print array's name if it is not null, the data arrays for vtkPoints
//    generally don't have names.
//
// ****************************************************************************

static vtkDataArray * 
ConvertDataArrayToFloat(vtkDataArray *oldArr)
{
    vtkDataArray *newArr = 0; 

    if (oldArr->GetDataType() != VTK_FLOAT)
    {
        newArr = vtkFloatArray::New();

        int numTuples = oldArr->GetNumberOfTuples();
        int numComponents = oldArr->GetNumberOfComponents();

        newArr->SetNumberOfComponents(numComponents);
        newArr->SetNumberOfTuples(numTuples);

        float *newBuf = (float*) newArr->GetVoidPointer(0);
        void *oldBuf = oldArr->GetVoidPointer(0);

        debug1 << "avtTransformManager: Converting vktDataArray, ";
        if (oldArr->GetName() != NULL)
               debug1 << "\"" << oldArr->GetName() << "\", ";
        debug1 << "with " << numTuples << " tuples and "
               << numComponents << " components from type \""
               << DataArrayTypeName(oldArr) << "\" to \"float\"" << endl;

        int numValues = numTuples * numComponents;
        switch (oldArr->GetDataType())
        {
            case VTK_CHAR:
                ConvertToType(newBuf, (char*) oldBuf, numValues);
                break;
            case VTK_UNSIGNED_CHAR:
                ConvertToType(newBuf, (unsigned char*) oldBuf, numValues);
                break;
            case VTK_SHORT:
                ConvertToType(newBuf, (short*) oldBuf, numValues);
                break;
            case VTK_UNSIGNED_SHORT:
                ConvertToType(newBuf, (unsigned short*) oldBuf, numValues);
                break;
            case VTK_INT:
                ConvertToType(newBuf, (int*) oldBuf, numValues);
                break;
            case VTK_UNSIGNED_INT:
                ConvertToType(newBuf, (unsigned int*) oldBuf, numValues);
                break;
            case VTK_LONG:
                ConvertToType(newBuf, (long*) oldBuf, numValues);
                break;
            case VTK_UNSIGNED_LONG:
                ConvertToType(newBuf, (unsigned long*) oldBuf, numValues);
                break;
            case VTK_DOUBLE:
                ConvertToType(newBuf, (double*) oldBuf, numValues);
                break;
            case VTK_ID_TYPE:
                ConvertToType(newBuf, (vtkIdType*) oldBuf, numValues);
                break;
            default:
                {   char msg[256];
                    SNPRINTF(msg, sizeof(msg), "Cannot convert from type \"%s\" to float",
                        DataArrayTypeName(oldArr));
                    EXCEPTION1(ImproperUseException, msg); 
                }
        }
    }

    if (newArr)
        newArr->SetName(oldArr->GetName());

    return newArr;
}

// ****************************************************************************
//  Funcion: GetCoordDataType
//
//  Purpose: Given a vtkDataSet, return a sample the first of its
//  coordinate arrays
//
//  Programmer: Mark C. Miller 
//  Creation:   April 4, 2005 
// 
//  Modifications:
//    Mark C. Miller, Tue Sep 13 20:07:48 PDT 2005
//    Made it return the coordinate data type explicitly. Also, made it
//    a little more lenient about getting null pointers
//
//    Mark C. Miller, Wed Sep 13 09:06:07 PDT 2006
//    Moved here from avtGenericDatabase.C
// ****************************************************************************
static int 
GetCoordDataType(vtkDataSet *ds)
{
    switch (ds->GetDataObjectType())
    {
        case VTK_POLY_DATA:
        case VTK_STRUCTURED_GRID:
        case VTK_UNSTRUCTURED_GRID:
            {
                vtkPointSet *ps = vtkPointSet::SafeDownCast(ds);
                if (ps && ps->GetPoints() && ps->GetPoints()->GetData())
                {
                    return ps->GetPoints()->GetData()->GetDataType();
                }
            }
            break;

        case VTK_RECTILINEAR_GRID:
            {
                vtkRectilinearGrid *rg = vtkRectilinearGrid::SafeDownCast(ds);
                if (rg && rg->GetXCoordinates())
                {
                    return rg->GetXCoordinates()->GetDataType();
                }
            }
            break;
    }
    return VTK_FLOAT;
}

// ****************************************************************************
//  Function: ConvertDataSetToFloat
//
//  Purpose: Given a vtkDataSet, convert its coordinate arrays, if any,
//  to float
//
//  Programmer: Mark C. Miller 
//  Creation:   April 4, 2005 
//
//  Moficiations:
// 
//    Mark C. Miller, Wed Sep 13 09:06:07 PDT 2006
//    Moved here from avtGenericDatabase.C
//
// ****************************************************************************
static vtkDataSet * 
ConvertDataSetToFloat(vtkDataSet *oldds)
{
    vtkDataSet *newds = 0;

    switch (oldds->GetDataObjectType())
    {
        case VTK_POLY_DATA:
        case VTK_STRUCTURED_GRID:
        case VTK_UNSTRUCTURED_GRID:
            {
                vtkPointSet *oldps = vtkPointSet::SafeDownCast(oldds);
                if (oldps != 0)
                {
                    vtkDataArray *oldArr = oldps->GetPoints()->GetData();
                    vtkDataArray *newArr = ConvertDataArrayToFloat(oldArr);

                    if (newArr != 0)
                    {
                        vtkPointSet *newps = oldps->NewInstance(); 
                        newps->DeepCopy(oldps);

                        vtkPoints *newpts = vtkPoints::New();
                        newpts->DeepCopy(oldps->GetPoints());

                        newpts->SetData(newArr);
                        newArr->Delete();

                        newps->SetPoints(newpts);
                        newpts->Delete();

                        newds = newps;
                    }
                }
            }
            break;

        case VTK_RECTILINEAR_GRID:
            {
                vtkRectilinearGrid *oldrg = vtkRectilinearGrid::SafeDownCast(oldds);
                if (oldrg != 0)
                {
                    vtkDataArray *oldX = oldrg->GetXCoordinates();
                    vtkDataArray *newX = ConvertDataArrayToFloat(oldX);
                    vtkDataArray *oldY = oldrg->GetYCoordinates();
                    vtkDataArray *newY = ConvertDataArrayToFloat(oldY);
                    vtkDataArray *oldZ = oldrg->GetZCoordinates();
                    vtkDataArray *newZ = ConvertDataArrayToFloat(oldZ);

                    if (newX != 0 && newY != 0 && newZ != 0)
                    {
                        vtkRectilinearGrid *newrg = vtkRectilinearGrid::New();
                        newrg->DeepCopy(oldrg);

                        newrg->SetXCoordinates(newX);
                        newX->Delete();

                        newrg->SetYCoordinates(newY);
                        newY->Delete();

                        newrg->SetZCoordinates(newZ);
                        newZ->Delete();

                        newds = newrg;
                    }
                }
            }
            break;

        case VTK_IMAGE_DATA:
        case VTK_STRUCTURED_POINTS:
            // we have nothing to do here
            break;
    }
    return newds;
}

// ****************************************************************************
//  Template: ConvertDataSetToFloat
//
//  Purpose: Build a mapping array to handle changes in zone/node numbering
//  and order when dealing with variables
//
//  Programmer: Mark C. Miller 
//  Creation:   December 4, 2006 
//
// ****************************************************************************
template <class iT>
static iT *
BuildMappedArray(const iT *const ibuf, int ncomps, const vector<int> &valsToMap)
{
    const int nvals = valsToMap.size();

    // we use malloc here because we'll assign the data to vtkDataArray using
    // SetVoidArray and, upon deletion, that assumes malloc was used
    iT *rbuf = (iT *) malloc(nvals * ncomps * sizeof(iT));
    for (int i = 0; i < nvals; i++)
    {
        for (int j = 0; j < ncomps; j++)
            rbuf[i*ncomps+j] = ibuf[valsToMap[i]*ncomps+j];
    }
    return rbuf;
}

// ****************************************************************************
//  Template: ConvertDataSetToFloat
//
//  Purpose: Build a mapping array to handle changes in zone/node numbering
//  and order when dealing with variables
//
//  Programmer: Mark C. Miller 
//  Creation:   December 4, 2006 
//
// ****************************************************************************
static vtkDataArray *
BuildMappedArray(vtkDataArray *da, const vector<int> &valsToMap)
{
    const int nvals = valsToMap.size();
    const int ncomps = da->GetNumberOfComponents();
    const void *buf = da->GetVoidPointer(0);
    vtkDataArray *rv;
    void *rbuf;
    switch (da->GetDataType())
    {
        case VTK_CHAR:
            rv = vtkCharArray::New();
            rbuf = BuildMappedArray((char*) buf, ncomps, valsToMap);
            break;
        case VTK_UNSIGNED_CHAR:
            rv = vtkUnsignedCharArray::New();
            rbuf = BuildMappedArray((unsigned char*) buf, ncomps, valsToMap);
            break;
        case VTK_SHORT:
            rv = vtkShortArray::New();
            rbuf = BuildMappedArray((short*) buf, ncomps, valsToMap);
            break;
        case VTK_UNSIGNED_SHORT:
            rv = vtkUnsignedShortArray::New();
            rbuf = BuildMappedArray((unsigned short*) buf, ncomps, valsToMap);
            break;
        case VTK_INT:
            rv = vtkIntArray::New();
            rbuf = BuildMappedArray((int*) buf, ncomps, valsToMap);
            break;
        case VTK_UNSIGNED_INT:
            rv = vtkUnsignedIntArray::New();
            rbuf = BuildMappedArray((unsigned int*) buf, ncomps, valsToMap);
            break;
        case VTK_LONG:
            rv = vtkLongArray::New();
            rbuf = BuildMappedArray((long*) buf, ncomps, valsToMap);
            break;
        case VTK_UNSIGNED_LONG:
            rv = vtkUnsignedLongArray::New();
            rbuf = BuildMappedArray((unsigned long*) buf, ncomps, valsToMap);
            break;
        case VTK_FLOAT:
            rv = vtkFloatArray::New();
            rbuf = BuildMappedArray((float*) buf, ncomps, valsToMap);
            break;
        case VTK_DOUBLE:
            rv = vtkDoubleArray::New();
            rbuf = BuildMappedArray((double*) buf, ncomps, valsToMap);
            break;
        case VTK_ID_TYPE:
            rv = vtkIdTypeArray::New();
            rbuf = BuildMappedArray((vtkIdType*) buf, ncomps, valsToMap);
            break;
        default:
            {   char msg[256];
                SNPRINTF(msg, sizeof(msg), "Cannot transform array of type \"%s\"",
                    DataArrayTypeName(da));
                EXCEPTION1(ImproperUseException, msg); 
            }
    }

    if (rv)
    {
        rv->SetName(da->GetName());
        rv->SetNumberOfComponents(ncomps);
        rv->SetVoidArray(rbuf, nvals*ncomps, 0);
    }

    return rv;
}

// ****************************************************************************
//  Function: IsInternalAVTArray 
//
//  Purpose: Filter certain internal AVT arrays that should not undergo
//  conversion 
//
//  Programmer: Mark C. Miller 
//  Creation:   December 5, 2006 
//
// ****************************************************************************
static bool
IsInternalAVTArray(vtkDataArray *da)
{
    if (strncmp(da->GetName(), "avt", 3) == 0)
        return true;
    return false;
}

// ****************************************************************************
//  Function: DestructDspec
//
//  Purpose: To support caching of avtDataRequest objects 
//
//  Programmer: Mark C. Miller 
//  Creation:   December 4, 2006 
//
// ****************************************************************************
static void
DestructDspec(void *p)
{
    avtDataRequest *dataRequest = (avtDataRequest *) p;
    delete dataRequest;
}

avtTransformManager::avtTransformManager(avtVariableCache *_gdbCache) :
    gdbCache(_gdbCache)
{
}

avtTransformManager::~avtTransformManager()
{
}

void
avtTransformManager::FreeUpResources(int lastts)
{
    cache.ClearTimestep(lastts);
}

// ****************************************************************************
//  Method: NativeToFloat transformation
//
//  Purpose: Convert dataset and/or data arrays defined on it to from their
//  native type to float
//
//  Programmer: Mark C. Miller 
//  Creation:   December 4, 2006 
//
//  Modifications:
//
//    Mark C. Miller, Tue Dec  5 12:36:36 PST 2006
//    Changed it to handle conversions even when object was not cached
//    in generic db's cache.
//
//    Mark C. Miller, Tue Dec  5 18:14:58 PST 2006
//    Made it handle cases where vars are not cached in generic db's cache.
//    Made it ignore certain internal AVT arrays. 
//    Added more debugging output.
//
//    Mark C. Miller, Wed Dec  6 13:40:17 PST 2006
//    Fixed use of SetScalars|Vectors|Tensors and AddArray for copying
//    over various cell/point data arrays.
//
//    Hank Childs, Fri May  9 16:02:39 PDT 2008
//    Added argument for the domain ID to the signature.  This is needed for
//    efficiency when accessing the cache.
//
//    Hank Childs, Mon Aug 25 16:19:57 PDT 2008
//    Make delete logic squeaky clean.
//
// ****************************************************************************
vtkDataSet *
avtTransformManager::NativeToFloat(const avtDatabaseMetaData *const md,
    const avtDataRequest_p &dataRequest, vtkDataSet *ds, int dom)
{
    if (!ds)
        return 0;

    const char *vname, *type, *mat;
    int i, ts;

    bool needNativePrecision;
    vector<int> admissibleDataTypes;
    if (*dataRequest)
    {
        needNativePrecision = dataRequest->NeedNativePrecision();
        admissibleDataTypes = dataRequest->GetAdmissibleDataTypes();
    }
    else
    {
        needNativePrecision = false;
        admissibleDataTypes.push_back(VTK_FLOAT);
    }

    //
    // We make two passes here, the first to simply decide if any
    // conversion is actually needed and then the second to actually
    // do it.
    //
    vtkDataSet *rv = ds;
    bool anyConversionNeeded = false;
    map<void*,bool> objectWasCachedInGenericDB;
    for (int pass = 0; pass < 1 + anyConversionNeeded; pass++)
    {
        if (pass == 1)
        {
            debug1 << "avtTransformManager: Applying NativeToFloat transform" << endl;
        }

        //
        // Deal with mesh first 
        //
        if (!IsAdmissibleDataType(admissibleDataTypes, GetCoordDataType(ds)) ||
            (!needNativePrecision && PrecisionInBytes(GetCoordDataType(ds)) > sizeof(float)))
        {
            anyConversionNeeded = true;
            if (pass == 1)
            {
                // look up this vtk object's "key" in GenericDb's cache
                objectWasCachedInGenericDB[ds] =
                    gdbCache->GetVTKObjectKey(&vname, &type, &ts, dom, &mat, ds);

                // first, see if a converted result is already in tmgr's cache
                if (objectWasCachedInGenericDB[ds])
                {
                    rv = (vtkDataSet*) cache.GetVTKObject(vname, type, ts, dom, mat);
                }
                else
                {
                    debug1 << "avtTransformManager: dataset is not in generic db's cache" << endl;
                    rv = 0;
                }

                bool needDelete = false;
                if (!rv)
                {
                    debug1 << "avtTransformManager: Converting data set from native to float" << endl;
                    rv = ConvertDataSetToFloat(ds);
                    needDelete = true;
                    if (objectWasCachedInGenericDB[ds])
                    {
                        cache.CacheVTKObject(vname, type, ts, dom, mat, rv);
                    }
                }
                vtkDataSet *tmprv = rv->NewInstance();
                tmprv->CopyStructure(rv);
                tmprv->GetFieldData()->ShallowCopy(rv->GetFieldData());
                if (needDelete)
                    rv->Delete();
                rv = tmprv;
            }
        }
        else if (pass == 1)
        {
            // if we're on a 2nd pass, we know we need conversion
            // for the variable, but not the mesh its defined on
            //rv = ds;
            rv = ds->NewInstance();
            rv->CopyStructure(ds);
            rv->GetFieldData()->ShallowCopy(ds->GetFieldData());
        }

        //
        // Now, deal with cell data
        //
        vtkCellData *cd = ds->GetCellData();
        for (i = 0; i < cd->GetNumberOfArrays(); i++)
        {
            vtkDataArray *da = cd->GetArray(i);
            if (!IsInternalAVTArray(da) && 
                (!IsAdmissibleDataType(admissibleDataTypes, da->GetDataType()) ||
                 (!needNativePrecision && PrecisionInBytes(da) > sizeof(float))))
            {
                anyConversionNeeded = true;
                if (pass == 1)
                {
                    // look up this vtk object's "key" in GenericDb's cache
                    objectWasCachedInGenericDB[da] =
                        gdbCache->GetVTKObjectKey(&vname, &type, &ts, dom, &mat, da);

                    vtkDataArray *newda = 0;
                    if (objectWasCachedInGenericDB[da])
                        newda = (vtkDataArray *) cache.GetVTKObject(vname, type, ts, dom, mat);
                    else
                        debug1 << "avtTransformManager: Array \"" << da->GetName() << "\" was not in generic db's cache" << endl;

                    bool needDelete = false;
                    if (!newda)
                    {
                        debug1 << "avtTransformManager: Array \"" << da->GetName() << "\" was not in tmngr's cache" << endl;
                        newda = ConvertDataArrayToFloat(da);
                        needDelete = true;
                        if (objectWasCachedInGenericDB[da])
                        {
                            cache.CacheVTKObject(vname, type, ts, dom, mat, newda);
                        }
                    }
                    if (cd->GetScalars() == da)
                        rv->GetCellData()->SetScalars(newda);
                    else if (cd->GetVectors() == da)
                        rv->GetCellData()->SetVectors(newda);
                    else if (cd->GetTensors() == da)
                        rv->GetCellData()->SetTensors(newda);
                    else
                        rv->GetCellData()->AddArray(newda);
                    if (needDelete)
                        newda->Delete();
                }
            }
            else if (pass == 1)
            {
                debug1 << "avtTransformManager: Passing along array \"" << da->GetName() << "\"" << endl;
                if (IsInternalAVTArray(da))
                    rv->GetCellData()->AddArray(da);
                else
                {
                    if (cd->GetScalars() == da)
                        rv->GetCellData()->SetScalars(da);
                    else if (cd->GetVectors() == da)
                        rv->GetCellData()->SetVectors(da);
                    else if (cd->GetTensors() == da)
                        rv->GetCellData()->SetTensors(da);
                    else
                        rv->GetCellData()->AddArray(da);
                }
            }
        }

        //
        // And now point data
        //
        vtkPointData *pd = ds->GetPointData();
        for (i = 0; i < pd->GetNumberOfArrays(); i++)
        {
            vtkDataArray *da = pd->GetArray(i);
            if (!IsInternalAVTArray(da) &&
                (!IsAdmissibleDataType(admissibleDataTypes, da->GetDataType()) ||
                 (!needNativePrecision && PrecisionInBytes(da) > sizeof(float))))
            {
                anyConversionNeeded = true;
                if (pass == 1)
                {
                    // look up this vtk object's "key" in GenericDb's cache
                    objectWasCachedInGenericDB[da] = 
                        gdbCache->GetVTKObjectKey(&vname, &type, &ts, dom, &mat, da);

                    vtkDataArray *newda = 0;
                    if (objectWasCachedInGenericDB[da])
                        newda = (vtkDataArray *) cache.GetVTKObject(vname, type, ts, dom, mat);
                    else
                        debug1 << "avtTransformManager: Array \"" << da->GetName() << "\" was not in generic db's cache" << endl;

                    bool needDelete = false;
                    if (!newda)
                    {
                        debug1 << "avtTransformManager: Array \"" << da->GetName() << "\" was not in tmngr's cache" << endl;
                        newda = ConvertDataArrayToFloat(da);
                        needDelete = true;
                        if (objectWasCachedInGenericDB[da])
                        {
                            cache.CacheVTKObject(vname, type, ts, dom, mat, newda);
                        }
                        else
                        {
                            debug1 << "Not caching this array" << endl;
                        }
                    }
                    if (pd->GetScalars() == da)
                        rv->GetPointData()->SetScalars(newda);
                    else if (pd->GetVectors() == da)
                        rv->GetPointData()->SetVectors(newda);
                    else if (pd->GetTensors() == da)
                        rv->GetPointData()->SetTensors(newda);
                    else
                        rv->GetPointData()->AddArray(newda);

                    if (needDelete)
                        newda->Delete();
                }
            }
            else if (pass == 1)
            {
                debug1 << "avtTransformManager: Passing along array \"" << da->GetName() << "\"" << endl;
                if (IsInternalAVTArray(da))
                    rv->GetPointData()->AddArray(da);
                else
                {
                    if (pd->GetScalars() == da)
                        rv->GetPointData()->SetScalars(da);
                    else if (pd->GetVectors() == da)
                        rv->GetPointData()->SetVectors(da);
                    else if (pd->GetTensors() == da)
                        rv->GetPointData()->SetTensors(da);
                    else
                        rv->GetPointData()->AddArray(da);
                }
            }
        }
    }

    return rv;
}


// ****************************************************************************
//  Method: CSGToDiscrete transformation
//
//  Purpose: Convert dataset and/or data arrays defined on it to from their
//  the CSG form to a discrete, unstructured mesh form.
//
//  Note: We currently do not support point data
//
//  Programmer: Mark C. Miller 
//  Creation:   December 4, 2006 
//
//  Modifications:
//
//    Mark C. Miller, Wed Dec  6 13:40:17 PST 2006
//    Fixed use of SetScalars|Vectors|Tensors and AddArray for copying
//    over various cell data arrays.
//
//    Mark C. Miller, Tue Jun 19 18:29:12 PDT 2007
//    Corrected code that puts mapped arrays onto output dataset to use
//    the SetScalars/SetVectors/SetTensors methods as in other cases
//
//    Hank Childs, Fri May  9 16:02:39 PDT 2008
//    Added argument for the domain ID to the signature.  This is needed for
//    efficiency when accessing the cache.
//
// ****************************************************************************
vtkDataSet *
avtTransformManager::CSGToDiscrete(const avtDatabaseMetaData *const md,
    const avtDataRequest_p &dataRequest, vtkDataSet *ds, int dom)
{
#ifndef PARALLEL
    const int rank = 0;
    const int nprocs = 1;
#else
    const int rank = PAR_Rank();
    const int nprocs = PAR_Size();
#endif

        if (ds->GetDataObjectType() == VTK_CSG_GRID)
        {
            //
            // look up this vtk object's "key" in GenericDb's cache
            //
            const char *vname, *type, *mat;
            int ts;
            if (!gdbCache->GetVTKObjectKey(&vname, &type, &ts, dom, &mat, ds))
            {
                EXCEPTION1(PointerNotInCacheException, ds);
            }
            int csgdom = dom, csgreg;
            md->ConvertCSGDomainToBlockAndRegion(vname, &csgdom, &csgreg);

	    debug1 << "Preparing to obtain CSG discretized grid for "
	           << ", dom=" << dom
		   << ", csgdom=" << csgdom
		   << ", csgreg=" << csgreg
		   << endl;

            //
            // See if we have discretized result in xform's cache
            //
            vtkDataSet *dgrid = (vtkDataSet *) cache.GetVTKObject(vname, type, ts, dom, mat);
            if (dgrid)
            {
                void_ref_ptr vrdataRequest = cache.GetVoidRef(vname,
                                           avtVariableCache::DATA_SPECIFICATION, ts, dom);
                avtDataRequest *olddataRequest = (avtDataRequest *) *vrdataRequest;
                if ((olddataRequest->DiscBoundaryOnly() != dataRequest->DiscBoundaryOnly()) ||
                    (olddataRequest->DiscTol() != dataRequest->DiscTol()) ||
                    (olddataRequest->FlatTol() != dataRequest->FlatTol()) ||
                    (olddataRequest->DiscMode() != dataRequest->DiscMode()))
                    dgrid = 0;
	        debug1 << "Found discretized CSG grid in cache" << endl;
            }
            if (!dgrid)
            {
	        debug1 << "No discretized CSG grid in cache. Computing a disrcetization..." << endl;

                vtkCSGGrid *csgmesh = vtkCSGGrid::SafeDownCast(ds);
                const double *bnds = csgmesh->GetBounds();
                if (dataRequest->DiscBoundaryOnly())
                {
                    dgrid = (vtkDataSet *) csgmesh->DiscretizeSurfaces(csgreg,
                                                     dataRequest->DiscTol(),
                                                     bnds[0], bnds[1], bnds[2],
                                                     bnds[3], bnds[4], bnds[5]);
                }
                else if (dataRequest->DiscMode() == 1)
                {
                    dgrid = csgmesh->DiscretizeSpace3(csgreg, rank, nprocs,
                                                     dataRequest->DiscTol(), dataRequest->FlatTol(),
                                                     bnds[0], bnds[1], bnds[2],
                                                     bnds[3], bnds[4], bnds[5]);
                }
                else
                {
                    dgrid = csgmesh->DiscretizeSpace(csgreg, dataRequest->DiscTol(),
                                                     bnds[0], bnds[1], bnds[2],
                                                     bnds[3], bnds[4], bnds[5]);
                }
                dgrid->Update();
                cache.CacheVTKObject(vname, type, ts, dom, mat, dgrid);
                dgrid->Delete();
                avtDataRequest *newdataRequest = new avtDataRequest(dataRequest);
                const void_ref_ptr vr = void_ref_ptr(newdataRequest, DestructDspec);
                cache.CacheVoidRef(vname, avtVariableCache::DATA_SPECIFICATION,
                    ts, dom, vr);
            }

            // copy same procuedure used in avtGenericDatabase to put object into
            // dataset collection
            vtkDataSet *rv = (vtkDataSet *) dgrid->NewInstance();
            rv->CopyStructure(dgrid);
            rv->GetFieldData()->ShallowCopy(dgrid->GetFieldData());

            //
            // Now handle any cell data on this mesh. CSGGrids can have only cell data
            //
            vtkCellData *cd = ds->GetCellData();
            for (int i = 0; i < cd->GetNumberOfArrays(); i++)
            {
                vtkDataArray *da = cd->GetArray(i);
                // look up this vtk object's "key" in GenericDb's cache
                if (!gdbCache->GetVTKObjectKey(&vname, &type, &ts, dom, &mat, da))
                {
                    EXCEPTION1(PointerNotInCacheException, da);
                }
                vtkDataArray *newda = (vtkDataArray *) cache.GetVTKObject(vname, type, ts, dom, mat);
                if (!newda)
                {
                    vector<int> mapvals;
                    for (int j = 0; j < rv->GetNumberOfCells(); j++)
                        mapvals.push_back(csgreg);
		    debug1 << "Mapping array \"" << da->GetName() << "\" to the discretized CSG mesh" << endl;
                    newda = BuildMappedArray(da, mapvals);
                    if (newda)
                    {
                        cache.CacheVTKObject(vname, type, ts, dom, mat, newda);
                        newda->Delete();
                    }
                }
                if (cd->GetScalars() == da)
                    rv->GetCellData()->SetScalars(newda);
                else if (cd->GetVectors() == da)
                    rv->GetCellData()->SetVectors(newda);
                else if (cd->GetTensors() == da)
                    rv->GetCellData()->SetTensors(newda);
                else
                    rv->GetCellData()->AddArray(newda);
            }
            return rv;
        }
        return ds;
}

// ****************************************************************************
//  Method: TransformMaterialDataset
//
//  Purpose: This is a special transformation to deal with material datasets
//  which can requested by VisIt via a GetMaterial or GetAuxiliaryData call.
//
//  Note: We currently do not support point data
//
//  Programmer: Mark C. Miller 
//  Creation:   December 4, 2006 
//
//  Modifications:
//
//    Hank Childs, Fri May  9 16:02:39 PDT 2008
//    Added argument for the domain ID to the signature.  This is needed for
//    efficiency when accessing the cache.
//
// ****************************************************************************
bool
avtTransformManager::TransformMaterialDataset(const avtDatabaseMetaData *const md,
    const avtDataRequest_p &dataRequest, avtMaterial **mat, int dom)
{
    const char *vname, *type;
    int ts;

    if (mat == 0 || *mat == 0)
        return false;

    // find the given material object in Generic DB's cache
    int refCount = 1;
    void_ref_ptr vr = void_ref_ptr(*mat, avtMaterial::Destruct, &refCount);
    if (!gdbCache->GetVoidRefKey(&vname, &type, &ts, dom, vr))
    {
        EXCEPTION1(PointerNotInCacheException, *vr);
    }
    
    string meshname = md->MeshForVar(vname);
    const avtMeshMetaData *mmd = md->GetMesh(meshname);
    if (mmd->meshType == AVT_CSG_MESH)
    {
        int csgdom = dom, csgreg;
        md->ConvertCSGDomainToBlockAndRegion(vname, &csgdom, &csgreg);

        // first, look for this material's mesh in xform's cache
        vtkDataSet *ds = (vtkDataSet *) cache.GetVTKObject(meshname.c_str(),
                             avtVariableCache::DATASET_NAME, ts, dom, "_all");
        if (!ds)
        {
            //
            // Here, it must be the case that the csg mesh on which this material
            // is defined has not yet been discretized. So, do it now.
            //
            ds = (vtkDataSet *) gdbCache->GetVTKObject(meshname.c_str(),
                     avtVariableCache::DATASET_NAME, ts, dom, "_all");
            if (!ds)
            {
                EXCEPTION1(PointerNotInCacheException, ds);
            }
            ds = CSGToDiscrete(md, dataRequest, ds, dom);
        }

        // see if we already have transformed avtMaterial result cached
        void_ref_ptr dvr = cache.GetVoidRef(vname, type, ts, dom);
        if (*dvr == 0)
        {
            int j, nvals = ds->GetNumberOfCells();
            vector<int> mapvals;
            for (j = 0; j < nvals; j++)
                mapvals.push_back(csgreg);

            const int *matlist = (*mat)->GetMatlist();
            int *newmatlist = BuildMappedArray(matlist, 1, mapvals);

            int nmats = (*mat)->GetNMaterials();
            int *matnos = new int[nmats];
            char **matnames = new char*[nmats];
            for (j = 0; j < nmats; j++)
            {
                matnos[j] = j;
                matnames[j] = CXX_strndup((*mat)->GetMaterials()[j].c_str(), 1024);
            }

            char dom_string[128];
            sprintf(dom_string, "Domain %d", dom);

            avtMaterial *newmat = new avtMaterial(nmats, matnos, matnames,
                                       1, &nvals, 0, newmatlist,
                                       0, 0, 0, 0, 0, dom_string);

            delete [] matnos;
            for (j = 0; j < nmats; j++)
                free(matnames[j]);

            if (newmat)
            {
                void_ref_ptr newvr = void_ref_ptr(newmat, avtMaterial::Destruct);
                cache.CacheVoidRef(vname, type, ts, dom, newvr);
                *mat = newmat; // old mat was already cached in Generic DB's cache
                return true;
            }
        }
        else
        {
            *mat = (avtMaterial*) *dvr;
            return true;
        }
    }

    return false;
}

// ****************************************************************************
//  Method: AddVertexCellsToPointsOnlyDataset
//
//  Purpose: Add VTK_VERTEX cells to a vtkPoints dataset that has only points
//  and no cells.
//
//  Programmer: Mark C. Miller, Wed Jan  7 09:35:22 PST 2009
//
//  Modifications:
//    Mark C. Miller, Tue Jan 13 10:50:19 PST 2009
//    Added logic to examine cell/pt data arrays too.
// ****************************************************************************

vtkDataSet *
avtTransformManager::AddVertexCellsToPointsOnlyDataset(vtkDataSet *ds)
{
    int i, doType = ds->GetDataObjectType();

    if (doType != VTK_POLY_DATA && 
        doType != VTK_UNSTRUCTURED_GRID)
        return ds; // no-op

    if (ds->GetNumberOfPoints() == 0)
        return ds; // no-op

    if (ds->GetNumberOfCells() != 0)
        return ds; // no-op

    if ((ds->GetCellData() == 0 || ds->GetCellData()->GetNumberOfArrays() == 0) &&
        (ds->GetPointData() == 0 || ds->GetPointData()->GetNumberOfArrays() == 0))
        return ds; // no-op

    bool hasEmptyCellDataArrays = true;
    for (i = 0; i < ds->GetCellData()->GetNumberOfArrays(); i++)
    {
        if (ds->GetCellData()->GetArray(i)->GetNumberOfTuples() == ds->GetNumberOfPoints())
        {
            hasEmptyCellDataArrays = false;
            break;
        }
    }
    bool hasEmptyPointDataArrays = true;
    for (i = 0; i < ds->GetPointData()->GetNumberOfArrays(); i++)
    {
        if (ds->GetPointData()->GetArray(i)->GetNumberOfTuples() == ds->GetNumberOfPoints())
        {
            hasEmptyPointDataArrays = false;
            break;
        }
    }
    if (hasEmptyCellDataArrays && hasEmptyPointDataArrays)
        return ds; // no-op

    debug1 << "avtTransformManager: Adding " << ds->GetNumberOfPoints() << " VTK_VERTEX cells" << endl;
    debug1 << "to a dataset that consists solely of points but no cells." << endl;

    //
    // The only way to arrive here is if we have non-zero number of points
    // but zero cells. So, we can add vertex cells.
    //
    vtkIdType onevertex[1];
    if (doType == VTK_POLY_DATA)
    {
        vtkPolyData *pd = vtkPolyData::SafeDownCast(ds);
        pd->Allocate(ds->GetNumberOfPoints());
        for (i = 0; i < ds->GetNumberOfPoints(); i++)
        {
            onevertex[0] = i;
            pd->InsertNextCell(VTK_VERTEX, 1, onevertex);
        }

    }
    else // must be VTK_UNSTRUCTURED_GRID
    {
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(ds);
        ugrid->Allocate(ds->GetNumberOfPoints());
        for (i = 0; i < ds->GetNumberOfPoints(); i++)
        {
            onevertex[0] = i;
            ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
        }
    }

    //
    // Note, we're returning the original pointer here. So, transform manager
    // will NOT think we've done any work on it. In fact, we have 'transformed'
    // it in place. This should be harmless.
    //
    return ds;
}

// ****************************************************************************
//  Method: TransformDataset
//
//  Purpose: This is a the main entry point to transformation manager to
//  transform all datasets in the dataset collection passed into it.
//
//  Note: We currently catch and then give-up cases where we're asked to
//  transform data that is not already in generic db's cache. Some plugins
//  DO NOT permit generic db to cache their data. So, those plugins will
//  NOT be able to serve up data to VisIt requiring transformation. This
//  restriction can and probably should be relaxed by re-writing the
//  transformations here to, when data is not cached, simply convert it
//  anyway and then NOT attempt to cache it in transform manager's cache
//  either.
//
//  Programmer: Mark C. Miller 
//  Creation:   December 4, 2006 
//
//  Modifications:
//
//    Hank Childs, Fri May  9 16:01:45 PDT 2008
//    Pass in domain IDs to transforming functions, because they are needed
//    to efficiently access cache.
//
// ****************************************************************************
bool
avtTransformManager::TransformDataset(avtDatasetCollection &dsc,
    intVector &domains, avtDataRequest_p &d_spec,
    avtSourceFromDatabase *src, boolVector &selectionsApplied,
    avtDatabaseMetaData *md)
{
    const char progressString[256] = "Transforming input data";
    bool transformsApplied = false;
    src->DatabaseProgress(0, 0, progressString);
    for (int i = 0; i < dsc.GetNDomains(); i++)
    {
        vtkDataSet *dsOrig, *ds;
        dsOrig = ds = dsc.GetDataset(i, 0);
        src->DatabaseProgress(i, dsc.GetNDomains(), progressString);

        if (!ds) continue;

        TRY
        {
            ds = CSGToDiscrete(md, d_spec, ds, domains[i]);

            // Handle vtkPoints datasets that have points but no cells
            ds = AddVertexCellsToPointsOnlyDataset(ds);

            //ds = HangingToConforming(md, d_spec, ds);

            // old way of "dealing" with polyhedral was to eliminate it
            //ds = ElminatePolyhedral(md, d_spec, ds);

            //ds = PolyhedralToZoo(md, d_spec, ds);

            ds = NativeToFloat(md, d_spec, ds, domains[i]);

            if (ds != dsOrig)
            {
                transformsApplied = true;
                dsc.SetDataset(i, 0, ds);
                ds->Delete();
            }
        }
        CATCH(PointerNotInCacheException)
        {
            ; // do nothing
        }
        ENDTRY
    }

    src->DatabaseProgress(1, 0, progressString);
    return transformsApplied;
}
