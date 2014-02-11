#ifndef JSCI_H
#define JSCI_H

#include <QObject>
#include <QVariant>
#include "../coreinterface.h"

class JsCiTest : public QObject
{
    Q_OBJECT
public:
    JsCiTest();
    QString loadXDFL();
    void runScript();

signals:

public slots:

    /**
     * This function starts a test of the program. It connects the Core Interface to a series of motions
     * These are run by connected the CoreInterface to the connected(int i) function. as States change the
     * tasks update in connected
     **/
    void test();
    void connected(int i);
    void pause();
    void resume();


private:
    CoreInterface *ci_;
    QScriptEngine engine_;
    QString script_;

};

#endif // JSCI_H
