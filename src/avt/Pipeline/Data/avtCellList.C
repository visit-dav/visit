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
//                                avtCellList.C                              //
// ************************************************************************* //

#include <avtCellList.h>

#include <avtHexahedronExtractor.h>
#include <avtPointExtractor.h>
#include <avtImagePartition.h>
#include <avtPyramidExtractor.h>
#include <avtTetrahedronExtractor.h>
#include <avtVolume.h>
#include <avtWedgeExtractor.h>

#include <ImproperUseException.h>
#include <InvalidCellTypeException.h>
#include <Utility.h>


// ****************************************************************************
//  Method: avtCellList constructor
//
//  Arguments:
//      nv      The number of variables.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Jan 27 14:51:58 PST 2006
//    Initialized useRestriction.
//
// ****************************************************************************

avtCellList::avtCellList(int nv)
{
    celllist  = new avtSerializedCell*[1024];
    celllistI = 0;
    celllistN = 1024;
    nVars     = nv;
    useRestriction = false;
    minWidth  = -1;
    maxWidth  = -1;
    minHeight = -1;
    maxHeight = -1;
}


// ****************************************************************************
//  Method: avtCellList destructor
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2001
//
// ****************************************************************************

avtCellList::~avtCellList()
{
    if (celllist != NULL)
    {
        for (int i = 0 ; i < celllistI ; i++)
        {
            if (celllist[i] != NULL)
            {
                if (celllist[i]->cell != NULL)
                {
                    delete [] celllist[i]->cell;
                }
                delete celllist[i];
            }
        }
        delete [] celllist;
    }
}


// ****************************************************************************
//  Method: avtCellList::Restrict
//
//  Purpose:
//      Restricts the cell list to extracting to a subvolume of interest.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2006
//
// ****************************************************************************

void
avtCellList::Restrict(int sw, int hw, int sh, int hh)
{
    useRestriction = true;
    minWidth  = sw;
    maxWidth  = hw;
    minHeight = sh;
    maxHeight = hh;
}


// ****************************************************************************
//  Method: avtCellList::Store (Hex)
//
//  Purpose:
//      Stores a hexahedron.
//
//  Arguments:
//      hex     The hexahedron
//      minx    The minimum x on the screen covered by the cell's bounding box. 
//      maxx    The maximum x on the screen covered by the cell's bounding box. 
//      miny    The minimum y on the screen covered by the cell's bounding box. 
//      maxy    The maximum y on the screen covered by the cell's bounding box. 
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2001
//
// ****************************************************************************

void
avtCellList::Store(const avtHexahedron &hex, int minx, int maxx, int miny,
                   int maxy)
{
    char *cell = Serialize(hex.pts, hex.val, 8);
    Store(cell, minx, maxx, miny, maxy, 8);
}


// ****************************************************************************
//  Method: avtCellList::Store (Pyramid)
//
//  Purpose:
//      Stores a pyramid.
//
//  Arguments:
//      pyr     The pyramid
//      minx    The minimum x on the screen covered by the cell's bounding box. 
//      maxx    The maximum x on the screen covered by the cell's bounding box. 
//      miny    The minimum y on the screen covered by the cell's bounding box. 
//      maxy    The maximum y on the screen covered by the cell's bounding box. 
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2001
//
// ****************************************************************************

void
avtCellList::Store(const avtPyramid &pyr, int minx, int maxx, int miny,
                   int maxy)
{
    char *cell = Serialize(pyr.pts, pyr.val, 5);
    Store(cell, minx, maxx, miny, maxy, 5);
}


// ****************************************************************************
//  Method: avtCellList::Store (Tetrahedron)
//
//  Purpose:
//      Stores a tetrahedron.
//
//  Arguments:
//      tet     The tetrahedron
//      minx    The minimum x on the screen covered by the cell's bounding box. 
//      maxx    The maximum x on the screen covered by the cell's bounding box. 
//      miny    The minimum y on the screen covered by the cell's bounding box. 
//      maxy    The maximum y on the screen covered by the cell's bounding box. 
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2001
//
// ****************************************************************************

void
avtCellList::Store(const avtTetrahedron &tet, int minx, int maxx, int miny,
                   int maxy)
{
    char *cell = Serialize(tet.pts, tet.val, 4);
    Store(cell, minx, maxx, miny, maxy, 4);
}


// ****************************************************************************
//  Method: avtCellList::Store (Wedge)
//
//  Purpose:
//      Stores a wedge.
//
//  Arguments:
//      wedge   The wedge
//      minx    The minimum x on the screen covered by the cell's bounding box. 
//      maxx    The maximum x on the screen covered by the cell's bounding box. 
//      miny    The minimum y on the screen covered by the cell's bounding box. 
//      maxy    The maximum y on the screen covered by the cell's bounding box. 
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2001
//
// ****************************************************************************

