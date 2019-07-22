// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtImagePartition.h                            //
// ************************************************************************* //

#ifndef AVT_IMAGE_PARTITION_H
#define AVT_IMAGE_PARTITION_H

#include <pipeline_exports.h>

#include <vector>


// ****************************************************************************
//  Class: avtImagePartition
//
//  Purpose:
//      Helps other modules partition image space into scanlines.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Mar  4 21:07:43 PST 2001
//    Allowed for this partition to be assigned dynamically.
//
//    Hank Childs, Tue Jan  1 11:18:50 PST 2002
//    Make partitions size be adaptively determined based on how many samples
//    fall along each scanline.
//
//    Hank Childs, Fri Dec 10 10:45:26 PST 2004
//    Add support for tiling.
//
//    Hank Childs, Fri Sep 30 17:27:02 PDT 2005
//    Add support for "produce overlaps".
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Tom Fogal, Wed Jun 17 19:04:46 MDT 2009
//    Switch to vectors for easier debugging.
//
// ****************************************************************************

class PIPELINE_API avtImagePartition
{
  public:
                            avtImagePartition(int, int, int = -1, int = -1);
    virtual                ~avtImagePartition();

    inline int              Partition(const int &, const int &, bool &);
    inline std::vector<int> GetPartitionsAsADuplicate(const int &, const int &);
    inline int              PartitionList(const int &, const int &,
                                          const int &, const int &, int *);

    void                    GetThisPartition(int &, int &, int &, int &);
    void                    GetPartition(int, int &, int &, int &, int &);

    int                     GetNumPartitions(void)  { return numProcessors; };
    void                    SetThisPartition(int);

    void                    EstablishPartitionBoundaries(int *);
    void                    DetermineAssignments(int *);

    const int              *GetPartitionToProcessorAssignments(void) 
                                 { return &ptpAssignments[0]; };
    const int              *GetScanlineToPartitionAssignemnts(void)
                                 { return &stpAssignments[0]; };

    int                     GetWidth(void)  { return width; };
    int                     GetHeight(void) { return height; };
    int                     GetTileWidth(void)  
                                  { return (!shouldDoTiling ?  width:
                                            tile_width_max-tile_width_min);};
    int                     GetTileHeight(void)  
                                  { return (!shouldDoTiling ?  height:
                                            tile_height_max-tile_height_min);};

    void                    RestrictToTile(int, int, int, int);
    void                    StopTiling(void) { shouldDoTiling = false; };
    void                    SetShouldProduceOverlaps(bool b) 
                                             { shouldProduceOverlaps = b; };

  protected:
    int                     width, height;
    int                     numProcessors;
    int                     thisProcessor;
    int                     thisPartition;

    bool                    shouldDoTiling;
    bool                    shouldProduceOverlaps;
    int                     tile_width_min, tile_width_max;
    int                     tile_height_min, tile_height_max;

    bool                    establishedPartitionBoundaries;

    std::vector<int>        ptpAssignments;
    std::vector<int>        stpAssignments;
    std::vector<int>        partitionStartsOnScanline;
    std::vector<int>        partitionStopsOnScanline;

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                        avtImagePartition(const avtImagePartition &) {;};
    avtImagePartition  &operator=(const avtImagePartition &) { return *this; };
};


#include <BadIndexException.h>
#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtImagePartition::Partition
//
//  Purpose:
//      Takes a width and a height and determines which partition it lies in.
//
//  Arguments:
//      w       The width of the pixel
//      h       The height of the pixel
//
//  Returns:    The partition (processor) the pixel belongs to.
//
//  Programmer: Hank Childs
//  Creation:   January 25, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Sep 18 10:20:35 PDT 2001
//    Fix bad cast.
//
//    Hank Childs, Tue Jan  1 12:52:59 PST 2002
//    Make use of new data member stpAssignments that allow for partitions to
//    cover different numbers of scanlines.
//
//    Hank Childs, Sun Oct  2 10:35:24 PDT 2005
//    Add support for overlaps.
//
// ****************************************************************************

inline int
avtImagePartition::Partition(const int &, const int &h, 
                             bool &thisPointOverlappedOnAnotherPartition)
{
    if (!establishedPartitionBoundaries)
    {
        EXCEPTION0(ImproperUseException);
    }
    if (h < 0 || h >= height)
    {
        EXCEPTION2(BadIndexException, h, height);
    }
    thisPointOverlappedOnAnotherPartition = false;
    if (shouldProduceOverlaps)
    {
        if (h < (height-1) && (stpAssignments[h] != stpAssignments[h+1]))
            thisPointOverlappedOnAnotherPartition = true;
    }
    return stpAssignments[h];
}


// ****************************************************************************
//  Method: avtImagePartition::GetPartitionsAsADuplicated
//
//  Purpose:
//      If the image partition produces overlaps, then this method will return
//      the partitions that the point is duplicated on.  So, if a point is
//      owned primarily by partition 3, but also is on partition 2 as a 
//      duplicated point, this will return a vector containing 2.  Also, for
//      the current scheme of the image partition, there can only ever by one
//      duplicate.  But this the right interface if we ever change the scheme
//      to include shafts, cubes, etc.
//
//  Arguments:
//      w       The width of the pixel
//      h       The height of the pixel
//
//  Returns:    A list of the partition (processor) the pixel belongs to as a
//              duplicate.
//
//  Programmer: Hank Childs
//  Creation:   October 2, 2005
//
// ****************************************************************************

std::vector<int>
avtImagePartition::GetPartitionsAsADuplicate(const int &, const int &h)
{
    if (!establishedPartitionBoundaries)
    {
        EXCEPTION0(ImproperUseException);
    }
    if (h < 0 || h >= height)
    {
        EXCEPTION2(BadIndexException, h, height);
    }
    std::vector<int> rv;
    if (shouldProduceOverlaps)
    {
        if (h < (height-1) && (stpAssignments[h] != stpAssignments[h+1]))
            rv.push_back(stpAssignments[h+1]);
    }
    return rv;
}


// ****************************************************************************
//  Method: avtImagePartition::PartitionList
//
//  Purpose:
//      Determines the partitions a range of pixels falls into.
//
//  Arguments:
//      minW     The minimum pixel in X.
//      maxW     The minimum pixel in X.
//      minH     The maximum pixel in Y.
//      maxH     The maximum pixel in Y.
//      list     A place to put the list of partitions.
//
//  Returns:     The number of partitions placed in 'list'.
//
//  Notes:       This routine depends heavily on the partitioning scheme
//               being scanline based and not brick based.
//
//  Programmer:  Hank Childs
//  Creation:    January 25, 2001
//
// ****************************************************************************

inline int
avtImagePartition::PartitionList(const int &minW, const int &maxW,
                                 const int &minH, const int &maxH, int *list)
{
    bool unused;
    int min = Partition(minW, minH, unused);
    int max = Partition(maxW, maxH, unused);

    for (int i = min ; i <= max ; i++)
    {
         list[i-min] = i;
    }

    return (max-min+1);
}

#endif
