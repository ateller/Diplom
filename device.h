#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QList>
#include <QString>

#define TEMPERATURE 0 //int
#define ON_OFF 1 //bool
#define PERCENT 2 //int
#define F_SIZE 3 //float > 0
#define COEFF 4//float

#define THERMOMETER -1
#define HEATER 2
#define WINDOW 3

#define HUMIDITY 1;

class device;

union val { int i; float f; bool b;};

struct parameter {int index; val value; int type;};

struct par_class {int index; QList<int> classes;};

class device : public QObject
{
    Q_OBJECT
public:
    QList<parameter> get_list();
    virtual QList<QString> get_names() = 0;
    virtual QList<bool> get_changeables() = 0;
    virtual int get_type() = 0;
    virtual QList<par_class> get_classes() = 0;
    QString name;
    bool broken = 0;
    virtual void to_be_controlled(int p, val new_val) = 0;
protected:
    QList<parameter> list;
public slots:
    virtual void update() = 0;
    void set_broken(bool b);
signals:
    void updated();

};

#endif // DEVICE_H
