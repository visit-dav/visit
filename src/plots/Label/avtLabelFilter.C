/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                              avtLabelFilter.C                             //
// ************************************************************************* //

#include <avtLabelFilter.h>

#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>

#include <DebugStream.h>
#include <TimingsManager.h>

#include <quant_vector_lookup.C>

// ****************************************************************************
// Method: avtLabelFilter constructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//
// ****************************************************************************

avtLabelFilter::avtLabelFilter()
{
    labelVariable = 0;
}


// ****************************************************************************
// Method: avtLabelFilter destructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 7 14:58:26 PST 2004
//
// Modifications:
//
// ****************************************************************************

avtLabelFilter::~avtLabelFilter()
{
    delete [] labelVariable;
}

// ****************************************************************************
// Method: avtLabelFilter::SetLabelVariable
//
// Purpose: 
//   Sets the name of the variable that will be used to create labels.
//
// Arguments:
//   var : The name of the variable.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:10:25 PDT 2004
//
// Modifications:
//   
// ****************************************************************************


void
avtLabelFilter::SetLabelVariable(const char *var)
{
    if(var == 0)
        debug3 << "avtLabelFilter::SetLabelVariable: var=NULL!" << endl;
    else
    {
        delete [] labelVariable;
        labelVariable = new char[strlen(var) + 1];
        strcpy(labelVariable, var);
    }
}

#if 1
void
print_array_names(vtkDataSet *inDS)
{
    vtkPointData *pd = inDS->GetPointData();
    int i;
    for (i = 0 ; i < pd->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = pd->GetArray(i);
        debug3 << "\tPoint Data array[" << i << "] = " << arr->GetName() << endl;
    }

    vtkCellData *cd = inDS->GetCellData();
    for (i = 0 ; i < cd->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = cd->GetArray(i);
        debug3 << "\tCell Data array[" << i << "] = " << arr->GetName() << endl;
    }

    for (i = 0 ; i < inDS->GetFieldData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = inDS->GetFieldData()->GetArray(i);
        debug3 << "\tField Data array[" << i << "] = " << arr->GetName() << endl;
    }
}
#endif


// ****************************************************************************
//  Method: avtLabelFilter::ExecuteData
//
//  Purpose:
//      Does the actual VTK code to modify the dataset.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jan 7 14:58:26 PST 2004
//
//  Modifications:
//    Brad Whitlock, Thu Oct 28 16:07:46 PST 2004
//    Added a faster algorithm for averaging the cell centers, when needed.
//    The new algorithm assumes that related cells will be contiguous in
//    the cell list, which may or may not be a valid assumption. It looks
//    okay for slices, which is when we do the center averaging anyway.
//
//    Brad Whitlock, Mon Apr 4 10:53:28 PDT 2005
//    Fixed a crash that happened for cells with more than 100 points.
//
// ****************************************************************************

