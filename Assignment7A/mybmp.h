/**
 * DCSA Assignmen7 
 * 4104056004 資工四 李家駿
 * mybmp.h
 * Used to read,process and output bmp files.
*/
#include <math.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>

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

typedef struct _bmpfile{
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
    unsigned char *image_raw;                //Image Raw
    int result[256][3];             //Number of every channel
    double mean[3];                 //Mean of RGB
    double std[3];                  //STD of RGB
    unsigned char max[3];                     //Max value of RGB
    unsigned char min[3];                     //Min value of RGB
} bmpfile;

static int src_count = 1;
int is_bmp(char*);
bmpfile readbmp(char*);
//bmpfile bmp_colortransfer(bmpfile,bmpfile);   
void bmp_GWcolortransfer(bmpfile,bmpfile); 
void copyheader_htof(bmpfile*,bmpheader);
void copyheader_ftoh(bmpheader*,bmpfile);
void copyheader_ftof(bmpfile*,bmpfile);
void printheader(bmpheader);
void countRGBnumber(unsigned char*,int width,int height,int[256][3]);
void countRGBmean(int[256][3],double[3],int);
void countRGBstd(int[256][3],double[3],double[3],int);
double countColorDistance(unsigned char*,unsigned char*,int,int);
void outputbmp(char*,char*,bmpfile,unsigned char*);
//void outputcsv(char*,int[256][3],double [3],double[3]);

int is_bmp(char* filename)
{
    int i,len=strlen(filename);
    char *check="pmb";
    i=len-1;
    while(filename[i]!='.' && filename[i]==check[len-i-1]) i--;
    if(len-i-1==3) return 0; //0 means it's bmp file.
    else return 1;
}
bmpfile readbmp(char* filename)
{
    FILE *fp;
    bmpheader bmp_h;
    bmpfile bmp_f;
    int i,j;
    /*1.Read bmp file stream.*/
    fp = fopen(filename,"rb");
    if(!fp)
    {
        printf("File %s doesn't exist.\n",filename);
        exit(0);
    }

    /*2.Read bmp header.*/
    fread((char*)&bmp_h,sizeof(char),sizeof(bmpheader),fp);
    //printheader(bmp_h);
    if((bmp_h.width*bmp_h.height*3)!=(bmp_h.filesize-bmp_h.bitmap_dataoffset)) //Error check
    {
        printf("BMP file checking faliure.\n");
        exit(0);
    }
    copyheader_htof(&bmp_f,bmp_h);
    /*3.Read bmp image raw.*/
    bmp_f.bitmap_datasize-=18;
    unsigned char image_raw_tmp[bmp_f.bitmap_datasize];
    fseek(fp, (long int)(bmp_f.bitmap_dataoffset-bmp_f.bitmap_headersize), SEEK_CUR);
    fread(image_raw_tmp,sizeof(char),bmp_f.bitmap_datasize,fp);
    bmp_f.image_raw =(unsigned char*)malloc(bmp_f.bitmap_datasize);
    for(i=0;i<bmp_f.bitmap_datasize;i++)
        bmp_f.image_raw[i]=image_raw_tmp[i];
    free(image_raw_tmp);
    fclose(fp);

    /*4.count number of every color channel ,means and stds*/
    int result[256][3]={0};
    double mean[3]={0};
    double std[3]={0};
    unsigned char max[3]={0},min[3]={0};

    //Convert RGB to LAB
    countRGBnumber(bmp_f.image_raw,bmp_f.width,bmp_f.height,result);    //count the number of RGB channel.
    countRGBmean(result,mean,bmp_f.width*bmp_f.height);          //Get mean of RGB
    countRGBstd(result,mean,std,bmp_f.width*bmp_f.height);      //Get std of RGB
    //copy result,mean and std to bmp_h.
    for(i=0;i<256;i++)
        for(j=0;j<3;j++)
            bmp_f.result[i][j]=result[i][j];
    for(i=0;i<3;i++)
    {
        bmp_f.mean[i]=mean[i];
        bmp_f.std[i]=std[i];
        //bmp_f.max[i]=max[i];
        //bmp_f.min[i]=min[i];
    } 

    return bmp_f;
}
// bmpfile bmp_colortransfer(bmpfile src_bmp,bmpfile tar_bmp)
// {
//     bmpfile res_bmp;
//     res_bmp.image_raw = (unsigned char*)malloc(src_bmp.bitmap_datasize);
//     int i,j;

