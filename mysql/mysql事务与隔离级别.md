### 文章目录

- - - [数据库事务](https://tonydong.blog.csdn.net/article/details/104291974#_7)

    - [事务控制](https://tonydong.blog.csdn.net/article/details/104291974#_29)

    - [隔离级别](https://tonydong.blog.csdn.net/article/details/104291974#_163)

    - - [可重复读](https://tonydong.blog.csdn.net/article/details/104291974#_196)
      - [读已提交](https://tonydong.blog.csdn.net/article/details/104291974#_358)
      - [更新丢失](https://tonydong.blog.csdn.net/article/details/104291974#_504)



[上一篇](https://tonydong.blog.csdn.net/article/details/104244235)我们介绍了 InnoDB 存储引擎的内存和磁盘体系结构。

MySQL 中的 InnoDB 存储引擎具有事务安全性，能够保证多个用户并发访问相同数据时的数据一致性和完整性；同时也不会由于系统崩溃或硬件故障导致数据的破坏。

### 数据库事务

在数据库中，**事务**（Transaction）是指一组相关的 SQL 语句操作，它们在业务逻辑上是一个原子单元。

> 📝原子在化学反应中不可分割，但是在物理状态中，原子由原子核和绕核运动的电子组成。不过在数据库领域中，我们仍然借助原子表示一个不可分割的整体操作。

最常见的数据库事务就是银行账户之间的转账操作。比如从 A 账户转出 200 元到 B 账户，其中就包含了多个操作：

1. 查询 A 账户的余额是否足够；
2. 从 A 账户减去 200 元；
3. 往 B 账户增加 200 元；
4. 记录本次转账流水。

显然，数据库必须保证所有的操作要么全部成功，要么全部失败。如果从 A 账户减去 1000 元成功执行，但是没有往 B 账户增加 1000 元，意味着客户将会损失 1000 元。用数据库中的术语来说，这种情况导致了数据库的不一致性。

通过以上案例，我们知道数据库事务需要满足一些特性。SQL 标准定义了事务的四种属性：**ACID**。

- **A**tomic，**原子性**。一个事务包含的所有 SQL 语句要么全部成功，要么全部失败。例如，某个事务需要更新 100 条记录，但是在更新到一半时系统出现故障；数据库必须保证能够回滚已经修改过的数据，就像没有执行过该事务一样。
- **C**onsistency，**一致性**。事务开始之前，数据库位于一致性的状态；事务完成之后，数据库仍然位于一致性的状态。例如，银行转账事务中，如果一个账户扣款成功但另一个账户加钱失败，那么就会出现数据不一致（此时需要回滚已经执行的扣款操作）。另外，数据库还必须保证满足完整性约束，比如账户扣款之后不能出现余额为负数（可以在余额字段上添加检查约束）。
- **I**solation，**隔离性**。隔离性与并发事务有关，隔离意味着一个事务对数据的修改在其完成之前对其他事务是不可见。例如，账户 A 向账户 B 转账的过程中，账户 B 查询的余额应该是转账之前的数目；如果多人同时向账户 B 转账，结果也应该保持一致性，就像依次转账的结果一样。MySQL 支持 SQL 标准中的 4 种事务隔离级别。
- **D**urability，**持久性**。已经提交的事务必须永久生效，即使发生断电、系统崩溃等故障，数据库都不会丢失数据。对于 InnoDB 而言，使用的是重做日志（REDO）实现事务的持久性。

### 事务控制

我们先来了解一下 InnoDB 中的事务控制。使用以下语句创建一个简单的示例表：

```sql
mysql> CREATE TABLE accounts(id INT AUTO_INCREMENT PRIMARY KEY, user_name varchar(50), balance numeric(10,4));
Query OK, 0 rows affected (0.75 sec)

mysql> ALTER TABLE accounts ADD CONSTRAINT bal_check CHECK(balance >= 0);
Query OK, 0 rows affected (2.95 sec)
Records: 0  Duplicates: 0  Warnings: 0
123456
```

accounts 用于存储账户信息，检查约束 bal_check 用于确保余额不会出现负数。

MySQL 中与事务管理相关的语句包括：

- **autocommit** 系统变量，控制是否自动提交，默认为 on；
- **BEGIN** 或者 **START TRANSACTION** 语句，用于开始一个新的事务；
- **COMMIT**，提交一个事务；
- **ROLLBACK**，撤销一个事务；
- **SAVEPOINT**，事务保存点，用于撤销一部分事务；

由于 MySQL 默认启用了自动提交（autocommit），任何数据操作都会自动提交：

```sql
show variables like 'autocommit';
Variable_name|Value|
-------------|-----|
autocommit   |ON   |
1234
```

我们插入一条数据到 accounts 表中：

```sql
mysql> INSERT INTO accounts(user_name, balance) VALUES ('UserA', 100);
Query OK, 1 row affected (0.11 sec)
12
```

由于打开了自动提交，MySQL 会自动执行一个`COMMIT`语句。此时表中已经存在了一个账户 UserA。

我们也可以明确进行事务的控制，例如：

```sql
mysql> BEGIN;
Query OK, 0 rows affected (0.00 sec)

mysql> INSERT INTO accounts(user_name, balance) VALUES ('UserB', 0);
Query OK, 1 row affected (0.01 sec)

mysql> COMMIT;
Query OK, 0 rows affected (0.06 sec)

mysql> select * from accounts;
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 100.0000 |
|  2 | UserB     |   0.0000 |
+----+-----------+----------+
2 rows in set (0.00 sec)
1234567891011121314151617
```

执行提交操作之后，accounts 表中存在两个账户。我们也可以使用`ROLLBACK`撤销事务的修改：

```sql
mysql> BEGIN;
Query OK, 0 rows affected (0.00 sec)

mysql> INSERT INTO accounts(user_name, balance) VALUES ('UserC', 0);
Query OK, 1 row affected (0.02 sec)

mysql> select * from accounts;
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 100.0000 |
|  2 | UserB     |   0.0000 |
|  3 | UserC     |   0.0000 |
+----+-----------+----------+
3 rows in set (0.00 sec)

mysql> ROLLBACK;
Query OK, 0 rows affected (0.04 sec)

mysql> select * from accounts;
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 100.0000 |
|  2 | UserB     |   0.0000 |
+----+-----------+----------+
2 rows in set (0.00 sec)
123456789101112131415161718192021222324252627
```

虽然在执行插入语句之后，可以查看到账户 UserC，但是并没有持久化；在执行`ROLLBACK`语句之后，accounts 表中仍然只存在两个账户。

最后我们演示一下保存点的使用：

```sql
mysql> BEGIN;
Query OK, 0 rows affected (0.00 sec)

mysql> INSERT INTO accounts(user_name, balance) VALUES ('UserC', 0);
Query OK, 1 row affected (0.07 sec)

mysql> SAVEPOINT sv1;
Query OK, 0 rows affected (0.00 sec)

mysql> INSERT INTO accounts(user_name, balance) VALUES ('UserD', 0);
Query OK, 1 row affected (0.00 sec)

mysql> ROLLBACK TO sv1;
Query OK, 0 rows affected (0.00 sec)

mysql> COMMIT;
Query OK, 0 rows affected (0.05 sec)

mysql> select * from accounts;
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 100.0000 |
|  2 | UserB     |   0.0000 |
|  4 | UserC     |   0.0000 |
+----+-----------+----------+
3 rows in set (0.00 sec)
123456789101112131415161718192021222324252627
```

在上面的示例中，首先插入了账户 UserC，设置了事务保存点 sv1；然后插入账户 UserD，撤销保存点 sv1 之后的修改；然后提交 sv1 之前的修改；最终 accounts 表中保存了三个账户。

> 📝除了使用以上语句控制事务的提交之外，MySQL 中还存在许多会引起自动隐式提交的语句，例如 DDL 语句；更多内容可以参考[官方文档](https://dev.mysql.com/doc/refman/8.0/en/implicit-commit.html)。

在企业应用中，通常需要支持多用户并发访问；并且保证多个用户并发访问相同的数据时，不会造成数据的不一致性和不完整性。数据库通常使用事务的隔离（加锁）来解决并发问题。

### 隔离级别

数据库的并发意味着多个用户同时访问相同的数据，例如 A 和 C 同时给 B 转账。数据库的并发访问可能带来以下问题：

- **脏读**（Dirty Read）。当一个事务允许读取另一个事务修改但未提交的数据时，就可能发生脏读。例如，B 的初始余额为 0；A 向 B 转账 100 元但没有提交；此时 B 能够看到 A 转过来的 100 元，并且成功取款 100 元；然后 A 取消了转账；银行损失了 100 元。很显然，银行不会允许这种事情发生。
- **不可重复读**（Nonrepeatable Read）。一个事务读取某一记录后，该数据被另一个事务修改提交，再次读取该记录时结果发生了改变。例如，B 查询初始余额为 0；此时 A 向 B 转账 100 元并且提交；B 再次查询发现余额变成了 100 元，以为天上掉馅饼了。
- **幻读**（Phantom Read）。一个事务第一次读取数据后，另一个事务增加或者删除了某些数据，再次读取时结果的数量发生了变化。幻读和非重复读有点类似，都是由于其他事务修改数据导致的结果变化。
- **更新丢失**（Lost Update）。第一类：当两个事务更新相同的数据时，如果第一个事务被提交，然后第二个事务被撤销；那么第一个事务的更新也会被撤销。第二类：当两个事务同时读取某一记录，然后分别进行修改提交；就会造成先提交的事务的修改丢失。例如卖票系统，两个操作人员都查询到了某张票；然后分别提交更新，结果一张票卖出去了两次。

为了解决并发可能导致的各种问题，SQL 标准定义了 4 种不同的事务隔离级别（从低到高）：

| 隔离级别                         | 脏读 | 不可重复读 | 幻读 | 更新丢失 |
| -------------------------------- | ---- | ---------- | ---- | -------- |
| **读未提交（Read Uncommitted）** | 可能 | 可能       | 可能 | 第二类   |
| **读已提交（Read Committed）**   | –    | 可能       | 可能 | 第二类   |
| **可重复读（Repeatable Read）**  | –    | –          | 可能 | –        |
| **序列化（Serializable）**       | –    | –          | –    | –        |

事务的隔离级别越高，越能保证数据的一致性；但同时会对并发带来更大的影响。大多数数据库系统使用读已提交（Read Committed）作为默认的隔离级别，MySQL InnoDB 存储引擎默认使用可重复读（Repeatable Read）隔离级别。

```sql
SELECT @@transaction_isolation;
@@transaction_isolation|
-----------------------|
REPEATABLE-READ        |
1234
```

另外，我们还需要注意的是 MySQL InnoDB 的实现与 SQL 标准的一些差异；它在可重复读隔离级别解决了幻读问题，但是存在第二类更新丢失问题 。

> 📝数据库事务隔离与并发控制的实现方式通常有两种：锁（Lock）与多版本并发控制（MVCC），具体可以参考[这篇文章](https://tonydong.blog.csdn.net/article/details/103324323)。

接下来我们演示一下 MySQL 中不同隔离级别的效果。

#### 可重复读

首先，在会话 1 中开始一个事务，并查询 UserA 的余额：

```sql
-- 会话 1
mysql> begin;
Query OK, 0 rows affected (0.00 sec)
mysql> select * from accounts where user_name= 'UserA';
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 100.0000 |
+----+-----------+----------+
1 row in set (0.00 sec)
12345678910
```

账户 UserA 的余额为 100 元。然后打开一个新的会话 2，开始一个事务并修改 UserA 的余额：

```sql
-- 会话 2
mysql> begin;
Query OK, 0 rows affected (0.00 sec)
mysql> update accounts
    -> set balance = balance + 100
    -> where user_name = 'UserA';
Query OK, 1 row affected (0.06 sec)
Rows matched: 1  Changed: 1  Warnings: 0

mysql> select * from accounts where user_name= 'UserA';
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 200.0000 |
+----+-----------+----------+
1 row in set (0.00 sec)
12345678910111213141516
```

会话 2 中显示 UserA 的余额已经被修改为 200。此时再查询会话 1：

```sql
-- 会话 1
mysql> select * from accounts where user_name= 'UserA';
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 100.0000 |
+----+-----------+----------+
1 row in set (0.00 sec)
12345678
```

结果仍然是 200，没有出现脏读。如果我们在会话 1 中并发修改 UserA 的数据：

```sql
mysql> update accounts
    -> set balance = 99
    -> where user_name='UserA';
ERROR 1205 (HY000): Lock wait timeout exceeded; try restarting transaction
1234
```

由于会话 2 已经锁定了该记录（未提交），会话 1 必须等待，直到等待超时。

> 📝无论哪种隔离级别，如果一个事务已经修改某个数据，则另一个事务不允许同时修改该数据，写操作一定是按照顺序执行。

我们回到会话 2 中，提交事务：

```sql
-- 会话 2
mysql> commit;
Query OK, 0 rows affected (0.10 sec)
123
```

然后再次查询会话 1：

```sql
-- 会话 1
mysql> select * from accounts where user_name= 'UserA';
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 100.0000 |
+----+-----------+----------+
1 row in set (0.00 sec)
12345678
```

虽然会话 2 已经提交了事务，会话 1 的查询结果仍然是 100，意味着可重复读取（Repeatable Read）。在会话 1 中提交事务：

```sql
-- 会话 1
mysql> commit;
Query OK, 0 rows affected (0.00 sec)
mysql> select * from accounts where user_name= 'UserA';
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 200.0000 |
+----+-----------+----------+
1 row in set (0.00 sec)
12345678910
```

此时，账户 UserA 的余额为 200，会话 1 读取到了会话 2 提交的修改。

我们再看一个幻读的示例，首先在会话 1 中执行以下命令：

```sql
-- 会话 1
mysql> begin;
Query OK, 0 rows affected (0.00 sec)

mysql> INSERT INTO accounts(user_name, balance) VALUES ('UserD', 0);
Query OK, 1 row affected (0.03 sec)
123456
```

会话 1 插入了一条数据但未提交。然后在会话 2 中执行查询：

```sql
-- 会话 2
mysql> begin;
Query OK, 0 rows affected (0.00 sec)

mysql> select * from accounts;
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 200.0000 |
|  2 | UserB     |   0.0000 |
|  3 | UserC     |   0.0000 |
+----+-----------+----------+
3 rows in set (0.00 sec)
12345678910111213
```

查询结果只有 3 条记录。此时回到会话 1 中提交事务：

```sql
-- 会话 1
mysql> commit;
Query OK, 0 rows affected (0.08 sec)
123
```

然后再次查询会话 2：

```sql
mysql> select * from accounts;
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 200.0000 |
|  2 | UserB     |   0.0000 |
|  3 | UserC     |   0.0000 |
+----+-----------+----------+
3 rows in set (0.00 sec)
123456789
```

仍然看不到 UserD，也就是说，**MySQL 可重复读（Repeatable Read）隔离级别避免了幻读**。

最后我们在会话 1 中撤销事务：

```sql
mysql> rollback;
Query OK, 0 rows affected (0.07 sec)
12
```

接下来我们看看在其他数据库中默认的读已提交（Read Committed）隔离级别。

#### 读已提交

MySQL 提供了以下命令，用于修改当前会话的隔离级别：

```sql
SET TRANSACTION ISOLATION LEVEL {
     REPEATABLE READ
   | READ COMMITTED
   | READ UNCOMMITTED
   | SERIALIZABLE
};
123456
```

我们将会话 1 的隔离级别设置为 READ COMMITTED，然后开始一个事务查询 UserA 的余额：

```sql
-- 会话 1
mysql> SET TRANSACTION ISOLATION LEVEL READ COMMITTED;
Query OK, 0 rows affected (0.00 sec)
mysql> begin;
Query OK, 0 rows affected (0.00 sec)
mysql> select * from accounts where user_name= 'UserA';
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 200.0000 |
+----+-----------+----------+
1 row in set (0.00 sec)
123456789101112
```

账户 UserA 的余额为 200 元。然后在会话 2 中修改 UserA 的余额：

```sql
-- 会话 2
mysql> begin;
Query OK, 0 rows affected (0.00 sec)
mysql> update accounts
    -> set balance = balance + 100
    -> where user_name = 'UserA';
Query OK, 1 row affected (0.06 sec)
Rows matched: 1  Changed: 1  Warnings: 0

mysql> select * from accounts where user_name = 'UserA';
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 300.0000 |
+----+-----------+----------+
1 row in set (0.00 sec)
12345678910111213141516
```

会话 2 中显示 UserA 的余额已经被修改为 300。此时再查询会话 1：

```sql
-- 会话 1
mysql> select * from accounts where user_name= 'UserA';
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 200.0000 |
+----+-----------+----------+
1 row in set (0.00 sec)
12345678
```

查询结果为 200，没有出现脏读。我们回到会话 2 中，提交事务：

```sql
-- 会话 2
mysql> commit;
Query OK, 0 rows affected (0.12 sec)
123
```

然后再次查询会话 1：

```sql
-- 会话 1
mysql> select * from accounts where user_name= 'UserA';
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 300.0000 |
+----+-----------+----------+
1 row in set (0.00 sec)
12345678
```

由于会话 2 已经提交了事务，会话 1 的查询结果发生了变化，意味着不可重复读取（Nonrepeatable Read）。在会话 1 中提交事务：

```sql
-- 会话 1
mysql> commit;
Query OK, 0 rows affected (0.00 sec)
123
```

我们同样看一个幻读的示例，首先在会话 2 中执行以下命令：

```sql
-- 会话 2
mysql> begin;
Query OK, 0 rows affected (0.00 sec)

mysql> INSERT INTO accounts(user_name, balance) VALUES ('UserD', 0);
Query OK, 1 row affected (0.03 sec)
123456
```

会话 2 插入了一条数据但未提交。然后在会话 1 中执行查询：

```sql
-- 会话 1
mysql> begin;
Query OK, 0 rows affected (0.00 sec)

mysql> select * from accounts;
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 200.0000 |
|  2 | UserB     |   0.0000 |
|  3 | UserC     |   0.0000 |
+----+-----------+----------+
3 rows in set (0.00 sec)
12345678910111213
```

查询结果只有 3 条记录。此时回到会话 2 中提交事务：

```sql
-- 会话 2
mysql> commit;
Query OK, 0 rows affected (0.08 sec)
123
```

然后再次查询会话 1：

```sql
mysql> select * from accounts;
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 300.0000 |
|  2 | UserB     |   0.0000 |
|  3 | UserC     |   0.0000 |
|  6 | UserD     |   0.0000 |
+----+-----------+----------+
4 rows in set (0.00 sec)
12345678910
```

会话 1 看到了 UserD，出现了幻读。

#### 更新丢失

现代数据库系统已经解决了第一类更新丢失问题，但是可能存在第二类更新丢失。对于 MySQL 而言，除非是设置可序列化的隔离级别，都可能存在第二类更新丢失问题。

以下试验仍然以默认的可重复读隔离级别为例，会话 1 查询数据：

```sql
-- 会话1
mysql> begin;
Query OK, 0 rows affected (0.00 sec)

mysql> select * from accounts where user_name='UserA';
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 300.0000 |
+----+-----------+----------+
1 row in set (0.00 sec)
1234567891011
```

同时，会话 2 也查询了该记录：

```sql
-- 会话1
mysql> begin;
Query OK, 0 rows affected (0.00 sec)

mysql> select * from accounts where user_name='UserA';
+----+-----------+----------+
| id | user_name | balance  |
+----+-----------+----------+
|  1 | UserA     | 300.0000 |
+----+-----------+----------+
1 row in set (0.00 sec)
1234567891011
```

然后会话 1 将 UserA 的余额增加 100：

```sql
mysql> update accounts
    -> set balance=balance+100
    -> where user_name='UserA';
Query OK, 1 row affected (0.03 sec)
Rows matched: 1  Changed: 1  Warnings: 0
12345
```

同时会话 2 将 UserA 的余额也增加 100：

```sql
-- 会话 2
mysql> update accounts
    -> set balance=balance+100
    -> where user_name='UserA';
Query OK, 1 row affected (4.04 sec)
Rows matched: 1  Changed: 1  Warnings: 0
123456
```

此时，会话 2 需要等待会话 1 提交：

```sql
-- 会话 1
mysql> commit;
Query OK, 0 rows affected (0.08 sec)
123
```

紧接着会话 2 也提交：

```sql
-- 会话 2
mysql> commit;
Query OK, 0 rows affected (0.07 sec)
123
```

以上场景相当于 UserA 给自己存了 100 元，但发现账户增加了 200 元；虽然这种情况有可能是允许的，UserA 可以查询交易记录发现有人给他转了账，并不是银行系统出错。但是对于卖票等交易系统，如果一张票被卖出两次就会出现问题了。

解决更新丢失的方法通常有两种：乐观锁（Optimistic Locking，类似于 MVCC）和悲观锁（Pessimistic Locking，`select for update`），具体参考[这篇文章](https://tonydong.blog.csdn.net/article/details/103324323)。

通常来说，隔离级别越高越能保证数据的一致性和完整性，但是支持的并发也会越低。一般来说，我们可以使用数据库默认的隔离级别；它可以保证不会出现脏读、不可重复读以及幻读问题，并且具有较好的并发性能。对于特殊的应用场景，可以通过应用程序主动加锁的方式进行处理。