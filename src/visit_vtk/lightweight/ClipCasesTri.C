/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

// Programmer: Jeremy Meredith
// Date      : September 18, 2003

// This file is meant to be read and created by a program other than a
// compiler.  If you must modify it by hand, at least be nice to the 
// parser and don't add anything else to this file or rearrange it.

int numClipCasesTri = 8;

int numClipShapesTri[8] = {
  1,  2,  2,  2,  2,  2,  2,  1  // cases 0 - 7
};

int startClipShapesTri[8] = {
  0, 5, 16, 27, 38, 49, 60, 71  // cases 0 - 7
};

unsigned char clipShapesTri[] = {
 // Case #0: Unique case #1
  ST_TRI, COLOR0, P0, P1, P2, 
 // Case #1: Unique case #2
  ST_QUA, COLOR0, P1, P2, EC, EA, 
  ST_TRI, COLOR1, P0, EA, EC, 
 // Case #2: (cloned #1)
  ST_QUA, COLOR0, P2, P0, EA, EB, 
  ST_TRI, COLOR1, P1, EB, EA, 
 // Case #3: Unique case #3
  ST_TRI, COLOR0, EC, EB, P2, 
  ST_QUA, COLOR1, P0, P1, EB, EC, 
 // Case #4: (cloned #1)
  ST_QUA, COLOR0, P0, P1, EB, EC, 
  ST_TRI, COLOR1, P2, EC, EB, 
 // Case #5: (cloned #3)
  ST_TRI, COLOR0, EB, EA, P1, 
  ST_QUA, COLOR1, P2, P0, EA, EB, 
 // Case #6: (cloned #3)
  ST_TRI, COLOR0, EA, EC, P0, 
  ST_QUA, COLOR1, P1, P2, EC, EA, 
 // Case #7: Unique case #4
  ST_TRI, COLOR1, P0, P1, P2, 
 // Dummy
  0
};

