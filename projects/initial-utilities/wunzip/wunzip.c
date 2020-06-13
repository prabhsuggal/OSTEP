#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char *argv[])
{

    switch (argc)
    {
    case 1:
    {
        printf("wunzip: file1 [file2 ...]\n");
        exit(1);
    }
    default:
    {
        for (int i = 1; i < argc; i++)
        {
            FILE *p = fopen(argv[i], "r");
            if (p == NULL)
            {
                printf("wunzip: cannot open file\n");
                exit(1);
            }

            int count = 0;
            char str;
            while (fread(&count, sizeof(int), 1, p) == 1)
            {
                fread(&str, sizeof(char), 1, p);
                while(count-- > 0){
                    fwrite(&str, sizeof(str), 1, stdout);
                }
            }
            fclose(p);
        }
    }
    }

    return 0;
}