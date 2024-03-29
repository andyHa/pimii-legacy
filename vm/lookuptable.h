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
  Represents a lookup table used by the storage engine for various tables.
  ---------------------------------------------------------------------------
  */
#ifndef LOOKUPTABLE_H
#define LOOKUPTABLE_H

#include <vector>
#include <map>
#include <utility>

#include <qglobal.h>

template<typename K, typename V, typename I>
class LookupTable
{
    std::map<K, I> mapping;
    std::vector< std::pair<K,V> > table;

    Q_DISABLE_COPY(LookupTable)
public:
    LookupTable() {}

    void clear(){
        mapping.clear();
        table.clear();
    }

    I add(const K& key, const V& initialValue){
        typename std::map<K,I>::iterator it = mapping.find(key);
        I result = 0;
        if (it == mapping.end()) {
            result = table.size();
            table.push_back(std::make_pair(key, initialValue));
            mapping[key] = result;
        } else {
            result = it->second;
        }
        return result;
    }

    bool find(const K& key, I* index){
        typename std::map<K, I>::iterator iter = mapping.find(key);
        bool result = (iter != mapping.end());
        if (result) {
            *index = iter->second;
        }
        return result;
    }

    K getKey(I index){
        return this->table[index].first;
    }

    void setValue(I index, V value){
        this->table[index].second = value;
    }

    V getValue(I index){
        return this->table[index].second;
    }

    I size() {
        return table.size();
    }

};

#endif // LOOKUPTABLE_H
