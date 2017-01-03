/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "ClipCases.h"

// This file is meant to be read and created by a program other than a
// compiler.  If you must modify it by hand, at least be nice to the 
// parser and don't add anything else to this file or rearrange it.

int numClipCasesPoly6 = 64;

int numClipShapesPoly6[64] = {
  2,  4,  4,  3,  4,  5,  3,  4, // cases 0 - 7
  4,  4,  5,  5,  3,  5,  4,  3, // cases 8 - 15
  4,  5,  4,  5,  5,  7,  5,  5, // cases 16 - 23
  3,  5,  5,  5,  4,  5,  3,  4, // cases 24 - 31
  4,  3,  5,  4,  4,  5,  5,  3, // cases 32 - 39
  5,  5,  7,  5,  5,  5,  5,  4, // cases 40 - 47
  3,  4,  5,  3,  5,  5,  5,  4, // cases 48 - 55
  4,  3,  5,  4,  3,  4,  4,  2  // cases 56 - 63
};

int startClipShapesPoly6[64] = {
  0, 12, 34, 56, 74, 96, 124, 142, // cases 0 - 7
  164, 186, 210, 238, 266, 284, 312, 334, // cases 8 - 15
  352, 374, 402, 426, 454, 482, 520, 548, // cases 16 - 23
  576, 594, 622, 650, 678, 700, 728, 746, // cases 24 - 31
  768, 790, 808, 836, 858, 882, 910, 938, // cases 32 - 39
  956, 984, 1012, 1050, 1078, 1106, 1134, 1162, // cases 40 - 47
  1184, 1202, 1224, 1252, 1270, 1298, 1326, 1354, // cases 48 - 55
  1376, 1398, 1416, 1444, 1466, 1484, 1506, 1528  // cases 56 - 63
};

