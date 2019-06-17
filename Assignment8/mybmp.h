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
    int result[256][3]={0};     //count the RGB number of result bmp
    double AMCD[101][10];       //Store the weight,RGB mean,RGB std,source/target color distance and total TCD.
    double weight_1_mean[3]={0};
    double weight_1_std[3]={0};
    double AMCD_src,AMCD_tar,TAMCD=0.0;
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
    /*Get mean and std of target(weight=1) bmp.*/
    countRGBnumber(weight_1_image_raw,src_bmp.width,src_bmp.height,result);    //count the number of RGB channel.
    countRGBmean(result,weight_1_mean,src_bmp.width*src_bmp.height);          //Get mean of RGB
    countRGBstd(result,weight_1_mean,weight_1_std,src_bmp.width*src_bmp.height);      //Get std of RGB
        
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
        int result[256][3]={0};
        double mean[3]={0};         //count the RGB mean value of result bmp
        double std[3]={0};          //count the RGB std value of result bmp
        
        countRGBnumber(res_bmp.image_raw,src_bmp.width,src_bmp.height,result);    //count the number of RGB channel.
        countRGBmean(result,mean,src_bmp.width*src_bmp.height);          //Get mean of RGB
        countRGBstd(result,mean,std,src_bmp.width*src_bmp.height);      //Get std of RGB
        //double CD_src = countColorDistance(src_bmp.image_raw,res_bmp.image_raw,src_bmp.height,src_bmp.width);     //Get color distance between source and result.
        //double CD_tar = countColorDistance(weight_1_image_raw,res_bmp.image_raw,src_bmp.height,src_bmp.width);     //Get color distance between target and result.
        AMCD_src = 3*(src_bmp.mean[0]-mean[0])*(src_bmp.mean[0]-mean[0])
                            +4*(src_bmp.mean[1]-mean[1])*(src_bmp.mean[1]-mean[1])
                            +2*(src_bmp.mean[2]-mean[2])*(src_bmp.mean[2]-mean[2]);
        AMCD_tar = 3*(weight_1_mean[0]-mean[0])*(weight_1_mean[0]-mean[0])
                            +4*(weight_1_mean[1]-mean[1])*(weight_1_mean[1]-mean[1])
                            +2*(weight_1_mean[2]-mean[2])*(weight_1_mean[2]-mean[2]);                    
        TAMCD =(AMCD_src>AMCD_tar)?(AMCD_src-AMCD_tar):(AMCD_tar-AMCD_src);

        //Store the AMCD result.
        AMCD[count-1][0]=weight;
        AMCD[count-1][1]=mean[0];
        AMCD[count-1][2]=mean[1];
        AMCD[count-1][3]=mean[2];
        AMCD[count-1][4]=std[0];
        AMCD[count-1][5]=std[1];
        AMCD[count-1][6]=std[2];
        AMCD[count-1][7]=AMCD_src;
        AMCD[count-1][8]=AMCD_tar;
        AMCD[count-1][9]=TAMCD;

        //Output result to bmp file.
        //Combine the target bmp file name.
        strcpy(bmp_filename,"AMCD");
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
        if(AMCD[i][9]<AMCD[opti_weight][9])
            opti_weight=i;
     
    //Generate file name
    strcpy(csv_filename,"AMCD");
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
    fprintf(fp,"Sample Pair,Weight,mean,mean,mean,std,std,std,Source,Target,Total,Mark,\n");
    fprintf(fp,",,red,green,blue,red,green,blue,AMCD,AMCD,TAMCD,,\n");
    i=0;
    fprintf(fp,"Source,,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,,,\n",AMCD[i][1],AMCD[i][2],AMCD[i][3],AMCD[i][4],AMCD[i][5],AMCD[i][6],AMCD[i][7],AMCD[i][8],AMCD[i][9]);
    for (i=1;i<100;i++)
    {
        if(i==opti_weight) //If it is the optimal weight,output an "Optimal".
            fprintf(fp,"%d,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,Optimal,\n",i,AMCD[i][0],AMCD[i][1],AMCD[i][2],AMCD[i][3],AMCD[i][4],AMCD[i][5],AMCD[i][6],AMCD[i][7],AMCD[i][8],AMCD[i][9]);
        else
            fprintf(fp,"%d,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,,\n",i,AMCD[i][0],AMCD[i][1],AMCD[i][2],AMCD[i][3],AMCD[i][4],AMCD[i][5],AMCD[i][6],AMCD[i][7],AMCD[i][8],AMCD[i][9]);
    }
    fprintf(fp,"CT(w=1.0),%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,,\n",AMCD[i][0],AMCD[i][1],AMCD[i][2],AMCD[i][3],AMCD[i][4],AMCD[i][5],AMCD[i][6],AMCD[i][7],AMCD[i][8],AMCD[i][9]);
    fprintf(fp,"Target,,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,%.6lf,,,,,\n",tar_bmp.mean[0],tar_bmp.mean[1],tar_bmp.mean[2],tar_bmp.std[0],tar_bmp.std[1],tar_bmp.std[2]);
    
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
    }
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