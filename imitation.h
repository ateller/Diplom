#ifndef IMITATION_H
#define IMITATION_H

#include <sensor.h>
#include <effector.h>
#include <QDialog>
#include <QFormLayout>
#include <QDoubleSpinBox>

//По идее должна считать физику и моделировать состояние системы на следующем шаге

class imitation: public QObject
{
Q_OBJECT
public:
    imitation();
    void effect(effector* eff);
    void sense(sensor* sen);
    void set(QByteArray);
    QByteArray* get();
    QWidget* get_ctrl_pointer();
private:
    float temperature;
    float humidity;
    float out_t;
    float v;
    QWidget* i_control;
    //const float c_air = 1.007;
    //const float p_air = 1.16;
private slots:
    void upd();
    void change_val(double);

};

#endif // IMITATION_H
