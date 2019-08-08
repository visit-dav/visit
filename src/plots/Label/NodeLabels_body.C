// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  Modifications:
//    Hank Childs, Wed Jul 20 17:21:46 PDT 2005
//    Add support for tensors, arrays.
//
//    Brad Whitlock, Thu Aug 4 15:08:28 PST 2005
//    I made tensors and arrays set maxLabelRows so the dynamic layout can
//    do a little better.
//
//    Brad Whitlock, Thu Dec 13 15:10:54 PST 2007
//    Added support for node origin.
//
//    Cyrus Harrison, Fri Aug 14 11:10:03 PDT 2009
//    Expanded use of 'Treat As ASCII' to handle label string case, removed
//    use of avtLabelVariableSize.
//
// ****************************************************************************


//
// Make sure that CREATE_LABEL defaults to snprintf if we have not
// defined it.
//
#ifndef CREATE_LABEL
#define CREATE_LABEL snprintf
#endif

    vtkPoints    *p = vtkVisItUtility::GetPoints(input);
    vtkDataArray *data = input->GetPointData()->GetArray(this->VarName.c_str());
    vtkIdType     npts = p ? p->GetNumberOfPoints() : 0;
    vtkIdType     skipIncrement = 1;

    // By default, the max number of rows is 1.
    maxLabelRows = 1;

    //
    // If the data array is empty then try and get the node numbers so we can
    // label the node numbers using the original node numbers.
    //
    vtkIntArray *originalNodes = 0;
    if(data == 0 && atts.GetVarType() == LabelAttributes::LABEL_VT_VECTOR_VAR)
    {
        data = input->GetPointData()->GetVectors();
        debug3 << "NodeLabels_body looking for a vector variable." << endl;
    }
    if(data == 0)
    {
        vtkDataArray *tmpNodes = input->GetPointData()->GetArray("LabelFilterOriginalNodeNumbers");
        if(tmpNodes == 0)
        {
            debug3 << "NodeLabels_body could not find LabelFilterOriginalNodeNumbers" << endl;
        }
        else if(!tmpNodes->IsA("vtkIntArray"))
        {
            debug3 << "NodeLabels_body found LabelFilterOriginalNodeNumbers but it "
                      "was not a vtkIntArray. It was a " << tmpNodes->GetClassName() << endl;
        }
        else
        {
            debug3 << "NodeLabels_body setting originalNodes=data." << endl;
            originalNodes = (vtkIntArray *)tmpNodes;
        }
    }

   vtkIntArray *logIndices = vtkIntArray::SafeDownCast(
        input->GetPointData()->GetArray("LabelFilterNodeLogicalIndices"));



    if(data != 0)
    {
        int numElements = data->GetNumberOfTuples();

        if(numElements != npts)
        {
            debug3 << "The number of scalars is: " << numElements
                   << ", while the #points is: " << npts << endl;
        }

        if(this->TreatAsASCII)
        {
            debug3 << "Labelling nodes with label data" << endl;

            int labelLength = data->GetNumberOfComponents();

            if(labelLength == 1)
            {
                for(vtkIdType id = 0; id < npts; ++id)
                {
                    BEGIN_LABEL
                        unsigned char scalarVal = (unsigned char)data->GetTuple1(id);
                        CREATE_LABEL(labelString, this->MaxLabelSize, "%c", scalarVal);
                    END_LABEL
                }
            }
            else if(data->IsA("vtkUnsignedCharArray"))
            {
                unsigned char *label = (unsigned char *)data->GetVoidPointer(0);
                for(vtkIdType id = 0; id < npts; ++id)
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
                for(vtkIdType id = 0; id < npts; ++id)
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
            debug3 << "Labelling nodes with scalar data" << endl;
            for(vtkIdType id = 0; id < npts; ++id)
            {
                BEGIN_LABEL
                    double scalarVal = data->GetTuple1(id);
                    CREATE_LABEL(labelString, this->MaxLabelSize, atts.GetFormatTemplate().c_str(), scalarVal);
                END_LABEL
            }
        }
        else if(data->GetNumberOfComponents() == 2)
        {
debug3 << "Labelling nodes with 2d vector data" << endl;
            char *tmp = (char *)malloc(strlen(atts.GetFormatTemplate().c_str())*2 + 5);
            sprintf(tmp, "<%s, %s>", atts.GetFormatTemplate().c_str(), atts.GetFormatTemplate().c_str());

            for(vtkIdType id = 0; id < npts; ++id)
            {
                // const float *vert = p->GetPoint(id);
                BEGIN_LABEL 
                    double *vectorVal = data->GetTuple2(id);
                    CREATE_LABEL(labelString, this->MaxLabelSize, tmp, vectorVal[0], vectorVal[1]);
                END_LABEL
            }
            free(tmp);
        }
        else if(data->GetNumberOfComponents() == 3)
        {
debug3 << "Labelling nodes with 3d vector data" << endl;
            char *tmp = (char *)malloc(strlen(atts.GetFormatTemplate().c_str())*3 + 7);
            sprintf(tmp, "<%s, %s, %s>", 
                atts.GetFormatTemplate().c_str(), 
                atts.GetFormatTemplate().c_str(), 
                atts.GetFormatTemplate().c_str());

            for(vtkIdType id = 0; id < npts; ++id)
            {
                // const float *vert = p->GetPoint(id);
                BEGIN_LABEL
                    double *vectorVal = data->GetTuple3(id);
                    CREATE_LABEL(labelString, this->MaxLabelSize, tmp, vectorVal[0], vectorVal[1], vectorVal[2]);
                END_LABEL
            }
            free(tmp);
        }
        else if(data->GetNumberOfComponents() == 9)
        {
debug3 << "Labelling nodes with 3d tensor data" << endl;
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
            for(vtkIdType id = 0; id < npts; ++id)
            {
                // float *vert = cellCenters->GetTuple3(id);
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
            debug3 << "Labelling points with arbitrary array data" << endl;
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

            for (vtkIdType id = 0 ; id < npts ; id += skipIncrement)
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
debug3 << "Labelling nodes with logical Indices: " << endl;
        const int *intptr = (const int*)logIndices->GetVoidPointer(0);
        int nc = logIndices->GetNumberOfComponents();
        if (nc == 2)
        {
            for(vtkIdType id = 0; id < npts; id += skipIncrement)
            {
                BEGIN_LABEL
                if (intptr[id*2+0] == -1)
                    CREATE_LABEL(labelString, this->MaxLabelSize, " ");
                else
                    CREATE_LABEL(labelString, this->MaxLabelSize, "%d,%d", intptr[id*2+0], intptr[id*2+1]);
                END_LABEL
            }
        }
        else
        {
            for(vtkIdType id = 0; id < npts; id += skipIncrement)
            {
                BEGIN_LABEL
                if (intptr[id*3+0] == -1)
                    CREATE_LABEL(labelString, this->MaxLabelSize, " ");
                else
                    CREATE_LABEL(labelString, this->MaxLabelSize, "%d,%d,%d", intptr[id*3+0], intptr[id*3+1], intptr[id*3+2]);
                END_LABEL
            }
        }
    }
    else if(originalNodes != 0)
    {
debug3 << "Labelling nodes with original node indices: "
       << "nOriginalNodes:" << originalNodes->GetNumberOfTuples()
       << ", npts=" << npts
       << endl;

        //
        // Figure out the first real index in x,y,z. This only matters if we
        // have ghost zones and structured indices.
        //
        vtkDataArray *rDims = input->GetFieldData()->
            GetArray("avtRealDims");
        unsigned int xbase = 0, ybase = 0, zbase = 0;
        if(rDims != 0 &&
           rDims->IsA("vtkIntArray") &&
           rDims->GetNumberOfTuples() == 6)
        {
            const unsigned int *iptr = (const unsigned int *)rDims->GetVoidPointer(0);
            xbase = iptr[0];
            ybase = iptr[2];
            zbase = iptr[4];
        }
        xbase -= this->NodeOrigin;
        ybase -= this->NodeOrigin;
        zbase -= this->NodeOrigin;

        vtkDataArray *sDims = input->GetFieldData()->
            GetArray("avtOriginalStructuredDimensions");
        if((atts.GetLabelDisplayFormat() == LabelAttributes::Natural ||
            atts.GetLabelDisplayFormat() == LabelAttributes::LogicalIndex) &&
           sDims != 0 &&
           sDims->IsA("vtkUnsignedIntArray") &&
           sDims->GetNumberOfTuples() == 3)
        {
            //
            // Add the node labels as structured indices.
            //
            const unsigned int *iptr = (const unsigned int *)sDims->GetVoidPointer(0);
            unsigned int xdims = iptr[0];
            unsigned int ydims = iptr[1];
            unsigned int zdims = iptr[2];

            if(zdims == 1)
            {
                // 2D
                for(vtkIdType id = 0; id < npts; id += skipIncrement)
                {
                    // const float *vert = p->GetPoint(id);
                    BEGIN_LABEL
                        unsigned int realNodeId = originalNodes->GetValue(id);
                        if(realNodeId == (unsigned int)-1)
                        {
                            CREATE_LABEL(labelString, this->MaxLabelSize, " ");
                        }
                        else
                        {
                            unsigned int y = (realNodeId / xdims) - ybase;
                            unsigned int x = (realNodeId % xdims) - xbase;
                            CREATE_LABEL(labelString, this->MaxLabelSize, "%d,%d", x,y);
                        }
                    END_LABEL
                }
            }
            else
            {
                // 3D
                unsigned int xydims = xdims * ydims;
                for(vtkIdType id = 0; id < npts; id += skipIncrement)
                {
                    // const float *vert = p->GetPoint(id);
                    BEGIN_LABEL
                        unsigned int realNodeId = originalNodes->GetValue(id);
                        if(realNodeId == (unsigned int)-1)
                        {
                            CREATE_LABEL(labelString, this->MaxLabelSize, " ");
                        }
                        else
                        {
                            unsigned int z = (realNodeId / xydims) - zbase;
                            unsigned int offset = realNodeId % xydims;
                            unsigned int y = (offset / xdims) - ybase;
                            unsigned int x = (offset % xdims) - xbase;
                            CREATE_LABEL(labelString, this->MaxLabelSize, "%d,%d,%d", x,y,z);
                        }
                    END_LABEL
                }
            }
        }
        else
        {
            //
            // Add the node labels as regular indices. This is the default for
            // unstructured meshes or structured meshes where we can't find the
            // information about how many nodes there are in each dimension.
            //
            for(vtkIdType id = 0; id < npts; id += skipIncrement)
            {
                // const float *vert = p->GetPoint(id);
                BEGIN_LABEL
                    unsigned int realNodeId = originalNodes->GetValue(id);
                    if(realNodeId == (unsigned int)-1)
                    {
                        CREATE_LABEL(labelString, this->MaxLabelSize, " ");
                    }
                    else
                    {
                        CREATE_LABEL(labelString, this->MaxLabelSize, "%lld", realNodeId + this->NodeOrigin);
                    }
                END_LABEL
            }
        }
    }
    else
    {
        //
        // Add the node indices as a function of the number of points. Note that
        // we don't do any sort of lookup into the originalNodes array since it
        // was not available.
        //
        debug3 << "NodeLabels_body: backup case for labelling nodes." << endl;
        for(vtkIdType id = 0; id < npts; id += skipIncrement)
        {
            // const float *vert = p->GetPoint(id);
            BEGIN_LABEL
                CREATE_LABEL(labelString, this->MaxLabelSize, "%lld", id + this->NodeOrigin);
            END_LABEL
        }
    }
    p->Delete();
