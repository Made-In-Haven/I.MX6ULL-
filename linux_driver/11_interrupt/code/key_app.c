#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>


/*
key测试app
usage:  ./key_app <filename> cmd

*/

#define CMD_BASE 0XEF
#define CLOSE_CMD   (_IO(CMD_BASE, 1))    //关闭命令
#define OPEN_CMD   (_IO(CMD_BASE, 2))
#define SET_PEROID_CMD   (_IOW(CMD_BASE, 3, int))

#define KEY_NAME "key"
#define KEY_VALID 0
#define KEY_INVALID 1


void cmd_handle(int fd, unsigned int cmd)
{
    unsigned long arg = 0;
    switch (cmd)
    {
        case CLOSE_CMD:
            ioctl(fd,cmd,0);
            break;

        case OPEN_CMD:
            ioctl(fd,cmd,0);
            break;

        case SET_PEROID_CMD:
            if(scanf("%lu",&arg)!=1)
            {
                while(getchar()!='\n');
                printf("错了,arg:%lu",arg);
                break;
            }
            // printf("arg:%lu\r\n",arg);
            while (getchar() != '\n');          //清除标准输入缓冲区
            ioctl(fd,cmd,arg);
            break;

        default:
            break;
    }
}

int main(int argc,char* argv[])
{
    char* filename;          
    unsigned char state[1];
    int fd;
    int retvalue;
    char value;
    unsigned int cmd;               //ioctl cmd
    unsigned char buf[2];
    
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
        retvalue = read(fd,buf,2);
        if(retvalue<0)
        {
            printf("读取失败\r\n");
            close(fd);
            return retvalue;
        }

        printf("key value:%d\r\n",buf[0]);
        printf("press times:%d\r\n",buf[1]);

        sleep(3);

    }


    close(fd);
    return 0;

}