#include "e_knowledge.h"

e_knowledge::e_knowledge()
{

}

void e_knowledge::add(sensor *s)
{
    env_param += s->get_list();
}

void e_knowledge::clear()
{
    env_param.clear();
}
