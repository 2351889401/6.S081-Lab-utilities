#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


void filter_pipe(int* x, int n) {
    if(n == 1) {
        printf("process id: %d | data: %d\n", getpid(), x[0]);
        return;
    }
    //否则先输出第一个数 然后创建子进程 将剩余的数据利用管道传输给子进程
    printf("process id: %d | data: %d\n", getpid(), x[0]);

    int p[2];
    pipe(p);// p[0]读 p[1]写

    int pid;
    pid = fork();
    if(!pid) {
        close(0);
        dup(p[0]);
        close(p[0]);//让子进程的0表示从管道读
        close(p[1]);//关掉子进程向前一个管道写的端口 因为子进程一定不会向前一个管道写数据
        read(0, x, (n-1)*4);
        // printf("process id: %d get data:\n", getpid());
        // int j;
        // for(j=0; j<n-1; j++) {
        //     printf("%d ", *(x+j));
        // }
        // printf("\n");
        filter_pipe(x, n-1);
        exit(0);
    }
    // printf("process id: %d send data:\n", getpid());
    // printf("n=%d\n", n);
    // int k;
    // for(k=1; k<n; k++) {
    //     printf("%d ", *(x+k));
    // }
    // printf("\n");

    close(1);
    dup(p[1]);
    close(p[0]);//关掉父进程对新管道的读窗口
    close(p[1]);//让父进程的1表示向管道里面写
    write(1, x+1, (n-1)*4);
    wait((int*)0);
}

int main()
{
    int x[4] = {1, 2, 3, 4};
    filter_pipe(x, 4);
    exit(0);
}