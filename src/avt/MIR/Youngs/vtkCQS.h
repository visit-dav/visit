// Contributed by Thierry Carrard from
// Commissariat a l'Energie Atomique, (CEA)
// BP12, 91297 Arpajon, France


#ifndef VTK_C_Q_S_H
#define VTK_C_Q_S_H

#ifndef SWIG
const static char * VTK_C_Q_S_H_SCCS_ID = "%Z% DSSI/SNEC/LDDC %M%   %I%     %G%";
#endif /*SWIG*/

#include <vtkObjectFactory.h>
#include <vtkSetGet.h>
#include <vtkDataSetAlgorithm.h>

/*!
\class vtkCQS
Detailed description of class vtkCQS ...
*/
class vtkCQS : public vtkDataSetAlgorithm
{
  public:

    static vtkCQS* New();
    vtkTypeRevisionMacro(vtkCQS,vtkDataSetAlgorithm);
    ~vtkCQS ();

  protected:
    vtkCQS ();
    virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  private:
    //! Unimplemented copy constructor
    vtkCQS (const vtkCQS &);

    //! Unimplemented operator
    vtkCQS & operator= (const vtkCQS &);
} ;

#endif /* VTK_C_Q_S_H */
