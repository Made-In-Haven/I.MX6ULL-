#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/*
ap3216c测试app
usage:  ./ap3216c_app <filename>

*/


int main(int argc,char* argv[])
{
    char* filename;          
    unsigned char state[1];
    int fd;
    int retvalue;
    short value[3];
    
    if(argc<2)
    {
        printf("usage: ./ap3216c_app <filename>\r\n");
        return 0;
    }

    filename = argv[1];             //获取文件名字
    
    fd = open(filename,O_RDWR);

    if(fd<0)
    {
        printf("open key device failed\r\n");
        return fd;
    }

    while(1)
    {
        read(fd,&value,sizeof(value));
        printf("ap3216c ir=%d, als=%d, ps=%d \r\n",value[0],value[1],value[2]);
        sleep(1);  
    }


    close(fd);
    return 0;

}