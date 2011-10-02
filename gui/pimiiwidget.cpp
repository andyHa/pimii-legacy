#include "pimiiwidget.h"
#include <QElapsedTimer>

PimiiWidget::PimiiWidget(QObject *parent) :
    QObject(parent), executor(this)
{
    engine = new Engine();
    engine->setInterceptor(this);
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
