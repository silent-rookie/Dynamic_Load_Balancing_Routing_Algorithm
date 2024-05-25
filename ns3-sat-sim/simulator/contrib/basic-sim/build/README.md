# Build

This directory, which is not part of the ns-3 code, is used to test this module. If you are only going to use the `basic-sim` module itself, this directory is not relevant to you (beyond possibly the example run folders).

The following commands are defined (in order in which they can be executed):

* `build.sh` : Building the module together with default ns-3 (multiple build objectives are possible).

* `rebuild.sh` : In case you have made a change to the `basic-sim` ns-3 code, you can check it still builds (keeps the build objective set by `build.sh` before).

* `run_assist.sh` : A wrapper around the run call -- this is just for convenience to be able to execute both non-distributed and distributed runs with the same command.

* `test.sh` : Run all the `basic-sim` tests. It outputs the results in `test_results/` and the coverage report in `coverage_report/`.

* `tutorial.sh` : Runs the tutorial described in the `<basic-sim>/doc/getting_started.md` (and showcases the plotting helper tools by producing a bunch of plots)

* `example.sh` : Runs a variety of example run folders (located in `example_run_folders/`).
 
* `clean.sh` : Removes test results, coverage report, builds and tutorial/example log files.
