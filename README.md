## 这是6.S081的第一个实验，实验内容主要是看书上第一章的内容，熟悉OS提供的系统调用，基于这些系统调用编写一些应用程序。虽然是编写应用程序，但仍有一些值得思考的地方。
  
## 自己阅读后的感悟：OS除了完成复用（硬件资源）、隔离（进程与进程、进程与内核）、交流（进程间通信）、抽象（将文件、设备抽象为文件描述符）等基础功能，保证系统内核稳定、高效的运行；是不是也需要为应用程序提供简单而又强大的接口、这样才能促进更多的使用者（不仅是操作系统程序员，也可以是应用程序员、甚至是非专业的编程人员）轻松地使用计算机资源、才能引起更多的流行、有更多的人来优化OS这样的一个平台？实际上这似乎就是Linux操作系统平台所做的事情。

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
上面这段是说**xv6**的“**shell**”程序（**user/sh.c**）中管道命令的实现方式：进程创建管道，然后使用“**fork**”创建左子进程和右子进程，等待这2个进程执行完，以这种方式保证管道命令执行的顺序性。  
为什么这样可以保证管道命令的顺序性呢？**shell**程序中管道命令的执行代码如下：
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
实验内容是：通过“**pipe**”和“**fork**”，完成“**埃氏筛**”的过程，筛选2~35中的质数，如下图所示：  
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
![](https://github.com/2351889401/6.S081-Lab-utilities/blob/main/images/primes.png)   

**4.** find (moderate)  
实验内容：寻找某一目录下有特定名称的全部文件。需要参考原始的应用程序“**user/ls.c**”中**读取目录**的方式完成实验。
注意点看一下代码中的注释就可以了。
```
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


void do_find(const char* dir, const char* filename) {
    char buf[512], *p; // 因为要修改路径 需要在目录后面添加'/'
    int fd;
    struct stat st;
    struct dirent de;
    // printf("dir = %s\n", dir);
    if((fd = open(dir, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", dir);
        return;
    }

    if(fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", dir);
        close(fd);
        return;
    }

    if(st.type == T_DIR) {
        strcpy(buf, dir);
        p = buf + strlen(buf); //p指向buf的结尾 利用p去修改“文件路径”（也就是buf的内容）
        *p++ = '/';
        while(read(fd, &de, sizeof(de)) == sizeof(de)) { //遍历所有的条目
            //读取磁盘块上的数据 每16字节读取一次 1024字节一共读取64次
            //如果某一次读取数据为空(从这以后没有文件名了) 它的de.inum为0
            
            //每次读磁盘块上的数据(就是看当前目录下有哪些文件)时 
            //总会有2个信息 '.' '..' 记录了当前目录的inode号和文件名 上层目录的inode号和文件名
            
            // printf("%s\n", dir);
            // printf("de.inum: %d   de.name: %s\n", de.inum, de.name);
            if(de.inum == 0 || !strcmp(de.name, ".") || !strcmp(de.name, "..") ) continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0) { //这一步是获取新的“文件路径”的信息 查看它是文件还是目录 是文件的话则不继续递归 是目录的话需要递归
                fprintf(2, "find: cannot stat %s\n", buf);
                continue;
            }
            if(st.type == T_FILE) {
                if(strcmp(filename, de.name) == 0) {
                    // printf("%s\n", buf);
                    write(1, buf, strlen(buf));
                    write(1, "\n", 1);
                }
            }
            else if(st.type == T_DIR) {
                do_find(buf, filename);
            }
        }
    }

    close(fd);
}

int main(int argc, char* argv[])
{
    if(argc < 3) {
        fprintf(2, "Usage: find path filename\n");
        exit(1);
    }
    // printf("%s\n", argv[1]);
    // printf("%s\n", argv[2]);
    do_find(argv[1], argv[2]);

    exit(0);
}
```
实验结果为：  
![](https://github.com/2351889401/6.S081-Lab-utilities/blob/main/images/find.png)  

**5.** xargs (moderate)  
**Linux**下的 “**xargs**” 命令常与 “**|**” 连起来用，起到的作用是“**从管道中读取输入数据，并进行一定程度的处理（分割、分批），然后传递给后面的命令**”，如下图所示：  
![](https://github.com/2351889401/6.S081-Lab-utilities/blob/main/images/xargs1.png)  
图中管道的输入为“**11m22m33**”，“**-d**”表示后面的字符为分隔符，“**-n**”表示分割之后每一批次传入的参数量，图中是一次传入2个，送给最后的“**echo**”命令，所以会有图中的输出。  

本次实验不需要像“**Linux**”中这样的优化，只需要每次读入一个参数（读到**“\n”**）即可。  
实验测试的脚本命令如下（**xargstest.sh**）：  
  
```
mkdir a
echo hello > a/b
mkdir c
echo hello > c/b
echo hello > b
find . b | xargs grep hello

```
  
“**find**”命令的实现是**4**中实现的。  
实际上可以看到，通过管道传递给“**xargs**”命令的参数应该是
```
./a/b
./c/b
./b
```
  
“**xargs**”从标准输入（这时的标准输入当然是管道的写端口了）每次读入一个参数（就是一个文件路径），后续的行为应该是什么？  
应该是创建子进程，让子进程通过“**exec()**”去执行“**grep hello 文件路径**”操作。所以代码如下，一些问题记录在了注释里面：  
```
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
    for(i=0; i<argc-1; i++) { //这里的参数移位操作是什么意义呢 参考下面的解释和图
        strcpy(argv[i], argv[i+1]);
    }

    int pid;
    while((n = my_gets(buf, fd)) > 0) {
        pid = fork();
        if(!pid) {
            strcpy(argv[argc-1], buf);
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
```
上述代码中涉及到参数的移位，为什么呢？当时做的时候好像“**argv[]**”数组的申请有点问题，后来就想了个办法：原始参数肯定需要“**argv[]**”数组的，那为什么不在原始参数的基础上修改呢？  
![](https://github.com/2351889401/6.S081-Lab-utilities/blob/main/images/method.png)  
如图中所示，“**xargs**”命令执行的时候“**argv[]**”数组是上面一行的情况，我的做法是将后面的字符串向前拷贝一格，然后最后的位置留给“**输入参数**”（这里也就是文件名）。  
实验结果为：  
![](https://github.com/2351889401/6.S081-Lab-utilities/blob/main/images/xargs2.png)
