#ifndef QENGINE_H
#define QENGINE_H

#include <QObject>
#include <QString>
#include <QThread>

#include "vm/env.h"
#include "vm/engine.h"

class QEngine : public QObject, public Logger
{
    Q_OBJECT
public:
    explicit QEngine(QObject *parent = 0);

    /**
      Evaluates the given source code.
      */
    void evaluate(QString source, QString filename);

    /**
      Determines if the computation is currently interrupted.
      */
    bool isInterrupted();

    /**
      Determines if the computation is running.
      */
    bool isRunnung();

    /**
      Provides access to the underlying engine.
      */
    Engine &getEngine();

    virtual void println(String std);

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
        QEngine* parent;
    public:
        Executor(QEngine* engine) : parent(engine) {}
        virtual void run();
    };

    Engine engine;
    Executor executor;
    friend class Executor;
};

#endif // QENGINE_H
