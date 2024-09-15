# smtp client  

* [中文版](./README_CN.md)

## smtp client introduction  

It is a simple email sending program implemented in C language. It only implements the basic functions. It is only for learning C language or the smtp protocol.  

## Overview  

### Features  

* Sending Emails from the Command Line
* Supports Both Linux and Windows Platforms

### files tree

```sh
smtp
├── lib
│   ├── base64.c  # Base64 encoding and decoding implementation file
│   └── base64.h  # Base64 header file
├── main.c        # Main function
├── makefile 
├── os
│   ├── linux.c   # Implementation file for Linux
│   └── windows.c # Implementation file for Windows
├── README.md
├── smtp          # Executable file generated after compilation on Linux
├── smtp.c        # smtp.c file, contains the implementation of functions declared in smtp.h
├── smtp.exe      # Executable file generated after compilation on Windows
└── smtp.h        # smtp.h file, contains declarations of functions used in smtp.c
```

## smtp compilation  

1. Download the source code to any directory:  

 ```bash
git clone https://github.com/ToyCodeLite/smtp.git
 ```

2. enter the source code directory  

```bash
cd ./smtp
```
`

3. (Optional) Modify the Makefile (only necessary for compilation on Windows):

The current Makefile is set up for cross-compiling to Windows, configured to use the `x86_64-w64-mingw32-gcc` compiler.
If you are using MinGW on Windows directly, you need to modify the Makefile by changing `GCC = x86_64-w64-mingw32-gcc` to `GCC = mingw32-gcc` or `GCC = gcc`.  

4. make build：

```bash
make
```

(Optional) Cross Compile for Windows on Linux

```bash
sudo apt-get install mingw-w64  # install mingw-w64 compiler package on linux
export OS=Windows_NT            # set the environment variable `OS` to `Windows_NT` (needed only for cross-platform compilation).
make
```

After a successful build, an executable file will be generated in the current directory, such as `smtp.exe` (for Windows) or `smtp` (for Linux).

## SMTP Usage Instructions  

1. Usage Info：

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
> TO/CC: Separate multiple recipient addresses with `,`(commas).  
> -p (lowercase): Email password (usually an authorization code). Note: SMTP service must be enabled on the email account to send emails.  
> -P (uppercase): SMTP server port, default is `25`.  
> -d/--domain: Default is `smtp.163.com` if not specifie  

2. send an email using the command line：

2.1. Linux:

```bash
# Short Parameter 
./smtp -d smtp.163.com -P 25 -u xxxxxx@163.com -p password -t xxxxxx@qq.com -s subject_linux_demo -b content_body -m
# Long Parameter 
./smtp --domain smtp.163.com --port 25 --user xxxxxx@163.com --password password --to xxxxxx@qq.com --subject subject_linux_demo --content content_body --show
```

2.2. Windows:

```bash
# Short Parameter 
smtp.exe -d smtp.163.com -P 25 -u xxxxxx@163.com -p password -t xxxxxx@qq.com -s subject_windows_demo -b content_body -m
# Long Parameter
smtp.exe --domain smtp.163.com --port 25 --user xxxxxx@163.com --password password --to xxxxxx@qq.com --subject subject_windows_demo --content content_body --show
```
