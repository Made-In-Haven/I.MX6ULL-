#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/select.h>


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


int main(int argc,char* argv[])
{
    char* filename;          
    unsigned char state[1];
    fd_set readfds; /* 读操作文件描述符集 */
    struct timeval timeout; /* 超时结构体 */
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
    
    
    fd = open(filename,O_RDWR|O_NONBLOCK);

    if(fd<0)
    {
        printf("open key device failed\r\n");
        return fd;
    }

    while(1)
    {
        FD_ZERO(&readfds);          /* 清除 readfds */
        FD_SET(fd, &readfds);       /* 将 fd 添加到 readfds 里面 */

        //设置阻塞访问超时事件
        timeout.tv_sec = 3;         //0秒
        timeout.tv_usec = 0;   //500ms
        retvalue = select(fd + 1, &readfds, NULL, NULL, &timeout);

        switch(retvalue)
        {
            case 0:     //超时
                printf("超时\r\n");

                break;

            case -1:    //错误
                printf("错误\r\n");
                return 0;

            default:
                if(FD_ISSET(fd, &readfds)) { /* 判断是否为 fd 文件描述符 */
                    /* 使用 read 函数读取数据 */
                    retvalue = read(fd,buf,2);
                    if(retvalue<0)
                    {
                        printf("读取失败\r\n");
                        close(fd);
                        return retvalue;
                    }
                    printf("key value:%d\r\n",buf[0]);
                    printf("press times:%d\r\n",buf[1]);
                }

        }


    }


    close(fd);
    return 0;

}