// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "vtkVisItTubeFilter.h"
#include <vtkCellArray.h>
#include <vtkCellArrayIterator.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyLine.h>

#include <algorithm>
#include <vector>

namespace
{

struct IdPointsEqual
{
    IdPointsEqual(vtkPoints *points)
        : Points(points)
    {
    }

    bool operator()(vtkIdType id1, vtkIdType id2) const
    {
        double p1[3], p2[3];
        this->Points->GetPoint(id1, p1);
        this->Points->GetPoint(id2, p2);
        return (p1[0] == p2[0] && p1[1] == p2[1] && p1[2] == p2[2]);
    }

    vtkPoints *Points;
};

}

vtkStandardNewMacro(vtkVisItTubeFilter);

vtkVisItTubeFilter::vtkVisItTubeFilter()
{
    this->ScalarsForRadius = nullptr;
}

vtkVisItTubeFilter::~vtkVisItTubeFilter()
{
    this->SetScalarsForRadius(nullptr);
}

//   Jeremy Meredith, Wed May 26 14:52:29 EDT 2010
//   Allow cell scalars for tube radius.
//
//    Kathleen Biagas, Tue Aug  7 10:58:16 PDT 2012
//    Use ScalarsForRadius to retrieve the scalars, if it is nullptr, then
//    GetScalars will retrieve the active scalar array.
//
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK9, use vtkCellArrayIterator and const for pts.
//
//    Kathleen Biagas, Mon July 20, 2026
//    Update with improvements/changes from vtk9.5 version of vtkTubeFilter.
//

