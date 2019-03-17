#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QList>
#include <QString>

#define TEMPERATURE 0
#define ON_OFF 1
#define PERCENT 2
#define AREA 3

//Объявлена всякаяя абстрактность для девайсов

class device;

struct parameter {int index; int value; int type;};
//Имя и значение в текстовом виде

class device : public QObject
//Устройства. Сенсоры, эффекторы
{
    Q_OBJECT
public:
    QList<parameter> get_list();
    virtual QList<QString> get_names() = 0;
    virtual QList<bool> get_changeables() = 0;
    QString name;
    bool broken = 0;
    virtual void to_be_controlled(int p, int new_val) = 0;
protected:
    QList<parameter> list;
public slots:
    virtual void update() = 0;
    void set_broken(bool b);
signals:
    void updated();

};

#endif // DEVICE_H
