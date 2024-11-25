#include "decode.h"

/* To cheak whether all the required files are mentioned for decoding */
Status read_and_validate_input_args(char *argv[], DecodeInfo *decInfo)
{
    // Read and validate input image filename
    if(argv[2] != NULL && strcmp(strstr(argv[2], "."), ".bmp") == 0)
    {
        decInfo->steg_img_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    if(argv[3] != NULL && strcmp(strstr(argv[3], "."), ".txt") == 0)
    {
        decInfo->sec_out_fname = argv[3];
        decInfo->out_arg_flag = 1;
    }
    else
    {
        decInfo->out_arg_flag = 0;
    }

    // If no failure occured
    return e_success;
}

/* To cheak whether all the required files are mentioned for decoding */
Status read_and_validate_output_args(DecodeInfo *decInfo)
{
    // Read and validate output filename
    if(!(decInfo->out_arg_flag))
    {
        printf("INFO: Output File not mentioned. Creating decoded.txt as default\n");
        decInfo->sec_out_fname = "decoded.txt";
    }
    
    return e_success;
}

/* To open required files in required mode of operation for decoding */
Status open_input_files(DecodeInfo *decInfo)
{
    printf("INFO: Opening required files\n");
    // Src Image file
    decInfo->fptr_steg_img = fopen(decInfo->steg_img_fname, "r");
    printf("INFO: Opened %s\n", decInfo->steg_img_fname);

    // Do Error handling
    if (decInfo->fptr_steg_img == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->steg_img_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/* To open required files in required mode of operation for decoding */
Status open_output_files(DecodeInfo *decInfo)
{
    // Secret file
    decInfo->fptr_sec_out = fopen(decInfo->sec_out_fname, "w");
    printf("INFO: Opened %s\n", decInfo->sec_out_fname);

    // Do Error handling
    if (decInfo->fptr_sec_out == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->sec_out_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/* To decode bytes from stego image into secret data */
Status decode_byte_to_data(char *image_buffer, DecodeInfo *decInfo)
{
    decInfo->sec_data = 0x00;
    for(int i = 0; i < 8; i++)
    {
        decInfo->sec_data = decInfo->sec_data | ((image_buffer[i] & 0x01) << (7 - i));
    }

    return e_success;
}

/* To decode magic string from stego image data */
Status decode_magic_string(char *magic_str, DecodeInfo *decInfo)
{
    printf("INFO: Decoding Magic String Signature\n");
    char mg_str[2];
    fseek(decInfo->fptr_steg_img, 54, SEEK_SET);
    for(int i = 0; i < strlen(magic_str); i++)
    {
        fread(decInfo->steg_data_buf, sizeof(char), 8, decInfo->fptr_steg_img);
        decode_byte_to_data(decInfo->steg_data_buf, decInfo);
        mg_str[i] = decInfo->sec_data;
    }
    
    //printf("Magic string = %s\n", mg_str);

    if(strcmp(mg_str, MAGIC_STRING) == 0)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

/* To decode secret file extension size from stego image data */
Status decode_secret_ext_size(DecodeInfo *decInfo)
{
    printf("INFO: Decoding Output File Extension Size\n");
    char rgb[32];
    fread(rgb, sizeof(char), 32, decInfo->fptr_steg_img);

    for(int i = 0; i < 31; i++)
    {
        decInfo->sec_ext_size = decInfo->sec_ext_size | ((rgb[i] & 1) << (31 - i));
    }

    //printf("sec_ext_size = %d\n", decInfo->sec_ext_size);

    return e_success;
}

/* To decode secret file extension from stego image data */
Status decode_secret_ext(DecodeInfo *decInfo)
{
    printf("INFO: Decoding Output File Extension\n");
    char sec_ext[decInfo->sec_ext_size + 1];
    for(int i = 0; i < decInfo->sec_ext_size; i++)
    {
        fread(decInfo->steg_data_buf, sizeof(char), 8, decInfo->fptr_steg_img);
        decode_byte_to_data(decInfo->steg_data_buf, decInfo);
        sec_ext[i] = decInfo->sec_data;
    }
    sec_ext[decInfo->sec_ext_size] = '\0';

    //printf("Secret file extension = %s\n", sec_ext);
    
    if(strcmp(sec_ext, ".txt") == 0)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

/* To decode secret file data size from stego image data */
Status decode_secret_data_size(DecodeInfo *decInfo)
{
    printf("INFO: Decoding %s File Size\n", decInfo->sec_out_fname);
    char rgb[32];
    fread(rgb, sizeof(char), 32, decInfo->fptr_steg_img);

    for(int i = 0; i < 32; i++)
    {
        decInfo->sec_data_size = decInfo->sec_data_size | ((rgb[i] & 1) << (31 - i));
    }

    //printf("secret data size = %ld\n", decInfo->sec_data_size);

    return e_success;
}

/* To decode secret file data from stego image data */
Status decode_secret_data(DecodeInfo *decInfo)
{
    printf("INFO: Decoding %s File Data\n", decInfo->sec_out_fname);
    for(int i = 0; i < decInfo->sec_data_size; i++)
    {
        fread(decInfo->steg_data_buf, sizeof(char), 8, decInfo->fptr_steg_img);
        decode_byte_to_data(decInfo->steg_data_buf, decInfo);
        fwrite(&(decInfo->sec_data), sizeof(char), 1, decInfo->fptr_sec_out);
    }

    return e_success;
}

/* To call required function to decode the secret data from stego image */
Status do_decoding(DecodeInfo *decInfo)
{
    if(open_input_files(decInfo) == e_success)
    {
        if(decode_magic_string(MAGIC_STRING, decInfo) == e_success)
        {
            printf("INFO: Done\n");
            if(decode_secret_ext_size(decInfo) == e_success)
            {
                printf("INFO: Done\n");
                if(decode_secret_ext(decInfo) == e_success)
                {
                    printf("INFO: Done\n");
                    if(read_and_validate_output_args(decInfo) == e_success)
                    {
                        if(open_output_files(decInfo) == e_success)
                        {
                            printf("INFO: Done. Opened all required files\n");
                            if(decode_secret_data_size(decInfo) == e_success)
                            {
                                printf("INFO: Done\n");
                                if(decode_secret_data(decInfo) == e_success)
                                {
                                    printf("INFO: Done\n");
                                }
                                else
                                {
                                    printf("INFO: Failed to Decode Secret data\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("INFO: Failed to Decode secret data size\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf("INFO: Error in opening output files\n");
                            return e_failure;
                        }
                        
                    }
                    else
                    {
                        printf("INFO: Error in validating output Arguments\n");
                        return e_failure;
                    }
                    
                }
                else
                {
                    printf("INFO: Failed to Decode secret file extension\n");
                    return e_failure;
                }
            }
            else
            {
                printf("INFO: Failed to Decode secret file extension size\n");
                return e_failure;
            }
        }
        else
        {
            printf("INFO: Magic string signature not found\n");
            return e_failure;
        }
    }
    else
    {
        printf("INFO: Error in opening input files\n");
        return e_failure;
    }

    return e_success;
}
