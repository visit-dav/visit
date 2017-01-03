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

int numClipCasesPoly7 = 128;

int numClipShapesPoly7[128] = {
  3,  4,  4,  4,  4,  5,  4,  4, // cases 0 - 7
  4,  5,  5,  6,  4,  6,  4,  4, // cases 8 - 15
  4,  5,  5,  5,  5,  7,  6,  5, // cases 16 - 23
  4,  5,  6,  5,  4,  5,  4,  4, // cases 24 - 31
  4,  5,  5,  6,  5,  7,  5,  5, // cases 32 - 39
  5,  7,  7,  7,  6,  7,  5,  6, // cases 40 - 47
  4,  6,  5,  5,  6,  7,  5,  6, // cases 48 - 55
  4,  5,  5,  6,  4,  6,  4,  4, // cases 56 - 63
  4,  4,  5,  4,  5,  6,  6,  4, // cases 64 - 71
  5,  5,  7,  5,  5,  5,  5,  4, // cases 72 - 79
  5,  6,  7,  5,  7,  7,  7,  6, // cases 80 - 87
  6,  5,  7,  6,  5,  6,  6,  4, // cases 88 - 95
  4,  4,  6,  4,  5,  5,  5,  4, // cases 96 - 103
  6,  5,  7,  6,  5,  6,  6,  4, // cases 104 - 111
  4,  4,  5,  4,  5,  6,  6,  4, // cases 112 - 119
  4,  4,  6,  4,  4,  4,  4,  3  // cases 120 - 127
};

int startClipShapesPoly7[128] = {
  0, 17, 40, 63, 86, 109, 138, 161, // cases 0 - 7
  184, 207, 236, 265, 298, 321, 354, 377, // cases 8 - 15
  400, 423, 452, 481, 510, 539, 578, 611, // cases 16 - 23
  640, 663, 692, 725, 754, 777, 806, 829, // cases 24 - 31
  852, 875, 904, 933, 966, 995, 1034, 1063, // cases 32 - 39
  1092, 1121, 1160, 1199, 1238, 1271, 1310, 1339, // cases 40 - 47
  1372, 1395, 1428, 1457, 1486, 1519, 1558, 1587, // cases 48 - 55
  1620, 1643, 1672, 1701, 1734, 1757, 1790, 1813, // cases 56 - 63
  1836, 1859, 1882, 1911, 1934, 1963, 1996, 2029, // cases 64 - 71
  2052, 2081, 2110, 2149, 2178, 2207, 2236, 2265, // cases 72 - 79
  2288, 2317, 2350, 2389, 2418, 2457, 2496, 2535, // cases 80 - 87
  2568, 2601, 2630, 2669, 2702, 2731, 2764, 2797, // cases 88 - 95
  2820, 2843, 2866, 2899, 2922, 2951, 2980, 3009, // cases 96 - 103
  3032, 3065, 3094, 3133, 3166, 3195, 3228, 3261, // cases 104 - 111
  3284, 3307, 3330, 3359, 3382, 3411, 3444, 3477, // cases 112 - 119
  3500, 3523, 3546, 3579, 3602, 3625, 3648, 3671  // cases 120 - 127
};

