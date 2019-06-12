#!/bin/sh

top_level=`git rev-parse --show-toplevel`
str_functions_rst="src/doc/cli_manual/functions.rst"
str_method_doc_C="src/visitpy/common/MethodDoc.C"
str_method_doc_h="src/visitpy/common/MethodDoc.h"
method_doc_staged=false

# Look to see if functions.rst is staged. If it is, then run function_to_method_doc and look for
# changes in MethodDoc.h/C. Enforce coupling between staged changes to functions.rst and resulting
# changes in MethodDoc.h/C
for staged in $(git diff --cached --name-only)
do
  if [ $staged == $str_functions_rst ]; then
    method_doc_staged=true
    $top_level/src/doc/functions_to_method_doc.py $top_level
      
    mdc_staged=false
    mdh_staged=false
    for unstaged in $(git diff --name-only)
    do
      # functions.rst should not have staged and unstaged changes because the
      # MethodDoc.h/C will not correlate with the commiting functions.rst
      if [ $unstaged == $str_functions_rst ]; then
        echo "ERROR: Cannot have both staged and unstaged changes to functions.rst."
        echo "Please stage all changes to functions.rst or stash unstaged changes."
        exit 1
      elif [ $unstaged == $str_method_doc_C ]; then
        mdc_staged=true
      elif [ $unstaged == $str_method_doc_h ]; then
        mdh_staged=true
      fi
    done
    
    # Output error messages and exit
    if [ $mdc_staged == true ] && [ $mdh_staged == true ]; then
      echo "ERROR: Your changes to functions.rst will modify MethodDoc.h and MethodDoc.C, but you are not staging those."
      echo "Please also stage MethodDoc.h and MethodDoc.C and try again."
      exit 1
    elif [ $mdc_staged == true ]; then
      echo "ERROR: Your changes to functions.rst will modify MethodDoc.C, but you are not staging it."
      echo "Please also stage MethodDoc.C and try again."
      exit 1
          
    elif [ $mdh_staged == true ]; then
      echo "ERROR: Your changes to functions.rst will modify MethodDoc.h, but you are not staging it."
      echo "Please also stage MethodDoc.h and try again."
      exit 1
    fi
    break
  fi
done

# If functions.rst is not staged, then look for staged changes to MethodDoc.h/C.
# Reject these because this file should not be manually editted.
if [ $method_doc_staged == false ]; then
  mdc_staged=false
  mdh_staged=false
  for staged in $(git diff --cached --name-only)
  do
    if [ $staged == $str_method_doc_C ] || [ $staged == $str_method_doc_h ]; then
      echo "ERROR: MethodDoc.h and MethodDoc.C are auto-generated files and should not be manually editted because all edits will be overwritten."
      echo "Please make changes to src/doc/cli_manual/functions.rst and run src/doc/functions_to_method_doc.py instead."
      exit 1
    fi
  done
fi
