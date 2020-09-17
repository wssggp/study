## 索引

索引是帮助mysql高效获取数据的排好序的数据结构

索引数据结构采用的数据结构：

- 二叉树，如果数据插入的顺序是顺序插入的，则二叉树会退化成一个链表

- 红黑树，当数据量大时，树的高度会变的很大，导致查询时io次数变大

- hash表，无法进行范围查找，可以设置表的索引用hash表结构存储

- B+树，mysql采用B+树，可以有效的控制树的高度

  mysql对B+树每个节点的大小做了控制，通过 show global status like 'Innodb_page_size'；来查询，一般为16kb

<img src="..\assert\image-20200916152752711.png" alt="image-20200916152752711" style="zoom:80%;" />



B+树索引的本质是B+树在数据库中的实现。但是B+树索引有一个特点是高扇出性，因此在数据库中，B+树的高度一般在2到3层。也就是说查找某一键值的记录，最多只需要2到3次IO开销。按磁盘每秒100次IO来计算，查询时间只需0.0.2到0.03秒。

数据库中B+树索引分为聚集索引（clustered index）和非聚集索引（secondary index）.这两种索引的共同点是内部都是B+树，高度都是平衡的，叶节点存放着所有数据。不同点是叶节点是否存放着一整行数据。

(1) 聚集索引（innoDB）

Innodb存储引擎表是索引组织表，即表中数据按主键顺序存放。而聚集索引就是按每张表的主键构造一颗B+树。并且叶节点存放整张表的行记录数据。每张表只能有一个聚集索引（一个主键）。

聚集索引的另一个好处是它对于主键的排序查找和范围的速度非常快。叶节点的数据就是我们要找的数据。

（2）高度计算

InnoDB存储引擎默认一个数据页大小为16kb，非叶子节点存放（key，pointer），pointer为6个字节，key为4个字节，即非叶子节点能存放16kb/14左右的key，pointer，而叶子节点如果一条数据大小为100字节，那一个叶子节点大约可存放160条数据。

如果高度为3，则可存放数据为：16kb/14 * 16kb/14 * 160大约1亿多数据。

因此InnoDB存储引擎b+树的高度基本为2-3.

 

###  存储引擎索引实现

存储引擎是用来形容数据库表级别的

myisam和innodb的数据存放方式

<img src="..\assert\image-20200916155622546.png" alt="image-20200916155622546" style="zoom:80%;" />



<img src="..\assert\image-20200916154055219.png" alt="image-20200916154055219" style="zoom:80%;" />

索引跟数据放一起叫聚集索引（innodb），不在一起叫非聚集索引（myisam）

<img src="..\assert\image-20200916155448652.png" alt="image-20200916155448652" style="zoom:80%;" />

**为什么innodb表必须有主键，且推荐使用整型自增：**

如果建表时不用主键，mysql会将没有重复数据的字段作为索引，如果每个列都有重复的，mysql会新增一个隐藏列用来作为表的索引，这显然增加了数据库的负担。另外推荐使用整型自增，一是为了提升查询性能，二是为了提升插入性能。如果不使用自增的，比如用一个字符串，每次查询时需要对比字符串，显然比对比整数要慢的多，不使用自增每次插入数据时会在B+树的内部插入，比直接在后面追加要慢。



**联合索引**

按字段的先后顺序排序，查询时最好使用从左到右进行比较，这样索引才有用。

<img src="..\assert\image-20200916162922077.png" alt="image-20200916162922077" style="zoom:80%;" />







#### 此文转自http://blogold.chinaunix.net/u3/93470/showart_2001536.html 

#### 1．索引作用

  在索引列上，除了上面提到的有序查找之外，数据库利用各种各样的快速定位技术，能够大大提高查询效率。特别是当数据量非常大，查询涉及多个表时，使用索引往往能使查询速度加快成千上万倍。

  例如，有3个未索引的表t1、t2、t3，分别只包含列c1、c2、c3，每个表分别含有1000行数据组成，指为1～1000的数值，查找对应值相等行的查询如下所示。

SELECT c1,c2,c3 FROM t1,t2,t3 WHERE c1=c2 AND c1=c3

  此查询结果应该为1000行，每行包含3个相等的值。在无索引的情况下处理此查询，必须寻找3个表所有的组合，以便得出与WHERE子句相配的那些行。而可能的组合数目为1000×1000×1000（十亿），显然查询将会非常慢。

  如果对每个表进行索引，就能极大地加速查询进程。利用索引的查询处理如下。

（1）从表t1中选择第一行，查看此行所包含的数据。

（2）使用表t2上的索引，直接定位t2中与t1的值匹配的行。类似，利用表t3上的索引，直接定位t3中与来自t1的值匹配的行。

