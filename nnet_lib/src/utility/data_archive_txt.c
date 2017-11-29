// Text file data archive format.
//
// The file is comprised of sections, with a section header indicating the size
// of the data payload and a section footer denoting the end of the section.
//
// Section format.
// ===[SECTION_NAME] BEGIN===
// # NUM_ELEMS n
// # TYPE float/int
// 1,2,3,4,.... (total of n comma separated values)
// ===[SECTION_NAME] END===

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/nnet_fwd_defs.h"
#include "utility/utility.h"
#include "utility/data_archive_impl.h"

const char* kTxtGlobalHeader = "===GLOBAL BEGIN===";
const char* kTxtGlobalFooter = "===GLOBAL END===";
const char* kTxtWeightsHeader = "===WEIGHTS BEGIN===";
const char* kTxtWeightsFooter = "===WEIGHTS END===";
const char* kTxtDataHeader = "===DATA BEGIN===";
const char* kTxtDataFooter = "===DATA END===";
const char* kTxtLabelsHeader = "===LABELS BEGIN===";
const char* kTxtLabelsFooter = "===LABELS END===";

static void save_farray_to_txt_file(FILE* fp, farray_t* data, unsigned size) {
    fprintf(fp, "# NUM_ELEMS %d\n# TYPE float\n", size);
    for (unsigned i = 0; i < size; i++) {
        fprintf(fp, "%2.8f,", data->d[i]);
    }
    fprintf(fp, "\n");
}

static void save_iarray_to_txt_file(FILE* fp, iarray_t* data, unsigned size) {
    fprintf(fp, "# NUM_ELEMS %d\n# TYPE int\n", size);
    for (unsigned i = 0; i < size; i++) {
        fprintf(fp, "%d,", data->d[i]);
    }
    fprintf(fp, "\n");
}

static bool find_section_start(FILE* fp, const char* section_header) {
    char* line = NULL;
    size_t line_len = 0;
    bool found_section = false;
    const size_t header_len = strlen(section_header);
    while (getline(&line, &line_len, fp) != -1) {
        if (strncmp(line, section_header, header_len) == 0) {
            found_section = true;
            break;
        }
    }
    if (line)
        free(line);

    return found_section;
}

static data_sec_header read_data_sec_header(FILE* fp,
                                            const char* section_header) {
    if (fp == NULL)
        FATAL_MSG("Can't open data file!\n");

    if (!find_section_start(fp, section_header)) {
        fclose(fp);
        FATAL_MSG("Section header was not found in the file!\n");
    }

    data_sec_header header;
    int num_elems;
    char data_type[6];
    int ret = fscanf(fp, "# NUM_ELEMS %d\n", &num_elems);
    if (ret != 1)
        FATAL_MSG("Corrupted header! NUM_ELEMS not found.\n");
    if (num_elems < 0)
        FATAL_MSG("NUM_ELEMS cannot be negative!\n");
    header.num_elems = num_elems;

    ret = fscanf(fp, "# TYPE %6s\n", &data_type[0]);
    if (ret != 1)
        FATAL_MSG("Corrupted header! Datatype not found.\n");

    if (strncmp(data_type, "float", 6) == 0) {
      header.type = SAVE_DATA_FLOAT;
    } else if (strncmp(data_type, "int", 4) == 0) {
      header.type = SAVE_DATA_INT;
    } else {
        FATAL_MSG("Invalid datatype %s found.\n", data_type);
    }
    return header;
}

static void read_fp_data_from_txt_file(const char* filename,
                                       farray_t* data,
                                       const char* section_header) {
    FILE* fp = fopen(filename, "r");
    data_sec_header header = read_data_sec_header(fp, section_header);
    if (header.num_elems > data->size) {
        FATAL_MSG("This file section contains more data than can be "
                  "stored in the provided array!\n");
    } else if (header.type != SAVE_DATA_FLOAT) {
        FATAL_MSG("Expected datatype float, got datatype int!");
    }

    float read_float;
    for (unsigned i = 0; i < header.num_elems; i++) {
        int ret_f_scanf = fscanf(fp, "%f,", &read_float);
        if (ret_f_scanf != 1) {
            fclose(fp);
            FATAL_MSG("The number of values expected to be read "
                      "exceeded the number of values found!\n");
        }
        data->d[i] = conv_float2fixed(read_float);
    }

    fclose(fp);
}

