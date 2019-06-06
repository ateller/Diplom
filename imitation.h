#ifndef IMITATION_H
#define IMITATION_H

#include <sensor.h>
#include <effector.h>
#include <QDialog>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <math.h>

class imitation: public QObject
{
Q_OBJECT
public:
    imitation();
    void effect(effector* eff);
    void sense(sensor* sen);
    void calculate_physics();
    void set(QByteArray);
    QByteArray* get();
    QWidget* get_ctrl_pointer();
private:
    double temperature;
    double humidity;
    double out_t;
    double v;
    double air_h;
    QWidget* i_control;
private slots:
    void upd();
    void change_val();

};

#endif // IMITATION_H
