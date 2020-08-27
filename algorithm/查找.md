## 问题的引出

我们有时会遇到这样的问题: 给你一些数，组成一个序列，如`[1 4 2 3]`,有两种操做:

- 操作一:给序列的第`i`个数加上`X` (`X`可以为负数)
- 操作二:询问序列中最大的数是什么? 格式`query(start, end)`，表示区间`[start, end]`内，最大值是多少？

我们很容易得到一个朴素的算法: 将这些数存入一个数组, 由于要询问最大值, 我们只需要遍历这个区间`[start, end]`即可，找出最大值。
 对于改变一个数, 我们就在这个数上加上`X`，比如`A[i] = A[i] + X`。
 这个算法的缺点是什么呢？`Query`询问最大值复杂度`O(N)`, 修改复杂度为`O(1)`，在有`Q`个`query`的情况下这样总的复杂度为`O(QN)`, 那么对于查询来说这样的复杂度是不可以接受的。

## Segment Tree 线段树

一颗线段树的构造就是根据区间的性质的来构造的, 如下是一棵区间`[0, 3]`的线段树，每个`[start, end]`都是一个二叉树中的节点。

![img](https:////upload-images.jianshu.io/upload_images/3415798-73454346218c96f3.png?imageMogr2/auto-orient/strip|imageView2/2/w/408/format/webp)

[0, 3]的线段树



区间划分大概就是上述的区间划分。可以看出每次都将区间的长度一分为二,数列长度为`n`,所以线段树的高度是`log(n)`,这是很多高效操作的基础。
 上述的区间存储的只是区间的左右边界。我们可以将区间的最大值加入进来,也就是树中的`Node`需要存储`left`，`right`左右子节点外，还需要存储`start`, `end`, `val`区间的范围和区间内表示的值。
 **可以储存不同的值，例如区间内的最大值，最小值，区间的求和等等。**

![img](https:////upload-images.jianshu.io/upload_images/3415798-d4bdd5d19ad20012.png?imageMogr2/auto-orient/strip|imageView2/2/w/476/format/webp)

将区间的最大值加入进来

因为每次将区间的长度一分为二,所有创造的节点个数，即底层有`n`个节点，那么倒数第二次约`n/2`个节点，倒数第三次约`n/4`个节点，依次类推：



```undefined
n + 1/2 * n + 1/4 * n + 1/8 * n + ...
=   (1 + 1/2 + 1/4 + 1/8 + ...) * n
=   2n
```

**所以构造线段树的时间复杂度和空间复杂度都为`O(n)`。**

### 线段树中的Node如何定义

二叉树的节点区间定义，`[start, end]`代表节点的区间范围，`max` 是节点在`[start, end]`区间上的最大值 `left` , `right` 是当前节点区间划分之后的左右节点区间：



```csharp
// 节点区间定义
// [start, end] 代表节点的区间范围
// max 是节点在(start,end)区间上的最大值
// left , right 是当前节点区间划分之后的左右节点区间
public class SegmentTreeNode {
    public int start, end, max;
    public SegmentTreeNode left, right;
    public SegmentTreeNode(int start, int end, int max) {
        this.start = start;
        this.end = end;
        this.max = max
        this.left = this.right = null;
    }
}
```

### 线段树区间最大值维护

给定一个区间，我们要维护线段树中存在的区间中最大的值。这将有利于我们高效的查询任何区间的最大值。给出`A`数组，基于`A`数组构建一棵维护最大值的线段树，我们可以在`O(logN)`的复杂度内查询任意区间的最大值：

比如原数组 `A = [1, 4, 2, 3]`

![img](https:////upload-images.jianshu.io/upload_images/3415798-a60cc510252e8504.png?imageMogr2/auto-orient/strip|imageView2/2/w/468/format/webp)

原数组 A = [1, 4, 2, 3]





```swift
// 构造的代码及注释
public SegmentTreeNode build(int[] A) {
    // write your code here
    return buildhelper(0, A.length - 1, A);
}
public SegmentTreeNode buildhelper(int left, int right, int[] A){
    if(left > right){
        return null;
    }
    SegmentTreeNode root = new SegmentTreeNode(left, right, A[left]); // 根据节点区间的左边界的序列值为节点赋初值
    if(left == right){
        return root; // 如果左边界和右边界相等,节点左边界的序列值就是线段树节点的接节点值
    }
    int mid = (left + right) / 2; // 划分当前区间的左右区间
    root.left = buildhelper(left, mid, A);
    root.right = buildhelper(mid + 1, right, A);
    root.max = Math.max(root.left.max, root.right.max); // 根据节点区间的左右区间的节点值得到当前节点的节点值
    return root;
}
```

**举一反三：**
 如果需要区间的最小值:
 `root.min = Math.min(root.left.min, root.right.min);`
 如果需要区间的和:
 `root.sum = root.left.sum + root.right.sum;`

### 线段树的单点更新

![img](https:////upload-images.jianshu.io/upload_images/3415798-749d148b79448983.png?imageMogr2/auto-orient/strip|imageView2/2/w/500/format/webp)

原线段树

更新序列中的一个节点，如何把这种变化体现到线段树中去，例如，将序列中的第4个点`A[3]`更新为5, 要变动3个区间中的值,分别为`[3,3],[2,3],[0,3]`

![img](https:////upload-images.jianshu.io/upload_images/3415798-ab62dd476a070e5b.png?imageMogr2/auto-orient/strip|imageView2/2/w/466/format/webp)

A[3] 更新为5

**提问：为什么需要更新这三个区间？：因为只有这三个在线段树中的区间，覆盖了3这个点。**

更新所以需要从叶子节点一路走到根节点, 去更新线段树上的值。因为线段树的高度为`log(n)`,所以更新序列中一个节点的复杂度为`log(n)`。



```swift
// 单点更新的代码及注释
public void modify(SegmentTreeNode root, int index, int value) {
    // write your code here
    if(root.start == root.end && root.start == index) { // 找到被改动的叶子节点
        root.max = value; // 改变value值
        return ;
    }
    int mid = (root.start + root.end) / 2; // 将当前节点区间分割为2个区间的分割线
    if(index <= mid){ // 如果index在当前节点的左边
        modify(root.left, index, value); // 递归操作
        root.max = Math.max(root.right.max, root.left.max); // 可能对当前节点的影响
    }
    else {            // 如果index在当前节点的右边
        modify(root.right, index, value); // 递归操作
        root.max = Math.max(root.left.max, root.right.max); // 可能对当前节点的影响
    }
    return ;
}
```

### 线段树的区间查询

线段树的区间查询操作就是将当前区间分解为较小的子区间,然后由子区间的最大值就可以快速得到需要查询区间的最大值。

![img](https:////upload-images.jianshu.io/upload_images/3415798-4e832396c378db55.png?imageMogr2/auto-orient/strip|imageView2/2/w/464/format/webp)

线段树

```
query(1,3) = max(query(1,1),query(2,3)) = max(4,3) = 4
```

任意长度的线段，最多被拆分成logn条线段树上存在的线段，所以查询的时间复杂度为`O(log(n))`。



```cpp
// 区间查询的代码及注释
public int query(TreeNode root, int start, int end) {
    if (start <= root.start && root.end <= end) {
        // 如果查询区间在当前节点的区间之内,直接输出结果
        return root.max;
    }
    int mid = (root.start + root.end) / 2; // 将当前节点区间分割为左右2个区间的分割线
    int ans = Integer.MIN_VALUE; // 给结果赋初值
    if (mid >= start) {   // 如果查询区间和左边节点区间有交集,则寻找查询区间在左边区间上的最大值
        ans = Math.max(ans, query(root.left, start, end));
    }
    if (mid + 1 <= end) { // 如果查询区间和右边节点区间有交集,则寻找查询区间在右边区间上的最大值
        ans = Math.max(ans, query(root.right, start, end));
    }
    return ans; // 返回查询结果
}
```

## Segment Tree 线段树 的数组实现

虽然 Segment Tree 线段树逻辑上是一棵二叉树，**但是实际存储时可以通过数组来实现，**通过父子节点的下标的数值关系可以访问父节点的子节点。

例如通过如下代码将一个数组转换为一个 Segment Tree：



```cpp
    // a segment tree
    private int[] seg;
    
    // the element count
    private int n;
    
    public NumArray(int[] nums) {
        n = nums.length;
 
        if(n > 0) {
            seg = new int[4 * n];
        
            build(nums, 0, n - 1, 0);
        }
    }
    
    // build segment tree, set the value of seg[idx]
    public void build(int[] nums, int start, int end, int idx) {
        if(start == end) {
            seg[idx] = nums[start];
        }
        else {
            int mid = (start + end) / 2;
            
            // build left tree
            build(nums, start, mid, 2 * idx + 1);
            
            // build right tree
            build(nums, mid + 1, end, 2 * idx + 2);
            
            seg[idx] = seg[2 * idx + 1] + seg[2 * idx + 2];
        }
    }
```

**更新元素：**更新元素需要更新两个地方，一是原数组对应的下标的值，另外一个是包含了这个元素的 Segment Tree 中的节点的值。具体也是通过递归实现：



```cpp
    public void update(int i, int val) {
        update(0, n - 1, i, val, 0);
    }
    
    public void update(int start, int end, int i, int val, int idx) {
        // leaf node. update element.
        if (start == end) {
            seg[idx] = val;
            return;
        }
        
        int mid = (start + end) / 2;
        
        // left tree
        if (i <= mid) {
            update(start, mid, i, val, 2 * idx + 1);
        }
        // right tree
        else {
            update(mid + 1, end, i, val, 2 * idx + 2);
        }
        
        seg[idx] = seg[2 * idx + 1] + seg[2 * idx + 2];
    }
```

**区间求和：**区间求和也是通过递归实现，关键在于根据当前节点表示的范围以及需要求的区间的范围的关系进行求和：



```cpp
    public int sumRange(int i, int j) {
        return sumRange(0, n - 1, i, j, 0);
    }
    
    public int sumRange(int start, int end, int i, int j, int idx) {
        // segment completely outside range, represents a null node
        if (start > j || end < i)
            return 0;
        
        // segment completely inside range
        if (i <= start && j >= end)
            return seg[idx];
        
        int mid = (start + end) / 2;
        
        return sumRange(start, mid, i, j, 2 * idx + 1) +
            sumRange(mid + 1, end, i, j, 2 * idx + 2);
    }
```

## Segment Tree 线段树 使用示例

LeetCode 题目：

**LintCode题目：**[207. 区间求和 II](http://www.lintcode.com/zh-cn/problem/interval-sum-ii/)
 在类的构造函数中给一个整数数组, 实现两个方法 `query(start, end)` 和 `modify(index, value)`:

- 对于 `query(start, end)`, 返回数组中下标 `start` 到 `end` 的 和。
- 对于 `modify(index, value)`, 修改数组中下标为 `index` 上的数为 `value`。

> 样例
>  给定数组 A = [1,2,7,8,5].
>  query(0, 2), 返回 10.
>  modify(0, 4), 将 A[0] 修改为 4.
>  query(0, 1), 返回 6.
>  modify(2, 1), 将 A[2] 修改为 1.
>  query(2, 4), 返回 14.



```csharp
    /* you may need to use some attributes here */
    
     class SegmentTreeNode {
        public int start, end;
        public int sum;
        public SegmentTreeNode left, right;
        public SegmentTreeNode(int start, int end, int sum) {
              this.start = start;
              this.end = end;
              this.sum = sum;
              this.left = this.right = null;
        }
    }
    SegmentTreeNode root;
    public SegmentTreeNode build(int start, int end, int[] A) {
        // write your code here
        if(start > end) {  // check core case
            return null;
        }
        
        SegmentTreeNode root = new SegmentTreeNode(start, end, 0);
        
        if(start != end) {
            int mid = (start + end) / 2;
            root.left = build(start, mid, A);
            root.right = build(mid+1, end, A);
            
            root.sum = root.left.sum + root.right.sum;
        } else {
            root.sum =  A[start];
            
        }
        return root;
    }
    public int querySegmentTree(SegmentTreeNode root, int start, int end) {
        // write your code here
        if(start == root.start && root.end == end) { // 相等 
            return root.sum;
        }
        
        
        int mid = (root.start + root.end)/2;
        int leftsum = 0, rightsum = 0;
        // 左子区
        if(start <= mid) {
            if( mid < end) { // 分裂 
                leftsum =  querySegmentTree(root.left, start, mid);
            } else { // 包含 
                leftsum = querySegmentTree(root.left, start, end);
            }
        }
        // 右子区
        if(mid < end) { // 分裂 3
            if(start <= mid) {
                rightsum = querySegmentTree(root.right, mid+1, end);
            } else { //  包含 
                rightsum = querySegmentTree(root.right, start, end);
            } 
        }  
        // else 就是不相交
        return leftsum + rightsum;
    }
    public void modifySegmentTree(SegmentTreeNode root, int index, int value) {
        // write your code here
        if(root.start == index && root.end == index) { // 查找到
            root.sum = value;
            return;
        }
        
        // 查询
        int mid = (root.start + root.end) / 2;
        if(root.start <= index && index <=mid) {
            modifySegmentTree(root.left, index, value);
        }
        
        if(mid < index && index <= root.end) {
            modifySegmentTree(root.right, index, value);
        }
        //更新
        root.sum = root.left.sum + root.right.sum;
    }
    /**
     * @param A: An integer array
     */
    public Solution(int[] A) {
        // write your code here
        root = build(0, A.length-1, A);
    }
    
    /**
     * @param start, end: Indices
     * @return: The sum from start to end
     */
    public long query(int start, int end) {
        // write your code here
        return querySegmentTree(root, start ,end);
    }
    
    /**
     * @param index, value: modify A[index] to value.
     */
    public void modify(int index, int value) {
        // write your code here
        modifySegmentTree(root, index, value);
    }
}
```

**LeetCode题目：**[315. Count of Smaller Numbers After Self](https://leetcode.com/problems/count-of-smaller-numbers-after-self/description/)
 You are given an integer array nums and you have to return a new counts array. The counts array has the property where `counts[i]` is the number of smaller elements to the right of `nums[i]`.

> Example:
>  Given nums = [5, 2, 6, 1]
>  To the right of 5 there are 2 smaller elements (2 and 1).
>  To the right of 2 there is only 1 smaller element (1).
>  To the right of 6 there is 1 smaller element (1).
>  To the right of 1 there is 0 smaller element.
>  Return the array [2, 1, 1, 0].



```dart
class Solution {
    class TreeNode {
        int val;
        int count; // **从左往右**，小于该val的数字的数目
        int duplicates; // 该val对应的数字的重复个数
        
        TreeNode left;
        TreeNode right;
        
        TreeNode(int val) {
            this.val = val;
        }
    }
    
    public List<Integer> countSmaller(int[] nums) {
        Integer[] result = new Integer[nums.length];
        TreeNode root = null;
        
        // 从右往左以此插入数字
        for(int i = nums.length - 1; i >= 0; i--) {
            root = insert(nums[i], root, result, i, 0);
        }
        
        return Arrays.asList(result);
    }
    
    // 将值为num的节点插入
    public TreeNode insert(int num, TreeNode node, Integer[] result, int idx, int preSum) {
        // 创建节点
        if(node == null) {
            node = new TreeNode(num);
            node.count = 0;
            node.duplicates = 1;
            
            result[idx] = preSum;
        }
        // 更新节点
        else if (node.val == num) {
            node.duplicates++;
            
            result[idx] = preSum + node.count;
        }
        // 在左子树添加
        else if (node.val > num) {
            node.count++;
            
            node.left = insert(num, node.left, result, idx, preSum);
        }
        // 在右子树添加
        else {
            node.right = insert(num, node.right, result, idx, preSum + node.duplicates + node.count);
        }
        
        return node;
    }
    
}
```

## 总结 - 线段树问题解决的框架

- 如果问题带有区间操作，或者可以转化成区间操作，可以尝试往线段树方向考虑
  - 从面试官给的题目中抽象问题，将问题转化成一列区间操作，注意这步很关键
- 当我们分析出问题是一些列区间操作的时候
  - 对区间的一个点的值进行修改
  - 对区间的一段值进行统一的修改
  - 询问区间的和
  - 询问区间的最大值、最小值
     我们都可以采用线段树来解决这个问题
- 套用我们前面讲到的经典步骤和写法，即可在面试中完美的解决这些题目！

**什么情况下，无法使用线段树？**
 如果我们删除或者增加区间中的元素，那么区间的大小将发生变化，此时是无法使用线段树解决这种问题的。



作者：专职跑龙套
链接：https://www.jianshu.com/p/91f2c503e62f
来源：简书
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。