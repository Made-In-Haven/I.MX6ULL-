#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>


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

int fd;

void sigio_handler(int signum)
{
    int ret=0;
    unsigned char buf[2];
    ret = read(fd,buf,sizeof(buf));
    
    if(ret<0)
    {
        printf("read error\r\n");
    }
    else
    {
        printf("key value:%d\r\n",buf[0]);
        printf("press times:%d\r\n",buf[1]);
    }
    
}   



int main(int argc,char* argv[])
{
    char* filename;          
    unsigned char state[1];
    
    int retvalue;
    char value;
    unsigned int cmd;               //ioctl cmd
    unsigned char buf[2];
    int flags = 0;
    
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
    signal(SIGIO, sigio_handler);
    fcntl(fd, F_SETOWN, getpid());  //设置能够接受SIGIO信号的进程
    flags = fcntl(fd, F_GETFL);     /* 获取当前的进程状态 */
    fcntl(fd, F_SETFL, flags | FASYNC); /* 开启当前进程异步通知功能 */


    while(1);


    close(fd);
    return 0;

}