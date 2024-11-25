#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"

int main(int argc, char *argv[])
{
    if(check_operation_type(argv) == e_encode)
    {
        EncodeInfo encInfo;
        //printf("Encoding Selected.\n");
        if(read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            if(do_encoding(&encInfo) == e_success)
            {
                printf("INFO: ## Encoding Done Successfully ##\n");
                fclose(encInfo.fptr_src_image);
                fclose(encInfo.fptr_stego_image);
                fclose(encInfo.fptr_secret);
            }
            else
            {
                printf("INFO: ## Encoding Failed ##\n");
            }
        }
        else
        {
            printf("INFO: Error in Validating Arguments\n");
        }        
    }
    else if(check_operation_type(argv) == e_decode)
    {
        DecodeInfo decInfo;
        printf("INFO: ## Decoding Procedure Started ##\n");
        if(read_and_validate_input_args(argv, &decInfo) == e_success)
        {
            if(do_decoding(&decInfo) == e_success)
            {
                printf("INFO: ## Decoding Done Successfully ##\n");
                fclose(decInfo.fptr_steg_img);
                fclose(decInfo.fptr_sec_out);
            }
            else
            {
                printf("INFO: Decoding failed\n");
            }
        }
        else
        {
            printf("INFO: Error in Validating input Arguments\n");
        }        
    }
    else
    {
        printf("Error!\nUSAGE:\n");
        printf("Encoding:  ./a.out -e beautiful.bmp secret.txt\n");
        printf("Decoding:  ./a.out -d stego.bmp\n");
    }
    return 0;
}

/* To determine which operation to be done based on Command Line Argument */
OperationType check_operation_type(char *argv[])
{
    if(argv[1] != NULL && strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if(argv[1] != NULL && strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
