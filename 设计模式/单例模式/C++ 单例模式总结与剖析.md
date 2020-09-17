# [C++ 单例模式总结与剖析](https://www.debugger.wiki/article/html/1550331735191819)

##### 2019-02-16

目录

- C++ 单例模式总结与剖析
  - [一、什么是单例](https://www.debugger.wiki/article/html/1550331735191819#一什么是单例)
  - 二、C++单例的实现
    - [2.1 基础要点](https://www.debugger.wiki/article/html/1550331735191819#基础要点)
    - [2.2 C++ 实现单例的几种方式](https://www.debugger.wiki/article/html/1550331735191819#c-实现单例的几种方式)
    - [2.3 单例的模板](https://www.debugger.wiki/article/html/1550331735191819#单例的模板)
  - 三、何时应该使用或者不使用单例
    - [反对单例的理由](https://www.debugger.wiki/article/html/1550331735191819#反对单例的理由)
  - [参考文章](https://www.debugger.wiki/article/html/1550331735191819#参考文章)

# C++ 单例模式总结与剖析

单例可能是最常用的简单的一种设计模式，实现方法多样，根据不同的需求有不同的写法; 同时单例也有其局限性，因此有很多人是反对使用单例的。本文对C++ 单例的常见写法进行了一个总结, 包括懒汉式、线程安全、单例模板等； 按照从简单到复杂，最终回归简单的的方式循序渐进地介绍，并且对各种实现方法的局限进行了简单的阐述，大量用到了C++ 11的特性如智能指针, magic static，线程锁; 从头到尾理解下来，对于学习和巩固C++语言特性还是很有帮助的。本文的全部代码在 g++ 5.4.0 编译器下编译运行通过，可以在我的github 仓库中找到。

## 一、什么是单例

单例 Singleton 是设计模式的一种，其特点是只提供**唯一**一个类的实例,具有全局变量的特点，在任何位置都可以通过接口获取到那个唯一实例;
具体运用场景如：

1. 设备管理器，系统中可能有多个设备，但是只有一个设备管理器，用于管理设备驱动;
2. 数据池，用来缓存数据的数据结构，需要在一处写，多处读取或者多处写，多处读取;

## 二、C++单例的实现

### 2.1 基础要点

- 全局只有一个实例：static 特性，同时禁止用户自己声明并定义实例（把构造函数设为 private）
- 线程安全
- 禁止赋值和拷贝
- 用户通过接口获取实例：使用 static 类成员函数

### 2.2 C++ 实现单例的几种方式

#### 2.2.1 有缺陷的懒汉式

懒汉式(Lazy-Initialization)的方法是直到使用时才实例化对象，也就说直到调用get_instance() 方法的时候才 new 一个单例的对象。好处是如果被调用就不会占用内存。

```
#include <iostream>
// version1:
// with problems below:
// 1. thread is not safe
// 2. memory leak

class Singleton{
private:
    Singleton(){
        std::cout<<"constructor called!"<<std::endl;
    }
    Singleton(Singleton&)=delete;
    Singleton& operator=(const Singleton&)=delete;
    static Singleton* m_instance_ptr;
public:
    ~Singleton(){
        std::cout<<"destructor called!"<<std::endl;
    }
    static Singleton* get_instance(){
        if(m_instance_ptr==nullptr){
              m_instance_ptr = new Singleton;
        }
        return m_instance_ptr;
    }
    void use() const { std::cout << "in use" << std::endl; }
};

Singleton* Singleton::m_instance_ptr = nullptr;

int main(){
    Singleton* instance = Singleton::get_instance();
    Singleton* instance_2 = Singleton::get_instance();
    return 0;
}
```

运行的结果是

```
constructor called!
```

可以看到，获取了两次类的实例，却只有一次类的构造函数被调用，表明只生成了唯一实例，这是个最基础版本的单例实现，他有哪些问题呢？

1. **线程安全的问题**,当多线程获取单例时有可能引发竞态条件：第一个线程在if中判断 `m_instance_ptr`是空的，于是开始实例化单例;同时第2个线程也尝试获取单例，这个时候判断`m_instance_ptr`还是空的，于是也开始实例化单例;这样就会实例化出两个对象,这就是线程安全问题的由来; **解决办法**:加锁
2. **内存泄漏**. 注意到类中只负责new出对象，却没有负责delete对象，因此只有构造函数被调用，析构函数却没有被调用;因此会导致内存泄漏。**解决办法**： 使用共享指针;

因此，这里提供一个改进的，线程安全的、使用智能指针的实现;

#### 2.2.2 线程安全、内存安全的懒汉式单例 （智能指针，锁）

```
#include <iostream>
#include <memory> // shared_ptr
#include <mutex>  // mutex

// version 2:
// with problems below fixed:
// 1. thread is safe now
// 2. memory doesn't leak

class Singleton{
public:
    typedef std::shared_ptr<Singleton> Ptr;
    ~Singleton(){
        std::cout<<"destructor called!"<<std::endl;
    }
    Singleton(Singleton&)=delete;
    Singleton& operator=(const Singleton&)=delete;
    static Ptr get_instance(){

        // "double checked lock"
        if(m_instance_ptr==nullptr){
            std::lock_guard<std::mutex> lk(m_mutex);
            if(m_instance_ptr == nullptr){
              m_instance_ptr = std::shared_ptr<Singleton>(new Singleton);
            }
            return m_instance_ptr;
        }
    }


private:
    Singleton(){
        std::cout<<"constructor called!"<<std::endl;
    }
    static Ptr m_instance_ptr;
    static std::mutex m_mutex;
};

// initialization static variables out of class
Singleton::Ptr Singleton::m_instance_ptr = nullptr;
std::mutex Singleton::m_mutex;

int main(){
    Singleton::Ptr instance = Singleton::get_instance();
    Singleton::Ptr instance2 = Singleton::get_instance();
    return 0;
}
```

运行结果如下，发现确实只构造了一次实例，并且发生了析构。

```
constructor called!
destructor called!
```

shared_ptr和mutex都是C++11的标准，以上这种方法的优点是

- 基于 shared_ptr, 用了C++比较倡导的 RAII思想，用对象管理资源,当 shared_ptr 析构的时候，new 出来的对象也会被 delete掉。以此避免内存泄漏。
- 加了锁，使用互斥量来达到线程安全。这里使用了两个 if判断语句的技术称为**双检锁**；好处是，只有判断指针为空的时候才加锁，避免每次调用 get_instance的方法都加锁，锁的开销毕竟还是有点大的。

不足之处在于： 使用智能指针会要求用户也得使用智能指针，非必要不应该提出这种约束; 使用锁也有开销; 同时代码量也增多了，实现上我们希望越简单越好。

还有更加严重的问题，在某些平台（与编译器和指令集架构有关），==双检锁会失效==！具体可以看[这篇文章](http://www.drdobbs.com/cpp/c-and-the-perils-of-double-checked-locki/184405726)，解释了为什么会发生这样的事情。

因此这里还有第三种的基于 Magic Staic的方法达到线程安全

#### 2.2.3 最推荐的懒汉式单例(magic static )——局部静态变量

```
#include <iostream>

class Singleton
{
public:
    ~Singleton(){
        std::cout<<"destructor called!"<<std::endl;
    }
    Singleton(const Singleton&)=delete;
    Singleton& operator=(const Singleton&)=delete;
    static Singleton& get_instance(){
        static Singleton instance;
        return instance;

    }
private:
    Singleton(){
        std::cout<<"constructor called!"<<std::endl;
    }
};

int main(int argc, char *argv[])
{
    Singleton& instance_1 = Singleton::get_instance();
    Singleton& instance_2 = Singleton::get_instance();
    return 0;
}
```

运行结果

```
constructor called!
destructor called!
```

这种方法又叫做 Meyers' Singleton[Meyer's的单例](https://stackoverflow.com/questions/449436/singleton-instance-declared-as-static-variable-of-getinstance-method-is-it-thre/449823#449823)， 是著名的写出《Effective C++》系列书籍的作者 Meyers 提出的。所用到的特性是在C++11标准中的[Magic Static](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2660.htm)特性：

> If control enters the declaration concurrently while the variable is being initialized, the concurrent execution shall wait for completion of the initialization.
> 如果当变量在初始化的时候，并发同时进入声明语句，并发线程将会阻塞等待初始化结束。

这样保证了并发线程在获取静态局部变量的时候一定是初始化过的，所以具有线程安全性。

[C++静态变量的生存期](https://stackoverflow.com/questions/246564/what-is-the-lifetime-of-a-static-variable-in-a-c-function) 是从声明到程序结束，这也是一种懒汉式。

**这是最推荐的一种单例实现方式：**

1. 通过局部静态变量的特性保证了线程安全 (C++11, GCC > 4.3, VS2015支持该特性);
2. 不需要使用共享指针，代码简洁；
3. 注意在使用的时候需要声明单例的引用 `Single&` 才能获取对象。

另外网上有人的实现返回指针而不是返回引用

```
static Singleton* get_instance(){
    static Singleton instance;
    return &instance;
}
```

这样做并不好，理由主要是无法避免用户使用`delete instance`导致对象被提前销毁。还是建议大家使用返回引用的方式。

#### 2.2.4 函数返回引用

有人在网上提供了这样一种单例的实现方式；

```
#include <iostream>

class A
{
public:
    A() {
        std::cout<<"constructor" <<std::endl;
    }
    ~A(){
        std::cout<<"destructor"<<std::endl;
    }
};


A& ret_singleton(){
    static A instance;
    return instance;
}

int main(int argc, char *argv[])
{
    A& instance_1 = ret_singleton();
    A& instance_2 = ret_singleton();
    return 0;
}
```

严格来说，这不属于单例了，因为类A只是个寻常的类，可以被定义出多个实例，但是亮点在于提供了`ret_singleton`的方法，可以返回一个全局（静态）变量，起到类似单例的效果，这要求用户必须保证想要获取 全局变量A ，只通过ret_singleton()的方法。

以上是各种方法实现单例的代码和说明，解释了各种技术实现的初衷和原因。这里会**比较推荐 C++11 标准下的 2.2.3 的方式**，即**使用static local的方法**，简单的理由来说是因为其足够简单却满足所有需求和顾虑。

在某些情况下，我们系统中可能有多个单例，如果都按照这种方式的话，实际上是一种重复，有没有什么方法可以只实现一次单例而能够复用其代码从而实现多个单例呢？ 很自然的我们会考虑使用模板技术或者继承的方法，
在我的博客中有介绍过如何使用单例的模板。

### 2.3 单例的模板

#### 2.3.1 CRTP 奇异递归模板模式实现

代码示例如下：

```
// brief: a singleton base class offering an easy way to create singleton
#include <iostream>

template<typename T>
class Singleton{
public:
    static T& get_instance(){
        static T instance;
        return instance;
    }
    virtual ~Singleton(){
        std::cout<<"destructor called!"<<std::endl;
    }
    Singleton(const Singleton&)=delete;
    Singleton& operator =(const Singleton&)=delete;
protected:
    Singleton(){
        std::cout<<"constructor called!"<<std::endl;
    }

};
/********************************************/
// Example:
// 1.friend class declaration is requiered!
// 2.constructor should be private


class DerivedSingle:public Singleton<DerivedSingle>{
   // !!!! attention!!!
   // needs to be friend in order to
   // access the private constructor/destructor
   friend class Singleton<DerivedSingle>;
public:
   DerivedSingle(const DerivedSingle&)=delete;
   DerivedSingle& operator =(const DerivedSingle&)= delete;
private:
   DerivedSingle()=default;
};

int main(int argc, char* argv[]){
    DerivedSingle& instance1 = DerivedSingle::get_instance();
    DerivedSingle& instance2 = DerivedSingle::get_instance();
    return 0;
}
```

以上实现一个单例的模板基类，使用方法如例子所示意，子类需要**将自己作为模板参数T** 传递给 `Singleton<T>` 模板; 同时需要**将基类声明为友元**，这样才能调用子类的私有构造函数。

基类模板的实现要点是：

1. 构造函数需要是 **protected**，这样子类才能继承；
2. 使用了[奇异递归模板模式](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)CRTP(Curiously recurring template pattern)
3. get instance 方法和 2.2.3 的static local方法一个原理。
4. 在这里基类的析构函数可以不需要 virtual ，因为子类在应用中只会用 Derived 类型，保证了析构时和构造时的类型一致

#### 2.3.2 不需要在子类声明友元的实现方法

在 [stackoverflow](https://codereview.stackexchange.com/questions/173929/modern-c-singleton-template)上， 有大神给出了**不需要在子类中声明友元的方法**，在这里一并放出;精髓在于使用一个代理类 token，子类构造函数需要传递token类才能构造，但是把 token保护其起来， 然后子类的构造函数就可以是公有的了，这个子类只有 `Derived(token)`的这样的构造函数，这样用户就无法自己定义一个类的实例了，起到控制其唯一性的作用。代码如下。

```
// brief: a singleton base class offering an easy way to create singleton
#include <iostream>

template<typename T>
class Singleton{
public:
    static T& get_instance() noexcept(std::is_nothrow_constructible<T>::value){
        static T instance{token()};
        return instance;
    }
    virtual ~Singleton() =default;
    Singleton(const Singleton&)=delete;
    Singleton& operator =(const Singleton&)=delete;
protected:
    struct token{}; // helper class
    Singleton() noexcept=default;
};


/********************************************/
// Example:
// constructor should be public because protected `token` control the access


class DerivedSingle:public Singleton<DerivedSingle>{
public:
   DerivedSingle(token){
       std::cout<<"destructor called!"<<std::endl;
   }

   ~DerivedSingle(){
       std::cout<<"constructor called!"<<std::endl;
   }
   DerivedSingle(const DerivedSingle&)=delete;
   DerivedSingle& operator =(const DerivedSingle&)= delete;
};

int main(int argc, char* argv[]){
    DerivedSingle& instance1 = DerivedSingle::get_instance();
    DerivedSingle& instance2 = DerivedSingle::get_instance();
    return 0;
}
```

#### 2.3.3 函数模板返回引用

在 2.2.4 中提供了一种类型的全局变量的方法，可以把一个一般的类，通过这种方式提供一个类似单例的
全局性效果（但是不能阻止用户自己声明定义这样的类的对象）;在这里我们把这个方法变成一个 template 模板函数，然后就可以得到任何一个类的全局变量。

```
#include <iostream>

class A
{
public:
    A() {
        std::cout<<"constructor" <<std::endl;
    }
    ~A(){
        std::cout<<"destructor"<<std::endl;
    }
};

template<typename T>
T& get_global(){
    static T instance;
    return instance;
}

int main(int argc, char *argv[])
{
    A& instance_1 = get_global<A>();
    A& instance_2 = get_global<A>();
    return 0;
}
```

可以看到这种方式确实非常简洁，同时类仍然具有一般类的特点而不受限制，当然也因此失去了单例那么强的约束（禁止赋值、构造和拷贝构造）。
这里把函数命名为 `get_global()` 是为了强调，这里可以通过这种方式获取得到单例最重要的全局变量特性；但是并不是单例的模式。

## 三、何时应该使用或者不使用单例

根据stackoverflow上的一个高票答案 [singleton-how-should-it-be-used](https://stackoverflow.com/questions/86582/singleton-how-should-it-be-used)：

> You need to have one and only one object of a type in system
> ==你需要系统中只有**唯一**一个实例存在的类的**全局**变量的时候才使用单例==。

- 如果使用单例，应该用什么样子的

  > How to create the best singleton:
  >
  > - The smaller, the better. I am a minimalist
  > - Make sure it is thread safe
  > - Make sure it is never null
  > - Make sure it is created only once
  > - Lazy or system initialization? Up to your requirements
  > - Sometimes the OS or the JVM creates singletons for you (e.g. in Java every class definition is a singleton)
  > - Provide a destructor or somehow figure out how to dispose resources
  > - Use little memory
  >   ==越小越好，越简单越好，线程安全，内存不泄露==

### 反对单例的理由

当然程序员是分流派的，有些是反对单例的，有些人是反对设计模式的，有些人甚至连面向对象都反对 :).

反对单例的理由有哪些：

## 参考文章

在本文写作的过程中参考了一些博客和stackoverflow 的回答，以超链接的方式体现在文中。另外还有一些我觉得非常精彩的回答，放在下面供读者拓展阅读

推荐阅读：

1. 高票回答中提供了一系列有益的链接(https://stackoverflow.com/questions/1008019/c-singleton-design-pattern/1008289#1008289)
2. 面试中的单例(http://www.cnblogs.com/loveis715/archive/2012/07/18/2598409.html)
3. 一些观点(https://segmentfault.com/q/1010000000593968)