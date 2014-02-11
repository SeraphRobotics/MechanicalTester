#ifndef UTIL_H
#define UTIL_H

#include <QFile>
#include <QTextStream>

#include "nmotion.h"
#include "npath.h"
#include "../material.h"

void testdump(NPath h,QString filename);
void test(NPath h);
void test(QString filename,NPath n);
void printNPath(QString str, NPath* n);
void printState(QString str,State s);
void printMaterial(Material m);
#endif // UTIL_H
