#ifndef EFFECTOR_H
#define EFFECTOR_H

#include <device.h>
#include <rules.h>
#include <math.h>

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

class heater : public effector
{
Q_OBJECT
public:
    heater();
    struct result {float vol; int t;};
    void update();
    QList<QString> get_names();
    QList<bool> get_changeables();
    void to_be_controlled(int p, val new_val);
    void exec_rule(QList<parameter> operation);
    int get_type();
    QList <par_class> get_classes();
    result effect();
private:
    int flow_t;
    //На сколько градусов поток воздуха теплее, чем в комнате
    int wind_power;
    //Скорость вращения вентилятора
    float vel;
    //Максимальная скорость потока воздуха
};

class window : public effector
{
Q_OBJECT
public:
    struct result {float h; double f;};
    window();
    void update();
    QList<QString> get_names();
    QList<bool> get_changeables();
    void to_be_controlled(int p, val new_val);
    void exec_rule(QList<parameter> operation);
    int get_type();
    QList <par_class> get_classes();
    result effect();
private:
    float size;
    //Площадь окна в кв.метрах
    int percent_opened;
    //На сколько процентов открыто
    float h;
    //Высота от пола, на которой находится окно
    float mu;
    //Коэффициент расхода воздуха
};

#endif // EFFECTOR_H
