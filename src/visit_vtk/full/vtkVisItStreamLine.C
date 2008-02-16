/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItStreamLine.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm     for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS     for A PARTICULAR 
     PURPOSE.  See the above copyright notice     for more in  formation.


  Brad Whitlock, Wed Nov 6 13:19:58 PST 2002
  The reason this code is kulled out from vtkStreamLine is that I believe
  vtkStreamline's epsilon value that it uses to stop streamline generation
  is too small and causes the code to go into an infinite loop with some
  of our datasets.

  Brad Whitlock, Mon Feb 10 12:34:43 PDT 2003
  I update the code so it works again on Windows with the new VTK.

=========================================================================*/
#include "vtkVisItStreamLine.h"

#include <vtkCellArray.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkGenericCell.h>
#include <vtkInitialValueProblemSolver.h>
#include <vtkInterpolatedVelocityField.h>
#include <vtkMath.h>
#include <vtkMultiThreader.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyLine.h>


vtkCxxRevisionMacro(vtkVisItStreamLine, "$Revision: 1.50 $");
vtkStandardNewMacro(vtkVisItStreamLine);

#define VTK_STREAMLINE_EPSILON 1e-6
#define VTK_START_FROM_POSITION 0
#define VTK_START_FROM_LOCATION 1

#ifdef WIN32
//
// VTK does not export these symbols from their Graphics DLL and it causes
// link problems for us on Windows, so define the functions here and hope it
// works.
//
//
vtkStreamer::StreamPoint *
vtkStreamer::StreamArray::Resize(vtkIdType sz)
{
  vtkStreamer::StreamPoint *newArray;
  vtkIdType newSize;

  if (sz >= this->Size)
    {
    newSize = this->Size + 
      this->Extend*(((sz-this->Size)/this->Extend)+1);
    }
  else
    {
    newSize = sz;
    }

  newArray = new vtkStreamer::StreamPoint[newSize];

  memcpy(newArray, this->Array,
         (sz < this->Size ? sz : this->Size) * sizeof(vtkStreamer::StreamPoint));

  this->Size = newSize;
  delete [] this->Array;
  this->Array = newArray;

  return this->Array;
}

vtkStreamer::StreamArray::StreamArray()
{
  this->MaxId = -1; 
  this->Array = new vtkStreamer::StreamPoint[1000];
  this->Size = 1000;
  this->Extend = 5000;
  this->Direction = VTK_INTEGRATE_FORWARD;
}
#endif

// Construct object with step size set to 1.0.
vtkVisItStreamLine::vtkVisItStreamLine()
{
    this->StepLength = 1.0;
    this->NumberOfStreamers = 0;
}

