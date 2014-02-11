#ifndef TESTERFORCI_H
#define TESTERFORCI_H

#include <QObject>
#include "../coreinterface.h"

class testerforci : public QObject
{
    Q_OBJECT
public:
    testerforci();
    testerforci(CoreInterface* coreinterface);

signals:

public slots:

    void setConfig();

    //connected to CI
    void updatedPosition(double x, double y, double z);
    void chagedState(int state);
    void moving();
    void estimated(double t, double v, int cmds);
    void materialsAvailable(QMap<int,Material> ms);
    void bayMaterial(int bayid, int materialid);
    void currentCommand(int c);
    void needMaterialLoaded(int i);


    void onPaused();


public:
    void loadXDFL();

private:
    void makeConnections();

private:
    CoreInterface* ci;
    QMap<int,Material> materials_;
    bool connected_;
};

#endif // TESTERFORCI_H
