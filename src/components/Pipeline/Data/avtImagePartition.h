// ************************************************************************* //
//                            avtImagePartition.h                            //
// ************************************************************************* //

#ifndef AVT_IMAGE_PARTITION_H
#define AVT_IMAGE_PARTITION_H
#include <pipeline_exports.h>


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
// ****************************************************************************

class PIPELINE_API avtImagePartition
{
  public:
                            avtImagePartition(int, int, int = -1, int = -1);
    virtual                ~avtImagePartition();

    inline int              Partition(const int &, const int &);
    inline int              PartitionList(const int &, const int &,
                                          const int &, const int &, int *);

    void                    GetThisPartition(int &, int &, int &, int &);
    void                    GetPartition(int, int &, int &, int &, int &);

    int                     GetNumPartitions(void)  { return numProcessors; };
    void                    SetThisPartition(int);

    void                    EstablishPartitionBoundaries(int *);
    void                    DetermineAssignments(int *);

    const int              *GetPartitionToProcessorAssignments(void) 
                                 { return ptpAssignments; };
    const int              *GetScanlineToPartitionAssignemnts(void)
                                 { return stpAssignments; };

    int                     GetWidth(void)  { return width; };
    int                     GetHeight(void) { return height; };

  protected:
    int                     width, height;
    int                     numProcessors;
    int                     thisProcessor;
    int                     thisPartition;

    bool                    establishedPartitionBoundaries;

    int                    *ptpAssignments;
    int                    *stpAssignments;
    int                    *partitionStartsOnScanline;
    int                    *partitionStopsOnScanline;
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
// ****************************************************************************

inline int
avtImagePartition::Partition(const int &, const int &h)
{
    if (!establishedPartitionBoundaries)
    {
        EXCEPTION0(ImproperUseException);
    }
    if (h < 0 || h >= height)
    {
        EXCEPTION2(BadIndexException, h, height);
    }
    return stpAssignments[h];
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
    int min = Partition(minW, minH);
    int max = Partition(maxW, maxH);

    for (int i = min ; i <= max ; i++)
    {
         list[i-min] = i;
    }

    return (max-min+1);
}


#endif


