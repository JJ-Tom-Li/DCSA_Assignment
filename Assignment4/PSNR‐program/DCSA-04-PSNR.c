/**
 * DCSA Assignmen4 Color Transfer And Recover
 * This program is used to caculating PSNR between source bmps and ct-recover bmps.
 * 4104056004 資工四 李家駿
*/
#include <stdio.h>
#include <stdlib.h>
#include "../mybmp.h"
#include <math.h>

double get_PSNR(bmpfile,bmpfile);
void PSNR_to_csv(char*,char*,double);
int main(int argc, char *argv[])
{
    int i=1;
    DIR* src_bmp_dir,*rev_bmp_dir;
    char bmp_filename[80],*image_raw; 
    struct dirent *src_ptr,*rev_ptr;
    double PSNR;
    bmpfile src_bmp,rev_bmp;

    if(argc<=2)
    {
        //No parameter
        printf("Please enter at least two parameter.\n\tUsage: ./PROGRAM_NAME [SOURCE_DIRECTORY_NAME] [RECOVER_SOURCE_DIRECTORY_NAME]\n");
        return 0;
    }

    src_bmp_dir = opendir(argv[1]);     //Read the source dir.
    rev_bmp_dir = opendir(argv[2]);     //Read the recover_source dir.
    if(!src_bmp_dir || !rev_bmp_dir)
    {
        printf("Open directory error.Please check if the directory exists.\n");
        return 0;
    }

    while((src_ptr=readdir(src_bmp_dir))!=NULL )
    {
        rev_ptr=readdir(rev_bmp_dir);   //read recover_source bmp file from directory.
        if(strlen(src_ptr->d_name)<=4 || is_bmp(src_ptr->d_name)!=0
            || strlen(rev_ptr->d_name)<=4 || is_bmp(rev_ptr->d_name)!=0 ) continue; //無效檔案
        
        //read source bmp file.
        strcpy(bmp_filename,argv[1]);           //Combine the bmp source file name.
        strcat(bmp_filename,"/");
        strcat(bmp_filename,src_ptr->d_name);
        src_bmp=readbmp(bmp_filename);          //read bmp file.
        printf("read %s\n",bmp_filename);

        //read rev_source bmp file.
        strcpy(bmp_filename,argv[2]);           //Combine the bmp rev_source file name.
        strcat(bmp_filename,"/");
        strcat(bmp_filename,rev_ptr->d_name);
        rev_bmp=readbmp(bmp_filename);          //read bmp file.
        printf("read %s\n",bmp_filename);

        //color transfer recover
        //rev_bmp=bmp_colortransfer_recover(src_bmp,tar_bmp,res_bmp);

        //Calculating PSNR
        PSNR = get_PSNR(src_bmp,rev_bmp);
        printf("PSNR:%lf\n",PSNR);

        //Free the memory to avoid memory overflow.
        free(src_bmp.image_raw);
        free(rev_bmp.image_raw);

        //Combine the xlsx file name.
        strcpy(bmp_filename,"PSNR_result");
        char tmp[0];
        tmp[0]=(char)(i++ + '0');
        tmp[1]='\0';
        strcat(bmp_filename,tmp);
        strcat(bmp_filename,".csv");
        printf("output %s\n",bmp_filename);

        //Output result to xlsx file.
        PSNR_to_csv(bmp_filename,"../PSNR_result",PSNR); 

    }

    //Close the DIR stream.
    closedir(src_bmp_dir);
    closedir(rev_bmp_dir);
    return 0;
}
double get_PSNR(bmpfile src_bmp,bmpfile rev_bmp)
{
    int width=src_bmp.width,height=src_bmp.height,i,j,k,sumR=0,sumG=0,sumB=0,tmp;
    int padding = rev_bmp.width%4;
    double PSNR=0.0;
    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            tmp=i*(width*3+padding)+j*3;
            sumR+=pow((src_bmp.image_raw[tmp+2]-rev_bmp.image_raw[tmp+2]),2);
            sumG+=pow((src_bmp.image_raw[tmp+1]-rev_bmp.image_raw[tmp+1]),2);
            sumB+=pow((src_bmp.image_raw[tmp+0]-rev_bmp.image_raw[tmp+0]),2);
        }
    }
    PSNR=(sumR+sumG+sumB)/(width*height);
    PSNR=PSNR/3;
    PSNR=20.0*log10((double)255/sqrt(PSNR));
    return PSNR;
}
void PSNR_to_csv(char*filename,char* dirname,double PSNR) 
{
    char bmp_filename[80];
    FILE *fp;
    DIR *dir;
    int i;
    //Check if the directory is exists.
    dir=opendir(dirname);
    if(dir)
    {
        closedir(dir);
    }
    else
    {
        mkdir(dirname);
        closedir(dir);
    }
    
    //Generate the filename.
    strcpy(bmp_filename,dirname);
    strcat(bmp_filename,"/");
    strcat(bmp_filename,filename);

    fp = fopen(bmp_filename,"w");
    if(!fp)
    {
        printf("File %s can't be opened.\n",bmp_filename);
        return;
    }
    fprintf(fp,"PSNR\n");
    fprintf(fp,"%lf\n",PSNR);

    fclose(fp);

    //char command[80]="del ";
    //strcat(command,filename);
    //system(command);
}