void
avtCellList::Store(const avtWedge &wedge, int minx, int maxx, int miny,
                   int maxy)
{
    char *cell = Serialize(wedge.pts, wedge.val, 6);
    Store(cell, minx, maxx, miny, maxy, 6);
}


// ****************************************************************************
//  Method: avtCellList::Store (Points)
//
//  Purpose:
//      Stores a point
//
//  Arguments:
//      pt      The point
//      minx    The minimum x on the screen covered by the cell's bounding box. 
//      maxx    The maximum x on the screen covered by the cell's bounding box. 
//      miny    The minimum y on the screen covered by the cell's bounding box. 
//      maxy    The maximum y on the screen covered by the cell's bounding box. 
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2006
//
// ****************************************************************************

void
avtCellList::Store(const avtPoint &pt, int minx, int maxx, int miny,
                   int maxy)
{
    char *cell = SerializePoint(pt.bbox, pt.val);
    Store(cell, minx, maxx, miny, maxy, 1);
}


// ****************************************************************************
//  Method: avtCellList::Store
//
//  Purpose:
//      Stores a cell in the cell list once it has been serialized.
//
//  Arguments:
//      cell    The serialized cell.
//      minx    The minimum x on the screen covered by the cell's bounding box. 
//      maxx    The maximum x on the screen covered by the cell's bounding box. 
//      miny    The minimum y on the screen covered by the cell's bounding box. 
//      maxy    The maximum y on the screen covered by the cell's bounding box. 
//      size    The size of the cell.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2001
//
// ****************************************************************************

void
avtCellList::Store(char *cell, int minx, int maxx, int miny, int maxy,int size)
{
    avtSerializedCell *c = new avtSerializedCell;
    c->cell = cell;
    c->minx = minx;
    c->maxx = maxx;
    c->miny = miny;
    c->maxy = maxy;
    c->size = size;

    if (celllistI >= celllistN)
    {
        celllistN *= 2;
        avtSerializedCell **newlist = new avtSerializedCell*[celllistN];
        for (int i = 0 ; i < celllistI ; i++)
        {
            newlist[i] = celllist[i];
        }

        delete [] celllist;
        celllist = newlist;
    }

    celllist[celllistI] = c;
    celllistI++;
}


// ****************************************************************************
//  Method: avtCellList::Serialize
//
//  Purpose:
//      Serializes a cell when given arrays and their lengths.
//
//  Arguments:
//      pts      The point list.
//      var      The variable list.
//      nvals    The number of values in each array.
//
//  Returns:     The serialized version of the arrays.
//
//  Programmer: Hank Childs
//  Creation:   January 27, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 15:17:27 PST 2001
//    Added support for multiple variables.
//
//    Hank Childs, Mon Dec 31 15:15:11 PST 2001
//    Fix bugs from last checkin supporting multiple variables.
//
// ****************************************************************************

char *
avtCellList::Serialize(const float (*pts)[3], 
                       const float (*var)[AVT_VARIABLE_LIMIT], int nvals)
{
    int numPoints    = 3;
    int bytesPerNode = (numPoints+nVars)*sizeof(float);
    char *serialized = new char[bytesPerNode*nvals];

    char *tmp = serialized;
    InlineCopy(tmp, (char *)pts, sizeof(float)*3*nvals);
    for (int i = 0 ; i < nvals ; i++)
    {
        InlineCopy(tmp, (char *)var[i], sizeof(float)*nVars);
    }

    return serialized;
}


// ****************************************************************************
//  Method: avtCellList::SerializePoint
//
//  Purpose:
//      Serializes a point when given arrays and their lengths.
//
//  Arguments:
//      bbox     The bounding box.
//      nvals    The number of values in each array.
//
//  Returns:     The serialized version of the arrays.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2006
//
// ****************************************************************************

char *
avtCellList::SerializePoint(const float *bbox, const float *var)
{
    int bboxSize     = 6;
    int bytesPerPt   = (bboxSize+nVars)*sizeof(float);
    char *serialized = new char[bytesPerPt];

    char *tmp = serialized;
    InlineCopy(tmp, (char *)bbox, sizeof(float)*bboxSize);
    InlineCopy(tmp, (char *)var, sizeof(float)*nVars);

    return serialized;
}