unsigned char clipShapesPoly6[] = {
 // Case #0: Unique case #1
  ST_QUA, COLOR0, P0, P1, P2, P3, 
  ST_QUA, COLOR0, P0, P3, P4, P5, 
 // Case #1: Unique case #2
  ST_QUA, COLOR0, EA, P1, P2, P3, 
  ST_QUA, COLOR0, EA, P3, P4, P5, 
  ST_TRI, COLOR0, EA, P5, EF, 
  ST_TRI, COLOR1, P0, EA, EF, 
 // Case #2: (cloned #1)
  ST_QUA, COLOR0, EB, P2, P3, P4, 
  ST_QUA, COLOR0, EB, P4, P5, P0, 
  ST_TRI, COLOR0, EB, P0, EA, 
  ST_TRI, COLOR1, P1, EB, EA, 
 // Case #3: Unique case #3
  ST_QUA, COLOR0, EB, P2, P3, P4, 
  ST_QUA, COLOR0, EB, P4, P5, EF, 
  ST_QUA, COLOR1, P0, P1, EB, EF, 
 // Case #4: (cloned #1)
  ST_QUA, COLOR0, EC, P3, P4, P5, 
  ST_QUA, COLOR0, EC, P5, P0, P1, 
  ST_TRI, COLOR0, EC, P1, EB, 
  ST_TRI, COLOR1, P2, EC, EB, 
 // Case #5: Unique case #4
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR0, EC, P3, P4, P5, 
  ST_TRI, COLOR0, EC, P5, EF, 
  ST_QUA, COLOR1, P0, EA, EB, P2, 
  ST_QUA, COLOR1, P0, P2, EC, EF, 
 // Case #6: (cloned #3)
  ST_QUA, COLOR0, EC, P3, P4, P5, 
  ST_QUA, COLOR0, EC, P5, P0, EA, 
  ST_QUA, COLOR1, P1, P2, EC, EA, 
 // Case #7: Unique case #5
  ST_QUA, COLOR0, EC, P3, P4, P5, 
  ST_TRI, COLOR0, EC, P5, EF, 
  ST_QUA, COLOR1, P0, P1, P2, EC, 
  ST_TRI, COLOR1, P0, EC, EF, 
 // Case #8: (cloned #1)
  ST_QUA, COLOR0, ED, P4, P5, P0, 
  ST_QUA, COLOR0, ED, P0, P1, P2, 
  ST_TRI, COLOR0, ED, P2, EC, 
  ST_TRI, COLOR1, P3, ED, EC, 
 // Case #9: Unique case #6
  ST_QUA, COLOR0, EA, P1, P2, EC, 
  ST_QUA, COLOR0, ED, P4, P5, EF, 
  ST_QUA, COLOR1, P0, EA, EC, P3, 
  ST_QUA, COLOR1, P0, P3, ED, EF, 
 // Case #10: (cloned #5)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR0, ED, P4, P5, P0, 
  ST_TRI, COLOR0, ED, P0, EA, 
  ST_QUA, COLOR1, P1, EB, EC, P3, 
  ST_QUA, COLOR1, P1, P3, ED, EA, 
 // Case #11: Unique case #7
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR0, ED, P4, P5, EF, 
  ST_QUA, COLOR1, P0, P1, EB, EC, 
  ST_QUA, COLOR1, P0, EC, P3, ED, 
  ST_TRI, COLOR1, P0, ED, EF, 
 // Case #12: (cloned #3)
  ST_QUA, COLOR0, ED, P4, P5, P0, 
  ST_QUA, COLOR0, ED, P0, P1, EB, 
  ST_QUA, COLOR1, P2, P3, ED, EB, 
 // Case #13: (cloned #11)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR0, ED, P4, P5, EF, 
  ST_QUA, COLOR1, EA, EB, P2, P3, 
  ST_QUA, COLOR1, EF, P0, EA, P3, 
  ST_TRI, COLOR1, ED, EF, P3, 
 // Case #14: (cloned #7)
  ST_QUA, COLOR0, ED, P4, P5, P0, 
  ST_TRI, COLOR0, ED, P0, EA, 
  ST_QUA, COLOR1, P1, P2, P3, ED, 
  ST_TRI, COLOR1, P1, ED, EA, 
 // Case #15: Unique case #8
  ST_QUA, COLOR0, ED, P4, P5, EF, 
  ST_QUA, COLOR1, P0, P1, P2, P3, 
  ST_QUA, COLOR1, P0, P3, ED, EF, 
 // Case #16: (cloned #1)
  ST_QUA, COLOR0, EE, P5, P0, P1, 
  ST_QUA, COLOR0, EE, P1, P2, P3, 
  ST_TRI, COLOR0, EE, P3, ED, 
  ST_TRI, COLOR1, P4, EE, ED, 
 // Case #17: (cloned #5)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR0, EA, P1, P2, P3, 
  ST_TRI, COLOR0, EA, P3, ED, 
  ST_QUA, COLOR1, P4, EE, EF, P0, 
  ST_QUA, COLOR1, P4, P0, EA, ED, 
 // Case #18: (cloned #9)
  ST_QUA, COLOR0, EB, P2, P3, ED, 
  ST_QUA, COLOR0, EE, P5, P0, EA, 
  ST_QUA, COLOR1, P1, EB, ED, P4, 
  ST_QUA, COLOR1, P1, P4, EE, EA, 
 // Case #19: (cloned #11)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR0, EB, P2, P3, ED, 
  ST_QUA, COLOR1, EE, EF, P0, P1, 
  ST_QUA, COLOR1, ED, P4, EE, P1, 
  ST_TRI, COLOR1, EB, ED, P1, 
 // Case #20: (cloned #5)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR0, EE, P5, P0, P1, 
  ST_TRI, COLOR0, EE, P1, EB, 
  ST_QUA, COLOR1, P2, EC, ED, P4, 
  ST_QUA, COLOR1, P2, P4, EE, EB, 
 // Case #21: Unique case #9
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR1, P0, EA, EB, P2, 
  ST_QUA, COLOR1, P0, P2, EC, ED, 
  ST_QUA, COLOR1, P0, ED, P4, EE, 
  ST_TRI, COLOR1, P0, EE, EF, 
 // Case #22: (cloned #11)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR0, EE, P5, P0, EA, 
  ST_QUA, COLOR1, P1, P2, EC, ED, 
  ST_QUA, COLOR1, P1, ED, P4, EE, 
  ST_TRI, COLOR1, P1, EE, EA, 
 // Case #23: Unique case #10
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR1, P0, P1, P2, EC, 
  ST_QUA, COLOR1, P0, EC, ED, P4, 
  ST_QUA, COLOR1, P0, P4, EE, EF, 
 // Case #24: (cloned #3)
  ST_QUA, COLOR0, EE, P5, P0, P1, 
  ST_QUA, COLOR0, EE, P1, P2, EC, 
  ST_QUA, COLOR1, P3, P4, EE, EC, 
 // Case #25: (cloned #11)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR0, EA, P1, P2, EC, 
  ST_QUA, COLOR1, P3, P4, EE, EF, 
  ST_QUA, COLOR1, P3, EF, P0, EA, 
  ST_TRI, COLOR1, P3, EA, EC, 
 // Case #26: (cloned #11)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR0, EE, P5, P0, EA, 
  ST_QUA, COLOR1, EB, EC, P3, P4, 
  ST_QUA, COLOR1, EA, P1, EB, P4, 
  ST_TRI, COLOR1, EE, EA, P4, 
 // Case #27: Unique case #11
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR1, P0, P1, EB, EC, 
  ST_QUA, COLOR1, P0, EC, P3, P4, 
  ST_QUA, COLOR1, P0, P4, EE, EF, 
 // Case #28: (cloned #7)
  ST_QUA, COLOR0, EE, P5, P0, P1, 
  ST_TRI, COLOR0, EE, P1, EB, 
  ST_QUA, COLOR1, P2, P3, P4, EE, 
  ST_TRI, COLOR1, P2, EE, EB, 
 // Case #29: (cloned #23)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR1, P2, P3, P4, EE, 
  ST_QUA, COLOR1, P2, EE, EF, P0, 
  ST_QUA, COLOR1, P2, P0, EA, EB, 
 // Case #30: (cloned #15)
  ST_QUA, COLOR0, EE, P5, P0, EA, 
  ST_QUA, COLOR1, P1, P2, P3, P4, 
  ST_QUA, COLOR1, P1, P4, EE, EA, 
 // Case #31: Unique case #12
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR1, P0, P1, P2, P3, 
  ST_QUA, COLOR1, P0, P3, P4, EE, 
  ST_TRI, COLOR1, EE, EF, P0, 
 // Case #32: (cloned #1)
  ST_QUA, COLOR0, EF, P0, P1, P2, 
  ST_QUA, COLOR0, EF, P2, P3, P4, 
  ST_TRI, COLOR0, EF, P4, EE, 
  ST_TRI, COLOR1, P5, EF, EE, 
 // Case #33: (cloned #3)
  ST_QUA, COLOR0, EA, P1, P2, P3, 
  ST_QUA, COLOR0, EA, P3, P4, EE, 
  ST_QUA, COLOR1, P5, P0, EA, EE, 
 // Case #34: (cloned #5)
  ST_TRI, COLOR0, EF, P0, EA, 
  ST_QUA, COLOR0, EB, P2, P3, P4, 
  ST_TRI, COLOR0, EB, P4, EE, 
  ST_QUA, COLOR1, P5, EF, EA, P1, 
  ST_QUA, COLOR1, P5, P1, EB, EE, 
 // Case #35: (cloned #7)
  ST_QUA, COLOR0, EB, P2, P3, P4, 
  ST_TRI, COLOR0, EB, P4, EE, 
  ST_QUA, COLOR1, P5, P0, P1, EB, 
  ST_TRI, COLOR1, P5, EB, EE, 
 // Case #36: (cloned #9)
  ST_QUA, COLOR0, EC, P3, P4, EE, 
  ST_QUA, COLOR0, EF, P0, P1, EB, 
  ST_QUA, COLOR1, P2, EC, EE, P5, 
  ST_QUA, COLOR1, P2, P5, EF, EB, 
 // Case #37: (cloned #11)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR0, EC, P3, P4, EE, 
  ST_QUA, COLOR1, P5, P0, EA, EB, 
  ST_QUA, COLOR1, P5, EB, P2, EC, 
  ST_TRI, COLOR1, P5, EC, EE, 
 // Case #38: (cloned #11)
  ST_TRI, COLOR0, EF, P0, EA, 
  ST_QUA, COLOR0, EC, P3, P4, EE, 
  ST_QUA, COLOR1, EF, EA, P1, P2, 
  ST_QUA, COLOR1, EE, P5, EF, P2, 
  ST_TRI, COLOR1, EC, EE, P2, 
 // Case #39: (cloned #15)
  ST_QUA, COLOR0, EC, P3, P4, EE, 
  ST_QUA, COLOR1, P5, P0, P1, P2, 
  ST_QUA, COLOR1, P5, P2, EC, EE, 
 // Case #40: (cloned #5)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR0, EF, P0, P1, P2, 
  ST_TRI, COLOR0, EF, P2, EC, 
  ST_QUA, COLOR1, P3, ED, EE, P5, 
  ST_QUA, COLOR1, P3, P5, EF, EC, 
 // Case #41: (cloned #11)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR0, EA, P1, P2, EC, 
  ST_QUA, COLOR1, ED, EE, P5, P0, 
  ST_QUA, COLOR1, EC, P3, ED, P0, 
  ST_TRI, COLOR1, EA, EC, P0, 
 // Case #42: (cloned #21)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_TRI, COLOR0, EF, P0, EA, 
  ST_QUA, COLOR1, P1, EB, EC, P3, 
  ST_QUA, COLOR1, P1, P3, ED, EE, 
  ST_QUA, COLOR1, P1, EE, P5, EF, 
  ST_TRI, COLOR1, P1, EF, EA, 
 // Case #43: (cloned #23)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR1, P5, P0, P1, EB, 
  ST_QUA, COLOR1, P5, EB, EC, P3, 
  ST_QUA, COLOR1, P5, P3, ED, EE, 
 // Case #44: (cloned #11)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR0, EF, P0, P1, EB, 
  ST_QUA, COLOR1, P2, P3, ED, EE, 
  ST_QUA, COLOR1, P2, EE, P5, EF, 
  ST_TRI, COLOR1, P2, EF, EB, 
 // Case #45: (cloned #27)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR1, P2, P3, ED, EE, 
  ST_QUA, COLOR1, P2, EE, P5, P0, 
  ST_QUA, COLOR1, P2, P0, EA, EB, 
 // Case #46: (cloned #23)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_TRI, COLOR0, EF, P0, EA, 
  ST_QUA, COLOR1, P1, P2, P3, ED, 
  ST_QUA, COLOR1, P1, ED, EE, P5, 
  ST_QUA, COLOR1, P1, P5, EF, EA, 
 // Case #47: (cloned #31)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR1, P5, P0, P1, P2, 
  ST_QUA, COLOR1, P5, P2, P3, ED, 
  ST_TRI, COLOR1, ED, EE, P5, 
 // Case #48: (cloned #3)
  ST_QUA, COLOR0, EF, P0, P1, P2, 
  ST_QUA, COLOR0, EF, P2, P3, ED, 
  ST_QUA, COLOR1, P4, P5, EF, ED, 
 // Case #49: (cloned #7)
  ST_QUA, COLOR0, EA, P1, P2, P3, 
  ST_TRI, COLOR0, EA, P3, ED, 
  ST_QUA, COLOR1, P4, P5, P0, EA, 
  ST_TRI, COLOR1, P4, EA, ED, 
 // Case #50: (cloned #11)
  ST_TRI, COLOR0, EF, P0, EA, 
  ST_QUA, COLOR0, EB, P2, P3, ED, 
  ST_QUA, COLOR1, P4, P5, EF, EA, 
  ST_QUA, COLOR1, P4, EA, P1, EB, 
  ST_TRI, COLOR1, P4, EB, ED, 
 // Case #51: (cloned #15)
  ST_QUA, COLOR0, EB, P2, P3, ED, 
  ST_QUA, COLOR1, P4, P5, P0, P1, 
  ST_QUA, COLOR1, P4, P1, EB, ED, 
 // Case #52: (cloned #11)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR0, EF, P0, P1, EB, 
  ST_QUA, COLOR1, EC, ED, P4, P5, 
  ST_QUA, COLOR1, EB, P2, EC, P5, 
  ST_TRI, COLOR1, EF, EB, P5, 
 // Case #53: (cloned #23)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR1, P4, P5, P0, EA, 
  ST_QUA, COLOR1, P4, EA, EB, P2, 
  ST_QUA, COLOR1, P4, P2, EC, ED, 
 // Case #54: (cloned #27)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_TRI, COLOR0, EF, P0, EA, 
  ST_QUA, COLOR1, P1, P2, EC, ED, 
  ST_QUA, COLOR1, P1, ED, P4, P5, 
  ST_QUA, COLOR1, P1, P5, EF, EA, 
 // Case #55: (cloned #31)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR1, P4, P5, P0, P1, 
  ST_QUA, COLOR1, P4, P1, P2, EC, 
  ST_TRI, COLOR1, EC, ED, P4, 
 // Case #56: (cloned #7)
  ST_QUA, COLOR0, EF, P0, P1, P2, 
  ST_TRI, COLOR0, EF, P2, EC, 
  ST_QUA, COLOR1, P3, P4, P5, EF, 
  ST_TRI, COLOR1, P3, EF, EC, 
 // Case #57: (cloned #15)
  ST_QUA, COLOR0, EA, P1, P2, EC, 
  ST_QUA, COLOR1, P3, P4, P5, P0, 
  ST_QUA, COLOR1, P3, P0, EA, EC, 
 // Case #58: (cloned #23)
  ST_TRI, COLOR0, EF, P0, EA, 
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR1, P3, P4, P5, EF, 
  ST_QUA, COLOR1, P3, EF, EA, P1, 
  ST_QUA, COLOR1, P3, P1, EB, EC, 
 // Case #59: (cloned #31)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR1, P3, P4, P5, P0, 
  ST_QUA, COLOR1, P3, P0, P1, EB, 
  ST_TRI, COLOR1, EB, EC, P3, 
 // Case #60: (cloned #15)
  ST_QUA, COLOR0, EF, P0, P1, EB, 
  ST_QUA, COLOR1, P2, P3, P4, P5, 
  ST_QUA, COLOR1, P2, P5, EF, EB, 
 // Case #61: (cloned #31)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR1, P2, P3, P4, P5, 
  ST_QUA, COLOR1, P2, P5, P0, EA, 
  ST_TRI, COLOR1, EA, EB, P2, 
 // Case #62: (cloned #31)
  ST_TRI, COLOR0, EF, P0, EA, 
  ST_QUA, COLOR1, P1, P2, P3, P4, 
  ST_QUA, COLOR1, P1, P4, P5, EF, 
  ST_TRI, COLOR1, EF, EA, P1, 
 // Case #63: Unique case #13
  ST_QUA, COLOR1, P0, P1, P2, P3, 
  ST_QUA, COLOR1, P0, P3, P4, P5, 
 // Dummy
  0
};

