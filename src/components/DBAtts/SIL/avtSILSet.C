// ************************************************************************* //
//                                 avtSILSet.C                               //
// ************************************************************************* //

#include <avtSILSet.h>


using std::string;
using std::vector;


// ****************************************************************************
//  Method: avtSILSet constructor
//
//  Arguments:
//      n       The name of the set.
//      i       An identifier for the set.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Jun 15 10:42:51 PDT 2001
//    Added identifier argument.
//
// ****************************************************************************

avtSILSet::avtSILSet(string n, int i)
{
    name = n;
    id   = i;
}


// ****************************************************************************
//  Method: avtSILSet::AddMapIn
//
//  Arguments:
//      in      The index in the SIL object of the collection that maps
//              into this set.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
// ****************************************************************************

void
avtSILSet::AddMapIn(int in)
{
    mapsIn.push_back(in);
}


// ****************************************************************************
//  Method: avtSILSet::AddMapOut
//
//  Arguments:
//      out     The index in the SIL object of the collection that maps
//              out of this set.
//
//  Programmer: Hank Childs
//  Creation:   March 9, 2001
//
// ****************************************************************************

void
avtSILSet::AddMapOut(int out)
{
    mapsOut.push_back(out);
    allMapsOut.push_back(out);
}


// ****************************************************************************
//  Method: avtSILSet::AddMatrixRow
//
//  Purpose:
//      Tells the set that it is a member in a matrix as a row.
//
//  Arguments:
//      mat     The index of the matrix.
//      row     The index of this set in the matrix.
//      coll    The index of the collection this corresponds to in the SIL.
//
//  Programmer: Hank Childs
//  Creation:   November 15, 2002
//
// ****************************************************************************

void
avtSILSet::AddMatrixRow(int mat, int row, int coll)
{
    allMapsOut.push_back(coll);
    idOfMatrixRow.push_back(mat);
    idInMatrixRow.push_back(row);
}


// ****************************************************************************
//  Method: avtSILSet::AddMatrixColumn
//
//  Purpose:
//      Tells the set that it is a member in a matrix as a column.
//
//  Arguments:
//      mat     The index of the matrix.
//      column  The index of this set in the matrix.
//      coll    The index of the collection this corresponds to in the SIL.
//
//  Programmer: Hank Childs
//  Creation:   November 15, 2002
//
// ****************************************************************************

void
avtSILSet::AddMatrixColumn(int mat, int column, int coll)
{
    allMapsOut.push_back(coll);
    idOfMatrixColumn.push_back(mat);
    idInMatrixColumn.push_back(column);
}


// ****************************************************************************
//  Method: avtSILSet::Print
//
//  Purpose:
//      Prints out a SIL set -- meant for debugging purposes only.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Dec  4 09:16:53 PST 2002
//    Don't print out unnecessary lines.
//
// ****************************************************************************

void
avtSILSet::Print(ostream &out) const
{
    out << "\tSet name = " << name.c_str() << ", identifier = " << id << endl;
    std::vector<int>::const_iterator it;
    if (mapsIn.size() > 0)
    {
        out << "\t\tMaps in ";
        for (it = mapsIn.begin() ; it != mapsIn.end() ; it++)
        {
            out << *it << ", ";
        }
        out << endl;
    }
    if (mapsOut.size() > 0)
    {
        out << "\t\tMaps out ";
        for (it = mapsOut.begin() ; it != mapsOut.end() ; it++)
        {
            out << *it << ", ";
        }
        out << endl;
    }
}


