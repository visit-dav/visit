This directory maintains a "list" of those .py files we run
Pull Request CI on (via Azure). That "list" is implemented
as a bunch of symlinks from this dir to various .py files
we want to be executed during Pull Request CI.

DO NOT ADD .py FILES HERE.

Add only links to existing .py files in other parts of
src/test/tests dir tree here and so so only under the
following conditions...

1) The test typically takes less than 20 seconds to run
   "normally" (e.g. on nightly testing).
2) The test uses only Silo or Blueprint data or no data
   at all.
3) The test tests something significantly different from
   all the other .py files already linked here.

If all of the above conditions are met, the adding a symlink
here to the associated .py file will have the effect of adding
it to the collection of tests performed during Pull Request CI.
