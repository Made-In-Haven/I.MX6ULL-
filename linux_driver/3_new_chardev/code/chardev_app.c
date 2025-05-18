#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/*
led测试app
usage:  ./led_app <filename> <1/0>
1表示开灯
0表示关灯
*/

#define LED_OFF 0
#define LED_ON 1

int main(int argc,char* argv[])
{
    char* filename;          
    unsigned char state[1];
    int fd;
    int retvalue;
    
    if(argc<3)
    {
        printf("usage: ./led_app <filename> <1/0>\r\n");
        return 0;
    }

    filename = argv[1];             //获取文件名字
    state[0] = atoi(argv[2]);       //获取led状态
    if(state[0]!=0 && state[0]!=1)
    {
        printf("usage: ./led_app <filename> <1/0>\r\n");
        return 0;
    }
    
    fd = open(filename,O_RDWR);

    if(fd<0)
    {
        printf("open led device failed\r\n");
        return fd;
    }

    retvalue = write(fd,state,1);
    if(retvalue<0)
    {
        printf("write to led device failed\r\n");
        return retvalue;
    }
    close(fd);
    return 0;

}