/**
 * copyJPG.cpp
 * 
 * Copy the JPG portion of one image to another image.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

/**
 * These constants describe the bytes that signify what segment of a JPEG file
 * we've entered.
 * 
 * dqt:  specifies one or more quantization tables
 * sof0: indicates that this is a baseline DCT-based JPEG, and specifies the
 *       widht, height, number of components and component subsampling
 * dht:  specifies one or more Huffman tables
 * eoi:  end of image
 */
// const unsigned char dqt[3]  = {0xff, 0xdb, 0x00};
// const unsigned char sof0[3] = {0xff, 0xc0, 0x00};
// const unsigned char dht[3]  = {0xff, 0xc4, 0x00};
// const unsigned char sos[3]  = {0xff, 0xda, 0x00};
// const unsigned char eoi[2]  = {0xff, 0xd9};

bool DEBUG = true;

int main(int argc, char const *argv[])
{
    // show usage if we didn't receive the right number of arguments
    if (argc < 3)
    {
        printf("Missing arguments!\nUsage: %s <from.jpg> <to.jpg>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // store our filenames here
    const char *inJpgFilename = argv[1];
    const char *outJpgFilename = argv[2];

    if (DEBUG) std::cout << "DEBUG: InFile: " << inJpgFilename << std::endl;
    if (DEBUG) std::cout << "DEBUG: OutFile: " << outJpgFilename << std::endl;

    // opening file
    std::ifstream inJpgFile(inJpgFilename, std::ios::in | std::ios::binary);
    std::ifstream outJpgFile(outJpgFilename, std::ios::in | std::ios::binary);

    // check if file exists
    if (!inJpgFile.good())
    {
        // print our error string
        std::cout << "In file does not exist." << std::endl;

        // tell the shell we've exited with non-zero status
        exit(EXIT_FAILURE);
    }
    // check if file exists
    if (!outJpgFile.good())
    {
        // print our error string
        std::cout << "Out file does not exist." << std::endl;

        // tell the shell we've exited with non-zero status
        exit(EXIT_FAILURE);
    }

    // establish file size by moving to end of file and asking what our "get" pointer is
    outJpgFile.seekg(0, std::ios::end);
    size_t out_file_size = outJpgFile.tellg();
    outJpgFile.seekg(0, std::ios::beg);
    if (DEBUG) std::cout << "DEBUG: filesize: " << out_file_size << std::endl;

    // store the content of our file here
    std::string outFileContent;

    // reserve the full filesize memory needed
    outFileContent.reserve(out_file_size);

    // store currently read in buffer
    char buffer[16384];

    // store amount of characters read in chars_read
    std::streamsize chars_read;

    // reading file
    while (outJpgFile.read(buffer, sizeof buffer), chars_read = outJpgFile.gcount())
    {
        outFileContent.append(buffer, chars_read);
    }

    // close our file
    outJpgFile.close();

    // store our search string here
    std::string endOfImagePattern("\xff\xd9");
    std::string startDqtPattern("\xff\xdb\x00");

    // FLIR images contain small JPEG thumbnails, so we first have to find the end of this thumbnail
    std::string::size_type endOfImageOffset = outFileContent.find(endOfImagePattern);
    std::cout << "DEBUG: EOI thumbnail offset: "<< endOfImageOffset << std::endl;
    // then we search for the start of our JPEG data
    std::string::size_type startDqtOffset = outFileContent.substr(endOfImageOffset).find(startDqtPattern);
    std::cout << "DEBUG: Start DQT offset: "<< startDqtOffset << std::endl;
    // our data starts here
    size_t outFileJpgDataOffset = endOfImageOffset + startDqtOffset;
    size_t outJpgDataLength = outFileContent.substr(endOfImageOffset+startDqtOffset).length();
    std::cout << "DEBUG: JPEG data offset: " << outFileJpgDataOffset << std::endl;
    std::cout << "DEBUG: JPEG data length: " << outJpgDataLength << std::endl;

    // IN JPEG FILE
    // establish file size by moving to end of file and asking what our "get" pointer is
    inJpgFile.seekg(0, std::ios::end);
    size_t in_file_size = inJpgFile.tellg();
    inJpgFile.seekg(0, std::ios::beg);
    if (DEBUG) std::cout << "DEBUG: filesize: " << in_file_size << std::endl;

    // store the content of our file here
    std::string inFileContent;

    // reserve the full filesize memory needed
    inFileContent.reserve(in_file_size);

    // reading file
    while (inJpgFile.read(buffer, sizeof buffer), chars_read = inJpgFile.gcount())
    {
        inFileContent.append(buffer, chars_read);
    }

    // close our file
    inJpgFile.close();

    // for the in file we only have to search for the DQT offset as there is no thumbnail embedded
    std::string::size_type inFileJpgDataOffset = inFileContent.find(startDqtPattern);
    if (DEBUG) std::cout << "DEBUG: JPEG data offset: " << inFileJpgDataOffset << std::endl;

    // then we search for the start of our JPEG data
    std::string inJpegData = inFileContent.substr(inFileJpgDataOffset);
    size_t inJpgDataLength = inJpegData.length();
    if (DEBUG) std::cout << "DEBUG: JPEG data length: " << inJpgDataLength << std::endl;

    // seek start of JPEG data in outFile
    std::fstream outFile(outJpgFilename);
    outFile.seekp(outFileJpgDataOffset);
    if (DEBUG) std::cout << "DEBUG: Seeking outfile to offset: " << outFileJpgDataOffset << std::endl;

    // write all jpeg data from inFile to outFile
    outFile.write(inJpegData.c_str(), inJpgDataLength);
    if (DEBUG) std::cout << "DEBUG: Written bytes of inJpgData: " << inJpgDataLength << std::endl;

    // sync outFile
    outFile.close();
    if (DEBUG) std::cout << "DEBUG: Closed outFile" << std::endl;

    /**
     * AFGELOPEN IEDEREEEN NAAR HUIS!!!!
     */
    exit(EXIT_SUCCESS);
}