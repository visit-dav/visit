/*-----------------------------------*
*                LOVE               *
*-----------------------------------*/
/*!
 \brief Implementation of class FichierU (component .Kernel)
 \author Thierry Carrard CEA/DAM
 \date 15 Fevrier 2005

 Modifications history :
 */

#define VERBOSE_DEBUG

const static char * FICHIER_U__C_SCCS_ID = "%Z% DSSI/SNEC/LDDC %M%   %I%     %G%";

#include "FichierU.h"

#include <string.h>
#include <assert.h>

#include <string>
#include <fstream>
#include <iostream>
using namespace std;


#define GPU_ID_STRING "GPU_UCD_COMPLEMENTAIRE_2004"
#define GPU_ID_STRING_LEN (strlen(GPU_ID_STRING))

// DA: Attention: le DEBUG_OUT peut etre TRES VERBEUX dans le cas ou
// il y a un grand nombre de materiaux, de domaines et de processeurs.
// => plusieurs centaines de milliers de lignes

#ifdef VERBOSE_DEBUG
#define DEBUG_OUT(x) cout x; cout.flush()
#else
#define DEBUG_OUT(x) (void)0
#endif

#ifdef DEBUG
#define BAD_FIELD(msg) { DEBUG_OUT(<<"\nUnrecognized token "<<field<<msg<<endl); ofstream dump("/tmp/carrardx/U_dump",ios::binary); dump.write(data,fsize); dump.close(); } assert(false)
#else
#define BAD_FIELD(msg)
#endif

/*!
 The default constructor
 \sa ~FichierU()
 */
