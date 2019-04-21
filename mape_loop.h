#ifndef MAPE_LOOP_H
#define MAPE_LOOP_H

#include <QObject>
#include <QList>
#include "knowledge.h"

//Это штука, которая всем управляет, у нее есть база данных и функции петли

#define NUM_OF_CLASSES 1

struct to_execute {int id; QList<parameter> operation; int timer; QList<post_cond> post;};

struct class_list_el {dev_parameters dev; QList<QList<history_value>> hist;};

struct splited{class_list_el el; int delta;};

struct class_list{QList<class_list_el> list; int delta;};

struct applicable_rule{int id; int index; rule r; int delta; QList<post_cond> post;};

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
    int import_knowledge(QFile *f);
    rule* generate_rule();//Дописать
    QList<splited> split(record);
private:
    bool uses(int id_1, QList<parameter> operation, int id_2, parameter p);
    bool check_par(int id, parameter p);
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
bool compare_cl(const class_list l1, const class_list l2);

#endif // MAPE_LOOP_H
