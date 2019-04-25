#include "effector.h"

void effector::add_rule(rule r)
{
    r.r_id = r_id_counter;
    ruleset += r;
    r_id_counter++;
}

void effector::delete_rule(int i)
{
    ruleset.removeAt(i);
}

heater::heater()
{
    flow_t = 20;
    wind_power = 0;
    vel = 2;
    r_id_counter = 0;
    update();
}

void heater::update()
{
    parameter temp;
    list.clear();

    temp.index = 0;
    temp.value.i = flow_t;
    temp.type = TEMPERATURE;
    list += temp;

    temp.index = 1;
    temp.value.i = wind_power;
    temp.type = PERCENT;
    list += temp;

    temp.index = 2;
    temp.value.f = vel;
    temp.type = F_SIZE;
    list += temp;
}

void heater::to_be_controlled(int p, val new_val)
{
    switch (p) {
    case 0:
        flow_t = new_val.i;
        break;
    case 1:
        wind_power = new_val.i;
        break;
    case 2:
        vel = new_val.f;
        break;
    }
}

void heater::exec_rule(QList<parameter> operation)
{
    parameter temp;
    foreach (temp, operation) {
        if(temp.index)
        {
            switch (temp.type) {
            case INCREASE:
                wind_power += temp.value.i;
                break;
            case DECREASE:
                wind_power -= temp.value.i;
                break;
            case ASSIGN:
                wind_power = temp.value.i;
                break;
            }
            if(wind_power < 0)
                wind_power = 0;
            if(wind_power > 100)
                wind_power = 100;
        }
        else
        {
            switch (temp.type) {
            case INCREASE:
                flow_t += temp.value.i;
                break;
            case DECREASE:
                flow_t -= temp.value.i;
                break;
            case ASSIGN:
                flow_t = temp.value.i;
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

heater::result heater::effect()
{
    result r;
    r.vol = (vel/100) * wind_power;
    r.t = flow_t;
    return r;
}

QList<par_class> heater::get_classes()
{
    QList<par_class> list;
    par_class temp;

    temp.index = 0;
    temp.classes.append(TEMPERATURE);
    list.append(temp);

    temp.classes.clear();

    temp.index = 1;
    temp.classes.append(TEMPERATURE);
    list.append(temp);

    return list;
}

QList<QString> heater::get_names()
{
    QList<QString> l;
    l.append("Flow temperature");
    l.append("Wind power");
    l.append("Max wind speed");
    return l;
}

QList<bool> heater::get_changeables()
{
    QList<bool> l;
    l.append(true);
    l.append(true);
    l.append(false);
    return l;
}

window::window()
{
    size = 1;
    percent_opened = 0;
    h = 1.5;
    mu = static_cast<float> (0.6);
    r_id_counter = 0;
    update();
}

void window::update()
{
    parameter temp;
    list.clear();

    temp.index = 0;
    temp.value.i = percent_opened;
    temp.type = PERCENT;
    list += temp;

    temp.index = 1;
    temp.value.f = size;
    temp.type = F_SIZE;
    list += temp;

    temp.index = 2;
    temp.value.f = h;
    temp.type = F_SIZE;
    list += temp;

    temp.index = 3;
    temp.value.f = mu;
    temp.type = COEFF;
    list += temp;
}

void window::to_be_controlled(int p, val new_val)
{
    switch (p) {
    case 0:
        percent_opened = new_val.i;
        break;
    case 1:
        size = new_val.f;
        break;
    case 2:
        h = new_val.f;
        break;
    case 3:
        mu = new_val.f;
        break;
    }
}

void window::exec_rule(QList<parameter> operation)
{
    parameter temp = operation[0];
    switch (temp.type) {
    case INCREASE:
         percent_opened += temp.value.i;
         break;
    case DECREASE:
         percent_opened -= temp.value.i;
         break;
    case ASSIGN:
         percent_opened = temp.value.i;
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

window::result window::effect()
{
    result e;
    e.h = h;

    double sinus = sin(M_PI_2 * (percent_opened / 100.0));

    e.f = static_cast<double> (mu) * sinus * static_cast<double> (size);
    return e;
}

QList<par_class> window::get_classes()
{
    QList<par_class> list;
    par_class temp;

    temp.index = 0;
    temp.classes.append(TEMPERATURE);
    list.append(temp);

    return list;
}

QList<QString> window::get_names()
{
    QList<QString> l;
    l.append("Percent opened");
    l.append("Size");
    l.append("Altitude");
    l.append("Air coefficient");
    return l;
}

QList<bool> window::get_changeables()
{
    QList<bool> l;
    l.append(true);
    l.append(false);
    l.append(false);
    l.append(false);
    return l;
}