void vtkVisItStreamLine::Execute()
{
    vtkStreamer::StreamPoint *sPrev, *sPtr;
    vtkPoints      *newPts;
    vtkFloatArray  *newVectors;
    vtkFloatArray  *newScalars = NULL;
    vtkCellArray   *newLines;
    vtkIdType ptId, i, id;
    int j;
    vtkIdList *pts;
    double tOffset, x[3], v[3], s, r;
    vtkPolyData   *output = this->GetOutput();
    vtkDataSet   *input = vtkDataSet::SafeDownCast(this->GetOutput());

#ifdef VORTICITY
    double theta;
    vtkPolyLine   *lineNormalGenerator = NULL;
    vtkFloatArray *normals = NULL;
    vtkFloatArray *rotation = 0;
#endif

    this->SavePointInterval = this->StepLength;

    // This can take a long time.
    this->Integrate();

    if(this->NumberOfStreamers <= 0)
    {
        return;
    }

    pts = vtkIdList::New();
    pts->Allocate(2500);

    //
    //  Convert streamer into lines. Lines may be dashed.
    //
    newPts  = vtkPoints::New();
    newPts->Allocate(1000);
    newVectors  = vtkFloatArray::New();
    newVectors->SetNumberOfComponents(3);
    newVectors->Allocate(3000);

#ifdef VORTICITY
    if(this->Vorticity)
    {
        lineNormalGenerator = vtkPolyLine::New();
        normals = vtkFloatArray::New();
        normals->SetNumberOfComponents(3);
        normals->Allocate(3000);
        rotation = vtkFloatArray::New();
        rotation->SetNumberOfComponents(1);
        rotation->Allocate(1000);
        rotation->SetName("Thetas");
        output->GetPointData()->AddArray(rotation);
    }
#endif

    if(input->GetPointData()->GetScalars() ||
       this->SpeedScalars || this->OrientationScalars)
    {
        newScalars = vtkFloatArray::New();
        newScalars->Allocate(1000);
    }
    newLines = vtkCellArray::New();
    newLines->Allocate(newLines->EstimateSize(2*this->NumberOfStreamers,
                                              VTK_CELL_SIZE));

    //
    // Loop over all streamers generating points
    //
    for(ptId = 0; ptId < this->NumberOfStreamers; ptId++)
    {
        if(this->Streamers[ptId].GetNumberOfPoints() < 2)
        {
            continue;
        }

        sPrev = this->Streamers[ptId].GetStreamPoint(0);
        sPtr = this->Streamers[ptId].GetStreamPoint(1);

        if(this->Streamers[ptId].GetNumberOfPoints() == 2 && sPtr->cellId >= 0)
        {
            continue;
        }

        tOffset = sPrev->t;

        // Take the largest of 2*maxtime or 100. It's simply an artificial
        // limit to the number of times to iterate to break out of a loop that
        // looks infinite.
        int MAX_ITERATIONS = int(this->GetMaximumPropagationTime() * 2.);
        MAX_ITERATIONS = (MAX_ITERATIONS < 100) ? 100 : MAX_ITERATIONS;

        for(i = 1; 
            i < this->Streamers[ptId].GetNumberOfPoints() && sPtr->cellId >= 0;
            i++, sPrev=sPtr, sPtr=this->Streamers[ptId].GetStreamPoint(i))
        {
            //
            // Create points for line
            //
            int iterations = 0;
            while(tOffset >= sPrev->t &&
                  tOffset < sPtr->t &&
                  iterations < MAX_ITERATIONS)
            {
                r = (tOffset - sPrev->t) / (sPtr->t - sPrev->t);

                for (j=0; j<3; j++)
                {
                    x[j] = sPrev->x[j] + r * (sPtr->x[j] - sPrev->x[j]);
                    v[j] = sPrev->v[j] + r * (sPtr->v[j] - sPrev->v[j]);
                }

                // add point to line
                id = newPts->InsertNextPoint(x);
                pts->InsertNextId(id);
                newVectors->InsertTuple(id,v);

                if(newScalars) 
                {
                    s = sPrev->s + r * (sPtr->s - sPrev->s);
                    newScalars->InsertTuple(id,&s);
                }

#ifdef VORTICITY
                if(this->Vorticity)
                {
                    // Store the rotation values. Used after all the
                    // streamlines are generated.
                    theta = sPrev->theta + r * (sPtr->theta - sPrev->theta);
                    rotation->InsertTuple(id, &theta);
                }
#endif

                tOffset += this->StepLength;
                ++iterations;
            } // while
        } //    for this streamer

        if(pts->GetNumberOfIds() > 1)
        {
            newLines->InsertNextCell(pts);
            pts->Reset();
        }
    } //  for all streamers

    vtkDebugMacro(<<"Created " << newPts->GetNumberOfPoints() << " points, "
                  << newLines->GetNumberOfCells() << " lines");

#ifdef VORTICITY
    if(this->Vorticity)
    {
        // Rotate the normal vectors with stream vorticity
        int nPts=newPts->GetNumberOfPoints();
        double normal[3], local1[3], local2[3], length, costheta, sintheta;

        lineNormalGenerator->GenerateSlidingNormals(newPts,newLines,normals);
    
        for(i=0; i<nPts; i++)
        {
            normals->GetTuple(i, normal);
            newVectors->GetTuple(i, v);
      
            // obtain two unit orthogonal vectors on the plane perpendicular to
            // the streamline
            for(j=0; j<3; j++)
                local1[j] = normal[j];
            length = vtkMath::Normalize(local1);
            vtkMath::Cross(local1, v, local2);
            vtkMath::Normalize(local2);
            // Rotate the normal with theta
            rotation->GetTuple(i, &theta);
            costheta = cos(theta);
            sintheta = sin(theta);
            for(j=0; j<3; j++)
            {
                normal[j] = length * (costheta*local1[j] + sintheta*local2[j]);
            }
            normals->SetTuple(i, normal);
        }
        output->GetPointData()->SetNormals(normals);
        normals->Delete();
        lineNormalGenerator->Delete();
        rotation->Delete();
    }
#endif

    output->SetPoints(newPts);
    newPts->Delete();

    output->GetPointData()->SetVectors(newVectors);
    newVectors->Delete();

    if(newScalars) 
    {
        output->GetPointData()->SetScalars(newScalars);
        newScalars->Delete();
    }

    pts->Delete();
    output->SetLines(newLines);
    newLines->Delete();

    // Delete the streamers since they are no longer needed
    delete [] this->Streamers;
    this->Streamers = 0;
    this->NumberOfStreamers = 0;

    output->Squeeze();
}