int vtkVisItTubeFilter::RequestData(vtkInformation *vtkNotUsed(request),
                                    vtkInformationVector **inputVector,
                                    vtkInformationVector *outputVector)
{
    // get the info objects
    vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
    vtkInformation *outInfo = outputVector->GetInformationObject(0);

    // get the input and ouptut
    vtkPolyData *input = vtkPolyData::SafeDownCast(
        inInfo->Get(vtkDataObject::DATA_OBJECT()));
    vtkPolyData *output = vtkPolyData::SafeDownCast(
        outInfo->Get(vtkDataObject::DATA_OBJECT()));

    vtkPointData *pd=input->GetPointData();
    vtkPointData *outPD=output->GetPointData();
    vtkCellData *cd=input->GetCellData();
    vtkCellData *outCD=output->GetCellData();
    vtkCellArray *inLines;
    vtkDataArray *inNormals;
    vtkDataArray *inScalars = pd->GetScalars(this->ScalarsForRadius);

    bool cellScalars = false;
    if (!inScalars)
    {
        cellScalars = true;
        inScalars = cd->GetScalars(this->ScalarsForRadius);
    }
    vtkDataArray *inVectors=pd->GetVectors();

    vtkPoints *inPts;
    vtkIdType numPts;
    vtkIdType numLines;
    vtkIdType numNewPts, numNewCells;
    vtkPoints *newPts;
    int deleteNormals=0;
    vtkFloatArray *newNormals;
    vtkIdType i;
    double range[2], maxSpeed=0;
    vtkCellArray *newStrips;
    vtkIdType npts=0;
    const vtkIdType *pts=nullptr;
    vtkIdType offset=0;
    vtkFloatArray *newTCoords=nullptr;
    bool abort = false;
    vtkIdType inCellId;
    double oldRadius=1.0;

    // Check input and initialize
    //
    vtkDebugMacro(<<"Creating tube");

    if ( !(inPts=input->GetPoints()) ||
         (numPts = inPts->GetNumberOfPoints()) < 1 ||
         !(inLines = input->GetLines()) ||
         (numLines = inLines->GetNumberOfCells()) < 1 )
    {
        return 1;
    }

    // Create the geometry and topology
    numNewPts = numPts * this->NumberOfSides;
    newPts = vtkPoints::New();
    if (this->OutputPointsPrecision == vtkAlgorithm::DEFAULT_PRECISION)
    {
        newPts->SetDataType(inPts->GetDataType());
    }
    else if (this->OutputPointsPrecision == vtkAlgorithm::SINGLE_PRECISION)
    {
        newPts->SetDataType(VTK_FLOAT);
    }
    else if (this->OutputPointsPrecision == vtkAlgorithm::DOUBLE_PRECISION)
    {
        newPts->SetDataType(VTK_DOUBLE);
    }
    newPts->Allocate(numNewPts);
    newNormals = vtkFloatArray::New();
    newNormals->SetName("TubeNormals");
    newNormals->SetNumberOfComponents(3);
    newNormals->Allocate(3*numNewPts);
    newStrips = vtkCellArray::New();
    newStrips->Allocate(newStrips->EstimateSize(1,numNewPts));
    vtkCellArray *singlePolyline = vtkCellArray::New();

    // Point data: copy scalars, vectors, tcoords. Normals may be computed here.
    outPD->CopyNormalsOff();
    if ( (this->GenerateTCoords == VTK_TCOORDS_FROM_SCALARS && inScalars && !cellScalars) ||
         this->GenerateTCoords == VTK_TCOORDS_FROM_LENGTH ||
         this->GenerateTCoords == VTK_TCOORDS_FROM_NORMALIZED_LENGTH )
    {
        newTCoords = vtkFloatArray::New();
        newTCoords->SetNumberOfComponents(2);
        newTCoords->Allocate(numNewPts);
        outPD->CopyTCoordsOff();
    }
    outPD->CopyAllocate(pd,numNewPts);

    int generateNormals = 0;
    if ( !(inNormals=pd->GetNormals()) || this->UseDefaultNormal )
    {
        deleteNormals = 1;
        inNormals = vtkFloatArray::New();
        inNormals->SetNumberOfComponents(3);
        inNormals->SetNumberOfTuples(numPts);

        if ( this->UseDefaultNormal )
        {
            for ( i=0; i < numPts; i++)
            {
                inNormals->SetTuple(i,this->DefaultNormal);
            }
        }
        else
        {
            // Normal generation has been moved to lower in the function.
            // This allows each different polylines to share vertices, but have
            // their normals (and hence their tubes) calculated independently
            generateNormals = 1;
        }
    }

    // If varying width, get appropriate info.
    //
    if ( inScalars )
    {
        inScalars->GetRange(range,0);
        if ((range[1] - range[0]) == 0.0)
        {
            if (this->VaryRadius == VTK_VARY_RADIUS_BY_SCALAR )
            {
                vtkWarningMacro(<< "Scalar range is zero!");
            }
            range[1] = range[0] + 1.0;
        }
        if (this->VaryRadius == VTK_VARY_RADIUS_BY_ABSOLUTE_SCALAR)
        {
            // temporarily set the radius to 1.0 so that radius*scalar = scalar
            oldRadius = this->Radius;
            this->Radius = 1.0;
            if (range[0] < 0.0)
            {
                vtkWarningMacro(<< "Scalar values fall below zero when using absolute radius values!");
            }
        }
    }
    if ( inVectors )
    {
        maxSpeed = inVectors->GetMaxNorm();
    }

    // Copy selected parts of cell data; certainly don't want normals
    //
    numNewCells = inLines->GetNumberOfCells() * this->NumberOfSides + 2;
    outCD->CopyNormalsOff();
    outCD->CopyAllocate(cd,numNewCells);

    //  Create points along each polyline that are connected into NumberOfSides
    //  triangle strips. Texture coordinates are optionally generated.
    //
    this->Theta = 2.0*vtkMath::Pi() / this->NumberOfSides;
    vtkPolyLine *lineNormalGenerator = vtkPolyLine::New();
    inCellId = input->GetNumberOfVerts();
    int checkAbortInterval = std::min(numLines / 10 + 1, (vtkIdType)1000);
    int progressCounter = 0;
    auto iter = vtk::TakeSmartPointer(inLines->NewIterator());
    for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal() && !abort;
         iter->GoToNextCell(), inCellId++)
    {
        iter->GetCurrentCell(npts, pts);
        this->UpdateProgress((double)progressCounter / numLines);
        if (progressCounter % checkAbortInterval == 0 && this->CheckAbort())
        {
            abort = this->CheckAbort();
            break;
        }
        progressCounter++;

        if (npts < 2)
        {
            vtkWarningMacro(<< "Less than two points in line!");
            continue; //skip tubing this polyline
        }

        std::vector<vtkIdType> ptsCopy(pts, pts + npts);
        vtkIdType *ptsPtr = ptsCopy.data();
        npts = static_cast<vtkIdType>(
            std::unique(ptsPtr, ptsPtr + npts, IdPointsEqual(inPts)) - ptsPtr);
        if (npts < 2)
        {
            continue; // skip tubing this polyline
        }

        // If necessary calculate normals, each polyline calculates its
        // normals independently, avoiding conflicts at shared vertices.
        if (generateNormals)
        {
            singlePolyline->Reset(); //avoid instantiation
            singlePolyline->InsertNextCell(npts,ptsPtr);
            if ( !lineNormalGenerator->GenerateSlidingNormals(inPts,singlePolyline,
                                                              inNormals) )
            {
                vtkWarningMacro("Could not generate normals for line. "
                                "Skipping to next.");
                continue; //skip tubing this polyline
            }
        }

        // Generate the points around the polyline. The tube is not stripped
        // if the polyline is bad.
        //
        if ( !this->GeneratePoints(offset,inCellId,
                                   npts,ptsPtr,inPts,newPts,pd,outPD,
                                   newNormals,
                                   inScalars, cellScalars,
                                   range,inVectors,
                                   maxSpeed,inNormals) )
        {
            vtkWarningMacro(<< "Could not generate points!");
            continue; //skip tubing this polyline
        }

        // Generate the strips for this polyline (including caps)
        //
        this->GenerateStrips(offset,npts,ptsPtr,inCellId,cd,outCD,newStrips);

        // Generate the texture coordinates for this polyline
        //
        if ( newTCoords )
        {
            this->GenerateTextureCoords(offset,npts,ptsPtr,inPts,
                                        inScalars,cellScalars,
                                        newTCoords);
        }

        // Compute the new offset for the next polyline
        offset = this->ComputeOffset(offset,npts);

    }//for all polylines

    singlePolyline->Delete();

    // reset the radius to ite orginal value if necessary
    if (this->VaryRadius == VTK_VARY_RADIUS_BY_ABSOLUTE_SCALAR)
    {
        this->Radius = oldRadius;
    }

    // Update ourselves
    //
    if ( deleteNormals )
    {
        inNormals->Delete();
    }

    if ( newTCoords )
    {
        outPD->SetTCoords(newTCoords);
        newTCoords->Delete();
    }

    output->SetPoints(newPts);
    newPts->Delete();

    output->SetStrips(newStrips);
    newStrips->Delete();

    outPD->SetNormals(newNormals);
    newNormals->Delete();
    lineNormalGenerator->Delete();

    output->Squeeze();

    return 1;
}

