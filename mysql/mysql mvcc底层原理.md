先理解下mysql事务隔离级别

## mysql事物隔离级别

mysql有四种隔离级别,默认的隔离级别是 RR，可重复读
- Read Uncommitted（读取未提交）
    在该隔离级别，所有事务都可以看到其他未提交事务的执行结果。本隔离级别很少用于实际应用，因为它的性能也不比其他级别好多少。读取未提交的数据，也被称之为脏读（Dirty Read）。


- Read Committed（读取已提交）
    这是大多数数据库系统的默认隔离级别（但不是MySQL默认的）。它满足了隔离的简单定义：一个事务只能看见已经提交事务所做的改变。这种隔离级别 也支持所谓的不可重复读（Nonrepeatable Read），因为同一事务的其他实例在该实例处理其间可能会有新的commit，所以同一select可能返回不同结果。
- Repeatable Read（可重读）
    这是MySQL的默认事务隔离级别，它确保同一事务的多个实例在并发读取数据时，会看到同样的数据行。不过理论上，这会导致另一个棘手的问题：幻读 （Phantom Read）。简单的说，幻读指当用户读取某一范围的数据行时，另一个事务又在该范围内插入了新行，当用户再读取该范围的数据行时，会发现有新的“幻影” 行。InnoDB和Falcon存储引擎通过多版本并发控制（MVCC，Multiversion Concurrency Control）机制解决了该问题（**这里纠正下，应该是mysql会对“当前读语句”读取的记录行加记录锁（record lock）和间隙锁（gap lock），禁止其他事务在间隙间插入记录行，来防止幻读。及行级锁+MVCC来实现的，下面会详细介绍**）。
- Serializable（可串行化） 
    这是最高的隔离级别，它通过强制事务排序，使之不可能相互冲突，从而解决幻读问题。简言之，它是在每个读的数据行上加上共享锁。在这个级别，可能导致大量的超时现象和锁竞争。

## 查看MYSQL的默认隔离级别

```
SELECT @@global.tx_isolation, @@tx_isolation;
```

<img src="..\assert\da88f24577457cb1ed0c8b2f050e6335b4f.jpg" alt="img" style="zoom:80%;" />

@@global.tx_isolation ： 全局事务隔离级别 @@tx_isolation ： 当前回话的隔离级别

## 更改MYSQL隔离级别

```
# 设全局事务为 RU 读未提交的隔离级别
SET @@global.tx_isolation = 0;
SET @@global.tx_isolation = 'READ-UNCOMMITTED';

# 设置全局事务为 RC 读已提交的事务隔离级别
SET @@global.tx_isolation = 1;
SET @@global.tx_isolation = 'READ-COMMITTED';

# 设置全局事务为 RR 可重复读的隔离级别
SET @@global.tx_isolation = 2;
SET @@global.tx_isolation = 'REPEATABLE-READ';

# 设置全局事务为 串行化 隔离级别
SET @@global.tx_isolation = 3;
SET @@global.tx_isolation = 'SERIALIZABLE';
```

## 脏读场景

这种场景只会在 RU(Read uncommitted 未提交读) 隔离级别中出现。

这种场景需要先将 事务的隔离级别设置为 RU(Read uncommitted 未提交读)。

这个也用 不可重复读的场景例子，跟不可重复读的区别在，B事务执行插入之后，不需要提交事务，A事务就能查询到数据了。

## 不可重复读场景

这种场景会在 RC(Read committed 已提交读) 隔离级别中出现。

先将全局事务隔离级别设置为 RC级别。 场景：先开启事务A，然后开启事务B，事务B插入数据，之后事务A去查询，这时候RR隔离级别应该查询不到这种条数据，RC隔离级别能查询到这条数据。不可重复读场景能查询到这条数据。 事务A:

```
begin ;  -- 第1步
select * from test ;  第 2步
select * from test ;  第3步
commit;  第4步
```

先开启A事务，执行第1步，第2步，查询到的结果如下：

<img src="..\assert\37890789e97e79024d01a9c719fa7a7b460.jpg" alt="img" style="zoom:80%;" />

接着开启事务B，执行 第1步、第2步

