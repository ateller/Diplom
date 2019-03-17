#include "imitation.h"

imitation::imitation()
{
    temperature = 20;
    out_t = 0;
    v = 30;
}

void imitation::effect(effector* eff)
{
    if(qobject_cast<window*>(eff) != nullptr)
    {
        float area = qobject_cast<window*>(eff)->effect();
        float a = area * static_cast<float> (0.7);
        temperature = (temperature * (v - a) + out_t * a)/v;
        return;
    }
    if(qobject_cast<heater*>(eff) != nullptr)
    {
        int t = qobject_cast<heater*>(eff)->effect();
        if (t) {
            float a = static_cast<float> (0.27);
            temperature = (temperature * (v - a) + t * a)/v;
        }
        return;
    }
}

void imitation::sense(sensor* sen)
{
    if(qobject_cast<thermometer*>(sen) != nullptr)
    {
        if (!sen->broken)
            qobject_cast<thermometer*>(sen)->sense(static_cast<int>(temperature));
        return;
    }
}

QByteArray* imitation::get()
{
    QByteArray* arr = new QByteArray;
    QDataStream stream(arr, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << temperature;
    stream << humidity;
    stream << out_t;
    stream << v;
    return arr;
}

void imitation::set(QByteArray arr)
{
    QDataStream stream(&arr, QIODevice::ReadOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream >> temperature;
    stream >> humidity;
    stream >> out_t;
    stream >> v;
}
