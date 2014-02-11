#ifndef JSCMOTION_H
#define JSCMOTION_H

#include <QObject>
#include "coordinatedmotion.h"
#include <QtScript>
#include "jsnpath.h"

// Used only for the runScipt function in virtualmachines
class JsCmotion : public QObject
{
    Q_OBJECT

public:
    /**
     * default constructor for the Javascript interface for a coordinated motion class
     */
    JsCmotion();

    /**
     * set the coordinated motion instance to be used
     */
    void setCmotion(CoordinatedMotion* cmotion);
    /**
     * used to validate the interface as being connected to a CoordinatedMotion object
     */
    bool hasCM;

public slots:

    /**
     * used by the Qt System to return the number of axis in javascript
     */
    int getNumberOfAxes();


    /**
     * used by the Qt System to convert an array to a relative NPath and execute it
     */
    void moveAlongPath(QScriptValue points);

private:
    CoordinatedMotion* cmotion_;

};

#endif // JSCMOTION_H
