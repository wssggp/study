## 日常操作命令

#### 查看当前所在的工作目录的全路径 pwd
```
[root@localhost ~]# pwd
/root
```

#### 查看当前系统的时间 date
```
[root@localhost ~]# date +%Y-%m-%d
2016-07-26

date +%Y-%m-%d  --date="-1 day" #加减也可以 month | year
2016-07-25

[root@localhost ~]# date -s "2016-07-28 16:12:00" ## 修改时间
Thu Jul 28 16:12:00 PDT 2016
```

#### 查看有谁在线（哪些人登陆到了服务器）
who  查看当前在线
```
[root@localhost ~]# who
hadoop   tty1         2016-07-26 00:01 (:0)
hadoop   pts/0        2016-07-26 00:49 (:0.0)
root     pts/1        2016-07-26 00:50 (192.168.233.1)
```

#### last 查看最近的登陆历史记录
```
[root@localhost ~]# last
root     pts/1        192.168.233.1    Tue Jul 26 00:50   still logged in   
hadoop   pts/0        :0.0             Tue Jul 26 00:49   still logged in   
hadoop   tty1         :0               Tue Jul 26 00:01   still logged in   
reboot   system boot  2.6.32-573.el6.x Tue Jul 26 07:58 - 16:23 (2+08:24)
```

#### 关机/重启
```
关机（必须用root用户）
shutdown -h now  ## 立刻关机
shutdown -h +10  ##  10分钟以后关机
shutdown -h 12:00:00  ##12点整的时候关机
halt   #  等于立刻关机
```

##### 重启
```
shutdown -r now
reboot   # 等于立刻重启
```

#### 清屏clear    
或者用快捷键  ctrl + l

#### 退出当前进程

ctrl+c   ##有些程序也可以用q键退出

#### 挂起当前进程
```
ctrl+z   ## 进程会挂起到后台
bg jobid  ## 让进程在后台继续执行
fg jobid   ## 让进程回到前台
```

#### echo 
相当于java中System.out.println(userName)

``` 
[root@localhost ~]# a="hi boy"
[root@localhost ~]# echo a
a
[root@localhost ~]# echo $a
hi boy
```

### 目录操作

#### 查看目录信息
```
ls /   ## 查看根目录下的子节点（文件夹和文件）信息
ls -al ##  -a是显示隐藏文件   -l是以更详细的列表形式显示
ls -l  ##有一个别名： ll    可以直接使用ll  <是两个L>
```

#### 切换工作目录
```
cd  /home/hadoop    ## 切换到用户主目录
cd ~     ## 切换到用户主目录
cd -     ##  回退到上次所在的目录
cd  什么路径都不带，则回到用户的主目录
```

#### 创建文件夹

```
mkdir aaa     ## 这是相对路径的写法 
mkdir  /data    ## 这是绝对路径的写法 
mkdir -p  aaa/bbb/ccc   ## 级联创建目录
```

#### 删除文件夹

```
rmdir  aaa   ## 可以删除空目录
rm  -r  aaa   ## 可以把aaa整个文件夹及其中的所有子节点全部删除
rm  -rf  aaa   ## 强制删除aaa
```

#### 修改文件夹名称

```
mv  aaa  boy
mv本质上是移动
mv  install.log  aaa/  将当前目录下的install.log 移动到aaa文件夹中去

rename 可以用来批量更改文件名
[root@localhost aaa]# ll
total 0
-rw-r--r--. 1 root root 0 Jul 28 17:33 1.txt
-rw-r--r--. 1 root root 0 Jul 28 17:33 2.txt
-rw-r--r--. 1 root root 0 Jul 28 17:33 3.txt
[root@localhost aaa]# rename .txt .txt.bak *
[root@localhost aaa]# ll
total 0
-rw-r--r--. 1 root root 0 Jul 28 17:33 1.txt.bak
-rw-r--r--. 1 root root 0 Jul 28 17:33 2.txt.bak
-rw-r--r--. 1 root root 0 Jul 28 17:33 3.txt.bak
```

### 文件操作
#### 创建文件

```
touch  somefile.1       
## 创建一个空文件

echo "hi,boy" > somefile.2     
## 利用重定向“>”的功能，将一条指令的输出结果写入到一个文件中，会覆盖原文件内容，如果指定的文件不存在，则会创建出来

echo "hi baby" >> somefile.2    
## 将一条指令的输出结果追加到一个文件中，不会覆盖原文件内容
```

