/**
 * @file smtp.h
 *
 */

#ifndef SMTP_H_
#define SMTP_H_


#define SMTP_BUFFER_SIZE 1024

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
 * @struct smtp
 */
struct smtp
{
    const char* domain;
    int port;
    unsigned char* user_name;
    unsigned char* password;
    char* subject;
    unsigned char* content;
    char** to;
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

int smtp_read(struct smtp* sm);

int hello(struct smtp*);
int auth(struct smtp*);
int send_mail(struct smtp*);
int quit(struct smtp*);

typedef int (*SMTP_FUN)(struct smtp*);
// const SMTP_FUN smtp_fun[SMTP_STATUS_MAX] = {NULL,hello,auth,send_mail,quit};
extern const SMTP_FUN smtp_fun[SMTP_STATUS_MAX];  // 声明，不定义

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 发送邮件
 */
int smtp_send(struct smtp* sm);

#ifdef __cplusplus
}
#endif /* end of extern "C" */

#endif /* SMTP_H_ */

