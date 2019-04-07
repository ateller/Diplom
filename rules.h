#include <QString>
#include <device.h>

#ifndef RULES_H
#define RULES_H

//Описание правил

#define INCREASE 0
#define DECREASE 1
#define ASSIGN 2

//В операциях тип отвечает за тип действия

#define LESS 0
#define LARGER 1
#define EQUAL 2

struct condition{int dev_id; parameter p;};

//Здесь тип отвечает за тип условия

struct rule{QList<condition> pre; QList<parameter> operation; int period; int last_use;QList<condition> post;};

#endif // RULES_H

