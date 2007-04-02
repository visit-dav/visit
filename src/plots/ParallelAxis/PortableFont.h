#ifndef PORTABLE_FONT_H
#define PORTABLE_FONT_H


#include <vector>


// ****************************************************************************
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
// ****************************************************************************

class PortableFont
{
public:
    enum PF_ORIENTATION
    {
        Rightward,
        Leftward,
        Upward,
        Downward
    };

    PortableFont();
   ~PortableFont();

    void StrokeText (std::vector<std::vector<float> > *strokeList,
        PF_ORIENTATION orientation, bool centerIt, double xStart, double yStart,
        double capWidth, double capHeight, int retraceCount, char *textString);
    void DoubleNumericalString (char doubleLabel[], double doubleValue);


private:
    void InitializeStrokeTables();
    void ComputeRotationAndScalingAndSpacing (double rotateScale[4],
        double *xPosDelta, double *yPosDelta, double capWidth, double capHeight,
PF_ORIENTATION orientation, int retraceIndex);
    void CenterStartPosition(double *xStartPos, double *yStartPos,
        PF_ORIENTATION orient,double capWidth,int retraceIndex,int stringLength);
    std::vector<double> *ComputeTraceOffsets (int retraceIndex);
};

#endif
