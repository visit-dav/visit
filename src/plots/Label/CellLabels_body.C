//
// Make sure that CREATE_LABEL defaults to SNPRINTF if we have not
// defined it.
//
#ifndef CREATE_LABEL
#define CREATE_LABEL SNPRINTF
#endif

    vtkIdType     nCells = input->GetNumberOfCells();
    vtkIdType     skipIncrement = 1;

    //
    // Look for the cell center array that the label filter calculated.
    //
    //vtkFloatArray *cellCenters = GetCellCenterArray();
    //if(cellCenters == 0)
    //    return;

    //
    // Look for the original cell number array.
    //
    vtkUnsignedIntArray *originalCells = 0;
    vtkDataArray *data = input->GetCellData()->GetArray("LabelFilterOriginalCellNumbers");
    if(data == 0)
    {
        debug3 << "avtLabelRenderer could not find LabelFilterOriginalCellNumbers" << endl;
    }
    else if(!data->IsA("vtkUnsignedIntArray"))
    {
        debug3 << "avtLabelRenderer found LabelFilterOriginalCellNumbers but it "
                  "was not a vtkUnsignedIntArray. It was a " << data->GetClassName() << endl;
    }
    else
    {
        originalCells = (vtkUnsignedIntArray *)data;
    }

    //
    // Look for the variable that we want to plot.
    //
    data = input->GetCellData()->GetArray(varname);

    if(data != 0)
    {
        int numElements = data->GetNumberOfTuples();

        if(numElements != nCells)
        {
            debug3 << "The number of vectors is: " << numElements
                 << ", while the #cells is: " << nCells << endl;
        }

        if(data->GetNumberOfComponents() == 1)
        {
debug3 << "Labelling cells with scalar data" << endl;

            if(treatAsASCII)
            {
                for(vtkIdType id = 0; id < nCells; id += skipIncrement)
                {
                    // float *vert = cellCenters->GetTuple3(id);
                    BEGIN_LABEL
                        unsigned char scalarVal = (unsigned char)data->GetTuple1(id);
                        CREATE_LABEL(labelString, MAX_LABEL_SIZE, "%c", scalarVal);
                    END_LABEL
                }
            }
            else
            {
                for(vtkIdType id = 0; id < nCells; id += skipIncrement)
                {
                    // float *vert = cellCenters->GetTuple3(id);
                    BEGIN_LABEL
                        float scalarVal = data->GetTuple1(id);
                        CREATE_LABEL(labelString, MAX_LABEL_SIZE, "%g", scalarVal);
                    END_LABEL
                }
            }
        }
        else if(data->GetNumberOfComponents() == 2)
        {
debug3 << "Labelling cells with 2d vector data" << endl;
            for(vtkIdType id = 0; id < nCells; id += skipIncrement)
            {
                // float *vert = cellCenters->GetTuple3(id);
                BEGIN_LABEL
                    float *vectorVal = data->GetTuple2(id);
                    CREATE_LABEL(labelString, MAX_LABEL_SIZE, "<%g, %g>",
                             vectorVal[0], vectorVal[1]);
                END_LABEL
            }
        }
        else if(data->GetNumberOfComponents() == 3)
        {
debug3 << "Labelling cells with 3d vector data" << endl;
            for(vtkIdType id = 0; id < nCells; id += skipIncrement)
            {
                // float *vert = cellCenters->GetTuple3(id);
                BEGIN_LABEL
                    float *vectorVal = data->GetTuple3(id);
                    CREATE_LABEL(labelString, MAX_LABEL_SIZE, "<%g, %g, %g>",
                             vectorVal[0], vectorVal[1], vectorVal[2]);
                END_LABEL
            }
        }
        else
        {
            debug3 << "The input vector has " << data->GetNumberOfComponents()
                 << " components. We don't like that!" << endl;
        }
    }
    else if(originalCells != 0)
    {
debug3 << "Labelling cells with original cell indices: "
       << "nOriginalCells:" << originalCells->GetNumberOfTuples()
       << ", nCells=" << nCells 
       << endl;

        //
        // Figure out the first real index in x,y,z. This only matters if we
        // have ghost zones and structured indices.
        //
        unsigned int xbase = 0, ybase = 0, zbase = 0;
        vtkDataArray *rDims = input->GetFieldData()->
            GetArray("avtRealDims");
        if(rDims != 0 &&
           rDims->IsA("vtkIntArray") &&
           rDims->GetNumberOfTuples() == 6)
        {
            const int *iptr2 = (const int *)rDims->GetVoidPointer(0);
            xbase = iptr2[0];
            ybase = iptr2[2];
            zbase = iptr2[4];
        }

        vtkDataArray *sDims = input->GetFieldData()->
            GetArray("avtOriginalStructuredDimensions");
        if((atts.GetLabelDisplayFormat() == LabelAttributes::Natural ||
            atts.GetLabelDisplayFormat() == LabelAttributes::LogicalIndex) &&
           sDims != 0 &&
           sDims->IsA("vtkUnsignedIntArray") &&
           sDims->GetNumberOfTuples() == 3)
        {
            //
            // Add the cell labels as structured indices.
            //
            const unsigned int *iptr = (const unsigned int *)sDims->GetVoidPointer(0);
            unsigned int xdims = iptr[0]-1;
            unsigned int ydims = iptr[1]-1;
            unsigned int zdims = iptr[2]-1;

            if(zdims == 0)
            {
                for(vtkIdType id = 0; id < nCells; id += skipIncrement)
                {
                    // float *vert = cellCenters->GetTuple3(id);
                    BEGIN_LABEL
                        unsigned int realCellId = originalCells->GetValue(id);
                        unsigned int y = (realCellId / xdims) - ybase;
                        unsigned int x = (realCellId % xdims) - xbase;
                        CREATE_LABEL(labelString, MAX_LABEL_SIZE, "%d,%d", x, y);
                    END_LABEL
                }
            }
            else
            {
                unsigned int xydims = xdims * ydims;
                for(vtkIdType id = 0; id < nCells; id += skipIncrement)
                {
                    // float *vert = cellCenters->GetTuple3(id);
                    BEGIN_LABEL
                        unsigned int realCellId = originalCells->GetValue(id);
                        unsigned int z = (realCellId / xydims) - zbase;
                        unsigned int offset = realCellId % xydims;
                        unsigned int y = (offset / xdims) - ybase;
                        unsigned int x = (offset % xdims) - xbase;
                        CREATE_LABEL(labelString, MAX_LABEL_SIZE, "%d,%d,%d", x, y, z);
                    END_LABEL
                }
            }
        }
        else
        {
            for(vtkIdType id = 0; id < nCells; id += skipIncrement)
            {
                // float *vert = cellCenters->GetTuple3(id);
                BEGIN_LABEL
                    unsigned int realCellId = originalCells->GetValue(id);
                    CREATE_LABEL(labelString, MAX_LABEL_SIZE, "%d", realCellId);
                END_LABEL
            }
        }
    }
    else
    {
        for(vtkIdType id = 0; id < nCells; id += skipIncrement)
        {
            // float *vert = cellCenters->GetTuple3(id);
            BEGIN_LABEL
                CREATE_LABEL(labelString, MAX_LABEL_SIZE, "%d", id);
            END_LABEL
        }
    }
