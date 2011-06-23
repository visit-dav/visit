#include "CubeReader.h"

#include <InvalidFilesException.h>


CubeReader::CubeReader(const char* fname) {

  // default units are in angstorms..
  atom_units = 1.0;
  grid_units = 1.0;

  dx[0]=dx[1]=dx[2]=0.0;
  dy[0]=dy[1]=dy[2]=0.0;
  dz[0]=dz[1]=dz[2]=0.0;
  
  isSheared = false;
  orbitals.clear();
  
  filename = string(fname);
  file.open(fname);
  if (!file){
     EXCEPTION1(InvalidFilesException, "Fild could not be opened.");
  }else{
     readMetaData(true); //read meta data and keep the file open
     file.close();
  }
}

CubeReader::~CubeReader() {

}

bool CubeReader::isGridSheared() const{
  return isSheared;
}

bool CubeReader::isFileValid() const{
    return (errorString.length()==0);
}

string CubeReader::getErrorString() const{
    return errorString;
}

void CubeReader::readMetaData(bool keepFileOpen) {

  //Open the file or if the file is already open, jump to the beginning of the file
  if( !file.is_open() ){
      file.open( (char*) filename.c_str() );
  }else if( file.tellg() >0 ){
      file.seekg( 0 , ios::beg );
  }
  errorString="";
  
  //1. line
  //This is here used as an additional (non-standard) check 
  //for orbitals. Another check for the standard-compliant definition of orbitals is done
  //at the end of this function
  string header;
  getline(file, header);
  if (header.find("orbital")!=string::npos) {// there are orbitals in this file
    string keyword;
    int norbitals;
    // first get how many orbitals followed by orbital number
    stringstream ss(header);
    ss>>keyword>>norbitals;

    cout<<"Found "<<norbitals<<" orbitals"<<endl;
    for (int o=0;o<norbitals; o++) {
      int orbital_num;
      ss>>orbital_num;
      orbitals.push_back(orbital_num);
      cout<<"        "<<orbital_num<<endl;
    }
  }

   //1. line Skip the second line as this is an unused comment
  getline(file, header);

  //3. line : Read the number of atoms and the grid origin. 
  getline( file , header);
  vector<string> tokens;
  istringstream iss(header);
  copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter<vector<string> >(tokens));
  if( tokens.size() >= 4 ){
    natoms   = atoi( tokens[0].c_str() );
    x_origin = atof( tokens[1].c_str() );
    y_origin = atof( tokens[2].c_str() );
    z_origin = atof( tokens[3].c_str() );
  }else{
     errorString = "Cube reader: Error in line 3. Too few entries to determine number of atoms and origin (expected 4).";
     return;
  }
  
  //file>>natoms>>x_origin>>y_origin>>z_origin;
  //Decide based on the sign of the number of atoms whether units are in bohr or angstrom
  if (natoms<0) {
    natoms = -1 * natoms;
    SetAtomUnitsToBohr();
    cout<<"Setting atom units to bohr"<<endl;
  }
    
  cout<<"There are "<<natoms<<" atoms "<<endl;
  cout<<"Grid starts at "<<x_origin
      <<", "<<y_origin
      <<", "<<z_origin
      <<endl;

  //4. line: Read the x grid
  float t1, t2;
  getline( file , header);
  tokens.clear();
  iss.clear();
  iss.str(header);
  copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter<vector<string> >(tokens));
  if( tokens.size() >= 4 ){
      x_size = atoi( tokens[0].c_str() );
      dx[0]  = atof( tokens[1].c_str() );
      dx[1]  = atof( tokens[2].c_str() );
      dx[2]  = atof( tokens[3].c_str() );
  }else{
     errorString = "Cube reader: Error in line 4: Too few entries to determine x grid (expected 4).";
     return;
  }
  if (x_size<0){
    x_size = -1 * x_size;
    SetGridUnitsToBohr();
    cout<<"Setting grid units to bohr"<<endl;
  }
  
  if ((dx[1]!=0)||(dx[2]!=0)) 
    isSheared = true;
  
  //5. line: Read the y grid
  getline( file , header);
  tokens.clear();
  iss.clear();
  iss.str(header);
  copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter<vector<string> >(tokens));
  if( tokens.size() >= 4 ){
      y_size = atoi( tokens[0].c_str() );
      dy[0]  = atof( tokens[1].c_str() );
      dy[1]  = atof( tokens[2].c_str() );
      dy[2]  = atof( tokens[3].c_str() );
  }else{
     errorString = "Cube reader: Error in line 5: Too few entries to determine y grid (expected 4).";
     return;
  }
  if (y_size<0){
    y_size = -1 * y_size;
    SetGridUnitsToBohr();
    cout<<"Setting grid units to bohr"<<endl;
  }
  if ((dy[0]!=0)||(dy[2]!=0)) 
    isSheared = true;

  //6. line: Read the z grid
  getline( file , header);
  tokens.clear();
  iss.clear();
  iss.str(header);
  copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter<vector<string> >(tokens));
  if( tokens.size() >= 4 ){
      z_size = atoi( tokens[0].c_str() );
      dz[0]  = atof( tokens[1].c_str() );
      dz[1]  = atof( tokens[2].c_str() );
      dz[2]  = atof( tokens[3].c_str() );
  }else{
     errorString = "Cube reader: Error in line 6: Too few entries to determine x grid (expected 4).";
     return;
  }
  if (z_size<0){
    z_size = -1 * z_size;
    SetGridUnitsToBohr();
    cout<<"Setting grid units to bohr"<<endl;
  }
  if ((dz[0]!=0)||(dz[1]!=0)) 
    isSheared = true;
  
  cout<<"Grid size is "<<x_size<<"x"
      <<y_size<<"x"
      <<z_size<<endl;
  
  cout<<"Step size is"
      <<"("<<dx[0]<<","<<dx[1]<<","<<dx[2]<<") x "
      <<"("<<dy[0]<<","<<dy[1]<<","<<dy[2]<<") x "
      <<"("<<dz[0]<<","<<dz[1]<<","<<dz[2]<<")"
      <<endl;
  
  //Line 7 to 7+natoms : Read for each atom its type, isotope and x ,y , z location
  atom_types.clear();
  atom_locations.clear();

  for (int i=0; i<natoms; i++) {
    float x,y,z;
    float isotope;
    
    getline( file , header);
    tokens.clear();
    iss.clear();
    iss.str(header);
    copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter<vector<string> >(tokens));
    if( tokens.size() >= 5 ){
      atom_types.push_back( atoi( tokens[0].c_str() ) );
      isotope = atof( tokens[1].c_str() );
      atom_locations.push_back( atof( tokens[2].c_str() ) );
      atom_locations.push_back( atof( tokens[3].c_str() ) );
      atom_locations.push_back( atof( tokens[4].c_str() ) );
    }else{
      errorString = "Cube reader: Error while parsing atoms. At least one of the atoms is not defined completely (expected 5 entried per atom).";
      return;
    }
  }

  cout<<"determined shear to be " << isSheared<<endl;
  cout<<"Atom type array has "<<atom_types.size()<<" entries"<<endl;
  cout<<"Atom locations array has "<<atom_locations.size()<<" entries"<<endl;
 
  //Check whether the file contains orbitals
  streampos endHeaderPos = file.tellg(); //save the current location
  getline(file, header);
  //Check if the next line contains only integer values
  if( !( (header.find(".")!=string::npos) || (header.find("E")!=string::npos) ) )  
  {
    tokens.clear();
    iss.clear();
    iss.str(header);
    copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter<vector<string> >(tokens));
    int norbitals;
    norbitals = atoi(tokens[0].c_str() );
    if( tokens.size() < (norbitals+1) ){
        errorString = "Cube reader: Number of orbitals does not match the number of orbitals given.";
        return;
    }
    cout<<"Found "<<norbitals<<" orbitals"<<endl;
    for (int o=1;o<=norbitals; o++) {
      orbitals.push_back( atoi( tokens[o].c_str() ) );
    cout<<"        "<<orbitals[o-1]<<endl;
    }
  }else{
    //Go back to the real ending of the header
    file.seekg( endHeaderPos );
  }
  
  //now the file pointer is at the beginning of the grid data
  
  //Close the file if the caller did not explicitly request to keep the file open
  if( !keepFileOpen ){
     file.close();
  }
  cout<<"---------------------------------------"<<endl;
}


