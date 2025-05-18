// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <stdio.h>
// #include <unistd.h>
// #include <string.h>



// int main(int argc, char* argv[])
// {
// 	int ret = 0;
// 	int fd = 0;
// 	int len = 0;
// 	int n = 0;
// 	unsigned char buf[200];
// 	int total_len;
// 	// char * p_str = NULL;
// 	char * str_test = "1234567879";
// 	memset(buf,0,sizeof(buf));
// 	char* filename;
// 	if(argc<2)
// 	{
// 		printf("usage:app <filename>\n");
// 		return 0;
// 	}

// 	filename = argv[1];
	
// 	fd = open(filename, O_RDWR|O_CREAT,S_IRWXU);
// 	if(fd<0)
// 	{
// 		printf("can't open file");
// 		perror("error");
// 	}

// 	while(1)
// 	{
// 		len = read(fd, buf, sizeof(buf)-1);
// 		if(len<0)
// 		{
// 			close(fd);
// 			printf("read error");
// 			perror("error");
// 		}
// 		else if(len==0)
// 		{
// 			printf("\n");
// 			break;
// 		}
// 		else
// 		{
// 			buf[len] = '\0';
// 			printf("%s",buf);
// 		}

// 	}

// 	close(fd);
	
	
// 	return 0;
// }

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char* argv[]) {
    int fd;
    ssize_t ret;
    char write_buf[] = "Hello, Kernel!";  // 要写入的数据
    // char read_buf[100] = {0};            // 存储读取数据的缓冲区
	char read_buf[100];
    char * file;


    if(argc<3)
    {
        printf("Error usage!\r\nusage: ./app <filename> <1:读数据 / 2:写数据>\r\n");
        return 0;
    }
    file = argv[1];

    // 1. 打开设备文件（触发驱动的.open)
    fd = open(file, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }
    printf("Device opened successfully.\r\n");

    if(atoi(argv[2])==1)        //atoi函数将字符串转为数字
    {
        
        // 3. 从设备读取数据（触发驱动的.read）
        ret = read(fd, read_buf, sizeof(read_buf)-1);
	
        if (ret < 0) {
            perror("Read failed");
            close(fd);
            return -1;
        }
        read_buf[ret] = '\0';  // 确保字符串终止
        printf("Read %d bytes from device: %s\r\n", ret, read_buf);
        // ret = read(fd, read_buf, 50);
		// if(ret < 0){
		// 	printf("read file failed!\r\n");
		// }else{
		// 	/*  读取成功，打印出读取成功的数据 */
		// 	printf("read data:%s\r\n",read_buf);
		// }
	
    }

    if(atoi(argv[2])==2)
    {
        // 2. 向设备写入数据（触发驱动的.write）
        ret = write(fd, write_buf, strlen(write_buf));
        
        if (ret < 0) {
            perror("Write failed");
            close(fd);
            return -1;
        }
        printf("Wrote bytes to device: %s\r\n", write_buf);

    }
    

    // 4. 关闭设备（触发驱动的.release）
    close(fd);
    printf("Device closed.\r\n");

    return 0;
}