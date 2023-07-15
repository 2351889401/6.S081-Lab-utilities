## 这是6.S081的第一个实验，实验内容主要是看书上第一章的内容，熟悉OS提供的系统调用，基于这些系统调用编写一些应用程序。虽然是编写应用程序，但仍有一些值得思考的地方。
  
## 自己阅读后的感悟：OS除了完成复用（硬件资源）、隔离（进程与进程、进程与内核）、交流（进程间通信）、抽象（将文件、设备抽象为文件描述符）等基础功能，保证系统内核稳定、高效的运行；是不是也需要为应用程序提供简单而又强大的端口、这样才能促进更多的使用者（不仅是操作系统程序员，也可以是应用程序员、甚至是非专业的编程人员）轻松地使用计算机资源、才能引起更多的流行、有更多的人来优化OS这样的一个平台？实际上这似乎就是Linux操作系统平台所做的事情。

### （这是个人的浅浅见解，不一定对。但是我觉得如果真的可以把困难的技术、简化到普通人都可以很轻松的使用，这本身对技术推动就是很了不起的一件事。比如抖音，虽然是短视频平台，但是它就是做到了让老百姓很轻松的去完成拍摄、发布这些本来要求相对较高的事情，所以它能获得很大的流行。这也是科技真正带给大家幸福的见证。我也希望所有的科技发展，最终都能给人们带去幸福！）

实验链接（https://pdos.csail.mit.edu/6.S081/2020/labs/util.html ）  
在正式开始实验前，先分享书上第一章我认为较难理解的部分，并画出一些图来帮助理解，欢迎大家批评指正。
```
The xv6 shell implements pipelines such as 'grep fork sh.c | wc -l' in a manner similar
to the above code (user/sh.c:100). The child process creates a pipe to connect the left end of the
pipeline with the right end. Then it calls 'fork' and 'runcmd' for the left end of the pipeline and
'fork' and 'runcmd' for the right end, and waits for both to finish. The right end of the pipeline
may be a command that itself includes a pipe (e.g., 'a | b | c'), which itself forks two new child
processes (one for 'b' and one for 'c'). Thus, the shell may create a tree of processes. The leaves
of this tree are commands and the interior nodes are processes that wait until the left and right
children complete.
```
上面段是说**xv6**中“**shell**”程序（**user/sh.c**）中管道的实现方式：进程创建管道，然后使用“**fork**”创建左子进程和右子进程，等待这2个进程执行完，以这种方式保证管道命令执行的顺序性。  
为什么这样可以保证管道命令的顺序性呢？
```
case PIPE:
    pcmd = (struct pipecmd*)cmd;
    if(pipe(p) < 0)
      panic("pipe");
    if(fork1() == 0){ //左子进程
      close(1);
      dup(p[1]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->left);
    }
    if(fork1() == 0){ //右子进程
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->right);
    }
    close(p[0]);
    close(p[1]);
    wait(0);
    wait(0);
    break;
```
可以看到代码中“左子进程的**1**号文件描述符（标准输出）重定向为管道的写端口”，“右子进程的**0**号文件描述符（标准输入）重定向为管道的读端口”。  
而直到左子进程完成，成功退出（执行了**exit**，此时会释放所有的文件描述符），这时管道的写端口文件描述符释放，读端口才能变成非阻塞的状态，右子进程才能继续执行下去，这样才能保证管道命令的顺序性。  
图示如下：  
![](https://github.com/2351889401/6.S081-Lab-utilities/blob/main/images/pipe1.png)  
  
而段中提到的由管道命令引起的进程树（**tree of processes**）是下面这个样子的：  
![](https://github.com/2351889401/6.S081-Lab-utilities/blob/main/images/pipe2.png)  
图中的红色虚线是数据的流向，也是管道命令的执行顺序。  

## 

  
下面开始正式的实验内容。  
**1.** sleep (easy)  
内容比较简单，就是让进程通过“**sleep()**”系统调用停顿一段时间。  
```
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char * argv[]) {
    if(argc < 2) {
        fprintf(2, "Usage: sleep seconds...\n");
        exit(1);
    }
    int i;
    i = atoi(argv[1]);
    sleep(i);
    exit(0);
}
```
  
**2.** pingpong (easy)  
实验内容是：通过“**fork()**”创建子进程。利用“**pipe**”创建管道，父进程和子进程得以通信，输出信息。  
具体流程就是：父进程先发送一字节的数据，子进程接收，输出信息；子进程发送一字节的数据，父进程接收，输出信息。  
```
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
        printf("%d: received ping\n", getpid());
        write(p[1], buf+1, 1);
        close(p[0]);
        close(p[1]);
        exit(0);
    }
    write(p[1], buf, 1);
    wait((int*)0); //通过wait()语句控制子进程完成上面的流程 此时父进程可以接收了
    if((n = read(p[0], buf+2, 1)) < 1) {
        fprintf(1, "parentpid %d: read failed\n", getpid());
        exit(1);
    }
    printf("%d: received pong\n", getpid());
    close(p[0]);
    close(p[1]);
    exit(0);
}
```
实验结果：  
![](https://github.com/2351889401/6.S081-Lab-utilities/blob/main/images/pingpong.png)  

**3.** primes (moderate/hard)  
实验内容是：通过“**pipe**”和“**fork**”，完成“**埃氏筛**”的过程，如下图所示：  
![](https://github.com/2351889401/6.S081-Lab-utilities/blob/main/images/aishishai.png)  
需要注意的就是文件描述符数量有限，进程中要及时关掉用不到的文件描述符。  
另外，从实验的提示中大致可以看到“**递归**”的影子。  
当时写的时候好像思考了蛮久，这里就直接展示代码了。
```
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
        close(p[1]); //子进程由于会进行下一次的递归 所以需要把用不到的文件描述符释放掉 减少资源的占用
        read(0, x, j*4); 

        hey_its_you(x, j);
        exit(0);
    }

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
```
实验结果为：  
