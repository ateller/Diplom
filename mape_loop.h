#ifndef MAPE_LOOP_H
#define MAPE_LOOP_H

#include <QObject>
#include <QList>
#include "knowledge.h"

//Это штука, которая всем управляет, у нее есть база данных и функции петли

#define NUM_OF_CLASSES 1

struct to_execute {int id; QList<parameter> operation; int timer; QList<post_cond> post;};

struct class_list_el {dev_parameters dev; QList<QList<history_value>> hist; QList<int> deltas;};

struct out_of_tol {int dev_index; int par_index; int delta;};

struct splited{class_list_el el; int delta;};

struct class_list{QList<class_list_el> list; int delta;};

struct applicable_rule{int id; int index; rule r; int delta; QList<post_cond> post;};

struct generated_rule{int id; rule r; post_state post; int delta;};

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
    generated_rule generate_rule(class_list* temp);
    QList<splited> split(record);
    int prognose_distance(QList<post_cond> post, int time, QList<to_execute> additional);
private:
    bool uses(int id_1, QList<parameter> operation, int id_2, parameter p);
    bool check_par(int id, parameter p);
signals:
    void system_update();
    void monitor_completed();
    void analysis_completed();
    void plan_completed(int);
    void executed(int);
    void rule_generated(int id, int i);
public slots:
    void loop();
};

device* new_device(int type);
bool compare_cl(const class_list l1, const class_list l2);
bool compare_out(const out_of_tol l1, const out_of_tol l2);

#endif // MAPE_LOOP_H