####  vi文本编辑器

```
最基本用法
vi  somefile.4
1 首先会进入“一般模式”，此模式只接受各种快捷键，不能编辑文件内容
2 按i键，就会从一般模式进入编辑模式，此模式下，敲入的都是文件内容
3 编辑完成之后，按Esc键退出编辑模式，回到一般模式；
4 再按：，进入“底行命令模式”，输入wq命令，回车即可，如果对修改的内容不想保存，直接:q!退出即可

常用快捷键
一些有用的快捷键（在一般模式下使用）：
h j k l 左 下 上 右移动，也可以用方向键移动
shift + ^ 移到行开头
shift + $ 移到行尾
w   光标位置向后移动一个单词
3w  光标位置向后移动三个单词
b   光标位置向前移动一个单词
3b  光标位置向前移动三个单词
a   在光标后一位开始插入
A   在该行的最后插入
o   在光标的下一行插入
O   在光标的上一行插入
I   在该行的最前面插入
gg   直接跳到文件的首行
G    直接跳到文件的末行
dd    删除一行
3dd   删除3行
yy    复制一行
3yy   复制3行
p     粘贴
u     undo
v        进入字符选择模式，选择完成后，按y复制，按p粘贴
ctrl+v   进入块选择模式，选择完成后，按y复制，按p粘贴
shift+v  进入行选择模式，选择完成后，按y复制，按p粘贴

查找并替换
1 显示行号
:set nu
2 隐藏行号
:set nonu
3 定位到第100行
:100
4 查找关键字
:/you       ## 效果：查找文件中出现的you，并定位到第一个找到的地方，按n可以定位到下一个匹配位置（按N定位到上一个）
5 替换操作
:s/sad/bbb    查找光标所在行的第一个sad，替换为bbb
:%s/sad/bbb      查找文件中所有sad，替换为bbb
```

#### 拷贝/删除/移动

```
cp  somefile.1   /home/hadoop/
rm /home/hadoop/somefile.1
rm -f /home/hadoop/somefile.1
mv /home/hadoop/somefile.1 ../
```

#### 查看文件内容

```
cat    somefile      一次性将文件内容全部输出（控制台）
more   somefile      可以翻页查看, 下翻一页(空格)    上翻一页（b）   退出（q）
less   somefile      可以翻页查看,下翻一页(空格)    上翻一页（b），上翻一行(↑)  下翻一行（↓）  可以搜索关键字（/keyword）
跳到文件末尾： G
跳到文件首行： gg
退出less ：  q

tail -10  install.log  查看文件尾部的10行
tail +10  install.log  查看文件 10-->末行
tail -f install.log    小f跟踪文件的唯一inode号，就算文件改名后，还是跟踪原来这个inode表示的文件
tail -F install.log    大F按照文件名来跟踪

head -10  install.log   查看文件头部的10行
```

### 打包压缩

#### gzip

```
压缩
gzip a.txt

解压
gunzip a.txt.gz
gzip -d a.txt.gz
```

#### bzip2
```
压缩
bzip2 a

解压
bunzip2 a.bz2
bzip2 -d a.bz2
```

#### tar

一般压缩和解压用tar命令的比较多，记住下面4,5,6的用法一般就够了
```
1. 打包：将指定文件或文件夹，只是打包，并没有压缩，打包的后缀为.tar,压缩的后缀为.gz
tar -cvf bak.tar  ./aaa
2. 将/etc/password追加文件到bak.tar中(r)
tar -rvf bak.tar /etc/password

3. 解压
tar -xvf bak.tar

4. 打包并压缩
tar -zcvf a.tar.gz  aaa/

5. 解包并解压缩(重要的事情说三遍!!!)
tar  -zxvf  a.tar.gz
6. 解压到/usr/下
tar  -zxvf  a.tar.gz  -C  /usr

7. 查看压缩包内容
tar -ztvf a.tar.gz
zip/unzip

8. 打包并压缩成bz2
tar -jcvf a.tar.bz2

9. 解压bz2
tar -jxvf a.tar.bz2
```

### 查找命令

