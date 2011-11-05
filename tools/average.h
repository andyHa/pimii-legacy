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
  Computes a sliding average over a given number of values.
  ---------------------------------------------------------------------------
  */
#ifndef AVERAGE_H
#define AVERAGE_H

template<typename V, int probes = 10>
class Average
{
private:
    V* table;
    int index;
    int count;
    /**
      An Average cannot be copied.
      */
    Average(const Average&);
    Average& operator =(const Average&);
public:
    Average() {
        index = 0;
        count = 0;
        table = new V[10];
    }
    ~Average() {
        delete table;
    }

    /**
      Adds a value to the average.
      */
    void addValue(V value) {
        table[index++] = value;
        index = index % probes;
        if (count < probes) {
            count++;
        }
    }

    /**
      Contains the average for the previously passed values.
      */
    V average() {
        if (count == 0) {
            return 0;
        }
        V sum = 0;
        for(int i = 0; i < count; i++) {
            sum += table[i];
        }
        return sum / count;
    }
};

typedef Average<double> DoubleAverage;

#endif // AVERAGE_H
