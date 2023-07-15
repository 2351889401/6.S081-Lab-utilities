#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

//read是在一个文件描述符里面进行连续读 读到最后返回0
//本次任务要求read读到换行'\n'退出

char buf[512];

int my_gets(char* p, int fd) { //从fd读取数据到p位置 返回读取的字符数 读到'\n'为止
    int i;
    i = 0;
    char c;

    while(1) {
        int now;
        now = read(fd, &c, 1);
        if(now == 0 || c == '\n') break;
        p[i] = c;
        i++;
    }
    p[i] = 0;
    // printf("read %s\n", p);
    return i;
}

void do_xargs(int argc, char *argv[], int fd) {
    //从文件描述符读取参数 生成子进程 子进程exec
    int i, n;
    for(i=0; i<argc-1; i++) {
        strcpy(argv[i], argv[i+1]);
    }

    int pid;
    while((n = my_gets(buf, fd)) > 0) {
        pid = fork();
        if(!pid) {
            strcpy(argv[argc-1], buf);
            // printf("%s\n", argv[0]);
            // for(int i=0; i<argc; i++) {
            //     printf("%s ", argv[i]);
            // }
            // printf("\n");
            exec(argv[0], argv);
            fprintf(2, "exec %s failed\n", argv[0]);
            exit(0);
        }
        wait((int*)0);
    }
}

int main(int argc, char* argv[])
{
    do_xargs(argc, argv, 0);
    exit(0);
}