static void read_int_data_from_txt_file(const char* filename,
                                        iarray_t* data,
                                        const char* section_header) {
    FILE* fp = fopen(filename, "r");
    data_sec_header header = read_data_sec_header(fp, section_header);
    if (header.num_elems > data->size) {
        FATAL_MSG("This file section contains more data than can be "
                  "stored in the provided array!\n");
    } else if (header.type != SAVE_DATA_INT) {
        FATAL_MSG("Expected datatype int, got datatype float!");
    }

    int read_int;
    for (unsigned i = 0; i < header.num_elems; i++) {
        int ret_f_scanf = fscanf(fp, "%d,", &read_int);
        if (ret_f_scanf != 1) {
            fclose(fp);
            FATAL_MSG("The number of data expected to be read "
                      "exceeded the number of data found!\n");
        }
        data->d[i] = read_int;
    }

    fclose(fp);
}

void save_global_parameters_to_txt_file(FILE* fp, network_t* network) {
    fprintf(fp, "%s\n", kTxtGlobalHeader);
    fprintf(fp,
            "# ARCHITECTURE = %s\n"
            "# NUM_LAYERS = %d\n"
            "# DATA_ALIGNMENT = %d\n",
            ARCH_STR, network->depth, DATA_ALIGNMENT);
    fprintf(fp, "%s\n", kTxtGlobalFooter);
}

global_sec_header read_global_header_from_txt_file(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp)
        FATAL_MSG("Can't open data file!\n");

    if (!find_section_start(fp, kTxtGlobalHeader))
        FATAL_MSG("Cannot find the global section header!\n");

    global_sec_header header;
    char* line = NULL;
    size_t line_len = 0;
    int ret = getline(&line, &line_len, fp);
    if (ret == -1)
        FATAL_MSG("Unable to read from the data file!\n");

    header.arch_str = (char*)malloc(line_len);
    ret = sscanf(line, "# ARCHITECTURE = %s\n", header.arch_str);
    if (ret != 1)
        FATAL_MSG("Could not determine the architecture that generated this "
                  "file!\n");
    if (strncmp(header.arch_str, "MONOLITHIC", line_len) == 0)
        header.arch = Arch_Monolithic;
    if (strncmp(header.arch_str, "COMPOSABLE", line_len) == 0)
        header.arch = Arch_Composable;
    if (strncmp(header.arch_str, "SMIV", line_len) == 0)
        header.arch = Arch_SMIV;
    if (strncmp(header.arch_str, "EIGEN", line_len) == 0)
        header.arch = Arch_Eigen;

    ret = fscanf(fp, "# NUM_LAYERS = %d\n", &header.num_layers);
    if (ret != 1)
        FATAL_MSG("Could not determine number of layers in this network!\n");
    ret = fscanf(fp, "# DATA_ALIGNMENT = %d\n", &header.data_alignment);
    if (ret != 1)
        FATAL_MSG("Could not determine data alignment of this archive!\n");

    fclose(fp);
    return header;
}

void save_weights_to_txt_file(FILE* fp, farray_t* weights, size_t num_weights) {
    fprintf(fp, "%s\n", kTxtWeightsHeader);
    save_farray_to_txt_file(fp, weights, num_weights);
    fprintf(fp, "%s\n", kTxtWeightsFooter);
}

void save_data_to_txt_file(FILE* fp, farray_t* data, size_t num_values) {
    fprintf(fp, "%s\n", kTxtDataHeader);
    save_farray_to_txt_file(fp, data, num_values);
    fprintf(fp, "%s\n", kTxtDataFooter);
}

void save_labels_to_txt_file(FILE* fp, iarray_t* labels, size_t num_labels) {
    fprintf(fp, "%s\n", kTxtLabelsHeader);
    save_iarray_to_txt_file(fp, labels, num_labels);
    fprintf(fp, "%s\n", kTxtLabelsFooter);
}

void read_weights_from_txt_file(const char* filename, farray_t* weights) {
    printf("Reading weights from %s...\n", filename);
    read_fp_data_from_txt_file(filename, weights, kTxtWeightsHeader);
}

void read_data_from_txt_file(const char* filename, farray_t* data) {
    printf("Reading input data from %s...\n", filename);
    read_fp_data_from_txt_file(filename, data, kTxtDataHeader);
}

void read_labels_from_txt_file(const char* filename, iarray_t* labels) {
    printf("Reading output labels from %s...\n", filename);
    read_int_data_from_txt_file(filename, labels, kTxtLabelsHeader);
}