# 压力测试工具Wrk介绍

**概念**

GitHub地址：https://github.com/wg/wrk

### 安装方式

安装环境：Contos7

- 切换到用户级的源码目录

> cd /usr/local/src

- 安装git，因为要从GitHub下载，如果已经安装，忽略此步，其他机器请使用其他命令如apt-get

> yum install git -y

- 从GitHub中clone源码

> git clone https://github.com/wg/wrk.git

- 完成以后，进入wrk目录

> cd wrk

- 然后执行编译操作，编译过程会比较久

> make

- 创建一个软连接，方便其他目录下使用

> ln -s /usr/local/src/wrk/wrk /usr/local/bin

### 基本用法

- 一般用法

> wrk -t1 -c400 —timeout 5s -d 100s —latency ‘http://localhost:8080/index.html‘

- 添加Header和Lua脚本（Lua脚本后面再详细说）

> wrk -t1 -c400 —timeout 5s -d 100s -s ‘/usr/local/src/test.lua’ -H “Authorization: ABCDEFGH123OPQ” —latency ‘http://localhost:8080/index.html‘

**参数说明**

- -c：总的连接数（每个线程处理的连接数=总连接数/线程数）
- -d：测试的持续时间，如2s(2second)，2m(2minute)，2h(hour)
- -t：需要执行的线程总数
- -s：执行Lua脚本，这里写lua脚本的路径和名称，后面会给出案例
- -H：需要添加的头信息，**注意header的语法**，举例，-H “token: abcdef”，说明一下，token，冒号，**空格**，abcdefg（不要忘记空格，否则会报错的）。
- —timeout：超时的时间
- —latency：显示延迟统计信息

**返回结果**

- Latency：响应时间
- Req/Sec：每个线程每秒钟的执行的连接数
- Avg：平均
- Max：最大
- Stdev：标准差
- +/- Stdev： 正负一个标准差占比
- Requests/sec：每秒请求数（也就是QPS），这是一项压力测试的性能指标，通过这个参数可以看出吞吐量
- Latency Distribution，如果命名中添加了—latency就会出现相关信息

### 注意检查连接状态

每次压测完成后，下一次压测前，建议先检查一下之前的连接是否全部断开，然后再重新压测，命令如下：

> ss  -sn

返回结果如下，timewait 0/0代表目前没有等待，说明所有的连接已全部断开

```bash
TTCP:   13 (estab 7, closed 2, orphaned 0, synrecv 0, timewait 1/0), ports 14
Transport Total     IP        IPv6
*          180       -         -        
RAW          0         0         0        
UDP          4         4         0        
TCP          11        11        0        
INET      15        15        0        
FRAG      0         0         0  
```



### 注意事项

- 使用wrk无法看到response，不能确定执行的接口返回的值是否复合预期，所以建议先基准测试，但是wrk不输出response信息，那就想办法输出一下response信息，确定结果符合预期，再压测，具体参见后面的lua脚本
- 一般线程数是系统CPU核数的2~4倍，可以根据业务需要，自行调整

### Lua脚本的相关介绍

为什么要使用这个脚本呢？header太长了，可以写在脚本里，还有一个上面介绍的“一般用法”只能执行get请求，对于post或者put的这种接口很是无奈，所以要扩展一下，废话不多说了，直接开始。

详细的lua脚本可以参考https://github.com/wg/wrk/tree/master/scripts的文件。

**Post请求，如创建test_post.lua文件**

```lua
awrk.method = "POST"
wrk.body = "foo=bar&baz=quux"
wrk.headers["Content-Type"] = "application/x-www-form-urlencoded"
```

使用的时候在wrk通过-s引入这个lua文件即可。

**lua还提供了几个函数**

setup 函数 、init 函数 、delay函数、request函数 、response函数、done函数。

谈一谈如何做基准测试，首先用到的是response函数，这个就是返回响应数据的，然后可以结合done函数使用，done函数的作用就是在所有请求执行完成后调用，一般用于统计结果。

既然要执行基准测试，那就一个线程，一个连接就可以了，不说done函数了，主要说一下我如何利用response函数做基准测试的。

```lua
function response(status, headers, body)
    print(body)
end
```

就把上述的代码写到lua文件中，然后引用lua文件就可以了，无论返回的结果是json或者是html，都会被打印出来，先说一下我遇到的情况，执行了一个线程，一个连接，但是wrk返回结果显示很多个请求（至于原因还没整明白），然后打印出了很多信息，虽然是基准测试，没有什么影响，但是内容太多，排查信息很麻烦，既然输出了多次结果，那就说明执行了多次response函数，这就好办了，让他执行一次response函数就好了。

```lua
a=1
function response(status, headers, body)
   if(a==1)
   then
           a=2
          print(body)
      end
end
```

简单说一下改造后的代码，先定义一个a=1，第一次调用response函数的时候判断，如果a的值为1，那就打印出返回的body，然后把a的状态改变为2，下一次再调用response函数的时候，a的值为2了，就不会再执行print函数，只会打印出一次结果。压测的时候如果lua中不含头信息和post数据，直接不引用即可，如果有这些信息，那就把response函数注释就OK了。

- 语法注意，lua的每一个功能块的结尾都需要**end**结尾，一个方法需要，甚至一个if语句块或者for循环的语句块都需要有end结尾。
- 再谈一谈扩展性，上面的内容介绍的都差不多了，可以把接口lua脚本都准备好，写一个shell脚本，利用变量执行不同并发的压测，利用ss -sn命令令、正则表达式、控制语句等自动执行压测。

**补充说明**：最近压测发现Wrk、ab、JMeter压测的结果有一些差距，这个问题需要再进一步深入研究一下。