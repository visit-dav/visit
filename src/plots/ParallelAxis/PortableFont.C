#include <PortableFont.h>
#include <PortableFontTables.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//  Class: PortableFont
//
//  Purpose: This class draws text strings as simple stroked characters.  When
//           given (1) a C-style 7-bit ASCII string, (2) a character width and
//           height, (3) a starting position, (4) a retrace count (like a line
//           width for stroking the characters), (5) an orientation, and (6) a
//           a justification flag, the main method outputs a vector of simple
//           2-D line segments that can then be rendered by any graphics library.
//           PortableFont is completely self-contained and uses only standard
//           C library functions and the C++ standard template vector class.
//
//  Notes:
//
//  Programmer: Mark Blair
//  Creation:   Thu May 05 19:11:00 PDT 2005
//
//  Modifications:
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



// *****************************************************************************
//  Method: PortableFont::PortableFont
//
//  Purpose: Constructor for the PortableFont class.
//
//  Notes:
//
//  Programmer: Mark Blair
//  Creation:   Thu May 05 19:11:00 PDT 2005
//
//  Modifications:
//
// *****************************************************************************

PortableFont::PortableFont()
{
    InitializeStrokeTables();
}


// *****************************************************************************
//  Method: PortableFont::~PortableFont
//
//  Purpose: Destructor for the PortableFont class.
//
//  Notes:
//
//  Programmer: Mark Blair
//  Creation:   Thu May 05 19:11:00 PDT 2005
//
//  Modifications:
//
// *****************************************************************************

PortableFont::~PortableFont()
{
    return;
}


// ****************************************************************************
//  Function: PortableFont::StrokeText
//
//  Purpose: Generates the strokes (i.e., line segments) for a text string.
//
//           Input to this method (not in argument order) is:
//               (1) a C-style 7-bit ASCII string
//               (2) a character width and height
//               (3) a 2-D starting position
//               (4) a stroke retrace count (proportional to number of times
//                   each character will be retraced with slight offsets in
//                   position to achieve the effect of stroke lines with width)
//               (5) one of the 4 orthogonal orientations
//               (6) a flag indicating whether the string is to be horizontally
//                   centered about its starting position
//               (7) a vector of floating-point (not double precision)
//                   quadruples to which the text string's strokes will be
//                   appended as additional quadrulpes (X1, Y1, X2, Y2)
//
//  Programmer: Mark Blair
//  Creation:   Thu May 05 19:11:00 PDT 2005
//
//  Modifications:
//
// ****************************************************************************

