#ifndef MAPE_LOOP_H
#define MAPE_LOOP_H

#include <QObject>
#include <QList>
#include "knowledge.h"

//Это штука, которая всем управляет, у нее есть база данных, списки сенсоров, эффекторов, и функции петли

class mape_loop : public QObject
{
//Петля
    Q_OBJECT
public:
    mape_loop();
    void monitor();
    void analysis();
    QList<to_execute> ex_plan;
    void plan();
    void execute();
    int add_device(device* pointer);
    knowledge* k;
    int indexof(sensor*);
    int indexof(effector*);
    int indexof(int);
    int dist;
    int tolerance;
    bool must_adapt;
    int import_knowledge(QFile *f);
private:
signals:
    void system_update();
    void monitor_completed();
    void analysis_completed();
    void plan_completed(int);
    void executed(int);
public slots:
    void loop();
};

device* new_device(int type);

#endif // MAPE_LOOP_H
