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
