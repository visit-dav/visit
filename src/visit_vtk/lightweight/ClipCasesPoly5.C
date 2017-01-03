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

int numClipCasesPoly5 = 32;

int numClipShapesPoly5[32] = {
  2,  3,  3,  3,  3,  4,  3,  3, // cases 0 - 7
  3,  4,  4,  5,  3,  5,  3,  3, // cases 8 - 15
  3,  3,  4,  3,  4,  5,  5,  3, // cases 16 - 23
  3,  3,  5,  3,  3,  3,  3,  2  // cases 24 - 31
};

int startClipShapesPoly5[32] = {
  0, 11, 28, 45, 62, 79, 102, 119, // cases 0 - 7
  136, 153, 176, 199, 226, 243, 270, 287, // cases 8 - 15
  304, 321, 338, 361, 378, 401, 428, 455, // cases 16 - 23
  472, 489, 506, 533, 550, 567, 584, 601  // cases 24 - 31
};

unsigned char clipShapesPoly5[] = {
 // Case #0: Unique case #1
  ST_QUA, COLOR0, P0, P1, P2, P3, 
  ST_TRI, COLOR0, P0, P3, P4, 
 // Case #1: Unique case #2
  ST_QUA, COLOR0, EA, P1, P2, P3, 
  ST_QUA, COLOR0, EA, P3, P4, EE, 
  ST_TRI, COLOR1, P0, EA, EE, 
 // Case #2: (cloned #1)
  ST_QUA, COLOR0, EB, P2, P3, P4, 
  ST_QUA, COLOR0, EB, P4, P0, EA, 
  ST_TRI, COLOR1, P1, EB, EA, 
 // Case #3: Unique case #3
  ST_QUA, COLOR0, EB, P2, P3, P4, 
  ST_TRI, COLOR0, EB, P4, EE, 
  ST_QUA, COLOR1, P0, P1, EB, EE, 
 // Case #4: (cloned #1)
  ST_QUA, COLOR0, EC, P3, P4, P0, 
  ST_QUA, COLOR0, EC, P0, P1, EB, 
  ST_TRI, COLOR1, P2, EC, EB, 
 // Case #5: Unique case #4
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR0, EC, P3, P4, EE, 
  ST_QUA, COLOR1, P0, EA, EB, P2, 
  ST_QUA, COLOR1, P0, P2, EC, EE, 
 // Case #6: (cloned #3)
  ST_QUA, COLOR0, EC, P3, P4, P0, 
  ST_TRI, COLOR0, EC, P0, EA, 
  ST_QUA, COLOR1, P1, P2, EC, EA, 
 // Case #7: Unique case #5
  ST_QUA, COLOR0, EC, P3, P4, EE, 
  ST_QUA, COLOR1, P0, P1, P2, EC, 
  ST_TRI, COLOR1, EC, EE, P0, 
 // Case #8: (cloned #1)
  ST_QUA, COLOR0, ED, P4, P0, P1, 
  ST_QUA, COLOR0, ED, P1, P2, EC, 
  ST_TRI, COLOR1, P3, ED, EC, 
 // Case #9: (cloned #5)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR0, EA, P1, P2, EC, 
  ST_QUA, COLOR1, P3, ED, EE, P0, 
  ST_QUA, COLOR1, P3, P0, EA, EC, 
 // Case #10: (cloned #5)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR0, ED, P4, P0, EA, 
  ST_QUA, COLOR1, P1, EB, EC, P3, 
  ST_QUA, COLOR1, P1, P3, ED, EA, 
 // Case #11: Unique case #6
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR1, P0, P1, EB, EC, 
  ST_QUA, COLOR1, P0, EC, P3, ED, 
  ST_TRI, COLOR1, P0, ED, EE, 
 // Case #12: (cloned #3)
  ST_QUA, COLOR0, ED, P4, P0, P1, 
  ST_TRI, COLOR0, ED, P1, EB, 
  ST_QUA, COLOR1, P2, P3, ED, EB, 
 // Case #13: (cloned #11)
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR1, P2, P3, ED, EE, 
  ST_QUA, COLOR1, P2, EE, P0, EA, 
  ST_TRI, COLOR1, P2, EA, EB, 
 // Case #14: (cloned #7)
  ST_QUA, COLOR0, ED, P4, P0, EA, 
  ST_QUA, COLOR1, P1, P2, P3, ED, 
  ST_TRI, COLOR1, ED, EA, P1, 
 // Case #15: Unique case #7
  ST_TRI, COLOR0, ED, P4, EE, 
  ST_QUA, COLOR1, P0, P1, P2, P3, 
  ST_QUA, COLOR1, P0, P3, ED, EE, 
 // Case #16: (cloned #1)
  ST_QUA, COLOR0, EE, P0, P1, P2, 
  ST_QUA, COLOR0, EE, P2, P3, ED, 
  ST_TRI, COLOR1, P4, EE, ED, 
 // Case #17: (cloned #3)
  ST_QUA, COLOR0, EA, P1, P2, P3, 
  ST_TRI, COLOR0, EA, P3, ED, 
  ST_QUA, COLOR1, P4, P0, EA, ED, 
 // Case #18: (cloned #5)
  ST_TRI, COLOR0, EE, P0, EA, 
  ST_QUA, COLOR0, EB, P2, P3, ED, 
  ST_QUA, COLOR1, P4, EE, EA, P1, 
  ST_QUA, COLOR1, P4, P1, EB, ED, 
 // Case #19: (cloned #7)
  ST_QUA, COLOR0, EB, P2, P3, ED, 
  ST_QUA, COLOR1, P4, P0, P1, EB, 
  ST_TRI, COLOR1, EB, ED, P4, 
 // Case #20: (cloned #5)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR0, EE, P0, P1, EB, 
  ST_QUA, COLOR1, P2, EC, ED, P4, 
  ST_QUA, COLOR1, P2, P4, EE, EB, 
 // Case #21: (cloned #11)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR1, P4, P0, EA, EB, 
  ST_QUA, COLOR1, P4, EB, P2, EC, 
  ST_TRI, COLOR1, P4, EC, ED, 
 // Case #22: (cloned #11)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_TRI, COLOR0, EE, P0, EA, 
  ST_QUA, COLOR1, P1, P2, EC, ED, 
  ST_QUA, COLOR1, P1, ED, P4, EE, 
  ST_TRI, COLOR1, P1, EE, EA, 
 // Case #23: (cloned #15)
  ST_TRI, COLOR0, EC, P3, ED, 
  ST_QUA, COLOR1, P4, P0, P1, P2, 
  ST_QUA, COLOR1, P4, P2, EC, ED, 
 // Case #24: (cloned #3)
  ST_QUA, COLOR0, EE, P0, P1, P2, 
  ST_TRI, COLOR0, EE, P2, EC, 
  ST_QUA, COLOR1, P3, P4, EE, EC, 
 // Case #25: (cloned #7)
  ST_QUA, COLOR0, EA, P1, P2, EC, 
  ST_QUA, COLOR1, P3, P4, P0, EA, 
  ST_TRI, COLOR1, EA, EC, P3, 
 // Case #26: (cloned #11)
  ST_TRI, COLOR0, EE, P0, EA, 
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR1, P3, P4, EE, EA, 
  ST_QUA, COLOR1, P3, EA, P1, EB, 
  ST_TRI, COLOR1, P3, EB, EC, 
 // Case #27: (cloned #15)
  ST_TRI, COLOR0, EB, P2, EC, 
  ST_QUA, COLOR1, P3, P4, P0, P1, 
  ST_QUA, COLOR1, P3, P1, EB, EC, 
 // Case #28: (cloned #7)
  ST_QUA, COLOR0, EE, P0, P1, EB, 
  ST_QUA, COLOR1, P2, P3, P4, EE, 
  ST_TRI, COLOR1, EE, EB, P2, 
 // Case #29: (cloned #15)
  ST_TRI, COLOR0, EA, P1, EB, 
  ST_QUA, COLOR1, P2, P3, P4, P0, 
  ST_QUA, COLOR1, P2, P0, EA, EB, 
 // Case #30: (cloned #15)
  ST_TRI, COLOR0, EE, P0, EA, 
  ST_QUA, COLOR1, P1, P2, P3, P4, 
  ST_QUA, COLOR1, P1, P4, EE, EA, 
 // Case #31: Unique case #8
  ST_QUA, COLOR1, P0, P1, P2, P3, 
  ST_TRI, COLOR1, P0, P3, P4, 
 // Dummy
  0
};

