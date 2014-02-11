#ifndef SCANCONTROLLER_H
#define SCANCONTROLLER_H

#include "FabConLib/FabConLib/virtualmachines.h"
#include <QString>
#include <QObject>
#include <QTimer>


class ScanController: public QObject
{
    Q_OBJECT
public:
    explicit ScanController(QObject *parent = 0);
    void loadVM(VirtualPrinter* vm);

    bool isReady();

public slots:
    void StartScan();
    void StopScan();
    void setScan(float scandistance, float stepsize, int scannumber);
    void setAxis(QString Axis);

private slots:
    void ScanStep();

signals:
    void update(float position);
    void updateStatus(QString s);
    void scanRunning(bool s);
    void scanComplete();
    void error(QString s);



private:
    VirtualPrinter* vm_;
    QTimer* timer_;
    QString axis_;
    bool ready_;
    float scandistance_;
    float scanspeed_;
    int scangoal_;
    int scannumber_;
    float position_;

    QMap<QString,int> axes_;
    QVector<float> xvector_;
    int count_;



};

#endif // SCANCONTROLLER_H
