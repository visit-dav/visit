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
