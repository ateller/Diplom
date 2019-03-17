#ifndef SENSOR_H
#define SENSOR_H

#include <device.h>

class sensor : public device
{
Q_OBJECT
public:
};

class thermometer: public sensor
{
Q_OBJECT
public:
    thermometer();
    void update();
    void to_be_controlled(int p, int new_val);
    QList<QString> get_names();
    QList<bool> get_changeables();
    int get_type();
    void sense(int t);
private:
    int temperature;
};

#endif // SENSOR_H
