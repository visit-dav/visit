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
//                              avtVariableCache.C                           //
// ************************************************************************* //

#include <vtkDataSet.h>

#include <avtFacelist.h>
#include <avtMaterial.h>
#include <avtVariableCache.h>

#include <ImproperUseException.h>

using std::string;
using std::vector;

inline void Indent(ostream &, int);


const char *avtVariableCache::SCALARS_NAME = "SCALARS";
const char *avtVariableCache::VECTORS_NAME = "VECTORS";
const char *avtVariableCache::TENSORS_NAME = "TENSORS";
const char *avtVariableCache::SYMMETRIC_TENSORS_NAME = "SYMMETRIC_TENSORS";
const char *avtVariableCache::LABELS_NAME = "LABELS";
const char *avtVariableCache::ARRAYS_NAME = "ARRAYS";
const char *avtVariableCache::DATASET_NAME = "DATASET";
const char *avtVariableCache::DATA_SPECIFICATION = "DATA_SPECIFICATION";

// ****************************************************************************
//  Method: avtVariableCache::DestructVTKObject 
//
//  Purpose:
//      Defines a destructor function meeting the requirements of
//      DestructorFunction for VTK objects.
//
//  Programmer: Mark C. Miller
//  Creation:   August 4, 2004 
//
// ****************************************************************************

void
avtVariableCache::DestructVTKObject(void *vtkObj)
{
    ((vtkObject*)vtkObj)->Delete();
}

// ****************************************************************************
//  Method: avtVariableCache constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtVariableCache::avtVariableCache()
{
    ;
}


// ****************************************************************************
//  Method: avtVariableCache destructor
//
//  Programmer: Hank Childs
//  Creation:   October 19, 2000
//
//  Modifications:
//
//    Hank Childs, Tue May 22 16:31:39 PDT 2001
//    Re-wrote for new class layout.
//
// ****************************************************************************

avtVariableCache::~avtVariableCache()
{
    std::vector<OneVar *>::iterator it;
    for (it = vtkVars.begin() ; it != vtkVars.end() ; it++)
    {
        delete *it;
    }
    for (it = voidRefVars.begin() ; it != voidRefVars.end() ; it++)
    {
        delete *it;
    }
}


// ****************************************************************************
//  Method: avtCachableItem constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtCachableItem::avtCachableItem()
{
    ;
}


// ****************************************************************************
//  Method: avtCachableItem destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtCachableItem::~avtCachableItem()
{
    ;
}


// ****************************************************************************
//  Method: avtVariableCache::GetVTKObject
//
//  Purpose:
//      Gets the vtk object specified by the arguments from the cache.
//
//  Arguments:
//      name      The name of the object.
//      type      The type of the object.
//      timestep  The timestep for this vtk object.
//      domain    The domain number (0-origin)
//      material  The material of the object.
//
//  Returns:      The vtk object corresponding to the parameters if it
//                exists, NULL otherwise.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Oct  5 17:31:30 PDT 2001
//    Added argument material.
//
// ****************************************************************************

vtkObject *
avtVariableCache::GetVTKObject(const char *var, const char *type, int timestep,
                               int domain, const char *material)
{
    avtCachableItem *item = NULL;
    std::vector<OneVar *>::iterator it;
    for (it = vtkVars.begin() ; it != vtkVars.end() ; it++)
    {
        if (strcmp((*it)->GetVar(),  var)  == 0 &&
            strcmp((*it)->GetType(), type) == 0)
        {
            item = (*it)->GetItem(material, timestep, domain);
            if (item != NULL)
            {
                avtCachedVTKObject *vo = (avtCachedVTKObject *) item;
                return vo->GetVTKObject();
            }
        }
    }

    //
    // We couldn't find the VTK object.
    //
    return NULL;
}


// ****************************************************************************
//  Method: avtVariableCache::CacheVTKObject
//
//  Purpose:
//      Stores the vtk object in the cache with the associated information.
//
//  Arguments:
//      name       The name of this VTK object.
//      type       The type of this VTK object.
//      domain     The domain for this VTK object.
//      timestep   The timestep this VTK object is from.
//      material   The material this VTK object is from.
//      obj        The actual VTK object.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2001
//
//  Modifications:
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added RemoveObjectPointerPair to ensure we don't have a pair with obj
//    already recorded
// ****************************************************************************