vtkDataSet *
avtLabelFilter::ExecuteData(vtkDataSet *inDS, int, std::string)
{
    vtkDataSet *newDS = (vtkDataSet *) inDS->NewInstance();
    newDS->ShallowCopy(inDS);
    vtkDataSet *outDS = newDS;

    int stageTimer, total = visitTimer->StartTimer();

//    debug3 << "avtLabelFilter::ExecuteData: 1: nCells=" << inDS->GetNumberOfCells() << endl;
//    debug3 << "avtLabelFilter::ExecuteData: 1.5: The data arrays are:" << endl;
//    print_array_names(inDS);

    //
    // Try to determine if the variable is a mesh. I tried using activeVariable
    // but it was NULL so I added labelVariable and made the plot set it.
    //
    bool variableProbablyIsAMesh = false;
    bool needCellArray = true;
    bool needNodeArray = true;
    if(labelVariable != 0)
    {
        vtkDataArray *cellVar = inDS->GetCellData()->GetArray(labelVariable);
        vtkDataArray *pointVar = inDS->GetPointData()->GetArray(labelVariable);
        variableProbablyIsAMesh = (cellVar == 0 && pointVar == 0);
        needCellArray = variableProbablyIsAMesh || (cellVar != 0);
        needNodeArray = variableProbablyIsAMesh || (pointVar != 0);
    }
    else
        variableProbablyIsAMesh = true;

//    debug3 << "avtLabelFilter::ExecuteData: 3.5: The data arrays are:" << endl;
//    print_array_names(outDS);

    //
    // If the variable is probably a mesh then rename the original cell numbers
    // array so it won't be removed by the condense dataset filter. If we
    // likely have a mesh then we'll need the original cell numbers in order
    // to correctly label the cells.
    //
    stageTimer = visitTimer->StartTimer();
    vtkDataArray *originalCellNumbers =
        outDS->GetCellData()->GetArray("avtOriginalCellNumbers");
    if(needCellArray &&
       originalCellNumbers != 0 &&
       originalCellNumbers->IsA("vtkUnsignedIntArray"))
    {
        if(originalCellNumbers->GetNumberOfComponents() == 2)
        {
            debug3 << "Creating LabelOriginalCellNumbers from avtOriginalCellNumbers"
                   << endl;
            int n = originalCellNumbers->GetNumberOfTuples();
            vtkUnsignedIntArray *newCellNos = vtkUnsignedIntArray::New();
            newCellNos->SetName("LabelFilterOriginalCellNumbers");
            newCellNos->SetNumberOfTuples(n);
            unsigned int *dest = (unsigned int*)newCellNos->GetVoidPointer(0);
            unsigned int *src = (unsigned int*)originalCellNumbers->GetVoidPointer(0);
            ++src;
            for(int i = 0; i < n; ++i, src+=2)
                *dest++ = *src;
            outDS->GetCellData()->AddArray(newCellNos);
            newCellNos->Delete();
        }
        else
        {
            debug3 << "Renaming avtOriginalCellNumbers->LabelFilterOriginalCellNumbers"
                   << endl;
            originalCellNumbers->SetName("LabelFilterOriginalCellNumbers");
        }
    }
    visitTimer->StopTimer(stageTimer, "Creating LabelFilterOriginalCellNumbers");

    //
    // If the variable is probably a mesh then rename the original cell numbers
    // array so it won't be removed by the condense dataset filter. If we
    // likely have a mesh then we'll need the original cell numbers in order
    // to correctly label the cells.
    //
    stageTimer = visitTimer->StartTimer();
    vtkDataArray *originalNodeNumbers =
        outDS->GetPointData()->GetArray("avtOriginalNodeNumbers");
    if(needNodeArray &&
       originalNodeNumbers != 0 &&
       originalNodeNumbers->IsA("vtkIntArray"))
    {
        if(originalNodeNumbers->GetNumberOfComponents() == 2)
        {
            debug3 << "Creating LabelOriginalNodeNumbers from avtOriginalNodeNumbers"
                   << endl;
            // Throw out the domain numbers.
            int n = originalNodeNumbers->GetNumberOfTuples();
            vtkUnsignedIntArray *newNodeNos = vtkUnsignedIntArray::New();
            newNodeNos->SetName("LabelFilterOriginalNodeNumbers");
            newNodeNos->SetNumberOfTuples(n);
            unsigned int *dest = (unsigned int*)newNodeNos->GetVoidPointer(0);
            int *src = (int*)originalNodeNumbers->GetVoidPointer(0);
            ++src;
            for(int i = 0; i < n; ++i, src+=2)
                *dest++ = *src;
            outDS->GetPointData()->AddArray(newNodeNos);
            newNodeNos->Delete();
        }
        else
        {
            debug3 << "Renaming avtOriginalNodeNumbers->LabelFilterOriginalNodeNumbers"
                   << endl;
            originalNodeNumbers->SetName("LabelFilterOriginalNodeNumbers");
        }
    }
    visitTimer->StopTimer(stageTimer, "Creating LabelFilterOriginalNodeNumbers");

    //
    // If we have normals then quantize them so the float3 gets assigned to a
    // vector that closest matches a vector on a unit sphere. This way we can
    // store an index to the vector, which takes much less storage and we can
    // at render time do simple comparisons on the indices of visible cells
    // instead of doing N vector dot products with the camera vector.
    //
    stageTimer = visitTimer->StartTimer();
    QuantizationRetval cQ, pQ;
    cQ = CreateQuantizedNormalsFromPointNormals(outDS, variableProbablyIsAMesh);
    pQ = CreateQuantizedNormalsFromCellNormals(outDS, variableProbablyIsAMesh);
    visitTimer->StopTimer(stageTimer, "Creating quantized normals");

    //
    // If the variable is a cell centered scalar, vector or if it
    // is a mesh then we want to compute the cell centers and
    // add that array to the dataset.
    //
    stageTimer = visitTimer->StartTimer();
    if(variableProbablyIsAMesh ||
       GetInput()->GetInfo().GetAttributes().GetCentering() == AVT_ZONECENT)
    {
        debug3 << "LabelFilter: adding LabelFilterCellCenters array" << endl;
        //
        // Allocate enough memory for a cell-centered vector
        //
        vtkFloatArray *cellCenters = vtkFloatArray::New();
        cellCenters->SetName("LabelFilterCellCenters");
        cellCenters->SetNumberOfComponents(3);
        cellCenters->SetNumberOfTuples(inDS->GetNumberOfCells());

        //
        // Figure out the center of each cell.
        //
        int    nWeights = 100;
        double *weights = new double[nWeights];
        for(vtkIdType cellid = 0; cellid < inDS->GetNumberOfCells(); ++cellid)
        {
            double center[] = {0., 0., 0.};

            vtkCell *cell = inDS->GetCell(cellid);
            if(cell != 0)
            {
                int subId = 0;
                double pcoords[3];
                cell->GetParametricCenter(pcoords);

                // Make sure that the weights array is big enough.
                if(cell->GetNumberOfPoints() > nWeights)
                {
                    delete [] weights;
                    nWeights = int(cell->GetNumberOfPoints() * 1.25);
                    weights = new double[nWeights];
                }
                cell->EvaluateLocation(subId, pcoords, center, weights);
            }
            cellCenters->SetTuple(cellid, center);
        }
        delete [] weights;

        //
        // Add the new array to the VTK dataset.
        //
        outDS->GetCellData()->AddArray(cellCenters);
        cellCenters->Delete();
    }
    visitTimer->StopTimer(stageTimer, "Creating LabelFilterCellCenters");

#define CELL_CENTER_HACK
#ifdef CELL_CENTER_HACK
    //
    // If we quantized normals but threw them away because the data was
    // planar AND the quantized normals were axis aligned then we should
    // consolidate any original cells into a point mesh where the point
    // is the average coordinate of the cell centers.
    //
    // That would be too much work so for now, just average the cell centers
    // of cells that share the same original cell. This will give the
    // appearance of 1 label per triangle on an orthogonal slice.
    //
    // Note -- we also do this if we're looking at 2D data that was
    //         derived from 3D or if we material selected 2D data.
    //
    stageTimer = visitTimer->StartTimer();
    if(cQ == DiscardedQuantizedNormals ||
       pQ == DiscardedQuantizedNormals ||
       // Check 2D geometry that's been sliced material selected.
       (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 2 &&
        GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2 &&
        (!GetInput()->GetInfo().GetValidity().GetUsingAllData() ||
         !GetInput()->GetInfo().GetValidity().GetZonesPreserved() ||
         GetInput()->GetInfo().GetValidity().SubdivisionOccurred()))
      )
    {
        vtkDataArray *d = outDS->GetCellData()->GetArray("LabelFilterOriginalCellNumbers");
        unsigned int *cellNumbers = (d != 0) ? (unsigned int *)d->GetVoidPointer(0) : 0;
        d = outDS->GetCellData()->GetArray("LabelFilterCellCenters");
        vtkFloatArray *cellCenters = (vtkFloatArray *)d;
        if(cellNumbers != 0 && cellCenters != 0)
        {
            debug3 << "The geometry seems to be a plane. This "
                   << "means that we should try and hide that there could "
                   << "be multiple cells that correspond to the same "
                   << "original cell." << endl;
#ifdef WORST_NSQUARED_CODE_IN_THE_WORLD
            vtkIdType dupIds[100];
            int i, n = outDS->GetNumberOfCells();
            bool *cellHandled = new bool[n];
            for(i = 0; i < n; ++i)
                cellHandled[i] = false;

            for(vtkIdType c1 = 0; c1 < n; ++c1)
            {
                if(cellHandled[c1])
                    continue;

                int nDupIds = 0;
                unsigned int originalNumber = cellNumbers[c1];
                for(vtkIdType c2 = 0; c2 < n; ++c2)
                {
                    if(originalNumber == cellNumbers[c2])
                        dupIds[nDupIds++] = c2;
                }

                //
                // Average the cell center coordinates
                //
                if(nDupIds > 1)
                {
                    float avgCoord[] = {0,0,0};
                    for(i = 0; i < nDupIds; ++i)
                    {
                        const float *pt = cellCenters->GetTuple3(dupIds[i]);
                        avgCoord[0] += pt[0];
                        avgCoord[1] += pt[1];
                        avgCoord[2] += pt[2];
                    }
                    float scale = 1.f / float(nDupIds);
                    avgCoord[0] *= scale;
                    avgCoord[1] *= scale;
                    avgCoord[2] *= scale;
                    for(i = 0; i < nDupIds; ++i)
                    {
                        cellCenters->SetTuple(dupIds[i], avgCoord);
                        cellHandled[dupIds[i]] = true;
                    }
                }

                cellHandled[c1] = true;
            }

            delete [] cellHandled;
#else 
            //
            // Assume that related cells are contiguous in the cell list.
            //
            int n = outDS->GetNumberOfCells();
            for(int i = 0; i < n; )
            {
                unsigned int cellId = cellNumbers[i];
                int j = i + 1;
                for(; j < n; ++j)
                {
                    if(cellNumbers[j] != cellId)
                        break;
                }
                int repeatCount = j - i;
                if(repeatCount > 1)
                {
                    float avgCoord[] = {0,0,0};
                    int k;
                    for(k = i; k < j; ++k)
                    {
                        const double *pt = cellCenters->GetTuple3(k);
                        avgCoord[0] += pt[0];
                        avgCoord[1] += pt[1];
                        avgCoord[2] += pt[2];
                    }
                    float scale = 1.f / float(repeatCount);
                    avgCoord[0] *= scale;
                    avgCoord[1] *= scale;
                    avgCoord[2] *= scale;
                    for(k = i; k < j; ++k)
                        cellCenters->SetTuple(k, avgCoord);
                }

                i = j;
            }
#endif
        }
    }
    visitTimer->StopTimer(stageTimer, "Averaging cell coordinates");
#endif

//    debug3 << "avtLabelFilter::ExecuteData: 3.75: The data arrays are:" << endl;
//    print_array_names(outDS);

    if (outDS == newDS)
    {
        ManageMemory(outDS);
    }
    newDS->Delete();

    //
    // Set a flag in the data validity that will prevent the plot from
    // creating normals since they are not appropriate.
    //
    GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true);

    visitTimer->StopTimer(total, "avtLabelFilter::ExecuteData");

    return outDS;
}


