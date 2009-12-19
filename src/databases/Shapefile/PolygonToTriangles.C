#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
// If we're on Apple then use the mangled Mesa GLU functions since it's 
// not readily apparent where else we can find them.
#define gluTess             mgluTess
#define gluDeleteTess       mgluDeleteTess
#define gluNewTess          mgluNewTess
#define gluTessBeginContour mgluTessBeginContour
#define gluTessBeginPolygon mgluTessBeginPolygon
#define gluTessCallback     mgluTessCallback
#define gluTessEndContour   mgluTessEndContour
#define gluTessEndPolygon   mgluTessEndPolygon
#define gluTessNormal       mgluTessNormal
#define gluTessProperty     mgluTessProperty
#define gluTessVertex       mgluTessVertex
#endif
#include <GL/glu.h>
#include <snprintf.h>
#include <map>

// ****************************************************************************
// Class: VertexManager
//
// Purpose:
//   This class manages inserts into a vtkPoints object to make sure that the
//   points are unique.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 7 08:58:03 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

class VertexManager
{
public:
    VertexManager(vtkPoints *pts)
    {
        points = pts;
    }

    ~VertexManager()
    {
    }

    int GetVertexId(const GLdouble *vert)
    {
        char keystr[200];
        SNPRINTF(keystr, 200, "%12.12e,%12.12e,%12.12e", vert[0], vert[1], vert[2]);
        std::string key(keystr);
        int ret;
        std::map<std::string, int>::const_iterator pos = 
            vertexNamesToIndex.find(key);
        if(pos != vertexNamesToIndex.end())
        {
            ret = pos->second;
        }
        else
        {
            int index = points->GetNumberOfPoints();
            points->InsertNextPoint((const double *)vert);

            vertexNamesToIndex[key] = index;
            ret = index;
        }
        return ret;
    }

private:
    vtkPoints *points; // Does not own this pointer.
    std::map<std::string, int> vertexNamesToIndex;
};

// ****************************************************************************
// Class: PolygonToTriangles
//
// Purpose:
//   This class contains a GLU tessellator object that can convert polygons
//   into triangles. This class provides encapsulation and a mechanism for
//   the resulting triangles to be read out and used.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 7 08:59:04 PDT 2007
//
// Modifications:
//   Eric Brugger, Tue Mar 13 15:50:51 PDT 2007
//   Added coding specific to gcc 3.2 to get around a compiler bug.
//   
//   Kathleen Bonnell, Thu Mar 29 09:01:59 PDT 2007 
//   Added WIN32 specific code.
//   
//   Kathleen Bonnell, Mon Oct  1 07:50:46 PDT 2007 
//   Added SetNormal and ClearTriangles methods. 
//   
// ****************************************************************************

class PolygonToTriangles
{
public:
    PolygonToTriangles(VertexManager *tuv) : vertexAllocs()
    {
        tessMode = GL_TRIANGLES;
        vertexMgr = tuv;
        nTrianglesInPolygon = 0;

        // Create a tessellator object and set up its callbacks.
        tess = gluNewTess();
#if defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ == 0)
        gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (GLvoid (*)(...))vertexCallback);
        gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (GLvoid (*)(...))beginCallback);
        gluTessCallback(tess, GLU_TESS_END_DATA, (GLvoid (*)(...))endCallback);
        gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (GLvoid (*)(...))combineCallback);
#elif defined(_WIN32)
        gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (VOID (CALLBACK *)())vertexCallback);
        gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (VOID (CALLBACK *)())beginCallback);
        gluTessCallback(tess, GLU_TESS_END_DATA, (VOID (CALLBACK *)())endCallback);
        gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (VOID (CALLBACK *)())combineCallback);
#else
        gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (GLvoid (*)())vertexCallback);
        gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (GLvoid (*)())beginCallback);
        gluTessCallback(tess, GLU_TESS_END_DATA, (GLvoid (*)())endCallback);
        gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (GLvoid (*)())combineCallback);
