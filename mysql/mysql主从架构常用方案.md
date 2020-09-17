![image-20200916125534618](..\assert\image-20200916125534618.png)



- 一主一从，并不能提高读写性能，主要是用来进行容灾，当主数据库挂掉后，从数据库可以切换为主，接管工作。
- 一主多从，主要是用来读写分离，一般用两到四个从数据库。一般一个从数据库专门用来查询比较耗时的操作，比如查询历史数据。一个用来作为master节点的备份，当master挂掉了，这个可以提升为master
- 双主，当写入性能太低了，可以用双主模式，两个主数据库写入的数据进行岔开，并互相同步，有点像分库分表。（用的比较少）
- 环形多主，跟双主类似，但是非常危险，一个挂掉，基本玩完。
- 级联同步，由第一级slave节点负责将数据同步到其他slave节点，降低master的负载。



### Atlas读写分离

可以加一个proxy节点，将读数据的请求转发给slave节点，写请求转发给master节点。

![image-20200916131441371](..\assert\image-20200916131441371.png)



### 主从同步配置



1. 主配置文件，可以show master status; 查看状态

   ```
   # 实例id，不能和集群中其他mysql实例相同
   server-id=1
   # bin log 文件前缀
   log-bin=mysql-bin
   # 对应要同步的数据库
   binlog-do-db=mytestdb1
   # 不需要同步的数据库
   binglog-ignore-db=mytestdb2
   binglog-ignore-db=mytestdb3
   ```

2. 从配置文件

   ```
   # 实例id，不能和集群中其他mysql实例相同
   server-id=2
   # bin log 文件前缀,不是必须的，但在级联同步的架构下需要
   log-bin=mysql-bin
   # 对应要同步的数据库
   binlog-do-db=mytestdb4
   # 不需要同步的数据库
   binglog-ignore-db=mytestdb2
   binglog-ignore-db=mytestdb3
   ```

3. 从节点连接主节点

   ```
   ## 在从节点上执行
   # 先停止
   stop slave;
   # 需要进行命令来动态配置
   change master to master_host='master ip or 域名', master_user='root', master_password='123456';
   # 启动
   start slave;
   # 查看状态
   show slave status;
   ```

4. 配置Atlas来实现读写分离

5. 写完之后立马读的场景，出现这种情况怎么处理

   - 强制读主数据库
   - 二次读取
   - 关键业务读写都由主库承担
   - 半同步复制：semi-sync复制，指的就是主库写入binlog日志后，就会将强制此时立即将数据同步到从库，从库将日志写入自己本地的relay log之后，接着会返回一个ack给主库，主库接收到至少一个从库ack之后才会认为写完成。