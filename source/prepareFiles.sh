#!/bin/bash

# copy metadata from original images to normalised images
# for file in *_R.JPG ; do
#     exiftool -tagsFromFile $file ${file/_R/_R_he}
# done

# JPG transfer and radiometrica data replacement
for file in *_R.JPG ; do
    # make our normalised image and csv filename
    norm_img=${file/_R/_R_he}
    norm_csv=${file/_R.JPG/_R_he.csv}

    echo "Copying JPEG data from $norm_img to $file"

    # copy JPG portion of normalised image to original
    ./copyJPG $norm_img $file

    echo "Replacing radiometric data in $file with data in $norm_csv"

    # replace radiometric data
    ./replaceRadioMetadata $file $norm_csv
done
