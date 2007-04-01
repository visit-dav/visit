// ************************************************************************* //
//                             avtVariableCache.h                            //
// ************************************************************************* //

#ifndef AVT_VARIABLE_CACHE_H
#define AVT_VARIABLE_CACHE_H

#include <database_exports.h>

#include <visitstream.h>
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
    static const char     *DATASET_NAME;

    vtkObject             *GetVTKObject(const char *name, const char *type,
                                        int ts, int domain, const char *mat);
    void                   CacheVTKObject(const char *name, const char *type,
                                          int ts, int domain, const char *mat,
                                          vtkObject *);

    bool                   HasVoidRef(const char *name, const char *type,
                                      int ts, int domain);
    void_ref_ptr           GetVoidRef(const char *name, const char *type,
                                      int ts, int domain);
    void                   CacheVoidRef(const char *name, const char *type,
                                        int ts, int domain, void_ref_ptr);
    void                   ClearTimestep(int);

    void                   Print(ostream &);

  protected:

    class OneDomain
    {
      public:
                          OneDomain(int);
        virtual          ~OneDomain();
    
        void              CacheItem(avtCachableItem *);
        int               GetDomain(void)   { return domain; };
        avtCachableItem  *GetItem(void)     { return item; };
    
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
        int                         GetTimestep(void) { return timestep; };
    
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
        const char                 *GetMaterial(void) { return material; };
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
        const char                  *GetVar(void)   { return var; };
        const char                  *GetType(void)  { return type; };
        void                         ClearTimestep(int);
    
        void                         Print(ostream &, int);

      protected:
        char                        *var;
        char                        *type;
        std::vector<OneMat *>        materials;
    };

    std::vector<OneVar *>            vtkVars;
    std::vector<OneVar *>            voidRefVars;
};


class DATABASE_API  avtCachableItem
{
  public:
                          avtCachableItem();
   virtual               ~avtCachableItem();
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


