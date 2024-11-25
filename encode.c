#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    //printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    //printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    printf("INFO: Opening required files\n");

    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    printf("INFO: Opened %s\n", encInfo->src_image_fname);
    
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
     encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    printf("INFO: Opened %s\n", encInfo->secret_fname);
    
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    printf("INFO: Opened %s\n", encInfo->stego_image_fname);
    
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/* To cheak whether all the required files are mentioned for encoding */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Read and validate source image file name
    if(argv[2] != NULL && strcmp(strstr(argv[2], "."), ".bmp") == 0)
    {
        encInfo -> src_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    // Read and validate secret text file name
    if(argv[3] != NULL && strcmp(strstr(argv[3], "."), ".txt") == 0)
    {
        encInfo -> secret_fname = argv[3];
    }
    else
    {
        return e_failure;
    }

    // Read and validate output file name
    if(argv[4] != NULL && strcmp(strstr(argv[4], "."), ".bmp") == 0)
    {
        encInfo -> stego_image_fname = argv[4];
    }
    else
    {
        printf("INFO: Output File not mentioned. Creating stego.bmp as default\n");
        encInfo -> stego_image_fname = "stego.bmp";
    }

    // No failure occured
    return e_success;
}

/* To get the size of secret file */
uint get_file_size(FILE *fptr_secret)
{
    fseek(fptr_secret, 0, SEEK_END);
    return ftell(fptr_secret);
}

/* To check if image capacity is enough to hold secret data */
Status check_capacity(EncodeInfo *encInfo)
{    
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    printf("INFO: Checking for %s size\n", encInfo->secret_fname);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    if(encInfo->size_secret_file > 0)
    {
        printf("INFO: Done. Not Empty\n");
        printf("INFO: Checking for %s capacity to handle %s\n", encInfo->src_image_fname, encInfo->secret_fname);
        if(encInfo->image_capacity > (54 + (2 + 4 + 4 + 4 + encInfo->size_secret_file) * 8))
        {
            printf("INFO: Done. Found OK\n");
            return e_success;
        }
        else
        {
            printf("INFO: Done. Not enough Capacity\n");
            return e_failure;
        }
    }
    else
    {
        printf("INFO: Done. File Empty\n");
        return e_failure;
    }
    
}

/* To copy source bmp file header to stego file */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    printf("INFO: Copying Image Header\n");
    char header[54];
    fseek(fptr_src_image, 0, SEEK_SET);
    fread(header, sizeof(char), 54, fptr_src_image);
    fwrite(header, sizeof(char), 54, fptr_dest_image);
    return e_success;
}

/* To encode secret data in byte to lsb of source data */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    unsigned char mask = 1 << 7;
    for(int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data & mask) >> (7 - i));
        mask = mask >> 1;
    }
    return e_success;
}

/* To store encode secret data in source data into stego file */
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image, EncodeInfo *encInfo)
{
    for(int i = 0; i < size; i++)
    {
        fread(encInfo->image_data, sizeof(char), 8, fptr_src_image);
        encode_byte_to_lsb(data[i], encInfo->image_data);
        fwrite(encInfo->image_data, sizeof(char), 8, fptr_stego_image);
    }
    return e_success;
}

/* To encode magic string into stego file which helps in decoding */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    printf("INFO: Encoding Magic String Signature\n");
    encode_data_to_image(magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}

/* To encode the file sizes into stego file */
Status encode_size_to_lsb(char *image_buffer, int size)
{
    unsigned int mask = 1 << 31;
    for(int i = 0; i < 32; i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((size & mask) >> (31 - i));
        mask = mask >> 1;
    }
    return e_success;
}

/* To encode secret file extension size into stego file */
Status encode_secret_file_ext_size(int size, FILE *fptr_src_image, FILE *fptr_stego_image, char *secret_fname)
{
    printf("INFO: Encoding %s File Extension Size\n", secret_fname);
    char rgb[32];
    fread(rgb, sizeof(char), 32, fptr_src_image);
    encode_size_to_lsb(rgb, size);
    fwrite(rgb, sizeof(char), 32, fptr_stego_image);
    return e_success;
}

/* To encode secret file extension into stego file */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    printf("INFO: Encoding %s File Extension\n", encInfo->secret_fname);
    file_extn = ".txt";
    encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}

/* To encode secret file size into stego file */
Status encode_secret_file_size(long int file_size, EncodeInfo *encInfo)
{
    printf("INFO: Encoding %s File Size\n", encInfo->secret_fname);
    char rgb[32];
    fread(rgb, sizeof(char), 32, encInfo->fptr_src_image);
    encode_size_to_lsb(rgb, file_size);
    fwrite(rgb, sizeof(char), 32, encInfo->fptr_stego_image);
    return e_success;
}

/* To encode secret file data into stego file */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    printf("INFO: Encoding %s File Data\n", encInfo->secret_fname);
    char ch;
    fseek(encInfo->fptr_secret, 0, SEEK_SET);
    for(int i = 0; i < encInfo->size_secret_file; i++)
    {
        fread(encInfo->image_data, sizeof(char), 8, encInfo->fptr_src_image);
        fread(&ch, sizeof(char), 1, encInfo->fptr_secret);
        encode_byte_to_lsb(ch, encInfo->image_data);
        fwrite(encInfo->image_data, sizeof(char), 8, encInfo->fptr_stego_image);
    }
    return e_success;
}

/* To copy remaining data in source file into stego file */
Status copy_remaining_img_data(FILE *fptr_src_image, FILE *fptr_stego_image)
{
    printf("INFO: Copying Left Over Data\n");
    char ch;
    while(fread(&ch, 1, 1, fptr_src_image) > 0)
    {
        fwrite(&ch, 1, 1, fptr_stego_image);
    }
    return e_success;
}

/* To call required function to perform encoding */
Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo) == e_success)
    {
        printf("INFO: Done\n");
        printf("INFO: ## Encoding Procedure Started ##\n");
        if(check_capacity(encInfo) == e_success)
        {
            if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("INFO: Done.\n");
                if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("INFO: Done.\n");
                    if(encode_secret_file_ext_size(strlen(".txt"), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo->secret_fname) == e_success)
                    {
                        printf("INFO: Done.\n");
                        if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            printf("INFO: Done.\n");
                            if(encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("INFO: Done.\n");
                                if(encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("INFO: Done.\n");
                                    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                                    {
                                        printf("INFO: Done.\n");
                                    }
                                    else
                                    {
                                        printf("INFO: Failed to Copy remaining data.\n");
                                    }
                                }
                                else
                                {
                                    printf("INFO: Failed to Encode secret file data.\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("INFO: Failed to Encode secret file size.\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf("INFO: Failed to Encode secret file extension.\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("INFO: Failed to Encode secret file extension size.\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("INFO: Failed to encode magic string.\n");
                    return e_failure;
                }
            }
            else
            {
                printf("INFO: Failed to copy the header.\n");
                return e_failure;
            }
        }
        else
        {
            return e_failure;
        }
    }
    else
    {
        printf("INFO: Failed to open files.\n");
        return e_failure;
    }
    
    return e_success;
}