// ****************************************************************************
//  Method: avtLabelFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Allows the filter to change its output's data object information, which
//      is a description of the data object.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Jan 7 14:58:26 PST 2004
//
// ****************************************************************************

void
avtLabelFilter::RefashionDataObjectInfo(void)
{
//    IF YOU SEE FUNNY THINGS WITH EXTENTS, ETC, YOU CAN CHANGE THAT HERE.
    debug3 << "avtLabelFilter::RefashionDataObjectInfo" << endl;
}

// ****************************************************************************
// Method: avtLabelFilter::FindClosestVector
//
// Purpose: 
//   Finds the quantized vector that is closest to the input vector.
//
// Arguments:
//   vert : The input vector.
//
// Returns:    The index of the closest quantized normal vector.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:11:28 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

unsigned char
avtLabelFilter::FindClosestVector(const double *vert) const
{
    //
    // Figure out the quantized vector index, which is the index of
    // the vector in the quant_vector_lookup table that most closely
    // matches the vector that we're considering.
    //
    // We find the minimum square of the distance between the tips of
    // the vectors to get the closest quantized vector.
    //
    // The lookup table is broken into octants and we figure out
    // which octant to use by looking at the signs of the vector components.
    // We can skip the distance check for a lot of vectors by doing this.
    //
#define DIST_SQUARED(I, VAR) \
    dX = (vert[0] - *lookup++); \
    dY = (vert[1] - *lookup++); \
    dZ = (vert[2] - *lookup++); \
    VAR = dX *dX + dY * dY + dZ * dZ;
    int bin = (((vert[2] >= 0.f) ? 0 : 1) << 2) | 
              (((vert[1] >= 0.f) ? 0 : 1) << 1) | 
               ((vert[0] >= 0.f) ? 0 : 1);
    unsigned char minIndex = quant_vector_lookup_binbounds[bin];
    unsigned char endIndex = quant_vector_lookup_binbounds[bin+1];
    const float *lookup = quant_vector_lookup[minIndex];
    float dX, dY, dZ;
    DIST_SQUARED(minIndex, float minDist)
    for(unsigned char quantIndex = minIndex+1;
        quantIndex < endIndex; ++quantIndex)
    {
        DIST_SQUARED(quantIndex, float distSquared)
        if(distSquared < minDist)
        {
            minIndex = quantIndex;
            minDist = distSquared;
        }
    }

    return minIndex;
}

