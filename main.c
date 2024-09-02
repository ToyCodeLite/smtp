/**
 * @file main.c
 *
 *  Author: zbc
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

#include "smtp.h"

#ifdef _WIN32
#include "os/windows.c"
#else
#include "os/linux.c"
#endif

//返回一个 char *arr[], size为返回数组的长度
char **explode(char sep, const char *str, int *size)
{
        int count = 0, i;
        for(i = 0; i < strlen(str); i++)
        {       
                if (str[i] == sep)
                {       
                        count ++;
                }
        }
 
        char **ret = calloc(++count, sizeof(char *));
 
        //int lastindex = -1;
        int lastindex = -1;
        int j = 0;
 
        for(i = 0; i < strlen(str); i++)
        {       
                if (str[i] == sep)
                {       
                    if ((i - lastindex -1) > 0)
                    {
                        ret[j] = calloc(i - lastindex, sizeof(char)); //分配子串长度+1的内存空间
                        memcpy(ret[j], str + lastindex + 1, i - lastindex - 1);
                        // if(j==0)
                        // {
                            // memcpy(ret[j], str + lastindex , i - lastindex);
                        // }
                        // else{
                            // memcpy(ret[j], str + lastindex + 1 , i - lastindex  -1 );
                        // }
                        j++;
                    }
                    lastindex = i;
                }
        }
        
        //处理最后一个子串
        //if (lastindex <= strlen(str) )
        if (lastindex <= (int)strlen(str) )
        {
            if ((strlen(str) - 1 - lastindex) > 0)
            {   
                ret[j] = calloc(strlen(str) - lastindex, sizeof(char));
                memcpy(ret[j], str + lastindex + 1, strlen(str) - 1 - lastindex);
                j++;
            }
        }
 
        *size = j;
 
        return ret;
}

void help_info()
{
    printf(
    "Usage: ./smtp [Option]\n"
    "    -h    --help             show this\n"
    "    -m    --show             Display sending info\n"
    "    -d    --domain           smtp server address[smtp.163.com]\n"
    "    -u    --user             username:Sender Email\n"
    "    -p    --password         \n"
    "    -P    --port             smtp server port[25]\n"
    "    -t    --to               Receiving Email list\n"
    "    -c    --cc               Carbon Copy Email list\n"
    "    -s    --subject          subject\n"
    "    -b    --content          email body partion\n"
    "    -f    --attachment       filname list\n"
    
    );
    exit(0);
}

//reference https://blog.csdn.net/zhaoyong26/article/details/54574398
void get_option(int argc, char **argv, struct smtp* sm)
{
    char *cmd = argv[0];
    int flag = 0;
    
    /**
    d:domain
    //username
    f:from
    p:password
    t:to 
    c:cc
    s:subject
    b:content
    a:attachment
    **/
    
    /**
    char *domain = NULL;
    char *from = NULL;
    char *password = NULL;
    char *to = NULL;
    char *cc = NULL;
    char *subject = NULL;
    char *content = NULL;
    char *attachment = NULL;
    **/
    
    while (1) {
        int option_index = 0;
        struct option long_options[] =
        {
            {"help"        , 0, 0, 'h'},  //0代表没有参数
            {"domain"      , 1, 0, 'd'},
            {"port"        , 1, 0, 'P'},
            //{"from"        , 1, 0, 'f'},
            {"user"        , 1, 0, 'u'},
            {"password"    , 1, 0, 'p'},
            {"to"          , 1, 0, 't'}, 
            {"cc"          , 1, 0, 'c'}, 
            {"subject"     , 1, 0, 's'}, 
            {"attachment"  , 1, 0, 'f'}, 
            {"content"     , 1, 0, 'b'}, //1代表有参数 
            {"show"        , 0, 0, 'm'}
        };
        int c;
 
        c = getopt_long(argc, argv, "h:d:u:p:P:t:c:s:f:b:m",long_options, &option_index);  //注意这里的冒号，有冒号就需要加参数值，没有冒号就不用加参数值
        if (c == -1)
                break;
 
        switch (c)
        {
            case 'h':
                 //printf("help->\n\t%s",help_info);
                 help_info();
                 break;
            case 'd':
                // printf("domain: %s\n", optarg);
                // domain = (char*)calloc(strlen(optarg),sizeof(char));
                sm->domain = optarg;
                break;
 
            case 'u':
                sm->user_name = optarg;
                break;
 
            case 'p':
                sm->password = optarg;
                break;
 
            case 'P':
                // 将 optarg 转换为 int 类型
                sm->port = strtol(optarg, NULL, 10);
                if (errno == ERANGE || sm->port <= 0 || sm->port > 65535) {
                    fprintf(stderr, "[Error] Invalid port number.\n");
                    exit(1);
                }
                break;
            
            case 't':
            {
                int to_len;
                sm->to = explode(',', optarg, &to_len);
                sm->to_len = to_len;
                break;
            }
            case 'c':
            {
                int cc_len;
                // sm->cc = explode(',', optarg, &cc_len);
                (*sm).cc = explode(',', optarg, &cc_len);
                // sm->cc_len = cc_len;
                (*sm).cc_len = cc_len;
                break;
            }
            case 'f':
            {
                int count;
                sm->attachment = explode(',', optarg, &count);
                sm->file_count = count;
                // printf("\\\\TODO:Unable to send attachment\n");
                // exit(1);
                break;
            }
            case 's':
                sm->subject = optarg;
                break;
 
            case 'b':
                sm->content = optarg;
                break;
            case 'm':
                flag = 1;
                break;
            default:
                // printf("this is default!\n");
                break;
        }
    }
    
    if(sm->user_name == NULL){
        // help_info();
        fprintf(stderr,"[Error] user cannot be empty.\n");
        exit(1);
    }
    if(sm->password == NULL){
        // help_info();
        fprintf(stderr,"[Error] password cannot be empty.\n");
        exit(1);
    }
    if(sm->to == NULL){
        // printf("[Error] Receive Email address cannot be empty.\n");
        fprintf(stderr,"[Error] Receive Email address cannot be empty.\n");
        exit(1);
    }
    if(sm->subject == NULL){
        fprintf(stderr,"[Error] Subject cannot be empty.\n");
        exit(1);
    }
    //打印邮件信息
    if(flag == 1)
    {
        
        if(sm->domain != NULL){
            printf("domain:%s\n",sm->domain);
            //free(domain);
            //domain = NULL;
        }
        
        if(sm->user_name != NULL){
            printf("from:%s\n",sm->user_name);
        }
        
        if(sm->password != NULL){
            // printf("password:%s\n",sm->password);
            printf("password:******\n");
        }
        
        if(sm->to != NULL)
        {
            printf("To: ");
            for(int i = 0; i < sm->to_len; i++)
            {
                printf("<%s>",sm->to[i]);
                if(i+1 == sm->to_len)
                    printf("\n");
                else
                    printf(", ");
            }
        }
        if(sm->cc != NULL)
        {
            printf("CC: ");
            for(int i = 0; i < sm->cc_len; i++)
            {
                printf("<%s>",sm->cc[i]);
                if(i+1 == sm->cc_len)
                    printf("\n");
                else
                    printf(", ");
            }
        }
        if(sm->attachment != NULL)
        {
            printf("attachment: ");
            for(int i = 0; i < sm->file_count; i++)
            {
                printf("<%s>",sm->attachment[i]);
                if(i+1 == sm->file_count)
                    printf("\n");
                else
                    printf(", ");
            }
        }
        
    }
    if(sm->subject != NULL)
    {
        printf("subject:%s\n", sm->subject);
    }
    return;
}


int main(int argc,char** argv)
{
    struct smtp sm = {};
    sm.domain = "smtp.163.com";
    sm.port = 25;
    sm.cc = NULL;
    // sm.cc_len = NULL;
    
    int to_len = 2;

    /**
        d:domain
        //username
        p:password
        f:from
        t:to 
        c:cc
        s:subject
        b:content
        a:attachment
    **/
    
    printf("\n\n\n----------------------------------\n");
    get_option(argc , argv, &sm);
    
    smtp_send(&sm);
    
    printf("\n\n\n----------------end------------------\n");
    
    return EXIT_SUCCESS;
}


