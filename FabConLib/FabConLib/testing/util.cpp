#include "util.h"
#include <QDebug>

void testdump(NPath h,QString filename) {
    QFile file(filename);
    if (!file.open(QFile::WriteOnly)) {
        qDebug("Could not open dump file");
        return;
    }
    QTextStream out(&file);
    for (int i=0; i < h.numberOfStates(); i++) {
        out << endl;
        State s = h.getState(i);
        for(int j=0;j<h.stateSize();j++) {
            QString str;
            out << str.sprintf("%f",s.at(j));
            if (j!=h.stateSize()-1) {out<<",";}
        }

    }
    file.close();
    qDebug("Wrote file %s",filename.toStdString().c_str());
    fflush(stdout);
}

void test(NPath h) {
    test("Test",h);
}

void test(QString filename,NPath h) {
    // This produces a series of CSV documents that explore NPath and HzPath in relative and absolute versions

    QString nprel("");
    QTextStream nprs(&nprel,QFile::WriteOnly);
    nprs<<filename<<"NpRel.csv";
    h.toRelative();
    testdump(h,nprel);


    QString npabs("");
    QTextStream npas(&npabs,QFile::WriteOnly);
    npas<<filename<<"NpAbs.csv";
    h.toAbsolute();
    testdump(h,npabs);


    QString hzabs("");
    QTextStream hzas(&hzabs,QFile::WriteOnly);
    hzas<<filename<<"HzAbs.csv";
    NPath hpath = NP::toHzPath(&h,0);
    hpath.toAbsolute();
    testdump(hpath,hzabs);

    QString hzrel("");
    QTextStream hzrs(&hzrel,QFile::WriteOnly);
    hzrs<<filename<<"HzRel.csv";

    hpath.toRelative();
    testdump(hpath,hzrel);
}

void printNPath(QString str, NPath* n) {
    qDebug(str.toStdString().c_str());
    State tempstate;
    for(int i=0; i<n->numberOfStates();i++) {
        tempstate = n->getState(i);
        qDebug(" %i:\t",i);
        printState("",tempstate);
    }
}

void printState(QString str,State s) {
    qDebug(str.toStdString().c_str());
    for(int i=0;i<s.size();i++) {
        qDebug("%f ",s.at(i));
    }
}

void printMaterial(Material m) {
    qDebug(" Material: %s",m.name.toStdString().c_str());
    qDebug("\tQ: %f",m.Q);
    QStringList keys = m.property.keys();
    QStringListIterator i(keys);
    while(i.hasNext()){
        QString temp =i.next();
        qDebug("\t%s:%s",temp.toStdString().c_str(),m.property[temp].toStdString().c_str());
    }
}
