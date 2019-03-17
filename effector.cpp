#include "effector.h"

void effector::add_rule(rule r)
{
    ruleset += r;
}

void effector::delete_rule(int i)
{
    ruleset.removeAt(i);
}

heater::heater()
{
    flow_t = 20;
    enabled = 0;
    update();
}

void heater::update()
{
    parameter temp;
    list.clear();

    temp.index = 0;
    temp.value = flow_t;
    temp.type = TEMPERATURE;
    list += temp;

    temp.index = 1;
    temp.value = enabled;
    temp.type = ON_OFF;
    list += temp;
}

void heater::to_be_controlled(int p, int new_val)
{
    switch (p) {
    case 0:
        flow_t = new_val;
        break;
    case 1:
        enabled = new_val;
        break;
    }
}

void heater::exec_rule(QList<parameter> operation)
{
    parameter temp;
    foreach (temp, operation) {
        if(temp.index)
        {
            enabled = temp.value;
        }
        else
        {
            switch (temp.type) {
            case INCREASE:
                flow_t += temp.value;
                break;
            case DECREASE:
                flow_t -= temp.value;
                break;
            case ASSIGN:
                flow_t = temp.value;
                break;
            }
            if(flow_t < 0)
                flow_t = 0;
            if(flow_t > 50)
                flow_t = 50;
        }
    }
}

int heater::get_type()
{
    return HEATER;
}

int heater::effect()
{
    if(enabled)
        return flow_t;
    else return 0;
}

QList<QString> heater::get_names()
{
    QList<QString> l;
    l.append("Flow temperature");
    l.append("Heat enabled");
    return l;
}

QList<bool> heater::get_changeables()
{
    QList<bool> l;
    l.append(true);
    l.append(true);
    return l;
}

window::window()
{
    size = 1;
    percent_opened = 0;
    update();
}

void window::update()
{
    parameter temp;
    list.clear();

    temp.index = 0;
    temp.value = percent_opened;
    temp.type = PERCENT;
    list += temp;

    temp.index = 1;
    temp.value = size;
    temp.type = AREA;
    list += temp;
}

void window::to_be_controlled(int p, int new_val)
{
    switch (p) {
    case 0:
        percent_opened = new_val;
        break;
    case 1:
        size = new_val;
        break;
    }
}

void window::exec_rule(QList<parameter> operation)
{
    parameter temp = operation[0];
    switch (temp.type) {
    case INCREASE:
         percent_opened += temp.value;
         break;
    case DECREASE:
         percent_opened -= temp.value;
         break;
    case ASSIGN:
         percent_opened = temp.value;
         break;
    }
    if(percent_opened < 0)
        percent_opened = 0;
    if(percent_opened > 100)
        percent_opened = 50;
}

int window::get_type()
{
    return WINDOW;
}

float window::effect()
{
    return (percent_opened/100.0) * size;
}

QList<QString> window::get_names()
{
    QList<QString> l;
    l.append("Percent opened");
    l.append("Size");
    return l;
}

QList<bool> window::get_changeables()
{
    QList<bool> l;
    l.append(true);
    l.append(false);
    return l;
}
