// ************************************************************************* //
//                           avtPosCMFEExpression.C                          //
// ************************************************************************* //

#include <avtPosCMFEExpression.h>

#include <float.h>
#include <math.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <vtkAppendFilter.h>
#include <vtkCellData.h>
#include <vtkCellLocator.h>
#include <vtkCharArray.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkGenericCell.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridRelevantPointsFilter.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkUnstructuredGridReader.h>

#include <Utility.h>
#include <vtkVisItUtility.h>

#include <avtCommonDataFunctions.h>
#include <avtDatasetExaminer.h>
#include <avtIntervalTree.h>
#include <avtParallel.h>
#include <avtPosCMFEAlgorithm.h>
#include <avtSILRestrictionTraverser.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <InvalidMergeException.h>
#include <TimingsManager.h>


// ****************************************************************************
//  Method: avtPosCMFEExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

avtPosCMFEExpression::avtPosCMFEExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtPosCMFEExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
//
// ****************************************************************************

avtPosCMFEExpression::~avtPosCMFEExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtPosCMFEExpression::PerformCMFE
//
//  Purpose:
//      Performs a cross-mesh field evaluation based on position-based 
//      differencing.
//
//  Programmer: Hank Childs
//  Creation:   October 11, 2005
// 
//  Modifications:
//
//    Hank Childs, Sat Jan 21 12:56:57 PST 2006
//    Re-wrote to use avtPosCMFEAlgorithm.
//
// ****************************************************************************

avtDataTree_p
avtPosCMFEExpression::PerformCMFE(avtDataTree_p output_mesh,
                                  avtDataTree_p mesh_to_be_sampled,
                                  const std::string &invar,
                                  const std::string &outvar)
{
    return avtPosCMFEAlgorithm::PerformCMFE(output_mesh, mesh_to_be_sampled,
                                            invar, varnames[1], outvar);
}


