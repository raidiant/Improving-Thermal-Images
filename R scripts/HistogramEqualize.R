# Load packages
library("Thermimage")
library("fields")
library("imager")
library("spatstat")
library("exifr")


# Set configurations

# Input path folder
input = "Data/"
# Output path folder
output = "Output/"

# Create our output directory
dir.create(file.path(output), showWarnings = FALSE)

# Load path images from directory
filenames <- list.files(input, pattern="*.JPG", full.names=FALSE)

# Read image function: load original thermograms in and store the sensor values
# for later use.
new.read_image <- function(image_file,
                           input_path="",
                           output_path="Output/")
{
    # Create image path
    image_path = paste(input_path, image_file, sep="")

    # Rotate image
    sensor_values = rotate270.matrix(readflirJPG(image_path))

    # Save our sensor values
    saveRDS(sensor_values, file = paste(output_path, tools::file_path_sans_ext(image_file), ".RData", sep=""))

    # Return the sensor values
    sensor_values
}

# Load sensor values
glob_sensorvalues_range <- unlist(lapply(filenames, new.read_image,
                                 input_path=input,
                                 output_path=output))

# Calculate global min and max values
min_temp <- min(glob_sensorvalues_range)
max_temp <- max(glob_sensorvalues_range)
print(min_temp)
print(max_temp)


# Calculate ECDF for histogram equalization
f <- ecdf(glob_sensorvalues_range)


# Normalize images (convert temperature values to gray values)
new.normalize <- function(image_file,
                          output_path="Output/",
                          min,
                          max,
                          f_ecdf=NULL)
{
    ## Create image path
    image_path = paste(output_path, tools::file_path_sans_ext(image_file), ".RData", sep="")
    temp_image <- readRDS(image_path)
    ## Delete .Rdata file
    unlink(image_path)

    temp_image <- mirror.matrix(temp_image)

    # Histogram equalize our sensor values
    equalized <- f_ecdf(temp_image)

    # Store normalized image
    img <- as.cimg(equalized, dim=dim(temp_image))
    save.image(img, file=paste(output_path, tools::file_path_sans_ext(image_file), "_he.JPG", sep=""), quality = 1)

    # Round off decimal values and write to CSV
    equalized <- round((equalized * (max - min)) + min)
    write.csv2(equalized, file=paste(output_path, tools::file_path_sans_ext(image_file), "_he.csv", sep = ""), row.names = FALSE)
}


# Plot normalized images
temp = lapply(filenames, new.normalize,
              output_path=output,
              min=min_temp, max=max_temp,
              f_ecdf=f)
