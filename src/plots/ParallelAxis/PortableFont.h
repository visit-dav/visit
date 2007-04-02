/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
