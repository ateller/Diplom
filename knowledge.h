#ifndef KNOWLEDGE_H
#define KNOWLEDGE_H

#include <QObject>
#include <sensor.h>
#include <effector.h>
#include <QFile>

//База данных. Модель s, модель d. Для каждого реального параметра целевое значение

#define PEACE_TRESHOLD 0.01

struct dev_parameters{int id; QList<parameter> par;};
struct goal{int index; val value; bool not_care;};
struct history_value {val value; int cycle_number;};
struct history{int index; QList<history_value> series;};
struct record{device* pointer; QList <QString> names; dev_parameters dev; QList<goal> goal_model; QList<history>histories; QList<par_class> classes;};
struct intermed_dist{int sum; int count;};

struct post_state{QList<post_cond> post; int time;};
struct relation{val d1; val d2; int time;};
struct weighed_rel{relation r; int w;};

struct executing_rule{int id; QList<parameter> operation; int timer; int start_loop; QList<post_cond> post;};

class knowledge: public QObject
{
Q_OBJECT
public:
    int id_counter;
    int distance();
    int distance(QList<dev_parameters> one, QList<dev_parameters> two);
    int distance(int loop);
    intermed_dist distance(QList<parameter> one, QList <parameter> two);
    relation* correlate(QList<history_value> dep, int dep_type, int id, int index, int type, QList<int> cl, val d, bool whose);
    knowledge();
    ~knowledge();
    int add(device* s);
    QList<record> env_model;
    QList<record> sys_model;
    void upd();
    void goal_ignore(int id, int par, bool not_care);
    device* get_device(int id);
    int indexof(sensor* s);
    int indexof(effector* e);
    int indexof(int);
    void finish_execution(QList<executing_rule>::iterator);
    void save(QFile *f);
    void save_record (record, QDataStream *);
    record import_record (QDataStream*);
    int import_from_file(QFile* f);
    int loops_counter;
    int delta(parameter p, val g);
    QList<executing_rule> exec_rules;
    post_state create_postcond(int id, QList<parameter> operation);
    int prognose_distance(QList<post_cond> post, int time);
    bool same_class(QList<int>, QList<int>);
public slots:
    void update_goal(int id, int par, val new_val);
signals:
    void added(int);
private:
    void upd_history(record);
    bool is_peace(QList<history_value>::iterator i, int type);
    weighed_rel norm_rel(int start, int fin, val d_d, val d_i, int type_d, int type_i, bool whose, val must);
    double calc_k(val v, int t, val m);
    val apply_k(double k, int t, val v);
    val subtract(val what, val from, int type);
    double add_val(val what, int weight, int type);
    val avg(double sum, int count, int type);
    parameter get_post(parameter delta);
    void apply_post(QList<dev_parameters>*state, QList<post_cond> post, int time_before, int time);
};

template<typename v>
bool compare(v one, v two, int type)
{
    switch (type) {
    case LESS:
        if(one >= two)
            return false;
        break;
    case LARGER:
        if(one <= two)
            return false;
        break;
    case EQUAL:
        if(one != two)
            return false;
        break;
    }
    return true;
}

#endif // KNOWLEDGE_H
