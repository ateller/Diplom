#ifndef KNOWLEDGE_H
#define KNOWLEDGE_H

#include <QObject>
#include <sensor.h>
#include <effector.h>
#include <QFile>

//База данных. Модель s, модель d. Для каждого реального параметра целевое значение


struct dev_parametres{int id; QList<parameter> par;};
struct goal{int index; int value; bool not_care;};
struct history_value {int value; int static_period;};
struct record{device* pointer; QList <QString> names; dev_parametres dev; QList<goal> goal_model; QList <history_value> history;};

class knowledge: public QObject
{
Q_OBJECT
public:
    int id_counter;
    int distance();
    int distance(QList<parameter> one, QList <parameter> two);
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
    void save(QFile *f);
    void save_record (record, QDataStream *);
    record import_record (QDataStream*);
    int import_from_file(QFile* f);
    int loops_counter;
public slots:
    void update_goal(int id, int par, int new_val);
signals:
    void added(int);
};

#endif // KNOWLEDGE_H
