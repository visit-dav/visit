// ****************************************************************************
// Function: PrintDataType
//
// Purpose: 
//   Prints the name of the data type to debug4.
//
// Arguments:
//   dt : The data type.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 31 09:33:43 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

static void
PrintDataType(DataType_t dt)
{
    switch(dt)
    {
    case DataTypeNull:
        debug4 << "DataTypeNull";
        break;
    case DataTypeUserDefined:
        debug4 << "DataTypeUserDefined";
        break;
    case Integer:
        debug4 << "Integer";
        break;
    case RealSingle:
        debug4 << "RealSingle";
        break;
    case RealDouble:
        debug4 << "RealDouble";
        break;
    case Character:
        debug4 << "Character";
        break;
    }
}

// ****************************************************************************
// Function: PrintElementType
//
// Purpose: 
//   Prints the name of the element type to debug4.
//
// Arguments:
//   et : The element type.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 31 09:33:43 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

static void
PrintElementType(ElementType_t et)
{
    if(et == ElementTypeNull) debug4 << "ElementTypeNull";
    else if(et == ElementTypeUserDefined) debug4 << "ElementTypeUserDefined";
    else if(et == NODE) debug4 << "NODE";
    else if(et == BAR_2) debug4 << "BAR_2";
    else if(et == BAR_3) debug4 << "BAR_3";
    else if(et == TRI_3) debug4 << "TRI_3";
    else if(et == TRI_6) debug4 << "TRI_6";
    else if(et == QUAD_4) debug4 << "QUAD_4";
    else if(et == QUAD_8) debug4 << "QUAD_8";
    else if(et == QUAD_9) debug4 << "QUAD_9";
    else if(et == TETRA_4) debug4 << "TETRA_4";
    else if(et == TETRA_10) debug4 << "TETRA_10";
    else if(et == PYRA_5) debug4 << "PYRA_5";
    else if(et == PYRA_14) debug4 << "PYRA_14";
    else if(et == PENTA_6) debug4 << "PENTA_6";
    else if(et == PENTA_15) debug4 << "PENTA_15";
    else if(et == PENTA_18) debug4 << "PENTA_18";
    else if(et == HEXA_8) debug4 << "HEXA_8";
    else if(et == HEXA_20) debug4 << "HEXA_20";
    else if(et == HEXA_27) debug4 << "HEXA_27";
    else if(et == MIXED) debug4 << "MIXED";
    else if(et == NGON_n) debug4 << "NGON_n";
}
