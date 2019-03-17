#ifndef GOAL_H
#define GOAL_H

#include <QObject>
#include <knowledge.h>

struct goal_param {parameter par; bool doesnt_care;};
struct goal_comp{device *pointer; QList<goal_param> list;};

class goal : public QObject
//Модель цели
{
    Q_OBJECT
public:
    goal();
    //goal(QList<device*> source);
    QList<goal_comp> env_goal;
    QList<goal_comp> sys_goal;
    void add_to_list(QList<goal_comp> *l, device* d);
    void add(device*);
public slots:
    void update(int i, QString par, QString new_val, bool list_type);
signals:
    added(bool);
};

#endif // GOAL_H
