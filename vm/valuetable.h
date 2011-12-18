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
  Represents a value table used by the storage engine for string values etc.
  ---------------------------------------------------------------------------
  */

#ifndef VALUETABLE_H
#define VALUETABLE_H

#include <vector>


template< typename I,typename V >
class ValueTable
{

    struct Entry {
        I refCount;
        bool inUse;
        union {
            I freePointer;
            V* data;
        };
    };

    I freeList;
    I usedCells;
    std::vector<Entry*> table;
public:
    ValueTable() {
        clear();
    }

    ~ValueTable() {
        clear();
    }

    void clear() {
        freeList = 0;
        usedCells = 0;

        for(typename std::vector<Entry*>::iterator
            i = table.begin();
            i != table.end();
            ++i)
        {
            delete *i;
        }
        table.clear();
    }

    I allocate(const V& initialValue) {
        usedCells++;
        if (freeList > 0) {
            I result = freeList - 1;
            freeList = table[result]->freePointer;
            table[result]->data = new V(initialValue);
            table[result]->inUse = true;
            return result;
        } else {
            Entry* e = new Entry();
            e->data = new V(initialValue);
            e->inUse = true;
            I result = table.size();
            table.push_back(e);
            return result;
        }
    }

    V get(I index) {
        return *table[index]->data;
    }

    bool inUse(I index) {
        return table[index]->inUse;
    }

    I getNumberOfUsedCells() {
        return usedCells;
    }

    I getTotalCells() {
        return table.size();
    }

    void resetRefCount() {
        for(I i = 0; i < size(); i++) {
            table[i]->refCount = 0;
        }
    }

    void inc(I index) {
        table[index]->refCount++;
    }

    void gc() {
        for(I i = 0; i < size(); i++) {
            Entry* e = table[i];
            if (e->refCount == 0 && e->inUse) {
                delete e->data;
                e->freePointer = freeList;
                e->inUse = false;
                freeList = i + 1;
                usedCells--;
            }
        }
    }

    I size() {
        return table.size();
    }


};

#endif // VALUETABLE_H
