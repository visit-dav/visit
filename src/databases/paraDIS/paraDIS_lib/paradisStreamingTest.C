/* just a file to contain a main() for testing paraDIS library */ 
#include "paradisStreaming.h"
#include "RC_c_lib/debugutil.h"
#include "RC_c_lib/args.h"

using namespace std; 

int main(int argc, char *argv[]) {
  long restrictBounds = 0, debugfiles = 0, verbosity=4; 
  long procnum = 0, numprocs = 1; 
  char inputfile[2048]=""; 
  argt args[7] = {
    {BOOL_TYPE, "-restrict", 1, &restrictBounds}, 
    {BOOL_TYPE, "-debugfiles", 1, &debugfiles}, 
    {LONG_TYPE, "-v", 1, &verbosity}, 
    {LONG_TYPE, "-procnum", 1, &procnum}, 
    {LONG_TYPE, "-numprocs", 1, &numprocs}, 
    {LONG_TYPE, "-verbose", 1, &verbosity}, 
    {STRING_TYPE, "-in", 2048, inputfile}
  };
  arg_expect_args(args, 7);
  arg_ignore_bad_args(1); 
  arg_parse_args(&argc, argv); 
  
  dbg_setverbose(verbosity); 
  
  try {
    rclib::Point<float> datamin, datamax; 
    paraDIS::ParadisDumpFile df; 
    
    char *datafile = "../paraDIS_data/rs0443.data";
    if (*inputfile) datafile = inputfile; 
    df.SetDumpFile(datafile); 

    /*    dsData.SetProcNum(procnum,numprocs); 
    dsData.SetDataFile(datafile); 
    dsData.EnableDebugOutput(debugfiles); 
    dsData.SetDebugOutputDir("debuginfo"); 
    dsData.GetBounds(datamin, datamax, datafile); 
    */ 
    /*!
      restrict the bounds for fun
    */
    /* 
    if (restrictBounds) {
      dsData.TestRestrictSubspace(); 
    }
    dsData.ReadData(); 
    */ 
    df.ProcessDumpFile(); 


  } catch (string err) {
    cerr <<"Error in main: "<<err<<endl; 
  }
  
  return 0; 
}
