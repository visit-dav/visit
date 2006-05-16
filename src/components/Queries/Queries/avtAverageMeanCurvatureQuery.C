// ************************************************************************* //
//                       avtAverageMeanCurvatureQuery.C                      //
// ************************************************************************* //

#include <avtAverageMeanCurvatureQuery.h>

#include <avtCurvatureExpression.h>

#include <DebugStream.h>
#include <NonQueryableInputException.h>


using     std::string;


// ****************************************************************************
//  Method: avtAverageMeanCurvatureQuery constructor
//
//  Programmer: Hank Childs 
//  Creation:   May 11, 2006
//
// ****************************************************************************

avtAverageMeanCurvatureQuery::avtAverageMeanCurvatureQuery() 
    : avtWeightedVariableSummationQuery()
{
    curvature = new avtCurvatureExpression;
    curvature->DoGaussCurvature(false);
    curvature->SetOutputVariableName("curvature");
}


// ****************************************************************************
//  Method: avtAverageMeanCurvatureQuery destructor
//
//  Programmer: Hank Childs 
//  Creation:   May 11, 2006
//
// ****************************************************************************

avtAverageMeanCurvatureQuery::~avtAverageMeanCurvatureQuery()
{
    delete curvature;
}


// ****************************************************************************
//  Method: avtAverageMeanCurvatureQuery::CreateVariable
//
//  Purpose:
//     Creates the variable for the summation.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2006
//
// ****************************************************************************

avtDataObject_p
avtAverageMeanCurvatureQuery::CreateVariable(avtDataObject_p inData)
{
    curvature->SetInput(inData);
    return curvature->GetOutput();
}


// ****************************************************************************
//  Method: avtAverageMeanCurvatureQuery::VerifyInput
//
//  Purpose:
//      Make sure we are operating on a surface.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2006
//
// ****************************************************************************

void
avtAverageMeanCurvatureQuery::VerifyInput(void)
{
    //
    // We want to do this in addition to what the base class does, so call the
    // base class' version of this method as well.
    //
    avtWeightedVariableSummationQuery::VerifyInput();

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 2)
    {
        EXCEPTION1(NonQueryableInputException, "The average mean curvature "
                   "query can only operate on surfaces.");
    }
}

