#ifndef S_KNOWLEDGE_H
#define S_KNOWLEDGE_H

#include <QObject>
#include <effector.h>

class s_knowledge : public QObject
//Модель системы
{
    Q_OBJECT
public:
    s_knowledge();
    void add(effector* s);
    QList<component_parametres> sys_param;
    void clear();
};

#endif // S_KNOWLEDGE_H