FichierU::FichierU(const string& fileName, ifstream* istr, long offset, long taille)
   : _time(0),
     _numCycle(0),
     _dim(0),
     _nbPointData(0),
     _nbCellData(0),
     _endian(false)
{
   bool closeFile = false;
   if( istr == 0 )
   {
      istr = new ifstream( fileName.c_str(), ios::in | ios::binary );
      closeFile = true;
   }

   ifstream& ufic = *istr;
   if(ufic.good())
   {
      ufic.seekg(offset, ios::beg);
      char c;
      ufic.read(&c,1);
      
      switch(c)
      {
        case 0x07 :
        {
           DEBUG_OUT(<<"format binaire #"<<(void*)(int)c<<endl);
           long fsize;
           if (taille != 0)
           {
              fsize = taille;
           }
           else
           {
              ufic.seekg(0, ios::end);
              fsize = ufic.tellg();
              fsize -= offset;
              fsize -= 1;
           }

           DEBUG_OUT(<<"lecture de "<<fsize<<" octets"<<endl);
           char * data = new char[fsize+1];
           ufic.seekg(offset+1, ios::beg);
           ufic.read(data,fsize);
           data[fsize] = '\0';

           if( fsize>=GPU_ID_STRING_LEN && strncmp(data,GPU_ID_STRING,GPU_ID_STRING_LEN)==0 )
           {
              DEBUG_OUT(<<"nouveau format binaire"<<endl);

              int ptr = 0;
              ptr += GPU_ID_STRING_LEN;

              char* eof = strstr(data+ptr,"FIN_FICHIER");
              if( eof!=0 )
              {
                 int diff = (eof+11) - data;
                 DEBUG_OUT(<<"diff = "<<diff<<endl);
              }

              bool endOfData = false;
              while( ptr<fsize && !endOfData )
              {
                 string field = readBinaryString(data,fsize,ptr);
                 DEBUG_OUT(<<field);
                 if(field=="NUMERO_CYCLE")
                 {
                    _numCycle = readBinaryInt(data,fsize,ptr);
                    DEBUG_OUT(<<"="<<_numCycle<<endl);
                 }
                 else if(field=="TEMPS_PHYSIQUE")
                 {
                    _time = readBinaryFloat(data,fsize,ptr);
                    DEBUG_OUT(<<"="<<_time<<endl);
                 }
                 else if(field=="DIMENSION")
                 {
                    _dim = readBinaryInt(data,fsize,ptr);
                    DEBUG_OUT(<<"="<<_dim<<endl);
                 }
                 else if(field=="NB_GD_NODALES")
                 {
                    _nbPointData = readBinaryInt(data,fsize,ptr);
                    DEBUG_OUT(<<"="<<_nbPointData<<endl);
                 }
                 else if(field=="NB_GD_CENTREES")
                 {
                    _nbCellData = readBinaryInt(data,fsize,ptr);
                    DEBUG_OUT(<<"="<<_nbCellData<<endl);
                 }
                 else if(field=="DEBUT_OBJET")
                 {
                    DEBUG_OUT(<<endl);
                    bool endOfMatList = false;
                    _nbMat = -1;
                    _matNames.clear();
                    while( ptr<fsize && !endOfMatList )
                    {
                       field = readBinaryString(data,fsize,ptr);
                       DEBUG_OUT(<<'\t'<<field);
                       if(field=="NB_OBJETS")
                       {
                          _nbMat = readBinaryInt(data,fsize,ptr);
                          DEBUG_OUT(<<"="<<_nbMat<<endl);
                       }
                       else if(field=="OBJET")
                       {
                          bool endOfMat = false;
                          while( ptr<fsize && !endOfMat )
                          {
                             field = readBinaryString(data,fsize,ptr);
                             if(field=="NOM")
                             {
                                _matNames.push_back(readBinaryString(data,fsize,ptr));
                                endOfMat = true;
                                DEBUG_OUT(<<" "<<_matNames.size()<<" = "<<_matNames.back()<<endl);
                             }
                             else { BAD_FIELD(" au lieu de NOM"); }
                          }
                       }
                       else if(field=="FIN_OBJET")
                       {
                          endOfMatList = true;
                          DEBUG_OUT(<<endl);
                       }
                       else { BAD_FIELD(" in object list"); }                    
                    }
                    if(_nbMat==-1) _nbMat = _matNames.size();
                    else if( _nbMat!=_matNames.size() )
                    {
                       cerr<<"Attention! "<<_nbMat<<"materiaux annoncés, mais seulement "<<_matNames.size()<<" décris"<<endl;
                    }
                 }
                 else if(field=="EXIST_GD_CENTREES")
                 {
                    int n = readBinaryInt(data,fsize,ptr);
                    DEBUG_OUT(<<"="<<n<<endl);
                    for(int i=0;i<n;i++)
                    {
                       readBinaryInt(data,fsize,ptr);
                    }
                 }
                 else if(field=="EXIST_GD_NODALES")
                 {
                    int n = readBinaryInt(data,fsize,ptr);
                    DEBUG_OUT(<<"="<<n<<endl);
                    for(int i=0;i<n;i++)
                    {
                       readBinaryInt(data,fsize,ptr);
                    }
                 }
                 else if(field=="DEBUT_GD_ENTIERES")
                 {
                    DEBUG_OUT(<<endl);
                    field = readBinaryString(data,fsize,ptr);
                    if(field=="NB_GD_ENTIERES")
                    {
                       int n = readBinaryInt(data,fsize,ptr);
                       DEBUG_OUT(<<"\tNB_GD_ENTIERES="<<n<<endl);
                       for(int i=0;i<n;i++)
                       {
                          string nom;
                          int typeGd=0, nbObj=0, indice=0, valeurs=0;
                          field = readBinaryString(data,fsize,ptr);
                          if(field=="GRANDEUR")
                          {
                             nom = readBinaryString(data,fsize,ptr);
                             DEBUG_OUT(<<"\tGRANDEUR="<<nom);
                          }
                          else { BAD_FIELD(" au lieu de GRANDEUR"); }

                          field = readBinaryString(data,fsize,ptr);
                          if(field=="TYPE_GD")
                          {
                             typeGd = readBinaryInt(data,fsize,ptr);
                             DEBUG_OUT(<<", TYPE_GD="<<typeGd);
                          }
                          else { BAD_FIELD(" au lieu de TYPE_GD"); }

                          field = readBinaryString(data,fsize,ptr);
                          if(field=="NB_OBJETS_GD")
                          {
                             nbObj = readBinaryInt(data,fsize,ptr);
                             DEBUG_OUT(<<", NB_OBJETS_GD="<<nbObj<<endl);
                          }
                          else { BAD_FIELD(" au lieu de NB_OBJETS_GD"); }
                          
                          for(int j=0;j<nbObj;j++)
                          {
                             field = readBinaryString(data,fsize,ptr);
                             if(field=="INDICE_OBJET_GD")
                             {
                                indice = readBinaryInt(data,fsize,ptr);
                                DEBUG_OUT(<<"\t\tINDICE_OBJET_GD="<<indice);
                             }
                             else { BAD_FIELD(" au lieu de INDICE_OBJET_GD"); }
                             
                             field = readBinaryString(data,fsize,ptr);
                             if(field=="VALEURS")
                             {
                                valeurs = readBinaryInt(data,fsize,ptr);
                                DEBUG_OUT(<<", VALEURS="<<valeurs<<endl);
                                for(int k=0;k<valeurs;k++)
                                {
                                   readBinaryInt(data,fsize,ptr);
                                }
                             }
                             else { BAD_FIELD(" au lieu de VALEURS"); }
                          }
                          
                       }
                    }
                    else { BAD_FIELD(" au lieu de NB_GD_ENTIERES"); }

                    field = readBinaryString(data,fsize,ptr);
                    if( field == "FIN_GD_ENTIERES")
                    {
                       DEBUG_OUT(<<field<<endl);
                    }
                    else { BAD_FIELD(" au lieu de FIN_GD_ENTIERES"); }

                 }
                 else if(field=="NB_FENETRES")
                 {
                    int n = readBinaryInt(data,fsize,ptr);
                    DEBUG_OUT(<<"="<<n<<endl);
                 }
                 else if(field=="SYM_OX")
                 {
                    int n = readBinaryInt(data,fsize,ptr);
                    DEBUG_OUT(<<"="<<n<<endl);
                 }
                 else if(field=="SYM_OY")
                 {
                    int n = readBinaryInt(data,fsize,ptr);
                    DEBUG_OUT(<<"="<<n<<endl);
                 }
                 else if(field=="SYM_OZ")
                 {
                    int n = readBinaryInt(data,fsize,ptr);
                    DEBUG_OUT(<<"="<<n<<endl);
                 }
                 else if(field=="FIN_FICHIER")
                 {
                    endOfData = true;
                    DEBUG_OUT(<<endl);
                 }
                 else { BAD_FIELD(" at global scope"); }
              }
           }
           else
           {
              DEBUG_OUT(<<"ancien format binaire"<<endl);
              char *numCycle = data+4*0;
              char *time     = data+4*1;
              char *dim      = data+4*2;
              char *nbMat    = data+4*3;
              char *ptr      = data+4*4;

              _endian = false;
              _dim = dim[0] | dim[1]<<8 | dim[2]<<16 | dim[3]<<24;
              if(_dim<0 || _dim>3)
              {
                 _numCycle = numCycle[3] | numCycle[2]<<8 | numCycle[1]<<16 | numCycle[0]<<24;
                 { char* t = (char*) &_time; for(int i=0;i<4;i++) t[i] = time[3-i]; }
                 _dim = dim[3] | dim[2]<<8 | dim[1]<<16 | dim[0]<<24;
                 _nbMat = nbMat[3] | nbMat[2]<<8 | nbMat[1]<<16 | nbMat[0]<<24;
              }
              else
              {
                 _endian=true;
                 _numCycle = numCycle[0] | numCycle[1]<<8 | numCycle[2]<<16 | numCycle[3]<<24;
                 { char* t = (char*) &_time; for(int i=0;i<4;i++) t[i] = time[i]; }
                 _nbMat = nbMat[0] | nbMat[1]<<8 | nbMat[2]<<16 | nbMat[3]<<24;
              }
         
              _matNames.resize(_nbMat);
              int cursor = 0;
              for(int m=0;m<_nbMat;m++)
              {
                 _matNames[m]=readBinaryString(ptr,fsize,cursor);
              }
           }
           delete [] data;
        }
        break;

        default :
        {
          //cerr<<"format inconnu #"<<(void*)c<<" donc lecture ASCII"<<endl;
           ufic.close();
           ufic.open(fileName.c_str());
           ufic>>_numCycle;
           ufic>>_time;
           ufic>>_dim;
           ufic>>_nbMat;
        }
        break;
      }

   }
   else
   {
cerr << "In else" << endl;
      _numCycle = 0;
      _time = 0;
      _dim = 0;
      _nbMat = 0;
   }

   if( closeFile )
   {
      istr->close();
      delete istr;
   }
}

