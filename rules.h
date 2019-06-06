#include <QString>
#include <device.h>

#ifndef RULES_H
#define RULES_H

#define INCREASE 0
#define DECREASE 1
#define ASSIGN 2

#define LESS 0
#define LARGER 1
#define EQUAL 2
#define SAME 2

struct condition{int dev_id; parameter p;};

struct post_cond{int dev_id; parameter p; int time;};

struct rule{int r_id; QList<condition> pre; QList<parameter> operation; int period; int last_use; double failure_rate;};

#endif // RULES_H

