最近不知道咋回事突然对**协程**的原理挺感兴趣，虽然之前也学习过Go语言，接触过协程，但是其实并不太了解具体的原理，尤其看到知乎上stackless与stackful之间作比较的文章，个人表示真心看不懂，但是带着越是看不懂越要装逼的冲动，上网查了查一些协程的实现，目前感觉比较知名的有：[微信libco](https://links.jianshu.com/go?to=https%3A%2F%2Fgithub.com%2FTencent%2Flibco)、[libgo](https://links.jianshu.com/go?to=https%3A%2F%2Fgithub.com%2Fyyzybb537%2Flibgo)、Go语言作者之一的Russ Cox个人写的[libtask](https://links.jianshu.com/go?to=https%3A%2F%2Fswtch.com%2Flibtask%2F)、[Java协程库](https://links.jianshu.com/go?to=http%3A%2F%2Fwww.paralleluniverse.co%2Fquasar%2F)以及本文要剖析的云风写的[coroutine](https://links.jianshu.com/go?to=https%3A%2F%2Fgithub.com%2Fcloudwu%2Fcoroutine%2F)。不废话，先上图：

![img](https:////upload-images.jianshu.io/upload_images/14447586-5a8a447feba2edfb.png?imageMogr2/auto-orient/strip|imageView2/2/w/1160/format/webp)

coroutine.png


 从上图可以看出coroutine有以下两个特点：
 第一：代码少，仅仅用不到300行代码就实现了协程的核心逻辑，对此只有一个大写的服。
 第二：用纯C语言写的（没有c++）；说实话，个人自认为C语言学得还行，但是C++老是学不会。
 看懂了这个，我感觉就可以看看其他稍微复杂点的实现（加入了poll等 因为目前这个简单的还没法做到IO阻塞时自动yield）。



在看之前，先聊点必备知识点，这是理解协程实现的核心。
 **getcontext、setcontext、makecontext、swapcontext**
 这四个函数是Linux提供的系统函数（忘了跟大家说了 云风这个版本只可以在linux下运行 虽然windows也有fiber这种东西 但是我感觉应该木人感兴趣....），理解了这四个函数，其实这个代码基本80%的难点就搞定了（剩下20%在于要理解函数调用的栈机制）。
 其实context这个词在编程中是个很重要的词，Java Spring框架中有ApplicationContext、OSGI框架中有BundleContext、线程也有自己存活的一个Context等等，其实context就是代表所必需的一个环境信息，有了context就可以决定你的一切走向。简单说，如果你所需要的仅仅是一个变量a=1，那么我们也可以说a=1就是你这个场景下的context。再比如线程切换的时候，总不能让线程切换回来的时候重新开始吧，他曾经走到过的地方，修改过的变量，总得给人家保存到一个地方吧，那么这些需要保存的信息其实也是这个线程能继续运行所依赖的context。只不过**线程**切换的涉及到CPU ring的变化（ring3 用户态 ring0 内核态），此时比较难处理的是每个ring下有自己的特用栈，而**协程**-作为轻量级的线程，不涉及到CPU ring的转变，仅仅在用户态模拟了这种切换（待会儿分析完coroutine就知道啥意思啦），这也是协程会比线程性能高的本质原因所在（Do less）。

getcontext，顾名思义，将当前context保存起来；setcontext，改变当前的context；makecontext这个毕竟牛逼，是制作一个你想要的context；最后swapcontext从一个context切换到另外一个context。先来个简单例子练练手：



```cpp
#include <stdio.h>
#include <ucontext.h>

void main(){
        char isVisit = 0;
        ucontext_t context1, context2; //ucontext_t也是linux提供的类型 用来存储当前context的
        getcontext(&context1);
        printf("I come here\n");

        if(!isVisit){
                isVisit = 1;
                swapcontext(&context2, &context1);
        }

        printf("end of main!");
}
```

猜猜这个执行结果是啥？ 先上答案：

![img](https:////upload-images.jianshu.io/upload_images/14447586-b708f432addef927.png?imageMogr2/auto-orient/strip|imageView2/2/w/453/format/webp)

result.png


 可以看出 I come here执行了两次， why？我们来一句一句的解释：首先声明了两个变量context1和context2，可以把这两个变量理解为容器，专门用来放当前环境的，第三行的时候执行了getcontext(&context1)意思是把当前上下文环境保存到context1这个变量中，然后打印了第一次“I come here”，然后运行到isVisit，因为一开始isVisit=0，所以会进入if语句之中，运行到swapcontext时候，这个函数的含义是把当前上下文环境保存到context2之中，同时跳到context1对应的上下文环境之中。由于context1当时保存的是getcontext调用时的上下文环境中（**运行程序到第几行也是上下文环境中一个元素**），所以swapcontext调用完毕后，又会切换到printf对应的这一行，再次打印出第二次的“I come here”，但是此时之前isVisit已经设置为1了，所以if不会进入了，直接到“end of main”了，程序结束！其实这里还是有个东西没有说清楚，上下文环境到底包含什么？ 我们知道真正执行指令归根结底还是CPU，而CPU在运行程序时会借助于很多寄存器，比如EIP（永远指向下一条要执行程序的地址）、ESP（stack pointer栈指针）、EBP（base pointer基址寄存器）等等，这些其实就是所谓的上下文环境，比如EIP，程序在执行时要靠这个寄存器指示下一步去执行哪条指令，如果你切换到其他**协程（线程也如此）**，回来的时候EIP已经找不到了，那你不悲剧了？那**协程（线程也如此）**岂不是又要重头来过？那这个时候显然需要一块内存去把这个上下文环境保存住，下次回来的时候从内存拿出来接着执行就行，就跟没有切换一个样。针对**协程**来说，上文Linux提供的ucontext_t这个变量就起到这个存储上下文的作用（**线程切换的上下文存储涉及到ring的改变 由操作系统完成的 而协程切换上下文保存需要咱们自己搞定 操作系统根本不参与 不过这样也减少了用户态与内核态的切换**）。结合这段话再回头看看上面的程序，应该会有新的体会。
 Let us move forward，继续再看下面一段程序：





```cpp
#include <stdio.h>
#include <ucontext.h>

#define STACK_SIZE (1024*1024)
char stack[STACK_SIZE];


void test(int a){
  char dummy = 12;
  int    hello     = 9;
  printf("test %d\n",a);
}

void main(){
    char isVisit = 0;
    ucontext_t context1, context2;
    getcontext(&context1);
    printf("I come here\n");
    context1.uc_stack.ss_sp = stack;
    context1.uc_stack.ss_size = STACK_SIZE;
    context1.uc_link = &context2;
    makecontext(&context1, (void (*)(void))test, 1, 10);

    if(!isVisit){
        isVisit = 1;
        swapcontext(&context2, &context1);
        printf("I come to if!\n");
    }

    printf("end of main!\n");
}
```

先上结果：



```bash
I come here
test 10
I come to if!
end of main!
```

这里又加了一个makecontext，这个可是context函数界的老大哥，协程核心全靠他，getcontext只是获取一个当前上下文，而makecontext却能在得到当前上下文的基础上对其进行社会主义改造，比如我们上面这段代码中修改了栈的信息，让test函数执行的时候使用stack[STACK_SIZE]作为其栈空间（**函数调用过程需要栈保存返回地址 函数参数 局部变量**），makecontext第三个参数代表test函数需要几个参数，因为我们的test函数只需要一个int a，所以这里传入1，后面的10就是具体参数值，再比如coroutine源码中有下面一个片段：



```cpp
makecontext(&C->ctx, (void (*)(void)) mainfunc, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));
```

想必大家也知道第三个参数2啥意思了。这里比较重要的是两个点：
 **1. makecontext中第二个参数是一个函数指针，意思是当context1生效时便执行test函数（可以理解为test函数执行的上下文环境就是context1）。**
 **2. 当test函数执行完毕后，会跳到context1.uc_link对应的环境上下文（上面代码设置为context2对应的上下文）。**
 这两点需要用心体会，结合上述代码便可以更好的理解。当然，我们这里用gdb调试一下，帮忙大家理解这个过程（**有意搞了一个dummy和hello变量在test函数 貌似没有用 实则很有用**）。

具体操作见下图：



![img](https:////upload-images.jianshu.io/upload_images/14447586-6131e482470f6708.png?imageMogr2/auto-orient/strip|imageView2/2/w/1158/format/webp)

image.png


 几个注意点，一是注意gcc 别忘了加上-g参数，这是为了生成可以调试的信息，否则gdb无法调试。gdb常用的调试命令可以参照  [gdb tutorial](https://links.jianshu.com/go?to=https%3A%2F%2Fwww.cs.cmu.edu%2F~gilpin%2Ftutorial%2F)
 常用就几个：b 加断点，比如上图中我在test函数加了一个断点，r是让程序跑起来，由于test加了断点，所以在test函数第一行代码停了下来（**显示的这一行代码是马上要执行但是还未执行的**），n是单步运行，但是碰到函数并不会进入，s也是单步运行，但是碰到函数可以step into进入到函数内部调试。p经常使用，打印变量的值，有时还使用p/x 将打印的值以16进制显示。知道这些就够用，接着看重要的细节：



1. p stack+0 打印stack的地址为 0x601080, p stack+1024*1024打印的地址为0x701080,而变量dummy的地址为0x70105f，显然这个地址正合适在stack+0与stack+1024*1024之间，我们知道局部变量都是在栈上分配的，显然test函数已经使用stack作为栈来使用了，这也验证了makecontext制作的上下文环境已经在生效。

2.如果dummy是在栈上分配的，那么下一个变量hello肯定也应该是在栈上分配吧。注意看hello的地址0x701058显然也是在stack范围内的。**注意为啥hello地址比dummy地址少8呢？毕竟先声明的是dummy变量，然后才是hello呀？**
 这里有两个点：一是少，少是因为现在操作系统的栈增长方向都是像地址减少的方向增加的，这个是目前通用的实现，至于为啥无从考究。也就是说，当我们调用push命令压栈一个元素，地址是减少的；二是为啥少8，那是因为目前我使用的linux是64位的，栈一个元素占用8个字节，虽然char只有一个字节，但是为了内存对齐（方便硬件读取 对齐后读取快），还是会在栈上给char分配8个字节（如果之前学过数字电子线路的同学应该知道内存地址编码的问题，如果不对齐，硬件会花费多次才能读出想要的内容，所以现在一般都会提高读取速度进行内存对齐）。
 既然说到栈了，其实栈是一种很有趣的数据结构，太多东西都用到它了，比如JVM对字节码的执行、Vue源码中解析模板生成AST、spring解析@Configure、@Import注解、Tomcat对xml的解析（Digester类）等等，太多栈的使用场景，这可不是一句简单的先进后出可以概括的，里面有很多深刻的内容。而在函数调用这个场景里，操作系统通常会利用栈存储返回地址、函数参数、局部变量等消息（详细分析可以参考 [函数如何使用栈](https://links.jianshu.com/go?to=https%3A%2F%2Fwww.zhihu.com%2Fquestion%2F22444939)），我们只看一个比较重要的图：

![img](https:////upload-images.jianshu.io/upload_images/14447586-1fcb61def3284750.png?imageMogr2/auto-orient/strip|imageView2/2/w/922/format/webp)

栈



我在原图基础上加上高低地址的说明，比如main函数里面调用func_A,func_A调用func_B,此时栈的结构就如上图所示，我们可以发现栈里面有很多重要的东西，返回地址（要不然执行完函数你根本不知道返回到哪儿 有了这个pop一下就拿到了返回地址）、局部变量（Java里面非逃逸对象也是直接在栈上分配哟 减轻GC的压力）等，图上还有个**栈帧**的概念，这个其实是软件层面的界定，比如main用到的栈的范围就是main栈帧，其他函数类似，有了这个东西可以对某个函数能够使用的栈空间进行一定的界定。这不是重点所在，重点就是要知道
 **1.栈是往地址小的方向增长的(但这不是说栈只往地址小的方向走，而是说用到的时候往小的地方走，函数调用完了，这个函数对应的栈帧就没用了，此时把栈指针加上一定的size即可，这时栈指针就往大的方向走啦)**
 **2.局部变量是在栈上分配的**

ok 要看懂coroutine源码所需要的知识都说完了，上源码吧，先看看调度器以及协程（调度器负责协程切换的上下文保护工作）的结构定义：



```cpp
coroutine.c:
struct schedule {
    char stack[STACK_SIZE]; //栈空间
    ucontext_t main; //存储主上下文
    int nco; //存储目前开了多少个协程
    int cap;//容量 如果不够了 会动态扩容 下面有实现
    int running;//目前调度器正在运行着的协程的id
    struct coroutine **co;//指向一个指针数组其中数组每个元素又指向一个协程的信息
    //（协程信息看下面这个结构体）
};

struct coroutine {
    coroutine_func func; //协程要执行的函数体 
    void *ud;//
    ucontext_t ctx;//存储当前协程所对应的上下文信息
    struct schedule * sch;//指向调度器 见上面的结构体 每个协程总得知道是谁在调度自己吧
    ptrdiff_t cap;//自己的栈的容量
    ptrdiff_t size;//自己的栈的真实使用size
    int status;//目前协程的状态 源码中有下面四种状态
    char *stack;//当前协程所使用的栈的指针
};


coroutine.h:
//协程的所有可能状态
#define COROUTINE_DEAD 0
#define COROUTINE_READY 1
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3
```

说一些细节：
 1  schedule结构体中的栈信息是实打实的一个数组，但是coroutine里的栈却是栈指针，为啥这样呢？这就是coroutine的奥妙之处，每个协程都使用调度器里面的空间作为其栈信息 ，然后调度到其他协程时，再为此协程分配内存，将调度器的栈的信息拷贝自己的的stack指针下保存起来，下次再调度自己时，再把stack指针的内容原样拷贝到调度器的stack数组里面，达到圆润切换的目的。

1. 可以看到struct coroutine里面有个ptrdiff_t的类型，这是个Linux系统提供的类型，意思是存储指针做减法后的数据类型，因为这两个字段是通过两个指针做减法算出来的，后面可以明白这里的精巧用心。

接着让我们看看初始化调度器的代码:



```rust
coroutine.c:
#define DEFAULT_COROUTINE 16
struct schedule * 
coroutine_open(void) {
    struct schedule *S = malloc(sizeof(*S));
    S->nco = 0;
    S->cap = DEFAULT_COROUTINE;
    S->running = -1;
    S->co = malloc(sizeof(struct coroutine *) * S->cap);
    memset(S->co, 0, sizeof(struct coroutine *) * S->cap);
    return S;
}
```

这里代码逻辑比较简单，利用malloc为调度器分配空间，然后赋上初值，然后为调度器里面的协程数组先分配默认16个空间，可以看到cap为16，nco为0。这就是一种预分配策略，就像Java里面的ArrayList似的，先初始化一定大小，然后等数据超过擦破大小，再动态扩容。coroutine里面动态扩容的逻辑在:



```rust
coroutine.c:
int 
coroutine_new(struct schedule *S, coroutine_func func, void *ud) {
    struct coroutine *co = _co_new(S, func , ud);
    if (S->nco >= S->cap) {
        int id = S->cap;
        S->co = realloc(S->co, S->cap * 2 * sizeof(struct coroutine *));
        memset(S->co + S->cap , 0 , sizeof(struct coroutine *) * S->cap);
        S->co[S->cap] = co;
        S->cap *= 2;
        ++S->nco;
        return id;
    } else {
        int i;
        for (i=0;i<S->cap;i++) {
            int id = (i+S->nco) % S->cap;
            if (S->co[id] == NULL) {
                S->co[id] = co;
                ++S->nco;
                return id;
            }
        }
    }
    assert(0);
    return -1;
}
```

这个函数的作用是为调度器生成新的协程，可以看到if的判断，当协程的数量要超过容量cap时，用了一个系统函数realloc重新再申请一块更大的内存（申请的大小是之前cap的两倍）。
 可以顺路看一下Java ArrayList的扩容玩法：



```cpp
    private void grow(int minCapacity) {
        // overflow-conscious code
        int oldCapacity = elementData.length;
        int newCapacity = oldCapacity + (oldCapacity >> 1);
        if (newCapacity - minCapacity < 0)
            newCapacity = minCapacity;
        if (newCapacity - MAX_ARRAY_SIZE > 0)
            newCapacity = hugeCapacity(minCapacity);
        // minCapacity is usually close to size, so this is a win:
        elementData = Arrays.copyOf(elementData, newCapacity);
    }
```

新的容量=老的容量+老的容量>>1, 右移相当于除以2，新容量大约是老容量的1.5倍。
 ok，到了coroutine最精彩的一部分啦：



```php
coroutine.c:
//此函数的作用是 让调度器运行协程号为id的协程的运行
void 
coroutine_resume(struct schedule * S, int id) {
    assert(S->running == -1);
    assert(id >=0 && id < S->cap);
    struct coroutine *C = S->co[id];//通过协程id拿到协程结构体信息 这个id是建立协程时返回的
    if (C == NULL)
        return;
    int status = C->status;
    switch(status) {
       /**有两种状态时可以让协程恢复运行 一是刚建立的时候 这时线程状态是READY
 但是由于刚建立 栈信息还没有配置好  所以利用getcontext以及makecontext为其配置上下文**/
    case COROUTINE_READY:
        getcontext(&C->ctx);
        C->ctx.uc_stack.ss_sp = S->stack;
        C->ctx.uc_stack.ss_size = STACK_SIZE;
        C->ctx.uc_link = &S->main;
        S->running = id;
        C->status = COROUTINE_RUNNING;
        uintptr_t ptr = (uintptr_t)S;
        makecontext(&C->ctx, (void (*)(void)) mainfunc, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));
        swapcontext(&S->main, &C->ctx);
        break;
 /**还有一种就是之前运行过了，然后被调度器调走了，运行别的协程了，
再次回来运行此协程时 状态变为SUSPEND 这个时候因为刚建立时栈信息已经配置好了 所以这里不需要makecontext配置栈信息了 
只需要swapcontext切换到此协程就ok**/
    case COROUTINE_SUSPEND:
        memcpy(S->stack + STACK_SIZE - C->size, C->stack, C->size);//将协程内部的栈数据拷贝到调度器的栈空间 这是为恢复本协程的执行做准备
        S->running = id;
        C->status = COROUTINE_RUNNING;
        swapcontext(&S->main, &C->ctx);
        break;
    default:
        assert(0);
    }
}
```

关键就是swapcontext进行上下文的切换，当切换到C->ctx时，由于之前为C->ctx配置的函数名是mainfunc，所以一切换就相当于下一步去执行mainfunc啦，同时2代表有两个参数，然后后面两个是调度器结构的地址，由于是64位的机器，这里将高低32位分别放到两个变量里传给mainfunc（这里如果不太明白继续看开头的例子）。
 而mainfunc我们可以想见，肯定要去调用协程关联的那个函数：



```cpp
coroutine.c:
static void
mainfunc(uint32_t low32, uint32_t hi32) {
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    struct schedule *S = (struct schedule *)ptr;
    int id = S->running;
    struct coroutine *C = S->co[id];
    C->func(S,C->ud);//调用协程要处理的业务函数 你自己写的
    _co_delete(C);
    S->co[id] = NULL;
    --S->nco;
    S->running = -1;
}
```

关键一步我已经加上注释，执行完协程肯定要销毁资源，下面那些都是在将调度器此协程id对应的协程结构体销毁掉。还有个小细节，这个函数作者前面加了一个static，由于此函数只在本文件（coroutine.c）内部使用，所以作者加了static（有点像private那种feel static修饰的函数只可以在声明所在的文件内部使用）。

最最精彩的来了，协程既然要切换，肯定要有个方法让出CPU吧，here it comes:



```rust
coroutine.c:
void
coroutine_yield(struct schedule * S) {
    int id = S->running;
    assert(id >= 0);
    struct coroutine * C = S->co[id];
    assert((char *)&C > S->stack);
    _save_stack(C,S->stack + STACK_SIZE);//让出CPU最牛逼的一句 将当前栈信息保存到协程结构体里面的char *stack那个指针里面
    C->status = COROUTINE_SUSPEND;//让出CPU后 状态自然要是SUSPEND
    S->running = -1;
    swapcontext(&C->ctx , &S->main);//正式让出CPU
}

static void
_save_stack(struct coroutine *C, char *top) {
    char dummy = 0;//神来之笔
    assert(top - &dummy <= STACK_SIZE);
    if (C->cap < top - &dummy) {
        free(C->stack);
        C->cap = top-&dummy;
        C->stack = malloc(C->cap);
    }
    C->size = top - &dummy;
    memcpy(C->stack, &dummy, C->size);
}
```

上面这个最精彩就是_save_stack里面那个char dummy，这个变量有毛线用呢？
 如果之前记得我们的例子，你应该知道这个dummy我们关心不是他的值（这里等于1 、2、3....100 whatever 不重要），重要的是这个dummy的地址，这个地址在哪儿呀？ 在栈上，而别忘了栈是朝地址小的方向增长的！！！那么top一定比dummy的地址大（因为栈是朝地址小的方向增长的），那么代码中top-&dummy对应一定是啥呀?那一定是这个协程执行过程中产生的栈信息（肯定不能丢呀 丢了这个协程再次回过头来懵逼了），所以下面利用memcpy函数将&dummy地址拷贝数据到C->stack里面（拷贝大小自然是top- &dummy 看得出这里才对协程中的char *stack进行内存分配 用时拷贝 一开始你也不知道要分配多少合适呀 同时这里也解释了为啥struct coroutine里面的cap和size变量是ptrdiff_t的类型 因为这两个字段都是靠指针做减法得到的）。这里的dummy变量真是天马行空，想象力奇特的一种写法，小弟敬佩！
 既然调度器里面栈信息已经拷贝到协程结构里面啦，那其他协程执行时是不是可以随意搞了，反正影响不到此协程了呀，真心是棒棒哒！

最后，看看作者在main函数里面给的例子吧：



```cpp
main.c:
#include "coroutine.h"
#include <stdio.h>

struct args {
    int n;
};

static void
foo(struct schedule * S, void *ud) {
    struct args * arg = ud;
    int start = arg->n;
    int i;
    for (i=0;i<5;i++) {
        printf("coroutine %d : %d\n",coroutine_running(S) , start + i);
        coroutine_yield(S);
    }
}

static void
test(struct schedule *S) {
    struct args arg1 = { 0 };
    struct args arg2 = { 100 };

    int co1 = coroutine_new(S, foo, &arg1);
    int co2 = coroutine_new(S, foo, &arg2);
    printf("main start\n");
    while (coroutine_status(S,co1) && coroutine_status(S,co2)) {
        coroutine_resume(S,co1);
        coroutine_resume(S,co2);
    } 
    printf("main end\n");
}

int 
main() {
    struct schedule * S = coroutine_open();
    test(S);
    coroutine_close(S);
    
    return 0;
}
```

上图中的foo函数相当于业务代码，需要我们自己写，这里循环一次让出一次CPU，所以代码执行结果如下：



```ruby
main start
coroutine 0 : 0
coroutine 1 : 100
coroutine 0 : 1
coroutine 1 : 101
coroutine 0 : 2
coroutine 1 : 102
coroutine 0 : 3
coroutine 1 : 103
coroutine 0 : 4
coroutine 1 : 104
main end
```

有没有一种多线程运行结果的既视感？ 但是可以看出代码中既没有使用fork函数（多进程）也没有使用pthread函数（多线程），操作系统压根没有感觉到你在切换（我们根本没有进入内核态），我们在用户态就实现了这种切换的feel。真心是棒棒哒！

还有几个小函数没有讲到，因为那几个真心太简单了，有的一两行，都是为上面的核心流程服务的，理解了核心流程，那几个函数自然而然的搞懂。
 当然这个版本毕竟比较简单，还只能做到协程自己主动让出CPU，但是这么雷锋的协程毕竟很少，其实更多时候是碰到阻塞操作了（比如IO操作），需要让出CPU，那就是要涉及IO多路复用啦，云风大神只是给我们打个样，让我们理解这个版本之后往更复杂的版本迈进！

先写到此吧，继续撸libtask和libgo的代码啦！有感再撰文！
 I love coroutine!



作者：海洋之木
链接：https://www.jianshu.com/p/c4c1af5a20d9
来源：简书
著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。