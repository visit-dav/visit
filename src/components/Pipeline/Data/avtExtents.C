/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                                 avtExtents.C                              //
// ************************************************************************* //

#include <avtExtents.h>

#include <float.h>
#include <visitstream.h>

#include <avtDataObjectString.h>
#include <avtDataObjectWriter.h>

#include <InvalidMergeException.h>
#include <ImproperUseException.h>

#include <vtkMatrix4x4.h>


// ****************************************************************************
//  Method: avtExtents constructor
//
//  Arguments:
//      d        The dimension of the variable.
//
//  Programmer:  Hank Childs
//  Creation:    September 4, 2001
//
// ****************************************************************************

avtExtents::avtExtents(int d)
{
    dimension = d;
    extents   = NULL;
}


// ****************************************************************************
//  Method: avtExtents copy constructor
//
//  Arguments:
//      ext      The extents to copy.
//
//  Programmer:  Hank Childs
//  Creation:    September 4, 2001
//
// ****************************************************************************

avtExtents::avtExtents(const avtExtents &exts)
{
    extents = NULL;
    *this = exts;
}


// ****************************************************************************
//  Method: avtExtents destructor
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
// ****************************************************************************

avtExtents::~avtExtents()
{
    if (extents != NULL)
    {
        delete [] extents;
        extents = NULL;
    }
}


// ****************************************************************************
//  Method: avtExtents::Print
//
//  Purpose:
//      Prints this object.
//
//  Programmer: Hank Childs
//  Creation:   February 25, 2004
//
// ****************************************************************************

void
avtExtents::Print(ostream &out)
{
    out << "Extents (dim = " << dimension << ") = ";
    if (extents == NULL)
        out << "Not set";
    else
    {
        for (int i = 0 ; i < dimension ; i++)
        {
             out << "{" << extents[2*i] << ", " << extents[2*i+1] << "}";
             if (i < dimension-1)
                 out << ", ";
        }
    }
    out << endl;
}


// ****************************************************************************
//  Method: avtExtents assignment operator
//
//  Arguments:
//      exts    The extents to copy.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Dec 18 11:38:40 PST 2007
//    Remove const return type of assignment operator to ensure that compiler
//    doesn't define a second assignment operator with a non-const return
//    type that does a bitwise copy.
//
// ****************************************************************************

avtExtents &
avtExtents::operator=(const avtExtents &exts)
{
    if (extents != NULL)
    {
        delete [] extents;
    }

    dimension = exts.dimension;
    if (exts.extents != NULL)
    {
        extents = new double[2*dimension];
        for (int i = 0 ; i < 2*dimension ; i++)
        {
            extents[i] = exts.extents[i];
        }
    }
    else
    {
        extents = NULL;
    }

    return *this;
}


// ****************************************************************************
//  Method: avtExtents::HasExtents
//
//  Purpose:
//      Returns whether or not there are actual extents in this object.
//
//  Returns:    true if it has extents, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
// ****************************************************************************

bool
avtExtents::HasExtents(void)
{
    return (extents != NULL ? true : false);
}


// ****************************************************************************
//  Method: avtExtents::CopyTo
//
//  Purpose:
//      Copies the contents of the extents to a buffer.
//
//  Arguments:
//      exts    The buffer to copy into.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Oct  1 14:10:37 PDT 2001
//    Fix bug where wrong pointer was compared against.
//
// ****************************************************************************

void
avtExtents::CopyTo(double *exts)
{
    if (exts == NULL)
    {
        //
        // How can we copy into a NULL array?
        //
        EXCEPTION0(ImproperUseException);
    }

    if (extents == NULL)
    {
        //
        // We don't have extents, so copy in the biggest bounds possible.
        //
        for (int i = 0 ; i < dimension ; i++)
        {
            exts[2*i]   = +DBL_MAX;
            exts[2*i+1] = -DBL_MAX;
        }
    }
    else
    {
        //
        // The most common case -- copy our extents over.
        //
        for (int i = 0 ; i < 2*dimension ; i++)
        {
            exts[i] = extents[i];
        }
    }
}


// ****************************************************************************
//  Method: avtExtents::Set
//
//  Purpose:
//      Uses an outside buffer to set the contents of this object.
//
//  Arguments:
//      exts    The buffer to copy from.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
// ****************************************************************************

void
avtExtents::Set(const double *exts)
{
    if (exts == NULL)
    {
        EXCEPTION0(ImproperUseException);
    }

    if (extents == NULL)
    {
        extents = new double[2*dimension];
    }
    // else just copy over the same buffer

    for (int i = 0 ; i < 2*dimension ; i++)
    {
        extents[i] = exts[i];
    }
}


// ****************************************************************************
//  Method: avtExtents::Merge
//
//  Purpose:
//      Merges two sets of extents and puts the results in this object.
//
//  Arguments:
//      exts      The extents to merge in.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
// ****************************************************************************

void
avtExtents::Merge(const avtExtents &exts)
{
    if (dimension != exts.dimension)
    {
        EXCEPTION2(InvalidMergeException, dimension, exts.dimension);
    }

    if (exts.extents == NULL)
    {
        //
        // If our extents are NULL or not, nothing we can do.
        //
        return;
    }

    Merge(exts.extents);
}


// ****************************************************************************
//  Method: avtExtents::Merge
//
//  Purpose:
//      Merges two sets of extents and puts the results in this object.
//
//  Arguments:
//      exts      The extents to merge in.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
// ****************************************************************************

