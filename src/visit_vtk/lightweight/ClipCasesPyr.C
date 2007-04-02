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

int numClipCasesPyr = 32;

int numClipShapesPyr[32] = {
  1,  3,  3,  8,  3,  4,  8,  4, // cases 0 - 7
  3,  8,  4,  4,  8,  4,  4,  2, // cases 8 - 15
  2,  4,  4,  8,  4,  4,  8,  3, // cases 16 - 23
  4,  8,  4,  3,  8,  3,  3,  1  // cases 24 - 31
};

int startClipShapesPyr[32] = {
  0, 7, 27, 47, 106, 126, 158, 217, // cases 0 - 7
  247, 267, 326, 358, 388, 447, 477, 507, // cases 8 - 15
  524, 541, 571, 601, 660, 690, 718, 777, // cases 16 - 23
  797, 827, 886, 914, 934, 993, 1013, 1033  // cases 24 - 31
};

unsigned char clipShapesPyr[] = {
 // Case #0: Unique case #1
  ST_PYR, COLOR0, P0, P1, P2, P3, P4, 
 // Case #1: Unique case #2
  ST_WDG, COLOR0, EA, EE, ED, P1, P4, P3, 
  ST_TET, COLOR0, P1, P2, P3, P4, 
  ST_TET, COLOR1, P0, EA, ED, EE, 
 // Case #2: (cloned #1)
  ST_WDG, COLOR0, EB, EF, EA, P2, P4, P0, 
  ST_TET, COLOR0, P2, P3, P0, P4, 
  ST_TET, COLOR1, P1, EB, EA, EF, 
 // Case #3: Unique case #3
  ST_PNT, 0, COLOR0, 7, P4, EF, EE, EB, ED, P2, P3, 
  ST_TET, COLOR0, EE, P4, EF, N0, 
  ST_PYR, COLOR0, EB, ED, EE, EF, N0, 
  ST_PYR, COLOR0, EB, EF, P4, P2, N0, 
  ST_TET, COLOR0, P2, P4, P3, N0, 
  ST_PYR, COLOR0, P3, P4, EE, ED, N0, 
  ST_PYR, COLOR0, P2, P3, ED, EB, N0, 
  ST_WDG, COLOR1, EB, EF, P1, ED, EE, P0, 
 // Case #4: (cloned #1)
  ST_WDG, COLOR0, EC, EG, EB, P3, P4, P1, 
  ST_TET, COLOR0, P3, P0, P1, P4, 
  ST_TET, COLOR1, P2, EC, EB, EG, 
 // Case #5: Unique case #4
  ST_WDG, COLOR0, EE, P4, EG, EA, P1, EB, 
  ST_WDG, COLOR0, P4, EE, EG, P3, ED, EC, 
  ST_WDG, COLOR1, P0, EA, EE, P2, EB, EG, 
  ST_WDG, COLOR1, ED, P0, EE, EC, P2, EG, 
 // Case #6: (cloned #3)
  ST_PNT, 0, COLOR0, 7, P4, EG, EF, EC, EA, P3, P0, 
  ST_TET, COLOR0, EF, P4, EG, N0, 
  ST_PYR, COLOR0, EC, EA, EF, EG, N0, 
  ST_PYR, COLOR0, EC, EG, P4, P3, N0, 
  ST_TET, COLOR0, P3, P4, P0, N0, 
  ST_PYR, COLOR0, P0, P4, EF, EA, N0, 
  ST_PYR, COLOR0, P3, P0, EA, EC, N0, 
  ST_WDG, COLOR1, EC, EG, P2, EA, EF, P1, 
 // Case #7: Unique case #5
  ST_TET, COLOR0, EE, EF, EG, P4, 
  ST_WDG, COLOR0, EC, ED, P3, EG, EE, P4, 
  ST_WDG, COLOR1, EE, EF, EG, P0, P1, P2, 
  ST_WDG, COLOR1, P2, EC, EG, P0, ED, EE, 
 // Case #8: (cloned #1)
  ST_WDG, COLOR0, ED, EH, EC, P0, P4, P2, 
  ST_TET, COLOR0, P0, P1, P2, P4, 
  ST_TET, COLOR1, P3, ED, EC, EH, 
 // Case #9: (cloned #3)
  ST_PNT, 0, COLOR0, 7, P4, EE, EH, EA, EC, P1, P2, 
  ST_TET, COLOR0, EH, P4, EE, N0, 
  ST_PYR, COLOR0, EA, EC, EH, EE, N0, 
  ST_PYR, COLOR0, EA, EE, P4, P1, N0, 
  ST_TET, COLOR0, P1, P4, P2, N0, 
  ST_PYR, COLOR0, P2, P4, EH, EC, N0, 
  ST_PYR, COLOR0, P1, P2, EC, EA, N0, 
  ST_WDG, COLOR1, EA, EE, P0, EC, EH, P3, 
 // Case #10: (cloned #5)
  ST_WDG, COLOR0, EH, P4, EF, ED, P0, EA, 
  ST_WDG, COLOR0, P4, EH, EF, P2, EC, EB, 
  ST_WDG, COLOR1, P3, ED, EH, P1, EA, EF, 
  ST_WDG, COLOR1, EC, P3, EH, EB, P1, EF, 
 // Case #11: (cloned #7)
  ST_TET, COLOR0, EH, EE, EF, P4, 
  ST_WDG, COLOR0, EB, EC, P2, EF, EH, P4, 
  ST_WDG, COLOR1, EH, EE, EF, P3, P0, P1, 
  ST_WDG, COLOR1, P1, EB, EF, P3, EC, EH, 
 // Case #12: (cloned #3)
  ST_PNT, 0, COLOR0, 7, P4, EH, EG, ED, EB, P0, P1, 
  ST_TET, COLOR0, EG, P4, EH, N0, 
  ST_PYR, COLOR0, ED, EB, EG, EH, N0, 
  ST_PYR, COLOR0, ED, EH, P4, P0, N0, 
  ST_TET, COLOR0, P0, P4, P1, N0, 
  ST_PYR, COLOR0, P1, P4, EG, EB, N0, 
  ST_PYR, COLOR0, P0, P1, EB, ED, N0, 
  ST_WDG, COLOR1, ED, EH, P3, EB, EG, P2, 
 // Case #13: (cloned #7)
  ST_TET, COLOR0, EG, EH, EE, P4, 
  ST_WDG, COLOR0, EA, EB, P1, EE, EG, P4, 
  ST_WDG, COLOR1, EG, EH, EE, P2, P3, P0, 
  ST_WDG, COLOR1, P0, EA, EE, P2, EB, EG, 
 // Case #14: (cloned #7)
  ST_TET, COLOR0, EF, EG, EH, P4, 
  ST_WDG, COLOR0, ED, EA, P0, EH, EF, P4, 
  ST_WDG, COLOR1, EF, EG, EH, P1, P2, P3, 
  ST_WDG, COLOR1, P3, ED, EH, P1, EA, EF, 
 // Case #15: Unique case #6
  ST_PYR, COLOR0, EE, EF, EG, EH, P4, 
  ST_HEX, COLOR1, P0, P1, P2, P3, EE, EF, EG, EH, 
 // Case #16: Unique case #7
  ST_HEX, COLOR0, P0, P1, P2, P3, EE, EF, EG, EH, 
  ST_PYR, COLOR1, EE, EF, EG, EH, P4, 
 // Case #17: Unique case #8
  ST_WDG, COLOR0, ED, EH, P3, EA, EF, P1, 
  ST_WDG, COLOR0, EF, EG, EH, P1, P2, P3, 
  ST_WDG, COLOR1, P4, EF, EH, P0, EA, ED, 
  ST_TET, COLOR1, EF, EG, EH, P4, 
 // Case #18: (cloned #17)
  ST_WDG, COLOR0, EA, EE, P0, EB, EG, P2, 
  ST_WDG, COLOR0, EG, EH, EE, P2, P3, P0, 
  ST_WDG, COLOR1, P4, EG, EE, P1, EB, EA, 
  ST_TET, COLOR1, EG, EH, EE, P4, 
 // Case #19: Unique case #9
  ST_PNT, 0, COLOR1, 7, EH, EG, ED, EB, P0, P1, P4, 
  ST_WDG, COLOR0, ED, EH, P3, EB, EG, P2, 
  ST_PYR, COLOR1, EG, EH, ED, EB, N0, 
  ST_PYR, COLOR1, ED, P0, P1, EB, N0, 
  ST_TET, COLOR1, P0, P4, P1, N0, 
  ST_TET, COLOR1, EH, EG, P4, N0, 
  ST_PYR, COLOR1, EH, P4, P0, ED, N0, 
  ST_PYR, COLOR1, P4, EG, EB, P1, N0, 
 // Case #20: (cloned #17)
  ST_WDG, COLOR0, EB, EF, P1, EC, EH, P3, 
  ST_WDG, COLOR0, EH, EE, EF, P3, P0, P1, 
  ST_WDG, COLOR1, P4, EH, EF, P2, EC, EB, 
  ST_TET, COLOR1, EH, EE, EF, P4, 
 // Case #21: Unique case #10
  ST_TET, COLOR0, EA, P1, EB, EF, 
  ST_TET, COLOR0, P3, ED, EC, EH, 
  ST_WDG, COLOR1, EA, EB, EF, P0, P2, P4, 
  ST_WDG, COLOR1, EC, ED, EH, P2, P0, P4, 
 // Case #22: (cloned #19)
  ST_PNT, 0, COLOR1, 7, EE, EH, EA, EC, P1, P2, P4, 
  ST_WDG, COLOR0, EA, EE, P0, EC, EH, P3, 
  ST_PYR, COLOR1, EH, EE, EA, EC, N0, 
  ST_PYR, COLOR1, EA, P1, P2, EC, N0, 
  ST_TET, COLOR1, P1, P4, P2, N0, 
  ST_TET, COLOR1, EE, EH, P4, N0, 
  ST_PYR, COLOR1, EE, P4, P1, EA, N0, 
  ST_PYR, COLOR1, P4, EH, EC, P2, N0, 
 // Case #23: Unique case #11
  ST_TET, COLOR0, P3, ED, EC, EH, 
  ST_WDG, COLOR1, P0, P2, P4, ED, EC, EH, 
  ST_TET, COLOR1, P0, P1, P2, P4, 
 // Case #24: (cloned #17)
  ST_WDG, COLOR0, EC, EG, P2, ED, EE, P0, 
  ST_WDG, COLOR0, EE, EF, EG, P0, P1, P2, 
  ST_WDG, COLOR1, P4, EE, EG, P3, ED, EC, 
  ST_TET, COLOR1, EE, EF, EG, P4, 
 // Case #25: (cloned #19)
  ST_PNT, 0, COLOR1, 7, EG, EF, EC, EA, P3, P0, P4, 
  ST_WDG, COLOR0, EC, EG, P2, EA, EF, P1, 
  ST_PYR, COLOR1, EF, EG, EC, EA, N0, 
  ST_PYR, COLOR1, EC, P3, P0, EA, N0, 
  ST_TET, COLOR1, P3, P4, P0, N0, 
  ST_TET, COLOR1, EG, EF, P4, N0, 
  ST_PYR, COLOR1, EG, P4, P3, EC, N0, 
  ST_PYR, COLOR1, P4, EF, EA, P0, N0, 
 // Case #26: (cloned #21)
  ST_TET, COLOR0, ED, P0, EA, EE, 
  ST_TET, COLOR0, P2, EC, EB, EG, 
  ST_WDG, COLOR1, ED, EA, EE, P3, P1, P4, 
  ST_WDG, COLOR1, EB, EC, EG, P1, P3, P4, 
 // Case #27: (cloned #23)
  ST_TET, COLOR0, P2, EC, EB, EG, 
  ST_WDG, COLOR1, P3, P1, P4, EC, EB, EG, 
  ST_TET, COLOR1, P3, P0, P1, P4, 
 // Case #28: (cloned #19)
  ST_PNT, 0, COLOR1, 7, EF, EE, EB, ED, P2, P3, P4, 
  ST_WDG, COLOR0, EB, EF, P1, ED, EE, P0, 
  ST_PYR, COLOR1, EE, EF, EB, ED, N0, 
  ST_PYR, COLOR1, EB, P2, P3, ED, N0, 
  ST_TET, COLOR1, P2, P4, P3, N0, 
  ST_TET, COLOR1, EF, EE, P4, N0, 
  ST_PYR, COLOR1, EF, P4, P2, EB, N0, 
  ST_PYR, COLOR1, P4, EE, ED, P3, N0, 
 // Case #29: (cloned #23)
  ST_TET, COLOR0, P1, EB, EA, EF, 
  ST_WDG, COLOR1, P2, P0, P4, EB, EA, EF, 
  ST_TET, COLOR1, P2, P3, P0, P4, 
 // Case #30: (cloned #23)
  ST_TET, COLOR0, P0, EA, ED, EE, 
  ST_WDG, COLOR1, P1, P3, P4, EA, ED, EE, 
  ST_TET, COLOR1, P1, P2, P3, P4, 
 // Case #31: Unique case #12
  ST_PYR, COLOR1, P0, P1, P2, P3, P4, 
 // Dummy
  0
};

