/**
    Licensed to the Apache Software Foundation (ASF) under one
    or more contributor license agreements.  See the NOTICE file
    distributed with this work for additional information
    regarding copyright ownership.  The ASF licenses this file
    to you under the Apache License, Version 2.0 (the
    "License"); you may not use this file except in compliance
    with the License.  You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing,
    software distributed under the License is distributed on an
    "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
    KIND, either express or implied.  See the License for the
    specific language governing permissions and limitations
    under the License.
 */
/**
  ---------------------------------------------------------------------------
  Contains a simple logging system.

  The idea is mainly based on:
    Dr.Dobb's - Logging In C++
    By Petru Marginean, September 05, 2007
    http://drdobbs.com/cpp/201804215
  ---------------------------------------------------------------------------
  */

#ifndef LOGGER_H
#define LOGGER_H

#include <sstream>
#include <string>
#include <stdio.h>
#include <vector>
#include <map>

enum Level {
    ERROR = 0,
    INFO = 1,
    FINE = 2,
    TRACE = 3
};

class Appender {
public:
    virtual void append(const std::string& msg) {}
};

class StdErrAppender : Appender {
    void append(std::string &msg) {
        std::cout << msg;
    }
};

class Logger {
private:
    static std::vector<Appender> appenders;
    static std::map<std::string, Logger*> instances;
    Logger(const Logger&);
    Logger& operator =(const Logger&);
public:
    Level level;
    const std::string name;

    static void log(const std::string& msg) {
        for(std::vector<Appender>::iterator
            iter = appenders.begin();
            iter != appenders.end();
            ++iter)
        {
            (*iter).append(msg);
        }
         std::cout << msg;
    }

    static void addAppender(Appender a) {
        appenders.push_back(a);
    }

    static void setLevel(std::string name, Level level) {
        std::map<std::string, Logger*>::iterator i = instances.find(name);
        if (i != instances.end()) {
            (*i).second->level = level;
        }
    }

    static Level getLevel(std::string name) {
        std::map<std::string, Logger*>::iterator i = instances.find(name);
        if (i != instances.end()) {
            return (*i).second->level;
        }
        return ERROR;
    }

    Logger(const char* loggerName) : name(std::string(loggerName)) {
        level = INFO;
        instances[name] = this;
    }

    ~Logger() {
        std::map<std::string, Logger*>::iterator i = instances.find(name);
        if (i != instances.end()) {
            instances.erase(i);
        }
    }
};

std::vector<Appender> Logger::appenders;
std::map<std::string, Logger*> Logger::instances;

class Log
{
public:
    const Logger& logger;
    const char* file;
    const int line;

    inline Log(const Logger& logger, const char* file, const int line) :
        logger(logger),
        file(file),
        line(line) {}

    inline ~Log() {
        os << " (" << file << ":" << line << ")" << std::endl;
        Logger::log(os.str());
    }

    inline std::ostringstream& get() {
        os << "[" << logger.name << "] ";
        return os;
    }

protected:
    std::ostringstream os;
private:
    Log(const Log&);
    Log& operator =(const Log&);
};

#define ERROR(logger) \
    if (logger.level < ERROR) ; \
    else Log(logger, __FILE__, __LINE__).get()

#define INFO(logger) \
    if (logger.level < INFO) ; \
    else Log(logger, __FILE__, __LINE__).get()

#define FINE(logger) \
    if (logger.level < FINE) ; \
    else Log(logger, __FILE__, __LINE__).get()

#define TRACE(logger) \
    if (logger.level < TRACE) ; \
    else Log(logger, __FILE__, __LINE__).get()

#endif // LOGGER_H
