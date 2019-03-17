#include "knowledge.h"

knowledge::knowledge()
{
    loops_counter = 0;
}

void knowledge::add(device *s, int id)
{
    record temp;
    parameter temp_p;
    temp.pointer = s;
    temp.dev.id = id;
    temp.names = s->get_names();
    temp.dev.par = s->get_list();
    QList <bool> c = s->get_changeables();
    foreach(temp_p, temp.dev.par)
    {
        if (!c[temp_p.index]) continue;
        goal temp_g;
        temp_g.value = temp_p.value;
        temp_g.index = temp_p.index;
        temp_g.not_care = 0;
        temp.goal_model += temp_g;
    }
    if (id < 0)
        env_model += temp;
    else
        sys_model += temp;
    emit added(id);
}

void knowledge::update_goal(int id, int par, int new_val)
{
    if (id < 0)
        env_model[indexof(id)].goal_model[par].value = new_val;
    else
        sys_model[indexof(id)].goal_model[par].value = new_val;
}

void knowledge::goal_ignore(int id, int par, bool not_care)
{
    if (id < 0)
        env_model[indexof(id)].goal_model[par].not_care = not_care;
    else
        sys_model[indexof(id)].goal_model[par].not_care = not_care;
}

void knowledge::upd()
{
    loops_counter++;
    QList<record>::iterator i = env_model.begin();
    for(; i != env_model.end(); i++)
    {
        (*i).dev.par = (*i).pointer->get_list();
    }
    i = sys_model.begin();
    for(; i != sys_model.end(); i++)
    {
        (*i).dev.par = (*i).pointer->get_list();
    }
}

device* knowledge::get_device(int id)
{
    record temp;
    if (id < 0)
    {
        foreach (temp, env_model) {
            if(temp.dev.id == id)
                return temp.pointer;
        }
    }
    else
    {
        foreach (temp, sys_model) {
            if(temp.dev.id == id)
                return temp.pointer;
        }
    }
    return NULL;
}

int knowledge::indexof(effector *e)
{
    int i = 0;
    record temp;
    foreach (temp, sys_model) {
        if (temp.pointer == e)
            return i;
    }
    return -1;
}
int knowledge::indexof(sensor *s)
{
    int i = 0;
    record temp;
    foreach (temp, env_model) {
        if (temp.pointer == s)
            return i;
    }
    return -1;
}

int knowledge::indexof(int id)
{
    if(id < 0)
    {
        int i = 0;
        record temp;
        foreach (temp, env_model) {
            if (temp.dev.id == id)
                return i;
            i++;
        }
        return -1;
    }
    else
    {
        int i = 0;
        record temp;
        foreach (temp, sys_model) {
            if (temp.dev.id == id)
                return i;
            i++;
        }
        return -1;
    }
}

void knowledge::save(QFile* f)
{

}

int knowledge::import_from_file(QFile* f)
{
    return 0;
}

int knowledge::distance()
{
    int count = 0;
    int sum = 0;
    record temp_comp;
    foreach(temp_comp, env_model)
    {
        goal temp;
        foreach(temp, temp_comp.goal_model)
        {
            if(!temp.not_care) {
                parameter p = temp_comp.dev.par[temp.index];
                switch (p.type) {
                int delta_t, delta_a, delta_p;
                case TEMPERATURE:
                    delta_t = temp.value - p.value;
                    if(delta_t < 0) delta_t = - delta_t;
                    if(delta_t > 100) delta_t = 100;
                    sum+=delta_t;
                    break;
                case ON_OFF:
                    if(temp.value != p.value)
                        sum+=100;
                case PERCENT:
                    delta_p = temp.value - p.value;
                    if(delta_p < 0) delta_p = - delta_p;
                    sum+=delta_p;
                    break;
                case AREA:
                    delta_a = temp.value - p.value;
                    if(delta_a < 0) delta_a = - delta_a;
                    delta_a *= 50;
                    if(delta_a > 100) delta_a = 100;
                    sum+=delta_a;
                }
                count++;
            }
        }
    }
    foreach(temp_comp, sys_model)
    {
        goal temp;
        foreach(temp, temp_comp.goal_model)
        {
            if(!temp.not_care) {
                parameter p = temp_comp.dev.par[temp.index];
                switch (p.type) {
                int delta_t, delta_a, delta_p;
                case TEMPERATURE:
                    delta_t = temp.value - p.value;
                    if(delta_t < 0) delta_t = - delta_t;
                    if(delta_t > 100) delta_t = 100;
                    sum+=delta_t;
                    break;
                case ON_OFF:
                    if(temp.value != p.value)
                        sum+=100;
                case PERCENT:
                    delta_p = temp.value - p.value;
                    if(delta_p < 0) delta_p = - delta_p;
                    sum+=delta_p;
                    break;
                case AREA:
                    delta_a = temp.value - p.value;
                    if(delta_a < 0) delta_a = - delta_a;
                    delta_a *= 50;
                    if(delta_a > 100) delta_a = 100;
                    sum+=delta_a;
                }
                count++;
            }
        }
    }
    return sum/count;
}