void
PortableFont::StrokeText (std::vector<std::vector<float> > *strokeList,
    PF_ORIENTATION orientation, bool centerIt, double xStart, double yStart,
    double capWidth, double capHeight, int retraceCount, char *textString)
{
    char textChar;
    int retraceIndex, offsetNum, charNum, sdIndex, startIndex, stopIndex;
    unsigned int uintChar;
    double rotScal[4];
    double wTraceOffset, hTraceOffset, xPosDelta, yPosDelta;
    double w1, h1, w2, h2, segmentX1, segmentY1, segmentX2, segmentY2;
    PF_STROKE_DEFINITION *strokeDefPtr;
    std::vector<float> *segmentPtr;

    if (retraceCount < 1)
        retraceIndex = 0;
    else if (retraceCount > 6)
        retraceIndex = 5;
    else
        retraceIndex = retraceCount - 1;

    ComputeRotationAndScalingAndSpacing (rotScal, &xPosDelta, &yPosDelta,
        capWidth, capHeight, orientation, retraceIndex);

    double xCurrentPos = xStart;
    double yCurrentPos = yStart;

    if (centerIt)
    {
        CenterStartPosition(&xCurrentPos, &yCurrentPos,
        orientation, capWidth, retraceIndex, strlen(textString));
    }

    std::vector<double> *traceOffsets = ComputeTraceOffsets(retraceIndex);

    int offsetCount = traceOffsets->size();

    for (char *charPtr = textString; *charPtr; charPtr++)
    {
        textChar = *charPtr;

        for (offsetNum = 0; offsetNum < offsetCount; offsetNum += 2)
        {
            wTraceOffset = (*traceOffsets)[offsetNum];
            hTraceOffset = (*traceOffsets)[offsetNum + 1];

            if ((uintChar = (unsigned int)textChar) > (int)' ')
            {
                if ((uintChar >= (int)'a') && (uintChar <= (int)'z'))
                {
                    textChar = (char)(uintChar - 0x20);
                }

                for (charNum=0; charNum < sizeof(PF_StrokedChars); charNum++)
                {
                    if (PF_StrokedChars[charNum] == textChar) break;
                }

                if (charNum < sizeof(PF_StrokedChars))
                {
                    startIndex = PF_StartStrokeIndexs[charNum];
                    stopIndex  = PF_StartStrokeIndexs[charNum+1];

                    for (sdIndex = startIndex; sdIndex < stopIndex; sdIndex++)
                    {
                        segmentPtr = new std::vector<float>;

                        strokeDefPtr = &PF_StrokeDefinitions[sdIndex];

                        w1 = (double)strokeDefPtr->X1 + wTraceOffset;
                        h1 = (double)strokeDefPtr->Y1 + hTraceOffset;
                        w2 = (double)strokeDefPtr->X2 + wTraceOffset;
                        h2 = (double)strokeDefPtr->Y2 + hTraceOffset;

                        segmentX1 = rotScal[0]*w1 + rotScal[2]*h1 + xCurrentPos;
                        segmentY1 = rotScal[1]*w1 + rotScal[3]*h1 + yCurrentPos;
                        segmentX2 = rotScal[0]*w2 + rotScal[2]*h2 + xCurrentPos;
                        segmentY2 = rotScal[1]*w2 + rotScal[3]*h2 + yCurrentPos;

                        segmentPtr->push_back((float)segmentX1);
                        segmentPtr->push_back((float)segmentY1);
                        segmentPtr->push_back((float)segmentX2);
                        segmentPtr->push_back((float)segmentY2);

                        strokeList->push_back(*segmentPtr);
                    }
                }
            }
        }

        xCurrentPos += xPosDelta; yCurrentPos += yPosDelta;
    }

    delete traceOffsets;
}


// *****************************************************************************
//  Function: PortableFont::DoubleNumericalString
//
//  Purpose: Converts a double-precision floating-point number to its ASCII
//           representation as a C-style string.
//
//  Programmer: Mark Blair
//  Creation:   Thu May 05 19:11:00 PDT 2005
//
//  Modifications:
//
// *****************************************************************************

void
PortableFont::DoubleNumericalString(char doubleLabel[], double doubleValue)
{
    int labelLen, labelCharID;

    if (doubleValue < -9e+36)
    {
        strcpy(doubleLabel, "(min unknown)");
        return;
    }

    if (doubleValue > +9e+36)
    {
        strcpy(doubleLabel, "(max unknown)");
        return;
    }

    sprintf (doubleLabel, "%g", doubleValue);

    if ((labelLen = strlen(doubleLabel)) < 3) return;

    for (labelCharID = labelLen - 1; labelCharID > 1; labelCharID--)
    {
        if (doubleLabel[labelCharID]   != '0') break;
        if (doubleLabel[labelCharID-1] == '.') break;
        if (doubleLabel[labelCharID-1] == '+') break;
        if (doubleLabel[labelCharID-1] == '-') break;
    }

    doubleLabel[labelCharID + 1] = '\0';
}


// *****************************************************************************
//  Function: PortableFont::InitializeStrokeTables
//
//  Purpose: Initializes tables needed to stroke the text.
//
//  Programmer: Mark Blair
//  Creation:   Thu May 05 19:11:00 PDT 2005
//
//  Modifications:
//
// *****************************************************************************

void
PortableFont::InitializeStrokeTables()
{
    PF_StartStrokeIndexs[0] = 0;

    for (int charNum = 0; charNum < sizeof(PF_StrokedChars); charNum++)
    {
        PF_StartStrokeIndexs[charNum+1] =
        PF_StartStrokeIndexs[charNum] + PF_StrokeCounts[charNum];
    }
}


// *****************************************************************************
//  Function: PortableFont::ComputeTransformAndSpacing
//
//  Purpose: Computes the rotational and scaling components of a transformation
//           that will be applied to normalized character stroke coordinates
//           from a table.  Also computes the X and Y changes in position from
//           one character to the next within s tring.
//
//  Programmer: Mark Blair
//  Creation:   Tue Jul 12 14:32:00 PDT 2005
//
//  Modifications:
//
// *****************************************************************************

