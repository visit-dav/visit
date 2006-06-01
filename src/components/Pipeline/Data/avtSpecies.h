/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                               avtSpecies.h                                //
// ************************************************************************* //

#ifndef AVT_SPECIES_H
#define AVT_SPECIES_H

#include <pipeline_exports.h>

#include <string>
#include <vector>

#include <void_ref_ptr.h>

class avtMaterial;

// ****************************************************************************
//  Class:  CellSpecInfo
//
//  Purpose:
//    Holds species info for a cell in response to a query.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 19, 2003
//
// ****************************************************************************
struct CellSpecInfo
{
    std::string name;
    float       mf;
    CellSpecInfo(std::string n="", float m=0.) : name(n), mf(m) { }
};

// ****************************************************************************
//  Class: avtSpecies
//
//  Purpose:
//      Holds a species.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
//  Modifications:
//
// ****************************************************************************

class PIPELINE_API avtSpecies
{
  public:
                                     avtSpecies(int, const int*,
                                                int, const int*,
                                                const int*, int, const int*,
                                                int, const float *);
                                     avtSpecies(const std::vector<int>,
                                                const std::vector<
                                                  std::vector<std::string> > &,
                                                int, const int*, int,
                                                const int*,int, const float *);
    virtual                         ~avtSpecies();

    static void                      Destruct(void *);

    int                              GetNZones(void)      { return nZones; };
    const std::vector<int>          &GetNSpecies(void)    { return nSpecies; };
    const std::vector<std::vector<std::string> > &
                                     GetSpecies(void)     { return species; };
    const int                       *GetSpeclist(void)    { return speclist; };
    int                              GetMixlen(void)      { return mixlen; };
    const int                       *GetMixSpeclist(void) 
                                                      { return mix_speclist; };
    int                              GetNSpecMF(void)  { return nspecies_mf; };
    const float                     *GetSpecMF(void)   { return species_mf; };
    int                              GetNMat(void) { return nSpecies.size(); };
    
    std::vector<CellSpecInfo>        ExtractCellSpecInfo(int c, int m,
                                                         avtMaterial *);

  protected:
    std::vector<int>                         nSpecies;
    std::vector<std::vector<std::string> >   species;
    int                                      nZones;
    int                                     *speclist;
    int                                      mixlen;
    int                                     *mix_speclist;
    int                                      nspecies_mf;
    float                                   *species_mf;

    void             Initialize(const std::vector<int>,
                                const std::vector<std::vector<std::string> > &,
                                int, const int*, int, const int*, int,
                                const float *);
};


// ****************************************************************************
//  Class: avtMultiSpecies
//
//  Purpose:
//      Contatins species for many domains.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
// ****************************************************************************

class PIPELINE_API avtMultiSpecies
{
  public:
                                avtMultiSpecies(int);
    virtual                    ~avtMultiSpecies();

    void                        SetDomain(avtSpecies *, int);
    avtSpecies                 *GetDomain(int);

  protected:
    avtSpecies                **species;
    int                         numDomains;
};


#endif