#### 常用查找命令的使用
```
1、查找可执行的命令所在的路径：
which ls

2、查找可执行的命令和帮助的位置：
whereis ls

3、从某个文件夹以文件名开始查找文件
find /work/ -name "*.cpp"
find / -name "*.cpp" -ls

4、查找并删除,执行下面的命令要确定自己知道在做什么
find / -name "hadooop*" -ok rm {} \;
find / -name "hadooop*" -exec rm {} \;

5、查找用户为guopeng的文件
find  /usr  -user  guopeng  -ls

6、查找用户为guopeng的文件夹
find /home -user guopeng -type d -ls

7、查找权限为777的文件
find / -perm -777 -type d -ls

9、显示命令历史
history
```


#### grep命令

```
1 基本使用
查询包含guopeng的行
grep guopeng  /etc/passwd
grep guopeng  ./*.txt 

2 cut截取以:分割保留第七段
grep guopeng /etc/passwd | cut -d: -f7

3 查询不包含guopeng的行
grep -v guopeng /etc/passwd

4 正则表达包含guopeng
grep 'guopeng' /etc/passwd

5 正则表达(点代表任意一个字符)
grep 'g.*g' /etc/passwd

6 正则表达以guopeng开头
grep '^guopeng' /etc/passwd

7 正则表达以guopeng结尾
grep 'guopeng$' /etc/passwd

规则：
.  : 任意一个字符
a* : 任意多个a(零个或多个a)
a? : 零个或一个a
a+ : 一个或多个a
.* : 任意多个任意字符
\. : 转义.
o\{2\} : o重复两次

查找不是以#和$开头的行
grep -v '^#' a.txt | grep -v '^$' 

以h或r开头的
grep '^[hr]' /etc/passwd

不是以h和r开头的
grep '^[^hr]' /etc/passwd

不是以h到r开头的
grep '^[^h-r]' /etc/passwd
```

### 替换命令 sed

linux替换目录下所有文件中的某字符串
比如，要将目录/modules下面所有文件中的zhangsan都修改成lisi，这样做：

```shell
sed -i “s/zhangsan/lisi/g” `grep zhangsan -rl /modules`
```

解释一下：

-i 表示inplace edit，就地修改文件
-r 表示搜索子目录
-l 表示输出匹配的文件名

这个命令组合很强大，要注意备份文件

在sed命令中,\是转义字符，所以如果要匹配\字符需要转义

 如：将当前目录下以.md结尾的文件中`D:\work\notes\study`替换成`..\..`

```shell
 sed -i "s/D:\\\\work\\\\notes\\\\study/..\\\\../g" *.md
```



### 文件权限的操作

#### linux文件权限的描述格式解读

```
drwxr-xr-xr      （也可以用二进制表示  111 101 101  -->  755）

d：标识节点类型（d：文件夹   -：文件  l:链接）
r：可读   w：可写    x：可执行 
第一组rwx：  ## 表示这个文件的拥有者对它的权限：可读可写可执行
第二组r-x：  ## 表示这个文件的所属组用户对它的权限：可读，不可写，可执行
第三组r-x：  ## 表示这个文件的其他用户（相对于上面两类用户）对它的权限：可读，不可写，可执行
```

#### 修改文件权限

```
chmod g-rw haha.dat		 ## 表示将haha.dat对所属组的rw权限取消
chmod o-rw haha.dat		 ## 表示将haha.dat对其他人的rw权限取消
chmod u+x haha.dat		 ## 表示将haha.dat对所属用户的权限增加x
chmod a-x haha.dat               ## 表示将haha.dat对所用户取消x权限

也可以用数字的方式来修改权限
chmod 664 haha.dat   
就会修改成   rw-rw-r--
如果要将一个文件夹的所有内容权限统一修改，则可以-R参数
chmod -R 770 aaa/
```

#### 修改文件所有权

```
<只有root权限能执行>
chown angela  aaa		## 改变所属用户
chown :angela  aaa		## 改变所属组
chown angela:angela aaa/	## 同时修改所属用户和所属组
```

### 基本的用户管理

```
添加一个用户：
useradd spark
passwd  spark     根据提示设置密码；
即可

删除一个用户：
userdel -r spark     加一个-r就表示把用户及用户的主目录都删除
```
#### 添加用户

```
添加一个tom用户，设置它属于users组，并添加注释信息
分步完成：useradd tom
          usermod -g users tom
	  usermod -c "hr tom" tom
一步完成：useradd -g users -c "hr tom" tom

设置tom用户的密码
passwd tom
```

