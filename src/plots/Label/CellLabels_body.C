// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  Modifications:
//    Hank Childs, Wed Jul 20 17:21:46 PDT 2005
//    Add support for tensors, arrays.
//
//    Brad Whitlock, Tue Aug 2 15:49:22 PST 2005
//    Added support for materials and subsets.
//
//    Brad Whitlock, Thu Dec 13 15:10:26 PST 2007
//    Added support for cell origin.
//
//    Cyrus Harrison, Fri Aug 14 11:10:03 PDT 2009
//    Expanded use of 'Treat As ASCII' to handle label string case, removed
//    use of avtLabelVariableSize.

// ****************************************************************************

//
// Make sure that CREATE_LABEL defaults to snprintf if we have not
// defined it.
//
#ifndef CREATE_LABEL
#define CREATE_LABEL snprintf
#endif

    vtkIdType     nCells = input->GetNumberOfCells();
    vtkIdType     skipIncrement = 1;

    // By default, the max number of rows is 1.
    maxLabelRows = 1;

    //
    // Look for the original cell number array.
    //
    vtkUnsignedIntArray  *originalCells = 0;
    vtkDataArray *data = input->GetCellData()->GetArray("LabelFilterOriginalCellNumbers");
    if(data == 0 && atts.GetVarType() == LabelAttributes::LABEL_VT_VECTOR_VAR)
    {
        data = input->GetCellData()->GetVectors();
        debug3 << "CellLabels_body looking for a vector variable." << endl;
    }
    if(data == 0)
    {
        debug3 << "CellLabels_body could not find LabelFilterOriginalCellNumbers" << endl;
    }
    else if(!data->IsA("vtkUnsignedIntArray"))
    {
        debug3 << "CellLabels_body found LabelFilterOriginalCellNumbers but it "
                  "was not a vtkUnsignedIntArray. It was a " << data->GetClassName() << endl;
    }
    else
    {
        debug3 << "CellLabels_body setting originalCells=data." << endl;
        originalCells = (vtkUnsignedIntArray *)data;
    }

    //
    // Look for the variable that we want to plot.
    //
    data = input->GetCellData()->GetArray(this->VarName.c_str());

    vtkUnsignedIntArray *logIndices = vtkUnsignedIntArray::SafeDownCast(
        input->GetCellData()->GetArray("LabelFilterCellLogicalIndices"));

    if(this->UseGlobalLabel)
    {
        debug3 << "CellLabels_body using global label." << endl;
        const char *gl = this->GlobalLabel.c_str();
        for(vtkIdType id = 0; id < nCells; id += skipIncrement)
        {
            BEGIN_LABEL
                CREATE_LABEL(labelString, this->MaxLabelSize, "%s", gl);
            END_LABEL
        }
    }
    else if(data != 0)
    {

        int numElements = data->GetNumberOfTuples();

        if(numElements != nCells)
        {
            debug3 << "The number of vectors is: " << numElements
                 << ", while the #cells is: " << nCells << endl;
        }

        if(this->TreatAsASCII)
        {
            int labelLength = data->GetNumberOfComponents();

            debug3 << "Labelling cells with label data using labelLength of " << labelLength << endl;

            if(labelLength == 1)  // handle the single char case
            {
                for(vtkIdType id = 0; id < nCells; id += skipIncrement)
                {
                    BEGIN_LABEL
                        unsigned char scalarVal = (unsigned char)data->GetTuple1(id);
                        CREATE_LABEL(labelString, this->MaxLabelSize, "%c", scalarVal);
                    END_LABEL
                }
            }
            else if(data->IsA("vtkUnsignedCharArray") || data->IsA("vtkCharArray"))
            {
                unsigned char *label = (unsigned char *)data->GetVoidPointer(0);
                for(vtkIdType id = 0; id < nCells; id += skipIncrement)
                {
                    BEGIN_LABEL
                        CREATE_LABEL(labelString, this->MaxLabelSize, "%s", label);
                    END_LABEL

                    label += labelLength;
                }
            }
            else
            { // VisIt turned the label into floats!
                debug3 << "*** WARNING - VisIt transformed the Label data from unsigned char. That is not efficient!" << endl;
                char *tempstr = new char[labelLength];
                memset(tempstr, 0, labelLength);
                for(vtkIdType id = 0; id < nCells; id += skipIncrement)
                {
                    // Store the float-ified label in a real string.
                    for(int k = 0; k < labelLength - 1; ++k)
                        tempstr[k] = static_cast<char>(data->GetComponent(id, k));

                    // Use the label.
                    BEGIN_LABEL
                        CREATE_LABEL(labelString, this->MaxLabelSize, "%s", tempstr);
                    END_LABEL
                }
                delete [] tempstr;
            }
        }
        else if(data->GetNumberOfComponents() == 1)
        {
            debug3 << "Labelling cells with scalar data" << endl;

            for(vtkIdType id = 0; id < nCells; id += skipIncrement)
            {
                // float *vert = cellCenters->GetTuple3(id);
                BEGIN_LABEL
                    double scalarVal = data->GetTuple1(id);
                    CREATE_LABEL(labelString, this->MaxLabelSize, atts.GetFormatTemplate().c_str(), scalarVal);
                END_LABEL
            }
        }
        else if(data->GetNumberOfComponents() == 2)
        {
            // NOTE: HRC -- I don't think this case ever happens, since we
            // leave it as a 3D vector.  Should we be using the spatial dim
            // of the input?
            debug3 << "Labelling cells with 2d vector data" << endl;
            char *tmp = (char *)malloc(strlen(atts.GetFormatTemplate().c_str())*2 + 5);
            sprintf(tmp, "<%s, %s>", atts.GetFormatTemplate().c_str(), atts.GetFormatTemplate().c_str());

            for(vtkIdType id = 0; id < nCells; id += skipIncrement)
            {
                BEGIN_LABEL
                    double *vectorVal = data->GetTuple2(id);
                    CREATE_LABEL(labelString, this->MaxLabelSize, tmp,
                             vectorVal[0], vectorVal[1]);
                END_LABEL
            }
            free(tmp);
        }
        else if(data->GetNumberOfComponents() == 3)
        {
            debug3 << "Labelling cells with 3d vector data" << endl;
            char *tmp = (char *)malloc(strlen(atts.GetFormatTemplate().c_str())*3 + 7);
            sprintf(tmp, "<%s, %s, %s>", 
                atts.GetFormatTemplate().c_str(), 
                atts.GetFormatTemplate().c_str(), 
                atts.GetFormatTemplate().c_str());

            for(vtkIdType id = 0; id < nCells; id += skipIncrement)
            {
                BEGIN_LABEL
                    double *vectorVal = data->GetTuple3(id);
                    CREATE_LABEL(labelString, this->MaxLabelSize, tmp,
                             vectorVal[0], vectorVal[1], vectorVal[2]);
                END_LABEL
            }
            free(tmp);
        }
        else if(data->GetNumberOfComponents() == 9)
        {
            debug3 << "Labelling cells with 3d tensor data" << endl;
            char *tmp = (char *)malloc(strlen(atts.GetFormatTemplate().c_str())*9 + 23);
            sprintf(tmp, "(%s, %s, %s)\n(%s, %s, %s)\n(%s, %s, %s)", 
                atts.GetFormatTemplate().c_str(), 
                atts.GetFormatTemplate().c_str(), 
                atts.GetFormatTemplate().c_str(), 
                atts.GetFormatTemplate().c_str(), 
                atts.GetFormatTemplate().c_str(), 
                atts.GetFormatTemplate().c_str(), 
                atts.GetFormatTemplate().c_str(), 
                atts.GetFormatTemplate().c_str(), 
                atts.GetFormatTemplate().c_str());

            maxLabelRows = 3;
            for(vtkIdType id = 0; id < nCells; id += skipIncrement)
            {
                BEGIN_LABEL
                    double *tensorVal = data->GetTuple9(id);
                    CREATE_LABEL(labelString, this->MaxLabelSize, tmp,
                             tensorVal[0], tensorVal[1], tensorVal[2],
                             tensorVal[3], tensorVal[4], tensorVal[5],
                             tensorVal[6], tensorVal[7], tensorVal[8]);
                END_LABEL
            }
            free(tmp);
        }
        else
        {
            debug3 << "Labelling cells with arbitrary array data" << endl;
            int nComps = data->GetNumberOfComponents();
            int row_size = 1;
            while (row_size*row_size < nComps)
                row_size++;
            maxLabelRows = row_size;

            char *formatStringStart  = (char *)malloc( strlen(atts.GetFormatTemplate().c_str())+4 );
            char *formatStringMiddle = (char *)malloc( strlen(atts.GetFormatTemplate().c_str())+3 );
            char *formatStringEnd    = (char *)malloc( strlen(atts.GetFormatTemplate().c_str())+4 );
            char *formatStringLast   = (char *)malloc( strlen(atts.GetFormatTemplate().c_str())+2 );

            sprintf(formatStringStart,  "(%s, ", atts.GetFormatTemplate().c_str());
            sprintf(formatStringMiddle, "%s, ",  atts.GetFormatTemplate().c_str());
            sprintf(formatStringEnd,    "%s)\n", atts.GetFormatTemplate().c_str());
            sprintf(formatStringLast,   "%s)",   atts.GetFormatTemplate().c_str());

            for (vtkIdType id = 0 ; id < nCells ; id += skipIncrement)
            {
                BEGIN_LABEL
                    labelString[0] = '\0';
                    double *vals = data->GetTuple(id);

                    for (int comp = 0 ; comp < nComps ; comp++)
                    {
                        char *formatString = NULL;
                        if ((comp % row_size) == 0)
                            formatString = formatStringStart;
                        else if (comp == nComps-1)
                            formatString = formatStringLast;
                        else if ((comp % row_size) == row_size-1)
                            formatString = formatStringEnd;
                        else
                            formatString = formatStringMiddle;
                        CREATE_LABEL(labelString + strlen(labelString),
                                     this->MaxLabelSize-strlen(labelString),
                                     formatString, vals[comp]);
                    }
                END_LABEL
            }
            free(formatStringStart);
            free(formatStringMiddle);
            free(formatStringEnd);
            free(formatStringLast);
        }
    }
    else if(logIndices != 0 && 
            atts.GetLabelDisplayFormat() != LabelAttributes::Index)
    {
debug3 << "Labelling cells with logical Indices: " << endl;
        const unsigned int *intptr = (const unsigned int*)logIndices->GetVoidPointer(0);
        int nc = logIndices->GetNumberOfComponents();
        if (nc == 2)
        {
            for(vtkIdType id = 0; id < nCells; id += skipIncrement)
            {
                BEGIN_LABEL
                CREATE_LABEL(labelString, this->MaxLabelSize, "%d,%d", intptr[id*2+0], intptr[id*2+1]);
                END_LABEL
            }
        }
        else
        {
            for(vtkIdType id = 0; id < nCells; id += skipIncrement)
            {
                BEGIN_LABEL
                CREATE_LABEL(labelString, this->MaxLabelSize, "%d,%d,%d", intptr[id*3+0], intptr[id*3+1], intptr[id*3+2]);
                END_LABEL
            }
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
        xbase -= this->CellOrigin;
        ybase -= this->CellOrigin;
        zbase -= this->CellOrigin;

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
debug3 << "Labelling cells as 2D structured indices" << endl;
                for(vtkIdType id = 0; id < nCells; id += skipIncrement)
                {
                    BEGIN_LABEL
                        unsigned int realCellId = originalCells->GetValue(id);
                        unsigned int y = (realCellId / xdims) - ybase;
                        unsigned int x = (realCellId % xdims) - xbase;
                        CREATE_LABEL(labelString, this->MaxLabelSize, "%d,%d", x, y);
                    END_LABEL
                }
            }
            else
            {
debug3 << "Labelling cells as 3D structured indices" << endl;
                unsigned int xydims = xdims * ydims;
                for(vtkIdType id = 0; id < nCells; id += skipIncrement)
                {
                    BEGIN_LABEL
                        unsigned int realCellId = originalCells->GetValue(id);
                        unsigned int z = (realCellId / xydims) - zbase;
                        unsigned int offset = realCellId % xydims;
                        unsigned int y = (offset / xdims) - ybase;
                        unsigned int x = (offset % xdims) - xbase;
                        CREATE_LABEL(labelString, this->MaxLabelSize, "%d,%d,%d", x, y, z);
                    END_LABEL
                }
            }
        }
        else
        {
debug3 << "Labelling as indices" << endl;
            for(vtkIdType id = 0; id < nCells; id += skipIncrement)
            {
                BEGIN_LABEL
                    unsigned int realCellId = originalCells->GetValue(id);
                CREATE_LABEL(labelString, this->MaxLabelSize, "%lld", realCellId + this->CellOrigin);
                END_LABEL
            }
        }
    }
    else
    {
        debug3 << "CellLabels_body: backup case for labelling cells." << endl;
        for(vtkIdType id = 0; id < nCells; id += skipIncrement)
        {
            BEGIN_LABEL
                CREATE_LABEL(labelString, this->MaxLabelSize, "%lld", id + this->CellOrigin);
            END_LABEL
        }
    }
