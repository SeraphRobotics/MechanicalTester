#ifndef MASTERCONTROLUNIT_H
#define MASTERCONTROLUNIT_H

#include <QObject>
#include <QDomDocument>
#include "scancontroller.h"
#include <QTimer>


class MasterControlUnit : public QObject
{
    Q_OBJECT
public:
    explicit MasterControlUnit(QObject *parent = 0);
    ~MasterControlUnit();
    void connectToVM(QString filestr, QString Port);
    void loadScanConfig(QString filestr);

signals:
    void error(QString);
    void update(QString);


public slots:
    void startScan();
    void scanState(bool b);


private slots:
    void makeConnections();


private:
    ScanController* SC_;
    VirtualPrinter* VM_;

};

#endif // MASTERCONTROLUNIT_H
