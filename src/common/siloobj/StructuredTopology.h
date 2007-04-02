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

// ************************************************************************* //
//                          StructuredTopology.h                             //
// ************************************************************************* //

#ifndef STRUCTURED_TOPOLOGY_H
#define STRUCTURED_TOPOLOGY_H
#include <siloobj_exports.h>

#include <visitstream.h>

#include <silo.h>


// ****************************************************************************
//  Class: StructuredTopology
//
//  Purpose:
//      Keeps information about a structured topology.
//
//  Data Members:
//      nDimensions  -  The number of dimensions.
//      dimensions   -  The dimensions of the structured blocks.  This is an
//                      array of size nDimensions*nDomains*2.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

class SILOOBJ_API StructuredTopology
{
  public:
                    StructuredTopology();
    virtual        ~StructuredTopology();
   
    char           *GetName()  { return name; };

    void            PrintSelf(ostream &);
    void            Read(DBobject *, DBfile *);
    virtual void    Write(DBfile *);
   
  protected:
    int             nDimensions;
    float          *dimensions;

    int             nDomains;
    char           *name;

    // Class-scoped constants
  public:
    static char * const    NAME;
    static char * const    SILO_TYPE;
  protected:
    static char * const    SILO_NAME;
    static int    const    SILO_NUM_COMPONENTS;
    static char * const    SILO_N_DIMENSIONS_NAME;
    static char * const    SILO_DIMENSIONS_NAME;
};


#endif


