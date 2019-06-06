#ifndef EFFECTOR_H
#define EFFECTOR_H

#include <device.h>
#include <rules.h>
#include <math.h>

class effector : public device
{
Q_OBJECT
public:
    QList<rule> ruleset;
    void add_rule(rule r, bool from_file);
    void delete_rule(int i);
    virtual void exec_rule(QList<parameter> operation) = 0;
    int r_id_counter;
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
    int wind_power;
    float vel;
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
    int percent_opened;
    float h;
    float mu;
};

#endif // EFFECTOR_H
