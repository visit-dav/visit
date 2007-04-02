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

#include "ClipCases.h"

// Programmer: Jeremy Meredith
// Date      : August 11, 2003
//
// Modifications:
//    Jeremy Meredith, Mon Sep 15 17:30:21 PDT 2003
//    Added ability for Centroid-Points to have an associated color.

// This file is meant to be read and created by a program other than a
// compiler.  If you must modify it by hand, at least be nice to the 
// parser and don't add anything else to this file or rearrange it.

int numClipCasesTet = 16;

int numClipShapesTet[16] = {
  1,  2,  2,  2,  2,  2,  2,  2, // cases 0 - 7
  2,  2,  2,  2,  2,  2,  2,  1  // cases 8 - 15
};

int startClipShapesTet[16] = {
  0, 6, 20, 34, 50, 64, 80, 96, // cases 0 - 7
  110, 124, 140, 156, 170, 186, 200, 214  // cases 8 - 15
};

unsigned char clipShapesTet[] = {
 // Case #0: Unique case #1
  ST_TET, COLOR0, P0, P1, P2, P3, 
 // Case #1: Unique case #2
  ST_WDG, COLOR0, EA, ED, EC, P1, P3, P2, 
  ST_TET, COLOR1, P0, EA, EC, ED, 
 // Case #2: (cloned #1)
  ST_WDG, COLOR0, P0, P3, P2, EA, EE, EB, 
  ST_TET, COLOR1, P1, EB, EA, EE, 
 // Case #3: Unique case #3
  ST_WDG, COLOR0, P3, ED, EE, P2, EC, EB, 
  ST_WDG, COLOR1, P0, ED, EC, P1, EE, EB, 
 // Case #4: (cloned #1)
  ST_WDG, COLOR0, EC, EF, EB, P0, P3, P1, 
  ST_TET, COLOR1, P2, EC, EB, EF, 
 // Case #5: (cloned #3)
  ST_WDG, COLOR0, P1, EA, EB, P3, ED, EF, 
  ST_WDG, COLOR1, P2, EF, EB, P0, ED, EA, 
 // Case #6: (cloned #3)
  ST_WDG, COLOR0, P3, EE, EF, P0, EA, EC, 
  ST_WDG, COLOR1, P1, EE, EA, P2, EF, EC, 
 // Case #7: Unique case #4
  ST_TET, COLOR0, ED, EE, EF, P3, 
  ST_WDG, COLOR1, ED, EE, EF, P0, P1, P2, 
 // Case #8: (cloned #1)
  ST_WDG, COLOR0, P0, P2, P1, ED, EF, EE, 
  ST_TET, COLOR1, P3, EE, ED, EF, 
 // Case #9: (cloned #3)
  ST_WDG, COLOR0, P2, EC, EF, P1, EA, EE, 
  ST_WDG, COLOR1, P0, EC, EA, P3, EF, EE, 
 // Case #10: (cloned #3)
  ST_WDG, COLOR0, P0, EA, ED, P2, EB, EF, 
  ST_WDG, COLOR1, P3, EF, ED, P1, EB, EA, 
 // Case #11: (cloned #7)
  ST_TET, COLOR0, EC, EF, EB, P2, 
  ST_WDG, COLOR1, P0, P1, P3, EC, EB, EF, 
 // Case #12: (cloned #3)
  ST_WDG, COLOR0, P1, EB, EE, P0, EC, ED, 
  ST_WDG, COLOR1, P2, EB, EC, P3, EE, ED, 
 // Case #13: (cloned #7)
  ST_TET, COLOR0, EA, EB, EE, P1, 
  ST_WDG, COLOR1, EA, EB, EE, P0, P2, P3, 
 // Case #14: (cloned #7)
  ST_TET, COLOR0, EA, ED, EC, P0, 
  ST_WDG, COLOR1, P1, P2, P3, EA, EC, ED, 
 // Case #15: Unique case #5
  ST_TET, COLOR1, P0, P1, P2, P3, 
 // Dummy
  0
};

