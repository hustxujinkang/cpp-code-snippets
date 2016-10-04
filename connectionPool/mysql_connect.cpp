#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include "mysql_connect.h"

ConnectionPool* ConnectionPool::pool = nullptr;

ConnectionPool::ConnectionPool(std::string name, std::string pwd, std::string nurl, int maxSize):
    username(name), password(pwd), url(nurl), poolSize(maxSize)
{
    driver = get_driver_instance();
    initConnectPool(poolSize / 2);
}
ConnectionPool::~ConnectionPool()
{
     destroyPool();
}

int ConnectionPool::getPoolSize()
{
     return conList.size();
}

void ConnectionPool::addConn(int size)
{
     for(int i = 0; i < size; i++)
     {
         Connection *conn = driver->connect(url, username, password);
         std::shared_ptr<Connection> sp(conn,
                 [](Connection* conn)
                 {
                      delete conn;
                 });
        conList.push_back(std::move(sp));
     }
}

void ConnectionPool::initConnectPool(int initialSize)
{
    std::lock_guard<std::mutex> locker(lock);
    addConn(initialSize);
}

void ConnectionPool::destroyOneConn()
{
    std::shared_ptr<Connection>&& sp = std::move(conList.front());
    sp->close();
    --poolSize;
}

void ConnectionPool::destroyPool()
{
     while(!conList.empty())
     {
         std::shared_ptr<Connection>&& sp = std::move(conList.front());
         conList.pop_front();
         sp->close();
     }
}

void ConnectionPool::expandPool(int size)
{
    std::lock_guard<std::mutex> locker(lock);
    addConn(size);
    poolSize += size;
}

void ConnectionPool::reducePool(int size)
{
    std::lock_guard<std::mutex> locker(lock);
    if(size > poolSize)
    {
        return;
    }
    for(int i = 0; i < size; i++)
    {
         destroyOneConn();
    }
    poolSize -= size;
}

ConnectionPool* ConnectionPool::getInstance()
{
     if(pool == nullptr)
     {
        pool = new ConnectionPool("root", "*****", "tcp://127.0.0.1:3306", 40);
     }
     return pool;
}
std::shared_ptr<Connection>
ConnectionPool::getConnect()
{
    std::lock_guard<std::mutex> locker(lock);
    std::shared_ptr<Connection> sp = conList.front();
    conList.pop_front();
    return sp;
}

void ConnectionPool::retConnect(std::shared_ptr<Connection>& ret)
{
    std::lock_guard<std::mutex> locker(lock);
    conList.push_back(ret);
}
