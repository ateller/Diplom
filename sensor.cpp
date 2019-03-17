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
    temp.value = temperature;
    temp.type = TEMPERATURE;
    list += temp;
}

void thermometer::to_be_controlled(int p, int new_val)
{
    temperature = new_val;
}

void thermometer::sense(int t)
{
    temperature = t;
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
