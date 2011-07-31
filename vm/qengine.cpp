#include "qengine.h"
#include <QElapsedTimer>

QEngine::QEngine(QObject *parent) :
    QObject(parent), engine(Engine(this)), executor(this)
{
}

void QEngine::println(String std) {
    emit log(QString::fromStdWString(std));
}

void QEngine::reportStatus(EngineStatus s) {
    emit status(s);
}

void QEngine::interrupt() {
    engine.interrupt();
}

void QEngine::continueEvaluation() {
    if (!executor.isRunning()) {
        executor.start();
    }
}

void QEngine::Executor::run() {
    emit parent->computationStarted();
    parent->engine.continueEvaluation();
    emit parent->computationStopped();
}


void QEngine::evaluate(QString source, QString filename) {
    if (!executor.isRunning()) {
        engine.prepareEval(source.toStdWString(), filename.toStdWString());
        executor.start();
    }
}

Engine& QEngine::getEngine() {
    return engine;
}

bool QEngine::isRunnung() {
    return executor.isRunning();
}