#### 修改用户

```
修改tom用户的登陆名为tomcat
usermod -l tomcat tom

将tomcat添加到sys和root组中
usermod -G sys,root tomcat

查看tomcat的组信息
groups tomcat
```

#### 用户组操作

```
添加一个叫america的组
groupadd america

将jerry添加到america组中
usermod -g america jerry

将tomcat用户从root组和sys组删除
gpasswd -d tomcat root
gpasswd -d tomcat sys

将america组名修改为am
groupmod -n am america
```

#### 为用户配置sudo权限

```
用root编辑 vi /etc/sudoers
在文件的如下位置，为hadoop添加一行即可
root    ALL=(ALL)       ALL     
hadoop  ALL=(ALL)       ALL

然后，hadoop用户就可以用sudo来执行系统级别的指令
[root@localhost ~]$ sudo useradd xiaoming
```

### 系统管理操作

#### 挂载外部存储设备

```
可以挂载光盘、硬盘、磁带、光盘镜像文件等
1/ 挂载光驱
mkdir   /mnt/cdrom      创建一个目录，用来挂载
mount -t iso9660 -o ro /dev/cdrom /mnt/cdrom/     将设备/dev/cdrom挂载到 挂载点 ：  /mnt/cdrom中

2/ 挂载光盘镜像文件（.iso文件）
mount -t iso9660 -o loop  /home/hadoop/Centos-6.7.DVD.iso /mnt/centos
注：挂载的资源在重启后即失效，需要重新挂载。要想自动挂载，可以将挂载信息设置到/etc/fstab配置文件中，如下：
/dev/cdrom              /mnt/cdrom              iso9660 defaults        0 0

3/ 卸载 umount
umount /mnt/cdrom

4/ 存储空间查看
df -h
```

#### 统计文件或文件夹的大小

```
du -sh  /mnt/cdrom/packages
df -h    查看磁盘的空间
```

#### 系统服务管理

```
service sshd status
service sshd stop 
service sshd start
service sshd restart
```

#### 系统启动级别管理

```
vi  /etc/inittab

       # Default runlevel. The runlevels used are:
       #   0 - halt (Do NOT set initdefault to this)
       #   1 - Single user mode
       #   2 - Multiuser, without NFS (The same as 3, if you do not have networking)
       #   3 - Full multiuser mode
       #   4 - unused
       #   5 - X11
       #   6 - reboot (Do NOT set initdefault to this)
       #
       id:3:initdefault:
       ## 通常将默认启动级别设置为：3
```

#### 进程管理

```
通过ps -ef查询所有进程信息，在通过管道|（管道就是将一个命令的输出作为另一个命令的输入，像流水一样） 加grep查找自己要找的进程，就可以找到对应的进程信息
[root@fed-master zhangbei]# ps -ef |grep httpd
root     11635     1  0 10:26 ?        00:00:00 /usr/sbin/httpd -DFOREGROUND
apache   11636 11635  0 10:26 ?        00:00:00 /usr/sbin/httpd -DFOREGROUND
apache   11638 11635  0 10:26 ?        00:00:00 /usr/sbin/httpd -DFOREGROUND
apache   11639 11635  0 10:26 ?        00:00:00 /usr/sbin/httpd -DFOREGROUND
apache   11640 11635  0 10:26 ?        00:00:00 /usr/sbin/httpd -DFOREGROUND
root     12002  2324  0 10:45 pts/0    00:00:00 grep --color=auto httpd
如果我们需要杀死服务可以使用kill命令加查询到的进程id，这里使用kill -9 
kill -9 11635
这里有很多个进程，我们一般只需要杀死父进程就可以，这里11635进程是11636 11638 11639 11640进程的父进程，12002是执行ps命令时的进程，可以忽略，可以看到执行ps命令时，对应的每行信息有两个进程id，其中第一个是进程自己的id，后面一个是其父进程的id。
```

#### 服务管理