```
begin;  -- 第1步
insert into test (id ,value) value(7,7);  -- 第2步
commit; -- 第3步
```

这时候再执行事务A的第3步，还是看不到7这条数据。接着执行B的第3步，这时候再执行A的第4步，这时候就能看到7这条数据了。

## 幻读场景

这种场景会在 RR(REPEATABLE READ 可重复读) 隔离级别中出现。

MYSQL默认的就是这种隔离级别。我们先就这种场景举例如下（这里id建立了唯一索引）：

业务需求：表中如果不存在id=6的数据，则插入

事务A执行步骤：

```
begin; -- 第1步

select * from test where id = 6  ; --  第2步

-- 业务判断 发现 不存在

insert into test (id ,value) value (6,6);  -- 第3步 

commit ;  -- 第4步
```

事务B执行步骤：

```
begin; -- 第1步

insert into test (id ,value) value (6,6);  -- 第2步 

commit ;  -- 第3步
```

假设A事务执行到了第1步，B事务执行到了第2步，接着执行A事务，这时候A事务第2步发现没有id为6的数据，但是第3步却执行不了，一直挂在锁等待状态。

<img src="..\assert\a217e31cf821118b1b5173a8af0a721ef27.jpg" alt="img" style="zoom:80%;" />

这种场景，A事务是看不到id=6的数据，但是A事务也不能执行插入操作，有魔幻的感觉，这就是幻读。

## 1、MVCC概念

​    ***\*多版本控制（Multiversion Concurrency Control）\****: 指的是一种提高并发的技术。最早的数据库系统，只有读读之间可以并发，读写，写读，写写都要阻塞。引入多版本之后，**只有写写之间相互阻塞**，其他三种操作都可以并行，这样大幅度提高了InnoDB的并发度。在内部实现中，InnoDB通过undo log保存每条数据的多个版本，并且能够找回数据历史版本提供给用户读，每个事务读到的数据版本可能是不一样的。在同一个事务中，用户只能看到该事务创建快照之前已经提交的修改和该事务本身做的修改。

​    MVCC在 Read Committed 和 Repeatable Read两个隔离级别下工作。

​    MySQL的InnoDB存储引擎默认事务隔离级别是RR(可重复读)，是通过 "行级锁+MVCC"一起实现的，正常读的时候不加锁，写的时候加锁。而 MVCC 的实现依赖：**隐藏字段、Read View、Undo log**。

### 1.1、隐藏字段

​    InnoDB存储引擎在每行数据的后面添加了三个隐藏字段：

​    \1. **DB_TRX_ID**(6字节)：表示最近一次对本记录行作修改（insert | update）的事务ID。至于delete操作，InnoDB认为是一个update操作，不过会更新一个另外的删除位，将行表示为deleted。并非真正删除。

​    \2. **DB_ROLL_PTR**(7字节)：回滚指针，指向当前记录行的undo log信息

​    \3. **DB_ROW_ID**(6字节)：随着新行插入而单调递增的行ID。理解：当表没有主键或唯一非空索引时，innodb就会使用这个行ID自动产生聚簇索引。如果表有主键或唯一非空索引，聚簇索引就不会包含这个行ID了。**这个DB_ROW_ID跟MVCC关系不大**。

