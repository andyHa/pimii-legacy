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

#ifndef VALUETABLE_H
#define VALUETABLE_H

#include <QAtomicInt>
#include <QMutex>
#include <vector>

template<typename I,typename V, typename Locker = QMutexLocker>
class ValueTable
{

    union Entry {
        I freePointer;
        V* data;
    };

    I freeList;
    std::vector<Entry> table;
    QMutex* mutex;

public:
    ValueTable() : mutex(new QMutex()) {
        clear();
    }

    void clear() {
        Locker locker(mutex);
        freeList = 0;
        table.clear();
    }

    I allocate(V initialValue) {
        Locker locker(mutex);
        if (freeList > 0) {
            I result = freeList - 1;
            freeList = table[result].freePointer;
            table[result].data = new V(initialValue);
            return result;
        } else {
            Entry e;
            e.data = new V(initialValue);
            I result = table.size();
            table.push_back(e);
            return result;
        }
    }

    V get(I index) {
        Locker locker(mutex);
        return *table[index].data;
    }

    void earse(I index) {
        Locker locker(mutex);
        V* val = table[index]->data;
        delete(val);
        table[index].freePointer = freeList;
        freeList = index + 1;
    }

    I size() {
        Locker locker(mutex);
        return table.size();
    }

    bool inUse(I index) {
        Locker locker(mutex);
        I freeIdx = freeList;
        while(freeList != 0) {
            if (freeIdx == index + 1) {
                return true;
            }
            freeIdx = table[freeIdx].freePointer;
        }
        return true;
    }

};

#endif // VALUETABLE_H