```
1 后台服务管理，主要是对服务的启动停止以及状态的查看，这里以httpd为例

service命令是老的Linux系统用的
service httpd status    查看指定服务的状态
service httpd stop     停止指定服务
service httpd start     启动指定服务
service httpd restart   重启指定服务
service --status-all       查看系统中所有的后台服务

现在比较新的系统一般用systemctl命令
systemctl status httpd
systemctl start httpd
systemctl stop httpd
systemctl restart httpd
systemctl status -all

2 设置后台服务的自启配置
老系统命令
chkconfig   查看所有服务器自启配置
chkconfig httpd off   关掉指定服务的自动启动
chkconfig httpd on   开启指定服务的自动启动
新系统命令
systemctl enable httpd        // 开机启动 httpd 服务
systemctl disable httpd         // 开机关闭 httpd 服务
```

#### 关闭防火墙

```
不同的系统关闭命令不一样，具体可以在需要时网上查询

 关闭 RHEL6/CGSLv4  系统的防火墙
# chkconfig --del ip6tables
# chkconfig --del iptables
# /etc/init.d/ip6tables stop
# /etc/init.d/iptables stop
 关闭 RHEL7  系统的防火墙

# systemctl disable iptables
# systemctl disable ip6tables
# systemctl disable firewalld
# systemctl stop iptables
# systemctl stop ip6tables
# systemctl stop firewalld
```

### 网络相关命令

#### wget

基本的语法是：wget [参数列表] URL，wget功能很强大，这里只介绍如何从网上下载文件

以下的例子是从网络下载一个文件并保存在当前目录

```
wget http://cn.wordpress.org/wordpress-3.1-zh_CN.zip
```

使用wget -c断点续传，使用wget -c重新启动下载中断的文件:

```
wget -c http://cn.wordpress.org/wordpress-3.1-zh_CN.zip
```

#### curl

curl 是常用的命令行工具，用来请求 Web 服务器。它的名字就是客户端（client）的 URL 工具的意思。它的功能非常强大，命令行参数多达几十种,可以用来模拟各种http请求，多用于接口测试。

-X参数指定 HTTP 请求的方法，如GET POST PUT等
-v参数输出通信的整个过程，用于调试。
-H参数添加 HTTP 请求的标头。
-d参数用于发送 POST 请求的数据体。
--cacert 指定证书位置，如果不需要证书的情况下不用改参数

如下为发送一个POST请求到指定的web或rest服务器， 行末“\”是表示换行，表示下面一行内容是在紧接在这一行。
```
curl -v --cacert conf/certs/ca.crt \
        -X POST \
        -H "Content-Type: application/json; charset=UTF-8" \
        https://zone1.gateway.skybilityha.com/path/of/url \
        -d '{"reason": "test", "dstState": "drMaster", "curState": "drSlave"}'
```

#### scp rsync远程拷贝

- rsync只对差异文件做更新，可以做增量或全量备份；而scp只能做全量备份。简单说就是rsync只传修改了的部分，如果改动较小就不需要全部重传
- rsync不是加密传输，而scp是加密传输，使用时可以按需选择。

##### scp

1. 命令格式：
scp [参数] [原路径] [目标路径]

2. 复制文件： 
```
scp 本地文件名 远端用户名@远端主机IP:新文件名
scp /test.txt root@172.16.70.71:/home/newtest.txt

scp local_file remote_username@remote_ip:remote_file 
scp local_file remote_ip:remote_folder   
scp local_file remote_ip:remote_file 
指定了用户名的，只需输入密码。没有指定的，会指示输入用户和密码
```
3. 复制目录：(复制目录要参数-r)
```
scp -r local_folder remote_username@remote_ip:remote_folder 

scp -rp /data root@172.16.70.71:/home   #-r 递归复制整个目录，-p保留原文件的修改时间，访问时间和访问权限
```

4. 从远程服务器复制到本地服务器：
```
从远程复制到本地，只要将从本地复制到远程的命令后面2个参数互换顺序就行了。
scp root@192.168.120.204:/opt/soft/nginx-0.5.38.tar.gz /opt/soft/
```
5. scp默认是22端口，如果改变端口要用大写的-P参数
```
scp -P 7777 -rp /data root@172.16.70.71:/my/backup/
```

##### rsync

1. 命令格式
rsync [OPTION] SRC DEST

2. 拷贝本地文件，SRC源、DES目的。如：rsync -avz /data /backup

3. 本地复制到远程主机  rsync [OPTION] SRC [USER@]host:DEST

```
rsync -avz *.c 172.16.70.71:/tmp
```

4. 远程拷本地 ：rsync [OPTION] [USER@]HOST:SRC DEST