void
avtVariableCache::CacheVTKObject(const char *name, const char *type,
                                 int timestep, int domain,
                                 const char *material, vtkObject *obj)
{
    OneVar  *v = NULL;

    std::vector<OneVar *>::iterator it;
    for (it = vtkVars.begin() ; it != vtkVars.end() ; it++)
    {
        if (strcmp((*it)->GetVar(),  name) == 0 &&
            strcmp((*it)->GetType(), type) == 0)
        {
            v = *it;
            break;
        }
    }
    if (v == NULL)
    {
        v = new OneVar(name, type);
        vtkVars.push_back(v);
    }

    RemoveObjectPointerPair(obj);
    avtCachedVTKObject *cvo = new avtCachedVTKObject(obj);
    v->CacheItem(material, timestep, domain, cvo);
}

// ****************************************************************************
//  Method: avtVariableCache::OneDomain::GetItem
//
//  Purpose: Like GetItem, except only tests for existence of item's pointer
//  and, if found, returns the dom associated with the item, if requested.
//
//  Programmer: Mark C. Miller
//  Creation:   December 3, 2006 
// 
// ****************************************************************************
bool avtVariableCache::OneDomain::GetItem(int *dom,
    avtCachableItem *theItem) const
{
    if ((item->GetItemType() == theItem->GetItemType()) &&
        (item->GetItemType() == avtCachableItem::VTKObject))
    {
        avtCachedVTKObject *thisObj = (avtCachedVTKObject *) item;
        avtCachedVTKObject *theObj = (avtCachedVTKObject *) theItem;
        vtkObject *thisVTKObject = thisObj->GetVTKObject();
        vtkObject *theVTKObject = theObj->GetVTKObject();
        if (thisVTKObject->GetMTime() == theVTKObject->GetMTime())
        {
            if (dom) *dom = GetDomain();
            return true;
        }
    }
    else if ((item->GetItemType() == theItem->GetItemType()) &&
             (item->GetItemType() == avtCachableItem::VoidRef))
    {
        avtCachedVoidRef *thisRef = (avtCachedVoidRef *) item;
        avtCachedVoidRef *theRef = (avtCachedVoidRef *) theItem;
        if (theRef && theRef->GetVoidRef() == thisRef->GetVoidRef())
        {
            if (dom) *dom = GetDomain();
            return true;
        }
    }
    return false;
}

// ****************************************************************************
//  Method: avtVariableCache::OneTimestep::GetItem
//
//  Purpose: Like GetItem, except only tests for existence of item's pointer
//  and, if found, returns other information associated with the item if
//  requested.
//
//  Programmer: Mark C. Miller
//  Creation:   December 3, 2006 
// 
// ****************************************************************************
bool
avtVariableCache::OneTimestep::GetItem(int *ts, int *dom,
    avtCachableItem *theItem) const
{
    std::vector<OneDomain *>::const_iterator it;
    for (it = domains.begin() ; it != domains.end() ; it++)
    {
        if ((*it)->GetItem(dom, theItem))
        {
            if (ts) *ts = GetTimestep();
            return true;
        }
    }
    return false;
}

// ****************************************************************************
//  Method: avtVariableCache::OneMat::GetItem
//
//  Purpose: Like GetItem, except only tests for existence of item's pointer
//  and, if found, returns other information associated with the item if
//  requested.
//
//  Programmer: Mark C. Miller
//  Creation:   December 3, 2006 
// 
// ****************************************************************************
bool
avtVariableCache::OneMat::GetItem(int *ts, int *dom,
    const char **mat, avtCachableItem *theItem) const
{
    std::vector<OneTimestep *>::const_iterator it;
    for (it = timesteps.begin() ; it != timesteps.end() ; it++)
    {
        if ((*it)->GetItem(ts, dom, theItem))
        {
            if (mat) *mat = GetMaterial();
            return true;
        }
    }
    return false;
}

