// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;


// ****************************************************************************
// Class: avtSymmetricTensorMetaData
//
// Purpose:
//    Contains symmetricTensor metadata attributes
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class avtSymmetricTensorMetaData extends avtVarMetaData
{
    private static int avtSymmetricTensorMetaData_numAdditionalAtts = 1;

    public avtSymmetricTensorMetaData()
    {
        super(avtSymmetricTensorMetaData_numAdditionalAtts);

        dim = 0;
    }

    public avtSymmetricTensorMetaData(int nMoreFields)
    {
        super(avtSymmetricTensorMetaData_numAdditionalAtts + nMoreFields);

        dim = 0;
    }

    public avtSymmetricTensorMetaData(avtSymmetricTensorMetaData obj)
    {
        super(obj);

        dim = obj.dim;

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return avtSymmetricTensorMetaData_numAdditionalAtts;
    }

    public boolean equals(avtSymmetricTensorMetaData obj)
    {
        // Create the return value
        return (super.equals(obj) && (dim == obj.dim));
    }

    // Property setting methods
    public void SetDim(int dim_)
    {
        dim = dim_;
        Select((new avtSymmetricTensorMetaData()).Offset() + 0);
    }

    // Property getting methods
    public int GetDim() { return dim; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        super.WriteAtts(buf);

        int offset = (new avtSymmetricTensorMetaData()).Offset();
        if(WriteSelect(offset + 0, buf))
            buf.WriteInt(dim);
    }

    public void ReadAtts(int id, CommunicationBuffer buf)
    {
        int offset = (new avtSymmetricTensorMetaData()).Offset();
        if(id == offset)
            SetDim(buf.ReadInt());
        else
            super.ReadAtts(id, buf);
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + intToString("dim", dim, indent) + "\n";
        return super.toString(indent) + str;
    }


    // Attributes
    private int dim;
}

