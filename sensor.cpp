#include "sensor.h"

thermometer::thermometer()
{
    temperature = 20;
    update();
}

void thermometer::update()
{
    parameter temp;
    list.clear();

    temp.index = 0;
    temp.value.i = temperature;
    temp.type = TEMPERATURE;
    list += temp;
}

void thermometer::to_be_controlled(int p, val new_val)
{
    temperature = new_val.i;
}

void thermometer::sense(int t)
{
    temperature = t;
}

QList<par_class> thermometer::get_classes()
{
    QList<par_class> list;
    par_class temp;

    temp.index = 0;
    temp.classes.append(TEMPERATURE);

    list.append(temp);

    return list;
}

QList<QString> thermometer::get_names()
{
    QList<QString> l;
    l.append("Temperature");
    return l;
}

QList<bool> thermometer::get_changeables()
{
    QList<bool> l;
    l.append(true);
    return l;
}

int thermometer::get_type()
{
    return THERMOMETER;
}
