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

std::set<Appender*> Logger::appenders;
Level Logger::level;
void Logger::log(const QString& msg, const QString& pos) {
    for(std::set<Appender*>::iterator
        iter = appenders.begin();
        iter != appenders.end();
        ++iter)
    {
        (*iter)->append(msg, pos);
    }
    std::cout << msg.toStdString().c_str()<<
                  " (" << pos.toStdString().c_str()<< ")" <<
                  std::endl;
}


void Logger::addAppender(Appender* a) {
    appenders.insert(a);
}

void Logger::removeAppender(Appender* a) {
    appenders.erase(a);
}

void Logger::setLevel(Level newlevel) {
    level = newlevel;
}

Level Logger::getLevel() {
    return level;
}

Logger::Logger(const QString& loggerName) : name(loggerName) {
}

Logger::~Logger() {

}
