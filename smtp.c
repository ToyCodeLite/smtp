/**
 * @file smtp.c
 *
 * 
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
// #include <winsock.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <strings.h>
#include<math.h>

#include "smtp.h"
#include "lib/base64.h"


/**
 * 返回的错误码
 */
enum SMTP_ERROR
{
    // success
    SMTP_ERROR_OK,

    // create socket fail!
    SMTP_ERROR_SOCKET,

    // connect socket fail
    SMTP_ERROR_CONNECT,

    // can not find domain
    SMTP_ERROR_DOMAIN,

    // server error
    SMTP_ERROR_READ,

    SMTP_ERROR_WRITE,

    // server status error
    SMTP_ERROR_SERVER_STATUS
};



/**
*去前后空白符
*/
void trim(char* str){
    if(str==NULL)
        return;
    char *begin=str;
    while(*begin&&(unsigned char)*begin<=32) begin++;  
    if(!*begin){
        *str=0;
        return;
    }   
    while(*str++=*begin++); 
    str-=2;
    while((unsigned char)*str--<=32);
    *(str+2)=0;
}


/* 功  能：将str字符串中的oldstr字符串替换为newstr字符串
 * 参  数：str：操作目标 oldstr：被替换者 newstr：替换者
 * 返回值：返回替换之后的字符串
 * 版  本： V0.2
 */
char *strrpc(char *str,char *oldstr,char *newstr){
    char bstr[strlen(str)];//转换缓冲区
    memset(bstr,0,sizeof(bstr));
 
    for(int i = 0;i < strlen(str);i++){
        if(!strncmp(str+i,oldstr,strlen(oldstr))){//查找目标字符串
            strcat(bstr,newstr);
            i += strlen(oldstr) - 1;
        }else{
        	strncat(bstr,str + i,1);//保存一字节进缓冲区
	    }
    }
 
    strcpy(str,bstr);
    return str;
}


/**
 * @enum smtp 状态
 */
enum SMTP_STATUS
{
    SMTP_STATUS_NULL,     //!< SMTP_STATUS_NULL
    SMTP_STATUS_EHLO,     //!< SMTP_STATUS_EHLO
    SMTP_STATUS_AUTH,     //!< SMTP_STATUS_AUTH
    SMTP_STATUS_SEND,     //!< SMTP_STATUS_SEND
    SMTP_STATUS_QUIT,     //!< SMTP_STATUS_QUIT
    SMTP_STATUS_MAX       //!< SMTP_STATUS_MAX
};

/**
 *  读取smtp服务器响应并简单解析出响应状态和响应参数
 * @param sm    smtp指针
 * @return 读取正常返回0，否则返回正数表示错误原因
 */
static int smtp_read(struct smtp* sm)
{
    for(;;)
    {
        int size = recv(sm->socket,sm->buffer,SMTP_BUFFER_SIZE - 1,0);
        if(size == -1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) continue;
        }

        // 套结字错误或者关闭
        if(size <= 0) break;

        sm->buffer[size] = 0;
        printf("SERVER: %s\n",sm->buffer);

        // 不确定这个是否找不到，smtp协议一般都会在响应状态后跟随参数
        sm->cmd = sm->buffer;
        char* p = strchr(sm->buffer,' ');
        if(p)
        {
            *p = '\0';
            sm->data = p + 1;
        }

        return 0;
    }

    printf("smtp_read() 接收信息错误\n");

    return SMTP_ERROR_READ;
}

/**
 *  向服务器发送信息
 * @param fd            套结字
 * @param buffer        要发送的数据
 * @param buffer_size   要发送的数据长度
 * @return  如果成功反送返回0，否则返回正数表示错误原因
 */
int smtp_write(int fd,const char* buffer)
{
    int size = strlen(buffer);
    for(int send_num = 0;send_num < size; )
    {
        int error = send(fd,&buffer[send_num],size - send_num,0);
        if(error < 0)
        {
            printf("发送数据错误 errno = %d size = %d send_num = %d",errno,size, send_num);
            if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) continue;
            return SMTP_ERROR_WRITE;
        }
        else send_num += error;
    }

    return 0;
}