int CubeReader::GetNAtoms() {
  return natoms;
}

int CubeReader::GetNTimesteps() {
  return 1;
}

void CubeReader::GetGridSize(int &xs, int &ys, int &zs) {
  xs = x_size;
  ys = y_size;
  zs = z_size;
}

// 1 Bohr =   0.529177 A
// 1 A    = 1/0.529177
void CubeReader::SetAtomUnitsToBohr() {
  atom_units = (1.0/0.529177)*atom_units;
}

void CubeReader::SetGridUnitsToBohr() {
  grid_units = (1.0/0.529177)*grid_units;
}

// assume that arrays have already been sized properly, just fill them
// in with co-ordinates
void CubeReader::GetGridLocations(float* x, float* y, float* z) {
  
  for (int i=0; i<x_size; i++) 
    x[i] = x_origin + i*dx[0]*grid_units;
  
  for (int j=0; j<y_size; j++)
    y[j] = y_origin + j*dy[1]*grid_units;

  for (int k=0; k<z_size; k++)
    z[k] = z_origin + k*dz[2]*grid_units;
  
}

// assume that arrays have already been sized properly, just fill them
// in with sheared co-ordinates
void CubeReader::GetShearedGridLocations(float* x, float* y, float* z) {
  
  int count = 0;
  for (int k=0; k<z_size; k++) 
    for (int j=0; j<y_size; j++)
      for (int i=0; i<x_size; i++) {
        float new_x,new_y,new_z;
        new_x = new_y = new_z = 0.0;

        // first take contribution from (x_origin,0,0) and dx vector
        new_x += x_origin + i*dx[0]*grid_units;
        new_y += 0        + i*dx[1]*grid_units;
        new_z += 0        + i*dx[2]*grid_units;

        // then take contribution from (0, y_origin, 0) and dy vector
        new_x += 0        + j*dy[0]*grid_units;
        new_y += y_origin + j*dy[1]*grid_units;
        new_z += 0        + j*dy[2]*grid_units;

        // finally take contribution from (0,0,z_origin) and dz vector
        new_x += 0        + k*dz[0]*grid_units;
        new_y += 0        + k*dz[1]*grid_units;
        new_z += z_origin + k*dz[2]*grid_units;

        // that's it!
        x[count] = new_x;
        y[count] = new_y;
        z[count] = new_z;

        count++;
      }  
}

