#!/bin/bash

BINDIR=build/bin
DATADIR=$1
OUTDIR=$2

if [[ -z $1 ]];then
    echo "Usage: ./prepareFiles.sh <data directory> <optional: out directory>"
    exit 1
fi

if [[ ! -f $BINDIR/copyJPG || ! -f $BINDIR/replaceRadioMetadata ]]; then
    echo "Binaries for copyJPG and replaceRadioMetadata are not present in $BINDIR"
    exit 1
fi

if [[ ! -d $DATADIR ]]; then
    echo "Given data directory is not present"
    exit 1
fi

if [[ -z $OUTDIR ]]; then
    OUTDIR=Output
fi

if [[ ! -d $OUTDIR ]]; then
    mkdir $OUTDIR
fi

# # Run our normalization step to output to images and CSV
echo "Normalizing thermograms and outputting to new images and CSV"
Rscript --vanilla R\ scripts/HistogramEqualize.R $DATADIR $OUTDIR > /dev/null 2>&1
echo "Finished normalizing thermograms, images and CSVs stored in $OUTDIR"

# Copy original thermograms
echo "Copying original thermograms to output folder"
cp -vr $DATADIR/* $OUTDIR

# JPG transfer and radiometrica data replacement
for file in $OUTDIR/*_R.JPG ; do
    # make our normalised image and csv filename
    norm_img=${file/_R/_R_he}
    norm_csv=${file/_R.JPG/_R_he.csv}

    echo "Copying JPEG data from $norm_img to $file"

    # copy JPG portion of normalised image to original
    $BINDIR/copyJPG $norm_img $file > /dev/null 2>&1

    echo "Replacing radiometric data in $file with data in $norm_csv"

    # replace radiometric data
    $BINDIR/replaceRadioMetadata $file $norm_csv > /dev/null 2>&1
done

echo "Processing finished. Files are stored in $OUTDIR"
