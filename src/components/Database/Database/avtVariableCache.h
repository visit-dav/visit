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
//                             avtVariableCache.h                            //
// ************************************************************************* //

#ifndef AVT_VARIABLE_CACHE_H
#define AVT_VARIABLE_CACHE_H

#include <database_exports.h>

#include <visitstream.h>
#include <map>
#include <vector>

#include <void_ref_ptr.h>

class   avtCachableItem;
class   vtkObject;


// ****************************************************************************
//  Class: avtVariableCache
//
//  Purpose:
//      Stores vtkDataSets so they can be retrieved without re-reading them 
//      from a file.  This is easily done because vtkDataSets are reference
//      counted, so issues of who owns which dataset are not relevant.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2000
//
//  Modifications:
//
//    Hank Childs, Tue May 22 16:14:58 PDT 2001
//    Added caching of arbitrary auxiliary data through VoidRef.  Also made
//    caching of VTK objects more arbitrary.
//
//    Hank Childs, Fri Oct  5 17:19:17 PDT 2001
//    Added caching of which material a variable is on.
//
//    Hank Childs, Mon Jan  7 17:24:27 PST 2002
//    Add ClearTimestep.
//
//    Jeremy Meredith, Thu Oct 24 13:21:15 PDT 2002
//    Added HasVoidRef to check if a void ref exists.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Brad Whitlock, Sat Apr 2 00:50:39 PDT 2005
//    Added label support.
//
//    Mark C. Miller, Tue Apr  5 10:30:16 PDT 2005
//    Added native cache
//
//    Hank Childs, Tue Jul 19 15:49:08 PDT 2005
//    Add support for arrays.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Removed 'NATIVE' cache tags, added DATA_SPECIFICATION tag. Added methods
//    to support getting object's keys from cache given the object's pointer
//    Added a map of pointer pairs to deal with cases where generic db returns
//    copies of objects in the cache. Added itemTypes to CacheableItem to
//    impl. single GetItem method for both classes (maybe should have
//    implemented separately for each cacheable item type)
//
// ****************************************************************************

class DATABASE_API avtVariableCache
{
  public:
                           avtVariableCache();
    virtual               ~avtVariableCache();

    static const char     *SCALARS_NAME;
    static const char     *VECTORS_NAME;
    static const char     *TENSORS_NAME;
    static const char     *SYMMETRIC_TENSORS_NAME;
    static const char     *LABELS_NAME;
    static const char     *ARRAYS_NAME;
    static const char     *DATASET_NAME;
    static const char     *DATA_SPECIFICATION;

    vtkObject             *GetVTKObject(const char *name, const char *type,
                                        int ts, int domain, const char *mat);
    void                   CacheVTKObject(const char *name, const char *type,
                                          int ts, int domain, const char *mat,
                                          vtkObject *);

    // given a VTK object pointer, find that object in the cache and return
    // the "key" (name, type, ts, domain, mat) where it is stored in the cache 
    bool                   GetVTKObjectKey(const char **name, const char **type,
                               int *ts, int *dom, const char **mat,
                               vtkObject *obj) const;

    bool                   HasVoidRef(const char *name, const char *type,
                                      int ts, int domain);
    void_ref_ptr           GetVoidRef(const char *name, const char *type,
                                      int ts, int domain);
    void                   CacheVoidRef(const char *name, const char *type,
                                        int ts, int domain, void_ref_ptr);

    bool                   GetVoidRefKey(const char **name, const char **type,
                                         int *ts, int *domain, void_ref_ptr vrp) const;

    void                   ClearTimestep(int);

    void                   Print(ostream &);

    // Special destructor function to permit us to spoof VTK objects
    // in the VoidRef cache so that the Auxiliary data interface can 
    // return vtkDataArrays (e.g. global node/zone ids)
    static void            DestructVTKObject(void *vtkObj);

    // functions to help transform manager find items in cache
    void                   AddObjectPointerPair(vtkObject *o1,
                                                vtkObject *o2);
    bool                   RemoveObjectPointerPair(vtkObject *o1);
    vtkObject             *FindObjectPointerPair(vtkObject *o1) const;

  protected:

    class OneDomain
    {
      public:
                          OneDomain(int);
        virtual          ~OneDomain();
    
        void              CacheItem(avtCachableItem *);
        int               GetDomain(void) const  { return domain; };
        avtCachableItem  *GetItem(void)     { return item; };
        bool              GetItem(int *dom, avtCachableItem *) const;
    
        void              Print(ostream &, int);

      protected:
        int               domain;
        avtCachableItem  *item;
    };

    class OneTimestep
    {
      public:
                                    OneTimestep(int);
        virtual                    ~OneTimestep();
        
        void                        CacheItem(int, avtCachableItem *);
        avtCachableItem            *GetItem(int);
        bool                        GetItem(int *ts, int *dom, avtCachableItem *) const;
        int                         GetTimestep(void) const { return timestep; };
    
        void                        Print(ostream &, int);

      protected:
        int                         timestep;
        std::vector<OneDomain *>    domains;
    };

    class OneMat
    {
      public:
                                    OneMat(const char *);
        virtual                    ~OneMat();

        void                        CacheItem(int, int, avtCachableItem *);
        avtCachableItem            *GetItem(int, int);
        const char                 *GetMaterial(void) const { return material; };
        bool                        GetItem(int *ts, int *dom,
                                        const char **mat, avtCachableItem *) const;
        void                        ClearTimestep(int);
    
        void                        Print(ostream &, int);

      protected:
        char                       *material;
        std::vector<OneTimestep *>  timesteps;
    };

    class OneVar
    {
      public:
                                     OneVar(const char *var, const char *type);
        virtual                     ~OneVar();
    
        void                         CacheItem(const char *, int, int,
                                               avtCachableItem *);
        avtCachableItem             *GetItem(const char *, int, int);
        const char                  *GetVar(void) const { return var; };
        const char                  *GetType(void) const { return type; };
        bool                         GetItem(const char **name, const char **_type,
                                         int *ts, int *dom, const char **mat,
                                         avtCachableItem *) const;
        void                         ClearTimestep(int);
    
        void                         Print(ostream &, int);

      protected:
        char                        *var;
        char                        *type;
        std::vector<OneMat *>        materials;
    };

    std::vector<OneVar *>            vtkVars;
    std::vector<OneVar *>            voidRefVars;

    std::map<vtkObject*, vtkObject*> objectPointerMap;
};


class DATABASE_API  avtCachableItem
{
  public:

   typedef enum {
       VTKObject,
       VoidRef
   } CachableItemType;
                          avtCachableItem();
   virtual               ~avtCachableItem();
   CachableItemType       GetItemType() const { return itemType; };

  protected:
    CachableItemType      itemType;
};


class DATABASE_API  avtCachedVTKObject : public avtCachableItem
{
  public:
                          avtCachedVTKObject(vtkObject *);
    virtual              ~avtCachedVTKObject();
    vtkObject            *GetVTKObject(void)  { return obj; };

  protected:
    vtkObject            *obj;
};


class DATABASE_API  avtCachedVoidRef : public avtCachableItem
{
  public:
                         avtCachedVoidRef(void_ref_ptr);
    virtual             ~avtCachedVoidRef();
    void_ref_ptr         GetVoidRef(void)  { return voidRef; };

  protected:
    void_ref_ptr         voidRef;
};


#endif


