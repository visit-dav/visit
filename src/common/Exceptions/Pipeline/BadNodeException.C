// ************************************************************************* //
//                             BadNodeException.C                            //
// ************************************************************************* //

#include <BadNodeException.h>
#include <snprintf.h>

using std::vector;
using std::string;

// ****************************************************************************
//  Method: BadNodeException constructor
//
//  Arguments:
//      index        The cell number that was out of range.
//      numCells     The total number of cells.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 13, 2004 
//
// ****************************************************************************

BadNodeException::BadNodeException(int index, int numNodes)
{
    char str[1024];
    SNPRINTF(str, 1024, "Node %d is invalid (%d maximum).", index, numNodes-1);
    msg = str;
}


// ****************************************************************************
//  Method: BadNodeException constructor
//
//  Arguments:
//      index        The node index that was out of range. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 13, 2004 
//
// ****************************************************************************

BadNodeException::BadNodeException(vector<int> &index)
{
    char str[1024];
    if (index.size() == 3)
    {
        SNPRINTF(str, 1024, "Node index (%d %d %d) is invalid.",  
                 index[0], index[1], index[2]);
    }
    else 
    {
        SNPRINTF(str, 1024, "Node index (%d %d) is invalid.", 
                 index[0], index[1]);
    }
    msg = str;
}


// ****************************************************************************
//  Method: BadNodeException constructor
//
//  Arguments:
//    index     The node number that was invalid.
//    numCells  The reason the node was invalid. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 19, 2005 
//
// ****************************************************************************

BadNodeException::BadNodeException(int index, string &reason)
{
    char str[1024];
    SNPRINTF(str, 1024, "Node %d is invalid. %s", index, reason.c_str());
    msg = str;
}