void vtkVisItStreamLine::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    os << indent << "Step Length: " << this->StepLength << "\n";
}


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Fri Feb 15 11:58:10 PST 2008
//    Fix memory leak in error condition.
//   
// ****************************************************************************

VTK_THREAD_RETURN_TYPE
vtkVisItStreamLine::ThreadedIntegrate( void *arg)
{
    vtkVisItStreamLine       *self;
    int                      thread_count;
    int                      thread_id;
    vtkStreamer::StreamArray           *streamer;
    vtkStreamer::StreamPoint           *sNext = 0, *sPtr;
    vtkStreamer::StreamPoint           pt1, pt2;
    int                      i;
    vtkIdType                idxNext, ptId;
    double                    d, step, dir;
    double                    xNext[3], vel[3], *cellVel, derivs[9];
    double                    *w, pcoords[3];
    double                    coords[4];
    vtkDataSet               *input;
    vtkGenericCell           *cell;
    vtkPointData             *pd;
    vtkDataArray             *inScalars;
    vtkDataArray             *inVectors;
    vtkDoubleArray           *cellVectors;
    vtkDataArray             *cellScalars=0;
    double tOffset, vort[3];
    double err;
    int nSavePts = 0, counter=0;

    thread_id = ((vtkMultiThreader::ThreadInfo *)(arg))->ThreadID;
    thread_count = ((vtkMultiThreader::ThreadInfo *)(arg))->NumberOfThreads;
    self = (vtkVisItStreamLine *)(((vtkMultiThreader::ThreadInfo *)(arg))->UserData);

    input     = vtkDataSet::SafeDownCast(self->GetInput());
    pd        = input->GetPointData();
    inScalars = pd->GetScalars();
    inVectors = pd->GetVectors();

    cell = vtkGenericCell::New();
    cellVectors = vtkDoubleArray::New();
    cellVectors->SetNumberOfComponents(3);
    cellVectors->Allocate(3*VTK_CELL_SIZE);
    if (inScalars)
    {
        cellScalars = inScalars->NewInstance();
        cellScalars->SetNumberOfComponents(inScalars->GetNumberOfComponents());
        cellScalars->Allocate(inScalars->GetNumberOfComponents()*VTK_CELL_SIZE);
    }

    // Set the function set to be integrated
    vtkInterpolatedVelocityField* func = vtkInterpolatedVelocityField::New();
    func->AddDataSet(input);

    if (self->GetIntegrator() == 0)
    {
        vtkGenericWarningMacro("No integrator is specified.");
        return VTK_THREAD_RETURN_VALUE;
    }

    w = new double[input->GetMaxCellSize()];

    // Create a new integrator, the type is the same as Integrator
    vtkInitialValueProblemSolver* integrator = self->GetIntegrator()->NewInstance();
    integrator->SetFunctionSet(func);

    // Used to avoid calling these function many times during
    // the integration
    double termspeed = self->GetTerminalSpeed();
    double maxtime = self->GetMaximumPropagationTime();
    double savePointInterval = self->GetSavePointInterval();

    // Take the largest of 2*maxtime or 100. It's simply an artificial
    // limit to the number of times to iterate to break out of a loop that
    // looks infinite.
    int MAX_ITERATIONS = int(maxtime * 2.);
    MAX_ITERATIONS = (MAX_ITERATIONS < 100) ? 100 : MAX_ITERATIONS;

    // For each streamer, integrate in appropriate direction
    // Do only the streamers that this thread should handle.
    for (ptId=0; ptId < self->GetNumberOfStreamers(); ptId++)
    {
        if(ptId % thread_count == thread_id)
        {
            // Get starting step
            streamer = self->GetStreamers() + ptId;
            sPtr = streamer->GetStreamPoint(0);
            if(sPtr->cellId < 0)
            {
                continue;
            }

            // Set the last cell id in the vtkInterpolatedVelocityField
            // object to speed up FindCell calls
            func->SetLastCellId(sPtr->cellId);

            dir = streamer->Direction;

            // Copy the first point
            pt1 = *sPtr;
            pt2 = *sPtr;
            tOffset = pt1.t;

            //integrate until time has been exceeded
            int iterations = 0;
            while(pt1.cellId >= 0 &&
                  pt1.speed > termspeed &&
                  pt1.t <  maxtime &&
                  iterations < MAX_ITERATIONS)
            {
                if(counter++ % 1000 == 0)
                {
                    if (!thread_id)
                    {
                        self->UpdateProgress((double)ptId/self->GetNumberOfStreamers()
                                             +pt1.t/maxtime/self->GetNumberOfStreamers());
                    }
                    if (self->GetAbortExecute())
                    {
                        break;
                    }
                }

                // Set the integration step to be characteristic cell length
                // time IntegrationStepLength
                input->GetCell(pt1.cellId, cell);
                step = dir * self->GetIntegrationStepLength() *
                       sqrt((double)cell->GetLength2())/pt1.speed;

                // Calculate the next step using the integrator provided
                if (integrator->ComputeNextStep(pt1.x, pt1.v, xNext, 0, step, 0, err) != 0)
                {
                    break;
                }

                for(i=0; i<3; i++)
                {
                    coords[i] = xNext[i];
                }

                // Interpolate the velocity field at coords
                if(!func->FunctionValues(coords, vel))
                {
                    break;
                }

                for(i=0; i<3; i++)
                {
                    pt2.v[i] = vel[i];
                }

                for (i=0; i<3; i++)
                {
                    pt2.x[i] = xNext[i];
                }
        
                pt2.cellId = func->GetLastCellId();
                func->GetLastWeights(w);
                func->GetLastLocalCoordinates(pcoords);
                input->GetCell(pt2.cellId, cell);
        
                if(inScalars)
                {
                    // Interpolate scalars
                    inScalars->GetTuples(cell->PointIds, cellScalars);
                    for (pt2.s=0.0, i=0; i < cell->GetNumberOfPoints(); i++)
                    {
                        pt2.s += cellScalars->GetComponent(i,0) * w[i];
                    }
                }

                pt2.speed = vtkMath::Norm(pt2.v);

                d = sqrt((double)vtkMath::Distance2BetweenPoints(pt1.x,pt2.x));
                pt2.d = pt1.d + d;
                // If at stagnation region, stop the integration
                if(d == 0 || (pt1.speed + pt2.speed) < VTK_STREAMLINE_EPSILON)
                {
                    pt2.t = pt1.t;
                    break;
                }
                pt2.t = pt1.t + (2.0 * d / (pt1.speed + pt2.speed));

                if (self->GetVorticity() && inVectors)
                {
                     // compute vorticity
                     inVectors->GetTuples(cell->PointIds, cellVectors);
      
                     cellVel = cellVectors->GetPointer(0);
                     cell->Derivatives(0, pcoords, cellVel, 3, derivs);
                     vort[0] = derivs[7] - derivs[5];
                     vort[1] = derivs[2] - derivs[6];
                     vort[2] = derivs[3] - derivs[1];
                     // rotation
                     pt2.omega = vtkMath::Dot(vort, pt2.v);
                     pt2.omega /= pt2.speed;
                     pt2.theta += (pt1.omega+pt2.omega)/2 * (pt2.t - pt1.t);
                }
        
                // Store only points which have a point to be displayed
                // between them
                if (tOffset >= pt1.t && tOffset <= pt2.t)
                {
                     // Do not store if same as the last point.
                     // To avoid storing some points twice.
                    if(!sNext || sNext->x[0] != pt1.x[0] || sNext->x[1] != pt1.x[1]
                         || sNext->x[2] != pt1.x[2])
                    {
                        idxNext = streamer->InsertNextStreamPoint();
                        sNext = streamer->GetStreamPoint(idxNext);
                        *sNext = pt1;
                        nSavePts++;
                    }
                    idxNext = streamer->InsertNextStreamPoint();
                    sNext = streamer->GetStreamPoint(idxNext);
                    *sNext = pt2;
                    nSavePts++;
                }

                if (tOffset < pt2.t)
                {
                    tOffset += ((int)(( pt2.t - tOffset) / savePointInterval) + 1) *
                               savePointInterval;
                }
      
                pt1 = pt2;

                ++iterations;
            } 

            // Store the last point anyway.
            if(!sNext || sNext->x[0] != pt2.x[0] || sNext->x[1] != pt2.x[1]
                || sNext->x[2] != pt2.x[2])
            {
                idxNext = streamer->InsertNextStreamPoint();
                sNext = streamer->GetStreamPoint(idxNext);
                *sNext = pt2;
                nSavePts++;
            }

            // Clear the last cell to avoid starting a search from
            // the last point in the streamline
            func->ClearLastCellId();
        }
    }

    integrator->Delete();
    func->Delete();

    cell->Delete();
    cellVectors->Delete();

    if (cellScalars)
    {
        cellScalars->Delete();
    }
    delete[] w;

    return VTK_THREAD_RETURN_VALUE;
}