// ****************************************************************************
//  Method: avtCellList::ConstructMessages
//
//  Purpose:
//      Constructs the messages to other processors using an image partition.
//
//  Arguments:
//      part    The image partition.
//      msgs    The messages to the other partitions.
//      lens    The lengths of the messages.
//
//  Returns:    The character string that all of the messages are on.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jan  1 10:47:03 PST 2002
//    Account for multiple variables in a cell.
//
//    Hank Childs, Wed Jan 25 07:15:38 PST 2006
//    Add support for points.
//
// ****************************************************************************

char *
avtCellList::ConstructMessages(avtImagePartition *part, char **msgs, int *lens)
{
    int  i, j;
    int  numPartitions = part->GetNumPartitions();

    int storageForCoord = 3;
    const int bytesPerNode = (storageForCoord+nVars)*sizeof(float); 
    int storageForBBox = 6;
    const int bytesForCellThatIsAPt = (storageForBBox+nVars)*sizeof(float);

    //
    // Set up memory to put our messages into.
    //
    for (i = 0 ; i < numPartitions ; i++)
    {
        lens[i] = 0;
    }
    int *partitions = new int[numPartitions];
    for (i = 0 ; i < celllistI ; i++)
    {
        //
        // PartitionList will make a list of the partitions that this cell
        // needs to be sent to.  It will place that list in `partitions'.
        //
        int numParts = part->PartitionList(celllist[i]->minx,celllist[i]->maxx,
                                           celllist[i]->miny,celllist[i]->maxy,
                                           partitions);
        int size = sizeof(int);
        if (celllist[i]->size > 1)
            size += bytesPerNode*celllist[i]->size;
        else
            size += bytesForCellThatIsAPt;
        for (j = 0 ; j < numParts ; j++)
        {
            lens[partitions[j]] += size;
        }
    }
    char *rv = CreateMessageStrings(msgs, lens, numPartitions);
    char **msgstemp = new char*[numPartitions];
    for (i = 0 ; i < numPartitions ; i++)
    {
        msgstemp[i] = msgs[i];
    }

    //
    // Go through our cell list and add each cell to the appropriate message.
    //
    for (i = 0 ; i < celllistI ; i++)
    {
        int numParts = part->PartitionList(celllist[i]->minx,celllist[i]->maxx,
                                           celllist[i]->miny,celllist[i]->maxy,
                                           partitions);
        for (j = 0 ; j < numParts ; j++)
        {
            int p = partitions[j];
            InlineCopy(msgstemp[p], (char *)&(celllist[i]->size), sizeof(int));
            int size;
            if (celllist[i]->size > 1)
                size = bytesPerNode*celllist[i]->size;
            else
                size = bytesForCellThatIsAPt;

            InlineCopy(msgstemp[p], celllist[i]->cell, size);
        }
    }

    delete [] msgstemp;
    delete [] partitions;

    return rv;
}


// ****************************************************************************
//  Method: avtCellList::ExtractCells
//
//  Purpose:
//      Takes in a list of messages for this partition, converts them into
//      cells and extracts them onto its volume.
//
//  Arguments:
//      msgs    The messages
//      lens    The lengths of each message.
//      np      The number of partitions.
//      vol     The volume the sample points should go into.
//
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jan  1 09:53:54 PST 2002
//    Account for multiple variables on a cell.
//
//    Hank Childs, Wed Jan 25 07:15:38 PST 2006
//    Add support for points.
//
//    Hank Childs, Fri Jan 27 14:40:14 PST 2006
//    Make sure that we only extract the parts of the cells that are actually
//    within our partition of the volume.
//
//    Hank Childs, Thu Feb 21 16:22:05 PST 2008
//    Initialize npts in case extraction fails.
//
// ****************************************************************************

