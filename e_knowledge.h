#ifndef E_KNOWLEDGE_H
#define E_KNOWLEDGE_H

#include <QObject>
#include <sensor.h>

class e_knowledge : public QObject
//Модель окружения
{
    Q_OBJECT
public:
    e_knowledge();
    void add(sensor* s);
    QList<component_parametres> env_param;
    void clear();
};

#endif // E_KNOWLEDGE_H
