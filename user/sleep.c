#include"kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int main(int argc,char *argv[])
{

    if(argc != 2)
    {
        fprintf(2,"Usage: sleep...\n");
        exit(1);
    }

    int time = atoi(argv[1]);

    int ret = sleep(time);
    if(ret)
    {
        fprintf(2,"error to sleep: %d",ret);
    }
    exit(0);
}