隐藏字段并不是什么创建版本、删除版本。官方文档：[14.3 InnoDB Multi-Versioning](https://dev.mysql.com/doc/refman/5.7/en/innodb-multi-versioning.html)

<img src="..\assert\20200409105342893.png" alt="img" style="zoom:80%;" />.

### 1.2、Read View 结构（重点）

​    其实Read View（读视图），跟快照、snapshot是一个概念。

​    Read View主要是用来做可见性判断的, 里面保存了“对本事务不可见的其他活跃事务”。

   [Read View 结构源码](https://github.com/facebook/mysql-5.6/blob/42a5444d52f264682c7805bf8117dd884095c476/storage/innobase/include/read0read.h#L125)，其中包括几个变量，在网上这些变量的解释各种各样，下面结合源码给出它们正确的解释。
    **① low_limit_id*****\*：\****目前出现过的最大的事务ID+1，即下一个将被分配的事务ID。源码 [350行](https://github.com/facebook/mysql-5.6/blob/42a5444d52f264682c7805bf8117dd884095c476/storage/innobase/read/read0read.cc#L350)：

​      max_trx_id的定义如下，源码 [628行](https://github.com/facebook/mysql-5.6/blob/42a5444d52f264682c7805bf8117dd884095c476/storage/innobase/include/trx0sys.h#L628)，翻译过来就是“还未分配的最小事务ID”，也就是下一个将被分配的事务ID。（low_limit_id 并不是活跃事务列表中最大的事务ID）

​    **②** **up_limit_id*****\*：\****活跃事务列表trx_ids中最小的事务ID，如果trx_ids为空，则up_limit_id 为 low_limit_id。源码 [358行](https://github.com/facebook/mysql-5.6/blob/42a5444d52f264682c7805bf8117dd884095c476/storage/innobase/read/read0read.cc#L358)：

​      因为trx_ids中的活跃事务号是逆序的，所以最后一个为最小活跃事务ID。（up_limit_id 并不是已提交的最大事务ID+1，后面的 [例子2](https://blog.csdn.net/Waves___/article/details/105295060#例子2) 会证明这是错误的）

​    ***\*③ trx_ids：\****Read View创建时其他未提交的活跃事务ID列表。意思就是创建Read View时，将当前未提交事务ID记录下来，后续即使它们修改了记录行的值，对于当前事务也是不可见的。
​      注意：Read View中trx_ids的活跃事务，不包括当前事务自己和已提交的事务（正在内存中），源码 [295行](https://github.com/facebook/mysql-5.6/blob/42a5444d52f264682c7805bf8117dd884095c476/storage/innobase/read/read0read.cc#L295)：

​    ***\*④ creator_trx_id：\****当前创建事务的ID，是一个递增的编号，源码 [345行](https://github.com/facebook/mysql-5.6/blob/42a5444d52f264682c7805bf8117dd884095c476/storage/innobase/read/read0read.cc#L345) 。（这个编号并不是DB_ROW_ID）

### 1.3、Undo log    

​    Undo log中存储的是老版本数据，当一个事务需要读取记录行时，如果当前记录行不可见，可以顺着undo log链找到满足其可见性条件的记录行版本。
​    大多数对数据的变更操作包括 insert/update/delete，在InnoDB里，undo log分为如下两类：
​    ①insert undo log : 事务对insert新记录时产生的undo log, 只在事务回滚时需要, 并且在事务提交后就可以立即丢弃。
​    ②update undo log : 事务对记录进行delete和update操作时产生的undo log，不仅在事务回滚时需要，快照读也需要，只有当数据库所使用的快照中不涉及该日志记录，对应的回滚日志才会被purge线程删除。

> ​    **Purge线程**：为了实现InnoDB的MVCC机制，更新或者删除操作都只是设置一下旧记录的deleted_bit，并不真正将旧记录删除。
> ​    为了节省磁盘空间，InnoDB有专门的purge线程来清理deleted_bit为true的记录。purge线程自己也维护了一个read view，如果某个记录的deleted_bit为true，并且DB_TRX_ID相对于purge线程的read view可见，那么这条记录一定是可以被安全清除的。

## 2、记录行修改的具体流程

​    假设有一条记录行如下，字段有Name和Honor，值分别为"curry"和"mvp"，最新修改这条记录的事务ID为1。<img src="..\assert\20200701205716343.png" alt="img" style="zoom:80%;" />

​    （1）现在事务A（事务ID为2）对该记录的Honor做出了修改，将Honor改为"fmvp"：

​         ①事务A先对该行加排它锁
​         ②然后把该行数据拷贝到undo log中，作为旧版本
​         ③拷贝完毕后，修改该行的Honor为"fmvp"，并且修改DB_TRX_ID为2（事务A的ID）, 回滚指针指向拷贝到undo log的旧版本。（然后还会将修改后的最新数据写入redo log）
​         ④事务提交，释放排他锁

<img src="..\assert\image-20200916212803428.png" alt="image-20200916212803428" style="zoom:80%;" />

​    （2） 接着事务B（事务ID为3）修改同一个记录行，将Name修改为"iguodala"：

​         ①事务B先对该行加排它锁
​         ②然后把该行数据拷贝到undo log中，作为旧版本
​         ③拷贝完毕后，修改该行Name为"iguodala"，并且修改DB_TRX_ID为3（事务B的ID）, 回滚指针指向拷贝到undo log最新的旧版本。
​         ④事务提交，释放排他锁

<img src="..\assert\image-20200916212853898.png" alt="image-20200916212853898" style="zoom:80%;" />

​    从上面可以看出，不同事务或者相同事务的对同一记录行的修改，会使该记录行的undo log成为一条链表，undo log的链首就是最新的旧记录，链尾就是最早的旧记录。

## 3、可见性比较算法

​    在innodb中，创建一个新事务后，执行第一个select语句的时候，innodb会创建一个快照（read view），快照中会保存系统当前不应该被本事务看到的其他活跃事务id列表（即trx_ids）。当用户在这个事务中要读取某个记录行的时候，innodb会将该记录行的DB_TRX_ID与该Read View中的一些变量进行比较，判断是否满足可见性条件。

​    假设当前事务要读取某一个记录行，该记录行的**DB_TRX_ID**（即最新修改该行的事务ID）为trx_id，Read View的活跃事务列表**trx_ids**中最早的事务ID为up_limit_id，将在生成这个Read Vew时系统出现过的最大的事务ID+1记为low_limit_id（即还未分配的事务ID）。

具体的比较算法如下（可以照着后面的 [例子](https://blog.csdn.net/Waves___/article/details/105295060#5、例子（帮助理解）) ，看这段）:

​    \1. 如果 trx_id < up_limit_id, 那么表明“最新修改该行的事务”在“当前事务”创建快照之前就提交了，所以该记录行的值对当前事务是可见的。跳到步骤5。

​    \2. 如果 trx_id >= low_limit_id, 那么表明“最新修改该行的事务”在“当前事务”创建快照之后才修改该行，所以该记录行的值对当前事务不可见。跳到步骤4。

​    \3. 如果 up_limit_id <= trx_id < low_limit_id, 表明“最新修改该行的事务”在“当前事务”创建快照的时候可能处于“活动状态”或者“已提交状态”；所以就要对活跃事务列表trx_ids进行查找（源码中是用的二分查找，因为是有序的）：

​      (1) 如果在活跃事务列表trx_ids中能找到 id 为 trx_id 的事务，表明①在“当前事务”创建快照前，“该记录行的值”被“id为trx_id的事务”修改了，但没有提交；或者②在“当前事务”创建快照后，“该记录行的值”被“id为trx_id的事务”修改了（不管有无提交）；这些情况下，这个记录行的值对当前事务都是不可见的，跳到步骤4；

​      (2)在活跃事务列表中找不到，则表明“id为trx_id的事务”在修改“该记录行的值”后，在“当前事务”创建快照前就已经提交了，所以记录行对当前事务可见，跳到步骤5。

​    \4. 在该记录行的 DB_ROLL_PTR 指针所指向的undo log回滚段中，取出最新的的旧事务号DB_TRX_ID, 将它赋给trx_id，然后跳到步骤1重新开始判断。

​    \5. 将该可见行的值返回。

比较算法源码 [84行](https://github.com/facebook/mysql-5.6/blob/42a5444d52f264682c7805bf8117dd884095c476/storage/innobase/include/read0read.ic#L84)，也可看下图，有注释，图代码来自 [link](http://www.leviathan.vip/2019/03/20/InnoDB的事务分析-MVCC/)：

## 4、当前读和快照读

​    **快照读(snapshot read)**：普通的 select 语句(不包括 select ... lock in share mode, select ... for update)

​    **当前读(current read)** ：select ... lock in share mode，select ... for update，insert，update，delete 语句（这些语句获取的是数据库中的***\*最新数据\****，官方文档：[14.7.2.4 Locking Reads](https://dev.mysql.com/doc/refman/5.7/en/innodb-locking-reads.html) ）

​    只靠 MVCC 实现RR隔离级别，可以保证可重复读，还能防止部分幻读，但并不是完全防止。

​    比如事务A开始后，执行普通select语句，创建了快照；之后事务B执行insert语句；然后事务A再执行普通select语句，得到的还是之前B没有insert过的数据，因为这时候A读的数据是符合快照可见性条件的数据。这就防止了**部分**幻读，此时事务A是**快照读**。

​    但是，如果事务A执行的不是普通select语句，而是select ... for update等语句，这时候，事务A是**当前读**，每次语句执行的时候都是获取的最新数据。也就是说，**在只有MVCC时**，A先执行 select ... where nid between 1 and 10 … for update；然后事务B再执行 insert … nid = 5 …；然后 A 再执行 select ... where nid between 1 and 10 … for update，就会发现，多了一条B insert进去的记录。这就产生幻读了，所以单独靠MVCC并不能完全防止幻读。

​    因此，InnoDB在实现RR隔离级别时，不仅使用了MVCC，还会对“当前读语句”读取的记录行加记录锁（record lock）和间隙锁（gap lock），禁止其他事务在间隙间插入记录行，来防止幻读。也就是前文说的"行级锁+MVCC"。

​    如果你对这些锁不是很熟悉，[这是一篇将MySQL 中锁机制讲的很详细的博客](https://tonydong.blog.csdn.net/article/details/103324323) 。

### RR和RC的Read View产生区别：

​    ①在innodb中的**Repeatable Read**级别, 只有事务在begin之后，执行***\*第一条\****select（读操作）时, 才会创建一个快照(read view)，将当前系统中活跃的其他事务记录起来；并且事务之后都是使用的这个快照，不会重新创建，直到事务结束。

​    ②在innodb中的**Read Committed**级别, 事务在begin之后，执行***\*每条\****select（读操作）语句时，快照会被重置，即会重新创建一个快照(read view)。

官方文档：[consistent read](https://dev.mysql.com/doc/refman/5.7/en/glossary.html#glos_consistent_read)，里面所说的consistent read 一致性读，我的理解就是 [快照读](https://blog.csdn.net/Waves___/article/details/105295060#4、当前读和快照读)，也就是普通select语句，它们不会对访问的数据加锁。   只有普通select语句才会创建快照，select ... lock in share mode，select ... for update不会，update、delete、insert语句也不会，因为它们都是 [当前读](https://blog.csdn.net/Waves___/article/details/105295060#4、当前读和快照读)，会对访问的数据加锁。
<img src="..\assert\20200409105504228.png" alt="img" style="zoom:80%;" />

## 5、例子（帮助理解）

| 假设原始数据行： |           |           |             |
| ---------------- | --------- | --------- | ----------- |
| Field            | DB_ROW_ID | DB_TRX_ID | DB_ROLL_PTR |
| 0                | 10        | 10000     | 0x13525342  |

### 例子1

<img src="..\assert\image-20200916213136836.png" alt="image-20200916213136836" style="zoom:80%;" />



### 例子2

（证明“up_limit_id为已提交的最大事务ID + 1”是错误的）

<img src="..\assert\image-20200916213212719.png" alt="image-20200916213212719" style="zoom:80%;" />

### 例子3

（跟例子2一样的情况，不过up_limit_id变为trx_ids中最小的事务ID）：

<img src="..\assert\image-20200916213259249.png" alt="image-20200916213259249" style="zoom:80%;" />



参考:
[MySQL-InnoDB-MVCC多版本并发控制](https://segmentfault.com/a/1190000012650596)
[MySQL数据库事务各隔离级别加锁情况--read committed && MVCC](https://www.imooc.com/article/17290)
[InnoDB存储引擎MVCC的工作原理](https://my.oschina.net/xinxingegeya/blog/505675)
[【MySQL笔记】正确的理解MySQL的MVCC及实现原理](https://blog.csdn.net/SnailMann/article/details/94724197)
[Mysql Innodb中undo-log和MVCC多版本一致性读 的实现](http://blog.sina.com.cn/s/blog_4673e603010111ty.html)
[InnoDB事务分析-MVCC](http://www.leviathan.vip/2019/03/20/InnoDB的事务分析-MVCC/)



下面是图灵学院视频教程关于mvcc解说的两张截图

<img src="..\assert\image-20200916184954169.png" alt="image-20200916184954169" style="zoom: 80%;" />





<img src="..\assert\image-20200916185324918.png" alt="image-20200916185324918" style="zoom:80%;" />