int CubeReader::GetNumOrbitals() {
  return orbitals.size();
}

int CubeReader::GetOrbitalNumber(int i) {
  return orbitals[i];
}

void CubeReader::GetOrbitalValues(float* vals, const char* varname) {
  
  cout<<"GetOrbitalValues called for "<<varname<<endl;
  string var = string(varname);
  // first figure out what orbital offset corresponds to this varname
  string keyword = "orbital_";
  size_t found_k = var.find(keyword.c_str(),0,keyword.length());
  string num_str = var.substr(keyword.length(), var.length()-keyword.length());
  cout<<"num_str = "<<num_str;
  int orbital_num = atoi(num_str.c_str());
  cout<<"determined target orbital_num to be "<<orbital_num<<endl;


  int index = -1;
  
  for (int i=0; i<orbitals.size(); i++) {
    if (orbitals[i]==orbital_num){
      index = i;
      cout<<"FOUND index = "<<index<<endl;
    }
  }
  
  if (index ==-1) {
    cout<<"WARNING: could not find orbital"<<endl;
    return;
  }

  // then start reading the file, taking care to read entries
  // corresponding to the offset
  FILE *file;
  file = fopen(filename.c_str(),"r");
  // Check if the open file branch was successful
  if (!file){
     EXCEPTION1(InvalidFilesException, "Fild could not be opened.");
  }

  
  char header[1024];
  for (int i=0; i<6; i++)
    fgets(header, 1024, file);

  char atoms[1024];
  for (int i=0; i<natoms; i++)
    fgets(atoms, 1024, file);
  
  int count = 0;
  int dest_index;

  float val= 99;
  cout<<"         Done with headers, now trying to read the data"<<endl;
  cout<<"                 size="<<x_size<<"x"<<y_size<<"x"<<z_size<<endl;

  int norbitals = orbitals.size();
  cout<<"                 norbitals = "<<norbitals<<endl;
  
  for (int i=0; i<x_size; i++) {
    for (int j=0; j<y_size; j++) {
      for (int k=0; k<z_size; k++) {
        for (int o=0; o<norbitals; o++) {
            fscanf(file, "%E", &val);
            
            if (o==index) {// if the offset matches
            dest_index = i + (j)*x_size + (k)*x_size*y_size;
            
            vals[dest_index] = val;
            count++;
            }

        } // orbital
      }//k
    }//j
  }//i

  cout<<"        done with reading data, closing file"<<endl;
  fclose(file);  
}


void CubeReader::GetAtomTypes(float *types) {
  for (int i=0; i<atom_types.size(); i++)
    types[i] = (float)atom_types[i];
  
}

void CubeReader::GetAtomLocations(vector<float> &locations) {

  for (int i=0; i<atom_locations.size(); i++)
    locations[i] = atom_locations[i]*atom_units;
  
}

// fill in electron density values, one per-grid-location
// array has already been allocated 

void CubeReader::GetGridValues(float* vals) {
  
  FILE *file;
  file = fopen(filename.c_str(),"r");
  if (!file){
     EXCEPTION1(InvalidFilesException, "Fild could not be opened.");
  }
    
  char header[1024];
  for (int i=0; i<6; i++)
    fgets(header, 1024, file);

  char atoms[1024];
  for (int i=0; i<natoms; i++)
    fgets(atoms, 1024, file);
  
  int count = 0;
  int dest_index;
  
  float val= 99;
  cout<<"         Done with headers, now trying to read the data"<<endl;
  cout<<"                 size="<<x_size<<"x"<<y_size<<"x"<<z_size<<endl;

  for (int i=0; i<x_size; i++) {
    for (int j=0; j<y_size; j++) {
      for (int k=0; k<z_size; k++) {
        // the input values are written w/ z being fastest index and x
        // being slowest; visit is exactly the opposite
        fscanf(file, "%E", &val);

        dest_index = i + (j)*x_size + (k)*x_size*y_size;

        vals[dest_index] = val;
        count++;
      }
    }
    //cout<<count<<endl;
  }
  cout<<"        done with reading data, closing file"<<endl;
  fclose(file);
}
