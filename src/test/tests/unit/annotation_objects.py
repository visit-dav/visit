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
    result = ""
    result += str(annot == "") + "\n"
    result += str(annot == annot)
    
    TestText("ComparisonOperator_00", result)

def RunMain():
    CheckComparisonOperator()

RunMain()
Exit()
