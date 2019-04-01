#include "imitation.h"

imitation::imitation()
{
    temperature = 20;
    humidity = 0;
    out_t = 0;
    v = 30;

    i_control = new QWidget;
    QFormLayout* l = new QFormLayout;
    i_control->setLayout(l);

    QDoubleSpinBox* s = new QDoubleSpinBox;
    s->setValue(static_cast<double>(temperature));
    s->setProperty("index", QVariant(0));
    connect(s, SIGNAL(editingFinished()), this, SLOT(change_val()));
    l->addRow("Temperature", s);

    s = new QDoubleSpinBox;
    s->setValue(static_cast<double>(humidity));
    s->setProperty("index", QVariant(1));
    connect(s, SIGNAL(editingFinished()), this, SLOT(change_val()));
    l->addRow("Humidity", s);

    s = new QDoubleSpinBox;
    s->setValue(static_cast<double>(out_t));
    s->setProperty("index", QVariant(2));
    connect(s, SIGNAL(editingFinished()), this, SLOT(change_val()));
    l->addRow("Outdoor temperature", s);

    s = new QDoubleSpinBox;
    s->setValue(static_cast<double>(v));
    s->setProperty("index", QVariant(3));
    connect(s, SIGNAL(editingFinished()), this, SLOT(change_val()));
    l->addRow("Room volume", s);

    i_control->setFixedSize(300, i_control->sizeHint().height());
}

void imitation::effect(effector* eff)
{
    if(eff->get_type() == WINDOW)
    {
        float area = qobject_cast<window*>(eff)->effect();
        double a = static_cast<double>(area) * 0.7;
        temperature = (temperature * (v - a) + out_t * a)/v;
    }
    else if(eff->get_type() == HEATER)
    {
        int t = qobject_cast<heater*>(eff)->effect();
        if (t) {
            double a = 0.27;
            temperature = (temperature * (v - a) + t * a)/v;
        }
    }
    upd();
}

void imitation::sense(sensor* sen)
{
    if(qobject_cast<thermometer*>(sen) != nullptr)
    {
        if (!sen->broken)
            qobject_cast<thermometer*>(sen)->sense(static_cast<int>(temperature+0.5));
        return;
    }
}

void imitation::calculate_physics()
{
    temperature = out_t + (temperature - out_t)*exp(-5.0/(45*(3600)));
    upd();
}

QByteArray* imitation::get()
{
    QByteArray* arr = new QByteArray;
    QDataStream stream(arr, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    stream << temperature;
    stream << humidity;
    stream << out_t;
    stream << v;
    return arr;
}

QWidget *imitation::get_ctrl_pointer()
{
    return i_control;
}

void imitation::upd()
{
    QFormLayout* l = qobject_cast<QFormLayout*> (i_control->layout());

    qobject_cast<QDoubleSpinBox*>(l->itemAt(0,QFormLayout::FieldRole)->widget())->setValue(temperature);
    qobject_cast<QDoubleSpinBox*>(l->itemAt(1,QFormLayout::FieldRole)->widget())->setValue(humidity);
    qobject_cast<QDoubleSpinBox*>(l->itemAt(2,QFormLayout::FieldRole)->widget())->setValue(out_t);
    qobject_cast<QDoubleSpinBox*>(l->itemAt(3,QFormLayout::FieldRole)->widget())->setValue(v);
}

void imitation::change_val()
{
    double val = qobject_cast<QDoubleSpinBox*>(sender())->value();
    switch (sender()->property("index").value<int>()) {
    case 0:
        temperature = val;
        break;
    case 1:
        humidity = val;
        break;
    case 2:
        out_t = val;
        break;
    case 3:
        v = val;
        break;
    }
}

void imitation::set(QByteArray arr)
{
    QDataStream stream(&arr, QIODevice::ReadOnly);
    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    stream >> temperature;
    stream >> humidity;
    stream >> out_t;
    stream >> v;
}