unsigned char clipShapesPoly7[] = {
 // Case #0: Unique case #1
  ST_QUA, COLOR0, P0, P1, P2, P3, 
  ST_QUA, COLOR0, P0, P3, P4, P5, 
  ST_TRI, COLOR0, P0, P5, P6, 
 // Case #1: Unique case #2
  ST_QUA, COLOR0, EA, P1, P2, P3, 
  ST_QUA, COLOR0, EA, P3, P4, P5, 
  ST_QUA, COLOR0, EA, P5, P6, EG, 
  ST_TRI, COLOR1, P0, EA, EG, 
 // Case #2: (cloned #1)
  ST_QUA, COLOR0, EB, P2, P3, P4, 
  ST_QUA, COLOR0, EB, P4, P5, P6, 
  ST_QUA, COLOR0, EB, P6, P0, EA, 
  ST_TRI, COLOR1, P1, EB, EA, 
 // Case #3: Unique case #3
  ST_QUA, COLOR0, EB, P2, P3, P4, 
  ST_QUA, COLOR0, EB, P4, P5, P6, 
  ST_TRI, COLOR0, EB, P6, EG, 
  ST_QUA, COLOR1, P0, P1, EB, EG, 
 // Case #4: (cloned #1)
  ST_QUA, COLOR0, EC, P3, P4, P5, 
  ST_QUA, COLOR0, EC, P5, P6, P0, 
  ST_QUA, COLOR0, EC, P0, P1, EB, 
  ST_TRI, COLOR1, P2, EC, EB, 
 // Case #5: Unique case #4
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR0, EC, P3, P4, P5, 
  ST_QUA, COLOR0, EC, P5, P6, EG, 
  ST_QUA, COLOR1, P0, EA, EB, P2, 
  ST_QUA, COLOR1, P0, P2, EC, EG, 
 // Case #6: (cloned #3)
  ST_QUA, COLOR0, EC, P3, P4, P5, 
  ST_QUA, COLOR0, EC, P5, P6, P0, 
  ST_TRI, COLOR0, EC, P0, EA, 
  ST_QUA, COLOR1, P1, P2, EC, EA, 
 // Case #7: Unique case #5
  ST_QUA, COLOR0, EC, P3, P4, P5, 
  ST_QUA, COLOR0, EC, P5, P6, EG, 
  ST_QUA, COLOR1, P0, P1, P2, EC, 
  ST_TRI, COLOR1, P0, EC, EG, 
 // Case #8: (cloned #1)
  ST_QUA, COLOR0, ED, P4, P5, P6, 
  ST_QUA, COLOR0, ED, P6, P0, P1, 
  ST_QUA, COLOR0, ED, P1, P2, EC, 
  ST_TRI, COLOR1, P3, ED, EC, 
 // Case #9: Unique case #6
  ST_QUA, COLOR0, EA, P1, P2, EC, 
  ST_QUA, COLOR0, ED, P4, P5, P6, 
  ST_TRI, COLOR0, ED, P6, EG, 
  ST_QUA, COLOR1, P0, EA, EC, P3, 
  ST_QUA, COLOR1, P0, P3, ED, EG, 
 // Case #10: (cloned #5)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR0, ED, P4, P5, P6, 
  ST_QUA, COLOR0, ED, P6, P0, EA, 
  ST_QUA, COLOR1, P1, EB, EC, P3, 
  ST_QUA, COLOR1, P1, P3, ED, EA, 
 // Case #11: Unique case #7
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR0, ED, P4, P5, P6, 
  ST_TRI, COLOR0, ED, P6, EG, 
  ST_QUA, COLOR1, P0, P1, EB, EC, 
  ST_QUA, COLOR1, P0, EC, P3, ED, 
  ST_TRI, COLOR1, P0, ED, EG, 
 // Case #12: (cloned #3)
  ST_QUA, COLOR0, ED, P4, P5, P6, 
  ST_QUA, COLOR0, ED, P6, P0, P1, 
  ST_TRI, COLOR0, ED, P1, EB, 
  ST_QUA, COLOR1, P2, P3, ED, EB, 
 // Case #13: (cloned #11)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR0, P4, P5, P6, EG, 
  ST_TRI, COLOR0, ED, P4, EG, 
  ST_QUA, COLOR1, EA, EB, P2, P3, 
  ST_QUA, COLOR1, EG, P0, EA, P3, 
  ST_TRI, COLOR1, ED, EG, P3, 
 // Case #14: (cloned #7)
  ST_QUA, COLOR0, ED, P4, P5, P6, 
  ST_QUA, COLOR0, ED, P6, P0, EA, 
  ST_QUA, COLOR1, P1, P2, P3, ED, 
  ST_TRI, COLOR1, P1, ED, EA, 
 // Case #15: Unique case #8
  ST_QUA, COLOR0, ED, P4, P5, P6, 
  ST_TRI, COLOR0, ED, P6, EG, 
  ST_QUA, COLOR1, P0, P1, P2, P3, 
  ST_QUA, COLOR1, P0, P3, ED, EG, 
 // Case #16: (cloned #1)
  ST_QUA, COLOR0, EE, P5, P6, P0, 
  ST_QUA, COLOR0, EE, P0, P1, P2, 
  ST_QUA, COLOR0, EE, P2, P3, ED, 
  ST_TRI, COLOR1, P4, EE, ED, 
 // Case #17: (cloned #9)
  ST_QUA, COLOR0, EE, P5, P6, EG, 
  ST_QUA, COLOR0, EA, P1, P2, P3, 
  ST_TRI, COLOR0, EA, P3, ED, 
  ST_QUA, COLOR1, P4, EE, EG, P0, 
  ST_QUA, COLOR1, P4, P0, EA, ED, 
 // Case #18: (cloned #9)
  ST_QUA, COLOR0, EB, P2, P3, ED, 
  ST_QUA, COLOR0, EE, P5, P6, P0, 
  ST_TRI, COLOR0, EE, P0, EA, 
  ST_QUA, COLOR1, P1, EB, ED, P4, 
  ST_QUA, COLOR1, P1, P4, EE, EA, 
 // Case #19: Unique case #9
  ST_QUA, COLOR0, EB, P2, P3, ED, 
  ST_QUA, COLOR0, EE, P5, P6, EG, 
  ST_QUA, COLOR1, P0, P1, EB, ED, 
  ST_QUA, COLOR1, P0, ED, P4, EE, 
  ST_TRI, COLOR1, P0, EE, EG, 
 // Case #20: (cloned #5)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR0, EE, P5, P6, P0, 
  ST_QUA, COLOR0, EE, P0, P1, EB, 
  ST_QUA, COLOR1, P2, EC, ED, P4, 
  ST_QUA, COLOR1, P2, P4, EE, EB, 
 // Case #21: Unique case #10
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR0, EE, P5, P6, EG, 
  ST_QUA, COLOR1, P0, EA, EB, P2, 
  ST_QUA, COLOR1, P0, P2, EC, ED, 
  ST_QUA, COLOR1, P0, ED, P4, EE, 
  ST_TRI, COLOR1, P0, EE, EG, 
 // Case #22: (cloned #11)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR0, EE, P5, P6, P0, 
  ST_TRI, COLOR0, EE, P0, EA, 
  ST_QUA, COLOR1, P1, P2, EC, ED, 
  ST_QUA, COLOR1, P1, ED, P4, EE, 
  ST_TRI, COLOR1, P1, EE, EA, 
 // Case #23: Unique case #11
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR0, EE, P5, P6, EG, 
  ST_QUA, COLOR1, P0, P1, P2, EC, 
  ST_QUA, COLOR1, P0, EC, ED, P4, 
  ST_QUA, COLOR1, P0, P4, EE, EG, 
 // Case #24: (cloned #3)
  ST_QUA, COLOR0, EE, P5, P6, P0, 
  ST_QUA, COLOR0, EE, P0, P1, P2, 
  ST_TRI, COLOR0, EE, P2, EC, 
  ST_QUA, COLOR1, P3, P4, EE, EC, 
 // Case #25: (cloned #19)
  ST_QUA, COLOR0, EE, P5, P6, EG, 
  ST_QUA, COLOR0, EA, P1, P2, EC, 
  ST_QUA, COLOR1, P3, P4, EE, EG, 
  ST_QUA, COLOR1, P3, EG, P0, EA, 
  ST_TRI, COLOR1, P3, EA, EC, 
 // Case #26: (cloned #11)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR0, P5, P6, P0, EA, 
  ST_TRI, COLOR0, EE, P5, EA, 
  ST_QUA, COLOR1, EB, EC, P3, P4, 
  ST_QUA, COLOR1, EA, P1, EB, P4, 
  ST_TRI, COLOR1, EE, EA, P4, 
 // Case #27: Unique case #12
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR0, EE, P5, P6, EG, 
  ST_QUA, COLOR1, P0, P1, EB, EC, 
  ST_QUA, COLOR1, P0, EC, P3, P4, 
  ST_QUA, COLOR1, P0, P4, EE, EG, 
 // Case #28: (cloned #7)
  ST_QUA, COLOR0, EE, P5, P6, P0, 
  ST_QUA, COLOR0, EE, P0, P1, EB, 
  ST_QUA, COLOR1, P2, P3, P4, EE, 
  ST_TRI, COLOR1, P2, EE, EB, 
 // Case #29: (cloned #23)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR0, EE, P5, P6, EG, 
  ST_QUA, COLOR1, EB, P2, P3, P4, 
  ST_QUA, COLOR1, P0, EA, EB, P4, 
  ST_QUA, COLOR1, EE, EG, P0, P4, 
 // Case #30: (cloned #15)
  ST_QUA, COLOR0, EE, P5, P6, P0, 
  ST_TRI, COLOR0, EE, P0, EA, 
  ST_QUA, COLOR1, P1, P2, P3, P4, 
  ST_QUA, COLOR1, P1, P4, EE, EA, 
 // Case #31: Unique case #13
  ST_QUA, COLOR0, EE, P5, P6, EG, 
  ST_QUA, COLOR1, P0, P1, P2, P3, 
  ST_QUA, COLOR1, P0, P3, P4, EE, 
  ST_TRI, COLOR1, P0, EE, EG, 
 // Case #32: (cloned #1)
  ST_QUA, COLOR0, EF, P6, P0, P1, 
  ST_QUA, COLOR0, EF, P1, P2, P3, 
  ST_QUA, COLOR0, EF, P3, P4, EE, 
  ST_TRI, COLOR1, P5, EF, EE, 
 // Case #33: (cloned #5)
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_QUA, COLOR0, EA, P1, P2, P3, 
  ST_QUA, COLOR0, EA, P3, P4, EE, 
  ST_QUA, COLOR1, P5, EF, EG, P0, 
  ST_QUA, COLOR1, P5, P0, EA, EE, 
 // Case #34: (cloned #9)
  ST_QUA, COLOR0, EF, P6, P0, EA, 
  ST_QUA, COLOR0, EB, P2, P3, P4, 
  ST_TRI, COLOR0, EB, P4, EE, 
  ST_QUA, COLOR1, P5, EF, EA, P1, 
  ST_QUA, COLOR1, P5, P1, EB, EE, 
 // Case #35: (cloned #11)
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_QUA, COLOR0, P2, P3, P4, EE, 
  ST_TRI, COLOR0, EB, P2, EE, 
  ST_QUA, COLOR1, EF, EG, P0, P1, 
  ST_QUA, COLOR1, EE, P5, EF, P1, 
  ST_TRI, COLOR1, EB, EE, P1, 
 // Case #36: (cloned #9)
  ST_QUA, COLOR0, EC, P3, P4, EE, 
  ST_QUA, COLOR0, EF, P6, P0, P1, 
  ST_TRI, COLOR0, EF, P1, EB, 
  ST_QUA, COLOR1, P2, EC, EE, P5, 
  ST_QUA, COLOR1, P2, P5, EF, EB, 
 // Case #37: (cloned #21)
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR0, EC, P3, P4, EE, 
  ST_QUA, COLOR1, P5, EF, EG, P0, 
  ST_QUA, COLOR1, P5, P0, EA, EB, 
  ST_QUA, COLOR1, P5, EB, P2, EC, 
  ST_TRI, COLOR1, P5, EC, EE, 
 // Case #38: (cloned #19)
  ST_QUA, COLOR0, EC, P3, P4, EE, 
  ST_QUA, COLOR0, EF, P6, P0, EA, 
  ST_QUA, COLOR1, P1, P2, EC, EE, 
  ST_QUA, COLOR1, P1, EE, P5, EF, 
  ST_TRI, COLOR1, P1, EF, EA, 
 // Case #39: (cloned #23)
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_QUA, COLOR0, EC, P3, P4, EE, 
  ST_QUA, COLOR1, EG, P0, P1, P2, 
  ST_QUA, COLOR1, P5, EF, EG, P2, 
  ST_QUA, COLOR1, EC, EE, P5, P2, 
 // Case #40: (cloned #5)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR0, EF, P6, P0, P1, 
  ST_QUA, COLOR0, EF, P1, P2, EC, 
  ST_QUA, COLOR1, P3, ED, EE, P5, 
  ST_QUA, COLOR1, P3, P5, EF, EC, 
 // Case #41: (cloned #21)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_QUA, COLOR0, EA, P1, P2, EC, 
  ST_QUA, COLOR1, P3, ED, EE, P5, 
  ST_QUA, COLOR1, P3, P5, EF, EG, 
  ST_QUA, COLOR1, P3, EG, P0, EA, 
  ST_TRI, COLOR1, P3, EA, EC, 
 // Case #42: (cloned #21)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR0, EF, P6, P0, EA, 
  ST_QUA, COLOR1, P1, EB, EC, P3, 
  ST_QUA, COLOR1, P1, P3, ED, EE, 
  ST_QUA, COLOR1, P1, EE, P5, EF, 
  ST_TRI, COLOR1, P1, EF, EA, 
 // Case #43: Unique case #14
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_QUA, COLOR1, P0, P1, EB, EC, 
  ST_QUA, COLOR1, P0, EC, P3, ED, 
  ST_QUA, COLOR1, P0, ED, EE, P5, 
  ST_QUA, COLOR1, P0, P5, EF, EG, 
 // Case #44: (cloned #11)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR0, EF, P6, P0, P1, 
  ST_TRI, COLOR0, EF, P1, EB, 
  ST_QUA, COLOR1, P2, P3, ED, EE, 
  ST_QUA, COLOR1, P2, EE, P5, EF, 
  ST_TRI, COLOR1, P2, EF, EB, 
 // Case #45: (cloned #43)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR1, P2, P3, ED, EE, 
  ST_QUA, COLOR1, P2, EE, P5, EF, 
  ST_QUA, COLOR1, P2, EF, EG, P0, 
  ST_QUA, COLOR1, P2, P0, EA, EB, 
 // Case #46: (cloned #23)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR0, EF, P6, P0, EA, 
  ST_QUA, COLOR1, P1, P2, P3, ED, 
  ST_QUA, COLOR1, P1, ED, EE, P5, 
  ST_QUA, COLOR1, P1, P5, EF, EA, 
 // Case #47: Unique case #15
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_QUA, COLOR1, P0, P1, P2, P3, 
  ST_QUA, COLOR1, P0, P3, ED, EE, 
  ST_QUA, COLOR1, P0, EE, P5, EF, 
  ST_TRI, COLOR1, P0, EF, EG, 
 // Case #48: (cloned #3)
  ST_QUA, COLOR0, EF, P6, P0, P1, 
  ST_QUA, COLOR0, EF, P1, P2, P3, 
  ST_TRI, COLOR0, EF, P3, ED, 
  ST_QUA, COLOR1, P4, P5, EF, ED, 
 // Case #49: (cloned #11)
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_QUA, COLOR0, EA, P1, P2, P3, 
  ST_TRI, COLOR0, EA, P3, ED, 
  ST_QUA, COLOR1, P4, P5, EF, EG, 
  ST_QUA, COLOR1, P4, EG, P0, EA, 
  ST_TRI, COLOR1, P4, EA, ED, 
 // Case #50: (cloned #19)
  ST_QUA, COLOR0, EF, P6, P0, EA, 
  ST_QUA, COLOR0, EB, P2, P3, ED, 
  ST_QUA, COLOR1, P4, P5, EF, EA, 
  ST_QUA, COLOR1, P4, EA, P1, EB, 
  ST_TRI, COLOR1, P4, EB, ED, 
 // Case #51: (cloned #27)
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_QUA, COLOR0, EB, P2, P3, ED, 
  ST_QUA, COLOR1, P4, P5, EF, EG, 
  ST_QUA, COLOR1, P4, EG, P0, P1, 
  ST_QUA, COLOR1, P4, P1, EB, ED, 
 // Case #52: (cloned #11)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR0, P6, P0, P1, EB, 
  ST_TRI, COLOR0, EF, P6, EB, 
  ST_QUA, COLOR1, EC, ED, P4, P5, 
  ST_QUA, COLOR1, EB, P2, EC, P5, 
  ST_TRI, COLOR1, EF, EB, P5, 
 // Case #53: (cloned #43)
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR1, P4, P5, EF, EG, 
  ST_QUA, COLOR1, P4, EG, P0, EA, 
  ST_QUA, COLOR1, P4, EA, EB, P2, 
  ST_QUA, COLOR1, P4, P2, EC, ED, 
 // Case #54: (cloned #27)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR0, EF, P6, P0, EA, 
  ST_QUA, COLOR1, P1, P2, EC, ED, 
  ST_QUA, COLOR1, P1, ED, P4, P5, 
  ST_QUA, COLOR1, P1, P5, EF, EA, 
 // Case #55: Unique case #16
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_QUA, COLOR1, P0, P1, P2, EC, 
  ST_QUA, COLOR1, P0, EC, ED, P4, 
  ST_QUA, COLOR1, P0, P4, P5, EF, 
  ST_TRI, COLOR1, P0, EF, EG, 
 // Case #56: (cloned #7)
  ST_QUA, COLOR0, EF, P6, P0, P1, 
  ST_QUA, COLOR0, EF, P1, P2, EC, 
  ST_QUA, COLOR1, P3, P4, P5, EF, 
  ST_TRI, COLOR1, P3, EF, EC, 
 // Case #57: (cloned #23)
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_QUA, COLOR0, EA, P1, P2, EC, 
  ST_QUA, COLOR1, P3, P4, P5, EF, 
  ST_QUA, COLOR1, P3, EF, EG, P0, 
  ST_QUA, COLOR1, P3, P0, EA, EC, 
 // Case #58: (cloned #23)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR0, EF, P6, P0, EA, 
  ST_QUA, COLOR1, EC, P3, P4, P5, 
  ST_QUA, COLOR1, P1, EB, EC, P5, 
  ST_QUA, COLOR1, EF, EA, P1, P5, 
 // Case #59: (cloned #55)
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR1, P3, P4, P5, EF, 
  ST_QUA, COLOR1, P3, EF, EG, P0, 
  ST_QUA, COLOR1, P3, P0, P1, EB, 
  ST_TRI, COLOR1, P3, EB, EC, 
 // Case #60: (cloned #15)
  ST_QUA, COLOR0, EF, P6, P0, P1, 
  ST_TRI, COLOR0, EF, P1, EB, 
  ST_QUA, COLOR1, P2, P3, P4, P5, 
  ST_QUA, COLOR1, P2, P5, EF, EB, 
 // Case #61: (cloned #47)
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR1, P2, P3, P4, P5, 
  ST_QUA, COLOR1, P2, P5, EF, EG, 
  ST_QUA, COLOR1, P2, EG, P0, EA, 
  ST_TRI, COLOR1, P2, EA, EB, 
 // Case #62: (cloned #31)
  ST_QUA, COLOR0, EF, P6, P0, EA, 
  ST_QUA, COLOR1, P1, P2, P3, P4, 
  ST_QUA, COLOR1, P1, P4, P5, EF, 
  ST_TRI, COLOR1, P1, EF, EA, 
 // Case #63: Unique case #17
  ST_TRI, COLOR0, EF, P6, EG, 
  ST_QUA, COLOR1, P0, P1, P2, P3, 
  ST_QUA, COLOR1, P0, P3, P4, P5, 
  ST_QUA, COLOR1, P0, P5, EF, EG, 
 // Case #64: (cloned #1)
  ST_QUA, COLOR0, EG, P0, P1, P2, 
  ST_QUA, COLOR0, EG, P2, P3, P4, 
  ST_QUA, COLOR0, EG, P4, P5, EF, 
  ST_TRI, COLOR1, P6, EG, EF, 
 // Case #65: (cloned #3)
  ST_QUA, COLOR0, EA, P1, P2, P3, 
  ST_QUA, COLOR0, EA, P3, P4, P5, 
  ST_TRI, COLOR0, EA, P5, EF, 
  ST_QUA, COLOR1, P6, P0, EA, EF, 
 // Case #66: (cloned #5)
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_QUA, COLOR0, EB, P2, P3, P4, 
  ST_QUA, COLOR0, EB, P4, P5, EF, 
  ST_QUA, COLOR1, P6, EG, EA, P1, 
  ST_QUA, COLOR1, P6, P1, EB, EF, 
 // Case #67: (cloned #7)
  ST_QUA, COLOR0, EB, P2, P3, P4, 
  ST_QUA, COLOR0, EB, P4, P5, EF, 
  ST_QUA, COLOR1, P6, P0, P1, EB, 
  ST_TRI, COLOR1, P6, EB, EF, 
 // Case #68: (cloned #9)
  ST_QUA, COLOR0, EG, P0, P1, EB, 
  ST_QUA, COLOR0, EC, P3, P4, P5, 
  ST_TRI, COLOR0, EC, P5, EF, 
  ST_QUA, COLOR1, P6, EG, EB, P2, 
  ST_QUA, COLOR1, P6, P2, EC, EF, 
 // Case #69: (cloned #11)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR0, EC, P3, P4, P5, 
  ST_TRI, COLOR0, EC, P5, EF, 
  ST_QUA, COLOR1, P6, P0, EA, EB, 
  ST_QUA, COLOR1, P6, EB, P2, EC, 
  ST_TRI, COLOR1, P6, EC, EF, 
 // Case #70: (cloned #11)
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_QUA, COLOR0, P3, P4, P5, EF, 
  ST_TRI, COLOR0, EC, P3, EF, 
  ST_QUA, COLOR1, EG, EA, P1, P2, 
  ST_QUA, COLOR1, EF, P6, EG, P2, 
  ST_TRI, COLOR1, EC, EF, P2, 
 // Case #71: (cloned #15)
  ST_QUA, COLOR0, EC, P3, P4, P5, 
  ST_TRI, COLOR0, EC, P5, EF, 
  ST_QUA, COLOR1, P6, P0, P1, P2, 
  ST_QUA, COLOR1, P6, P2, EC, EF, 
 // Case #72: (cloned #9)
  ST_QUA, COLOR0, ED, P4, P5, EF, 
  ST_QUA, COLOR0, EG, P0, P1, P2, 
  ST_TRI, COLOR0, EG, P2, EC, 
  ST_QUA, COLOR1, P3, ED, EF, P6, 
  ST_QUA, COLOR1, P3, P6, EG, EC, 
 // Case #73: (cloned #19)
  ST_QUA, COLOR0, EA, P1, P2, EC, 
  ST_QUA, COLOR0, ED, P4, P5, EF, 
  ST_QUA, COLOR1, P6, P0, EA, EC, 
  ST_QUA, COLOR1, P6, EC, P3, ED, 
  ST_TRI, COLOR1, P6, ED, EF, 
 // Case #74: (cloned #21)
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR0, ED, P4, P5, EF, 
  ST_QUA, COLOR1, P6, EG, EA, P1, 
  ST_QUA, COLOR1, P6, P1, EB, EC, 
  ST_QUA, COLOR1, P6, EC, P3, ED, 
  ST_TRI, COLOR1, P6, ED, EF, 
 // Case #75: (cloned #23)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR0, ED, P4, P5, EF, 
  ST_QUA, COLOR1, P6, P0, P1, EB, 
  ST_QUA, COLOR1, P6, EB, EC, P3, 
  ST_QUA, COLOR1, P6, P3, ED, EF, 
 // Case #76: (cloned #19)
  ST_QUA, COLOR0, ED, P4, P5, EF, 
  ST_QUA, COLOR0, EG, P0, P1, EB, 
  ST_QUA, COLOR1, P2, P3, ED, EF, 
  ST_QUA, COLOR1, P2, EF, P6, EG, 
  ST_TRI, COLOR1, P2, EG, EB, 
 // Case #77: (cloned #27)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR0, ED, P4, P5, EF, 
  ST_QUA, COLOR1, P6, P0, EA, EB, 
  ST_QUA, COLOR1, P6, EB, P2, P3, 
  ST_QUA, COLOR1, P6, P3, ED, EF, 
 // Case #78: (cloned #23)
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_QUA, COLOR0, ED, P4, P5, EF, 
  ST_QUA, COLOR1, EA, P1, P2, P3, 
  ST_QUA, COLOR1, P6, EG, EA, P3, 
  ST_QUA, COLOR1, ED, EF, P6, P3, 
 // Case #79: (cloned #31)
  ST_QUA, COLOR0, ED, P4, P5, EF, 
  ST_QUA, COLOR1, P6, P0, P1, P2, 
  ST_QUA, COLOR1, P6, P2, P3, ED, 
  ST_TRI, COLOR1, P6, ED, EF, 
 // Case #80: (cloned #5)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR0, EG, P0, P1, P2, 
  ST_QUA, COLOR0, EG, P2, P3, ED, 
  ST_QUA, COLOR1, P4, EE, EF, P6, 
  ST_QUA, COLOR1, P4, P6, EG, ED, 
 // Case #81: (cloned #11)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR0, P1, P2, P3, ED, 
  ST_TRI, COLOR0, EA, P1, ED, 
  ST_QUA, COLOR1, EE, EF, P6, P0, 
  ST_QUA, COLOR1, ED, P4, EE, P0, 
  ST_TRI, COLOR1, EA, ED, P0, 
 // Case #82: (cloned #21)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_QUA, COLOR0, EB, P2, P3, ED, 
  ST_QUA, COLOR1, P4, EE, EF, P6, 
  ST_QUA, COLOR1, P4, P6, EG, EA, 
  ST_QUA, COLOR1, P4, EA, P1, EB, 
  ST_TRI, COLOR1, P4, EB, ED, 
 // Case #83: (cloned #23)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR0, EB, P2, P3, ED, 
  ST_QUA, COLOR1, EF, P6, P0, P1, 
  ST_QUA, COLOR1, P4, EE, EF, P1, 
  ST_QUA, COLOR1, EB, ED, P4, P1, 
 // Case #84: (cloned #21)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR0, EG, P0, P1, EB, 
  ST_QUA, COLOR1, P2, EC, ED, P4, 
  ST_QUA, COLOR1, P2, P4, EE, EF, 
  ST_QUA, COLOR1, P2, EF, P6, EG, 
  ST_TRI, COLOR1, P2, EG, EB, 
 // Case #85: (cloned #43)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR1, P6, P0, EA, EB, 
  ST_QUA, COLOR1, P6, EB, P2, EC, 
  ST_QUA, COLOR1, P6, EC, ED, P4, 
  ST_QUA, COLOR1, P6, P4, EE, EF, 
 // Case #86: (cloned #43)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_QUA, COLOR1, P1, P2, EC, ED, 
  ST_QUA, COLOR1, P1, ED, P4, EE, 
  ST_QUA, COLOR1, P1, EE, EF, P6, 
  ST_QUA, COLOR1, P1, P6, EG, EA, 
 // Case #87: (cloned #47)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR1, P6, P0, P1, P2, 
  ST_QUA, COLOR1, P6, P2, EC, ED, 
  ST_QUA, COLOR1, P6, ED, P4, EE, 
  ST_TRI, COLOR1, P6, EE, EF, 
 // Case #88: (cloned #11)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR0, EG, P0, P1, P2, 
  ST_TRI, COLOR0, EG, P2, EC, 
  ST_QUA, COLOR1, P3, P4, EE, EF, 
  ST_QUA, COLOR1, P3, EF, P6, EG, 
  ST_TRI, COLOR1, P3, EG, EC, 
 // Case #89: (cloned #27)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR0, EA, P1, P2, EC, 
  ST_QUA, COLOR1, P3, P4, EE, EF, 
  ST_QUA, COLOR1, P3, EF, P6, P0, 
  ST_QUA, COLOR1, P3, P0, EA, EC, 
 // Case #90: (cloned #43)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR1, P3, P4, EE, EF, 
  ST_QUA, COLOR1, P3, EF, P6, EG, 
  ST_QUA, COLOR1, P3, EG, EA, P1, 
  ST_QUA, COLOR1, P3, P1, EB, EC, 
 // Case #91: (cloned #55)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR1, P6, P0, P1, EB, 
  ST_QUA, COLOR1, P6, EB, EC, P3, 
  ST_QUA, COLOR1, P6, P3, P4, EE, 
  ST_TRI, COLOR1, P6, EE, EF, 
 // Case #92: (cloned #23)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR0, EG, P0, P1, EB, 
  ST_QUA, COLOR1, P2, P3, P4, EE, 
  ST_QUA, COLOR1, P2, EE, EF, P6, 
  ST_QUA, COLOR1, P2, P6, EG, EB, 
 // Case #93: (cloned #55)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR1, P2, P3, P4, EE, 
  ST_QUA, COLOR1, P2, EE, EF, P6, 
  ST_QUA, COLOR1, P2, P6, P0, EA, 
  ST_TRI, COLOR1, P2, EA, EB, 
 // Case #94: (cloned #47)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_QUA, COLOR1, P1, P2, P3, P4, 
  ST_QUA, COLOR1, P1, P4, EE, EF, 
  ST_QUA, COLOR1, P1, EF, P6, EG, 
  ST_TRI, COLOR1, P1, EG, EA, 
 // Case #95: (cloned #63)
  ST_TRI, COLOR0, EE, P5, EF, 
  ST_QUA, COLOR1, P6, P0, P1, P2, 
  ST_QUA, COLOR1, P6, P2, P3, P4, 
  ST_QUA, COLOR1, P6, P4, EE, EF, 
 // Case #96: (cloned #3)
  ST_QUA, COLOR0, EG, P0, P1, P2, 
  ST_QUA, COLOR0, EG, P2, P3, P4, 
  ST_TRI, COLOR0, EG, P4, EE, 
  ST_QUA, COLOR1, P5, P6, EG, EE, 
 // Case #97: (cloned #7)
  ST_QUA, COLOR0, EA, P1, P2, P3, 
  ST_QUA, COLOR0, EA, P3, P4, EE, 
  ST_QUA, COLOR1, P5, P6, P0, EA, 
  ST_TRI, COLOR1, P5, EA, EE, 
 // Case #98: (cloned #11)
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_QUA, COLOR0, EB, P2, P3, P4, 
  ST_TRI, COLOR0, EB, P4, EE, 
  ST_QUA, COLOR1, P5, P6, EG, EA, 
  ST_QUA, COLOR1, P5, EA, P1, EB, 
  ST_TRI, COLOR1, P5, EB, EE, 
 // Case #99: (cloned #15)
  ST_QUA, COLOR0, EB, P2, P3, P4, 
  ST_TRI, COLOR0, EB, P4, EE, 
  ST_QUA, COLOR1, P5, P6, P0, P1, 
  ST_QUA, COLOR1, P5, P1, EB, EE, 
 // Case #100: (cloned #19)
  ST_QUA, COLOR0, EG, P0, P1, EB, 
  ST_QUA, COLOR0, EC, P3, P4, EE, 
  ST_QUA, COLOR1, P5, P6, EG, EB, 
  ST_QUA, COLOR1, P5, EB, P2, EC, 
  ST_TRI, COLOR1, P5, EC, EE, 
 // Case #101: (cloned #23)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR0, EC, P3, P4, EE, 
  ST_QUA, COLOR1, P5, P6, P0, EA, 
  ST_QUA, COLOR1, P5, EA, EB, P2, 
  ST_QUA, COLOR1, P5, P2, EC, EE, 
 // Case #102: (cloned #27)
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_QUA, COLOR0, EC, P3, P4, EE, 
  ST_QUA, COLOR1, P5, P6, EG, EA, 
  ST_QUA, COLOR1, P5, EA, P1, P2, 
  ST_QUA, COLOR1, P5, P2, EC, EE, 
 // Case #103: (cloned #31)
  ST_QUA, COLOR0, EC, P3, P4, EE, 
  ST_QUA, COLOR1, P5, P6, P0, P1, 
  ST_QUA, COLOR1, P5, P1, P2, EC, 
  ST_TRI, COLOR1, P5, EC, EE, 
 // Case #104: (cloned #11)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR0, P0, P1, P2, EC, 
  ST_TRI, COLOR0, EG, P0, EC, 
  ST_QUA, COLOR1, ED, EE, P5, P6, 
  ST_QUA, COLOR1, EC, P3, ED, P6, 
  ST_TRI, COLOR1, EG, EC, P6, 
 // Case #105: (cloned #23)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR0, EA, P1, P2, EC, 
  ST_QUA, COLOR1, EE, P5, P6, P0, 
  ST_QUA, COLOR1, P3, ED, EE, P0, 
  ST_QUA, COLOR1, EA, EC, P3, P0, 
 // Case #106: (cloned #43)
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR1, P5, P6, EG, EA, 
  ST_QUA, COLOR1, P5, EA, P1, EB, 
  ST_QUA, COLOR1, P5, EB, EC, P3, 
  ST_QUA, COLOR1, P5, P3, ED, EE, 
 // Case #107: (cloned #47)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR1, P5, P6, P0, P1, 
  ST_QUA, COLOR1, P5, P1, EB, EC, 
  ST_QUA, COLOR1, P5, EC, P3, ED, 
  ST_TRI, COLOR1, P5, ED, EE, 
 // Case #108: (cloned #27)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR0, EG, P0, P1, EB, 
  ST_QUA, COLOR1, P2, P3, ED, EE, 
  ST_QUA, COLOR1, P2, EE, P5, P6, 
  ST_QUA, COLOR1, P2, P6, EG, EB, 
 // Case #109: (cloned #55)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR1, P5, P6, P0, EA, 
  ST_QUA, COLOR1, P5, EA, EB, P2, 
  ST_QUA, COLOR1, P5, P2, P3, ED, 
  ST_TRI, COLOR1, P5, ED, EE, 
 // Case #110: (cloned #55)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_QUA, COLOR1, P1, P2, P3, ED, 
  ST_QUA, COLOR1, P1, ED, EE, P5, 
  ST_QUA, COLOR1, P1, P5, P6, EG, 
  ST_TRI, COLOR1, P1, EG, EA, 
 // Case #111: (cloned #63)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR1, P5, P6, P0, P1, 
  ST_QUA, COLOR1, P5, P1, P2, P3, 
  ST_QUA, COLOR1, P5, P3, ED, EE, 
 // Case #112: (cloned #7)
  ST_QUA, COLOR0, EG, P0, P1, P2, 
  ST_QUA, COLOR0, EG, P2, P3, ED, 
  ST_QUA, COLOR1, P4, P5, P6, EG, 
  ST_TRI, COLOR1, P4, EG, ED, 
 // Case #113: (cloned #15)
  ST_QUA, COLOR0, EA, P1, P2, P3, 
  ST_TRI, COLOR0, EA, P3, ED, 
  ST_QUA, COLOR1, P4, P5, P6, P0, 
  ST_QUA, COLOR1, P4, P0, EA, ED, 
 // Case #114: (cloned #23)
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_QUA, COLOR0, EB, P2, P3, ED, 
  ST_QUA, COLOR1, P4, P5, P6, EG, 
  ST_QUA, COLOR1, P4, EG, EA, P1, 
  ST_QUA, COLOR1, P4, P1, EB, ED, 
 // Case #115: (cloned #31)
  ST_QUA, COLOR0, EB, P2, P3, ED, 
  ST_QUA, COLOR1, P4, P5, P6, P0, 
  ST_QUA, COLOR1, P4, P0, P1, EB, 
  ST_TRI, COLOR1, P4, EB, ED, 
 // Case #116: (cloned #23)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR0, EG, P0, P1, EB, 
  ST_QUA, COLOR1, ED, P4, P5, P6, 
  ST_QUA, COLOR1, P2, EC, ED, P6, 
  ST_QUA, COLOR1, EG, EB, P2, P6, 
 // Case #117: (cloned #47)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR1, P4, P5, P6, P0, 
  ST_QUA, COLOR1, P4, P0, EA, EB, 
  ST_QUA, COLOR1, P4, EB, P2, EC, 
  ST_TRI, COLOR1, P4, EC, ED, 
 // Case #118: (cloned #55)
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR1, P4, P5, P6, EG, 
  ST_QUA, COLOR1, P4, EG, EA, P1, 
  ST_QUA, COLOR1, P4, P1, P2, EC, 
  ST_TRI, COLOR1, P4, EC, ED, 
 // Case #119: (cloned #63)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR1, P4, P5, P6, P0, 
  ST_QUA, COLOR1, P4, P0, P1, P2, 
  ST_QUA, COLOR1, P4, P2, EC, ED, 
 // Case #120: (cloned #15)
  ST_QUA, COLOR0, EG, P0, P1, P2, 
  ST_TRI, COLOR0, EG, P2, EC, 
  ST_QUA, COLOR1, P3, P4, P5, P6, 
  ST_QUA, COLOR1, P3, P6, EG, EC, 
 // Case #121: (cloned #31)
  ST_QUA, COLOR0, EA, P1, P2, EC, 
  ST_QUA, COLOR1, P3, P4, P5, P6, 
  ST_QUA, COLOR1, P3, P6, P0, EA, 
  ST_TRI, COLOR1, P3, EA, EC, 
 // Case #122: (cloned #47)
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR1, P3, P4, P5, P6, 
  ST_QUA, COLOR1, P3, P6, EG, EA, 
  ST_QUA, COLOR1, P3, EA, P1, EB, 
  ST_TRI, COLOR1, P3, EB, EC, 
 // Case #123: (cloned #63)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR1, P3, P4, P5, P6, 
  ST_QUA, COLOR1, P3, P6, P0, P1, 
  ST_QUA, COLOR1, P3, P1, EB, EC, 
 // Case #124: (cloned #31)
  ST_QUA, COLOR0, EG, P0, P1, EB, 
  ST_QUA, COLOR1, P2, P3, P4, P5, 
  ST_QUA, COLOR1, P2, P5, P6, EG, 
  ST_TRI, COLOR1, P2, EG, EB, 
 // Case #125: (cloned #63)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR1, P2, P3, P4, P5, 
  ST_QUA, COLOR1, P2, P5, P6, P0, 
  ST_QUA, COLOR1, P2, P0, EA, EB, 
 // Case #126: (cloned #63)
  ST_TRI, COLOR0, EG, P0, EA, 
  ST_QUA, COLOR1, P1, P2, P3, P4, 
  ST_QUA, COLOR1, P1, P4, P5, P6, 
  ST_QUA, COLOR1, P1, P6, EG, EA, 
 // Case #127: Unique case #18
  ST_QUA, COLOR1, P0, P1, P2, P3, 
  ST_QUA, COLOR1, P0, P3, P4, P5, 
  ST_TRI, COLOR1, P0, P5, P6, 
 // Dummy
  0
};

