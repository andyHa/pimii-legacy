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
// ---------------------------------------------------------------------------
// Represents a value table used by the storage engine for string values etc.
// ---------------------------------------------------------------------------
#ifndef VALUETABLE_H
#define VALUETABLE_H

#include <vector>

#include <QReadWriteLock>

template<typename I,typename V>
class ValueTable
{

    union Entry {
        I refCount;
        I freePointer;
        V* data;
    };

    I freeList;
    I usedCells;
    std::vector<Entry> table;    
    QReadWriteLock* lock;

public:
    ValueTable() : lock(new QReadWriteLock()) {
        clear();
    }
    ~ValueTable() {
        delete lock;
    }

    void clear() {
        QWriteLocker locker(lock);
        freeList = 0;
        usedCells = 0;
        table.clear();
    }

    I allocate(V initialValue) {
        QWriteLocker locker(lock);
        usedCells++;
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
        QReadLocker locker(lock);
        return *table[index].data;
    }

    I getNumberOfUsedCells() {
        QReadLocker locker(lock);
        return usedCells;
    }

    I getTotalCells() {
        QReadLocker locker(lock);
        return table.size();
    }

    void resetRefCount() {
        //QWriteLocker locker(lock);
        for(I i = 0; i < size(); i++) {
            table[i].refCount = 0;
        }
    }

    void inc(I index) {
        QReadLocker locker(lock);
        table[index].refCount++;
    }

    void gc() {
        //QWriteLocker locker(lock);
        for(I i = 0; i < size(); i++) {
            Entry e = table[i];
            if (e.refCount == 0) {
                delete(e.data);
                e.freePointer = freeList;
                freeList = i + 1;
                usedCells--;
            }
        }
    }

    I size() {
        QReadLocker locker(lock);
        return table.size();
    }


};

#endif // VALUETABLE_H
