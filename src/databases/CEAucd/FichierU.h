#ifndef FICHIER_U_H
#define FICHIER_U_H
/*-----------------------------------*
 *                LOVE               *
 *-----------------------------------*/
/*!
\brief Contains definition of class FichierU (component .Kernel)
\author Thierry Carrard
\date 15 Fevrier 2005

Modifications history :
*/

#ifndef SWIG
const static char * FICHIER_U_H_SCCS_ID = "%Z% DSSI/SNEC/LDDC %M%   %I%     %G%";
#endif /*SWIG*/

#include <string>
#include <vector>
#include <fstream>

/*!
\en
FichierU is used to read files named U_XXXXX or U_XXXXX_FEN_XXXXX that are addition to Avs UCD files to store informations like material names, time step values etc.
\_en
\fr
la classes FichierU permet de lire les fichiers complémentaires U_XXXXX ou U_XXXXX_FEN_XXXXX, adjoints aux fichiers UCD en sortie des codes et dans lesquels sont stoqués
les noms des matériaux, les valeurs de temps. Ces fichiers sont générés par l'api GPU et peuvent etre binaire ou ASCII.
\_fr
*/
class FichierU
{
   public:

     /*! \fr
      \param fileName nom du fichier complémentaire
      \param offset emplacement du début du fichier, util dans le cas de collections gme.
      \param taille nombre maximum d'octets que l'on peut lire (evite de lire la suite d'une collection gme si le fichier U est erroné)
      \_fr */
      FichierU( const std::string& fileName, std::ifstream* istr=0, long offset = 0, long size = 0 );

      inline int getNbMat() const { return _nbMat; }
      inline const std::string& getMatName(int i) const { return _matNames[i]; }
      inline float getTime() const { return _time; }
      inline int getNumCycle() const { return  _numCycle; }
      inline int getDim() const { return _dim; }

//! Destructor
      ~FichierU ();

   protected:
      std::string readBinaryString(const char* data, int dataSize, int &ptr);
      int readBinaryInt(const char* data, int dataSize, int &ptr);
      float readBinaryFloat(const char* data, int dataSize, int &ptr);

   private:
      std::vector<std::string> _matNames;
      float _time;
      int _numCycle;
      int _dim;
      int _nbMat;
      int _nbPointData;
      int _nbCellData;
      bool _endian;

      FichierU (const FichierU &);
      FichierU & operator= (const FichierU &);
} ;

#endif /* FICHIER_U_H */
