#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>


/*
led测试app
usage:  ./led_app <filename> <1/0>
1表示开灯
0表示关灯
*/
struct info{
    unsigned char state[1];
    char* path_name;
};



void* led_func1(void* arg)
{
    int fd;
    int* retvalue = (int*)malloc(1*sizeof(int));
    
    if (!retvalue) {
        perror("malloc failed");
        return NULL;
    }
    struct info* info_ptr = (struct info*)arg;
    fd = open(info_ptr->path_name,O_RDWR);
    printf("led_func1\r\n");

    if(*retvalue<0)
    {
        *retvalue = fd;
        printf("open led device failed\r\n");
        return retvalue;
    }

    *retvalue = write(fd,info_ptr->state,1);
    close(fd);
    if(*retvalue<0)
    {
        printf("write to led device failed\r\n");
        return retvalue;
    }
    *retvalue = 0;
    return retvalue;
}

void* led_func2(void* arg)
{
    int fd;
    int* retvalue = (int*)malloc(1*sizeof(int));
    
    if (!retvalue) {
        perror("malloc failed");
        return NULL;
    }
    struct info* info_ptr = (struct info*)arg;
    fd = open(info_ptr->path_name,O_RDWR);
    printf("led_func2\r\n");

    if(*retvalue<0)
    {
        *retvalue = fd;
        printf("open led device failed\r\n");
        return retvalue;
    }

    *retvalue = write(fd,info_ptr->state,1);
    close(fd);
    if(*retvalue<0)
    {
        printf("write to led device failed\r\n");
        return retvalue;
    }
    *retvalue = 0;
    return retvalue;
}

int main(int argc,void* argv[])
{
    int fd;
    int retvalue1,retvalue2;
    void * thread1_result = NULL;
    void * thread2_result = NULL;
    int cnt = 0;
    pthread_t mythread1,mythread2;
    struct info info1;

    
    if(argc<3)
    {
        printf("usage: ./led_app <filename> <1/0>\r\n");
        return 0;
    }

    info1.path_name = argv[1];             //获取文件名字
    info1.state[0] = atoi(argv[2]);       //获取led状态
    if(info1.state[0]!=0 && info1.state[0]!=1)
    {
        printf("usage: ./led_app <filename> <1/0>\r\n");
        return 0;
    }
    retvalue1 = pthread_create(&mythread1,NULL,led_func1,&info1);
    if (retvalue1 != 0) {
        printf("线程1创建失败");
        return -1;
    }
    retvalue2 = pthread_create(&mythread2,NULL,led_func2,&info1);
    if (retvalue2 != 0) {
        printf("线程2创建失败");
        return -1;
    }
    

    //阻塞主线程，直至 mythread1 线程执行结束，用 thread_result 指向接收到的返回值，阻塞状态才消除。
    retvalue1 = pthread_join(mythread1, &thread1_result);

    //阻塞主线程，直至 mythread2 线程执行结束，用 thread_result 指向接收到的返回值，阻塞状态才消除。
    retvalue2 = pthread_join(mythread2, &thread2_result);
    // 处理结果
    if (thread1_result) {
        printf("线程1返回：%d\n", *(int*)thread1_result);
        free(thread1_result);
    }
    if (thread2_result) {
        printf("线程2返回：%d\n", *(int*)thread2_result);
        free(thread2_result);
    }

    printf("主线程执行完毕");
    return 0;
}