//     /*Image Raw color transfer.*/
//     int R,G,B,maxR,maxG,maxB;
//     int padding = src_bmp.width%4;
//     int tmp;
//     for(i=0;i<src_bmp.height;i++)
//     {
//         for(j=0;j<src_bmp.width;j++)
//         {
//             tmp=i*(src_bmp.width*3+padding)+j*3;
//             R=(int)floor((tar_bmp.std[0]/src_bmp.std[0])*((double)src_bmp.image_raw[tmp+2]-src_bmp.mean[0])+tar_bmp.mean[0]+0.5);
//             G=(int)floor((tar_bmp.std[1]/src_bmp.std[1])*((double)src_bmp.image_raw[tmp+1]-src_bmp.mean[1])+tar_bmp.mean[1]+0.5);
//             B=(int)floor((tar_bmp.std[2]/src_bmp.std[2])*((double)src_bmp.image_raw[tmp+0]-src_bmp.mean[2])+tar_bmp.mean[2]+0.5);

//             //Check if the value is in[0,255]/*
//             if(R>255) R=255; 
//             if(G>255) G=255;
//             if(B>255) B=255;
//             if(R<0) R=0;
//             if(G<0) G=0;
//             if(B<0) B=0;

//             //printf("%d %d %d\n",R,G,B);
//             //Store the value
//             res_bmp.image_raw[tmp+2]=(unsigned char)R;
//             res_bmp.image_raw[tmp+1]=(unsigned char)G;
//             res_bmp.image_raw[tmp+0]=(unsigned char)B;
//         }
//     }
//     //Turn the LAB back to RGB.
//     //LABtoRGB(res_bmp.image_raw,src_bmp.height,src_bmp.width);
//     return res_bmp;
// }
void bmp_GWcolortransfer(bmpfile src_bmp,bmpfile tar_bmp)    //Doing generalized weighted color transfer using color distance.
{
    bmpfile res_bmp;
    res_bmp.image_raw = (unsigned char*)malloc(src_bmp.bitmap_datasize);
    unsigned char* weight_1_image_raw=(unsigned char*)malloc(src_bmp.bitmap_datasize);
    int i,j,k,count=0;
    double weight=0.0;
    
    char bmp_filename[20],csv_filename[20];
    char tmpc[2],buffer[10];
    int R,G,B;
    int padding = src_bmp.width%4;
    int tmp;
    double OWCT[101][10];       //Store the weight,RGB mean,RGB std,source/target color distance and total TCD.
    
    /*Generate weight 1.0 color transfer bmp*/
    weight = 1.0;
    for(i=0;i<src_bmp.height;i++)
    {
        for(j=0;j<src_bmp.width;j++)
        {
            tmp=i*(src_bmp.width*3+padding)+j*3;
            R=floor(((weight*tar_bmp.std[0]+(1-weight)*src_bmp.std[0])/src_bmp.std[0])*((int)src_bmp.image_raw[tmp+2]-src_bmp.mean[0])+weight*tar_bmp.mean[0]+(1-weight)*src_bmp.mean[0]);
            G=floor(((weight*tar_bmp.std[1]+(1-weight)*src_bmp.std[1])/src_bmp.std[1])*((int)src_bmp.image_raw[tmp+1]-src_bmp.mean[1])+weight*tar_bmp.mean[1]+(1-weight)*src_bmp.mean[1]);
            B=floor(((weight*tar_bmp.std[2]+(1-weight)*src_bmp.std[2])/src_bmp.std[2])*((int)src_bmp.image_raw[tmp+0]-src_bmp.mean[2])+weight*tar_bmp.mean[2]+(1-weight)*src_bmp.mean[2]);

            //Check if the value is in[0,255]/*
            if(R>255)
            {
                // printf("R:%d\n",R);
                R=255;
            } 
            if(G>255)
            {
                //printf("G:%d\n",G);
                G=255;
            } 
            if(B>255)
            {
                // printf("B:%d\n",B);
                B=255;
                
            } 
            if(R<0){
                // printf("R:%d\n",R);
                // R=0-R;
                R=0;
            } 
            if(G<0){
                // printf("G:%d\n",G);
                // G=0-G;
                G=0;
            } 
            if(B<0){
                //printf("B:%d\n",B);
                // B=0-B;
                B=0;
            } 

            //printf("%d %d %d\n",R,G,B);
            //Store the value
            weight_1_image_raw[tmp+2]=(unsigned char)R;
            weight_1_image_raw[tmp+1]=(unsigned char)G;
            weight_1_image_raw[tmp+0]=(unsigned char)B;
        }
    }

    /*Do color transfer from weight=0.01 to weight=1.0*/
    for (count=1;count<=101;count++)
    {
        weight=(count-1)*0.01;
       // printf("%lf,%lf %lf,%lf %lf,%lf\n",(weight*tar_bmp.std[0]),(1-weight)*src_bmp.std[0],(weight*tar_bmp.std[1]),(1-weight)*src_bmp.std[1],(weight*tar_bmp.std[2]),(1-weight)*src_bmp.std[2]);
        for(i=0;i<src_bmp.height;i++)
        {
            for(j=0;j<src_bmp.width;j++)
            {
                tmp=i*(src_bmp.width*3+padding)+j*3;
                R=floor(((weight*tar_bmp.std[0]+(1-weight)*src_bmp.std[0])/src_bmp.std[0])*((int)src_bmp.image_raw[tmp+2]-src_bmp.mean[0])+weight*tar_bmp.mean[0]+(1-weight)*src_bmp.mean[0]);
                G=floor(((weight*tar_bmp.std[1]+(1-weight)*src_bmp.std[1])/src_bmp.std[1])*((int)src_bmp.image_raw[tmp+1]-src_bmp.mean[1])+weight*tar_bmp.mean[1]+(1-weight)*src_bmp.mean[1]);
                B=floor(((weight*tar_bmp.std[2]+(1-weight)*src_bmp.std[2])/src_bmp.std[2])*((int)src_bmp.image_raw[tmp+0]-src_bmp.mean[2])+weight*tar_bmp.mean[2]+(1-weight)*src_bmp.mean[2]);

                //Check if the value is in[0,255]/*
                if(R>255)
                {
                   // printf("R:%d\n",R);
                    R=255;
                } 
                if(G>255)
                {
                    //printf("G:%d\n",G);
                    G=255;
                } 
                if(B>255)
                {
                   // printf("B:%d\n",B);
                   B=255;
                   
                } 
                if(R<0){
                   // printf("R:%d\n",R);
                   // R=0-R;
                    R=0;
                } 
                if(G<0){
                   // printf("G:%d\n",G);
                   // G=0-G;
                   G=0;
                } 
                if(B<0){
                    //printf("B:%d\n",B);
                   // B=0-B;
                   B=0;
                } 

                //printf("%d %d %d\n",R,G,B);
                //Store the value
                res_bmp.image_raw[tmp+2]=(unsigned char)R;
                res_bmp.image_raw[tmp+1]=(unsigned char)G;
                res_bmp.image_raw[tmp+0]=(unsigned char)B;
            }
        }
        /*Get the informations of result bmp.*/
        int result[256][3]={0};     //count the RGB number of result bmp
        double mean[3]={0};         //count the RGB mean value of result bmp
        double std[3]={0};          //count the RGB std value of result bmp
        
        countRGBnumber(res_bmp.image_raw,src_bmp.width,src_bmp.height,result);    //count the number of RGB channel.
        countRGBmean(result,mean,src_bmp.width*src_bmp.height);          //Get mean of RGB
        countRGBstd(result,mean,std,src_bmp.width*src_bmp.height);      //Get std of RGB
        // printf("R  0:%d 255:%d\n",result[0][0],result[255][0]);
        // printf("G  0:%d 255:%d\n",result[0][1],result[255][1]);
        // printf("B  0:%d 255:%d\n",result[0][2],result[255][2]);
        double CD_src = countColorDistance(src_bmp.image_raw,res_bmp.image_raw,src_bmp.height,src_bmp.width);     //Get color distance between source and result.
        double CD_tar = countColorDistance(weight_1_image_raw,res_bmp.image_raw,src_bmp.height,src_bmp.width);     //Get color distance between target and result.
        double TCD = (double)abs(CD_src - CD_tar);
        //printf(",TCD=%lf\n",TCD);

        //Store the OWCT result.
        OWCT[count-1][0]=weight;
        OWCT[count-1][1]=mean[0];
        OWCT[count-1][2]=mean[1];
        OWCT[count-1][3]=mean[2];
        OWCT[count-1][4]=std[0];
        OWCT[count-1][5]=std[1];
        OWCT[count-1][6]=std[2];
        OWCT[count-1][7]=CD_src;
        OWCT[count-1][8]=CD_tar;
        OWCT[count-1][9]=TCD;

        //Output result to bmp file.
        //Combine the target bmp file name.
        strcpy(bmp_filename,"OWCT");
        tmpc[0]=(char)(src_count + '0');
        tmpc[1]='\0';
        strcat(bmp_filename,tmpc);
        sprintf(buffer,"-%d",count);
        strcat(bmp_filename,buffer);
        strcat(bmp_filename,".bmp");
        printf("output %s\n",bmp_filename);
        outputbmp(bmp_filename,"ct-result",src_bmp,res_bmp.image_raw);
    }
       
    /*Output csv file.*/
    //Find optimal weight
    int opti_weight=0;
    for (i=1;i<101;i++)
        if(OWCT[i][9]<OWCT[opti_weight][9])
            opti_weight=i;
     
    //Generate file name
    strcpy(csv_filename,"OWCT");
    tmpc[0]=(char)(src_count + '0');
    tmpc[1]='\0';
    strcat(csv_filename,tmpc);
    strcat(csv_filename,"-excel.csv");
    //Processing csv file.
    FILE *fp;
    fp = fopen(csv_filename,"w");
    if(!fp)
    {
        printf("File %s can't be opened.\n",csv_filename);
        return;
    }
    fprintf(fp,"Sample Pair,Weight,mean,mean,mean,std,std,std,color distance,color distance,Total,Mark,\n");
    fprintf(fp,",,red,green,blue,red,green,blue,source,target,TCD,,\n");
    for (i=0;i<101;i++)
    {
        if(i==opti_weight) //If it is the optimal weight,output an "Optimal".
            fprintf(fp,"%d,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,Optimal,\n",i,OWCT[i][0],OWCT[i][1],OWCT[i][2],OWCT[i][3],OWCT[i][4],OWCT[i][5],OWCT[i][6],OWCT[i][7],OWCT[i][8],OWCT[i][9]);
        else
            fprintf(fp,"%d,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,%.2lf,,\n",i,OWCT[i][0],OWCT[i][1],OWCT[i][2],OWCT[i][3],OWCT[i][4],OWCT[i][5],OWCT[i][6],OWCT[i][7],OWCT[i][8],OWCT[i][9]);
    }
    //close file stream.
    fclose(fp);
    free(res_bmp.image_raw);
    free(weight_1_image_raw);
    src_count++;
}
void copyheader_htof(bmpfile *h1,bmpheader h2)
{
    (*h1).identifier=h2.identifier;
    (*h1).filesize=h2.filesize;
    (*h1).reserved=h2.reserved;
    (*h1).bitmap_dataoffset=h2.bitmap_dataoffset;
    (*h1).bitmap_headersize=h2.bitmap_headersize;
    (*h1).width=h2.width;
    (*h1).height=h2.height;
    (*h1).planes=h2.planes;
    (*h1).bits_perpixel=h2.bits_perpixel;
    (*h1).compression=h2.compression;
    (*h1).bitmap_datasize=h2.bitmap_datasize;
    (*h1).hresolution=h2.hresolution;
    (*h1).vresolution=h2.vresolution;
    (*h1).usedcolors=h2.usedcolors;
    (*h1).importantcolors=h2.importantcolors;
    (*h1).palette=h2.palette;
}
void copyheader_ftoh(bmpheader *h1,bmpfile h2)
{
    (*h1).identifier=h2.identifier;
    (*h1).filesize=h2.filesize;
    (*h1).reserved=h2.reserved;
    (*h1).bitmap_dataoffset=h2.bitmap_dataoffset;
    (*h1).bitmap_headersize=h2.bitmap_headersize;
    (*h1).width=h2.width;
    (*h1).height=h2.height;
    (*h1).planes=h2.planes;
    (*h1).bits_perpixel=h2.bits_perpixel;
    (*h1).compression=h2.compression;
    (*h1).bitmap_datasize=h2.bitmap_datasize;
    (*h1).hresolution=h2.hresolution;
    (*h1).vresolution=h2.vresolution;
    (*h1).usedcolors=h2.usedcolors;
    (*h1).importantcolors=h2.importantcolors;
    (*h1).palette=h2.palette;
}
void copyheader_ftof(bmpfile *h1,bmpfile h2)
{
    (*h1).identifier=h2.identifier;
    (*h1).filesize=h2.filesize;
    (*h1).reserved=h2.reserved;
    (*h1).bitmap_dataoffset=h2.bitmap_dataoffset;
    (*h1).bitmap_headersize=h2.bitmap_headersize;
    (*h1).width=h2.width;
    (*h1).height=h2.height;
    (*h1).planes=h2.planes;
    (*h1).bits_perpixel=h2.bits_perpixel;
    (*h1).compression=h2.compression;
    (*h1).bitmap_datasize=h2.bitmap_datasize;
    (*h1).hresolution=h2.hresolution;
    (*h1).vresolution=h2.vresolution;
    (*h1).usedcolors=h2.usedcolors;
    (*h1).importantcolors=h2.importantcolors;
    (*h1).palette=h2.palette;
}
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
}

