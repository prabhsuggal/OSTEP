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
        printf("wzip: file1 [file2 ...]\n");
        exit(1);
    }
    default:
    {
        int count = 0;
        char str;
        bool one_byte_read = false;
        for (int i = 1; i < argc; i++)
        {
            FILE *p = fopen(argv[i], "r");
            if (p == NULL)
            {
                printf("wzip: cannot open file\n");
                exit(1);
            }

            char buff;
            if ((buff = fgetc(p)) != EOF)
            {
                if (!one_byte_read)
                {
                    str = buff;
                    count++;
                    one_byte_read = true;
                }
                else
                {
                    if (str == buff)
                        count++;
                    else
                    {
                        fwrite(&count, sizeof(count), 1, stdout);
                        fwrite(&str, sizeof(str), 1, stdout);
                        str = buff;
                        count = 1;
                    }
                }
            }
            while ((buff = fgetc(p)) != EOF)
            {
                if (str == buff)
                {
                    count++;
                }
                else
                {
                    fwrite(&count, sizeof(count), 1, stdout);
                    fwrite(&str, sizeof(str), 1, stdout);
                    str = buff;
                    count = 1;
                }
            }
            fclose(p);
        }
        if (count != 0)
        {
            fwrite(&count, sizeof(count), 1, stdout);
            fwrite(&str, sizeof(str), 1, stdout);
        }
    }
    }

    return 0;
}
