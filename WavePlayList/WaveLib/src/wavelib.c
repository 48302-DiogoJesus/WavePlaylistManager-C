#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <stdint.h>
#include <stdbool.h> 

#include "wavelib.h"

static size_t wav_read_bytes(const char* filename, size_t start, size_t block_size, uint8_t *buffer);
static int wave_extract_header_field(Wave *wave, const int offset, const int field_size, const bool little_endian);
static int regex_match(const char *string, const char *pattern);
static int ConvertToInt(uint8_t value[], int bytesNum, const bool littleEndian);
static int wave_get_data_size(Wave *wave);

/* ----------------------------------- WAVE LIBRARY FUNCTIONS ----------------------------------- */

/**
 * Wave Load (Creates a Wave file representation in memory)
 * @param filename Name of the file to load
 * @returns pointer to the new Wave struct
*/
Wave *wave_load(const char* filename) {
    if (!regex_match(filename, "\\.wav$")) 
        return NULL;
    if (access(filename, F_OK) != 0)
        return NULL;
        
    Wave *new_wave = (Wave *)malloc(sizeof(Wave));
    if (new_wave == NULL) {
        printf("Out of memory!");
        return NULL;
    }
    
    new_wave->filepath = filename;

    // Calculate file Data size to allocate memory for it
    int data_size = wave_get_data_size(new_wave);
    new_wave->data = (uint8_t *)malloc(data_size);
    // Copy all the file Data to a buffer (new_wave->data)
    const int dataOffset = 44;  // Header Size
    wav_read_bytes(new_wave->filepath, dataOffset, data_size, new_wave->data);

    return new_wave;
}

/**
 * Wave Destroy (Deletes the representation of a Wave file in memory)
 * @param wave Pointer to the wave object to destroy
*/
void wave_destroy(Wave *wave) {
    free(wave);
}

/**
 * Get Bits per Sample
 * @param wave Pointer to the wave object
 * @returns number of bits per sample
*/
int wave_get_bits_per_sample(Wave *wave) {
    if (strlen(wave->filepath) == 0)
        return -1;

    return wave_extract_header_field(wave, 34, 2, true);;
}

/**
 * Get Number of Channels
 * @param wave Pointer to the wave object
 * @returns number of channels
*/
int wave_get_number_of_channels(Wave *wave) {
    if (strlen(wave->filepath) == 0)
        return -1;

    return wave_extract_header_field(wave, 22, 2, true);
}

/**
 * Get Sample Rate
 * @param wave Pointer to the wave object
 * @returns sample rate number
*/
int wave_get_sample_rate(Wave *wave) {
    if (strlen(wave->filepath) == 0)
        return -1;

    return wave_extract_header_field(wave, 24, 4, true);
}

/**
 * Wave Get Samples (Extract wave samples from Wave object)
 * @param wave Pointer to the wave object
 * @param frame_index Index for the first frame (group of samples)
 * @param buffer Data Buffer where the samples will be stored
 * @param frame_count Number of frames to retrieve
 * @returns number of samples
*/
size_t wave_get_samples(Wave *wave, size_t frame_index, uint8_t *buffer, size_t frame_count) {
    if (strlen(wave->filepath) == 0)
        return -1;

    int bits_per_sample = wave_get_bits_per_sample(wave);
    int channels_number = wave_get_number_of_channels(wave);

    // 1st complete index = ((bits_per_sample / 8) * channels_number) * frame_index
    // last complete index = ((bits_per_sample / 8) * channels_number) * (frame_index + frame_counts - 1)

    int frame_size = (bits_per_sample / 8) * channels_number; // Bytes per frame (1 Frame has [channels_number] Samples)

    // Both in Bytes
    const int startIndex = frame_index * frame_size;
    const int endIndex = ((frame_index + frame_count) * frame_size) - 1;


    // endIndex - startIndex + 1 -> Offset(in Bytes) from the start
    memcpy(buffer, wave->data + startIndex, endIndex - startIndex + 1);

    return frame_count * channels_number;
}

/* ----------------------------------- AUXILIARY FUNCTIONS ----------------------------------- */

/**
* Extract the "Subchunk2Size" field from the headers which represents the data size
* @param wave Pointer to the wave object
* @returns wave file data field size
*/
static int wave_get_data_size(Wave *wave) {
    if (strlen(wave->filepath) == 0)
        return -1;

    return wave_extract_header_field(wave, 40, 4, true);
}

/**
* Extract the value from a field in the header of a wave file
* @param wave Pointer to the wave object
* @param offset Offset from the beggining of the file to the field we wish to get the value from
* @param field_size Size of the header field we wish to retrieve
* @param little_endian Indicates if the field is in little-endian format or big-endian format
* @returns the header field asked in the position [0 + offset; 0 + offset + field_size]
*/
static int wave_extract_header_field(Wave *wave, const int offset, const int field_size, const bool little_endian) {
    uint8_t *buffer = (uint8_t *)malloc(field_size);

    wav_read_bytes(wave->filepath, offset, field_size, buffer);

    int field = ConvertToInt(buffer, field_size, little_endian);

    free(buffer);
    return field;
}

/**
* Read Bytes from any file
* @param filename Name of the file
* @param start Number of offset bytes from the begging of the file
* @param block_size Number of bytes to extract to the buffer
* @param buffer Holds the bytes retrieved from the file
* @returns number of bytes put inside the buffer
*/
static size_t wav_read_bytes(const char* filename, size_t start, size_t block_size, uint8_t *buffer) {
    FILE *fp = fopen(filename, "r");
    int iterations = start;
    // Add [start] offset to file pointer (in Bytes)
    while (iterations--) {
        fread(buffer, 1, 1 , fp);
    }
    fread(buffer, 1, block_size , fp);

    fclose(fp);
    return block_size;
}

/**
* Convert a little-endian/big-endian value spreaded in an array of 1 byte each position
* @param value Bytes Array where the value is contained
* @param bytesNum Number of Bytes inside the [value] array
* @param littleEndian Indicates if the field is in little-endian format or big-endian format
* @returns Value inside [value] converted to integer(2 Bytes(short int) or 4 Bytes(int))
*/
static int ConvertToInt(uint8_t value[], int bytesNum, const bool littleEndian) {
    if (littleEndian && bytesNum == 2) {
        short newValue = value[0] | (value[1] << 8);
        return newValue;
    } else if (littleEndian && bytesNum == 4) {
        int newValue = (value[0] & 0xFF) | ((value[1] & 0xFF) << 8) | ((value[2] & 0xFF) << 16) | ((value[3] & 0xFF) << 24);
        return newValue;
    } else if (!littleEndian && bytesNum == 2) {
        short newValue = value[1] | (value[0] << 8);
        return newValue;
    } else if (!littleEndian && bytesNum == 4) {
        int newValue = (value[3] & 0xFF) | ((value[2] & 0xFF) << 8) | ((value[1] & 0xFF) << 16) | ((value[0] & 0xFF) << 24);
        return newValue;
    }
    return 0;
}

/**
* Regex Match
* Simple function to check if a string matches a regular expression
* @param string Pointer to the string
* @param pattern Pointer to the pattern we wish to verify on the [string]
* @returns 1 if pattern matches and 0 if not
*/
static int regex_match(const char* string, const char* pattern) {
    regex_t regex;
    regcomp(&regex, pattern, 0);
    return !regexec(&regex, string, 0, NULL, 0);
}