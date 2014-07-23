#ifndef BASEPROJECT_H
#define BASEPROJECT_H

#include <QObject>
#include <QString>
#include <QStringList>

#include "ugeninterface.h"

class BaseProject : public QObject
{
    Q_OBJECT
public:
    BaseProject(QString dir);
    virtual ~BaseProject();

    QString getType() {return m_projectType;}
    QString getTarget() {return m_target;}
    QString getBoardId() {return m_target;}
    void setTarget(QString target) {m_target = target;}
    void setBoardId(QString id) {m_board_id = id;}

    virtual void setProjectName(QString name) = 0;
    virtual void save() = 0;

public slots:
    virtual void build() {}
    virtual void flash() {}
    virtual void run(bool pressed) {Q_UNUSED(pressed);}
    virtual QStringList listTargets() {return QStringList();}
    virtual QStringList listDevices() {return QStringList();}


protected:
    QString m_projectDir;
    QString m_projectType;
    QString m_target;
    QString m_board_id;

    UgenInterface ugens;

signals:
    void outputText(QString text);
    void errorText(QString text);
    void programStopped();
};

#endif // BASEPROJECT_H
