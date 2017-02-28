/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
//                           avtTransformManager.C                           //
// ************************************************************************* //
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCSGGrid.h>
#include <vtkCharArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkShortArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkInformation.h>
#include <vtkIntArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkLongArray.h>
#include <vtkLongLongArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkUnsignedLongLongArray.h>
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
#include <vtkStructuredGrid.h>

#include <vtkVisItUtility.h>

#include <avtCallback.h>
#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtDatabaseMetaData.h>
#include <avtDatasetCollection.h>
#include <avtParallel.h>
#include <avtScalarMetaData.h>
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

#if defined (_MSC_VER) && (_MSC_VER < 1800) && !defined(round)
inline double round(double x) {return (x-floor(x)) > 0.5 ? ceil(x) : floor(x);}
#endif

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
//    Mark C. Miller, Tue Jul 20 19:21:34 PDT 2010
//    Added support for LONG LONG types.
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
        case VTK_LONG_LONG:      return "long long";
        case VTK_UNSIGNED_LONG:  return "unsigned long";
        case VTK_UNSIGNED_LONG_LONG:  return "unsigned long long";
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
//    Mark C. Miller, Tue Jul 20 19:21:34 PDT 2010
//    Added support for LONG LONG types.
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
        case VTK_LONG_LONG:      return sizeof(long long);
        case VTK_UNSIGNED_LONG_LONG:  return sizeof(unsigned long long);
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
    for (size_t i = 0; i < admissibleTypes.size(); i++)
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
//    Gunther H. Weber, Thu Nov  8 10:20:08 PST 2012
//    Use size_t in loop instead of int
//
// ****************************************************************************
template <class oT, class iT>
static void ConvertToType(oT *obuf, const iT* ibuf, size_t n)
{
    for (size_t i = 0; i < n; i++)
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
//    Mark C. Miller, Tue Jul 20 19:21:34 PDT 2010
//    Added support for LONG LONG types.
//
//    Gunther H. Weber, Thu Nov  8 10:20:32 PST 2012
//    Use size_t instead of int
//
//    Brad Whitlock, Thu Jul 23 16:01:46 PDT 2015
//    Support for non-standard memory layout. Use vtkTemplateMacro.
//
// ****************************************************************************

static vtkDataArray * 
ConvertDataArrayToFloat(vtkDataArray *oldArr)
{
    vtkDataArray *newArr = 0; 

    if (oldArr->GetDataType() != VTK_FLOAT)
    {
        newArr = vtkFloatArray::New();

        vtkIdType numTuples = oldArr->GetNumberOfTuples();
        int numComponents = oldArr->GetNumberOfComponents();

        newArr->SetNumberOfComponents(numComponents);
        newArr->SetNumberOfTuples(numTuples);

        debug1 << "avtTransformManager: Converting vktDataArray, ";
        if (oldArr->GetName() != NULL) 
        {
               debug1 << "\"" << oldArr->GetName() << "\", ";
        }
        debug1 << "with " << numTuples << " tuples and "
               << numComponents << " components from type \""
               << DataArrayTypeName(oldArr) << "\" to \"float\"" << endl;

        size_t numValues = numTuples * numComponents;
        if(oldArr->HasStandardMemoryLayout())
        {
            float *newBuf = (float*) newArr->GetVoidPointer(0);
            void *oldBuf = oldArr->GetVoidPointer(0);
            switch (oldArr->GetDataType())
            {
            vtkTemplateMacro(
                ConvertToType(newBuf, (VTK_TT *) oldBuf, numValues);
            );
            }
        }
        else
        {
            vtkIdType nTuples = oldArr->GetNumberOfTuples();
            for (vtkIdType i = 0; i < nTuples; i++)
                newArr->SetTuple(i, oldArr->GetTuple(i));
        }
    }

    if (newArr)
        newArr->SetName(oldArr->GetName());

    vtkInformation* info = oldArr->GetInformation();
    if (info && info->Has(avtVariableCache::OFFSET_3())) 
    {
        double* vals = info->Get(avtVariableCache::OFFSET_3());
        vtkInformation* newInfo = newArr->GetInformation();
        newInfo->Set(avtVariableCache::OFFSET_3(), vals[0], vals[1], vals[2]);
    }

    return newArr;
}


// ****************************************************************************
//  Function: ConvertDataArrayToDouble
//
//  Purpose: Given a vtk data array, make a copy of it that is converted
//  to double
//
//  Notes:  copied form ConvertDataArrayToFloat
//
//  Programmer: Kathleen Biagas
//  Creation:   July 29, 2013
//
//  Modifications:
//
// ****************************************************************************

static vtkDataArray *
ConvertDataArrayToDouble(vtkDataArray *oldArr)
{
    vtkDataArray *newArr = 0;

    if (oldArr->GetDataType() != VTK_DOUBLE)
    {
        newArr = vtkDoubleArray::New();

        vtkIdType numTuples = oldArr->GetNumberOfTuples();
        int numComponents = oldArr->GetNumberOfComponents();

        newArr->SetNumberOfComponents(numComponents);
        newArr->SetNumberOfTuples(numTuples);

        double *newBuf = (double*) newArr->GetVoidPointer(0);
        void *oldBuf = oldArr->GetVoidPointer(0);

        debug1 << "avtTransformManager: Converting vktDataArray, ";
        if (oldArr->GetName() != NULL) 
        {
               debug1 << "\"" << oldArr->GetName() << "\", ";
        }
        debug1 << "with " << numTuples << " tuples and "
               << numComponents << " components from type \""
               << DataArrayTypeName(oldArr) << "\" to \"double\"" << endl;

        size_t numValues = numTuples * numComponents;
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
            case VTK_LONG_LONG:
                ConvertToType(newBuf, (long long*) oldBuf, numValues);
                break;
            case VTK_UNSIGNED_LONG:
                ConvertToType(newBuf, (unsigned long*) oldBuf, numValues);
                break;
            case VTK_UNSIGNED_LONG_LONG:
                ConvertToType(newBuf, (unsigned long long*) oldBuf, numValues);
                break;
            case VTK_FLOAT:
                ConvertToType(newBuf, (float*) oldBuf, numValues);
                break;
            case VTK_ID_TYPE:
                ConvertToType(newBuf, (vtkIdType*) oldBuf, numValues);
                break;
            default:
                {
                    char msg[256];
                    SNPRINTF(msg, sizeof(msg),
                        "Cannot convert from type \"%s\" to double",
                        DataArrayTypeName(oldArr));
                    EXCEPTION1(ImproperUseException, msg); 
                }
        }
    }

    if (newArr)
        newArr->SetName(oldArr->GetName());
  
    vtkInformation* info = oldArr->GetInformation();
    if (info && info->Has(avtVariableCache::OFFSET_3())) 
    {
        double* vals = info->Get(avtVariableCache::OFFSET_3());
        vtkInformation* newInfo = newArr->GetInformation();
        newInfo->Set(avtVariableCache::OFFSET_3(), vals[0], vals[1], vals[2]);
    }
  
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
//    Kathleen Biagas, Wed Aug  7 12:47:47 PDT 2013
//    Added toFloat argument.
//
// ****************************************************************************
static vtkDataSet * 
ConvertDataSet(vtkDataSet *oldds, bool toFloat)
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
                    vtkDataArray *newArr = NULL;
                    if (toFloat)
                        newArr = ConvertDataArrayToFloat(oldArr);
                    else
                        newArr = ConvertDataArrayToDouble(oldArr);

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
                    vtkDataArray *newX = NULL;
                    vtkDataArray *oldY = oldrg->GetYCoordinates();
                    vtkDataArray *newY = NULL;
                    vtkDataArray *oldZ = oldrg->GetZCoordinates();
                    vtkDataArray *newZ = NULL;
                    if (toFloat)
                    {
                        newX = ConvertDataArrayToFloat(oldX);
                        newY = ConvertDataArrayToFloat(oldY);
                        newZ = ConvertDataArrayToFloat(oldZ);
                    }
                    else
                    {
                        newX = ConvertDataArrayToDouble(oldX);
                        newY = ConvertDataArrayToDouble(oldY);
                        newZ = ConvertDataArrayToDouble(oldZ);
                    }

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
//  Template: BuildMappedArray
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
    size_t nvals = valsToMap.size();

    // we use malloc here because we'll assign the data to vtkDataArray using
    // SetVoidArray and, upon deletion, that assumes malloc was used
    iT *rbuf = (iT *) malloc(nvals * ncomps * sizeof(iT));
    for (size_t i = 0; i < nvals; i++)
    {
        for (int j = 0; j < ncomps; j++)
            rbuf[i*ncomps+j] = ibuf[valsToMap[i]*ncomps+j];
    }
    return rbuf;
}

