#ifndef _BRANCH_H_
#define _BRANCH_H_

#include <boost/cstdint.hpp>
#include <cassert>
#include <iostream>
#include <vector>

class Branch
{
    public:
        typedef boost::uint64_t IdxT;
        typedef double ValueT;
        IdxT extremum;
        ValueT extremumVal;
        IdxT saddle;
        ValueT saddleVal;
        std::vector<Branch*> children;
        IdxT volume;
        IdxT volumeAfterLastSaddle;
        IdxT id;
        IdxT depth;
        Branch *parent;

        static Branch *unarchive(std::istream &is, unsigned int depth = 0);
        void print(int indent=0) const;
        static IdxT branchIdForValue(const Branch *b1, const Branch*b2, ValueT val);
        IdxT maxId() const;
        void generateIdToBranchMap(const Branch **idToBranchMap) const;
};

inline Branch *Branch::unarchive(std::istream &is, unsigned int depth)
{
    Branch *b = new Branch();
    is.read(reinterpret_cast<char*>(&b->extremum), sizeof(b->extremum));
    is.read(reinterpret_cast<char*>(&b->extremumVal), sizeof(b->extremumVal));
    is.read(reinterpret_cast<char*>(&b->saddle), sizeof(b->saddle));
    is.read(reinterpret_cast<char*>(&b->saddleVal), sizeof(b->saddleVal));
    is.read(reinterpret_cast<char*>(&b->volume), sizeof(b->volume));
    is.read(reinterpret_cast<char*>(&b->volumeAfterLastSaddle), sizeof(b->volumeAfterLastSaddle));
    is.read(reinterpret_cast<char*>(&b->id), sizeof(b->id));
    b->depth = depth;
    size_t num = 0;
    is.read(reinterpret_cast<char*>(&num), sizeof(num));
    b->children.resize(num);
    for (unsigned int i=0; i<num; ++i)
    {
        b->children[i] = unarchive(is, depth + 1);
        b->children[i]->parent = b;
    }
    return b;
}

inline Branch::IdxT Branch::branchIdForValue(const Branch *b1, const Branch*b2, ValueT val)
{
    //std::cout << "Finding branch between " << b1->id << " and " << b2->id << " for value " << val << std::endl;
    while (b1->depth > b2->depth)
    {
        //std::cout << ">Val = " << val << " b1->saddleVal = " << b1->saddleVal << std::endl;
        if (val < b1->saddleVal)
            return b1->id;
        assert(b1->parent);
        b1 = b1->parent;
        //std::cout << "Going to parent of branch 1. New branch id is " << b1->id << std::endl;
    }
    while (b2->depth > b1->depth)
    {
        //std::cout << "<Val = " << val << " b2->saddleVal = " << b2->saddleVal << std::endl;
        if (val > b2->saddleVal)
            return b2->id;
        assert(b2->parent);
        b2 = b2->parent;
        //std::cout << "Going to parent of branch 2. New branch id is " << b2->id << std::endl;
    }
    while (b1 != b2)
    {
        //std::cout << "=Val = " << val << " b1->saddleVal = " << b1->saddleVal << std::endl;
        if (val < b1->saddleVal)
            return b1->id;
        //std::cout << "=Val = " << val << " b2->saddleVal = " << b2->saddleVal << std::endl;
        if (val > b2->saddleVal)
            return b2->id;
        assert(b1->parent);
        b1 = b1->parent;
        assert(b2->parent);
        b2 = b2->parent;
        assert(b1 && b2);
        //std::cout << "Going to parents of both branches. New branch ids are " << b1->id << " and " << b2->id << std::endl;
    }
    assert(b1 == b2);
    return b1->id;
}

inline void Branch::print(int indent) const
{
   for (int i=0; i<indent; ++i) std::cout << " ";
   std::cout << "{" << std::endl;
   for (int i=0; i<indent+2; ++i) std::cout << " ";
   std::cout << "Extremum = " << extremumVal << "(" <<extremum << ")" << std::endl;
   for (int i=0; i<indent+2; ++i) std::cout << " ";
   std::cout << "Saddle = " << saddleVal << "(" << saddle << ")" << std::endl;
   for (int i=0; i<indent+2; ++i) std::cout << " ";
   std::cout << "Id = " << id << std::endl;
   for (int i=0; i<indent+2; ++i) std::cout << " ";
   std::cout << "Volume = " << volume << std::endl;
   for (int i=0; i<indent+2; ++i) std::cout << " ";
   std::cout << "VolumeAfterLastSaddle = " << volumeAfterLastSaddle << std::endl;
   for (int i=0; i<indent+2; ++i) std::cout << " ";
   std::cout << "Children = [" << std::endl;
   for (std::vector<Branch*>::const_iterator it = children.begin(); it != children.end(); ++it) {
      for (int i=0; i<indent; ++i) std::cout << " ";
      (*it)->print(indent+4);
   }
   for (int i=0; i<indent+2; ++i) std::cout << " ";
   std::cout << "]" << std::endl;
   for (int i=0; i<indent; ++i) std::cout << " ";
   std::cout << "}" << std::endl;
}

inline Branch::IdxT Branch::maxId() const
{
    IdxT mId = id;
    for (std::vector<Branch*>::const_iterator it = children.begin(); it != children.end(); ++it)
        mId = std::max(mId, (*it)->maxId());
    return mId;
}

inline void Branch::generateIdToBranchMap(const Branch **idToBranchMap) const
{
    idToBranchMap[id] = this;
    for (std::vector<Branch*>::const_iterator it = children.begin(); it != children.end(); ++it)
        (*it)->generateIdToBranchMap(idToBranchMap);
}

#endif