void vtkVisItStreamLine::Integrate()
{
    vtkDataSet *input = vtkDataSet::SafeDownCast(this->GetInput());
    vtkDataSet *source = this->GetSource();
    vtkPointData *pd=input->GetPointData();
    vtkDataArray *inScalars;
    vtkDataArray *inVectors;
    vtkIdType numSourcePts, idx, idxNext;
    vtkStreamer::StreamPoint *sNext, *sPtr;
    vtkIdType ptId, i;
    int j, offset;
    vtkCell *cell;
    double *v, *cellVel, derivs[9], xNext[3], vort[3];
    double tol2;
    double *w=new double[input->GetMaxCellSize()];
    vtkDoubleArray *cellVectors;
    vtkDataArray *cellScalars=0;

    vtkDebugMacro(<<"Generating streamers");
    this->NumberOfStreamers = 0;

    // reexecuting - delete old stuff
    delete [] this->Streamers;
    this->Streamers = NULL;

    if(! (inVectors=pd->GetVectors()))
    {
        delete [] w;
        vtkErrorMacro(<<"No vector data defined!");
        return;
    }

    cellVectors = vtkDoubleArray::New();
    cellVectors->SetNumberOfComponents(3);
    cellVectors->Allocate(3*VTK_CELL_SIZE);

    inScalars = pd->GetScalars();

    if (inScalars)
    {
        cellScalars = inScalars->NewInstance();
        cellScalars->SetNumberOfComponents(inScalars->GetNumberOfComponents());
        cellScalars->Allocate(cellScalars->GetNumberOfComponents()*VTK_CELL_SIZE);
    }
  
    tol2 = input->GetLength()/1000; 
    tol2 = tol2*tol2;

    //
    // Create starting points
    //
    this->NumberOfStreamers = numSourcePts = offset = 1;
    if(this->GetSource())
    {
        this->NumberOfStreamers = numSourcePts = source->GetNumberOfPoints();
    }
 
    if(this->IntegrationDirection == VTK_INTEGRATE_BOTH_DIRECTIONS)
    {
        offset = 2;
        this->NumberOfStreamers *= 2;
    }

    this->Streamers = new vtkStreamer::StreamArray[this->NumberOfStreamers];

    if(this->StartFrom == VTK_START_FROM_POSITION && !this->GetSource())
    {
        idx = this->Streamers[0].InsertNextStreamPoint();
        sPtr = this->Streamers[0].GetStreamPoint(idx);
        sPtr->subId = 0;
        for (i=0; i<3; i++)
        {
            sPtr->x[i] = this->StartPosition[i];
        }
        sPtr->cellId = input->FindCell(this->StartPosition, NULL, -1, 0.0, 
                                       sPtr->subId, sPtr->p, w);
    }
    else if(this->StartFrom == VTK_START_FROM_LOCATION && !this->GetSource())
    {
        idx = this->Streamers[0].InsertNextStreamPoint();
        sPtr = this->Streamers[0].GetStreamPoint(idx);
        sPtr->subId = 0;
        cell =  input->GetCell(sPtr->cellId);
        cell->EvaluateLocation(sPtr->subId, sPtr->p, sPtr->x, w);
    }
    else //VTK_START_FROM_SOURCE
    {
        for (ptId=0; ptId < numSourcePts; ptId++)
        {
            idx = this->Streamers[offset*ptId].InsertNextStreamPoint();
            sPtr = this->Streamers[offset*ptId].GetStreamPoint(idx);
            sPtr->subId = 0;
            source->GetPoint(ptId,sPtr->x);
            sPtr->cellId = input->FindCell(sPtr->x, NULL, -1, tol2,
                                           sPtr->subId, sPtr->p, w);
        }
    }

    //
    // Finish initializing each streamer
    //
    for (idx=0, ptId=0; ptId < numSourcePts; ptId++)
    {
        this->Streamers[offset*ptId].Direction = 1.0;
        sPtr = this->Streamers[offset*ptId].GetStreamPoint(idx);
        sPtr->d = 0.0;
        sPtr->t = 0.0;
        sPtr->s = 0.0;
        sPtr->theta = 0.0;
        sPtr->omega = 0.0;
    
        if(sPtr->cellId >= 0) //starting point in dataset
        {
            cell = input->GetCell(sPtr->cellId);
            cell->EvaluateLocation(sPtr->subId, sPtr->p, xNext, w);

            inVectors->GetTuples(cell->PointIds, cellVectors);
            sPtr->v[0]  = sPtr->v[1] = sPtr->v[2] = 0.0;
            for (i=0; i < cell->GetNumberOfPoints(); i++)
            {
                v = cellVectors->GetTuple(i);
                for (j=0; j<3; j++)
                {
                    sPtr->v[j] += v[j] * w[i];
                }
            }

            sPtr->speed = vtkMath::Norm(sPtr->v);

            if (this->GetVorticity() && inVectors)
            {
                // compute vorticity
                inVectors->GetTuples(cell->PointIds, cellVectors);
                cellVel = cellVectors->GetPointer(0);
                cell->Derivatives(0, sPtr->p, cellVel, 3, derivs);
                vort[0] = derivs[7] - derivs[5];
                vort[1] = derivs[2] - derivs[6];
                vort[2] = derivs[3] - derivs[1];
                // rotation
                sPtr->omega = vtkMath::Dot(vort, sPtr->v);
                sPtr->omega /= sPtr->speed;
                sPtr->theta = 0;
            }

            if(inScalars) 
            {
                inScalars->GetTuples(cell->PointIds, cellScalars);
                for (sPtr->s=0, i=0; i < cell->GetNumberOfPoints(); i++)
                {
                    sPtr->s += cellScalars->GetComponent(i,0) * w[i];
                }
            }
        }
        else
        {
            for (j=0; j<3; j++)
            {
                sPtr->p[j] = 0.0;
                sPtr->v[j] = 0.0;
            }
            sPtr->speed = 0;
        }

        if(this->IntegrationDirection == VTK_INTEGRATE_BOTH_DIRECTIONS)
        {
            this->Streamers[offset*ptId+1].Direction = -1.0;
            idxNext = this->Streamers[offset*ptId+1].InsertNextStreamPoint();
            sNext = this->Streamers[offset*ptId+1].GetStreamPoint(idxNext);
            sPtr = this->Streamers[offset*ptId].GetStreamPoint(idx);
            *sNext = *sPtr;
        }
        else if(this->IntegrationDirection == VTK_INTEGRATE_BACKWARD)
        {
            this->Streamers[offset*ptId].Direction = -1.0;
        }
    } //for each streamer

    // Some data access methods must be called once from a single thread before they
    // can safely be used. Call those now
    vtkGenericCell *gcell = vtkGenericCell::New();
    input->GetCell(0,gcell);
    gcell->Delete();

#if 0
    // Set up and execute the thread
    this->Threader->SetNumberOfThreads( this->NumberOfThreads);
    this->Threader->SetSingleMethod( vtkVisItStreamLine::ThreadedIntegrate, (void *)this);
    this->Threader->SingleMethodExecute();
#else
    // Don't do it multithreaded.
    this->NumberOfThreads = 1;
    vtkMultiThreader::ThreadInfo t;
    t.ThreadID = 0;
    t.NumberOfThreads = 1;
    t.UserData = (void *)this;
    ThreadedIntegrate((void *)&t);
#endif

    //
    // Now create appropriate representation
    //
    if(this->OrientationScalars && !this->SpeedScalars)
    {
        for (ptId=0; ptId < this->NumberOfStreamers; ptId++)
        {
            for(sPtr=this->Streamers[ptId].GetStreamPoint(0), i=0; 
                i < this->Streamers[ptId].GetNumberOfPoints() && sPtr->cellId >= 0; 
                i++, sPtr=this->Streamers[ptId].GetStreamPoint(i))
            {
                sPtr->s = sPtr->theta;
            }
        }
    }

    if(this->SpeedScalars)
    {
        for (ptId=0; ptId < this->NumberOfStreamers; ptId++)
        { 
            for(sPtr=this->Streamers[ptId].GetStreamPoint(0), i=0; 
                i < this->Streamers[ptId].GetNumberOfPoints() && sPtr->cellId >= 0; 
                i++, sPtr=this->Streamers[ptId].GetStreamPoint(i))
            {
                sPtr->s = sPtr->speed;
            }
        }
    }

    delete [] w;
    cellVectors->Delete();
    if (cellScalars)
    {
        cellScalars->Delete();
    }
}
