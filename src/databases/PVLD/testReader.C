

#include <chrono>
#include <iostream>

#include <mpi.h>

#include <pvldReader.h>

int myrank,nprocs;
const char h5name[] = "test.pvld";

int main( int argc, char** argv )
{
  MPI_Init( &argc, &argv );
  MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_Comm_size( MPI_COMM_WORLD, &nprocs );

  PVLD_Reader::mpi_comm = MPI_COMM_WORLD;

  PVLD_Reader reader;
  reader.SetFileName( h5name );
  reader.ReadTOC();

  auto t1 = std::chrono::high_resolution_clock::now();
  reader.ReadNodes();
  auto t2 = std::chrono::high_resolution_clock::now();
  auto dd = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
  for( int r=0; r<nprocs; r++ ) {
    if( r==myrank ) 
      std::cout << "ReadNodes() took " << dd << " milliseconds\n";
    MPI_Barrier( MPI_COMM_WORLD );
  }
  if( myrank==0 ) std::cout << "\n\n\n";

  reader.FreeResource();

  vector<int> sft(nprocs+1);
  EquallyPartition( nprocs, reader.GetNumOfNodes(), &sft[0] );

  t1 = std::chrono::high_resolution_clock::now();
  reader.ReadNodes( sft );
  t2 = std::chrono::high_resolution_clock::now();
  dd = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
  for( int r=0; r<nprocs; r++ ) {
    if( r==myrank ) 
      std::cout << "ReadNodes() took " << dd << " milliseconds\n";
    MPI_Barrier( MPI_COMM_WORLD );
  }


  MPI_Finalize();
  return 0;
}