// ****************************************************************************
//  Method: avtVariableCache::OneVar::GetItem
//
//  Purpose: Like GetItem, except only tests for existence of item's pointer
//  and, if found, returns other information associated with the item if
//  requested.
//
//  Programmer: Mark C. Miller
//  Creation:   December 3, 2006 
// 
// ****************************************************************************
bool
avtVariableCache::OneVar::GetItem(
    const char **name, const char **_type,
    int *ts, int *dom, const char **mat,
    avtCachableItem *theItem) const
{
    std::vector<OneMat *>::const_iterator it;
    for (it = materials.begin() ; it != materials.end() ; it++)
    {
        if ((*it)->GetItem(ts, dom, mat, theItem))
        {
            if (name) *name = GetVar();
            if (_type) *_type = GetType();
            return true;
        }
    }
    return false;
}

// ****************************************************************************
//  Method: avtVariableCache::GetVTKObjectKey
//
//  Purpose: Given a vtkObject pointer, find it in the cache and return other
//  information associated with it. If the pointer is paired, use the paired
//  pointer to find it. Otherwise, use the originally given pointer.
//
//  Programmer: Mark C. Miller
//  Creation:   December 3, 2006 
// 
// ****************************************************************************
bool
avtVariableCache::GetVTKObjectKey(
    const char **name, const char **type,
    int *ts, int *dom, const char **mat,
    vtkObject *theObj) const
{
    vtkObject *tmpObj = FindObjectPointerPair(theObj);
    if (tmpObj) theObj = tmpObj;

    // theItem needs to live only for duration of this method
    avtCachedVTKObject theItem(theObj);
    std::vector<OneVar *>::const_iterator it;
    for (it = vtkVars.begin() ; it != vtkVars.end() ; it++)
    {
        if ((*it)->GetItem(name, type, ts, dom, mat, &theItem))
            return true;
    }
    return false;
}

// ****************************************************************************
//  Method: avtVariableCache::GetVoidRefKey
//
//  Purpose: Given a void_ref_ptr pointer, find it in the cache and return
//  other information associated with it. If the pointer is paired, use the
//  paired pointer to find it. Otherwise, use the originally given pointer.
//
//  Programmer: Mark C. Miller
//  Creation:   December 3, 2006 
// 
// ****************************************************************************
bool
avtVariableCache::GetVoidRefKey(
    const char **name, const char **type,
    int *ts, int *dom, void_ref_ptr vrp) const
{
    // theItem needs to live only for duration of this method
    avtCachedVoidRef theItem(vrp);
    std::vector<OneVar *>::const_iterator it;
    for (it = voidRefVars.begin() ; it != voidRefVars.end() ; it++)
    {
        if ((*it)->GetItem(name, type, ts, dom, 0, &theItem))
            return true;
    }
    return false;
}

// ****************************************************************************
//  Method: avtVariableCache::HasVoidRef
//
//  Purpose:
//      Returns true if there is a void reference
//      specified by the arguments from the cache.
//
//  Arguments:
//      name      The name of the object.
//      type      The type of the object.
//      timestep  The timestep for this object.
//      domain    The domain number (0-origin)
//
//  Returns:      true if the void reference corresponding to the
//                parameters if it exists, false otherwise.
//
//  Programmer: Jeremy Meredith
//  Creation:   October 24, 2002
//
// ****************************************************************************

bool
avtVariableCache::HasVoidRef(const char *var, const char *type, int timestep,
                             int domain)
{
    avtCachableItem *item = NULL;
    std::vector<OneVar *>::iterator it;
    for (it = voidRefVars.begin() ; it != voidRefVars.end() ; it++)
    {
        if (strcmp((*it)->GetVar(),  var)  == 0 &&
            strcmp((*it)->GetType(), type) == 0)
        {
            item = (*it)->GetItem("N/A", timestep, domain);
            if (item != NULL)
            {
                return true;
            }
        }
    }

    //
    // We couldn't find the void reference.
    //
    return false;
}