// ****************************************************************************
// Method: avtLabelFilter::CreateQuantizedNormalsFromPointNormals
//
// Purpose: 
//   Transforms the dataset's Normals into quantized normals, which are less
//   accurate but much faster and smaller.
//
// Arguments:
//   outDS  : The dataset to modify.
//   isMesh : Whether the dataset being modified corresponds to a mesh var.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:12:18 PDT 2004
//
// Modifications:
//   
//   Hank Childs, Tue Nov  2 05:16:53 PST 2004
//   Do not delete memory we don't own.
//
// ****************************************************************************

avtLabelFilter::QuantizationRetval
avtLabelFilter::CreateQuantizedNormalsFromPointNormals(vtkDataSet *outDS,
    bool isMesh)
{
    QuantizationRetval retval = NoAction;

    vtkDataArray *normals_array = outDS->GetPointData()->GetArray("Normals");
    if(normals_array != 0)
    {
        vtkFloatArray *normals = (vtkFloatArray *)normals_array;
        int n = normals->GetNumberOfTuples();

        if(n > 0)
        {
            debug3 << "Creating quantized normals from point normals." << endl;

            // 
            // Create quantized normals for the nodes.
            //
            vtkUnsignedCharArray *quantNodeNormals = vtkUnsignedCharArray::New();
            quantNodeNormals->SetName("LabelFilterQuantizedNodeNormals");
            quantNodeNormals->SetNumberOfTuples(n);
            unsigned char *quant = (unsigned char *)quantNodeNormals->GetVoidPointer(0);
            int i;
            //
            // Do the first normal so we can compare subsequent normals to see
            // if they are equal so we can opt to not save the quantized normals
            // since the geometry is most likely a slice plane.
            //
            const double *vert = normals->GetTuple3(0);
            unsigned char first_quantized_vector = FindClosestVector(vert);
            bool vectors_same = true;
            *quant++ = first_quantized_vector;
            // Find the opposite vector too in case some cells were inverted.
            double oppositevec[] = {
                -quant_vector_lookup[first_quantized_vector][0],
                -quant_vector_lookup[first_quantized_vector][1],
                -quant_vector_lookup[first_quantized_vector][2],
            };
            unsigned char opposite_quant_vec = FindClosestVector(oppositevec);

            //
            // Calculate the other quantized normals.
            //
            for(i = 1; i < n; ++i)
            {
                vert = normals->GetTuple3(i);
                unsigned char qvec = FindClosestVector(vert);
                *quant++ = qvec;
                vectors_same &= (qvec == first_quantized_vector ||
                                 qvec == opposite_quant_vec);
            }

            // Add the quantized node vector indices to the outgoing dataset
            // if the vectors are different. If all of the vectors are the
            // same then the geometry is or is close to being a plane. Let's
            // assume that it was a slice plane and not save the quantized
            // normals since we don't want planes to have a direction.
            if(vectors_same)
            {
                debug3 << "The quantized node vectors were all the same. "
                       << "Not storing quantized normals." << endl;
                retval = DiscardedQuantizedNormals;
            }
            else
            {
                outDS->GetPointData()->AddArray(quantNodeNormals);
                retval = CreatedQuantizedNormals;
            }
            quantNodeNormals->Delete();
           
            //
            // Create quantized normals for the cells.
            //
            if(!vectors_same && isMesh)
            {
                n = outDS->GetNumberOfCells();
                vtkUnsignedCharArray *quantCellNormals = vtkUnsignedCharArray::New();
                quantCellNormals->SetName("LabelFilterQuantizedCellNormals");
                quantCellNormals->SetNumberOfTuples(n);
                quant = (unsigned char *)quantNodeNormals->GetVoidPointer(0);
                unsigned char *cellquant = (unsigned char *)quantCellNormals->
                    GetVoidPointer(0);
                for(i = 0; i < n; ++i)
                {
                    vtkIdType firstPointId = outDS->GetCell(i)->GetPointId(0);
    
                    // Use the quantized normal of the first point. This is not
                    // that great since for large cells, the cell normal will not
                    // likely match the point normal but it ought to be okay for now.
                    *cellquant++ = quant[firstPointId];
                }

                // Add the quantized cell vector indices to the outgoing dataset.
                outDS->GetCellData()->AddArray(quantCellNormals);
                quantCellNormals->Delete();
            }
        }

        // Remove the vectors array since we've created the quantized
        // vectors array.
        outDS->GetPointData()->RemoveArray("Normals");
    }

    return retval;
}

