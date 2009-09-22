#include <MshData.h>

#include <vtkCellType.h>
#include <vtkPoints.h>
#include <vtkUnstructuredGrid.h>

#include <DebugStream.h>
#include <InvalidDBTypeException.h>

// ****************************************************************************
// Method: MshData::MshData
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 21 14:59:33 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

MshData::MshData() : domains(), cellType(0)
{
}

// ****************************************************************************
// Method: MshData::~MshData
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 21 14:59:43 PDT 2009
//
// Modifications:
//   
// ****************************************************************************
MshData::~MshData()
{
    ReleaseData();
}

// ****************************************************************************
// Method: MshData::Open
//
// Purpose: 
//   Opens the MSH file and reads its contents, creating vtkUnstructuredGrid
//   objects.
//
// Arguments:
//   filename : The name of the file to open.
//
// Returns:    True on success; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 21 14:59:53 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

bool
MshData::Open(const std::string &filename)
{
    const char *mName = "MshData::Open: ";

    bool retval = false;
    ReleaseData();

    // If the file is an ".msh" file then we may have more than 1 domain file.
    if(filename.size() >= 3 &&
       filename.substr(filename.size()-3, filename.size()) == "msh")
    {
        // Open the file.
        ifstream ifile(filename.c_str());
        char line[1024];
        if(!ifile.fail())
        {
            debug4 << mName << "Opened " << filename << endl;

            // Process the file
            int nCells = 0;
            ifile.getline(line, 1024);
            if(sscanf(line, "%d", &nCells) != 1 ||
               nCells < 1)
            {
                EXCEPTION1(InvalidDBTypeException, "This is not a MSH file");
            }
            debug4 << mName << "nCells = " << nCells << endl;

            //
            // Read the connectivity
            //
            vtkIdType conn[10];
            ifile.getline(line, 1024);
            int nNodesPerCell = sscanf(line, "%d %d %d %d %d %d %d %d %d %d", 
                &conn[0], &conn[1], &conn[2], &conn[3], &conn[4], 
                &conn[5], &conn[6], &conn[7], &conn[8], &conn[9]);
            if(nNodesPerCell == 4)
                cellType = 0;
            else if(nNodesPerCell == 10)
                cellType = 1;
            else if(nNodesPerCell == 8)
                cellType = 2;
            else
            {
                EXCEPTION1(InvalidDBTypeException, "This is not a MSH file");
            }
            debug4 << mName << "cellType = " << cellType << endl;

#ifndef MDSERVER
            // Can we use sscanf %d into vtkIdType in 64bit?
            vtkIdType *connectivity = new vtkIdType[nNodesPerCell * nCells];
            memcpy(connectivity, conn, sizeof(vtkIdType)*nNodesPerCell);
            vtkIdType *c = connectivity + nNodesPerCell;
            for(int i = 1; i < nCells; ++i)
            {
                ifile.getline(line, 1024);
                if(nNodesPerCell == 4)
                    sscanf(line, "%d %d %d %d", &c[0], &c[1], &c[2], &c[3]);
                else if(nNodesPerCell == 10)
                {
                    sscanf(line, "%d %d %d %d %d %d %d %d %d %d", 
                        &c[0], &c[1], &c[2], &c[3], &c[4], &c[5], &c[6], &c[7], &c[8], &c[9]);
                }
                else if(nNodesPerCell == 8)
                {
                    sscanf(line, "%d %d %d %d %d %d %d %d", 
                        &c[0], &c[1], &c[2], &c[3], &c[4], &c[5], &c[6], &c[7]);
                }
                c += nNodesPerCell;
            }

            //
            // Read the nodes
            //
            int nNodes = 0;
            ifile.getline(line, 1024);
            if(sscanf(line, "%d", &nNodes) != 1 ||
               nNodes < 1)
            {
                delete [] connectivity;
                EXCEPTION1(InvalidDBTypeException, "This is not a MSH file");
            }
            debug4 << mName << "nNodes = " << nNodes << endl;

            vtkPoints *pts = vtkPoints::New();
            pts->SetNumberOfPoints(nNodes);
            float *f = (float *)pts->GetVoidPointer(0);
            for(int i = 0; i < nNodes; ++i)
            {
                ifile.getline(line, 1024);
                if(sscanf(line, "%g %g %g", &f[0], &f[1], &f[2]) != 3)
                {
                    debug4 << mName << "Could not read node " << i << endl;
                    f[0] = f[1] = f[2] = 0.f;
                }
                f += 3;
            }

            // Try and read the number of domains.
            int nDomains = 0;
            ifile.getline(line, 1024);
            if(sscanf(line, "%d", &nDomains) == 1)
            {
                if(nDomains < 1)
                    nDomains = 0;
            }
            debug4 << mName << "line = " << line << endl;
            debug4 << mName << "nDomains = " << nDomains << endl;

            //
            // Now read the cellids in the domains
            //
            if(nDomains == 0)
            {
                debug4 << mName << "Creating single mesh" << endl;

                // There are no domains listed so assume 1 domain.
                vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
                ugrid->Allocate(nCells);
                ugrid->SetPoints(pts);
                if(cellType == 0)
                {
                    for(int cellid = 0; cellid < nCells; ++cellid)
                        ugrid->InsertNextCell(VTK_TETRA, 4, connectivity + cellid * 4);
                }
                else if(cellType == 1)
                {
                    for(int cellid = 0; cellid < nCells; ++cellid)
                    {
                        vtkIdType *mshconn = connectivity + cellid * 10;
                        // Reorder the connectivity since msh and VTK use different node ordering.
                        conn[0] = mshconn[0];
                        conn[1] = mshconn[1];
                        conn[2] = mshconn[2];
                        conn[3] = mshconn[3];
                        conn[4] = mshconn[4];
                        conn[5] = mshconn[7];
                        conn[6] = mshconn[5];
                        conn[7] = mshconn[6];
                        conn[8] = mshconn[9];
                        conn[9] = mshconn[8];
                        ugrid->InsertNextCell(VTK_QUADRATIC_TETRA, 10, conn);
                    }
                }
                else if(cellType == 2)
                {
                    for(int cellid = 0; cellid < nCells; ++cellid)
                        ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, connectivity + cellid * 8);
                }

                domains.push_back(ugrid);
            }
            else
            {
                debug4 << mName << "Creating multi-domain mesh" << endl;

                // Now read the cellids in the domains
                for(int d = 0; d < nDomains; ++d)
                {
                    int cellsInDomain = 0;
                    ifile.getline(line, 1024);
                    sscanf(line, "%d", &cellsInDomain);
                    debug4 << mName << "domain " << d << " has " << cellsInDomain << " cells." << endl;
                    if(cellsInDomain >= 1)
                    {
                        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
                        ugrid->Allocate(cellsInDomain);
                        ugrid->SetPoints(pts); // All our grids will share the same points

                        for(int i = 0; i < cellsInDomain; ++i)
                        {
                            int cellid = 0;
                            ifile.getline(line, 1024);
                            sscanf(line, "%d", &cellid);
    
                            if(cellType == 0)
                                ugrid->InsertNextCell(VTK_TETRA, 4, connectivity + cellid * 4);
                            else if(cellType == 1)
                            {
                                vtkIdType *mshconn = connectivity + cellid * 10;
                                // Reorder the connectivity since msh and VTK use different node ordering.
                                conn[0] = mshconn[0];
                                conn[1] = mshconn[1];
                                conn[2] = mshconn[2];
                                conn[3] = mshconn[3];
                                conn[4] = mshconn[4];
                                conn[5] = mshconn[7];
                                conn[6] = mshconn[5];
                                conn[7] = mshconn[6];
                                conn[8] = mshconn[9];
                                conn[9] = mshconn[8];
                                ugrid->InsertNextCell(VTK_QUADRATIC_TETRA, 10, conn);
                            }
                            else if(cellType == 2)
                                ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, connectivity + cellid * 8);
                        }

                        debug4 << mName << "Added domain " << d << endl;
                        domains.push_back(ugrid);
                    }
                }
            }

            delete [] connectivity;
            pts->Delete();
#endif
            retval = true;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: MshData::GetNumDomains
//
// Purpose: 
//   Return the number of domains
//
// Returns:    The number of domains.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 21 15:00:32 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

int
MshData::GetNumDomains() const
{
    return (int)domains.size();
}

// ****************************************************************************
// Method: MshData::GetCellType
//
// Purpose: 
//   Return the cell type
//
// Returns:    The cell type.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 21 15:00:32 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

int
MshData::GetCellType() const
{
    return cellType;
}

// ****************************************************************************
// Method: MshData::ReleaseData()
//
// Purpose: 
//   Releases datasets that have been read.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 21 15:01:19 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
MshData::ReleaseData()
{
    for(size_t i = 0; i < domains.size(); ++i)
        domains[i]->Delete();
    domains.clear();
}

// ****************************************************************************
// Method: MshData::GetMesh
//
// Purpose: 
//   Returns the i'th domain.
//
// Arguments:
//   domain : The domain to return.
//
// Returns:    The dataset for the specified domain.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 21 15:01:40 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
MshData::GetMesh(int domain)
{
    if(domain >= 0 && domain < domains.size())
    {
        domains[domain]->Register(NULL);
        return domains[domain];
    }
    return 0;
}
