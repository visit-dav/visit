// ************************************************************************* //
//                            avtDataSetWriter.C                             //
// ************************************************************************* //

#include <avtDataSetWriter.h>

#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>

#include <NoInputException.h>


using std::string;


// ****************************************************************************
//  Method: avtDataSetWriter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataSetWriter::avtDataSetWriter()
{
    ;
}


// ****************************************************************************
//  Method: avtDataSetWriter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDataSetWriter::~avtDataSetWriter()
{
    ;
}


// ****************************************************************************
//  Method: avtDataSetWriter::DataObjectWrite
//
//  Purpose:
//      Does the part of the write that is specific to a dataset.
//
//  Arguments:
//      str         The data object string to write to.
//
//  Programmer: Hank Childs
//  Creation:   October 1, 2001
//
// ****************************************************************************

void
avtDataSetWriter::DataObjectWrite(avtDataObjectString &str)
{
    WriteDataTree(GetInputDataTree(), str);
}


// ****************************************************************************
//  Method: avtDataSetWriter::WriteDataTree
//
//  Arguments:
//      tree        The domain tree to write out.
//      str         The data object string to write to.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 1, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Wed Apr 11 08:01:46 PDT 2001
//    Rename method to reflect avtDomainTree now called avtDataTree.
//    Change method calls retrieving the data string, to reflect new
//    names.
//
//    Hank Childs, Sat May 26 09:02:06 PDT 2001
//    Changed arguments to be an avtDataObjectString to get around
//    string::append bottleneck.
//
//    Hank Childs, Tue Jun  5 09:53:11 PDT 2001
//    Wrote out chunk numbers.
//
//    Hank Childs, Tue Jun 19 16:35:01 PDT 2001
//    Handle bad data trees better.
//
//    Hank Childs, Sun Sep 16 16:17:19 PDT 2001
//    Reflect new interface for data object strings.
//
//    Hank Childs, Wed Sep 19 09:51:09 PDT 2001
//    Make use of array writes.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001 
//    Write out label associated with dataset. 
//
//    Hank Childs, Thu Sep 27 17:18:01 PDT 2001
//    Made use of DatasetType.
//
//    Hank Childs, Mon Oct  1 09:16:42 PDT 2001
//    Removed objwriter argument.
//
//    Hank Childs, Wed Mar 17 20:40:56 PST 2004
//    Reduce the number of socket writes.
//
//    Mark C. Miller, Wed Nov 16 14:17:01 PST 2005
//    Added compression
// ****************************************************************************

void
avtDataSetWriter::WriteDataTree(avtDataTree_p tree, avtDataObjectString &str)
{
    if (*tree == NULL||(tree->GetNChildren() <= 0 && tree->HasData() == false))
    {
        WriteInt(str, -1);
        return;
    }

    // get & write the number of children 
    int nc = tree->GetNChildren();
    WriteInt(str, nc);

    if (nc > 0)
    {
        for (int i = 0; i < nc; ++i)
        {
            if ( tree->ChildIsPresent(i) )
            {
                // write 1 to indicate child tree is present
                WriteInt(str, 1);
                // now write out this child tree
                WriteDataTree(tree->GetChild(i), str);
            }
            else 
            {
                // write 0 to indicate child tree is absent
                WriteInt(str, 0);
            }
        }
    }
    else
    {
        int len;
        int lengthAndChunkAndDSTAndLabel[4];
        unsigned char * s;
        DataSetType dst;

        // get the domain string and its length
        if (useCompression)
            s = tree->GetDataRepresentation().GetCompressedDataString(len, dst);
        else
            s = tree->GetDataRepresentation().GetDataString(len, dst);

        lengthAndChunkAndDSTAndLabel[0] = len;
        // write out the length 

        // write out the chunk index
        int chunk = tree->GetDataRepresentation().GetDomain();
        lengthAndChunkAndDSTAndLabel[1] = chunk;

        // write out the dataset's type.
        lengthAndChunkAndDSTAndLabel[2] = dst;

        // write out the label
        string label = tree->GetDataRepresentation().GetLabel();
        lengthAndChunkAndDSTAndLabel[3] = label.size();

        WriteInt(str, lengthAndChunkAndDSTAndLabel, 4);

        if (label.size() > 0)
        {
            str.Append((char *) label.c_str(), label.size(), 
                  avtDataObjectString::DATA_OBJECT_STRING_SHOULD_MAKE_COPY);
        }

        // write out the string 
        if (len > 0)
        {
            str.Append((char*) s, len, 
               avtDataObjectString::DATA_OBJECT_STRING_DOES_NOT_OWN_REFERENCE);
        }
    }
}


