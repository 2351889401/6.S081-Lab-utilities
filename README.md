## 这是6.S081的第一个实验，实验内容主要是看书上第一章的内容，熟悉OS提供的系统调用，基于这些系统调用编写一些应用程序。虽然是编写应用程序，但仍有一些值得思考的地方。
  
## 自己阅读后的感悟：OS除了完成复用（硬件资源）、隔离（进程与进程、进程与内核）、交流（进程间通信）、抽象（将文件、设备抽象为文件描述符）等基础功能，保证系统内核稳定、高效的运行；是不是也需要为应用程序提供简单而又强大的端口、这样才能促进更多的使用者（不仅是操作系统程序员，也可以是应用程序员、甚至是非专业的编程人员）轻松地使用计算机资源、才能引起更多的流行、有更多的人来优化OS这样的一个平台？实际上这似乎就是Linux操作系统平台所做的事情。

### （这是个人的浅浅见解，不一定对。但是我觉得如果真的可以把困难的技术、简化到普通人都可以很轻松的使用，这本身对技术推动就是很了不起的一件事。比如抖音，虽然是短视频平台，但是它就是做到了让老百姓很轻松的去完成拍摄、发布这些本来要求相对较高的事情，所以它能获得很大的流行。这也是科技真正带给大家幸福的见证。我也希望所有的科技发展，最终都能给人们带去幸福！）

在正式开始实验前，先分享书上第一章我认为较难理解的部分，并画出一些图来帮助理解，欢迎大家批评指正。
```
The xv6 shell implements pipelines such as grep fork sh.c | wc -l in a manner similar
to the above code (user/sh.c:100). The child process creates a pipe to connect the left end of the
pipeline with the right end. Then it calls fork and runcmd for the left end of the pipeline and
fork and runcmd for the right end, and waits for both to finish. The right end of the pipeline
may be a command that itself includes a pipe (e.g., a | b | c), which itself forks two new child
processes (one for b and one for c). Thus, the shell may create a tree of processes. The leaves
of this tree are commands and the interior nodes are processes that wait until the left and right
children complete.

In principle, one could have the interior nodes run the left end of a pipeline, but doing so
correctly would complicate the implementation. Consider making just the following modification: 
change sh.c to not fork for p->left and run runcmd(p->left) in the interior process. 
Then, for example, echo hi | wc won’t produce output, because when echo hi exits
in runcmd, the interior process exits and never calls fork to run the right end of the pipe. This
incorrect behavior could be fixed by not calling exit in runcmd for interior processes, but this
fix complicates the code: now runcmd needs to know if it a interior process or not. Complications
also arise when not forking for runcmd(p->right). For example, with just that modification,
sleep 10 | echo hi will immediately print “hi” instead of after 10 seconds, because echo
runs immediately and exits, not waiting for sleep to finish. Since the goal of the sh.c is to be as
simple as possible, it doesn’t try to avoid creating interior processes.
```
