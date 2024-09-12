# smtp项目  

[toc]

## smtp 项目介绍  

SMTP 是一个基于 C 语言实现的邮件发送项目，实现了以命令行的方式发送邮件。只实现了最基础的功能。仅供学习C语言或者smtp协议使用。
密送和附件功能待实现。

## smtp 代码结构  

```sh
smtp
├── lib
│   ├── base64.c  # base64编码实现文件
│   └── base64.h  # base64编码头文件
├── main.c        # main函数
├── makefile      # makefile文件
├── os
│   ├── linux.c   # linux下实现文件
│   └── windows.c # windows下实现文件
├── README.md
├── smtp          # Linux下编译生成的可执行文件
├── smtp.c        # smtp.c文件，包含了smtp.h中声明的函数实现
├── smtp.exe      # windows下编译生成的可执行文件
└── smtp.h        # smtp.h文件，包含了smtp.c中用到的函数声明
```

## smtp 源码编译  

1. 下载源码到任意目录:  

 ```bash
git clone https://github.com/ToyCodeLite/smtp.git
 ```

2. 切换到代码目录  

```bash
cd ./smtp
```

3. (optional) 修改makefile文件(仅Windows平台下编译需要修改)：

> 当前makefile编译Windows文件是基于交叉编译写的，makefile配置的是x86_64-w64-mingw32-gcc编译器
> 如果在Windows下使用的是mingw-32编译的话,需要修改makefile文件，将`GCC = x86_64-w64-mingw32-gcc`改为`GCC = mingw32-gcc`或者`GCC = gcc`

4. 编译：

```bash
make
```

(optional) Linux下进行交叉编译：

```bash
sudo apt-get install mingw-w64  # 安装mingw-w64
export OS=Windows_NT            # 临时设置环境变量OS的值为Windows_NT(仅跨平台的交叉编译需设置环境变量)
make
```

成功后在当前目录下生成可执行文件，例如：smtp.exe(windows)或smtp(Linux)

## smtp 使用说明  

1. 查看帮助信息：

```bash
# display help info and exit
smtp.exe --help # windows platform
# or
./smtp --help # linux platform
```

```log
$ ./smtp --help




----------------------------------
Usage: ./smtp [Option]
    -h    --help             show this
    -m    --show             Display sending info
    -d    --domain           smtp server address[smtp.163.com]
    -u    --user             username:Sender Email
    -p    --password
    -P    --port             smtp server port[25]
    -t    --to               Receiving Email list
    -c    --cc               Carbon Copy Email list
    -s    --subject          subject
    -b    --content          email body partion
    -f    --attachment       filname list
```

Note:
主送/抄送: 多个收件人地址之间用逗号隔开。
-p(小写): 邮箱服务器密码(一般都是授权码)，注意：邮箱需要开启SMTP服务，才能发送邮件。
-P(大写): smtp服务器端口，默认为25
-d/--domain: 不输入默认为smtp.163.com

2. 使用命令行发送邮件：

2.1. Linux:

```bash
# 短参数形式
./smtp -d smtp.163.com -P 25 -u xxxxxx@163.com -p password -t xxxxxx@qq.com -s subject_linux_demo -b content_body -m
# 长参数形式
./smtp --domain smtp.163.com --port 25 --user xxxxxx@163.com --password password --to xxxxxx@qq.com --subject subject_linux_demo --content content_body --show
```

2.2. Windows:

```bash
# 短参数形式
smtp.exe -d smtp.163.com -P 25 -u xxxxxx@163.com -p password -t xxxxxx@qq.com -s subject_windows_demo -b content_body -m
# 长参数形式
smtp.exe --domain smtp.163.com --port 25 --user xxxxxx@163.com --password password --to xxxxxx@qq.com --subject subject_windows_demo --content content_body --show
```
