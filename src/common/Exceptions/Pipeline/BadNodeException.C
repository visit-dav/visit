// ************************************************************************* //
//                             BadNodeException.C                            //
// ************************************************************************* //

#include <stdio.h>               // for sprintf
#include <BadNodeException.h>

using std::vector;

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
    sprintf(str, "Node %d is invalid (%d maximum).", index, numNodes-1);
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
        sprintf(str, "Node index (%d %d %d) is invalid.",  
                index[0], index[1], index[2]);
  
    }
    else 
    {
        sprintf(str, "Node index (%d %d) is invalid.", index[0], index[1]);
  
    }
    msg = str;
}