#endif
        gluTessNormal(tess, 0, 0, 1);
        gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
        gluTessProperty(tess, GLU_TESS_TOLERANCE, 0.);
    }

    ~PolygonToTriangles()
    {
        // Free all of the vertices that we allocated for the tessellator.
        for(int i = 0; i < vertexAllocs.size(); ++i)
            free(vertexAllocs[i]);
        vertexAllocs.clear();

        gluDeleteTess(tess);
    }

    void SetNormal(const double*n)
    {
        gluTessNormal(tess, n[0], n[1], n[2]);
    }

    void BeginPolygon()
    {
        nTrianglesInPolygon = 0;
        gluTessBeginPolygon(tess, (void *)this);
    }

    void BeginContour()
    {
        gluTessBeginContour(tess);
    }

    void AddVertex(const double *pt)
    {
        GLdouble *newPt = AllocateVertex();
        newPt[0] = pt[0];
        newPt[1] = pt[1];
        newPt[2] = pt[2];
        gluTessVertex(tess, newPt, newPt);
    }

    void EndContour()
    {
        gluTessEndContour(tess);
    }

    void EndPolygon()
    {
        gluTessEndPolygon(tess);
    }

    int GetNumTrianglesInLastPolygon() const
    {
        return nTrianglesInPolygon;
    }

    int GetNumTriangles() const
    {
        return triangles.size() / 3;
    }

    bool GetTriangle(int t, int &a, int &b, int &c) const
    {
        bool ret;
        if((ret = (t >= 0 && t < triangles.size())) == true)
        {
            a = triangles[t*3];
            b = triangles[t*3+1];
            c = triangles[t*3+2];
        }
        return ret;
    }

    void ClearTriangles()
    {
        nTrianglesInPolygon = 0;
        triangles.clear();
        intermediateVertexIds.clear();
    }
private:
    // Allocates vertex memory and logs it for deletion later.
    GLdouble *AllocateVertex()
    {
        GLdouble *arr = (GLdouble *)malloc(sizeof(GLdouble) * 3);
        vertexAllocs.push_back(arr);
        return arr;
    }

    int GetVertexId(GLdouble *vertex)
    {
        // Adds a vertex index using the vertex manager. This can
        // cause the VTK points to grow.
        int id = vertexMgr->GetVertexId(vertex);
        return id;
    }

    void AddTriangle(int a, int b, int c)
    {
        nTrianglesInPolygon ++;
        triangles.push_back(a);
        triangles.push_back(b);
        triangles.push_back(c);
    }

    // GLU tess callbacks
    static void beginCallback(GLenum w, void *cbdata);
    static void endCallback(void *cbdata);
    static void vertexCallback(GLvoid *vertex, void *cbdata);
    static void combineCallback(GLdouble coords[3],
                                void *vertex_data[4], 
                                GLfloat weight[4], void **dataOut,
                                void *cbdata);

    GLUtesselator           *tess;
    GLenum                   tessMode;
    VertexManager           *vertexMgr;
    std::vector<int>         intermediateVertexIds;
    int                      nTrianglesInPolygon;

    // List of triangles created over many calls to 
    std::vector<int>         triangles;

    // For vertices allocated due to tesselation
    std::vector<GLdouble *>  vertexAllocs; 
};

// ****************************************************************************
// Method: PolygonToTriangles::beginCallback
//
// Purpose: 
//   Begin callback for the tessellator.
//
// Arguments:
//   w      : The type of primitives being created (TRIANGLES, TRIANGLE_STRIP, 
//            TRIANGLE_FAN)
//   cbdata : Callback data that points to "this".
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 7 09:01:13 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
PolygonToTriangles::beginCallback(GLenum w, void *cbdata)
{
    PolygonToTriangles *This = (PolygonToTriangles *)cbdata;

    This->tessMode = w;
    This->intermediateVertexIds.clear();
}

