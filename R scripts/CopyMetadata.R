library("exifr")

# Create a function with arguments.
new.copy_metadata <- function(image_file, folder_path="", output_path="Output/") {
  ## Create image path
  image_ori = paste(folder_path, image_file, sep="")
  image_cal = paste(output_path, image_file, sep="")
  output_folder = paste("-o ", output_path,"GPS_metadata/", sep="")

  # print(image_ori)
  # print(image_cal)
  argument = paste("-tagsfromfile", image_ori, image_cal, output_folder,sep=" ")
  # print(argument)
  exiftool_call(argument)
}

input_path = "../Data/"
filenames <- list.files(path=input_path, pattern="*.JPG", full.names=FALSE)

                        
OP = "1_Nothing/"
lapply(filenames, new.copy_metadata, folder_path=input_path, output_path=OP)
                    
OP = "2_HE/"
lapply(filenames, new.copy_metadata, folder_path=input_path, output_path=OP)

OP = "3_NF/"
lapply(filenames, new.copy_metadata, folder_path=input_path, output_path=OP)

OP = "4_HE_NF/"
lapply(filenames, new.copy_metadata, folder_path=input_path, output_path=OP)

