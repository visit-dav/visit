#ifndef VISIT_MAP_H
#define VISIT_MAP_H

// ****************************************************************************
// Class: VisItMap
//
// Purpose:
//   This is a simplified version of the std::map class. It has most of the
//   map's interface but it lacks some of the less used methods.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 15 08:20:39 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Apr 22 12:58:06 PDT 2003
//   I rewrote it so that it is always sorted and so that it uses pointers
//   to the elements so that insertion is faster. I eliminated the use of
//   bsearch and qsort because they caused memory problems in certain
//   situations.
//
// ****************************************************************************

template <class K, class T>
class VisItMap
{
public:
    // Storage type
    class value_type
    {
    public:
        value_type() : first(), second()
        {
        }

        value_type(const K &key, const T &value) : first(key), second(value)
        {
        }

        value_type(const value_type &val) : first(val.first), second(val.second)
        {
        }

        virtual ~value_type()
        {
        }

        void operator = (const value_type &val)
        {
            first = val.first;
            second = val.second;
        }

        K first;
        T second;
    };

    // Iterator type
    class iterator
    {
        friend class VisItMap<K,T>;
    public:
        iterator()
        {
            m = 0;
            i = 0;
        }

        iterator(const iterator &it)
        {
            m = it.m;
            i = it.i;
        }

        virtual ~iterator()
        {
        }

        iterator operator = (const iterator &it)
        {
            m = it.m;
            i = it.i;

            return *this;
        }

        bool operator == (const iterator &it) const
        {
            return (m == it.m) && (i == it.i);
        }

        bool operator != (const iterator &it) const
        {
            return (m != it.m) || (i != it.i);
        }

        void operator ++()    { ++i; }
        void operator ++(int) { ++i; }
        void operator --()    { --i; }
        void operator --(int) { --i; }

        value_type *operator ->()
        {
            return *(m->storage + i);
        }
    protected:
        const VisItMap<K, T> *m;
        int                   i;
    };

    // Const iterator type.
    class const_iterator : public iterator
    {
    public:
        const_iterator() : iterator()
        {
        }

        const_iterator(const iterator &it) : iterator(it)
        {
        }

        virtual ~const_iterator()
        {
        }

        const_iterator operator = (const const_iterator &it)
        {
            m = it.m;
            i = it.i;

            return *this;
        }

        const value_type *operator ->() const
        {
            return *(m->storage + i);
        }
    };

    //
    // VisItMap methods
    //

    VisItMap()
    {
        storage = 0;
        elements = 0;
        nalloc = 0;
        increment = 32;
    }

    VisItMap(const VisItMap<K,T> &m)
    {
        if(m.nalloc > 0)
        {
            storage = new value_type *[m.nalloc];
            for(int i = 0; i < m.elements; ++i)
                storage[i] = m.storage[i];
            
            elements = m.elements;
            nalloc = m.nalloc;
        }
        else
        {
            storage = 0;
            elements = nalloc = 0;
        }
        increment = m.increment;
    }

    void clear()
    {
        if(storage)
        {
            for(int i = 0; i < elements; ++i)
                delete storage[i];
            delete [] storage;
        }
        storage = 0;
        elements = 0;
        nalloc = 0;
    }

    virtual ~VisItMap()
    {
        clear();
    }

    void operator = (const VisItMap<K,T> &m)
    {
        if(m.elements > elements)
            enough_space(m.elements);
        for(int i = 0; i < elements; ++i)
            delete storage[i];
        for(int j = 0; j < m.elements; ++j)
            storage[j] = new value_type(*(m.storage[j]));
        elements = m.elements;
    }

    void insert(const value_type &t)
    {
        this->operator[](t.first) = t.second;
    }

    iterator begin() const
    {
        iterator it;
        it.m = this;
        it.i = 0;
        return it;
    }

    iterator end() const
    {
        iterator it;
        it.m = this;
        it.i = elements;
        return it;
    }

    iterator find(const K &key) const
    {
        iterator it(end());

        if(storage)
        {
            int index = -1;
            if(findIndex(key, index))
                it.i = index;
        }

        return it;
    }

    T &operator[](const K &key)
    {
        iterator it;
        it.m = this;

        int index = 0;
        if(findIndex(key, index))
        {
            it.i = index;
        }
        else
        {
            enough_space();

            if(elements > 0)
            {
                for(int i = elements; i > index; --i)
                    storage[i] = storage[i - 1];
            }

            storage[index] = new value_type;
            storage[index]->first = key;
            ++elements;

            it.i = index;
        }

        return it->second;       
    }

    int size() const { return elements; }
    bool empty() const { return elements == 0; }

    void erase(const iterator &it)
    {
        if(storage != 0 && elements > 0 && it.i >= 0 && it.i < elements)
        {
            delete storage[it.i];

            for(int i = it.i; i < elements - 1; ++i)
                storage[i] = storage[i+1];
            --elements;
        }
    }

    void swap(VisItMap<K,T> &m)
    {
        value_type **temp0 = storage;
        int temp1 = elements;
        int temp2 = nalloc;

        storage = m.storage;
        elements = m.elements;
        nalloc = m.nalloc;

        m.storage = temp0;
        m.elements = temp1;
        m.nalloc = temp2;
    }

    bool operator == (const VisItMap<K,T> &m) const
    {
        bool retval = false;
        if(elements == m.elements)
        {
            retval = true;
            for(int i = 0; i < m.elements && retval; ++i)
            {
                retval &= ((storage[i]->first  == m.storage[i]->first) &&
                           (storage[i]->second == m.storage[i]->second));
            }
        }
        return retval;
    }

private:
    friend class iterator;
    friend class const_iterator;

    value_type **storage;
    int elements;
    int nalloc;
    int increment;

    void enough_space(int growth = 1)
    {
        if(elements+growth >= nalloc)
        {
            int s = ((nalloc+increment) >= (elements+growth)) ? increment : (growth+1);
            value_type **newStorage = new value_type*[nalloc + s];
            register int i;
            if(nalloc > 0)
            {
                for(i = 0; i < elements; ++i)
                    newStorage[i] = storage[i];
                for(i = nalloc; i < nalloc + s; ++i)
                    newStorage[i] = 0;
            }
            delete [] storage;
            storage = newStorage;
            nalloc += s;
            increment *= 2;
        }
    }

    bool findIndex(const K &key, int &index) const
    {
        int first = 0;
        int last  = elements - 1;
        bool retval = false;
        bool looping = (storage != 0 && elements > 0);
        index = 0;

        while(looping)
        {
            if(key > storage[last]->first)
            {
                // The key is the largest thing in the map.
                index = elements;
                retval = false;
                looping = false;
            }
            else if(key == storage[last]->first)
            { 
                index = last;
                retval = true;
                looping = false;
            }
            else if(key < storage[first]->first)
            {
                index = first;
                retval = false;
                looping = false;
            }
            else if(key == storage[first]->first)
            {
                index = first;
                retval = true;
                looping = false;
            }
            else
            {
                int delta = (last - first) >> 1;
                int pivot = first + delta;

                if(key == storage[pivot]->first)
                {
                    index = pivot;
                    retval = true;
                    looping = false;
                }
                else if(key < storage[pivot]->first)
                    last = pivot;
                else
                    first = pivot + 1;
            }
        }

        return retval;
    }
};

#endif
