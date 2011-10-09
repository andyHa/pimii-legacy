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

#include "pimiiwidget.h"
#include <QElapsedTimer>

PimiiWidget::PimiiWidget(QObject *parent) :
    QObject(parent), executor(this)
{
    engine = new Engine();
    engine->initialize(this);
    ww = NULL;
}

PimiiWidget::~PimiiWidget() {
    if (ww != NULL) {
        delete ww;
    }
    delete engine;
}

QWebFrame* PimiiWidget::getFrame() {
    if (ww == NULL) {
        ww = new WebWindow(NULL);
        ww->show();
    }
    return ww->getWebView()->page()->mainFrame();
}


void PimiiWidget::println(const QString& std) {
    emit log(std);
}

void PimiiWidget::reportStatus(const EngineStatus& s) {
    emit status(s);
}

void PimiiWidget::interrupt() {
    engine->interrupt();
}

void PimiiWidget::continueEvaluation() {
    if (!executor.isRunning()) {
        executor.start();
    }
}

void PimiiWidget::Executor::run() {
    emit parent->computationStarted();
    parent->engine->continueEvaluation();
    emit parent->computationStopped();
}


void PimiiWidget::evaluate(QString source, QString filename) {
    if (!executor.isRunning()) {
        engine->prepareEval(source, filename);
        executor.start();
    }
}

Engine* PimiiWidget::getEngine() {
    return engine;
}

bool PimiiWidget::isRunning() {
    return executor.isRunning();
}