```
rsync -avz root@172.16.70.71:/tmp /data
```

rsync的部分同步参数选项：
```
-a ：归档模式，表示以递归模式传输文件，并保持文件所有属性相当于-rtopgdl
-v :详细模式输出，传输时的进度等信息
-z :传输时进行压缩以提高效率—compress-level=num可按级别压缩
-r :对子目录以递归模式，即目录下的所有目录都同样传输。
-t :保持文件的时间信息—time
-o ：保持文件属主信息owner
-p ：保持文件权限
-g ：保持文件的属组信息
-P :--progress 显示同步的过程及传输时的进度等信息
-e ：使用的信道协议，指定替代rsh的shell程序。例如：ssh
-D :保持设备文件信息
-l ：--links 保留软连接
--progress  :显示备份过程
--delete    :删除那些DST中SRC没有的文件
--exclude=PATTERN 　指定排除不需要传输的文件模式
```

我平时用rsync比较多，一般直接rsync -av SRC DES目，其他选项没必要都记住

#### nc

netcat是网络工具中的瑞士军刀，它能通过TCP和UDP在网络中读写数据

参数
```
-l 用于指定 nc 将处于侦听模式。指定该参数，则意味着 nc 被当作 server，侦听并
接受连接，而非向其它地址发起连接。
-p 暂未用到（老版本的 nc 可能需要在端口号前加-p 参数，下面测试环境是
centos6.6，nc 版本是 nc-1.84，未用到-p 参数）
-s 指定发送数据的源 IP 地址，适用于多网卡机
-u 指定 nc 使用 UDP 协议，默认为 TCP
-v 输出交互或出错信息，新手调试时尤为有用
-w 超时秒数，后面跟数字
```

常用示例：

端口扫描。端口扫描经常被系统管理员和黑客用来发现在一些机器上开放的端口，帮助他们识别系统中的漏洞
```
nc -z -v 192.168.3.179 22
可以运行在TCP或者UDP模式，默认是TCP，-u参数调整为udp.
z 参数告诉netcat使用0 IO,连接成功后立即关闭连接， 不进行数据交换
```

```
nc -l 9999 # 开启一个本地 9999 的 TCP 协议端口，由客户端主动
发起连接，一旦连接必须由服务端发起关闭
nc -vw 2 192.168.3.178 9999 # 通过 nc 去访问 192.168.3.178 主机的 9999 端
口，确认是否存活；可不加参数

这样客户端跟服务端就可以直接通讯了，可以在命令行上互发消息
```

#### tcpdump抓包

大部分 Linux 发行版都内置了 Tcpdump 工具。如果没有，也可以直接使用对应的包管理器进行安装（如：`$ sudo apt-get install tcpdump` 和 `$ sudo yum install tcpdump`）

tcpdump命令很复杂，这里只是列几个例子，如何分析抓到的包的数据，这里不做介绍。

**常用场景**

1、获取`10.1.85.21`和`10.1.85.19`之间的通信，使用命令注意转义符号。

```css
[root@centos daocoder]# tcpdump host 10.1.85.21 and \( 10.1.85.19\) -i ens5f0 -nn -c 10
```

2、获取从`10.1.85.21`发来的包。

```css
[root@centos daocoder]# tcpdump src host 10.1.85.21 -c 10 -i ens5f1
```

3、监听tcp（udp）端口。

```csharp
[root@centos daocoder]# tcpdump tcp port 22 -c 10
```

4、获取主机`10.1.85.21`和除`10.1.85.19`之外所有主机的通信。

```css
[root@centos daocoder]# tcpdump ip host 10.1.85.21 and ! 10.1.85.19 -c 10 -i any
```

5、获取从`10.1.85.19`且端口主机到`10.1.85.21`主机的通信。

```css
[root@centos daocoder]# tcpdump src host 10.1.85.19 and src port 48565 and dst host 10.1.85.21 and dst port 5090 -i any -c 10 -nn
```

#### tc模拟网络延迟，丢包、抖动

tc命令很复杂，记住下面的几个常见用法，主要用来模拟网络异常时，我们开发的服务能否正常运行

##### 模拟网络延迟

- 添加一个固定延迟到本地网卡 eth0

```
// delay: 100ms
tc qdisc add dev eth0 root netem delay 100ms
```

- 给延迟加上上下 10ms 的波动