// ****************************************************************************
//  Method: avtVariableCache::GetVoidRef
//
//  Purpose:
//      Gets the void reference specified by the arguments from the cache.
//
//  Arguments:
//      name      The name of the object.
//      type      The type of the object.
//      timestep  The timestep for this object.
//      domain    The domain number (0-origin)
//
//  Returns:      The void reference corresponding to the parameters if it
//                exists, NULL otherwise.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2001
//
// ****************************************************************************

void_ref_ptr
avtVariableCache::GetVoidRef(const char *var, const char *type, int timestep,
                             int domain)
{
    avtCachableItem *item = NULL;
    std::vector<OneVar *>::iterator it;
    for (it = voidRefVars.begin() ; it != voidRefVars.end() ; it++)
    {
        if (strcmp((*it)->GetVar(),  var)  == 0 &&
            strcmp((*it)->GetType(), type) == 0)
        {
            item = (*it)->GetItem("N/A", timestep, domain);
            if (item != NULL)
            {
                avtCachedVoidRef *vr = (avtCachedVoidRef *) item;
                return vr->GetVoidRef();
            }
        }
    }

    //
    // We couldn't find the void reference.
    //
    return void_ref_ptr();
}


// ****************************************************************************
//  Method: avtVariableCache::CacheVoidRef
//
//  Purpose:
//      Stores the void reference in the cache with the associated information.
//
//  Arguments:
//      name       The name of this object.
//      type       The type of this object.
//      domain     The domain for this object.
//      timestep   The timestep this object is from.
//      vr         The actual void reference.
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2001
//
// ****************************************************************************

void
avtVariableCache::CacheVoidRef(const char *name, const char *type,
                                 int timestep, int domain, void_ref_ptr vr)
{
    OneVar  *v = NULL;

    std::vector<OneVar *>::iterator it;
    for (it = voidRefVars.begin() ; it != voidRefVars.end() ; it++)
    {
        if (strcmp((*it)->GetVar(),  name) == 0 &&
            strcmp((*it)->GetType(), type) == 0)
        {
            v = *it;
            break;
        }
    }
    if (v == NULL)
    {
        v = new OneVar(name, type);
        voidRefVars.push_back(v);
    }

    avtCachedVoidRef *cvr = new avtCachedVoidRef(vr);
    v->CacheItem("N/A", timestep, domain, cvr);
}


// ****************************************************************************
//  Method: avtVariableCache::ClearTimestep
//
//  Purpose:
//      Clears out a timestep.
//
//  Arguments:
//      ts      The timestep to clear out.
//
//  Programmer: Hank Childs
//  Creation:   January 7, 2002
//
//  Modifications:
//
//    Mark C. Miller, Thu Nov 30 19:58:19 PST 2006
//    Added code to clear out the objectPointerMap for pointer pairs
//
// ****************************************************************************

void
avtVariableCache::ClearTimestep(int ts)
{
    int   i;

    // clear out objectPointerMap items *before* vtkVars and voidRefVars
    std::vector<std::map<vtkObject*,vtkObject*>::iterator> itemsToRemove;
    std::map<vtkObject*,vtkObject*>::iterator it;
    for (it = objectPointerMap.begin(); it != objectPointerMap.end(); it++)
    {
        int objts;

        GetVTKObjectKey(0, 0, &objts, 0, 0, it->second);
        if (objts == ts)
        {
            itemsToRemove.push_back(it);
        }
    }
    for (i = 0 ; i < itemsToRemove.size() ; i++)
    {
        objectPointerMap.erase(itemsToRemove[i]);
    }
    for (i = 0 ; i < vtkVars.size() ; i++)
    {
        vtkVars[i]->ClearTimestep(ts);
    }
    for (i = 0 ; i < voidRefVars.size() ; i++)
    {
        voidRefVars[i]->ClearTimestep(ts);
    }
}

// ****************************************************************************
//  Method: OneDomain constructor
//
//  Arguments:
//      d       This object's one domain.
//
//  Programmer: Hank Childs
//  Creation:   September 15, 2000
//
// ****************************************************************************

