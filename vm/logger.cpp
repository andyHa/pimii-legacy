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

#include "logger.h"
#include <sstream>
#include <string>
#include <iostream>

std::vector<Appender*> Logger::appenders;
std::map<std::string, Logger*> Logger::instances;

void Logger::log(const std::string& msg) {
    for(std::vector<Appender*>::iterator
        iter = appenders.begin();
        iter != appenders.end();
        ++iter)
    {
        (*iter)->append(msg);
    }
     std::cout << msg;
}


void Logger::addAppender(Appender* a) {
    appenders.push_back(a);
}

void Logger::setLevel(std::string name, Level level) {
    std::map<std::string, Logger*>::iterator i = instances.find(name);
    if (i != instances.end()) {
        (*i).second->level = level;
    }
}

Level Logger::getLevel(std::string name) {
    std::map<std::string, Logger*>::iterator i = instances.find(name);
    if (i != instances.end()) {
        return (*i).second->level;
    }
    return ERROR;
}

Logger::Logger(const char* loggerName) : name(std::string(loggerName)) {
    level = INFO;
    instances[name] = this;
}

Logger::~Logger() {
    std::map<std::string, Logger*>::iterator i = instances.find(name);
    if (i != instances.end()) {
        instances.erase(i);
    }
}
