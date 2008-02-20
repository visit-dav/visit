/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                           avtSourceFromDatabase.h                         //
// ************************************************************************* //

#ifndef AVT_SOURCE_FROM_DATABASE_H
#define AVT_SOURCE_FROM_DATABASE_H

#include <database_exports.h>

#include <avtOriginatingDatasetSource.h>
#include <avtDataTree.h>


class  vtkDataSet;

class  avtDatasetDatabase;
class  PickAttributes;


// ****************************************************************************
//  Class: avtSourceFromDatabase
//
//  Purpose:
//      A source object (pipeline terminator) that is created from an avt
//      database.
//
//  Programmer: Hank Childs
//  Creation:   August 9, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Tue Sep 26 09:42:26 PDT 2000
//    Made FetchDomain return an avtDomain.
//
//    Kathleen Bonnell, Tue Dec 19 09:06:23 PST 2000 
//    Renamed avtOriginatingSource as avtOriginatingDatasetSource. 
//
//    Kathleen Bonnell, Fri Feb  9 16:38:40 PST 2001 
//    Made FetchDomain return avtDomainTree_p. 
//
//    Hank Childs, Thu Mar  1 11:20:47 PST 2001
//    Modified routine to return all of the domain at once instead of one at
//    a time.
//
//    Hank Childs, Tue Apr 10 16:13:07 PDT 2001
//    Added GetMaterialAuxiliaryData.
//
//    Kathleen Bonnell, Wed Apr 11 08:44:46 PDT 2001 
//    Changed return type of FetchDomains to be avtDataTree_p. 
//
//    Hank Childs, Tue Jun 19 21:54:53 PDT 2001
//    Overhauled interface to reflect changes in the base class.
//
//    Hank Childs, Thu Oct 25 16:49:26 PDT 2001
//    Allow for databases that don't do dynamic load balancing.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002 
//    Add Query method. 
//
//    Kathleen Bonnell, Thu Nov 13 08:44:01 PST 2003 
//    Add FindElementForPoint method. 
//
//    Kathleen Bonnell, Mon Dec 22 14:39:30 PST 2003
//    Add GetDomainName method. 
//
//    Kathleen Bonnell, Tue May 25 16:16:25 PDT 2004 
//    Add QueryZoneCenter method. 
//
//    Jeremy Meredith, Wed Jun  9 09:12:24 PDT 2004
//    Added species aux data.
//
//    Kathleen Bonnell, Thu Jun 10 18:15:11 PDT 2004 
//    Rename QueryZoneCenter to QueryCoords, added bool arg.
//
//    Kathleen Bonnell, Thu Dec 16 17:11:19 PST 2004 
//    Added another bool arg to QueryCoords. 
//
//    Kathleen Bonnell, Mon Jan  3 13:26:20 PST 2005 
//    Add method 'GetSIL'. 
//
//    Kathleen Bonnell, Tue Jan 25 07:59:28 PST 2005 
//    Added const char* arg to QueryCoords.
//
//    Hank Childs, Sun Feb 27 12:44:40 PST 2005
//    Removed method CanDoDynamicLoadBalancing.  Database is now asked
//    directly.  Also added new ReleaseData method that is aware of DLB.
//
// ****************************************************************************

class DATABASE_API avtSourceFromDatabase : public avtOriginatingDatasetSource
{
  public:
                        avtSourceFromDatabase(avtDatasetDatabase *, 
                                              const char *, int);
    virtual            ~avtSourceFromDatabase();

    virtual void        FetchMeshAuxiliaryData(const char *type, void *args,
                                        avtDataRequest_p, VoidRefList &);
    virtual void        FetchVariableAuxiliaryData(const char *type,
                            void *args, avtDataRequest_p, VoidRefList &);
    virtual void        FetchMaterialAuxiliaryData(const char *type, 
                            void *args, avtDataRequest_p, VoidRefList &);
    virtual void        FetchSpeciesAuxiliaryData(const char *type, 
                            void *args, avtDataRequest_p, VoidRefList &);

    avtSIL             *GetSIL(int stateIndex);

    virtual avtDataRequest_p
                        GetFullDataRequest(void);

    void                InvalidateZones(void);
    void                DatabaseProgress(int, int, const char *);
    virtual void        Query(PickAttributes *);
    virtual bool        QueryCoords(const std::string &, const int, const int,
                                    const int, double[3], const bool, const bool, const char *mn = NULL);
    virtual bool        FindElementForPoint(const char *, const int, 
                            const int, const char *, double[3], int &);
    virtual void        GetDomainName(const std::string &, const int, 
                            const int, std::string &);

    virtual void        ReleaseData(void);

  protected:
    avtDatasetDatabase      *database;
    char                    *variable;
    int                      timestep;
    avtDataRequest_p   lastSpec;

    virtual bool        FetchDataset(avtDataRequest_p, avtDataTree_p &);
    virtual int         NumStagesForFetch(avtDataRequest_p);
};


#endif


