#include "kernel/types.h"
#include"kernel/param.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"




int main(int argc,char*argv[])
{
    if(argc < 2 || argc - 1 >= MAXARG)
    {
        printf("error to input\n");
        exit(1);
    }

    char*args[MAXARG];

    for(int i = 1;i < argc;i++)
    {
        args[i-1] = argv[i];
    }

    char buf[256];
    int index = argc - 1;
    int ind = 0;
    int line_end = 0;
    while(read(0,&buf[ind],sizeof(char)))
    {
        // 如果遇到结尾的话需要需要结束命令的输入
        if(buf[ind] == ' ' || buf[ind] == '\n')
        {
        // 一行的标志位,说明要执行一次任务再获取下一行
            if(buf[ind] == '\n')
            {
                line_end = 1;
            }
            buf[ind] = '\0';
            args[index] = buf;
            index++;
            ind = 0;
        }
        ind++;
        if(line_end)
        {
            line_end = 0;
            // 将执行当前的任务
            args[index] = 0;
            // 重新执行命令,刷新指针
            index = argc - 1;
            int pid = fork();
            if(pid == 0)
            {
                exec(args[0],args);
            }
            else if(pid  > 0)
            {
                wait((int*)0);
            }
            else
            {
                exit(1);
            }
        }
    }
    exit(0);
}