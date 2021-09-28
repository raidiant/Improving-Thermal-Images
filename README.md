# Improving Thermal Images

This repository contains some scripts written in R and bash as well as source
code written in C++ that normalize thermographic images and their radiometric
data. A short setup and usage explanation are given below.

## Setup

`R` and `gcc` should be installed as well as some libraries and tools:

* libx11-dev
* exiftool

When above programs and libraries are installed, the R libraries should be
installed. This is simply done by running the provided `packages.R` script in
the `R Scripts` directory.

## Usage

All thermograms should be placed in directory of your choice, which preferably
would be placed inside this directory.

First the C++ binaries should be compiled. This is simply done by issuing the
make command in the head directory. This will put all binaries in the `build/bin`
directory.

```
make
```

Afterwards the shell script `prepareFiles.sh` can be run. The arguments for this
are the directory in which the thermograms can be found and an optional output
directory. When the output directory argument is ommitted `Output` is used. An
example to run the shell script with input directory and output directory:

```
./prepareFiles.sh Data Out
```

The radiometric will be extracted from the thermograms using the R script
`HistogramEqualize.R`. This will further normalize the radiometric data and save
the normalized data to CSV as well as JPEG image.

When this is finished the original thermograms will be copied to the out
directory. We copy the original thermograms to the out directory as their
radiometric data and JPEG representation will be overwritten with the data we've
stored in our CSV files as well as our new JPEG images.

What we end up with in our out directory are normalized thermograms.
