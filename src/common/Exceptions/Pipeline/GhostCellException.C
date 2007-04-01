// ************************************************************************* //
//                           GhostCellException.C                            //
// ************************************************************************* //

#include <stdio.h>               // for sprintf
#include <GhostCellException.h>

using std::vector;

// ****************************************************************************
//  Method: GhostCellException constructor
//
//  Arguments:
//    index     The index of the ghost cell. 
//    m         Additional message information.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 14, 2002 
//
// ****************************************************************************

GhostCellException::GhostCellException(int index, char *m)
{
    char str[1024];
    if (m)
    {
        sprintf(str, "Cell %d is a ghost cell.  %s", index, m);
    }
    else 
    {
        sprintf(str, "Cell %d is a ghost cell.", index);
    }
    msg = str;
}


// ****************************************************************************
//  Method: GhostCellException constructor
//
//  Arguments:
//    index     The index of the ghost cell. 
//    m         Additional message information.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 14, 2002 
//
// ****************************************************************************

GhostCellException::GhostCellException(const vector<int> &index, const char *m)
{
    char str[1024];
    if (m)
    {
        if (index.size() == 3)
        {
           sprintf(str, "Cell index (%d %d %d) is a ghost cell.  %s", 
                   index[0], index[1], index[2], m);
        }
        else
        {
           sprintf(str, "Cell index (%d %d) is a ghost cell.  %s", 
                   index[0], index[1], m);
        }
    }
    else 
    {
        if (index.size() == 3)
        {
            sprintf(str, "Cell index (%d %d %d) is a ghost cell.", 
                    index[0], index[1] , index[2]);
        }
        else 
        {
            sprintf(str, "Cell index (%d %d) is a ghost cell.", 
                    index[0], index[1]);
        }
    }
    msg = str;
}


