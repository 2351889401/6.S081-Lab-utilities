#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void hey_its_you(int* x, int n) {
    if(n == 1) {
        printf("prime %d\n", x[0]);
        return;
    }

    printf("prime %d\n", x[0]);

    //从剩余(n-1)个数中筛选掉 "%x[0] == 0" 的数
    int i, j;
    j = 0;
    int now = x[0];
    for(i=1; i<n; i++) {
        if(x[i] % now == 0) continue;
        x[j++] = x[i];
    }
    //最终j表示剩余的个数

    int p[2];
    pipe(p);

    int pid;
    pid = fork();
    if(!pid) {
        //子进程
        close(0);
        dup(p[0]);
        close(p[0]);
        close(p[1]);
        read(0, x, j*4);

        // printf("process_id %d recv data\n", getpid());
        // int k2;
        // for(k2=0; k2<j; k2++) printf("%d ", *(x+k2));
        // printf("\n");

        hey_its_you(x, j);
        exit(0);
    }

    // printf("process_id %d send data\n", getpid());
    // int k1;
    // for(k1=0; k1<j; k1++) printf("%d ", *(x+k1));
    // printf("\n");

    //主进程将剩余数据write到管道里给子进程
    close(1);
    dup(p[1]);
    close(p[1]);
    close(p[0]);//主进程不从新生成的管道读 只向新管道里面写
    write(1, x, j*4);
    wait((int*)0);
}

int main()
{
    int i;
    int n = 0;
    int a[35];
    for(i=2; i<=35; i++) {
        a[i-2] = i;
        n++;
    }
    hey_its_you(a, n);
    exit(0);
}