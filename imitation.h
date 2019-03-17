#ifndef IMITATION_H
#define IMITATION_H

#include <sensor.h>
#include <effector.h>

//По идее должна считать физику и моделировать состояние системы на следующем шаге

class imitation
{
public:
    imitation();
    void effect(effector* eff);
    void sense(sensor* sen);
    void set(QByteArray);
    QByteArray* get();
private:
    float temperature;
    float humidity;
    float out_t;
    float v;
    //const float c_air = 1.007;
    //const float p_air = 1.16;
};

#endif // IMITATION_H