void
avtExtents::Merge(const double *exts)
{
    if (extents == NULL)
    {
        extents = new double[2*dimension];
        for (int i = 0 ; i < 2*dimension ; i++)
        {
            extents[i] = exts[i];
        }
    }
    else
    {
        for (int i = 0 ; i < dimension ; i++)
        {
            if (exts[2*i] < extents[2*i])
            {
                extents[2*i] = exts[2*i];
            }
            if (exts[2*i+1] > extents[2*i+1])
            {
                extents[2*i+1] = exts[2*i+1];
            }
        }
    }
}


// ****************************************************************************
//  Method: avtExtents::Write
//
//  Purpose:
//      Writes the extents to a data object string with the help of a data
//      object writer (which does type conversion for remote platforms, etc).
//
//  Arguments:
//      str     The memory friendly string to write into.
//      wrtr    The writer that does type conversion for remote platforms.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2001
//
//  Modifications:
//    Hank Childs, Wed Sep 19 10:24:02 PDT 2001
//    Make use of new array writing facilities.
//
//    Jeremy Meredith, Thu Feb  7 14:44:42 EST 2008
//    Support arbitrarily large dimension.
//
// ****************************************************************************

void
avtExtents::Write(avtDataObjectString &str, const avtDataObjectWriter *wrtr)
{
    if (extents == NULL)
    {
        double *dummyExtents = new double[2*dimension];
        for (int i=0; i<dimension; i++)
        {
            dummyExtents[2*i+0] =  DBL_MAX;
            dummyExtents[2*i+1] = -DBL_MAX;
        }
        //
        // We have no extents; write out '0' to indicate this.
        //
        wrtr->WriteInt(str, 0);

        // still write some data for extents so all processor's message
        // size will agree
        wrtr->WriteDouble(str, dummyExtents, 2*dimension);

        delete[] dummyExtents;
    }
    else
    {
        //
        // We do have extents; write out '1' to indicate this.
        //
        wrtr->WriteInt(str, 1);

        //
        // Write out the extents.
        //
        wrtr->WriteDouble(str, extents, 2*dimension);
    }
}


// ****************************************************************************
//  Method: avtExtents::Read
//
//  Purpose:
//      Reads in the extents from a string.
//
//  Arguments:
//      buffer   A buffer that contains the encoded extents.
//
//  Returns:     The number of bytes read from the buffer.
//
//  Programmer:  Hank Childs
//  Creation:    September 4, 2001
//
// ****************************************************************************

int
avtExtents::Read(char *buffer)
{
    int  size = 0;

    if (extents != NULL)
    {
        delete [] extents;
        extents = NULL;
    }

    //
    // Determine if there are extents to read.
    //
    int  hasExtents;
    memcpy(&hasExtents, buffer, sizeof(int));
    buffer += sizeof(int); size += sizeof(int);

    double *tmpExtents = new double[2*dimension];
    for (int i = 0 ; i < 2*dimension ; i++)
    {
        memcpy(&(tmpExtents[i]), buffer, sizeof(double));
        buffer += sizeof(double); size += sizeof(double);
    }

    if (hasExtents != 0)
        extents = tmpExtents; 
    else
        delete [] tmpExtents;

    return size;
}


// ****************************************************************************
//  Method:  avtExtents::Transform
//
//  Purpose:
//    Transform the bounding box by a matrix to get a new bounding box.
//
//  Arguments:
//    t          the matrix to transform by
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 24, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jun 19 21:07:30 PDT 2002
//    Make sure we don't write past array bounds.
//
// ****************************************************************************

void
avtExtents::Transform(vtkMatrix4x4 *t)
{
    if (extents == NULL)
        return;

    // set up a blank extents array
    double *newExtents = new double[2*dimension];
    int i;
    for (i = 0; i < dimension; i++)
    {
        newExtents[2*i]   =  DBL_MAX;
        newExtents[2*i+1] = -DBL_MAX;
    }

    double p1[4];
    double p2[4];

    // Transform each point of the bounding box and update the extents
    for (i=0; i<=(dimension>0 ? 1 : 0); i++)
    {
        for (int j=0; j<=(dimension>1 ? 1 : 0); j++)
        {
            for (int k=0; k<=(dimension>2 ? 1 : 0); k++)
            {
                // create the point of the bounding box
                p1[0] = (dimension>0) ? extents[2*0 + i] : 0.;
                p1[1] = (dimension>1) ? extents[2*1 + j] : 0.;
                p1[2] = (dimension>2) ? extents[2*2 + k] : 0.;
                p1[3] = 1.;
                // transform and homogenize it
                t->MultiplyPoint(p1, p2);
                p2[0] /= p2[3];
                p2[1] /= p2[3];
                p2[2] /= p2[3];
                p2[3] =  1.;

                // update the extents from the new point
                if (dimension > 0)
                {
                    if (newExtents[0] > p2[0])
                        newExtents[0] = p2[0];
                    if (newExtents[1] < p2[0])
                        newExtents[1] = p2[0];
                }
                if (dimension > 1)
                {
                    if (newExtents[2] > p2[1])
                        newExtents[2] = p2[1];
                    if (newExtents[3] < p2[1])
                        newExtents[3] = p2[1];
                }
                if (dimension > 2)
                {
                    if (newExtents[4] > p2[2])
                        newExtents[4] = p2[2];
                    if (newExtents[5] < p2[2])
                        newExtents[5] = p2[2];
                }
            }
        }
    }
    delete[] extents;
    extents = newExtents;
}


// ****************************************************************************
//  Method: avtExtents::Clear
//
//  Purpose:
//      Clears out the extents.
//
//  Programmer: Hank Childs
//  Creation:   November 6, 2001
//
// ****************************************************************************

void
avtExtents::Clear(void)
{
    if (extents != NULL)
    {
        delete [] extents;
        extents = NULL;
    }
}


