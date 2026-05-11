# VSFS

Very Simple File System project.

## Building

Command line:

* `make` builds the project.
* `make test` runs the tests.
* `make clean` removes object files.
* `make pristine` removes all build products.

## Files

* `image.c`: Opens and closes the filesystem image.
* `image.h`: Header for image functions.
* `block.c`: Reads and writes filesystem blocks.
* `block.h`: Header for block functions.
* `testfs.c`: Test program for the filesystem.
* `ctest.h`: Simple testing framework.
* `Makefile`: Builds the project.

## Data

The filesystem uses blocks of 4096 bytes.

A file called `disk.img` is used as the simulated filesystem image.

## Functions

* `image_open()`: Opens the filesystem image.
* `image_close()`: Closes the filesystem image.
* `bread()`: Reads a block from the image.
* `bwrite()`: Writes a block to the image.

## Notes

* The project uses Unix system calls such as `open()`, `read()`, `write()`, and `lseek()`.
* Tests are implemented using `ctest.h`.