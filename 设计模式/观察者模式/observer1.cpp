#include <iostream>
#include <list>

using namespace std;

class Subject;

class Observer
{
public:
    Observer(){}
    virtual ~Observer(){}
    virtual void update(Subject *subject) = 0;
    virtual void update(string content) = 0;
};


class  Subject
{
public:
    Subject() {}
    virtual ~ Subject() {}
    virtual string getContent() = 0;
    virtual string getAbstractContent() = 0;
    virtual void setContent(string content) = 0;
    // 订阅主题
    void attach(Observer *observer) {
        observers.push_back(observer);
    }
    // 取消订阅
    void detach(Observer *observer) {
        observers.remove(observer);
    }

    // 通知所有的订阅者
    virtual void notifyObservers() {
        for(Observer *reader: observers) {
            // 拉模型  (也是有推送的性质,只是粒度小一些)
            reader->update(this);
        }
    }
    // 通知所有的订阅者
    virtual void notifyObservers(string content) {
        for(Observer *reader: observers) {
            // 推模型
            reader->update(content);
        }
    }
private:
    list<Observer *> observers;    // 保存注册的观察者
};

class Reader : public Observer
{
public:
    Reader() {}
    virtual ~Reader() {}
    virtual void update(Subject *subject) {
        // 调用对应的方法去拉取内容 subject->getContent()
        cout << m_name << "收到报纸和阅读它, 具体内容" << subject->getContent() << endl;
    }
    virtual void update(string content) {
        // 推模型
        cout << m_name << "收到报纸和阅读它, 具体内容" << content << endl;
    }
    string getName() {
        return m_name;
    }
    void setName(string name) {
        m_name = name;
    }
private:
    string m_name;
};


class NewsPaper: public Subject
{
public:
    NewsPaper() {}
    virtual ~NewsPaper() {}

    void setContent(string content) {
        m_content = content;
        notifyObservers();
    }
    virtual string getContent() {
        return m_content;
    }
    virtual string getAbstractContent() {
        return  "摘要:";
    }
private:
    string m_content;
};


int main()
{
    // 创建一个报纸主题
    NewsPaper *subject = new NewsPaper();

    // 创建观察者，读者
    Reader *reader1 = new Reader();
    reader1->setName("king老师");

    Reader *reader2 = new Reader();
    reader2->setName("柚子老师");

    Reader *reader3 = new Reader();
    reader3->setName("秋香老师");

    subject->attach(reader1);
    subject->setContent("今天上设计模式, 柚子老师主持了");
    cout << "-----------------------" << endl;
    subject->attach(reader2);
    subject->attach(reader3);

    subject->setContent("King老师去上公开课了, 秋香老师主持了");

    cout << "Hello World!" << endl;
    return 0;
}
