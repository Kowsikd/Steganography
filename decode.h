#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include <string.h>
#include "types.h"
#include "common.h"

/* Structure to hold variables needed for decoding */
typedef struct _DecodeInfo
{
    /* stego image info */
    char *steg_img_fname;
    FILE *fptr_steg_img;
    char steg_data_buf[8];

    /* secret file info */
    char *sec_out_fname;
    FILE *fptr_sec_out;
    char sec_data;
    int sec_ext_size;
    long int sec_data_size;
    int out_arg_flag;
    
}DecodeInfo;

/* Read and validate Decode args from argv */
Status read_and_validate_input_args(char *argv[], DecodeInfo *decInfo);

/* Read and validate Decode args from argv */
Status read_and_validate_output_args(DecodeInfo *decInfo);

/* Get File pointers for i/p files */
Status open_input_files(DecodeInfo *decInfo);

/* Get File pointers for o/p files */
Status open_output_files(DecodeInfo *decInfo);

/* Check Magic String */
Status decode_magic_string(char *magic_str, DecodeInfo *decInfo);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Decode the bytes from LSB of image into data */
Status decode_byte_to_data(char *image_buffer, DecodeInfo *decInfo);

/* Decode secret file extension size */
Status decode_secret_ext_size(DecodeInfo *decInfo);

/* Decode secret file extension */
Status decode_secret_ext(DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_data_size(DecodeInfo *decInfo);

/* Decode secret file data */
Status decode_secret_data(DecodeInfo *decInfo);

#endif
