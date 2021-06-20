/**
 * @file smtp.h
 *
 */

#ifndef SMTP_H_
#define SMTP_H_


#define SMTP_BUFFER_SIZE 1024

/**
 * @struct smtp
 */
struct smtp
{
    const char* domain;
    const char* user_name;
    const char* password;
    const char* subject;
    const char* content;
    const char** to;
    // char** to;
    int to_len;
    char ** cc;
    int cc_len;
    char ** attachment;
    int file_count;
    int status;
    int socket;
    char buffer[SMTP_BUFFER_SIZE];
    char* cmd;
    char* data;
};


#ifdef __cplusplus
extern "C"
{
#endif

/**
 *  使用smtp协议发送邮件。不做参数检查，外部自己处理好。
 * @param domain        域名
 * @param port          端口号
 * @param user_name     用户名
 * @param password      密码
 * @param subject       标题
 * @param content       邮件内容
 * @param to            发送目标
 * @param to_len        有多少个目标
 * @return 如果发送成功返回0，否则返回一个正数表示错误原因。
 *
 * @par Sample Code:
 * @code
 *  smtp_send("smtp.163.com",25,"xxx@163.com","mypassword","email-subject","email-content",
 *          "xxx@gmail.com");
 * @code
 *
 */
int smtp_send(const char* domain,int port,const char* user_name,const char* password,
              const char* subject,const char* content,const char** to,int to_len,const char** cc, int cc_len);


#ifdef __cplusplus
}
#endif /* end of extern "C" */

#endif /* SMTP_H_ */

