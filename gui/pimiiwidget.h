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

#ifndef PIMII_WIDGET_H
#define PIMII_WIDGET_H

#include <QObject>
#include <QString>
#include <QThread>

#include "vm/env.h"
#include "vm/engine.h"
#include "gui/webwindow.h"

class PimiiWidget : public QObject, public Interceptor
{
    Q_OBJECT
public:
    explicit PimiiWidget(QObject *parent = 0);
    ~PimiiWidget();

    /**
      Evaluates the given source code.
      */
    void evaluate(QString source, QString filename);

    /**
      Determines if the computation is running.
      */
    bool isRunning();

    /**
      Provides access to the underlying engine.
      */
    Engine* getEngine();

    virtual void println(const QString& std);

    virtual void reportStatus(const EngineStatus& status);

    virtual QWebFrame* getFrame();

signals:
    /**
      Emitted if exectuion starts.
      */
    void computationStarted(void);

    /**
      Emitted if execution is stopped or interrupted.
      */
    void computationStopped(void);

    /**
      Emits a log message.
      */
    void log(QString msg);

    /**
      Emits the current machine status.
      */
    void status(EngineStatus status);

public slots:

    /**
      Interrupts the current computation
      */
    void interrupt();

    /**
      Continues the evaluation
      */
    void continueEvaluation();

private:
    class Executor : public QThread {
    private:
        PimiiWidget* parent;
    public:
        Executor(PimiiWidget* engine) : parent(engine) {}
        virtual void run();
    };

    Engine* engine;
    Executor executor;
    WebWindow* ww;

    friend class Executor;
};

#endif // PIMII_WIDGET_H