// ****************************************************************************
// Method: PolygonToTriangles::endCallback
//
// Purpose: 
//   End callback for the tessellator. Translates intermediate vertices and
//   drawing mode into triangle definitions.
//
// Arguments:
//   w      : The type of primitives being created (TRIANGLES, TRIANGLE_STRIP, 
//            TRIANGLE_FAN)
//   cbdata : Callback data that points to "this".
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 7 09:01:13 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
PolygonToTriangles::endCallback(void *cbdata)
{
    PolygonToTriangles *This = (PolygonToTriangles *)cbdata;

    // Use the tessMode, intermediateVertexIds, and the vertexList to
    // create triangles that get put into triangleList.
    if(This->tessMode == GL_TRIANGLES)
    {
        for(unsigned int i = 0; i < This->intermediateVertexIds.size(); i += 3)
        {
            // Prevent degenerate triangles
            int a = This->intermediateVertexIds[i];
            int b = This->intermediateVertexIds[i+1];
            int c = This->intermediateVertexIds[i+2];
            if(a == b || b == c || a == c)
                continue;

            This->AddTriangle(a, b, c);
        }
    }
    else if(This->tessMode == GL_TRIANGLE_STRIP)
    {
        for(unsigned int i = 2; i < This->intermediateVertexIds.size(); ++i)
        {
            int N = i-2;
            int N_1 = i-1;
            int N_2 = i;

            // Prevent degenerate triangles
            int a = This->intermediateVertexIds[N];
            int b = This->intermediateVertexIds[N_1];
            int c = This->intermediateVertexIds[N_2];
            if(a == b || b == c || a == c)
                continue;

            This->AddTriangle(a, b, c);
        }
    }
    else if(This->tessMode == GL_TRIANGLE_FAN)
    {
        for(unsigned int i = 2; i < This->intermediateVertexIds.size(); ++i)
        {
            int N = 0;
            int N_1 = i-1;
            int N_2 = i;

            // Prevent degenerate triangles
            int a = This->intermediateVertexIds[N];
            int b = This->intermediateVertexIds[N_1];
            int c = This->intermediateVertexIds[N_2];
            if(a == b || b == c || a == c)
                continue;

            This->AddTriangle(a, b, c);
        }
    }
}

// ****************************************************************************
// Method: PolygonToTriangles::vertexCallback
//
// Purpose: 
//   Vertex callback for the tessellator. This method uses the unique vertex
//   object to translate the vertex into a vertex id (storing the vertex into
//   the vtkPoints, if needed) and stores that id into the intermediate vertex
//   ids that will be used to create triangle connectivity.
//
// Arguments:
//   vertex : The triangle vertex.
//   cbdata : Callback data that points to "this".
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 7 09:01:13 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
PolygonToTriangles::vertexCallback(GLvoid *vertex, void *cbdata)
{
    GLdouble *p = (GLdouble *)vertex;
    PolygonToTriangles *This = (PolygonToTriangles *)cbdata;

    // Add the vertex to the list of vertices.
    This->intermediateVertexIds.push_back(This->GetVertexId(p));
}

// ****************************************************************************
// Method: PolygonToTriangles::combineCallback
//
// Purpose: 
//   Combine callback for the tessellator that gets called when vertices need
//   to be combined.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 7 09:07:03 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
PolygonToTriangles::combineCallback(GLdouble coords[3],
    void *vertex_data[4], 
    GLfloat weight[4], void **dataOut, void *cbdata)
{
    PolygonToTriangles *This = (PolygonToTriangles *)cbdata;
    GLdouble *vertex = This->AllocateVertex();

    // Blend the height data for the vertex.
    GLdouble **vd = (GLdouble **)vertex_data;
    for(int i = 0; i < 3; ++i)
    {
        double a = (vertex_data[0]!=0) ? (weight[0] * vd[0][i]) : 0.; 
        double b = (vertex_data[1]!=0) ? (weight[1] * vd[1][i]) : 0.; 
        double c = (vertex_data[2]!=0) ? (weight[2] * vd[2][i]) : 0.;
        double d = (vertex_data[3]!=0) ? (weight[3] * vd[3][i]) : 0.; 

        vertex[i] = a + b + c + d;
    }

    *dataOut = vertex;
}