avtVariableCache::OneDomain::OneDomain(int d)
{
    domain = d;
    item   = NULL;
}


// ****************************************************************************
//  Method: OneDomain destructor
//
//  Programmer: Hank Childs
//  Creation:   September 15, 2000
//
// ****************************************************************************

avtVariableCache::OneDomain::~OneDomain()
{
    if (item != NULL)
    {
        delete item;
        item = NULL;
    }
}


// ****************************************************************************
//  Method: OneDomain::CacheItem
//
//  Purpose:
//      Caches a avtCachableItem with this object.
//
//  Arguments:
//      i       A avtCachableItem that corresponds to this domain.
//
//  Programmer: Hank Childs
//  Creation:   September 15, 2000
//
// ****************************************************************************

void
avtVariableCache::OneDomain::CacheItem(avtCachableItem *i)
{
    if (item != NULL)
    {
        delete item;
        item = NULL;
    }

    item = i;
}


// ****************************************************************************
//  Method: OneTimestep contructor
//
//  Arguments:
//      t        A timestep.
//
//  Programmer: Hank Childs
//  Creation:   September 15, 2000
//
// ****************************************************************************

avtVariableCache::OneTimestep::OneTimestep(int t)
{
    timestep = t;
}


// ****************************************************************************
//  Method: OneTimestep destructor
//
//  Programmer: Hank Childs
//  Creation:   October 19, 2000
//
// ****************************************************************************

avtVariableCache::OneTimestep::~OneTimestep()
{
    std::vector<OneDomain *>::iterator it;
    for (it = domains.begin() ; it != domains.end() ; it++)
    {
        delete *it;
    }
}


// ****************************************************************************
//  Method: OneTimestep::CacheItem
//
//  Purpose:
//      Caches an item.
//
//  Arguments:
//      domain     The domain the item corresponds to.
//      im         The item to cache.
//
//  Programmer: Hank Childs
//  Creation:   September 15, 2000
//
// ****************************************************************************

void
avtVariableCache::OneTimestep::CacheItem(int domain, avtCachableItem *im)
{
    OneDomain  *d = NULL;
    std::vector<OneDomain *>::iterator it;
    for (it = domains.begin() ; it != domains.end() ; it++)
    {
        if (domain == (*it)->GetDomain())
        {
            d = *it;
            break;
        }
    }

    if (d == NULL)
    {
        d = new OneDomain(domain);
        domains.push_back(d);
    }

    d->CacheItem(im);
}


// ****************************************************************************
//  Method: OneTimestep::GetItem
//
//  Arguments:
//      domain    The domain of the item to get.
//
//  Returns:    The item corresponding to domain.  Might be NULL.
//
//  Programmer: Hank Childs
//  Creation:   September 15, 2000
//
// ****************************************************************************

avtCachableItem *
avtVariableCache::OneTimestep::GetItem(int domain)
{
    std::vector<OneDomain *>::iterator it;
    for (it = domains.begin() ; it != domains.end() ; it++)
    {
        if ((*it)->GetDomain() == domain)
        {
            return (*it)->GetItem();
        }
    }

    return NULL;
}


// ****************************************************************************
//  Method: OneMat constructor
//
//  Arguments:
//      m       The name of this object's material.
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2001
//
// ****************************************************************************

avtVariableCache::OneMat::OneMat(const char *m)
{
    material = new char[strlen(m)+1];
    strcpy(material, m);
}


// ****************************************************************************
//  Method: OneMat destructor
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2001
//
// ****************************************************************************

avtVariableCache::OneMat::~OneMat()
{
    if (material != NULL)
    {
        delete [] material;
        material = NULL;
    }
    std::vector<OneTimestep *>::iterator it;
    for (it = timesteps.begin() ; it != timesteps.end() ; it++)
    {
        delete *it;
    }
}


// ****************************************************************************
//  Method: OneMat::CacheItem
//
//  Purpose:
//      Caches the item for the given timestep, domain.
//
//  Arguments:
//      timestep     The timestep this dataset corresponds to.
//      domain       The domain this dataset corresponds to.
//      im           The item.
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2001
//
// ****************************************************************************