//   Jeremy Meredith, Wed May 26 14:52:29 EDT 2010
//   Allow cell scalars for tube radius.
//
//   Jeremy Meredith, Thu Jan 23 13:21:34 EST 2014
//   Fix dead code that read past the end of an array.
//
//    Kathleen Biagas, Thu Aug 11, 2022
//    Support VTK9, change pts arg to const.
//

int vtkVisItTubeFilter::GeneratePoints(vtkIdType offset, vtkIdType inCellId,
                                       vtkIdType npts, const vtkIdType *pts,
                                       vtkPoints *inPts, vtkPoints *newPts,
                                       vtkPointData *pd, vtkPointData *outPD,
                                       vtkFloatArray *newNormals,
                                       vtkDataArray *inScalars, bool cellScalars,
                                       double range[2],
                                       vtkDataArray *inVectors, double maxSpeed,
                                       vtkDataArray *inNormals)
{
    vtkIdType j;
    int i, k;
    double p[3];
    double pNext[3];
    double sNext[3] = {0., 0., 0};
    double sPrev[3];
    double startCapNorm[3], endCapNorm[3];
    double n[3];
    double s[3];
    //double bevelAngle;
    double w[3];
    double nP[3];
    double sFactor=1.0;
    double normal[3];
    vtkIdType ptId=offset;

    // Use "averaged" segment to create beveled effect.
    // Watch out for first and last points.
    //
    for (j=0; j < npts; j++)
    {
        if ( j == 0 ) //first point
        {
            inPts->GetPoint(pts[0],p);
            inPts->GetPoint(pts[1],pNext);
            for (i=0; i<3; i++)
            {
                sNext[i] = pNext[i] - p[i];
                sPrev[i] = sNext[i];
                startCapNorm[i] = -sPrev[i];
            }
            vtkMath::Normalize(startCapNorm);
        }
        else if ( j == (npts-1) ) //last point
        {
            for (i=0; i<3; i++)
            {
                sPrev[i] = sNext[i];
                p[i] = pNext[i];
                endCapNorm[i] = sNext[i];
            }
            vtkMath::Normalize(endCapNorm);
        }
        else
        {
            for (i=0; i<3; i++)
            {
                p[i] = pNext[i];
            }
            inPts->GetPoint(pts[j+1],pNext);
            for (i=0; i<3; i++)
            {
                sPrev[i] = sNext[i];
                sNext[i] = pNext[i] - p[i];
            }
        }

        inNormals->GetTuple(pts[j], n);

        if ( vtkMath::Normalize(sNext) == 0.0 )
        {
            vtkWarningMacro(<<"Coincident points!");
            return 0;
        }

        for (i=0; i<3; i++)
        {
            s[i] = (sPrev[i] + sNext[i]) / 2.0; //average vector
        }
        // if s is zero then just use sPrev cross n
        if (vtkMath::Normalize(s) == 0.0)
        {
            vtkDebugMacro(<< "Using alternate bevel vector");
            vtkMath::Cross(sPrev,n,s);
            if (vtkMath::Normalize(s) == 0.0)
            {
                vtkDebugMacro(<< "Using alternate bevel vector");
            }
        }

        /*    if ( (bevelAngle = vtkMath::Dot(sNext,sPrev)) > 1.0 )
              {
              bevelAngle = 1.0;
              }
              if ( bevelAngle < -1.0 )
              {
              bevelAngle = -1.0;
              }
              bevelAngle = acos((double)bevelAngle) / 2.0; //(0->90 degrees)
              if ( (bevelAngle = cos(bevelAngle)) == 0.0 )
              {
              bevelAngle = 1.0;
              }

              bevelAngle = this->Radius / bevelAngle; //keep tube constant radius
        */
        vtkMath::Cross(s,n,w);
        if ( vtkMath::Normalize(w) == 0.0)
        {
            vtkWarningMacro(<<"Bad normal s = " <<s[0]<<" "<<s[1]<<" "<< s[2]
                            << " n = " << n[0] << " " << n[1] << " " << n[2]);
            return 0;
        }

        vtkMath::Cross(w,s,nP); //create orthogonal coordinate system
        vtkMath::Normalize(nP);

        // Compute a scale factor based on scalars or vectors
        if ( inScalars && this->VaryRadius == VTK_VARY_RADIUS_BY_SCALAR )
        {
            double value = (cellScalars ?
                            inScalars->GetComponent(inCellId,0) :
                            inScalars->GetComponent(pts[j],0));

            sFactor = 1.0 + ((this->RadiusFactor - 1.0) *
                             (value - range[0])
                             / (range[1]-range[0]));
        }
        else if ( inVectors && this->VaryRadius == VTK_VARY_RADIUS_BY_VECTOR )
        {
            sFactor =
                sqrt((double)maxSpeed/vtkMath::Norm(inVectors->GetTuple(pts[j])));
            if ( sFactor > this->RadiusFactor )
            {
                sFactor = this->RadiusFactor;
            }
        }
        else if ( inVectors && this->VaryRadius == VTK_VARY_RADIUS_BY_VECTOR_NORM )
        {
            sFactor = 1.0 + ((this->RadiusFactor - 1.0) *
                             vtkMath::Norm(inVectors->GetTuple(pts[j])) /
                             maxSpeed);
        }
        else if ( inScalars &&
                  this->VaryRadius == VTK_VARY_RADIUS_BY_ABSOLUTE_SCALAR )
        {
            sFactor = (cellScalars ?
                       inScalars->GetComponent(inCellId,0) :
                       inScalars->GetComponent(pts[j],0));
            if (sFactor < 0.0)
            {
                vtkWarningMacro(<<"Scalar value less than zero, skipping line");
                return 0;
            }
        }

        //create points around line
        if (this->SidesShareVertices)
        {
            for (k=0; k < this->NumberOfSides; k++)
            {
                for (i=0; i<3; i++)
                {
                    normal[i] = w[i]*cos((double)k*this->Theta) +
                        nP[i]*sin((double)k*this->Theta);
                    s[i] = p[i] + this->Radius * sFactor * normal[i];
                }
                newPts->InsertPoint(ptId,s);
                newNormals->InsertTuple(ptId,normal);
                outPD->CopyData(pd,pts[j],ptId);
                ptId++;
            }//for each side
        }
        else
        {
            double n_left[3], n_right[3];
            for (k=0; k < this->NumberOfSides; k++)
            {
                for (i=0; i<3; i++)
                {
                    // Create duplicate vertices at each point
                    // and adjust the associated normals so that they are
                    // oriented with the facets. This preserves the tube's
                    // polygonal appearance, as if by flat-shading around the tube,
                    // while still allowing smooth (gouraud) shading along the
                    // tube as it bends.
                    normal[i]  = w[i]*cos((double)(k+0.0)*this->Theta) +
                        nP[i]*sin((double)(k+0.0)*this->Theta);
                    n_right[i] = w[i]*cos((double)(k-0.5)*this->Theta) +
                        nP[i]*sin((double)(k-0.5)*this->Theta);
                    n_left[i]  = w[i]*cos((double)(k+0.5)*this->Theta) +
                        nP[i]*sin((double)(k+0.5)*this->Theta);
                    s[i] = p[i] + this->Radius * sFactor * normal[i];
                }
                newPts->InsertPoint(ptId,s);
                newNormals->InsertTuple(ptId,n_right);
                outPD->CopyData(pd,pts[j],ptId);
                newPts->InsertPoint(ptId+1,s);
                newNormals->InsertTuple(ptId+1,n_left);
                outPD->CopyData(pd,pts[j],ptId+1);
                ptId += 2;
            }//for each side
        }//else separate vertices
    }//for all points in polyline

    //Produce end points for cap. They are placed at tail end of points.
    if (this->Capping)
    {
        int numCapSides = this->NumberOfSides;
        int capIncr = 1;
        if ( ! this->SidesShareVertices )
        {
            numCapSides = 2 * this->NumberOfSides;
            capIncr = 2;
        }

        //the start cap
        for (k=0; k < numCapSides; k+=capIncr)
        {
            newPts->GetPoint(offset+k,s);
            newPts->InsertPoint(ptId,s);
            newNormals->InsertTuple(ptId,startCapNorm);
            outPD->CopyData(pd,pts[0],ptId);
            ptId++;
        }
        //the end cap
        int endOffset = offset + (npts-1)*this->NumberOfSides;
        if ( ! this->SidesShareVertices )
        {
            endOffset = offset + 2*(npts-1)*this->NumberOfSides;
        }
        for (k=0; k < numCapSides; k+=capIncr)
        {
            newPts->GetPoint(endOffset+k,s);
            newPts->InsertPoint(ptId,s);
            newNormals->InsertTuple(ptId,endCapNorm);
            outPD->CopyData(pd,pts[npts-1],ptId);
            ptId++;
        }
    }//if capping

    return 1;
}