void
avtCellList::ExtractCells(const char * const *msgs, const int *lens, int np,
                          avtVolume *vol)
{
    avtHexahedron            hex;
    avtPoint                 pt;
    avtPyramid               pyr;
    avtTetrahedron           tet;
    avtWedge                 wedge;

    hex.nVars   = nVars;
    pyr.nVars   = nVars;
    pt.nVars    = nVars;
    tet.nVars   = nVars;
    wedge.nVars = nVars;

    int   width  = vol->GetVolumeWidth();
    int   height = vol->GetVolumeHeight();
    int   depth  = vol->GetVolumeDepth();

    avtHexahedronExtractor hexExtractor(width, height, depth, vol, this);
    avtPointExtractor pointExtractor(width, height, depth, vol, this);
    avtPyramidExtractor pyrExtractor(width, height, depth, vol, this);
    avtTetrahedronExtractor tetExtractor(width, height, depth, vol, this);
    avtWedgeExtractor wedgeExtractor(width, height, depth, vol, this);

    if (useRestriction)
    {
        hexExtractor.Restrict(minWidth, maxWidth, minHeight, maxHeight);
        pointExtractor.Restrict(minWidth, maxWidth, minHeight, maxHeight);
        pyrExtractor.Restrict(minWidth, maxWidth, minHeight, maxHeight);
        tetExtractor.Restrict(minWidth, maxWidth, minHeight, maxHeight);
        wedgeExtractor.Restrict(minWidth, maxWidth, minHeight, maxHeight);
    }

    for (int i = 0 ; i < np ; i++)
    {
        const char *tmpmsg = msgs[i];
        while (tmpmsg < msgs[i] + lens[i])
        {
            int npts = 0;
            InlineExtract((char *)&npts, tmpmsg, sizeof(int));
           
            switch (npts)
            {
                case 1:
                   UnserializePoint(pt.bbox, pt.val, tmpmsg);
                   pointExtractor.Extract(pt);
                   break;

                case 4:
                   Unserialize(tet.pts, tet.val, 4, tmpmsg);
                   tetExtractor.Extract(tet);
                   break;

                case 5:
                   Unserialize(pyr.pts, pyr.val, 5, tmpmsg);
                   pyrExtractor.Extract(pyr);
                   break;

                case 6:
                   Unserialize(wedge.pts, wedge.val, 6, tmpmsg);
                   wedgeExtractor.Extract(wedge);
                   break;

                case 8:
                   Unserialize(hex.pts, hex.val, 8, tmpmsg);
                   hexExtractor.Extract(hex);
                   break;

                default:
                   EXCEPTION1(InvalidCellTypeException, npts);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtCellList::Unserialize
//
//  Purpose:
//      Unserializes the character string onto the arrays.
//
//  Arguments:
//      pts     The points for the cell.
//      var     The variable for the cell.
//      nvals   The number of values.
//      str     The string to extract from.
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Nov 14 15:17:27 PST 2001
//    Added support for multiple variables.
//
//    Hank Childs, Mon Dec 31 15:15:11 PST 2001
//    Fix bugs from last checkin supporting multiple variables.
//
// ****************************************************************************

void
avtCellList::Unserialize(float (*pts)[3], float (*var)[AVT_VARIABLE_LIMIT],
                         int nvals, const char *&str)
{
    InlineExtract((char *)pts, str, nvals*3*sizeof(float));
    for (int i = 0 ; i < nvals ; i++)
    {
        InlineExtract((char *)var[i], str, nVars*sizeof(float));
    }
}


// ****************************************************************************
//  Method: avtCellList::UnserializePoint
//
//  Purpose:
//      Unserializes the character string onto the arrays.
//
//  Arguments:
//      bbox    The bbox for the cell.
//      var     The variable for the cell.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2006
//
// ****************************************************************************

void
avtCellList::UnserializePoint(float *bbox, float *var, const char *&str)
{
    InlineExtract((char *)bbox, str, 6*sizeof(float));
    InlineExtract((char *)var, str, nVars*sizeof(float));
}


// ****************************************************************************
//  Method: avtCellList::EstimateNumberOfSamplesPerScanline
//
//  Purpose:
//      Estimates the number of samples in each scanline.
//
//  Arguments:
//      samples    A running list of how many samples points there are per
//                 scanline.
//
//  Programmer:    Hank Childs
//  Creation:      January 1, 2002
//
// ****************************************************************************

void
avtCellList::EstimateNumberOfSamplesPerScanline(int *samples)
{
    for (int i = 0 ; i < celllistI ; i++)
    {
        const avtSerializedCell *cell = celllist[i];

        //
        // We know what the footprint of the cell is in x and y, but have no
        // idea what it is in z.  This is supposed to be an *estimate* right?
        // Let's just say that z is the minimum of the span of x and the span
        // of y.  I chose minimum over maximum because there probably aren't
        // as many samples in the cell as there are in its footprint, so we
        // are starting with an overestimate.
        //
        // We should be okay as long as we don't encounter too many cells with
        // high aspect ratios.  Besides, this is just an optimization, so
        // getting it wrong shouldn't hurt too bad -- just means we perform
        // suboptimally.
        //
        int sampsInX = cell->maxx - cell->minx + 1;
        int sampsInY = cell->maxy - cell->miny + 1;
        int sampsInZ = (sampsInX < sampsInY ? sampsInX : sampsInY);
        int sampsPerScanline = sampsInX * sampsInZ;
        for (int j = cell->miny ; j < cell->maxy ; j++)
        {
            samples[j] += sampsPerScanline;
        }
    }
}


