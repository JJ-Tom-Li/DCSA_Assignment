/**
 * DCSA Assignmen1
 * 4104056004 資工四 李家駿
 * Usage: ./4104056004-李家駿-DCSA-01 [PICTURE_NAME]
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> 
#include <string.h>

typedef struct _bmpheader{
    #pragma pack(1)                 //Alignment
    unsigned short identifier;      // 0x0000
    unsigned int filesize;          // 0x0002
    unsigned int reserved;          // 0x0006
    unsigned int bitmap_dataoffset; // 0x000A
    unsigned int bitmap_headersize; // 0x000E
    unsigned int width;             // 0x0012
    int height;                     // 0x0016
    unsigned short planes;          // 0x001A
    unsigned short bits_perpixel;   // 0x001C
    unsigned int compression;       // 0x001E
    unsigned int bitmap_datasize;   // 0x0022
    unsigned int hresolution;       // 0x0026
    unsigned int vresolution;       // 0x002A
    unsigned int usedcolors;        // 0x002E
    unsigned int importantcolors;   // 0x0032
    unsigned int palette;           // 0x0036
} bmpheader;

int readbmp(char*);
//void printheader(bmpheader);
void countRGBnumber(char*,int width,int height,int[256][3]);
void countRGBmean(int[256][3],double[3],int);
void countRGBstd(int[256][3],double[3],double[3],int);
//void outputbmp(char*,bmpheader,char*);
void outputcsv(char*,int[256][3],double [3],double[3]);
int main(int argc, char *argv[])
{
    int i,c;
    if(argc==1)
    {
        //No parameter
        printf("Please enter at least one parameter.\n\tUsage: ./PROGRAM_NAME [PICTURE_NAME]\n");
        return 0;
    }
    for (i=1;i<argc;i++)
    {
        c=readbmp(argv[i]);
        if(c==-1)
            printf("Error.\n");
    }

    return 0;
}
int readbmp(char* filename)
{
    FILE *fp;
    bmpheader bmp_h;
    /*1.Read bmp file stream.*/
    fp = fopen(filename,"rb");
    if(!fp)
    {
        printf("File %s doesn't exist.\n",filename);
        return -1;
    }

    /*2.Read bmp header.*/
    fread((char*)&bmp_h,sizeof(char),sizeof(bmpheader),fp);
    //printheader(bmp_h);
    if((bmp_h.width*bmp_h.height*3)!=(bmp_h.filesize-bmp_h.bitmap_dataoffset)) //Error check
    {
        printf("BMP file checking faliure.\n");
        return -1;
    }

    /*3.Read bmp image raw.*/
    char image_raw[bmp_h.bitmap_datasize];
    fread(image_raw,sizeof(char),bmp_h.bitmap_datasize,fp);

    // output a copy bmp file.
    //outputbmp(filename,bmp_h,image_raw);

    fclose(fp);

    /*4.count number of every color channel ,means and stds*/
    int result[256][3]={0};
    double mean[3]={0};
    double std[3]={0};
    countRGBnumber(image_raw,bmp_h.width,bmp_h.height,result);    //count the number of RGB channel.
    countRGBmean(result,mean,bmp_h.width*bmp_h.height);          //Get mean of RGB
    countRGBstd(result,mean,std,bmp_h.width*bmp_h.height);      //Get std of RGB

    /*5.Output result to csv file*/
    outputcsv(filename,result,mean,std);

    return 0;
}
/*
void printheader(bmpheader bmp_h)
{
    printf("identifier:%x\n",bmp_h.identifier);
    printf("filesize:%x\n",bmp_h.filesize);
    printf("reserved:%x\n",bmp_h.reserved);
    printf("bitmap_dataoffset:%x\n",bmp_h.bitmap_dataoffset);
    printf("bitmap_headersize%x\n",bmp_h.bitmap_headersize);
    printf("width:%x\n",bmp_h.width);
    printf("height:%x\n",bmp_h.height);
    printf("planes:%x\n",bmp_h.planes);
    printf("bits_perpixel:%x\n",bmp_h.bits_perpixel);
    printf("compression:%x\n",bmp_h.compression);
    printf("bitmap_datasize:%x\n",bmp_h.bitmap_datasize);
    printf("hresolution:%x\n",bmp_h.hresolution);
    printf("vresolution:%x\n",bmp_h.vresolution);
    printf("usedcolors:%x\n",bmp_h.usedcolors);
    printf("importantcolors:%x\n",bmp_h.importantcolors);
    printf("palette:%x\n",bmp_h.palette);
}*/
void countRGBnumber(char* image_raw,int width,int height,int result[256][3])
{
    int i,j;
    unsigned char R,G,B;
    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            R=image_raw[3*(i*width+j)+0];
            G=image_raw[3*(i*width+j)+1];
            B=image_raw[3*(i*width+j)+2];
            result[R][1]++;
            result[G][0]++;
            result[B][2]++;
        }
    }
}
void countRGBmean(int result[256][3],double mean[3],int totalnumber)
{
    int i;
    for (i=0;i<256;i++)
    {
        mean[0]+=result[i][0]*(i+1);
        mean[1]+=result[i][1]*(i+1);
        mean[2]+=result[i][2]*(i+1);
    }
    mean[0]/=totalnumber;
    mean[1]/=totalnumber;
    mean[2]/=totalnumber;
}
void countRGBstd(int result[256][3],double mean[3],double std[3],int totalnumber)
{
    int i;
    for (i=0;i<256;i++)
    {
        std[0]+=(i+1-mean[0])*(i+1-mean[0])*result[i][0];
        std[1]+=(i+1-mean[1])*(i+1-mean[1])*result[i][1];
        std[2]+=(i+1-mean[2])*(i+1-mean[2])*result[i][2];
    }
    std[0] = sqrt(std[0]/totalnumber);
    std[1] = sqrt(std[1]/totalnumber);
    std[2] = sqrt(std[2]/totalnumber);
}
/*
void outputbmp(char*filename,bmpheader bmp_h,char* image_raw)
{
    char bmp_filename[80];
    FILE *fp;
    int i;
    //Generate the filename.
    for(i=0;i<strlen(filename)-4;i++)
        bmp_filename[i]=filename[i];
    bmp_filename[i]='\0';
    strcat(bmp_filename,"_output.bmp");

    fp=fopen(bmp_filename,"wb");
    if(!fp)
    {
        printf("File %s can't be opened.\n",bmp_filename);
        return;
    }
    fwrite((char*)&bmp_h,sizeof(char),sizeof(bmpheader),fp);
    fwrite(image_raw,sizeof(char),bmp_h.bitmap_datasize,fp);
    fclose(fp);
}*/
void outputcsv(char*filename,int result[256][3],double mean[3],double std[3])
{
    char csv_filename[80];
    FILE *fp;
    int i,j;
    //Generate the filename.
    for(i=0;i<strlen(filename)-4;i++)
        csv_filename[i]=filename[i];
    csv_filename[i]='\0';
    strcat(csv_filename,"_hist.csv");

    fp = fopen(csv_filename,"w");
    if(!fp)
    {
        printf("File %s can't be opened.\n",csv_filename);
        return;
    }
    fprintf(fp,"Index,Red,Green,Blue\n");
    for (i=0;i<256;i++)
    {
        //printf("index:%d,",i);
        fprintf(fp,"%d,",i);
        for (j=0;j<3;j++)
            fprintf(fp,"%d,",result[i][j]);
        fprintf(fp,"\n");
    }
    fprintf(fp,"Means,%.2lf,%.2lf,%.2lf\n",mean[0],mean[1],mean[2]);
    fprintf(fp,"STD,%.2lf,%.2lf,%.2lf\n",std[0],std[1],std[2]);

    fclose(fp);
}