/**
 *  分割接收到的数据，主要是区分base64编码结果。同时sm->data节点会被修改
 * @param sm
 * @return 返回原来的sm->data。
 */
static char* explode(struct smtp* sm)
{
    char* old = sm->data;
    char* p = old;
    while(*p)
    {
        if((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') ||
               *p == '+' || *p == '/' || *p == '=')
        {
            p++;
        }
        else
        {
            sm->data = p;
            *p = '\0';
            break;
        }
    }

    return old;
}

static int hello(struct smtp* sm)
{
    // 发送HELO命令
    char buffer[256];
    memset(buffer, 0 , sizeof(buffer));
    int size = sprintf(buffer,"HELO %s\r\n",sm->domain);

    if(smtp_write(sm->socket,buffer)) return SMTP_ERROR_WRITE;

    // 服务器应该正常返回250
    if(smtp_read(sm) || strcmp(sm->cmd,"250")) return SMTP_ERROR_READ;

    sm->status = SMTP_STATUS_AUTH;

    return 0;
}

static int auth(struct smtp* sm)
{
    // 发送AUTH命令,第一次接到的数据应该是base64编码后的Username，如果不是直接返回
    // 然后第二次应该是base64后的Password
    if(smtp_write(sm->socket,"AUTH LOGIN\r\n")) return SMTP_ERROR_WRITE;
    if(smtp_read(sm) || strcmp(sm->cmd,"334")) return SMTP_ERROR_READ;
    
    // username:
    char* p = explode(sm);
    char buffer[256];
    char* BASE64_USERNAME = base64_decode(p);
    int size = strlen(BASE64_USERNAME);
    strcpy(buffer, BASE64_USERNAME);
    free(BASE64_USERNAME);
    
    if(size < 0) return SMTP_ERROR_SERVER_STATUS;
    buffer[size] = 0;
    if(strcasecmp(buffer,"username:")) return SMTP_ERROR_SERVER_STATUS;
    
    char* username = base64_encode(sm->user_name);
    size = strlen(username);
    // strcat(buffer, p2);
    strcpy(buffer, username);
    free(username);
    
    if(size < 0 || size + 2 > 256) return SMTP_ERROR_WRITE;
    buffer[size++] = '\r';
    buffer[size++] = '\n';
    buffer[size] = '\0';
    
    if(smtp_write(sm->socket,buffer)) return SMTP_ERROR_WRITE;
    if(smtp_read(sm) || strcmp(sm->cmd,"334")) return SMTP_ERROR_READ;

    // Password:
    p = explode(sm);
    char* p2 = base64_decode(p);
    size = strlen(p2);
    strcpy(buffer, p2);
    
    if(size < 0) return SMTP_ERROR_SERVER_STATUS;
    buffer[size] = 0;
    
    // if(strcasecmp(buffer,"password:")) return SMTP_ERROR_SERVER_STATUS;
    if(strcasecmp(buffer,"Password:")) return SMTP_ERROR_SERVER_STATUS;
    char* psw = base64_encode(sm->password);
    
    // email password
    strcpy(buffer, psw);
    size = strlen(psw);
    if(size < 0 || size + 2 > 256) return SMTP_ERROR_WRITE;
    buffer[size++] = '\r';
    buffer[size++] = '\n';
    buffer[size] = '\0';
    
    if(smtp_write(sm->socket,buffer)) return SMTP_ERROR_WRITE;
    if(smtp_read(sm) || strcmp(sm->cmd,"235")) return SMTP_ERROR_READ;
    sm->status = SMTP_STATUS_SEND;

    return 0;
}

/**
 * 生成smtp格式的时间字符串：Wed, 30 Jan 2019 22:45:26 +0800 (CST)
 *        这里直接返回在堆栈上的缓冲区，意味着只能立刻使用，一旦堆栈有变
 *        动就不能在使用了。
 * @param buffer
 * @return 返回 buffer
 */
static char* smtp_time(char* buffer)
{
    time_t t2;
    time(&t2);

    struct tm t;
    localtime_r(&t2,&t);
    // localtime_s(&t2,&t);
    
    static char* week[] = {"Sun" , "Mon" , "Tue" , "Wed" , "Thu" , "Fri" , "Sat"};
    static char* month[] = {"Jan" , "Feb" , "Mar" , "Apr" , "May" , "Jun" , "Jul" , "Aug" , "Sep" , "Oct" , "Nov" , "Dec"};
    // C      Date: Wed, 30 Jan 2019 05:48:24 --84199186
    // C      Date: Wed, 30 Jan 2019 22:45:26 +0800 (CST)
    sprintf(buffer,"%s, %02d %s %04d %02d:%02d:%02d %s%02d00 (CST)",
    // Linux
    week[t.tm_wday],t.tm_mday,month[t.tm_mon], t.tm_year + 1900,t.tm_hour,t.tm_min,t.tm_sec,t.tm_gmtoff >= 0? "+":"-",abs(t.tm_gmtoff / 3600));  
    // TODO: Windows  
    // week[t.tm_wday],t.tm_mday,month[t.tm_mon], t.tm_year + 1900,t.tm_hour,t.tm_min,t.tm_sec,t.__tm_gmtoff >= 0? "+":"-",abs(t.__tm_gmtoff / 3600));  // C99
    return buffer;
}

static int send_mail(struct smtp* sm)
{
    // MAIL FROM
    char buffer[256];
    int size = sprintf(buffer,"MAIL FROM: <%s>\r\n",sm->user_name);
    if(smtp_write(sm->socket,buffer)) return SMTP_ERROR_WRITE;
    if(smtp_read(sm) || strcmp(sm->cmd,"250")) return SMTP_ERROR_READ;

    // RCPT TO
    int i;
    for(i = 0; i < sm->to_len; i++)
    {
        size = sprintf(buffer,"RCPT TO: <%s>\r\n",sm->to[i]);
        if(smtp_write(sm->socket,buffer)) return SMTP_ERROR_WRITE;
        if(smtp_read(sm) || strcmp(sm->cmd,"250")) return SMTP_ERROR_READ;
    }

    // DATA,最后一行是 "\r\n.\r\n" 表示邮件结束
    if(smtp_write(sm->socket,"DATA\r\n")) return SMTP_ERROR_WRITE;
    if(smtp_read(sm) || strcmp(sm->cmd,"354")) return SMTP_ERROR_READ;

    // 分配足够大缓冲区存储邮件头，唯一不确定的就是群发数量，这里先统计一下发送目标占用的字节大小
    int to_size = 0;
    for(i = 0; i < sm->to_len; i++) to_size += strlen(sm->to[i]);

    char header[to_size + 512 + strlen(sm->user_name)];
    
    // From
    char * from = (char*)malloc(sizeof(char)*(strlen("From: %s<%s>\r\n")+strlen(sm->user_name)+strlen(sm->user_name)));
    int pos = sprintf(from,"From: %s<%s>\r\n",sm->user_name,sm->user_name);
    //sprintf(&header[pos],"From: %s<%s>\r\n",sm->user_name,sm->user_name);
    // int pos = strlen("MIME-Version: 1.0\r\nContent-Type: text/html\r\n");
    memcpy(header,from,pos);
    
    // To:
    for(i = 0; i < sm->to_len; i++)
    {
        pos += sprintf(&header[pos],"To: %s\r\n",sm->to[i]);
    }
    
    // CC: TODO
    if(sm->cc != NULL && sm->cc_len > 0)
    {
        for(i = 0; i < sm->cc_len; i++)
        {
            pos += sprintf(&header[pos],"Cc: %s\r\n",sm->cc[i]);
        }
    }
    
    // Subject:
    pos += sprintf(&header[pos],"Subject: %s\r\n",sm->subject);
    
    // char mime_version[] = "Mime-Version: 1.0\r\nContent-Type: text/plain; charset=UTF-8\r\n";
    char mime_version[] = "Mime-Version: 1.0\r\nContent-Type: text/html; charset=utf-8\r\n";
    
    pos += sprintf(&header[pos], mime_version);
    
    // Content-Transfer-Encoding: base64
    pos += sprintf(&header[pos],"Content-Transfer-Encoding: base64\r\n");
    pos += sprintf(&header[pos],"Message-ID: <%ld.%s>\r\n",time(NULL),sm->user_name);
    
    char date[50];
    pos += sprintf(&header[pos],"Date: %s\r\n\r\n",smtp_time(date));
    free(from);
    
    if(smtp_write(sm->socket,header)) return SMTP_ERROR_WRITE;
    if(smtp_write(sm->socket,sm->content)) return SMTP_ERROR_WRITE;
    if(smtp_write(sm->socket,"\r\n.\r\n")) return SMTP_ERROR_WRITE;
    if(smtp_read(sm) || strcmp(sm->cmd,"250")) return SMTP_ERROR_READ;

    sm->status = SMTP_STATUS_QUIT;
    return 0;
}

static int quit(struct smtp* sm)
{
    // if(smtp_write(sm->socket,"QUIT \r\n",strlen("QUIT \r\n"))) return SMTP_ERROR_WRITE;
    if(smtp_write(sm->socket,"QUIT \r\n")) return SMTP_ERROR_WRITE;
    if(smtp_read(sm) || strcmp(sm->cmd,"221")) return SMTP_ERROR_READ;

    sm->status = SMTP_STATUS_NULL;

    return 0;
}

typedef int (*SMTP_FUN)(struct smtp*);
static const SMTP_FUN smtp_fun[SMTP_STATUS_MAX] = {NULL,hello,auth,send_mail,quit};

int smtp_send(const char* domain,int port,const char* user_name,const char* password,
              const char* subject,const char* content,const char** to,int to_len,const char** cc, int cc_len)
{
    struct hostent* host = gethostbyname(domain);
    if(!host)
    {
        printf("domain can not find！\n");
        return SMTP_ERROR_DOMAIN;
    }

    if(host->h_addrtype != AF_INET)
    {
        // Linux 
        // if(host->h_addrtype == AF_INET6) printf("ipv6 is not support!\n");
        // Windows
        if(host->h_addrtype == AF_INET) printf("ipv6 is not support!\n");
        else printf("address type is not support %d\n ",host->h_addrtype);
        return SMTP_ERROR_DOMAIN;
    }

    int sock_fd = socket(AF_INET,SOCK_STREAM,0);
    if(-1 == sock_fd)
    {
        printf("can not create socket!\n");
        return SMTP_ERROR_SOCKET;
    }

    // 连接到服务器
    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr = *(struct in_addr*)host->h_addr_list[0];
    memset(local.sin_zero,0,sizeof(local.sin_zero));

    if(-1 == connect(sock_fd,(struct sockaddr*)&local,sizeof(local)))
    {
        printf("can not connect socket!\n");
        return SMTP_ERROR_CONNECT;
    }

    printf("connect ok ,ip address %s \n",inet_ntoa(local.sin_addr));
    
    struct smtp sm = {.domain=domain,.user_name=user_name,.password=password,.subject=subject,
                      .content=base64_encode(content),.status=SMTP_STATUS_EHLO,.socket=sock_fd,.to=to,
                      .to_len=to_len,.cc=cc,.cc_len=cc_len};

    // free(sm.content);
    // 这里应该是服务器欢迎信息，如果状态不是220，直接返回
    if(smtp_read(&sm) || strcmp(sm.cmd,"220")) return SMTP_ERROR_READ;

    while(sm.status != SMTP_STATUS_NULL)
    {
        int error = smtp_fun[sm.status](&sm);
        if(error)
        {
            printf("error = %d\n",error);
            return error;
        }
    }

    close(sock_fd);

    return 0;
}