（3）扫描表t1的下一行并重复前面的过程，直到遍历t1中所有的行。

  在此情形下，仍然对表t1执行了一个完全扫描，但能够在表t2和t3上进行索引查找直接取出这些表中的行，比未用索引时要快一百万倍。

  利用索引，MySQL加速了WHERE子句满足条件行的搜索，而在多表连接查询时，在执行连接时加快了与其他表中的行匹配的速度。

### 2. 创建索引

在执行CREATE TABLE语句时可以创建索引，也可以单独用CREATE INDEX或ALTER TABLE来为表增加索引。

#### 1．ALTER TABLE

ALTER TABLE用来创建普通索引、UNIQUE索引或PRIMARY KEY索引。（关于UNIQUE的解释：要强制执行一个或多个列的唯一性值，我们经常使用PRIMARY KEY约束。但是，每个表只有一个主键。 如果要使用多个列或一组具有唯一值的列，则不能使用主键约束。幸运的是，MySQL提供了另一种称为UNIQUE索引的索引，它允许您在一个或多个列中强制实现值的唯一性。 与PRIMARY KEY索引不同，每个表可以有多个UNIQUE索引。）

 

ALTER TABLE table_name ADD INDEX index_name (column_list)

ALTER TABLE table_name ADD UNIQUE (column_list)

ALTER TABLE table_name ADD PRIMARY KEY (column_list)

 

其中table_name是要增加索引的表名，column_list指出对哪些列进行索引，多列时各列之间用逗号分隔。索引名index_name可选，缺省时，MySQL将根据第一个索引列赋一个名称。另外，ALTER TABLE允许在单个语句中更改多个表，因此可以在同时创建多个索引。

#### 2．CREATE INDEX

CREATE INDEX可对表增加普通索引或UNIQUE索引。

 

CREATE INDEX index_name ON table_name (column_list)

CREATE UNIQUE INDEX index_name ON table_name (column_list)

 

table_name、index_name和column_list具有与ALTER TABLE语句中相同的含义，索引名不可选。另外，不能用CREATE INDEX语句创建PRIMARY KEY索引。

#### 3．索引类型

在创建索引时，可以规定索引能否包含重复值。如果不包含，则索引应该创建为PRIMARY KEY或UNIQUE索引。对于单列惟一性索引，这保证单列不包含重复的值。对于多列惟一性索引，保证多个值的组合不重复。

PRIMARY KEY索引和UNIQUE索引非常类似。事实上，PRIMARY KEY索引仅是一个具有名称PRIMARY的UNIQUE索引。这表示一个表只能包含一个PRIMARY KEY，因为一个表中不可能具有两个同名的索引。

下面的SQL语句对students表在sid上添加PRIMARY KEY索引。

 

ALTER TABLE students ADD PRIMARY KEY (sid)

### 4. 删除索引

可利用ALTER TABLE或DROP INDEX语句来删除索引。类似于CREATE INDEX语句，DROP INDEX可以在ALTER TABLE内部作为一条语句处理，语法如下。

 

DROP INDEX index_name ON talbe_name

ALTER TABLE table_name DROP INDEX index_name

ALTER TABLE table_name DROP PRIMARY KEY

 

其中，前两条语句是等价的，删除掉table_name中的索引index_name。

第3条语句只在删除PRIMARY KEY索引时使用，因为一个表只可能有一个PRIMARY KEY索引，因此不需要指定索引名。如果没有创建PRIMARY KEY索引，但表具有一个或多个UNIQUE索引，则MySQL将删除第一个UNIQUE索引。

如果从表中删除了某列，则索引会受到影响。对于多列组合的索引，如果删除其中的某列，则该列也会从索引中删除。如果删除组成索引的所有列，则整个索引将被删除。



**5．查看索引**

mysql> show index from tblname;

mysql> show keys from tblname;

　　· Table

　　表的名称。

　　· Non_unique

　　如果索引不能包括重复词，则为0。如果可以，则为1。

　　· Key_name

　　索引的名称。

　　· Seq_in_index

　　索引中的列序列号，从1开始。

　　· Column_name

　　列名称。

　　· Collation

　　列以什么方式存储在索引中。在MySQL中，有值‘A’（升序）或NULL（无分类）。

　　· Cardinality

　　索引中唯一值的数目的估计值。通过运行ANALYZE TABLE或myisamchk -a可以更新。基数根据被存储为整数的统计数据来计数，所以即使对于小型表，该值也没有必要是精确的。基数越大，当进行联合时，MySQL使用该索引的机会就越大。

　　· Sub_part

　　如果列只是被部分地编入索引，则为被编入索引的字符的数目。如果整列被编入索引，则为NULL。

　　· Packed

　　指示关键字如何被压缩。如果没有被压缩，则为NULL。

　　· Null

　　如果列含有NULL，则含有YES。如果没有，则该列含有NO。

　　· Index_type

　　用过的索引方法（BTREE, FULLTEXT, HASH, RTREE）。

　　· Comment