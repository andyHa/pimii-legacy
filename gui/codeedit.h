#ifndef CODEEDIT_H
#define CODEEDIT_H

#include <QTextEdit>

class CodeEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit CodeEdit(QWidget *parent = 0);
    virtual ~CodeEdit();
protected:
    virtual void keyPressEvent(QKeyEvent *e);
signals:

public slots:

};

#endif // CODEEDIT_H