string FichierU::readBinaryString(const char* data, int dataSize, int &ptr)
{
   int len;

   if( (ptr+4) > dataSize )
   {
      DEBUG_OUT(<<"FichierU::readBinaryString : fin de fichier prematuree"<<endl);
      ptr = dataSize;
      return string();
   }

   unsigned char * buf = (unsigned char *) (data+ptr);
   ptr+=4;

   if(_endian) len = buf[0] | buf[1]<<8 | buf[2]<<16 | buf[3]<<24;
   else len = buf[3] | buf[2]<<8 | buf[1]<<16 | buf[0]<<24;
   if(len<0 || len>32768)
   {
      DEBUG_OUT(<<" (len="<<len);
      _endian = !_endian;
      if(_endian) len = buf[0] | buf[1]<<8 | buf[2]<<16 | buf[3]<<24;
      else len = buf[3] | buf[2]<<8 | buf[1]<<16 | buf[0]<<24;
      DEBUG_OUT(<<" -> "<<len<<") ");
   }

   if( (ptr+len) > dataSize )
   {
      DEBUG_OUT(<<"FichierU::readBinaryString : fin de fichier prematuree"<<endl);
      len = dataSize-ptr;
   }

   ptr+=len;
   return string(data+ptr-len,len);
}

int FichierU::readBinaryInt(const char* data, int dataSize, int &ptr)
{
   int n;
   if( (ptr+4) > dataSize )
   {
      DEBUG_OUT(<<"FichierU::readBinaryInt : fin de fichier prematuree"<<endl);
      ptr = dataSize;
      return 0;
   }

   const unsigned char * buf = (const unsigned char *) (data+ptr);
   ptr+=4;

   if(_endian) n = buf[0] | buf[1]<<8 | buf[2]<<16 | buf[3]<<24;
   else n = buf[3] | buf[2]<<8 | buf[1]<<16 | buf[0]<<24;
   return n;
}

float FichierU::readBinaryFloat(const char* data, int dataSize, int &ptr)
{
   if( (ptr+4) > dataSize )
   {
      DEBUG_OUT(<<"FichierU::readBinaryInt : fin de fichier prematuree"<<endl);
      ptr = dataSize;
      return 0;
   }

   const unsigned char * buf = (const unsigned char *) (data+ptr);
   ptr+=4;

   if(!_endian)
   {
      char tmp[4];
      tmp[0] = buf[3];
      tmp[1] = buf[2];
      tmp[2] = buf[1];
      tmp[3] = buf[0];
      return *(float*)(tmp);
   }
   else
   {
      return *(float*)(buf);
   }
}


/*!
 The destrcutor
 \sa FichierU()
 */
FichierU::~FichierU()
{
}