void countRGBnumber(unsigned char* image_raw,int width,int height,int result[256][3])
{
    int i,j;
    unsigned char R,G,B;
    int padding = width%4;
    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            R=image_raw[i*(width*3+padding)+j*3+2];
            G=image_raw[i*(width*3+padding)+j*3+1];
            B=image_raw[i*(width*3+padding)+j*3+0];
            result[R][0]++;
            result[G][1]++;
            result[B][2]++;
        }
    }
}
void countRGBmean(int result[256][3],double mean[3],int totalnumber)
{
    int i;
    for (i=0;i<256;i++)
    {
        mean[0]+=result[i][0]*(i);
        mean[1]+=result[i][1]*(i);
        mean[2]+=result[i][2]*(i);
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
        std[0]+=(i-mean[0])*(i-mean[0])*result[i][0];
        std[1]+=(i-mean[1])*(i-mean[1])*result[i][1];
        std[2]+=(i-mean[2])*(i-mean[2])*result[i][2];
    }
    std[0] = sqrt(std[0]/totalnumber);
    std[1] = sqrt(std[1]/totalnumber);
    std[2] = sqrt(std[2]/totalnumber);
}
double countColorDistance(unsigned char* image_raw1,unsigned char* image_raw2,int height,int width)
{
    int R,G,B;
    
    int tmp,i,j;
    double sum=0.0;
    int number = height*width;
    for(i=0;i<height;i++)
    {
        for(j=0;j<width;j++)
        {
            tmp=i*(width*3)+j*3;
            R=3*(image_raw1[tmp+2]-image_raw2[tmp+2])*(image_raw1[tmp+2]-image_raw2[tmp+2]);
            G=4*(image_raw1[tmp+1]-image_raw2[tmp+1])*(image_raw1[tmp+1]-image_raw2[tmp+1]);
            B=2*(image_raw1[tmp+0]-image_raw2[tmp+0])*(image_raw1[tmp+0]-image_raw2[tmp+0]);
            
            sum+=(double)(R+G+B);
            
        }
        //printf("(%u,%u,%u) (%u,%u,%u)\n",image_raw1[tmp+2],image_raw1[tmp+1],image_raw1[tmp+0],image_raw2[tmp+2],image_raw2[tmp+1],image_raw2[tmp+0]);
        //printf("%d %d %d %lf\n",R,G,B,sum);
        //printf("%lf\n",sum);
    }
    //printf("\n%u,%d,%d,%d\n",sum,height,width,height*width);
    sum=sum/number;
    return sum;
}
void outputbmp(char*filename,char* dirname,bmpfile bmp_f,unsigned char* image_raw)
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

    fp=fopen(bmp_filename,"wb");
    if(!fp)
    {
        printf("File %s can't be opened.\n",bmp_filename);
        return;
    }
    bmpheader bmp_h;
    copyheader_ftoh(&bmp_h,bmp_f);
    fwrite((char*)&bmp_h,sizeof(char),sizeof(bmpheader),fp);
    fseek(fp, (long int)(bmp_f.bitmap_dataoffset-bmp_f.bitmap_headersize), SEEK_CUR);
    //for (i=0;i<bmp_f.bitmap_datasize/1000;i++)
    //    printf("%d\n",image_raw[i]);
    fwrite(image_raw,sizeof(char),bmp_f.bitmap_datasize,fp);
    fclose(fp);
}