```
tc qdisc change dev eth0 root netem delay 100ms 10ms
```

- 加一个 25% 的相关概率

> 相关性，是这当前的延迟会和上一次数据包的延迟有关，短时间里相邻报文的延迟应该是近似的而不是完全随机的。这个值是个百分比，如果为 100%，就退化到固定延迟的情况；如果是 0% 则退化到随机延迟的情况， Pn = 25% Pn-1 + 75% Random

```
tc qdisc change dev eth0 root netem delay 100ms 10ms 25%
```

- 让波动变成正态分布的

```
tc qdisc change dev eth0 root netem delay 100ms 20ms distribution normal
```

##### 模拟网络丢包

- 设置丢包率为 1%

```
tc qdisc change dev eth0 root netem loss 1%
```

- 添加一个相关性参数

> 这个参数表示当前丢包的概率与上一条数据包丢包概率有 25% 的相关性 Pn = 25% Pn-1 + 75% Random

```
tc qdisc change dev eth0 root netem loss 1% 25%
```

##### 模拟数据包重复

- 1% 的数据包重复

```
tc qdisc change dev eth0 root netem duplicate 1%
```

##### 模拟包损坏

- 2% 的包损坏

```
tc qdisc add dev eth0 root netem corrupt 2%
```

##### 模拟包乱序

网络传输并不能保证顺序，传输层 TCP 会对报文进行重组保证顺序，所以报文乱序对应用的影响比上面的几种问题要小。

报文乱序可前面的参数不太一样，因为上面的报文问题都是独立的，针对单个报文做操作就行，而乱序则牵涉到多个报文的重组。模拟报乱序一定会用到延迟（因为模拟乱序的本质就是把一些包延迟发送），netem 有两种方法可以做。

1. 第一种是固定的每隔一定数量的报文就乱序一次：
2. 每 5th (10th, 15th…) 的包延迟 10ms

```
tc qdisc change dev eth0 root netem gap 5 delay 10ms
```

1. 第二种方法使用概率来选择乱序，相对来说更偏向实际情况一些
2. 25 % 的立刻发送（50% 的相关性），其余的延迟 10ms

```
tc qdisc change dev eth0 root netem delay 10ms reorder 25% 50%
```

1. 只使用 delay 也可能造成数据包的乱序

```
 tc qdisc change dev eth0 root netem delay 100ms 75ms
```

> 比如 first one random 100ms, second random 25ms，这样就是会造成第一个包先于第二个包发送

##### 限制出口带宽

限制 100mbit

```
tc qdisc add dev ens33 root tbf rate 100mbit latency 50ms burst 1600
```

限制延迟 100ms, 流量 100mbit

```
tc qdisc add dev eth0 root handle 1:0 netem delay 100ms
tc qdisc add dev eth0 parent 1:1 handle 10: tbf rate 256kbit buffer 1600 limit 3000
```

##### 删除规则，

```
# tc qdisc del dev eth0 root
# tc -s qdisc ls dev eth0
```

#### netstat ss

后面在补充

### 软件安装

Linux软件安装方式：

1.apt，rpm，yum，dnf；

2.源代码安装，根据具体情况执行对应命令安装，一般会提供安装方法，这里不做说明

#### apt 安装

**1、APT方式（apt是Ubuntu的软件包管理工具）**

（1）普通安装：apt-get install softname1 softname2 …;

（2）修复安装：apt-get -f install softname1 softname2... ;(-f 是用来修复损坏的依赖关系)

（3）重新安装：apt-get --reinstall install softname1 softname2...;

**2、Ubuntu中软件包的卸载方法**

1、APT方式

（1）移除式卸载：apt-get remove softname1 softname2 …;（移除软件包，当包尾部有+时，意为安装）

（2）清除式卸载 ：apt-get --purge remove softname1 softname2...;(同时清除配置)

（3） 清除式卸载：apt-get purge sofname1 softname2...;(同上，也清除配置文件)

常用参数：

-y 自动回应是否安装软件包，在一些自动化脚本中经常用到

-s 模拟安装

-q 静默安装

-f 修复损坏的依赖关系

-- reinstall 重新安装已经安装但可能存在问题的软件包

-- install-suggests 同时安装APT给出的建议安装的软件包

apt-get包含的常用工具：

update  从软件源镜像服务器上下载/更新用于更新本地软件源的软件包

