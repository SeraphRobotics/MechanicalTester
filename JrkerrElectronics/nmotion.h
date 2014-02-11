#ifndef NMOTION_H
#define NMOTION_H
#include <QString>
#include <QMap>
#include <QList>
#include <QVector>
#include "npath.h"

typedef unsigned char byte;


namespace NP {
typedef NPath HzPath;

int SetPathParams(int statesize, int freq, int nbuf,
                  QMap<int,int> motorMap, int groupaddr, int leaderaddr,
                  QMap<int,double> scale, QString* error_string );
float InitPath(QString* error_string);
int DlPathPoints(QString* error_string);
int AddStates(HzPath statelist);
void ClearStates();
int DlSev(QString* error_string, int pointcount, byte tempId, long int pointBuffer[]);
int GetNextPoints(int stateVariableIndex,long int pointBuffer[],int bufferLength);
int StartPathMode(QString* error_string);

void resetStates();


/**
 * returns an absolute hzpath from the npath
 * uses the npath in relative mode internally
 **/
HzPath toHzPath(NPath* npath, int startIndex);

void testdump(HzPath h, QString file);

}
#endif // NMOTION_H
