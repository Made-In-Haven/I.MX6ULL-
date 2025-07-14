#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <sys/ioctl.h>


#define KEY_PRESS 0
#define KEY_RELEASE 1

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "用法: %s /dev/input/eventX\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    char name[256] = "Unknown";
    ioctl(fd, EVIOCGNAME(sizeof(name)), name);
    printf("正在监控设备: %s (%s)\r\n", argv[1], name);

    struct input_event ev;
    while (1) {
        ssize_t n = read(fd, &ev, sizeof(ev));
        if (n < sizeof(ev)) {
            perror("read\r\n");
            break;
        }

        printf("时间: %ld.%06ld, 类型: \r\n", ev.time.tv_sec, ev.time.tv_usec);
        switch (ev.type) {
            case EV_KEY:
                printf("按键事件, 键码: %d, %s\r\n", 
                       ev.code, ev.value==KEY_PRESS ? "按下" : "释放");
                break;

            case EV_REP:
                printf("按键重复按下，按键%d按下次数: %d\r\n",ev.code, ev.value);
                break;

            case EV_REL:
                printf("相对坐标事件, 坐标: ");
                switch (ev.code) {
                    case REL_X: printf("X=%d\n", ev.value); break;
                    case REL_Y: printf("Y=%d\n", ev.value); break;
                    default: printf("未知(%d)=%d\n", ev.code, ev.value);
                }
                break;
            case EV_ABS:
                printf("绝对坐标事件, 坐标: ");
                switch (ev.code) {
                    case ABS_X: printf("X=%d\n", ev.value); break;
                    case ABS_Y: printf("Y=%d\n", ev.value); break;
                    default: printf("未知(%d)=%d\n", ev.code, ev.value);
                }
                break;
            case EV_SYN:
                //不做处理
                break;
            default:
                printf("其他事件(type=%d), code=%d, value=%d\n", 
                       ev.type, ev.code, ev.value);
        }
    }

    close(fd);
    return 0;
}