// void outputcsv(char*filename,int result[256][3],double mean[3],double std[3])
// {
//     char csv_filename[80];
//     FILE *fp;
//     int i,j=0;
    
//     mkdir("csv");	//Create the directory.
	
//     //Generate the filename.
//     while(filename[j++]!='/' && j<strlen(filename));//check if the filename contains directory;
//     j = (j==strlen(filename))?0:j;      
//     strcpy(csv_filename,"csv/");
//     for(i=0;i+j<strlen(filename)-4;i++)
//         csv_filename[i+4]=filename[j+i];
//     csv_filename[i+4]='\0';
//     strcat(csv_filename,"_hist.csv");

//     fp = fopen(csv_filename,"w");
//     if(!fp)
//     {
//         printf("File %s can't be opened.\n",csv_filename);
//         return;
//     }
//     fprintf(fp,"Index,Red,Green,Blue\n");
//     for (i=0;i<256;i++)
//     {
//         //printf("index:%d,",i);
//         fprintf(fp,"%d,",i);
//         for (j=0;j<3;j++)
//             fprintf(fp,"%d,",result[i][j]);
//         fprintf(fp,"\n");
//     }
//     fprintf(fp,"Means,%.2lf,%.2lf,%.2lf\n",mean[0],mean[1],mean[2]);
//     fprintf(fp,"STD,%.2lf,%.2lf,%.2lf\n",std[0],std[1],std[2]);

//     fclose(fp);
// }