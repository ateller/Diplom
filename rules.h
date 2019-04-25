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
#define SAME 2
//В посте если параметр не изменится

struct condition{int dev_id; parameter p;};

//Здесь тип отвечает за тип условия

struct post_cond{int dev_id; parameter p; int time;};
//Значение, на сколько изменится и время, за которое это произойдет

struct rule{int r_id; QList<condition> pre; QList<parameter> operation; int period; int last_use;};

#endif // RULES_H

