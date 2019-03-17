#ifndef EFFECTOR_H
#define EFFECTOR_H

#include <device.h>
#include <rules.h>

 //Абстрактность и конкретность для эффекторов

class effector : public device
{
Q_OBJECT
public:
    QList<rule> ruleset;
    void add_rule(rule r);
    void delete_rule(int i);
    virtual void exec_rule(QList<parameter> operation) = 0;
};

struct to_execute {effector* subj; QList<parameter> operation;};

class heater : public effector
{
Q_OBJECT
public:
    heater();
    void update();
    QList<QString> get_names();
    QList<bool> get_changeables();
    void to_be_controlled(int p, int new_val);
    void exec_rule(QList<parameter> operation);
    int effect();
private:
    int flow_t;
    //На сколько градусов поток воздуха теплее, чем в комнате
    bool enabled;
};

class window : public effector
{
Q_OBJECT
public:
    window();
    void update();
    QList<QString> get_names();
    QList<bool> get_changeables();
    void to_be_controlled(int p, int new_val);
    void exec_rule(QList<parameter> operation);
    float effect();
private:
    float size;
    //Площадь окна в кв.метрах
    int percent_opened;
    //На сколько процентов открыто
};

#endif // EFFECTOR_H
