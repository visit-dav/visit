    int numPts;
    vtkPolyData *input= vtkPolyData::SafeDownCast(this->GetInput());
    int j;
    vtkPoints *p, *displayPts;
    vtkUnsignedCharArray *c=NULL;
    unsigned char *rgba;
    double *ftmp;
    int cellScalars = 0;
    int cellNum = 0;
    int lastX, lastY, X, Y;
    int currSize = 1024;
    vtkDebugMacro (<< "vtkRubberBandMapper2D::RenderOverlay");

    if ( input == NULL )
    {
        vtkErrorMacro(<< "No input!");
        CLEAN_UP();
        return;
    }
    else
    {
        numPts = input->GetNumberOfPoints();
    }

    if (numPts == 0)
    {
        vtkDebugMacro(<< "No points!");
        CLEAN_UP();
        return;
    }

    if ( this->LookupTable == NULL )
    {
        this->CreateDefaultLookupTable();
    }

    //
    // if something has changed regenrate colors and display lists
    // if required
    //
    if ( this->GetMTime() > this->BuildTime ||
         input->GetMTime() > this->BuildTime ||
         this->LookupTable->GetMTime() > this->BuildTime ||
         actor->GetProperty()->GetMTime() > this->BuildTime)
    {
         // sets this->Colors as side effect
        this->MapScalars(1.0);
        this->BuildTime.Modified();
    }

    // Get the position of the text actor
    int* actorPos =
        actor->GetPositionCoordinate()->GetComputedLocalDisplayValue(viewport);

    // Transform the points, if necessary
    p = input->GetPoints();
    if ( this->TransformCoordinate )
    {
        int *itmp;
        numPts = p->GetNumberOfPoints();
        displayPts = vtkPoints::New();
        displayPts->SetNumberOfPoints(numPts);
        for ( j=0; j < numPts; j++ )
        {
            this->TransformCoordinate->SetValue(p->GetPoint(j));
            itmp = this->TransformCoordinate->GetComputedDisplayValue(viewport);
            displayPts->SetPoint(j, itmp[0], itmp[1], 0.0);
        }
        p = displayPts;
    }

    // Get colors
    if ( this->Colors )
    {
        c = this->Colors;
        if (!input->GetPointData()->GetScalars())
        {
            cellScalars = 1;
        }
    }

    // Draw the polygons.
    vtkIdType npts;
    auto aPrim = vtk::TakeSmartPointer(input->GetPolys()->NewIterator());
    const vtkIdType *pts;
    for (aPrim->GoToFirstCell(); !aPrim->IsDoneWithTraversal(); aPrim->GoToNextCell(), cellNum++)
    {
        aPrim->GetCurrentCell(npts,pts);
        if (c)
        {
            if (cellScalars)
                rgba = c->GetPointer(4*cellNum);
            else
                rgba = c->GetPointer(4*pts[0]);

            SET_FOREGROUND(rgba);
        }

        RESIZE_POINT_ARRAY(points, npts, currSize);

        for (j = 0; j < npts; j++)
        {
            ftmp = p->GetPoint(pts[j]);
            STORE_POINT(points[j],
                        actorPos[0] + ftmp[0],
                        actorPos[1] - ftmp[1]);
        }

        DRAW_POLYGON(points, npts);
    }

    //
    // Draw the lines.
    // We need to scale our coordinates by the devicePixelRatio, which takes
    // the OSX retina display into account. From the docs:
    //
    //     "Common values are 1 for normal-dpi displays and 2 for high-dpi
    //     'retina' displays."
    //
    int devicePixelRatio = privateInstance->widget->devicePixelRatio();
    aPrim = vtk::TakeSmartPointer(input->GetLines()->NewIterator());
    for (aPrim->GoToFirstCell(); !aPrim->IsDoneWithTraversal(); aPrim->GoToNextCell(), cellNum++)
    {
        aPrim->GetCurrentCell(npts,pts);
        if (c && cellScalars)
        {
            rgba = c->GetPointer(4*cellNum);
            SET_FOREGROUND(rgba);
        }
        ftmp = p->GetPoint(pts[0]);

        lastX = (int)(actorPos[0] + ftmp[0]) / devicePixelRatio;
        lastY = (int)(actorPos[1] - ftmp[1]) / devicePixelRatio;
        BEGIN_POLYLINE(lastX, lastY);

        for (j = 1; j < npts; j++)
        {
            ftmp = p->GetPoint(pts[j]);
            if (c && !cellScalars)
            {
                rgba = c->GetPointer(4*pts[j]);
                SET_FOREGROUND(rgba)
            }
            X = (int)(actorPos[0] + ftmp[0]) / devicePixelRatio;
            Y = (int)(actorPos[1] - ftmp[1]) / devicePixelRatio;

            DRAW_XOR_LINE(lastX, lastY, X, Y);

            lastX = X;
            lastY = Y;
        }

        END_POLYLINE();
    }

    // Finish drawing.
    FLUSH_AND_SYNC();

    // Clean up.
    CLEAN_UP();
    if ( this->TransformCoordinate )
        p->Delete();

#undef STORE_POINT
#undef DRAW_POLYGON
#undef RESIZE_POINT_ARRAY
#undef SET_FOREGROUND_D
#undef SET_FOREGROUND
#undef DRAW_XOR_LINE
#undef FLUSH_AND_SYNC
#undef CLEAN_UP
