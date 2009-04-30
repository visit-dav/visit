#include "paradis.h"
#include "Prefs.h"
#include "RC_c_lib/args.h"
paraDIS::DataSet *gDataSet = NULL; 

void usage(void) {
  fprintf(stderr, "=========================================================\n"); 
  fprintf(stderr, "usage:  analyzeParaDIS [options] inputfile\n"); 
  fprintf(stderr, "---------------------------------------\n"); 
  fprintf(stderr, "options:  \n"); 
  fprintf(stderr, " -debugfiles:  dump out detailed analysis in files\n"); 
  fprintf(stderr, " -v (or -verbose) num:  set verbosity level to num (scale of 0-5, 5 is very verbose, 0 is strangely quiet)\n"); 
  return; 
}

int main(int argc, char *argv[]) {

  try {
    long debugfiles = 0, verbosity=0, help=false; 
    argt args[7] = {
      {BOOL_TYPE, "-debugfiles", 1, &debugfiles}, 
      {BOOL_TYPE, "-help", 1, &help}, 
      {LONG_TYPE, "-v", 1, &verbosity}, 
      {LONG_TYPE, "-verbose", 1, &verbosity}, 
    };
    arg_expect_args(args, 3);
    arg_ignore_bad_args(1); 
    if (!arg_parse_args(&argc, argv)) {
      fprintf(stderr, "****************************************\n"); 
      fprintf(stderr, "Error in args_parse_args\n"); 
      usage(); 
      exit(1); 
    }
    if (help) {
      usage(); 
      exit(0); 
    }
    if (argc < 2) {
      fprintf(stderr, "Error:  need a filename\n"); 
      usage(); 
      exit(1); 
    }
    
    gDataSet = new paraDIS::DataSet; 
    
    if (verbosity) {
      gDataSet->SetVerbosity(verbosity, "analyzeParaDIS-debug.txt"); 
    }
    
    gDataSet->SetDataFile(argv[1]); 
    
    gDataSet->EnableDebugOutput(debugfiles); 

    gDataSet->ReadData(); 
    
    gDataSet->PrintArmStats();

  } catch (string err) {
    cerr << "Error: " << err << endl; 
  }
  return 0; 
}
