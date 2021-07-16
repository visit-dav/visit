# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  annotation_objects.py
#
#  Tests:      AnnotationObject unit test
#
#  Alister Maguire, Fri Jul 16 12:50:11 PDT 2021
#
#  Modifications:
#
# ----------------------------------------------------------------------------


def CheckComparisonOperator():
    annot = CreateAnnotationObject("Text2D")

    #
    # In the past, a bug caused a crash whenever we compared to a non-annotation
    # type object.
    #
    TestValueEQ("ComparisonOperator_00", annot == "", False)
    TestValueEQ("ComparisonOperator_01", annot == annot, True)

def RunMain():
    CheckComparisonOperator()

RunMain()
Exit()
