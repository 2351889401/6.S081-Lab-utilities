#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main()
{
    char buf[10];
    buf[0] = 'a';

    int p[2];
    pipe(p); //p[0]为读端口 p[1]为写端口

    //要求 父进程发送一个字节数据 子进程接收到 输出信息
    //之后 子进程向父进程发送一个字节数据 父进程收到 输出信息
    int n;
    int pid;
    pid = fork();

    if(!pid) {//子进程
        if((n = read(p[0], buf+1, 1)) < 1) {
            fprintf(1, "childpid %d: read failed\n", getpid());
            exit(1);
        }
        // buf[1] += 1;
        printf("%d: received ping\n", getpid());
        // printf("%c\n", buf[1]);
        write(p[1], buf+1, 1);
        close(p[0]);
        close(p[1]);
        exit(0);
    }
    write(p[1], buf, 1);
    wait((int*)0);
    if((n = read(p[0], buf+2, 1)) < 1) {
        fprintf(1, "parentpid %d: read failed\n", getpid());
        exit(1);
    }
    // buf[2] += 1;
    printf("%d: received pong\n", getpid());
    // printf("%c\n", buf[2]);
    close(p[0]);
    close(p[1]);
    // printf("%c %c %c\n", buf[0], buf[1], buf[2]);
    // printf("%c %c %c\n", 'x', 'y', 'z');
    exit(0);
    // printf("hello world\n");

    // char buf[10];
    // buf[0] = 'a';

    // write(p[1], buf, 1);

    // char rcv[10];
    // if(read(p[0], rcv, 1) <= 0) {
    //     fprintf(2, "failed to read...\n");
    //     close(p[0]);
    //     close(p[1]);
    //     exit(1);
    // }

    // printf("recv message: %c\n", rcv[0]);

    // close(p[0]);
    // close(p[1]);

    // exit(0);
}