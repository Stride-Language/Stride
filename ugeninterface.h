#ifndef UGENINTERFACE_H
#define UGENINTERFACE_H

#include <QObject>
#include "ugen.h"

class UgenInterface : public QObject
{
    Q_OBJECT
public:
    explicit UgenInterface(QString type = "xmos", QObject *parent = 0);
    bool isUgen(QString token);
signals:

public slots:

private:
    QVector<Ugen> ugens;
};

#endif // UGENINTERFACE_H