upgrade  升级本地可更新的全部软件包，但存在依赖问题时将不会升级，通常会在更新之前执行一次update

purge  与remove相同，但会完全移除软件包，包含其配置文件

autoremove  移除之前被其他软件包依赖，但现在不再被使用的软件包

**常用的APT命令:**

**apt-cache search package 搜索包**

apt-cache show package 获取包的相关信息，如说明、大小、版本等

apt-cache depends package 了解使用依赖

apt-cache rdepends package 查看该包被哪些包依赖

sudo apt-get install package 安装包，apt会自动下载安装，若有依赖性软件包，apt也会自动下载安装

**sudo apt-get install package --reinstall 重新安装包**

sudo apt-get -f install 修复安装"-f = --fix-missing"

**sudo apt-get remove package 删除包，如有依赖的软件包，则会一并移除**

**sudo apt-get remove package --purge 删除包，包括删除配置文件等**

**sudo apt-get update 更新源**（该指令是用来取得记录在 /etc/apt/sources.list 内的远端服务器的软件包清单，在使用apt-get dist-upgrade指令升级软件前，一定要记得先用此指令将软件包清单更新）

sudo apt-get upgrade 更新已安装的软件包

sudo apt-get dist-upgrade 升级系统

sudo apt-get dselect-upgrade 使用 dselect 升级

sudo apt-get build-dep package 安装相关的编译环境

apt-get source package 下载该包的源代码

sudo apt-get clean && sudo apt-get autoclean 清理无用的包（当使用 apt-get install 指令安装后，下载的软件包会放置在 /var/cache/apt/archives，使用apt-get clean指令可将其删除）

sudo apt-get check 检查是否有损坏的依赖

#### rpm（Red Hat Linux软件包管理工具）

rpm原本是Red Hat Linux发行版专门用来管理Linux各项套件的程序

语法：

rpm [选项] [软件包]

1.安装

rpm -ivh example.rpm 安装 example.rpm 包并在安装过程中显示正在安装的文件信息及安装进度；

2.查询

RPM 查询操作

命令：

rpm -q …

附加查询命令：

a 查询所有已经安装的包以下两个附加命令用于查询安装包的信息；

i 显示安装包的信息；

l 显示安装包中的所有文件被安装到哪些目录下；

s 显示安装版中的所有文件状态及被安装到哪些目录下；以下两个附加命令用于指定需要查询的是安装包还是已安装后的文件；

p 查询的是安装包的信息；

f 查询的是已安装的某文件信息；

例，rpm -qa | grep tomcat4 查看 tomcat4 是否被安装；

3.RPM 卸载操作

命令：

rpm -e 需要卸载的安装包

在卸载之前，通常需要使用rpm -q …命令查出需要卸载的安装包名称。

举例如下：

rpm -e tomcat4 卸载 tomcat4 软件包；

4.RPM 升级操作

命令：

rpm -U 需要升级的包

举例如下：

rpm -Uvh example.rpm 升级 example.rpm 软件包；

#### yum(Yellow dog Updater, Modified）

老的Linux系统用yum，当前新的Linux都用dnf命令，但是都也兼容yum


1.安装：

yum install package1 #安装指定的安装包package1

2.更新和升级：

yum update #全部更新 

yum update package1 #更新指定程序包package1 

yum check-update #检查可更新的程序 

yum upgrade package1 #升级指定程序包package1 

yum groupupdate group1 #升级程序组group1

3.卸载：

yum remove package1

4.清除缓存:

yum clean packages #清除缓存目录下的软件包 

yum clean headers #清除缓存目录下的 headers 

yum clean oldheaders #清除缓存目录下旧的 headers

5.查找和显示:

yum info package1 #显示安装包信息package1 

yum list #显示所有已经安装和可以安装的程序包 

yum list package1 #显示指定程序包安装情况package1 

yum groupinfo group1 #显示程序组group1信息yum search string 根据关键字string查找安装包



## 总结



上面介绍的都是一些比较基础的命令，在平时工作中应该足够，每个命令其实都有很多其他用法，这里只是列出了比较常规的用法，其他的命令或用法需要根据具体的工作内容决定，在平时工作总要注意总结。



网络相关章节的命令，一般只需要记住scp、rsync这两个拷贝命令即可，其他的在随着工作的深入在深入了解。