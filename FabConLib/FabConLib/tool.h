#ifndef TOOL_H
#define TOOL_H

#include <QObject>
#include <QMap>
#include "bay.h"

class QDomNode;

class Tool : public QObject
{
    Q_OBJECT

public:

    /**
     * Tool is a container for the bays
     */
    Tool();

    Tool(const QDomNode& sourceDomNode);

    ~Tool();


public slots:

    Bay* getBay(int bayID);

    QString getName()const;



private :
        QMap<int,Bay*> baymap_;
        QString name_;

};

#endif // TOOL_H
