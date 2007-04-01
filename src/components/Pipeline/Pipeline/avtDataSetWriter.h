// ************************************************************************* //
//                            avtDataSetWriter.h                             //
// ************************************************************************* //

#ifndef AVT_DATASET_WRITER_H
#define AVT_DATASET_WRITER_H

#include <pipeline_exports.h>

#include <avtOriginatingDatasetSink.h>
#include <avtDataObjectWriter.h>


class     avtDataObjectString;
class     avtDataTree;


// ****************************************************************************
//  Class: avtDataSetWriter
//
//  Purpose:
//      A class which takes as input an avtDataSet and can serialize it.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 22, 2000
//
//  Modifications:
//
//    Brad Whitlock, Mon Oct 23 10:58:35 PDT 2000
//    I renamed the type reps and added a private method to write an integer.
//
//    Hank Childs, Thu Dec 28 16:27:33 PST 2000
//    Stripped out a lot of functionality to go into avtDataObjectWriter.
//
//    Kathleen Bonnell, Thu Mar  1 10:01:44 PST 2001
//    Added method WriteDomainTree.
//
//    Kathleen Bonnell, Wed Apr 11 08:01:46 PDT 2001 
//    Reflect that avtDomainTree now called avtDataTree. 
// 
//    Hank Childs, Mon Oct  1 08:47:41 PDT 2001
//    Re-worked inheritance hierarchy.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
// ****************************************************************************

class PIPELINE_API avtDataSetWriter : virtual public avtOriginatingDatasetSink, 
                         virtual public avtDataObjectWriter
{
  public:
                    avtDataSetWriter();
    virtual        ~avtDataSetWriter();

    virtual bool    MustMergeParallelStreams(void) { return true; };

  protected:
    virtual void    DataObjectWrite(avtDataObjectString &);
    void            WriteDataTree(avtDataTree_p, avtDataObjectString &);
};


#endif


