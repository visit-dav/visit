#include <avtStructuredDomainNesting.h>
#include <VisItException.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkStructuredGrid.h>

// ****************************************************************************
//  Destructor:  avtStructuredDomainNesting::Destruct
//
//  Programmer:  Hank Childs
//  Creation:    September 25, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Oct  8 14:45:46 PDT 2002
//    Cleaned up stupid mistake where void pointer was being deleted.
//
// ****************************************************************************
void
avtStructuredDomainNesting::Destruct(void *p)
{
    avtStructuredDomainNesting *sdn = (avtStructuredDomainNesting *) p;
    delete sdn;
}

// ****************************************************************************
//  Method: avtStructuredDomainNesting::ApplyGhost
//
//  Purpose:
//    Applies "vtkGhostLevels" array to the meshes passed in 
//
//  Programmer:  Mark C. Miller 
//  Creation:    October 13, 2003
//
// ****************************************************************************
bool
avtStructuredDomainNesting::ApplyGhost(vector<int> domainList,
                                       vector<int> allDomainList,
                                       vector<vtkDataSet*> meshes)
{
    bool didGhost = false;

    for (int i = 0; i < domainList.size(); i++)
    {

        int parentDom = domainList[i];

        vtkUnsignedCharArray *ghostArray = vtkUnsignedCharArray::New();
        int numCells = meshes[i]->GetNumberOfCells();
        ghostArray->SetNumberOfTuples(numCells);

        unsigned char *ghostData = (unsigned char *) ghostArray->GetVoidPointer(0);
        memset(ghostData, 0x0, numCells);

        //
        // Look at each of the children of this domain.
        // If in current selection, ghost the appropriate elements here 
        //
        for (int j = 0; j < domainNesting[parentDom].childDomains.size(); j++)
        {

            bool thisChildIsOn = false;
            for (int k = 0; k < allDomainList.size(); k++)
            {
                if (allDomainList[k] == domainNesting[parentDom].childDomains[j])
                {
                   thisChildIsOn = true;
                   break;
                }
            }

            if (thisChildIsOn)
            {
                int ratio[3];
                int childDom   = domainNesting[parentDom].childDomains[j];
                int childLevel = domainNesting[childDom].level;
                int Ni         = domainNesting[parentDom].logicalExtents[3] -
                                 domainNesting[parentDom].logicalExtents[0] + 1;
                int Nj         = domainNesting[parentDom].logicalExtents[4] -
                                 domainNesting[parentDom].logicalExtents[1] + 1;
                int Nk         = domainNesting[parentDom].logicalExtents[5] -
                                 domainNesting[parentDom].logicalExtents[2] + 1;
                int I0         = domainNesting[parentDom].logicalExtents[0];
                int I1         = domainNesting[parentDom].logicalExtents[3] + 1;
                int J0         = domainNesting[parentDom].logicalExtents[1];
                int J1         = domainNesting[parentDom].logicalExtents[4] + 1;
                int K0         = domainNesting[parentDom].logicalExtents[2];
                int K1         = domainNesting[parentDom].logicalExtents[5] + 1;
                ratio[0]       = levelRatios[childLevel][0];
                ratio[1]       = numDimensions >= 2 ? levelRatios[childLevel][1] : 1;
                ratio[2]       = numDimensions >= 3 ? levelRatios[childLevel][2] : 1;
                int i0         = domainNesting[childDom].logicalExtents[0] / ratio[0];
                int i1         = (domainNesting[childDom].logicalExtents[3] + 1) / ratio[0];
                int j0         = domainNesting[childDom].logicalExtents[1] / ratio[1];
                int j1         = (domainNesting[childDom].logicalExtents[4] + 1) / ratio[1];
                int k0         = domainNesting[childDom].logicalExtents[2] / ratio[2];
                int k1         = (domainNesting[childDom].logicalExtents[5] + 1) / ratio[2];

                //
                // Often a child domain spans multiple parents. So, we need to
                // clip the bounds computed above to the parent's upper bounds
                //
                if (i1 > I1) i1 = I1;
                if (j1 > J1) j1 = J1;
                if (k1 > K1) k1 = K1;
                if (i0 < I0) i0 = I0;
                if (j0 < J0) j0 = J0;
                if (k0 < K0) k0 = K0;

                didGhost = true;

                if (numDimensions == 3)
                {
                    for (int ii = i0; ii < i1; ii++)
                        for (int jj = j0; jj < j1; jj++)
                            for (int kk = k0; kk < k1; kk++)
                            {
                                int a = ii - I0;
                                int b = jj - J0;
                                int c = kk - K0;
                                if ((c*Ni*Nj + b*Ni + a >= numCells) ||
                                   (c*Ni*Nj + b*Ni + a < 0))
                                {
                                   cerr << "bad index value in avtStructuredDomainNesting "
                                        << c << ", " << b << ", " << a << endl;
                                   cerr << "parent = " << i << ", child = " << childDom << endl;
                                   cerr << "I0 = " << I0 << ", I1 = " << I1 << endl;
                                   cerr << "i0 = " << i0 << ", i1 = " << i1 << endl;
                                   cerr << "J0 = " << J0 << ", J1 = " << J1 << endl;
                                   cerr << "j0 = " << j0 << ", j1 = " << j1 << endl;
                                   cerr << "K0 = " << K0 << ", K1 = " << K1 << endl;
                                   cerr << "k0 = " << k0 << ", k1 = " << k1 << endl;
                                   cerr << "numCells = " << numCells << ", index = " << c*Ni*Nj + b*Ni + a << endl;
                                }
                                else
                                {
                                   ghostData[c*Ni*Nj + b*Ni + a] = 0x1;
                                }
                            }
                }
                else if (numDimensions == 2)
                {
                    for (int ii = i0; ii < i1; ii++)
                        for (int jj = j0; jj < j1; jj++)
                        {
                            int a = ii - I0;
                            int b = jj - J0;
                            if ((b*Ni + a >= numCells) ||
                                (b*Ni + a < 0))
                            {
                                cerr << "bad index value in avtStructuredDomainNesting "
                                     << b << ", " << a << endl;
                            }
                            else
                            {
                                ghostData[b*Ni + a] = 0x1;
                            }
                        }
                }
                else
                {
                    for (int ii = i0; ii < i1; ii++)
                    {
                        int a = ii - I0;
                        if ((a >= numCells) ||
                            (a < 0))
                        {
                            cerr << "bad index value in avtStructuredDomainNesting "
                                 << a << endl;
                        }
                        else
                        {
                            ghostData[a] = 0x1;
                        }
                    }
                }
            }
        }

        ghostArray->SetName("vtkGhostLevels");
        meshes[i]->GetCellData()->AddArray(ghostArray);
    }

    return didGhost;

}
