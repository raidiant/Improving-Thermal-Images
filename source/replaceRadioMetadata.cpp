#include <regex>
#include <iterator>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

bool DEBUG = true;

int main(int argc, char const *argv[])
{
    if (argc < 3)
    {
        printf("Not the right amount of arguments given!\nUsage: %s <therm.jpg> <data.csv>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // TODO: option handling when needed
    // TODO: filter options from filenames

    // TODO: consolidate to function handling file checking
    const char *fileName = argv[1];
    const char *dataFileName = argv[2];

    if (DEBUG) std::cout << "DEBUG: File: " << fileName << std::endl;

    // opening file
    std::ifstream thermImageFile(fileName, std::ios::in | std::ios::binary);
    std::ifstream normalisedDataFile(dataFileName, std::ios::in);

    // check if file exists
    if (!thermImageFile.good())
    {
        printf("%s does not exist.\n", fileName);
        exit(EXIT_FAILURE);
    }
    // check if file exists
    if (!normalisedDataFile.good())
    {
        printf("%s does not exist.\n", dataFileName);
        exit(EXIT_FAILURE);
    }

    // establish file size by moving to end of file and asking what our "get" pointer is
    thermImageFile.seekg(0, std::ios::end);
    size_t thermal_file_size = thermImageFile.tellg();
    thermImageFile.seekg(0, std::ios::beg);
    if (DEBUG) std::cout << "DEBUG: thermal image filesize: " << thermal_file_size << std::endl;
    // establish file size by moving to end of file and asking what our "get" pointer is
    normalisedDataFile.seekg(0, std::ios::end);
    size_t data_file_size = normalisedDataFile.tellg();
    normalisedDataFile.seekg(0, std::ios::beg);
    if (DEBUG) std::cout << "DEBUG: normalised data filesize: " << data_file_size << std::endl;

    // store the content of our file here
    std::string jpg_file_content;
    std::string data_file_content;

    // reserve the full filesize memory needed
    jpg_file_content.reserve(thermal_file_size);
    data_file_content.reserve(data_file_size);

    // store currently read in buffer
    char buffer[16384];

    // store amount of characters read in chars_read
    std::streamsize chars_read;

    // reading file
    while (thermImageFile.read(buffer, sizeof buffer), chars_read = thermImageFile.gcount())
    {
        jpg_file_content.append(buffer, chars_read);
    }
    while (normalisedDataFile.read(buffer, sizeof buffer), chars_read = normalisedDataFile.gcount())
    {
        data_file_content.append(buffer, chars_read);
    }

    // define our search term here
    //   we're searching for "FLIR" as this denotes the start of a FLIR segment
    std::string segment_search_term = "FLIR";
    size_t segment_search_term_size = segment_search_term.size();

    // we want to know where all FLIR segments start to search for an FFF
    // header in the segment so we store the offsets here
    std::vector<off_t> segments;

    // start searching for our FLIR segment header when we reach EOF
    if (thermImageFile.eof())
    {
        // print that we've reached EOF when in DEBUG
        if (DEBUG) std::cout << "DEBUG: File: Reached EOF, found following offsets where FLIR data resides" << std::endl;
        for (std::string::size_type offset = 0, found_at;
             thermal_file_size > offset && (found_at = jpg_file_content.find(segment_search_term, offset)) != std::string::npos;
             offset = found_at + segment_search_term_size)
        {
            // print the offsets we found to stdout when in DEBUG
            if (DEBUG) std::cout << "DEBUG: \t" << found_at << std::endl;

            // add our found_at to our segments vector
            //   this shows where the start of the segment is
            segments.push_back(found_at);
        }
    }

    // finding FLIR metadata
    //   metadata is at a 0x0440 offset from the FFF header
    //   the FFF header, when present, is 8 bytes into the FLIR segment
    std::string fff_search_term = "FFF";

    // store our FFF metadata offset here
    off_t fff_header_offset;

    // for all found segments, check if the FFF header is present, we only
    // process the segment in which the FFF header resides
    for (std::vector<off_t>::iterator segment_offset = segments.begin(); segment_offset != segments.end(); ++segment_offset)
    {
        // as said before our FFF header should be 8 bytes in
        off_t header_offset = (*segment_offset) + 8;

        // set "get" pointer to segment start and clear flags
        thermImageFile.clear();
        thermImageFile.seekg(header_offset);

        // create 8 byte buffer
        char buffer[8];
        std::string segment_header;

        // reading file
        thermImageFile.read(buffer, sizeof buffer);
        segment_header.append(buffer);

        // check if we have a FFF header 8 bytes in
        std::string::size_type found = segment_header.find(fff_search_term);
        if (found != std::string::npos)
        {
            // store our position in header offset var
            fff_header_offset = header_offset;

            // output to DEBUG which offset we've found
            if (DEBUG)
                std::cout << "DEBUG: Found FFF header at: " << header_offset << std::endl;

            // we don't need to look further
            thermImageFile.close();
            break;
        }
    }

    // these are not necessary but would clean up a lot of rainbow numbers used for offsets
    //   TODO: read in tags
    //   TODO: find RawThermalImage tag

    // size for accessing 640x512 x 16bit of raw sensor data
    //   note: stored Little-Endian
    size_t image_size = 640 * 512 * sizeof(uint16_t);
    // size_t img_size_with_headers =

    // prepare file descriptor and pointers for mapping
    void *addr;
    int fd = open(fileName, O_RDWR);

    // the offset from our header is 0x220 so we add it here and determine our page offset
    off_t radio_data_offset = fff_header_offset + 0x220;
    off_t pa_offset = radio_data_offset & ~(sysconf(_SC_PAGE_SIZE) - 1);

    // we also calculate how much bytes we have to skip from the start of the page
    // the 6 * 10 is the skipping of the FLIR segment header that is introduced
    off_t data_start_offset_diff = radio_data_offset - pa_offset;
    off_t size_with_data_offset = image_size + data_start_offset_diff + 6 * 10;

    // tell user our mapping variables when in DEBUG
    if (DEBUG)
    {
        std::cout << "DEBUG: Mapping variables:" << std::endl;
        std::cout << "DEBUG:\timage_size = " << image_size << std::endl;
        std::cout << "DEBUG:\taddressed_size = " << size_with_data_offset << std::endl;
        std::cout << "DEBUG:\tfd = " << fd << std::endl;
        std::cout << "DEBUG:\tradio_data_offset = " << radio_data_offset << std::endl;
        std::cout << "DEBUG:\tpa_offset = " << pa_offset << std::endl;
        std::cout << "DEBUG:\toffset diff; data start = " << data_start_offset_diff << std::endl;
        std::cout << "DEBUG: Mapping file to memory" << std::endl;
    }

    // memory map to the page where our data starts
    addr = mmap(NULL, size_with_data_offset, PROT_READ | PROT_WRITE, MAP_SHARED, fd, pa_offset);

    // offset it with the difference between our page offset and data offset
    addr = (char *)addr + data_start_offset_diff;

    // print the first line of radiometric data when in DEBUG
    if (DEBUG)
    {
        std::cout << "DEBUG: First 16 bytes of radiometric data:" << std::endl
                  << "DEBUG: \t";

        for (size_t i = 0; i < 16; i++)
        {
            unsigned char c = ((char *)addr)[i];
            if (i % 2 == 0) printf("%02x", c);
            else printf("%02x ", c);
        }

        printf("\nDEBUG:\t");

        for (size_t i = 0; i < 8; i++)
        {
            uint16_t c = ((uint16_t *)addr)[i];
            printf("%04u ", c);
        }

        printf("\n");
    }

    // create a uint_16t pointer for easier access to stored values
    uint16_t *radio_data = (uint16_t *)addr;

    // make our height and width easily accessible
    int width = 640;
    int height = 512;

    // store our index and values read here
    size_t index = 0;
    size_t read = 0;

    // store our values in an array
    uint16_t data[width * height];

    // new way of searching for values
    std::regex sensor_value_regex("(\\d{4})");
    auto sensor_values_begin = std::sregex_iterator(data_file_content.begin(), data_file_content.end(), sensor_value_regex);
    auto sensor_values_end = std::sregex_iterator();

    // find and assing our values
    for (std::sregex_iterator i = sensor_values_begin; i != sensor_values_end; ++i)
    {
        // store our match
        std::smatch match = *i;

        // assign it to our data array
        data[index] = std::stoul(match.str());

        // advance our array index
        ++index;
    }

    // show what our new values will be
    if (DEBUG)
    {
        std::cout << "DEBUG: First 16 bytes of normalised radiometric data:" << std::endl
                  << "DEBUG: \t";

        for (size_t i = 0; i < 16; i++)
        {
            unsigned char c = ((char *)data)[i];
            if (i % 2 == 0) printf("%02x", c);
            else printf("%02x ", c);
        }

        printf("\nDEBUG:\t");

        for (size_t i = 0; i < 8; i++)
        {
            uint16_t c = ((uint16_t *)data)[i];
            printf("%04u ", c);
        }

        printf("\n");
    }

    // // old way of searching for values
    // while (false)
    // {
    //     // find semicolon
    //     std::string::size_type semicolon = rest.find(';');
    //     // when we haven't found anything we move on
    //     if (semicolon == std::string::npos) break;
    //     // everytime we find a semicolon, the 4 characters afterwards are our
    //     // integer
    //     rest = rest.substr(semicolon+1, std::string::npos);
    //     // interpret string as unsigned integer
    //     data[index] = std::stoul(rest.substr(0, 4));
    //     if (DEBUG) printf("DEBUG: Found integer: %04u\n", data[index]);
    //     ++index;
    // }

    // reset our counter
    index = 0;

    while (read < width * height)
    {
        // when our current value is 0xffe1 we came upon a segment header
        // thus we move forward 6 uint16_t (12 bytes) to go to the rest of
        // our binary data
        if (radio_data[index] == 57855)
        {
            index += 6;
            continue;
        }

        // write our value to our mapped memory in the file
        radio_data[index] = data[read];

        // increment both our index and read counter
        ++index;
        ++read;
    }

    // make sure we've written everything back to the file
    if (DEBUG) std::cout << "DEBUG: Done assigning values\nDEBUG: Syncing back to file" << std::endl;
    msync(addr, size_with_data_offset, MS_SYNC);
    if (DEBUG) std::cout << "DEBUG:\tdone!" << std::endl;
    munmap(addr, size_with_data_offset);
    if (DEBUG) std::cout << "DEBUG:\tunmapped file" << std::endl;

    return EXIT_SUCCESS;
}