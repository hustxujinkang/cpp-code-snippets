#ifndef MYSQL_CONNECT_H
#define MYSQL_CONNECT_H
#include<iostream>
#include<string>
#include<list>
#include<memory>
#include<functional>


#include<mysql_driver.h>
#include<mysql_connection.h>

#include<cppconn/driver.h>
#include<cppconn/statement.h>
#include<cppconn/prepared_statement.h>
#include<cppconn/resultset.h>
#include<exception>

#include<mutex>

using namespace sql;
using delFunc = std::function<void(Connection*)>;

class ConnectionPool
{
public:
    static ConnectionPool* getInstance();
    auto getConnect()->std::shared_ptr<Connection>;
    auto retConnect(std::shared_ptr<Connection>&ret)->void;
    ~ConnectionPool();
private:
    ConnectionPool(std::string name, std::string pwd, std::string url, int maxSize);
    auto initConnectPool(int initialSize)->void;
    auto destroyPool()->void;
    auto destroyOneConn()->void;
    auto expandPool(int size)->void;
    auto reducePool(int size)->void;
    auto addConn(int size)->void;
public:
    auto getPoolSize()->int;
private:
    std::string username;
    std::string password;
    std::string url;
    int poolSize;

    std::list<std::shared_ptr<Connection>> conList;
    static ConnectionPool* pool;
    std::mutex lock;
    Driver *driver;
};




#endif
