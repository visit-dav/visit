// ************************************************************************* //
//                             BadCellException.C                            //
// ************************************************************************* //

#include <stdio.h>               // for sprintf
#include <BadCellException.h>

using std::vector;

// ****************************************************************************
//  Method: BadCellException constructor
//
//  Arguments:
//      index        The cell number that was out of range.
//      numCells     The total number of cells.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2002
//
// ****************************************************************************

BadCellException::BadCellException(int index, int numCells)
{
    char str[1024];
    sprintf(str, "Cell %d is invalid (%d maximum).", index, numCells-1);
    msg = str;
}


// ****************************************************************************
//  Method: BadCellException constructor
//
//  Arguments:
//      index        The cell index that was out of range. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 16, 2002 
//
// ****************************************************************************

BadCellException::BadCellException(vector<int> &index)
{
    char str[1024];
    if (index.size() == 3)
    {
        sprintf(str, "Cell index (%d %d %d) is invalid.",  
                index[0], index[1], index[2]);
  
    }
    else 
    {
        sprintf(str, "Cell index (%d %d) is invalid.", index[0], index[1]);
  
    }
    msg = str;
}


