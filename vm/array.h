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
  Represents arrays which provide fast random access.
  ---------------------------------------------------------------------------
  */
#ifndef ARRAY_H
#define ARRAY_H

#include "env.h"
#include <stdlib.h>

#include <QSharedPointer>

/**
  Represents an array of Atoms with fast random access. In contrast to C arrays
  this class uses 1 for the first index of the array!
  */
class Array {
private:
    int _length;
    Atom* _data;

    void ensureSize(int minSize) {
        if (_length <= minSize) {
            _data = (Atom*)realloc(_data, minSize * sizeof(Atom));
            for(int i = _length; i < minSize; i++) {
                _data[i] = NIL;
            }
            _length = minSize;
        }
    }
/**
x := array::make(10);
from: 1 to: 15 do: [ i -> array::write(x, i, i); ];
sys::log(x);
*/
    Q_DISABLE_COPY(Array)
public:
    /**
      Used by the garbage collector to avoid double-checking of arrays.
      */
    bool checked;

    Array(int size) {
        _data = (Atom*)malloc(size* sizeof(Atom));
        for(int i = 0; i < size; i++) {
            _data[i] = NIL;
        }
        _length = size;
    }

    ~Array() {
        delete _data;
    }

    Atom at(int pos) {
        assert(pos >= 1);
        ensureSize(pos);
        return _data[pos - 1];
    }

    void put(int pos, Atom value) {
        assert(pos >= 1);
        ensureSize(pos);
        _data[pos - 1] = value;
    }

    int length() {
        return _length;
    }
};

#endif // ARRAY_H