void
avtVariableCache::OneMat::CacheItem(int timestep, int domain,
                                    avtCachableItem *im)
{
    OneTimestep *t = NULL;

    std::vector<OneTimestep *>::iterator it;
    for (it = timesteps.begin() ; it != timesteps.end() ; it++)
    {
        if ((*it)->GetTimestep() == timestep)
        {
            t = *it;
            break;
        }
    }

    if (t == NULL)
    {
        t = new OneTimestep(timestep);
        timesteps.push_back(t);
    }

    t->CacheItem(domain, im);
}


// ****************************************************************************
//  Method: OneMat::GetItem
//
//  Purpose:
//      Gets the item for the timestep, domain.
//
//  Arguments:
//      timestep   The timestep for the desired item.
//      domain     The domain for the desired item.
//
//  Returns:    The item corresponding to timestep and domain.  Might be NULL.
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2001
//
// ****************************************************************************

avtCachableItem *
avtVariableCache::OneMat::GetItem(int timestep, int domain)
{
    std::vector<OneTimestep *>::iterator it;
    for (it = timesteps.begin() ; it != timesteps.end() ; it++)
    {
        if ((*it)->GetTimestep() == timestep)
        {
            return (*it)->GetItem(domain);
        }
    }

    //
    // We could not find the right timestep.
    //
    return NULL;
}


// ****************************************************************************
//  Method: OneMat::ClearTimestep
//
//  Purpose:
//      Clears out all containers of the specified timestep.
//
//  Arguments:
//      ts      The timestep to clear out.
//
//  Programmer: Hank Childs
//  Creation:   January 7, 2001
//
// ****************************************************************************

void
avtVariableCache::OneMat::ClearTimestep(int ts)
{
     std::vector<OneTimestep *> newTimesteps;
     for (int i = 0 ; i < timesteps.size() ; i++)
     {
         if (timesteps[i]->GetTimestep() == ts)
         {
              delete timesteps[i];
         }
         else
         {
              newTimesteps.push_back(timesteps[i]);
         }
     }

     //
     // Copy over the new vector that doesn't contain any of the ones we have
     // just deleted.
     //
     timesteps = newTimesteps;
}


// ****************************************************************************
//  Method: OneVar constructor
//
//  Arguments:
//      v       The name of this object's variable.
//      t       The name of this object's type.
//
//  Programmer: Hank Childs
//  Creation:   September 15, 2000
//
// ****************************************************************************

avtVariableCache::OneVar::OneVar(const char *v, const char *t)
{
    var = new char[strlen(v)+1];
    strcpy(var, v);
    type = new char[strlen(t)+1];
    strcpy(type, t);
}


// ****************************************************************************
//  Method: OneVar destructor
//
//  Programmer: Hank Childs
//  Creation:   September 15, 2000
//
// ****************************************************************************

avtVariableCache::OneVar::~OneVar()
{
    if (var != NULL)
    {
        delete [] var;
        var = NULL;
    }
    if (type != NULL)
    {
        delete [] type;
        type = NULL;
    }
    std::vector<OneMat *>::iterator it;
    for (it = materials.begin() ; it != materials.end() ; it++)
    {
        delete *it;
    }
}


// ****************************************************************************
//  Method: OneVar::CacheItem
//
//  Purpose:
//      Caches the item for the given timestep, domain.
//
//  Arguments:
//      material     The material this dataset corresponds to.
//      timestep     The timestep this dataset corresponds to.
//      domain       The domain this dataset corresponds to.
//      im           The item.
//
//  Programmer: Hank Childs
//  Creation:   September 15, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Oct  5 17:26:03 PDT 2001
//    Added an argument for the material.
//
//    Hank Childs, Tue Oct 16 07:53:35 PDT 2001
//    Fix cut-n-paste error.
//
// ****************************************************************************