// ****************************************************************************
// Method: avtLabelFilter::CreateQuantizedNormalsFromCellNormals
//
// Purpose: 
//   Transforms the dataset's Normals into quantized normals, which are less
//   accurate but much faster and smaller.
//
// Arguments:
//   outDS  : The dataset to modify.
//   isMesh : Whether the dataset being modified corresponds to a mesh var.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 25 09:12:18 PDT 2004
//
// Modifications:
//   
//   Hank Childs, Tue Nov  2 05:16:53 PST 2004
//   Do not delete memory we don't own.
//
// ****************************************************************************

avtLabelFilter::QuantizationRetval
avtLabelFilter::CreateQuantizedNormalsFromCellNormals(vtkDataSet *outDS,
    bool isMesh)
{
    QuantizationRetval retval = NoAction;

    vtkDataArray *normals_array = outDS->GetCellData()->GetArray("Normals");
    if(normals_array != 0)
    {
        vtkFloatArray *normals = (vtkFloatArray *)normals_array;
        int n = normals->GetNumberOfTuples();

        if(n > 0)
        {
            debug3 << "Creating quantized normals from cell normals." << endl;
            //
            // Create quantized normals for the cells.
            //
            vtkUnsignedCharArray *quantCellNormals = vtkUnsignedCharArray::New();
            quantCellNormals->SetName("LabelFilterQuantizedCellNormals");
            quantCellNormals->SetNumberOfTuples(n);
            unsigned char *quant = (unsigned char *)quantCellNormals->GetVoidPointer(0);
            int i;
            //
            // Do the first normal so we can compare subsequent normals to see
            // if they are equal so we can opt to not save the quantized normals
            // since the geometry is most likely a slice plane.
            //
            const double *vert = normals->GetTuple3(0);
            unsigned char first_quantized_vector = FindClosestVector(vert);
            bool vectors_same = true;
            *quant++ = first_quantized_vector;
            // Find the opposite vector too in case some cells were inverted.
            double oppositevec[] = {
                -quant_vector_lookup[first_quantized_vector][0],
                -quant_vector_lookup[first_quantized_vector][1],
                -quant_vector_lookup[first_quantized_vector][2],
            };
            unsigned char opposite_quant_vec = FindClosestVector(oppositevec);

            //
            // Calculate the other quantized normals.
            //
            for(i = 1; i < n; ++i)
            {
                vert = normals->GetTuple3(i);
                unsigned char qvec = FindClosestVector(vert);
                *quant++ = qvec;
                vectors_same &= (qvec == first_quantized_vector ||
                                 qvec == opposite_quant_vec);
            }
            quant = (unsigned char *)quantCellNormals->GetVoidPointer(0);

            // Add the quantized cell vector indices to the outgoing dataset
            // if the vectors are different. If all of the vectors are the
            // same then the geometry is or is close to being a plane. Let's
            // assume that it was a slice plane and not save the quantized
            // normals since we don't want planes to have a direction.
            if(vectors_same)
            {
                debug3 << "The quantized cell vectors were all the same. "
                       << "Not storing quantized normals." << endl;
                retval = DiscardedQuantizedNormals;
            }
            else
            {
                outDS->GetCellData()->AddArray(quantCellNormals);
                retval = CreatedQuantizedNormals;
            }
            quantCellNormals->Delete();

            //
            // Create quantized normals for the nodes.
            //
            if(!vectors_same && isMesh)
            {
                int dsnpts = outDS->GetNumberOfPoints();
                vtkUnsignedCharArray *quantNodeNormals = vtkUnsignedCharArray::New();
                quantNodeNormals->SetName("LabelFilterQuantizedNodeNormals");
                quantNodeNormals->SetNumberOfTuples(dsnpts);
                unsigned char *nodequant = (unsigned char *)quantNodeNormals->
                    GetVoidPointer(0);

                // Zero out the nodes.
                memset(nodequant, 0, dsnpts);

#if 1
                //
                // BAD CODE ALERT!!!!
                //
                // Iterate over the number cells and for each point in the cell, set the
                // point's quantized normal to be the same as that of the cell. This is
                // wasteful and probably slow but it should work.
                for(i = 0; i < n; ++i)
                {
                    vtkCell *cell = outDS->GetCell(i);
                    unsigned char cellNorm = quant[i];
                    int nptids = cell->GetNumberOfPoints();
                    for(int j = 0; j < nptids; ++j)
                    {
                        vtkIdType id = cell->GetPointId(j);
                        if(id < dsnpts)
                            nodequant[id] = cellNorm;
                    }
                }
#endif

                // Add the quantized node vector indices to the outgoing dataset.
                outDS->GetPointData()->AddArray(quantNodeNormals);
                quantNodeNormals->Delete();
            }
        }

        // Remove the vectors array since we've created the quantized
        // vectors array.
        outDS->GetCellData()->RemoveArray("Normals");
    }

    return retval;
}
