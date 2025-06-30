#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/*
key测试app
usage:  ./key_app <filename>

*/

#define KEY_NAME "key"
#define KEY_VALID 0
#define KEY_INVALID 1

int main(int argc,char* argv[])
{
    char* filename;          
    unsigned char state[1];
    int fd;
    int retvalue;
    char value;
    
    if(argc<2)
    {
        printf("usage: ./key_app <filename>\r\n");
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
        if(value==KEY_VALID)
        {
            printf("key按下,value = %d\r\n",value);
        }
    }


    close(fd);
    return 0;

}