void
avtVariableCache::OneVar::CacheItem(const char *material, int timestep,
                                    int domain, avtCachableItem *im)
{
    OneMat *t = NULL;

    std::vector<OneMat *>::iterator it;
    for (it = materials.begin() ; it != materials.end() ; it++)
    {
        if (strcmp((*it)->GetMaterial(), material) == 0)
        {
            t = *it;
            break;
        }
    }

    if (t == NULL)
    {
        t = new OneMat(material);
        materials.push_back(t);
    }

    t->CacheItem(timestep, domain, im);
}


// ****************************************************************************
//  Method: OneVar::GetItem
//
//  Purpose:
//      Gets the item for the timestep, domain.
//
//  Arguments:
//      material   The material for the desired item.
//      timestep   The timestep for the desired item.
//      domain     The domain for the desired item.
//
//  Returns:    The item corresponding to timestep and domain.  Might be NULL.
//
//  Programmer: Hank Childs
//  Creation:   September 15, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Oct  5 17:26:03 PDT 2001
//    Added an argument for the material.
//
// ****************************************************************************

avtCachableItem *
avtVariableCache::OneVar::GetItem(const char *material,
    int timestep,int domain)
{
    std::vector<OneMat *>::iterator it;
    for (it = materials.begin() ; it != materials.end() ; it++)
    {
        if (strcmp((*it)->GetMaterial(), material) == 0)
        {
            return (*it)->GetItem(timestep, domain);
        }
    }

    //
    // We could not find the right timestep.
    //
    return NULL;
}

// ****************************************************************************
//  Method: OneVar::ClearTimestep
//
//  Purpose:
//      Clears out all containers of the specified timestep.
//
//  Arguments:
//      ts      The timestep to clear out.
//
//  Programmer: Hank Childs
//  Creation:   January 7, 2001
//
// ****************************************************************************

void
avtVariableCache::OneVar::ClearTimestep(int ts)
{
    for (int i = 0 ; i < materials.size() ; i++)
    {
        materials[i]->ClearTimestep(ts);
    }
}


// ***************************************************************************
//  Method: avtVariableCache::Print
//
//  Purpose:
//      Prints out the object for debugging purposes.
//
//  Arguments:
//      out     An ostream.
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2000
//
// ****************************************************************************

void
avtVariableCache::Print(ostream &out)
{
    std::vector<OneVar *>::iterator it;

    out << "Cache:" << endl;

    out << "\tVoidRef:" << endl;
    for (it = voidRefVars.begin() ; it != voidRefVars.end() ; it++)
    {
        (*it)->Print(out, 1);
    }

    out << "\tVTK Objects:" << endl;
    for (it = vtkVars.begin() ; it != vtkVars.end() ; it++)
    {
        (*it)->Print(out, 1);
    }
}


// ****************************************************************************
//  Method: OneVar::Print
//
//  Purpose:
//      Prints out a OneVar for debugging purposes.
//
//  Arguments:
//      out      An ostream.
//      indent   The level of indentation.
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2000
//
// ****************************************************************************

void
avtVariableCache::OneVar::Print(ostream &out, int indent)
{
    Indent(out, indent);
    out << "Name = " << var << endl;
    Indent(out, indent);
    out << "Type = " << type << endl;
    std::vector<OneMat *>::iterator it;
    for (it = materials.begin() ; it != materials.end() ; it++)
    {
        (*it)->Print(out, indent+1);
    }
}


// ****************************************************************************
//  Method: OneMat::Print
//
//  Purpose:
//      Prints out a OneMat for debugging purposes.
//
//  Arguments:
//      out      An ostream.
//      indent   The level of indentation.
//
//  Programmer: Hank Childs
//  Creation:   October 5, 2001
//
// ****************************************************************************

void
avtVariableCache::OneMat::Print(ostream &out, int indent)
{
    Indent(out, indent);
    out << "Material = " << material << endl;
    std::vector<OneTimestep *>::iterator it;
    for (it = timesteps.begin() ; it != timesteps.end() ; it++)
    {
        (*it)->Print(out, indent+1);
    }
}


// ****************************************************************************
//  Method: OneTimestep::Print
//
//  Purpose:
//      Prints out a OneTimestep for debugging purposes.
//
//  Arguments:
//      out      An ostream.
//      indent   The level of indentation.
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2000
//
// ****************************************************************************

