#include <stdio.h>
#include <stdlib.h>

#include "TokenArray.h"

#define FLASH_SIZE 8096

int main(int argc, char *argv[]) 
{
    if (argc != 4)
    {
        printf("Usage:\n\t%s <input file> <output bin file> <output hex file>\n", argv[0]);
        return 1;
    }

    FILE *asmFile = fopen(argv[1], "rb");
    if (!asmFile)
    {
        printf("Invalid input file (%s).\n", argv[1]);
        return 1;
    }

    fseek(asmFile, 0, SEEK_END);
    size_t len = ftell(asmFile);
    fseek(asmFile, 0, SEEK_SET);

    printf("Length of %s is %lu bytes.\n", argv[1], len);
    TokenArray tokenList(asmFile, len);
    fclose(asmFile);
    tokenList.ResolveLabels();
    tokenList.Dump();

    FILE *binFile = fopen(argv[2], "wb");
    if (!binFile)
    {
        printf("Can't open output file %s.\n", argv[2]);
        return 1;
    }

    FILE *hexFile = fopen(argv[3], "wb");
    if (!hexFile)
    {
        printf("Can't open output file %s.\n", argv[3]);
        return 1;
    }

    tokenList.Make() ? printf("\x1b[32mAssemblied successfully.\x1b[0m\n") : printf("\x1b[31mAssembly failed.\x1b[0m\n");
    printf("Size of Intel hex file is %lu bytes.\n", tokenList.MakeHex(hexFile));
    size_t binFileSize = tokenList.MakeBin(binFile);
    float percentFilled = binFileSize * 100.0 / FLASH_SIZE;
    printf("Size of binary image is %lu bytes / %u bytes (%4.2f%%).\n", binFileSize, FLASH_SIZE, percentFilled);

    fclose(binFile);
    fclose(hexFile);

    return 0;
}