void
PortableFont::ComputeRotationAndScalingAndSpacing (double rotateScale[4],
    double *xPosDelta, double *yPosDelta, double capWidth, double capHeight,
    PF_ORIENTATION orientation, int retraceIndex)
{
    double widthScale  = capWidth / 70.0;
    double heightScale = capHeight / 100.0;

    double posDeltaMagnitude = capWidth *
        ((PF_StrokeHeightFactors[retraceIndex]*10.0 + 9.0) / 7.0);

    if (orientation == Rightward)
    {
        rotateScale[0] = widthScale; rotateScale[1] = 0.0;
        rotateScale[2] = 0.0;        rotateScale[3] = heightScale;

        *xPosDelta = posDeltaMagnitude; *yPosDelta = 0.0;
    }
    else if (orientation == Leftward)
    {
        rotateScale[0] = -widthScale; rotateScale[1] = 0.0;
        rotateScale[2] = 0.0;         rotateScale[3] = -heightScale;

        *xPosDelta = -posDeltaMagnitude; *yPosDelta = 0.0;
    }
    else if (orientation == Upward)
    {
        rotateScale[0] = 0.0;          rotateScale[1] = widthScale;
        rotateScale[2] = -heightScale; rotateScale[3] = 0.0;

        *xPosDelta = 0.0; *yPosDelta = posDeltaMagnitude;
    }
    else if (orientation == Downward)
    {
        rotateScale[0] = 0.0;         rotateScale[1] = -widthScale;
        rotateScale[2] = heightScale; rotateScale[3] = 0.0;

        *xPosDelta = 0.0; *yPosDelta = -posDeltaMagnitude;
    }
    else
    {   // Same as Rightward if orientation argument is not valid.
        rotateScale[0] = widthScale; rotateScale[1] = 0.0;
        rotateScale[2] = 0.0;        rotateScale[3] = heightScale;

        *xPosDelta = posDeltaMagnitude; *yPosDelta = 0.0;
    }
}


// *****************************************************************************
//  Function: PortableFont::CenterStartPosition
//
//  Purpose: Computes a new position for the lower left corner of the first
//           character of a string of given length if the string is to be
//           centered about the given start position.
//
//  Programmer: Mark Blair
//  Creation:   Thu May 05 19:11:00 PDT 2005
//
//  Modifications:
//
// *****************************************************************************

void
PortableFont::CenterStartPosition(double *xStartPos, double *yStartPos,
    PF_ORIENTATION orient, double capWidth, int retraceIndex, int stringLength)
{
    double spaceWidth =
        ((PF_StrokeHeightFactors[retraceIndex]*10.0 + 2.0) / 7.0) * capWidth;
    double offsetMagnitude =
        ((double)stringLength*(capWidth+spaceWidth) - spaceWidth) * 0.5;

    if (orient == Rightward)
        *xStartPos -= offsetMagnitude;
    else if (orient == Leftward)
        *xStartPos += offsetMagnitude;
    else if (orient == Upward)
        *yStartPos -= offsetMagnitude;
    else if (orient == Downward)
        *yStartPos += offsetMagnitude;
    else
        *xStartPos -= offsetMagnitude; // Rightward
}


// *****************************************************************************
//  Function: PortableFont::ComputeTraceOffsets
//
//  Purpose: Each character in a string is stroked a number of times with small
//           offsets in position to achieve the effect of stroke lines with
//           width.  This method computes those offsets.
//
//  Programmer: Mark Blair
//  Creation:   Thu May 05 19:11:00 PDT 2005
//
//  Modifications:
//
// *****************************************************************************

std::vector<double> *
PortableFont::ComputeTraceOffsets (int retraceIndex)
{
    double fullOffset = PF_StrokeHeightFactors[retraceIndex] * 50.0;

    std::vector<double> *positionOffsets = new std::vector<double>;

    int startIndex = PF_StartPosOffIndexs[retraceIndex];
    int stopIndex  = PF_StartPosOffIndexs[retraceIndex + 1];

    for (int poIndex = startIndex; poIndex < stopIndex; poIndex++)
    {
        positionOffsets->push_back(PF_XPosOffFactors[poIndex] * fullOffset);
        positionOffsets->push_back(PF_YPosOffFactors[poIndex] * fullOffset);
    }

    return positionOffsets;
}