void
avtVariableCache::OneTimestep::Print(ostream &out, int indent)
{
    Indent(out, indent);
    out << "Timestep = " << timestep << endl;
    std::vector<OneDomain *>::iterator it;
    for (it = domains.begin() ; it != domains.end() ; it++)
    {
        (*it)->Print(out, indent+1);
    }
}


// ****************************************************************************
//  Method: OneDomain::Print
//
//  Purpose:
//      Prints out a OneDomain for debugging purposes.
//
//  Arguments:
//      out      An ostream.
//      indent   The level of indentation.
//
//  Programmer: Hank Childs
//  Creation:   September 20, 2000
//
// ****************************************************************************

void
avtVariableCache::OneDomain::Print(ostream &out, int indent)
{
    Indent(out, indent);
    out << "Domain = " << domain << endl;
    Indent(out, indent);
    out << "Item = " << item << endl;
}

// ****************************************************************************
//  Method: avtVariableCache::AddObjectPointerPair 
//
//  Purpose: Manager object pointers for cases where generic database returns
//  copies of what is cached. So far, this only happens for vtkObject pointers
//  and not VoidRef pointers.
//
//  Programmer: Mark C. Miller
//  Creation:   December 3, 2006 
//
// ****************************************************************************
void
avtVariableCache::AddObjectPointerPair(
    vtkObject *o1, vtkObject *o2)
{
    objectPointerMap[o1] = o2;
};

// ****************************************************************************
//  Method: avtVariableCache::RemoveObjectPointerPair
//
//  Purpose: Manager object pointers for cases where generic database returns
//  copies of what is cached
//
//  Programmer: Mark C. Miller
//  Creation:   December 3, 2006 
//
// ****************************************************************************
bool
avtVariableCache::RemoveObjectPointerPair(vtkObject *o1)
{
    std::map<vtkObject*,vtkObject*>::iterator it =
        objectPointerMap.find(o1);
    if (it != objectPointerMap.end())
    {
        objectPointerMap.erase(it);
        return true;
    }
    return false;
}

// ****************************************************************************
//  Method: avtVariableCache::FindObjectPointerPair
//
//  Purpose: Manager object pointers for cases where generic database returns
//  copies of what is cached
//
//  Programmer: Mark C. Miller
//  Creation:   December 3, 2006 
//
// ****************************************************************************
vtkObject*
avtVariableCache::FindObjectPointerPair(vtkObject *o1) const
{
    std::map<vtkObject*,vtkObject*>::const_iterator it =
        objectPointerMap.find(o1);
    if (it != objectPointerMap.end())
        return it->second;
    return 0;
}

inline void 
Indent(ostream &out, int n)
{
    for (int i = 0 ; i < n ; i++)
    {
        out << "\t";
    }
}

// ****************************************************************************
//  Method: avtCachedVoidRef constructor
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2001
//
//  Modifications:
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added setting of itemType so one impl. of GetItem is sufficient for
//    both types of items
//    
// ****************************************************************************

avtCachedVoidRef::avtCachedVoidRef(void_ref_ptr vr)
{
    itemType = VoidRef;
    voidRef = vr;
}


// ****************************************************************************
//  Method: avtCachedVoidRef destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtCachedVoidRef::~avtCachedVoidRef()
{
    ;
}


// ****************************************************************************
//  Method: avtCachedVTKObject constructor
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2001
//    
//  Modifications:
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added setting of itemType so one impl. of GetItem is sufficient for
//    both types of items
// ****************************************************************************

avtCachedVTKObject::avtCachedVTKObject(vtkObject *o)
{
    itemType = VTKObject;
    obj = o;
    if (obj != NULL)
    {
        obj->Register(NULL);
    }
}


// ****************************************************************************
//  Method: avtCachedVTKObject destructor
//
//  Programmer: Hank Childs
//  Creation:   May 22, 2001
//
// ****************************************************************************

avtCachedVTKObject::~avtCachedVTKObject()
{
    if (obj != NULL)
    {
        obj->Delete();
        obj = NULL;
    }
}
