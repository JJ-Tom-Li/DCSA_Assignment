/**
 * DCSA Assignmen4 Color Transfer And Recover
 * This program is used to doing color transfer recovering of bmps.
 * 4104056004 資工四 李家駿
*/
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "../mybmp.h"
bmpfile bmp_colortransfer_recover(bmpfile,bmpfile,bmpfile);
int main(int argc, char *argv[])
{
    int i=1;
    DIR* src_bmp_dir,*tar_bmp_dir,*res_bmp_dir;
    char bmp_filename[80],*image_raw; 
    struct dirent *src_ptr,*tar_ptr,*res_ptr;
    
    bmpfile src_bmp,tar_bmp,res_bmp,rev_bmp;
    if(argc<=3)
    {
        //No parameter
        printf("Please enter at least three parameter.\n\tUsage: ./PROGRAM_NAME [SOURCE_DIRECTORY_NAME] [TARGET_DIRECTORY_NAME] [RESULT_DIRECTORY_NAME]\n");
        return 0;
    }

    src_bmp_dir = opendir(argv[1]);     //Read the source dir.
    tar_bmp_dir = opendir(argv[2]);     //Read the target dir.
    res_bmp_dir = opendir(argv[3]);     //Read the result dir.
    if(!src_bmp_dir || !tar_bmp_dir || !res_bmp_dir)
    {
        printf("Open directory error.Please check if the directory exists.\n");
        return 0;
    }

    while((src_ptr=readdir(src_bmp_dir))!=NULL )//|| (tar_ptr=readdir(tar_bmp_dir))!=NULL)
    {
        tar_ptr=readdir(tar_bmp_dir);   //read target bmp file from directory.
        res_ptr=readdir(res_bmp_dir);   //read result bmp file from directory.
        if(strlen(src_ptr->d_name)<=4 || is_bmp(src_ptr->d_name)!=0 
            || strlen(tar_ptr->d_name)<=4 || is_bmp(tar_ptr->d_name)!=0
            || strlen(res_ptr->d_name)<=4 || is_bmp(res_ptr->d_name)!=0 ) continue; //無效檔案
        
        //read source bmp file.
        strcpy(bmp_filename,argv[1]);           //Combine the bmp source file name.
        strcat(bmp_filename,"/");
        strcat(bmp_filename,src_ptr->d_name);
        src_bmp=readbmp(bmp_filename);          //read bmp file.
        printf("read %s\n",bmp_filename);

        //read target bmp file.
        strcpy(bmp_filename,argv[2]);           //Combine the bmp target file name.
        strcat(bmp_filename,"/");
        strcat(bmp_filename,tar_ptr->d_name);
        tar_bmp=readbmp(bmp_filename);          //read bmp file.
        printf("read %s\n",bmp_filename);

        //Free the memory to avoid memory overflow.
        free(src_bmp.image_raw);
        free(tar_bmp.image_raw);

        //read result bmp file.
        strcpy(bmp_filename,argv[3]);           //Combine the bmp target file name.
        strcat(bmp_filename,"/");
        strcat(bmp_filename,res_ptr->d_name);
        res_bmp=readbmp(bmp_filename);          //read bmp file.
        printf("read %s\n",bmp_filename);

        //color transfer recover
        rev_bmp=bmp_colortransfer_recover(src_bmp,tar_bmp,res_bmp);

        //Free the memory to avoid memory overflow.
        free(res_bmp.image_raw);

        //Combine the target bmp file name.
        strcpy(bmp_filename,"rs");
        char tmp[0];
        tmp[0]=(char)(i++ + '0');
        tmp[1]='\0';
        strcat(bmp_filename,tmp);
        strcat(bmp_filename,".bmp");
        printf("output %s\n",bmp_filename);

        /*bmp header*/
        copyheader_ftof(&rev_bmp,res_bmp);

        //Output result to bmp file.
        outputbmp(bmp_filename,"../recov_source",rev_bmp,rev_bmp.image_raw);
        
        //Free the image raw memory.
        free(rev_bmp.image_raw);
    }

    //Close the DIR stream.
    closedir(src_bmp_dir);
    closedir(tar_bmp_dir);
    closedir(res_bmp_dir);
    return 0;
}
bmpfile bmp_colortransfer_recover(bmpfile src_bmp,bmpfile tar_bmp,bmpfile res_bmp)
{
    bmpfile rev_bmp;
    rev_bmp.image_raw = (unsigned char*)malloc(res_bmp.bitmap_datasize);
    int i,j;

    /*Image Raw color transfer.*/
    int R,G,B;
    int padding = res_bmp.width%4;
    int tmp;
    for(i=0;i<res_bmp.height;i++)
    {
        for(j=0;j<res_bmp.width;j++)
        {
            tmp=i*(src_bmp.width*3+padding)+j*3;
            R=(int)floor((src_bmp.std[0]/tar_bmp.std[0])*((double)res_bmp.image_raw[tmp+2]-tar_bmp.mean[0]-0.5)+src_bmp.mean[0]);
            G=(int)floor((src_bmp.std[1]/tar_bmp.std[1])*((double)res_bmp.image_raw[tmp+1]-tar_bmp.mean[1]-0.5)+src_bmp.mean[1]);
            B=(int)floor((src_bmp.std[2]/tar_bmp.std[2])*((double)res_bmp.image_raw[tmp+0]-tar_bmp.mean[2]-0.5)+src_bmp.mean[2]);
            
            //Check if the value is in[0,255]/*
            if(R>255) R=255; 
            if(G>255) G=255;
            if(B>255) B=255;
            if(R<0) R=0;
            if(G<0) G=0;
            if(B<0) B=0;

            //printf("%d %d %d\n",R,G,B);
            //Store the value
            rev_bmp.image_raw[tmp+2]=(unsigned char)R;
            rev_bmp.image_raw[tmp+1]=(unsigned char)G;
            rev_bmp.image_raw[tmp+0]=(unsigned char)B;
        }
    }
    //Turn the LAB back to RGB.
    //LABtoRGB(res_bmp.image_raw,src_bmp.height,src_bmp.width);
    return rev_bmp;
}