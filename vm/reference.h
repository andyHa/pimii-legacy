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
  Represents a base class for all kinds of object which can be managed by the
  pimii storage in the referenceTable.
  ---------------------------------------------------------------------------
  */

#ifndef REFERENCE_H
#define REFERENCE_H

#include <QString>

#include <iostream>

/**
  Represents a base class for references to various object, which are managed
  within the pimii memory.
  */
class Reference
{
public:
    Reference() {}
    virtual ~Reference() {}

    /**
      Returns a string representation of this object.
      */
    virtual QString toString() = 0;
};

class DummyReference : public Reference {
    QString data;
public:
    DummyReference(QString str) : data(str) {}
    virtual ~DummyReference() {
        std::wcout << data.toStdWString() << std::endl;
    }

    virtual QString toString() {
        return data;
    }
};

/**
  Used by the storage in the referenceTable to really free reference which
  are no longer used.
  */
class ReferenceGarbageCollector {
public:
    static void reclaimStorage(Reference*& value) {
        delete value;
    }
};

#endif // REFERENCE_H
