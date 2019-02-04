# Adding data for new tests

Sometimes new data files need to be added to support the new tests.
Doing this involves adding either an entirely new data 7z archive or
adding a new file to an existing 7z archive. All the data archives
are named more or less for the data format(s) for which they contain
data.

For more details on dealing with 7z, run `cmake` in the visit-dav/data
and then `make help`.

# Adding new tests: 

  * Add code to an existing `.py` file or create a new `.py` file copying
  the basic format of an existing one including boilerplat calls to functions
  like `TurnOffAllAnnotations()` and `data_path()` when opening a database file
  and `Exit()` when terminating a test.
  * If adding a new `.py` file, be careful to use the correct
  _category_ directory. To see existing categories, have a look at the
  directory/folder names in `tests`.
  * From within a `.py` file, you generate image or textual test results with the
  `Test()` and `TestText()` functions.
  
Once you've added new code and calls to either `Test()` or `TestText()` to
a `.py` file, you will run the new tests for the _first_ time with a command
like so

```
./runtest -b ../test/baseline -d ../../data -e <path-to-visit-executable> <path-to-.py-file>
```

The test will of course _fail_ the first time because there are no baseline
results defined for it. However, the test results that are produced will be
written to `current/<category>/<.py-file-name>/`. If the new result are as
expected, to create the baseline results, you would simply copy those new
`.png` or `.txt` file(s) to their respective place(s) in the `test/baseline`
directory tree being careful to follow the same _category_ name you may have
introduced above.

To make debugging a new test case easier, add the `-v` (-verbose flag)
or `-v --vargs -debug 5` to the `runtest` command, above.

Finally, make sure you tag the test in a comment block with a space
separated list of CLASSES and MODES the test supports. See runtest for
a further explanation.