//   Jeremy Meredith, Wed May 26 14:52:29 EDT 2010
//   Allow cell scalars for tube radius.
//   We don't support them for texture coordinates, but we need to
//   know if inScalars is point- or cell-based so we know if we
//   should ignore them.
//
//   Kathleen Biagas, Thu Aug 11, 2022
//   Support VTK9, change pts arg to const.
//
//   Kathleen Biagas, Mon July 20, 2026
//   Update with improvements/changes from vtk9.5 version of vtkTubeFilter.
//
void vtkVisItTubeFilter::GenerateTextureCoords(vtkIdType offset,
                                               vtkIdType npts, const vtkIdType *pts,
                                               vtkPoints *inPts,
                                               vtkDataArray *inScalars_,
                                               bool cellScalars,
                                               vtkFloatArray *newTCoords)
{
    vtkIdType i;
    int k;
    double tc=0.0;

    // We only handle point-centered scalars
    vtkDataArray *inScalars = cellScalars ? nullptr : inScalars_;

    int numSides = this->NumberOfSides;
    if ( ! this->SidesShareVertices )
    {
        numSides = 2 * this->NumberOfSides;
    }

    double s0, s;
    //The first texture coordinate is always 0.
    for ( k=0; k < numSides; k++)
    {
        double tcy = static_cast<double>(k) / (numSides - 1);
        newTCoords->InsertTuple2(offset+k,0.0,tcy);
    }
    if ( this->GenerateTCoords == VTK_TCOORDS_FROM_SCALARS )
    {
        if (inScalars == nullptr)
        {
            return;
        }
        s0 = inScalars->GetTuple1(pts[0]);
        for (i=1; i < npts; i++)
        {
            s = inScalars->GetTuple1(pts[i]);
            tc = (s - s0) / this->TextureLength;
            for ( k=0; k < numSides; k++)
            {
                double tcy = static_cast<double>(k) / (numSides - 1);
                newTCoords->InsertTuple2(offset+i*numSides+k,tc,tcy);
            }
        }
    }
    else if ( this->GenerateTCoords == VTK_TCOORDS_FROM_LENGTH )
    {
        double xPrev[3], x[3], len=0.0;
        inPts->GetPoint(pts[0],xPrev);
        for (i=1; i < npts; i++)
        {
            inPts->GetPoint(pts[i],x);
            len += sqrt(vtkMath::Distance2BetweenPoints(x,xPrev));
            tc = len / this->TextureLength;
            for ( k=0; k < numSides; k++)
            {
                double tcy = static_cast<double>(k) / (numSides - 1);
                newTCoords->InsertTuple2(offset+i*numSides+k,tc,tcy);
            }
            xPrev[0]=x[0]; xPrev[1]=x[1]; xPrev[2]=x[2];
        }
    }
    else if ( this->GenerateTCoords == VTK_TCOORDS_FROM_NORMALIZED_LENGTH )
    {
        double xPrev[3], x[3], length=0.0, len=0.0;
        inPts->GetPoint(pts[0],xPrev);
        for (i=1; i < npts; i++)
        {
            inPts->GetPoint(pts[i],x);
            length += sqrt(vtkMath::Distance2BetweenPoints(x,xPrev));
            xPrev[0]=x[0]; xPrev[1]=x[1]; xPrev[2]=x[2];
        }

        inPts->GetPoint(pts[0],xPrev);
        for (i=1; i < npts; i++)
        {
            inPts->GetPoint(pts[i],x);
            len += sqrt(vtkMath::Distance2BetweenPoints(x,xPrev));
            tc = len / length;
            for ( k=0; k < numSides; k++)
            {
                double tcy = static_cast<double>(k) / (numSides - 1);
                newTCoords->InsertTuple2(offset+i*numSides+k,tc,tcy);
            }
            xPrev[0]=x[0]; xPrev[1]=x[1]; xPrev[2]=x[2];
        }
    }

    // Capping, set the endpoints as appropriate
    if ( this->Capping )
    {
        int ik;
        vtkIdType startIdx = offset + npts*numSides;

        //start cap
        for (ik=0; ik < this->NumberOfSides; ik++)
        {
            newTCoords->InsertTuple2(startIdx+ik,0.0,0.0);
        }

        //end cap
        for (ik=0; ik < this->NumberOfSides; ik++)
        {
            newTCoords->InsertTuple2(startIdx+this->NumberOfSides+ik,tc,0.0);
        }
    }
}

void vtkVisItTubeFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);
    os << indent << "Scalars For Radius: "
       << (this->ScalarsForRadius ? this->ScalarsForRadius : "(none)") << endl;
}
