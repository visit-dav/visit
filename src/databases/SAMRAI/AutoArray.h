
template<class T>
class AutoArray
{
    public:

        AutoArray(int _size=1000, int _ainc=1000, bool _strict=false)
            {    size = _size;
                 ainc = _ainc;
                 maxidx = -1;
                 strictSize = _strict;
                 dataRetrieved = false;
                 dummy = 0.0;

                 if (size <= 0)
                     size = 1000;
                 if (ainc <= 0)
                     ainc = 1000;

                 data = (T*) malloc(size * sizeof(T));
            };

        ~AutoArray()
            {
                if (!dataRetrieved)
                {
                    if (data != 0)
                        free(data);
                }
            };

        int GetSize() const
            { 
                if (dataRetrieved)
                    return size;
                    
                return -1;
            };

        T* GetData()
            { 
                if (dataRetrieved)
                    return 0;

                dataRetrieved = true;

                if (strictSize || (maxidx+1 < size/2))
                {
                    data = (T*) realloc((void*)data, (maxidx+1) * sizeof(T)); 
                    size = maxidx+1;
                }

                return data;
            };

        T& operator[](int idx)
            {
                if (dataRetrieved)
                    return dummy;

                // check if we need to resize
                if (idx >= size)
                {
                   int newSize = size;

                   while (idx >= newSize)
                       newSize += ainc; 

                   // do the resize
                   size = newSize;
                   data = (T*) realloc((void*)data, size * sizeof(T)); 
                }

                if (idx > maxidx)
                    maxidx = idx;

                return data[idx];
            }

    private:
        T* data;
        int size;
        int ainc;
        int maxidx;
        float dummy;
        bool strictSize;
        bool dataRetrieved;
};
