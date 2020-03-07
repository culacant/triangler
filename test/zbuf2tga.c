#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int data[1920*1080];

void txt_to_data(const char *filename)
{
	FILE *fp = fopen(filename, "r");
    if(!fp)
    {
        printf("Can't open file %s\n", filename);
        return;
    }
    for(int i=0;i< 1920*1080;i++)
    {
		fscanf(fp, "%i, ", &data[i]);
    }
	fclose(fp);
}

void zbuf_to_tga(const char *filename)
{
    char header[18] = {0};
    short width = (short)1920;
    short height = (short)1080;
    char *colordata = calloc(sizeof(unsigned char), 1920*1080*4);

    FILE *fp = fopen(filename, "wb");

    if(!fp)
    {
        printf("Can't open file %s\n", filename);
        return;
    }
    memcpy(&header[12], &width, sizeof(char)*2);
    memcpy(&header[14], &height, sizeof(char)*2);
    header[2] = 2;      // image type
    header[16] = 32;    // BPP

    int cnt = 0;
    for(int i=0;i< 1920*1080;i++)
    {
        colordata[cnt++] = (char)data[i];
        colordata[cnt++] = (char)data[i];
        colordata[cnt++] = (char)data[i];
        colordata[cnt++] = 255;
    }
    fwrite(header, sizeof(char), 18, fp);
    fwrite(colordata, sizeof(char), 1920*1080* 4, fp);

    fclose(fp);
    free(colordata);

    return;
}


int main() 
{
	txt_to_data("zbuf.txt");
    for(int i=0;i< 1920*1080;i++)
		printf("%i, ", data[i]);
//	zbuf_to_tga("zbuf.txt");
	return 0;
}