// ****************************************************************************
//  Template: BuildMappedArray
//
//  Purpose: Build a mapping array to handle changes in zone/node numbering
//  and order when dealing with variables
//
//  Programmer: Mark C. Miller 
//  Creation:   December 4, 2006 
//
//  Modifications:
//
//    Mark C. Miller, Tue Jul 20 19:21:34 PDT 2010
//    Added support for LONG LONG types.
// ****************************************************************************
static vtkDataArray *
BuildMappedArray(vtkDataArray *da, const vector<int> &valsToMap)
{
    const int nvals = static_cast<int>(valsToMap.size());
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
        case VTK_LONG_LONG:
            rv = vtkLongLongArray::New();
            rbuf = BuildMappedArray((long long*) buf, ncomps, valsToMap);
            break;
        case VTK_UNSIGNED_LONG:
            rv = vtkUnsignedLongArray::New();
            rbuf = BuildMappedArray((unsigned long*) buf, ncomps, valsToMap);
            break;
        case VTK_UNSIGNED_LONG_LONG:
            rv = vtkUnsignedLongLongArray::New();
            rbuf = BuildMappedArray((unsigned long long*) buf, ncomps, valsToMap);
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
//  Function: ShouldIgnoreVariableForConversions
//
//  Purpose: Filter certain internal AVT arrays that should not undergo
//  conversion 
//
//  Programmer: Mark C. Miller 
//  Creation:   December 5, 2006 
//
//    Mark C. Miller, Fri Apr 13 10:32:47 PDT 2012
//    Renamed to serve as a more generalized filter for ignoring variables
//    for various reasons.
// ****************************************************************************
static bool
ShouldIgnoreVariableForConversions(vtkDataArray *da,
    const avtDatabaseMetaData *const md,
    const avtDataRequest_p &dataRequest)
{
    bool ignoreIt = false;

    if (!ignoreIt && strncmp(da->GetName(), "avt", 3) == 0)
        ignoreIt = true;

    if (!ignoreIt && da->GetDataType() == VTK_BIT)
    {
        const avtScalarMetaData *smd = md->GetScalar(da->GetName());
        if (smd && smd->GetEnumerationType() == avtScalarMetaData::ByBitMask)
            ignoreIt = true;
    }

    if (ignoreIt) 
    {
        debug4 << "Ignoring variable/array \"" << da->GetName()
               << "\" for type conversions" << endl;
    }

    return ignoreIt;
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
    // Take care to clear any of the funky timesteps that might have been
    // introduced to handle things we want to cache ACROSS timesteps. See,
    // for example, CSGToDiscrete().
    cache.ClearTimestep(-1);
}

void
avtTransformManager::FreeUpResources(int lastts)
{
    cache.ClearTimestep(lastts);
}

// ****************************************************************************
// Method: avtTransformManager::CoordinatesHaveExcessPrecision
//
// Purpose: 
//   Determines whether the input dataset's coordinates have excess precision
//   that we can forfeit in the interest of using float for lower memory usage.
//
// Arguments:
//   ds                  : The input dataset.
//   needNativePrecision : Whether the contract says we need native precision.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Sat Apr 21 23:53:45 PDT 2012
//
// Modifications:
//    Kathleen Biagas, Wed Aug  7 12:48:50 PDT 2013
//    Take into consideration the precision set by user, as specifed in
//    avtDatabaseFactory.
//
// ****************************************************************************

bool
avtTransformManager::CoordinatesHaveExcessPrecision(vtkDataSet *ds, 
    bool needNativePrecision) const
{
    bool excessPrecision;
    avtPrecisionType pType = avtDatabaseFactory::GetPrecisionType();
    if(pType == AVT_PRECISION_DOUBLE)
        excessPrecision = false;
    else
    {
        // The transform manager makes a decision here that since we don't
        // need native precision (according to the contract) that it's okay
        // to lose some precision.
        excessPrecision = !needNativePrecision  &&
            pType != AVT_PRECISION_NATIVE &&
            ((size_t)PrecisionInBytes(GetCoordDataType(ds)) > sizeof(float));
    }

    return excessPrecision;
}


// ****************************************************************************
// Method: avtTransformManager::CoordinatesHaveInsufficientPrecision
//
// Purpose:
//   Determines whether the input dataset's coordinates have insufficient
//   precision based on user request.
//
// Arguments:
//   ds                  : The input dataset.
//   needNativePrecision : Whether the contract says we need native precision.
//
// Returns:
//
// Note:
//
// Programmer: Kathleen Biagas
// Creation:   July 29, 2013
//
// Modifications:
//
// ****************************************************************************

bool
avtTransformManager::CoordinatesHaveInsufficientPrecision(vtkDataSet *ds,
    bool needNativePrecision) const
{
    bool insufficientPrecision;
    avtPrecisionType pType = avtDatabaseFactory::GetPrecisionType();
    if(pType == AVT_PRECISION_FLOAT)
        insufficientPrecision = false;
    else
    {
        // The transform manager makes a decision here that since we don't
        // need native precision (according to the contract) that it's okay
        // to increase precision.
        insufficientPrecision = !needNativePrecision &&
            pType != AVT_PRECISION_NATIVE  &&
            ((size_t)PrecisionInBytes(GetCoordDataType(ds)) < sizeof(double));
    }

    return insufficientPrecision;
}

// ****************************************************************************
// Method: avtTransformManager::DataHasExcessPrecision
//
// Purpose:
//   Determines whether the input data array has excess precision that we can
//   forfeit in the interest of using float for lower memory usage.
//
// Arguments:
//   da                  : The input data array.
//   needNativePrecision : Whether the contract says we need native precision.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Sat Apr 21 23:53:45 PDT 2012
//
// Modifications:
//    Kathleen Biagas, Wed Aug  7 12:48:50 PDT 2013
//    Take into consideration the precision set by user, as specifed in
//    avtDatabaseFactory.
//
// ****************************************************************************

bool
avtTransformManager::DataHasExcessPrecision(vtkDataArray *da,
    bool needNativePrecision) const
{
    bool excessPrecision;
    avtPrecisionType pType = avtDatabaseFactory::GetPrecisionType();
    if(pType == AVT_PRECISION_DOUBLE)
        excessPrecision = false;
    else
    {
        // The transform manager makes a decision here that since we don't
        // need native precision (according to the contract) that it's okay
        // to lose some precision.
        excessPrecision = !needNativePrecision && 
            pType != AVT_PRECISION_NATIVE &&
            ((size_t)PrecisionInBytes(da) > sizeof(float));
    }

    return excessPrecision;
}


// ****************************************************************************
// Method: avtTransformManager::DataHasInsufficientPrecision
//
// Purpose:
//   Determines whether the input data array has insufficient precision.
//
// Arguments:
//   da                  : The input data array.
//   needNativePrecision : Whether the contract says we need native precision.
//
// Returns:
//
// Note:
//
// Programmer: Kathleen Biagas
// Creation:   July 29, 2013
//
// Modifications:
//
// ****************************************************************************

bool
avtTransformManager::DataHasInsufficientPrecision(vtkDataArray *da,
    bool needNativePrecision) const
{
    bool insufficientPrecision;
    avtPrecisionType pType = avtDatabaseFactory::GetPrecisionType();
    if(pType == AVT_PRECISION_FLOAT)
        insufficientPrecision = false;
    else
    {
        // The transform manager makes a decision here that since we don't
        // need native precision (according to the contract) that it's okay
        // to lose some precision.
        insufficientPrecision = !needNativePrecision &&
             pType != AVT_PRECISION_NATIVE &&
            ((size_t)PrecisionInBytes(da) < sizeof(double));
    }

    return insufficientPrecision;
}

// ****************************************************************************
//  Method: NativeToFloat transformation
//
//  Purpose: Convert dataset and/or data arrays defined on it to from their
//  native type to float or double
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
//    Brad Whitlock, Sun Apr 22 00:02:42 PDT 2012
//    Call some helper functions to help determine whether there is 
//    excess precision. Print the reason for conversion to the logs.
//
//    Kathleen Biagas, Wed Aug  7 12:51:33 PDT 2013
//    Added 'InsufficientPrecision' methods that help determine if conversion
//    to double should be utilized.
//
// ****************************************************************************
//#define DEBUG_DOUBLE_PIPELINE
vtkDataSet *
avtTransformManager::NativeToFloat(const avtDatabaseMetaData *const md,
    const avtDataRequest_p &dataRequest, vtkDataSet *ds, int dom)
{
    const char *mName = "avtTransformManager::NativeToFloat: ";

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
    debug4 << mName << "needNativePrecision=" << (needNativePrecision?"true":"false")
           << ", admissibleDataTypes={";
    bool doubleAllowed = false;
    for(size_t q = 0; q < admissibleDataTypes.size(); ++q)
    {
        switch (admissibleDataTypes[q])
        {
        case VTK_CHAR:               debug4 << "VTK_CHAR"; break;
        case VTK_UNSIGNED_CHAR:      debug4 << "VTK_UNSIGNED_CHAR"; break;
        case VTK_SHORT:              debug4 << "VTK_SHORT"; break;
        case VTK_UNSIGNED_SHORT:     debug4 << "VTK_UNSIGNED_SHORT"; break;
        case VTK_INT:                debug4 << "VTK_INT"; break;
        case VTK_UNSIGNED_INT:       debug4 << "VTK_UNSIGNED_INT"; break;
        case VTK_LONG:               debug4 << "VTK_LONG"; break;
        case VTK_LONG_LONG:          debug4 << "VTK_LONG_LONG"; break;
        case VTK_UNSIGNED_LONG:      debug4 << "VTK_UNSIGNED_LONG"; break;
        case VTK_UNSIGNED_LONG_LONG: debug4 << "VTK_UNSIGNED_LONG_LONG"; break;
        case VTK_FLOAT:              debug4 << "VTK_FLOAT"; break;
        case VTK_DOUBLE:             debug4 << "VTK_DOUBLE"; doubleAllowed = true; break;
        case VTK_ID_TYPE:            debug4 << "VTK_ID_TYPE"; break;
        default:                     debug4 << admissibleDataTypes[q]; break;                
        }
        debug4 << ",";
    }
    debug4 << "}" << endl;

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
            debug1 << mName << "Applying transform" << endl;
        }

        //
        // Deal with mesh first 
        //
        bool disallowedType =
             !IsAdmissibleDataType(admissibleDataTypes, GetCoordDataType(ds));
        bool excessPrecision =
             CoordinatesHaveExcessPrecision(ds, needNativePrecision);
        bool insufficientPrecision =
             CoordinatesHaveInsufficientPrecision(ds, needNativePrecision);

#ifndef DEBUG_DOUBLE_PIPELINE
        if (insufficientPrecision && !doubleAllowed)
        {
            debug1 << "User requested increased precision, but the pipeline has disallowed "
                   << "VTK_DOUBLE" << endl;
            insufficientPrecision = false;
        }
#endif

        if(disallowedType || excessPrecision || insufficientPrecision)
        {
            anyConversionNeeded = true;
            if (pass == 1)
            {
                if(disallowedType)
                {
                    debug1 << mName << "Convert coordinates due to disallowed"
                           << " type." << endl;
                }
                if(excessPrecision)
                {
                    debug1 << mName << "Convert coordinates due to excess "
                           << "precision." << endl;
                }
                if(insufficientPrecision)
                {
                    debug1 << mName << "Convert coordinates due to "
                           << "insufficient precision." << endl;
                }

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
                    debug1 << mName << "dataset is not in generic db's cache" << endl;
                    rv = 0;
                }

                bool needDelete = false;
                if (!rv)
                {
                    if (disallowedType || excessPrecision)
                    {
                        debug1 << mName << "Converting data set from native"
                               << " to float" << endl;
                        rv = ConvertDataSet(ds, true);
                    }
                    else
                    {
                        debug1 << mName << "Converting data set from native"
                               << " to double" << endl;
                        rv = ConvertDataSet(ds, false);
                    }
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
            bool eligible = !ShouldIgnoreVariableForConversions(da, md, dataRequest);
            disallowedType = eligible &&
                !IsAdmissibleDataType(admissibleDataTypes, da->GetDataType());
            excessPrecision = eligible &&
                DataHasExcessPrecision(da, needNativePrecision);
            insufficientPrecision = eligible &&
                DataHasInsufficientPrecision(da, needNativePrecision);
#ifndef DEBUG_DOUBLE_PIPELINE
            if (insufficientPrecision && !doubleAllowed)
            {
                debug1 << "User requested increased precision, but the pipeline has disallowed"
                       << " VTK_DOUBLE" << endl;
                insufficientPrecision = false;
            }
#endif
            bool ignore = false;
            if (insufficientPrecision)
            {
                avtVarType vt = md->DetermineVarType(da->GetName());
                ignore = (vt == AVT_MATERIAL || vt == AVT_MATSPECIES);
                if (ignore) 
                {
                    debug4 << "Conversion to double ignored for " << "da->GetName()" << endl;
                }
            }
            if(!ignore && (disallowedType || excessPrecision || insufficientPrecision))
            {
                anyConversionNeeded = true;
                if (pass == 1)
                {
                    if(disallowedType)
                    {
                        debug1 << mName << "Convert \"" << da->GetName()
                               << "\" array due to disallowed type." << endl;
                    }
                    if(excessPrecision)
                    {
                        debug1 << mName << "Convert \"" << da->GetName()
                               << "\" array due to excess precision." << endl;
                    }
                    if(insufficientPrecision)
                    {
                        debug1 << mName << "Convert \"" << da->GetName()
                               << "\" array due to insufficient precision." << endl;
                    }

                    // look up this vtk object's "key" in GenericDb's cache
                    objectWasCachedInGenericDB[da] =
                        gdbCache->GetVTKObjectKey(&vname, &type, &ts, dom, &mat, da);

                    vtkDataArray *newda = 0;
                    if (objectWasCachedInGenericDB[da])
                    {
                        newda = (vtkDataArray *) cache.GetVTKObject(vname, type, ts, dom, mat);
                    }
                    else
                    {
                        debug1 << mName << "Array \"" << da->GetName()
                               << "\" was not in generic db's cache" << endl;
                    }

                    bool needDelete = false;
                    if (!newda)
                    {
                        debug1 << mName << "Array \"" << da->GetName()
                               << "\" was not in tmngr's cache" << endl;
                        if (disallowedType || excessPrecision)
                        {
                            newda = ConvertDataArrayToFloat(da);
                        }
                        else // insufficientPrecision
                        {
                            newda = ConvertDataArrayToDouble(da);
                        }
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
                debug1 << mName << "Passing along array \"" << da->GetName() << "\"" << endl;
                if (ShouldIgnoreVariableForConversions(da, md, dataRequest))
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
            bool eligible = !ShouldIgnoreVariableForConversions(da, md, dataRequest);
            disallowedType = eligible &&
                !IsAdmissibleDataType(admissibleDataTypes, da->GetDataType());
            excessPrecision = eligible &&
                DataHasExcessPrecision(da, needNativePrecision);
            insufficientPrecision = eligible &&
                DataHasInsufficientPrecision(da, needNativePrecision);
#ifndef DEBUG_DOUBLE_PIPELINE
            if (insufficientPrecision && !doubleAllowed)
            {
                debug1 << "User requested increased precision, but the pipeline has "
                       << "disallowed VTK_DOUBLE" << endl;
                insufficientPrecision = false;
            }
#endif
            if(disallowedType || excessPrecision || insufficientPrecision)
            {
                anyConversionNeeded = true;
                if (pass == 1)
                {
                    if(disallowedType)
                    {
                        debug1 << mName << "Convert \"" << da->GetName()
                               << "\" array due to disallowed type." << endl;
                    }
                    if(excessPrecision)
                    {
                        debug1 << mName << "Convert \"" << da->GetName()
                               << "\" array due to excess precision." << endl;
                    }
                    if(insufficientPrecision)
                    {
                        debug1 << mName << "Convert \"" << da->GetName()
                               << "\" array due to insufficient precision." << endl;
                    }

                    // look up this vtk object's "key" in GenericDb's cache
                    objectWasCachedInGenericDB[da] = 
                        gdbCache->GetVTKObjectKey(&vname, &type, &ts, dom, &mat, da);

                    vtkDataArray *newda = 0;
                    if (objectWasCachedInGenericDB[da])
                        newda = (vtkDataArray *)
                              cache.GetVTKObject(vname, type, ts, dom, mat);
                    else
                    {
                        debug1 << mName << "Array \"" << da->GetName()
                               << "\" was not in generic db's cache" << endl;
                    }

                    bool needDelete = false;
                    if (!newda)
                    {
                        debug1 << mName << "Array \"" << da->GetName()
                               << "\" was not in tmngr's cache" << endl;
                        if (disallowedType || excessPrecision)
                        {
                            newda = ConvertDataArrayToFloat(da);
                        }
                        else // insufficientPrecision
                        {
                            newda = ConvertDataArrayToDouble(da);
                        }
                        needDelete = true;
                        if (objectWasCachedInGenericDB[da])
                        {
                            cache.CacheVTKObject(vname, type, ts, dom, mat, newda);
                        }
                        else
                        {
                            debug1 << mName << "Not caching this array" << endl;
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
                debug1 << mName << "Passing along array \"" << da->GetName() << "\"" << endl;
                if (ShouldIgnoreVariableForConversions(da, md, dataRequest))
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
//  Method: FindMatchingCSGDiscretization 
//
//  Purpose: Search all timesteps in cache for matching discretization.
//
//  Programmer: Mark C. Miller 
//  Creation:   Friday, February 13, 2009 
//
//  Modifications:
//    Eric Brugger, Wed Nov 19 08:46:49 PST 2014
//    I reduced the number of reads of CSG meshes to only once per CSG mesh
//    instead of once per region in order to reduce the number of times the
//    same CSG mesh was cached. Typically there is one CSG mesh with many
//    regions, so this is a significant saving. CSG meshes with thousands
//    of regions were exhausting memory in the previous scheme.
//
// ****************************************************************************

vtkDataSet *
avtTransformManager::FindMatchingCSGDiscretization(
    const avtDatabaseMetaData *const md,
    const avtDataRequest_p &dataRequest,
    const char *vname, const char *type,
    int ts, int csgdom, int dom, const char *mat)
{
    vtkCSGGrid *curTsDs = (vtkCSGGrid *) gdbCache->GetVTKObject(vname, type, ts, csgdom, mat);
    if (curTsDs == 0)
        return 0;

    // compare the CSGGrid object as read from the format to the
    // current object
    vtkCSGGrid *oldTsDs = (vtkCSGGrid *) cache.GetVTKObject(vname, type, -1, csgdom, mat);
    if (oldTsDs && *oldTsDs == *curTsDs)
    {
        // compare the discretization parameters
        void_ref_ptr oldVrDr = cache.GetVoidRef(vname,
                               avtVariableCache::DATA_SPECIFICATION, -1, csgdom);
        avtDataRequest *oldDr = (avtDataRequest *) *oldVrDr;
        if ((oldDr->DiscBoundaryOnly() == dataRequest->DiscBoundaryOnly()) &&
            (oldDr->DiscTol() == dataRequest->DiscTol()) &&
            (oldDr->FlatTol() == dataRequest->FlatTol()) && 
            (oldDr->DiscMode() == dataRequest->DiscMode()))
        {
            //
            // If we arrive here, we know that the CSGGrid objects
            // are the same as well as the discretization parameters
            //
            debug1 << "For CSGGrid object \"" << vname << "\"(ts=" << ts 
                   << "), found matching discretization at (ts=" << -1 << ")." << endl;
            return (vtkDataSet*) cache.GetVTKObject(vname, "DISCRETIZED_CSG", -1, dom, mat);
        }
    }
    return 0;
}

// ****************************************************************************
//  Method: ComputeCellSize
//
//  Purpose:
//    Calculate the size of a cell in the mesh based on the tolerance and
//    the longest dimension.
//
//  Programmer: Eric Brugger
//  Creation:   July 25, 2012 
//
//  Modifications:
//
// ****************************************************************************
static
double ComputeCellSize(double tol,
                       double minX, double maxX,
                       double minY, double maxY,
                       double minZ, double maxZ)
{
    //
    // Determine the size of a cell as a fraction of the longest dimension.
    //
    double cellSize;

    if ((maxX - minX) > (maxY - minY))
    {
        if ((maxX - minX) > (maxZ - minZ))
            cellSize = tol * (maxX - minX);
        else
            cellSize = tol * (maxZ - minZ);
    }
    else
    {
        if ((maxY - minY) > (maxZ - minZ))
            cellSize = tol * (maxY - minY);
        else
            cellSize = tol * (maxZ - minZ);
    }

    return cellSize;
}

// ****************************************************************************
//  Method: avtTransformManager::CSGToDiscrete
//
//  Purpose:
//    Convert dataset and/or data arrays defined on it to from their
//    the CSG form to a discrete, unstructured mesh form.
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
//    Mark C. Miller, Fri Feb 13 17:07:38 PST 2009
//    Added call to find matching CSG discretization. Made it smarter about
//    caching discretizations to reduce frequency of cases where a CSG mesh
//    that does not vary with time is re-discretized each timestep.
//
//    Jeremy Meredith, Fri Feb 26 11:43:27 EST 2010
//    Added multi-pass discretization mode.  Requires it to be able to
//    store off the split-up mesh between domains, so we use the cache
//    for it and store it with a -1 domain.  This mode might fail
//    (since it's restricted to a fixed-length bitfield for the boundaries)
//    in which case we fall back to the Uniform algorithm.
//
//    Eric Brugger, Wed Jul 25 09:02:59 PDT 2012
//    I modified the multi-pass discretization of CSG meshes to only process
//    a portion of the mesh on each processor instead of the entire mesh.
//
//    Eric Brugger, Wed Apr  2 12:12:49 PDT 2014
//    I modified the multi-pass discretization of CSG meshes to process
//    each domain independently if the number total number of boundary
//    surfaces is above the internal limit.
//
//    Eric Brugger, Wed Aug 20 14:17:49 PDT 2014
//    I corrected a bug in the calculation of the subregion index extents
//    where it would calculate degenerate regions when the number of elements
//    per block was small.
//
//    Eric Brugger, Wed Nov 19 08:46:49 PST 2014
//    I reduced the number of reads of CSG meshes to only once per CSG mesh
//    instead of once per region in order to reduce the number of times the
//    same CSG mesh was cached. Typically there is one CSG mesh with many
//    regions, so this is a significant saving. CSG meshes with thousands
//    of regions were exhausting memory in the previous scheme.
//
//    Eric Brugger, Fri Nov 21 15:36:42 PST 2014
//    I added code to try the uniform discretization method if the multi
//    pass method failed. I also added code to return an empty mesh if we
//    were unable to discretize the mesh.
//
//    Eric Brugger, Mon Nov 24 16:04:47 PST 2014
//    I added an argument to vtkCSGGrid::DoDiscretizationMultiPass that
//    specifies if all the regions should be discretized at once. I am
//    passing a hard coded false since it gives better performance for
//    complex CSG meshes. In the future this will be user controllable.
//
// ****************************************************************************
vtkDataSet *
avtTransformManager::CSGToDiscrete(avtDatabaseMetaData *md,
    const avtDataRequest_p &dataRequest, vtkDataSet *ds, int dom)
{
#ifndef PARALLEL
    const int rank = 0;
    const int nprocs = 1;
#else
    const int rank = PAR_Rank();
    const int nprocs = PAR_Size();
#endif

    if (ds->GetDataObjectType() != VTK_CSG_GRID)
        return ds;

    //
    // look up this vtk object's "key" in GenericDb's cache
    //
    const char *vname, *type, *mat;
    int ts;

    vname = dataRequest->GetVariable();
    int csgdom = dom, csgreg;
    md->ConvertCSGDomainToBlockAndRegion(vname, &csgdom, &csgreg);

    if (!gdbCache->GetVTKObjectKey(&vname, &type, &ts, csgdom, &mat, ds))
    {
        EXCEPTION1(PointerNotInCacheException, ds);
    }

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
        else
            debug1 << "Found discretized CSG grid in cache for current timestate." << endl;
    }

    //
    // Ok, we didn't find a result for the current timestate. But, there might
    // be a result from another timestate we could use. We look for one now.
    //
    if (dgrid == 0)
        dgrid = FindMatchingCSGDiscretization(md, dataRequest, vname, type, ts, csgdom, dom, mat);

    //
    // Ok, we need to discretize the CSG grid.
    //
    if (dgrid == 0)
    {
        debug1 << "No discretized CSG grid in cache. Computing a disrcetization..." << endl;

        vtkCSGGrid *csgmesh = vtkCSGGrid::SafeDownCast(ds);
        const double *bnds = csgmesh->GetBounds();

        double tol;
        double minX, maxX, minY, maxY, minZ, maxZ;

        tol = dataRequest->DiscTol();
        minX = bnds[0]; maxX = bnds[1];
        minY = bnds[2]; maxY = bnds[3];
        minZ = bnds[4]; maxZ = bnds[5];
        tol = ComputeCellSize(tol, minX, maxX, minY, maxY, minZ, maxZ);
        int nX = (int) ((maxX - minX) / tol);
        int nY = (int) ((maxY - minY) / tol);
        int nZ = (int) ((maxZ - minZ) / tol);

        int dims[3] = {nX, nY, nZ};

        if (nZ < 1)
            nZ = 1;

        //
        // Determine this processor's pieces.
        //
        int nXBlocks, nYBlocks, nZBlocks;
#ifdef PARALLEL
        int nDims = (nZ < 1) ? 2 : 3;
        if (nDims == 2)
        {
            //
            // The arrays are intentionally ordered Y, Z, X because the
            // function does poor decompositions when the X dimension is 1.
            //
            avtDatabase::ComputeRectilinearDecomposition(nDims, nprocs, nY,
                nZ, nX, &nYBlocks, &nZBlocks, &nXBlocks);
        }
        else
        {
            avtDatabase::ComputeRectilinearDecomposition(nDims, nprocs, nX,
                nY, nZ, &nXBlocks, &nYBlocks, &nZBlocks);
        }
#else
        nXBlocks = 1;
        nYBlocks = 1;
        nZBlocks = 1;
#endif

        double deltaX = double(nX) / double(nXBlocks);
        double deltaY = double(nY) / double(nYBlocks);
        double deltaZ = double(nZ) / double(nZBlocks);

        int iBlock = rank;
        int iXBlock = iBlock / (nYBlocks * nZBlocks);
        int iMin = int(round(deltaX * double(iXBlock))) - 1;
        iMin = iMin >= 0 ? iMin : 0;
        int iMax = int(round(deltaX * double(iXBlock + 1))) + 1;
        iMax = iMax < nX ? iMax : nX;

        iBlock = iBlock % (nYBlocks * nZBlocks);
        int iYBlock = iBlock / nZBlocks;
        int jMin = int(round(deltaY * double(iYBlock))) - 1;
        jMin = jMin >= 0 ? jMin : 0;
        int jMax = int(round(deltaY * double(iYBlock + 1))) + 1;
        jMax = jMax < nY ? jMax : nY;

        int iZBlock = iBlock % nZBlocks;
        int kMin = int(round(deltaZ * double(iZBlock))) - 1;
        kMin = kMin >= 0 ? kMin : 0;
        int kMax = int(round(deltaZ * double(iZBlock + 1))) + 1;
        kMax = kMax < nZ ? kMax : nZ;

        int subRegion[6];
        subRegion[0] = iMin; subRegion[1] = iMax;
        subRegion[2] = jMin; subRegion[3] = jMax;
        subRegion[4] = kMin; subRegion[5] = kMax;

        if (dataRequest->DiscBoundaryOnly())
        {
            dgrid = (vtkDataSet *) csgmesh->DiscretizeSurfaces(csgreg,
                                             dataRequest->DiscTol(),
                                             bnds[0], bnds[1], bnds[2],
                                             bnds[3], bnds[4], bnds[5]);
        }
        else if (dataRequest->DiscMode() == 0) // uniform
        {
            dgrid = csgmesh->DiscretizeSpace(csgreg,
                                             dataRequest->DiscTol(),
                                             bnds[0], bnds[1], bnds[2],
                                             bnds[3], bnds[4], bnds[5]);
        }
        else if (dataRequest->DiscMode() == 1) // adaptive
        {
            dgrid = csgmesh->DiscretizeSpace3(csgreg, rank, nprocs,
                                              dataRequest->DiscTol(),
                                              dataRequest->FlatTol(),
                                              bnds[0], bnds[1], bnds[2],
                                              bnds[3], bnds[4], bnds[5]);
        }
        else // if (dataRequest->DiscMode() == 2) // multi-pass
        {
            // Look to see if there's a processed mesh we
            // stored for all domains.
            vtkDataSet *processed = 
                (vtkDataSet*)cache.GetVTKObject(vname, type, -1, -1, mat);

            // If so, make sure the settings used to generate it
            // match our current settings.
            if (processed)
            {
                void_ref_ptr oldVrDr = cache.GetVoidRef(vname,
                                                        avtVariableCache::DATA_SPECIFICATION, -1, -1);
                avtDataRequest *oldDr = (avtDataRequest *) *oldVrDr;
                if (oldDr->DiscBoundaryOnly() != dataRequest->DiscBoundaryOnly() ||
                    oldDr->DiscTol() != dataRequest->DiscTol() ||
                    oldDr->FlatTol() != dataRequest->FlatTol() || 
                    oldDr->DiscMode() != dataRequest->DiscMode())
                {
                    processed = NULL;
                }
            }

            if (processed)
                csgmesh = vtkCSGGrid::SafeDownCast(processed);

            dgrid = csgmesh->DiscretizeSpaceMultiPass(csgreg, false,
                bnds, dims, subRegion);

            if (dgrid == NULL)
            {
                std::ostringstream oss;
                oss << "Unable to discretize domain " << dom << " with "
                    << "the multi pass method, trying the uniform method.";
                std::string msg(oss.str());
                avtCallback::IssueWarning(msg.c_str());

                dgrid = csgmesh->DiscretizeSpace(csgreg,
                                             dataRequest->DiscTol(),
                                             bnds[0], bnds[1], bnds[2],
                                             bnds[3], bnds[4], bnds[5]);
            }
            cache.CacheVTKObject(vname, type, -1, -1, mat, csgmesh);
            avtDataRequest *newdataRequest = new avtDataRequest(dataRequest);
            const void_ref_ptr vr = void_ref_ptr(newdataRequest, DestructDspec);
            cache.CacheVoidRef(vname, avtVariableCache::DATA_SPECIFICATION,
                               -1, -1, vr);
        }

        if (dgrid == NULL)
        {
            std::ostringstream oss;
            oss << "Unable to discretize domain " << dom << ". Ignoring it.";
            std::string msg(oss.str());
            avtCallback::IssueWarning(msg.c_str());
            dgrid = vtkUnstructuredGrid::New();
        }

        //
        // Cache the discretized mesh for this timestep
        //
        cache.CacheVTKObject(vname, type, ts, dom, mat, dgrid);
        dgrid->Delete();

        avtDataRequest *newdataRequest = new avtDataRequest(dataRequest);
        const void_ref_ptr vr = void_ref_ptr(newdataRequest, DestructDspec);
        cache.CacheVoidRef(vname, avtVariableCache::DATA_SPECIFICATION,
                           ts, dom, vr);

        //
        // Ok, this is a bit of a hack. We want to cache BOTH the
        // discretized result AND the original CSG input mesh in such
        // a way that a later ClearTimestep() WILL NOT delete 'em. So,
        // we stick them in the cache at timestep=-1. Also, we use 
        // a TransformManager unique 'type' of "DISCRETEIZED_CSG" to
        // disambiguate between the csg input and its discretized output.
        // We do this caching so that if a CSG grid does NOT change with
        // time, we can avoid re-discretizing it each timestep. 
        //
        if (cache.GetVTKObject(vname, type, -1, csgdom, mat) == NULL)
        {
            vtkCSGGrid *csgcopy = vtkCSGGrid::New();
            csgcopy->ShallowCopy(csgmesh);
            cache.CacheVTKObject(vname, type, -1, csgdom, mat, csgcopy);
            csgcopy->Delete();

            cache.CacheVoidRef(vname, avtVariableCache::DATA_SPECIFICATION,
                               -1, csgdom, vr);
        }

        vtkDataSet *dgridcopy = dgrid->NewInstance();
        dgridcopy->ShallowCopy(dgrid);
        cache.CacheVTKObject(vname, "DISCRETIZED_CSG", -1, dom, mat, dgridcopy);
        dgridcopy->Delete();
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
        if (!gdbCache->GetVTKObjectKey(&vname, &type, &ts, csgdom, &mat, da))
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
    vtkDataArray *ghostZones = dgrid->GetCellData()->GetArray("avtGhostZones");
    rv->GetCellData()->AddArray(ghostZones);

    std::string meshname = md->MeshForVar(vname);
    md->SetContainsGhostZones(meshname, AVT_CREATED_GHOSTS);

    return rv;
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
//    Eric Brugger, Wed Jul 25 09:02:59 PDT 2012
//    I modified the multi-pass discretizion of CSG meshes to only process
//    a portion of the mesh on each processor instead of the entire mesh.
//
//    Eric Brugger, Thu Jul 26 14:36:12 PDT 2012
//    I modified the routine to ignore the cache if any of the mesh
//    discretization parameters have changed.
//
//    Mark C. Miller, Wed Aug 22 09:01:36 PDT 2012
//    Fixed leak of matnames
//
//    Burlen Loring, Mon Jul 14 15:52:49 PDT 2014
//    fix alloc-dealloc-mismatch (operator new [] vs free) of matnames
//
//    Eric Brugger, Wed Nov 19 08:46:49 PST 2014
//    I reduced the number of reads of CSG meshes to only once per CSG mesh
//    instead of once per region in order to reduce the number of times the
//    same CSG mesh was cached. Typically there is one CSG mesh with many
//    regions, so this is a significant saving. CSG meshes with thousands
//    of regions were exhausting memory in the previous scheme.
//
// ****************************************************************************
bool
avtTransformManager::TransformMaterialDataset(avtDatabaseMetaData *md,
    const avtDataRequest_p &dataRequest, avtMaterial **mat, int dom)
{
    const char *vname, *type;
    int ts;

    if (mat == 0 || *mat == 0)
        return false;

    vname = dataRequest->GetVariable();
    std::string meshname = md->MeshForVar(vname);
    const avtMeshMetaData *mmd = md->GetMesh(meshname);
    if (mmd->meshType == AVT_CSG_MESH)
    {
        int csgdom = dom, csgreg;
        md->ConvertCSGDomainToBlockAndRegion(vname, &csgdom, &csgreg);

        // find the given material object in Generic DB's cache
        int refCount = 1;
        void_ref_ptr vr = void_ref_ptr(*mat, avtMaterial::Destruct, &refCount);
        if (!gdbCache->GetVoidRefKey(&vname, &type, &ts, csgdom, vr))
        {
            EXCEPTION1(PointerNotInCacheException, *vr);
        }
    
        // Determine if the cache is valid based on whether or not the
        // discretization parameters have changed.
        bool cache_valid = true;
        void_ref_ptr vrdataRequest = cache.GetVoidRef(meshname.c_str(),
                                   avtVariableCache::DATA_SPECIFICATION, ts, dom);
        avtDataRequest *olddataRequest = (avtDataRequest *) *vrdataRequest;
        if (olddataRequest != NULL)
        {
            if ((olddataRequest->DiscBoundaryOnly() != dataRequest->DiscBoundaryOnly()) ||
                (olddataRequest->DiscTol() != dataRequest->DiscTol()) ||
                (olddataRequest->FlatTol() != dataRequest->FlatTol()) ||
                (olddataRequest->DiscMode() != dataRequest->DiscMode()))
            {
                cache_valid = false;
            }
        }

        // first, look for this material's mesh in xform's cache
        vtkDataSet *ds = (vtkDataSet *) cache.GetVTKObject(meshname.c_str(),
                             avtVariableCache::DATASET_NAME, ts, dom, "_all");
        if (!cache_valid || !ds)
        {
            //
            // Here, it must be the case that the csg mesh on which this material
            // is defined has not yet been discretized. So, do it now.
            //
            ds = (vtkDataSet *) gdbCache->GetVTKObject(meshname.c_str(),
                     avtVariableCache::DATASET_NAME, ts, csgdom, "_all");
            if (!ds)
            {
                EXCEPTION1(PointerNotInCacheException, ds);
            }
            ds = CSGToDiscrete(md, dataRequest, ds, dom);
        }

        // see if we already have transformed avtMaterial result cached
        void_ref_ptr dvr = cache.GetVoidRef(vname, type, ts, dom);
        if (!cache_valid || *dvr == 0)
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
                delete [] matnames[j];
            delete [] matnames;

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
//
//    Mark C. Miller, Wed May  6 13:51:30 PDT 2009
//    Fix md for the mesh if we indeed add VERTEX cells.
//
//    Kathleen Biagas, Tue Apr 12 16:58:47 PDT 2016
//    Removed examination of cell/pt data arrays, as the restriction prevents
//    creation of Vertex Cells for Mesh plots.
//
// ****************************************************************************

vtkDataSet *
avtTransformManager::AddVertexCellsToPointsOnlyDataset(avtDatabaseMetaData *md,
    vtkDataSet *ds, int dom)
{
    int i, doType = ds->GetDataObjectType();

    if (doType != VTK_POLY_DATA && 
        doType != VTK_UNSTRUCTURED_GRID)
        return ds; // no-op

    if (ds->GetNumberOfPoints() == 0)
        return ds; // no-op

    if (ds->GetNumberOfCells() != 0)
        return ds; // no-op

    // Ok, really look this object up via reverse lookup
    const char *mname;
    if (!gdbCache->GetVTKObjectKey(&mname, 0, 0, dom, 0, ds))
    {
        EXCEPTION1(PointerNotInCacheException, ds);
    }

    debug1 << "avtTransformManager: Adding " << ds->GetNumberOfPoints() << " VTK_VERTEX cells" << endl;
    debug1 << "to a dataset that consists solely of points but no cells." << endl;

    // This is really a point mesh. So, ensure md reflects that.
    // We're only doing this on engine though. Could be a problem.
    for (i = 0; i < md->GetNumMeshes(); i++)
    {
        avtMeshMetaData &mmd = md->GetMeshes(i);
        if (mmd.name == std::string(mname))
        {
            mmd.meshType = AVT_POINT_MESH;
            mmd.topologicalDimension = 0;
            break;
        }
    }

    //
    // The only way to arrive here is if we have non-zero number of points
    // but zero cells. So, we can add vertex cells.
    //
    vtkIdType onevertex[1];
    if (doType == VTK_POLY_DATA)
    {
        vtkPolyData *pd = vtkPolyData::SafeDownCast(ds);
        if(pd != NULL)
        {
            pd->Allocate(ds->GetNumberOfPoints());
            for (i = 0; i < ds->GetNumberOfPoints(); i++)
            {
                onevertex[0] = i;
                pd->InsertNextCell(VTK_VERTEX, 1, onevertex);
            }
        }

    }
    else // must be VTK_UNSTRUCTURED_GRID
    {
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(ds);
        if(ugrid != NULL)
        {
            ugrid->Allocate(ds->GetNumberOfPoints());
            for (i = 0; i < ds->GetNumberOfPoints(); i++)
            {
                onevertex[0] = i;
                ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
            }
        }
    }

    //
    // Note, we're returning the original pointer here. So, transform manager
    // will NOT think we've done any work on it. In fact, we have 'transformed'
    // it 'in place' meaning the version in generic db's cache was modified.
    // This should be harmless to TransformManager.
    //
    return ds;
}

//
// Convenience macro to instantiate type-specific code for each VTK
// data array type.
//
#define SET_VALS(vN,cN)                                \
    vN  *ina = vN::SafeDownCast(inda);                 \
    vN *outa = vN::SafeDownCast(outda);                \
    cN *invals = ina->GetPointer(0);                   \
    cN *outvals = outa->GetPointer(0);                 \
    int off = 0;                                       \
    if (skip == 3 && invals[1] != (cN) 0) off = 1;     \
    if (skip == 3 && invals[2] != (cN) 0) off = 2;     \
    for (int i = 0, j = off; i < npts; i++, j += skip) \
        outvals[i] = invals[j];

static void
CopyDataArrayVals(vtkDataArray *inda, vtkDataArray *outda, int npts, int skip)
{
    switch (inda->GetDataType())
    {
        case VTK_CHAR: { SET_VALS(vtkCharArray,char); break; }
        case VTK_UNSIGNED_CHAR: { SET_VALS(vtkUnsignedCharArray, unsigned char); break; }
        case VTK_SHORT: { SET_VALS(vtkShortArray, short); break; }
        case VTK_UNSIGNED_SHORT: { SET_VALS(vtkUnsignedShortArray, unsigned short); break; }
        case VTK_INT: { SET_VALS(vtkIntArray, int); break; }
        case VTK_UNSIGNED_INT: { SET_VALS(vtkUnsignedIntArray, unsigned int); break; }
        case VTK_LONG: { SET_VALS(vtkLongArray, long); break; }
        case VTK_LONG_LONG: { SET_VALS(vtkLongLongArray, long long); break; }
        case VTK_UNSIGNED_LONG: { SET_VALS(vtkUnsignedLongArray, unsigned long); break; }
        case VTK_UNSIGNED_LONG_LONG: { SET_VALS(vtkUnsignedLongLongArray, unsigned long long); break; }
        case VTK_FLOAT: { SET_VALS(vtkFloatArray, float); break; }
        case VTK_DOUBLE: { SET_VALS(vtkDoubleArray, double); break; }
        case VTK_ID_TYPE: { SET_VALS(vtkIdTypeArray, vtkIdType); break; }
    }
}

// ****************************************************************************
//  Method: ConvertCurvesToRectGrids
//
//  Purpose: Handle situations in which a plugin serves up what amounts to
//  curve data as a 1D, non-rectilinear-grid, dataset. 
//
//  Programmer: Mark C. Miller
//  Creation: February 9, 2009
//  
//  Modifications:
//    Mark C. Miller, Fri Feb 13 09:12:38 PST 2009
//    Fixed problem where cmd could be non-zero after exiting loop looking
//    for curve mds of the given name. 
//
//    Mark C. Miller, Wed Feb 18 17:24:29 PST 2009
//    Fixed lookup of vtk object key to use yvals data array instead of the
//    dataset itself which returns the mesh object.
// ****************************************************************************

vtkDataSet *
avtTransformManager::ConvertCurvesToRectGrids(avtDatabaseMetaData *md,
vtkDataSet *ds, int dom)
{
    // As cheaply as possible, we want to avoid as much as possible the
    // call to GetVTKObjectKey() which does a reverse lookup in generic
    // db's cache. So, we first do a number of checks to see if we
    // indeed have any work to do. 

    // If this dataset is for a curve, then if its already a rect grid,
    // we don't have any work to do on it. 
    int doType = ds->GetDataObjectType();
    if (doType == VTK_RECTILINEAR_GRID)
        return ds;

    // Can't possibly be any work to do if we don't have any curves in md
    if (md->GetNumCurves() <= 0)
        return ds;

    // Rule out any datasets that cannot possibly be curves.
    int xvalsType = 0;
    vtkDataArray *xvals = NULL;
    switch (doType)
    {
        case VTK_POLY_DATA:
        {
            // It can't possibly be a curve if its got polys or strips
            vtkPolyData *pd = vtkPolyData::SafeDownCast(ds);
            if (pd->GetNumberOfPolys() || pd->GetNumberOfStrips())
                return ds;
            xvalsType = pd->GetPoints()->GetDataType();
            xvals = pd->GetPoints()->GetData();
            break;
        }
        case VTK_STRUCTURED_GRID:
        {
            // It can't possibly be curve data if it has more than one dimension
            // that is of size greater than one.
            int dims[3];
            vtkStructuredGrid *sgrid = vtkStructuredGrid::SafeDownCast(ds);
            sgrid->GetDimensions(dims);
            int numDimsGtOne = 0;
            for (int i = 0; i < 3; i++)
            {
                if (dims[i] > 1)
                    numDimsGtOne++;
            }
            if (numDimsGtOne > 1)
                return ds;
            xvalsType = sgrid->GetPoints()->GetDataType();
            xvals = sgrid->GetPoints()->GetData();
            break;
        }
        case VTK_UNSTRUCTURED_GRID:
        {
            //
            // There isn't a whole lot we can do to check this case without doing
            // problem sized work. But, a curve should be a set of N points and
            // N-1 (or N if the only cells are VTK_VERTEX) cells. So, check that.
            //
            vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(ds);
            if (ugrid->GetNumberOfCells() > 0 &&
                ugrid->GetNumberOfCells() != ugrid->GetNumberOfPoints()-1 &&
                ugrid->GetNumberOfCells() != ugrid->GetNumberOfPoints())
                return ds;
            xvalsType = ugrid->GetPoints()->GetDataType();
            xvals = ugrid->GetPoints()->GetData();
            break;
        }
    }

    //
    // Do some additional sanity checks and setup the yvals array
    //
    int npts = ds->GetNumberOfPoints();
    vtkDataArray *yvals;
    yvals = ds->GetPointData()->GetScalars();
    if (yvals == 0 || yvals->GetNumberOfTuples() != npts)
        return ds;
    int yvalsType = yvals->GetDataType();

    //
    // Ok, now a more expensive check to see if MetaData says this object
    // is a curve. To do it, we need to do the reverse lookup in generic
    // db's cache.
    //
    int i;
    const char *vname;
    if (!gdbCache->GetVTKObjectKey(&vname, 0, 0, dom, 0, yvals))
    {
        EXCEPTION1(PointerNotInCacheException, yvals);
    }
    const avtCurveMetaData *cmd = 0;
    for (i = 0; i < md->GetNumCurves(); i++)
    {
        cmd = md->GetCurve(i);
        if (cmd->from1DScalarName == std::string(vname))
            break;
        cmd = 0;
    }
    if (cmd == 0)
        return ds;

    debug1 << "avtTransformManager: Converting \"" << vname
           << "\" scalar dataset of size " << npts
           << " to a curve dataset" << endl;

    //
    // Arriving here, we're sure this is a curve object. So, now convert
    // it to the form VisIt wants.
    //
    vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(npts, xvalsType);
    vtkDataArray *xvalsNew = rg->GetXCoordinates(); 
    vtkDataArray *yvalsNew = vtkDataArray::CreateDataArray(yvalsType);
    yvalsNew->SetNumberOfComponents(1);
    yvalsNew->SetName(cmd->name.c_str());
    yvalsNew->SetNumberOfTuples(npts);
    // The first call to CopyDataArrayVals here uses a 'skip' of 3
    // because coords in polydata, s|ugrids are always xyz triplets
    CopyDataArrayVals(xvals, xvalsNew, npts, 3);
    CopyDataArrayVals(yvals, yvalsNew, npts, 1);
    rg->GetPointData()->SetScalars(yvalsNew);
    yvalsNew->Delete();

    return rg;
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
//    Mark C. Miller, Thu Feb 12 02:18:45 PST 2009
//    Convert datasets that are intended to be curves but served up as 
//    non-rectilinear-grid, 1D datasets to correct type.
//
//    Mark C. Miller, Mon Nov  9 10:34:15 PST 2009
//    Changed name and interface to routine to reflect the fact that it is
//    operating on only a single dataset and not a dataset collection as it
//    was previously designed. This allows it to be integrated with generic db
//    as each dataset is read from a plugin instead of after all datasets have
//    been read.
// ****************************************************************************
vtkDataSet *
avtTransformManager::TransformSingleDataset(vtkDataSet *ds,
    int domain, avtDataRequest_p &d_spec,
    avtSourceFromDatabase *src, boolVector &selectionsApplied,
    avtDatabaseMetaData *md)
{
    if (!ds) return 0;

    TRY
    {
        ds = CSGToDiscrete(md, d_spec, ds, domain);

        // Handle vtkPoints datasets that have points but no cells
        ds = AddVertexCellsToPointsOnlyDataset(md, ds, domain);

        // Handle 1D datasets as curves (rect grids)
        ds = ConvertCurvesToRectGrids(md, ds, domain);

        //ds = HangingToConforming(md, d_spec, ds);

        //ds = PolyhedralToZoo(md, d_spec, ds);

        if (avtDatabaseFactory::GetRemoveDuplicateNodes())
        {
            ds = RemoveDuplicateNodes(ds);
        }

        ds = NativeToFloat(md, d_spec, ds, domain);
    }
    CATCH(PointerNotInCacheException)
    {
        ; // do nothing
    }
    ENDTRY

    return ds;
}


// ****************************************************************************
//  Method: RemoveDuplicateNodes
//
//  Purpose: Remove duplicate nodes from fully-disonnected unstructured grids.
//
//  Notes:
//    Moved from avtVTKFileReader::ReadInDataset on December 22, 2014.
//
//  Programmer: Mark C. Miller
//  Creation: July 2, 2014
//  
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtTransformManager::RemoveDuplicateNodes(vtkDataSet *ds)
{
    //
    // Try to remove duplicate nodes in datasets meeting the following
    // criteria...
    //    a) unstructured grid, and
    //    b) more than 1,000,000 points, and
    //    c) number of points is 3x more than number of cells
 
    if (ds->GetDataObjectType() != VTK_UNSTRUCTURED_GRID)
    {
        return ds;
    }

    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::SafeDownCast(ds);

    // Detect the "fully disconnected" case
    if ((ugrid->GetCells()->GetSize() - ugrid->GetNumberOfCells()) <
         ugrid->GetNumberOfPoints())
    {
       return ds;
    }

    debug2 << "In avtVTKFileReader::ReadInDataset, the unstructured grid is"
           << " fully disconnected..." << endl;
    debug2 << "...detecting and removing any spatial duplicate points to"
           << " re-connect mesh." << endl;

    // build list of unique points
    vtkPoints *pts = ugrid->GetPoints();
    std::map<double, std::map<double, std::map<double, vtkIdType> > > uniqpts;
    int n = 0;
    for (int i = 0; i < pts->GetNumberOfPoints(); i++)
    {
        double pt[3];
        pts->GetPoint(i, pt);
        std::map<double, std::map<double, std::map<double, vtkIdType> > >::iterator e0it = uniqpts.find(pt[0]);
        if (e0it != uniqpts.end())
        {
            std::map<double, std::map<double, vtkIdType> >::iterator e1it = e0it->second.find(pt[1]);
            if (e1it != e0it->second.end())
            {
                std::map<double, vtkIdType>::iterator e2it = e1it->second.find(pt[2]);
                if (e2it != e1it->second.end())
                    continue;
            }
        }
        uniqpts[pt[0]][pt[1]][pt[2]] = n++;
    }

    debug2 << "...discovered " << 100.0 * n / pts->GetNumberOfPoints()
           << "% of points are spatially unique." << endl;
    debug2 << "...now reconnecting mesh using unique points." << endl;

    for (int i = 0; i < ugrid->GetNumberOfCells(); i++)
    {
        vtkIdType nCellPts=0, *cellPts=0;
        ugrid->GetCellPoints(i, nCellPts, cellPts);
        for (int j = 0; j < nCellPts; j++)
        {
            double pt[3];
            pts->GetPoint(cellPts[j], pt);
            std::map<double, std::map<double, std::map<double, vtkIdType> > >::const_iterator e0it = uniqpts.find(pt[0]);
            if (e0it == uniqpts.end())
                continue;
            std::map<double, std::map<double, vtkIdType> >::const_iterator e1it = e0it->second.find(pt[1]);
            if (e1it == e0it->second.end())
                continue;
            std::map<double, vtkIdType>::const_iterator e2it = e1it->second.find(pt[2]);
            if (e2it == e1it->second.end())
                continue;
            cellPts[j] = e2it->second;
        }
        ugrid->ReplaceCell(i, nCellPts, cellPts);
    }

    pts->Initialize();
    pts->SetNumberOfPoints(n);
    std::map<double, 
             std::map<double, std::map<double, vtkIdType> >
            >::iterator e0it;
    for (e0it = uniqpts.begin(); e0it != uniqpts.end(); e0it++)
    {
        std::map<double, std::map<double, vtkIdType> >::iterator e1it;
        for (e1it = e0it->second.begin(); e1it != e0it->second.end(); e1it++)
        {
            std::map<double, vtkIdType>::iterator e2it;
            for (e2it = e1it->second.begin(); e2it != e1it->second.end(); e2it++)
                pts->SetPoint(e2it->second, e0it->first, e1it->first, e2it->first);
        }
    }
    return ugrid;
}
