    int numPts;
    vtkPolyData *input= vtkPolyData::SafeDownCast(this->GetInput());
    vtkIdType npts, *pts;
    int j;
    vtkPoints *p, *displayPts;
    vtkCellArray *aPrim;
    vtkUnsignedCharArray *c=NULL;
    unsigned char *rgba;
    double *ftmp;
    int cellScalars = 0;
    int cellNum = 0;
    int lastX, lastY, X, Y; 
 
    vtkDebugMacro (<< "vtkDashedXorGridMapper2D::RenderOverlay");

    if ( input == NULL ) 
    {
        vtkErrorMacro(<< "No input!");
        CLEAN_UP();
        return;
    }
    else
    {
        input->Update();
        numPts = input->GetNumberOfPoints();
    } 

    if (numPts == 0)
    {
        vtkDebugMacro(<< "No points!");
        CLEAN_UP();
        return;
    }

    // Try and override the values we put into the line properties.
    vtkDataArray *props = input->GetFieldData()->GetArray("LineProperties");
    if(props != NULL)
    {
        SetDots((int)props->GetTuple1(0), (int)props->GetTuple1(1));
        SetHorizontalBias(props->GetTuple1(2)>0.);
    }

    if (pixelDrawn <= 0 || pixelSpaced <= 0)
    {
        vtkDebugMacro(<< "Bad settings. Try SetDots first.");
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

    // Draw the lines.
    aPrim = input->GetLines();
    for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); cellNum++)
    { 
        if (c && cellScalars) 
        {
            rgba = c->GetPointer(4*cellNum);
            SET_FOREGROUND(rgba);
        }
        ftmp = p->GetPoint(pts[0]);

        lastX = (int)(actorPos[0] + ftmp[0]);
        lastY = (int)(actorPos[1] - ftmp[1]);

        for (j = 1; j < npts; j++) 
        {
            ftmp = p->GetPoint(pts[j]);
            if (c && !cellScalars)
            {
                rgba = c->GetPointer(4*pts[j]);
                SET_FOREGROUND(rgba)
            }
            X = (int)(actorPos[0] + ftmp[0]);
            Y = (int)(actorPos[1] - ftmp[1]);

            int delta = pixelDrawn + pixelSpaced;
            // Divide the two cases.
            
            bool horizontal;
            
            // If we're asked for a point
            if (X == lastX && Y == lastY)
                horizontal = horizontalBias;
            else
                horizontal = (Y == lastY);
            
            // Horizontal line
            if (horizontal)
            {
                // Ensure we're drawing left to right
                if (X > lastX)
                {
                    int tmp = X;
                    X = lastX;
                    lastX = tmp;
                }
                
                int nextX;
                
                // Three cases for first dashed line
                // If we're on a white space, we draw nothing, and advance
                // to the next state.
                if (!IsDash(X))
                {
                    X = NextDash(X);
                }
                // If we're in the middle of a dash
                else if (!IsBeginningDash(X))
                {
                    nextX = NextDash(X);
                    nextX -= (pixelSpaced);
                    // Special case: if there's only part of a dash.
                    if (nextX > lastX)
                    {
                        // Draw the line segment to the end of line.
                        DRAW_XOR_LINE( X, Y, lastX, Y);
                    }
                    else
                    {
                        // Draw the segment
                        DRAW_XOR_LINE( X, Y, nextX, Y);
                    }
                    // Advance X to the next dash
                    X = nextX + pixelSpaced;
                }
                // If we're at the beginning of a dash, we're fine.

                nextX = X + pixelDrawn;

                for (;;)
                {
                    // If X and nextX strattle lastX, draw a final segment.
                    // End loop.
                    if ( X <= lastX && nextX >= lastX)
                    {
                        DRAW_XOR_LINE(X, Y, lastX, Y);
                        break;
                    }

                    // If X is past lastX, End Loop.
                    if ( X > lastX)
                        break;

                    // Draw the next dash
                    DRAW_XOR_LINE(X, Y, nextX, Y);
                    X += delta;
                    nextX += delta;
                }
            }

            // Vertical line
            else
            {
                // Ensure we're drawing down to up
                if (Y > lastY)
                {
                    int tmp = Y;
                    Y = lastY;
                    lastY = tmp;
                }
                
                int nextY;
                
                // Three cases for first dashed line
                // If we're on a white space, we draw nothing, and advance
                // to the next state.
                if (!IsDash(Y))
                {
                    Y = NextDash(Y);
                }
                // If we're in the middle of a dash
                else if (!IsBeginningDash(Y))
                {
                    nextY = NextDash(Y);
                    nextY -= (pixelSpaced);
                    // Special case: if there's only part of a dash.
                    if (nextY > lastY)
                    {
                        // Draw the line segment to the end of line.
                        DRAW_XOR_LINE( X, Y, X, lastY);
                    }
                    else
                    {
                        // Draw the segment
                        DRAW_XOR_LINE( X, Y, X, nextY);
                    }
                    // Advance Y to the next dash
                    Y = nextY + pixelSpaced;
                }
                // If we're at the beginning of a dash, we're fine.

                nextY = Y + pixelDrawn;
                
                for (;;)
                {
                    // If Y and nextY strattle lastY, draw a final segment.
                    // End loop.
                    if ( Y <= lastY && nextY >= lastY)
                    {
                        DRAW_XOR_LINE(X, Y, X, lastY);
                        break;
                    }

                    // If Y is past lastY, End Loop.
                    if ( Y > lastY)
                        break;

                    // Draw the next dash
                    DRAW_XOR_LINE(X, Y, X, nextY);
                    Y += delta;
                    nextY += delta;
                }
            }
        }
    }

    // Finish drawing.
    FLUSH_AND_SYNC();

    // Clean up.
    CLEAN_UP();
    if ( this->TransformCoordinate )
        p->Delete();

#undef SET_FOREGROUND_D
#undef SET_FOREGROUND
#undef DRAW_XOR_LINE
#undef FLUSH_AND_SYNC
#undef CLEAN_UP
