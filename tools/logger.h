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

/**
  Used to mark the purpose of a message which is sent to a logger. Each
  logger also keeps a level, and only logs messages with a level lower or
  equal to its own.
  */
enum Level {
    /**
      Used for errors or failures.
      */
    ERROR = 0,
    /**
      Used for informational messages which concern all users.
      */
    INFO = 1,
    /**
      Used for detailed output to show what a subsystem is doing.
      */
    FINE = 2,
    /**
      Provides very detailed output of each step performed by a subsystem.
      */
    TRACE = 3
};

/**
  Handles log messages.
  */
class Appender {
public:
    /**
      Called by a logger, to handle (output) the given message.
      */
    virtual void append(const std::string& msg) {}
};

/**
  A logger represents the output for a given component or subsystem.

  A Logger has an internal name and a log-level assigned. Additionally
  this class provides static methods to control the log level of loggers
  by name. Also it supports a method to register a new appender.
  */
class Logger {
private:
    /**
      Contains all know appenders.
      */
    static std::vector<Appender*> appenders;
    /**
     Maps names to all known loggers.
     */
    static std::map<std::string, Logger*> instances;

    /**
      A Logger cannot be copied.
      */
    Logger(const Logger&);
    Logger& operator =(const Logger&);
public:
    /**
      Represents the level of this logger.
      */
    Level level;
    /**
      Contains the name of this logger.
      */
    const std::string name;

    /**
      Called by instances to distribute the given message to all appenders.
      */
    static void log(const std::string& msg);

    /**
      Registers a new appender.
      */
    static void addAppender(Appender* a);

    /**
      Sets the level of the given logger.
      */
    static void setLevel(std::string name, Level level);

    /**
      Determines the level of the given logger.
      */
    static Level getLevel(std::string name);

    /**
      Creates a new logger.
      */
    Logger(const char* loggerName);

    /**
      Remove the logger from the instances map.
      */
    ~Logger();
};

/**
  Internal class constructed by the macros below. Used to generate one log
  message
  */
class Log {
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
        os << "[" << logger.name << "]\t";
        return os;
    }

protected:
    std::ostringstream os;
private:
    Log(const Log&);
    Log& operator =(const Log&);
};

/**
  The macros below generate messages with the named levels. The generated if
  statements should not impose performance constraints, but logging can be
  completely turned off by defining NO_LOGGIG (or NO_TRACING) to disable
  logging (or just tracing) statements.
  */

#ifndef NO_LOGGING
    #define ERROR(logger, msg) \
        if (logger.level >= ERROR) { \
            Log(logger, __FILE__, __LINE__).get() << msg; \
        }

    #define INFO(logger, msg) \
        if (logger.level >= INFO) { \
            Log(logger, __FILE__, __LINE__).get() << msg; \
        }

    #define FINE(logger, msg) \
        if (logger.level >= FINE) { \
            Log(logger, __FILE__, __LINE__).get() << msg; \
        }

    #ifndef NO_TRACING

        #define TRACE(logger, msg) \
            if (logger.level >= TRACE) { \
                Log(logger, __FILE__, __LINE__).get() << msg; \
            }
    #else
        #define TRACE(logger, msg)
    #endif
#else
    #define ERROR(logger, msg)

    #define INFO(logger, msg)

    #define FINE(logger, msg)

    #define TRACE(logger, msg)
#endif

#endif // LOGGER_H
