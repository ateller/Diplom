#include "s_knowledge.h"

s_knowledge::s_knowledge()
{

}

void s_knowledge::add(effector *s)
{
    sys_param += s->get_list();
}

void s_knowledge::clear()
{
    sys_param.clear();
}
