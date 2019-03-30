#include "knowledge.h"

knowledge::knowledge()
{
    loops_counter = 0;
    id_counter = 1;
}

knowledge::~knowledge()
{
    QList<record>::iterator i = env_model.begin();
    for(; i != env_model.end(); i++)
    {
        delete (*i).pointer;
    }
    i = sys_model.begin();
    for(; i != sys_model.end(); i++)
    {
        delete (*i).pointer;
    }
}

int knowledge::add(device *s)
{
    record temp;
    parameter temp_p;
    temp.pointer = s;

    if(s->get_type() < 0) temp.dev.id = -id_counter;
    else temp.dev.id = id_counter;
    id_counter++;

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
    if (temp.dev.id < 0)
        env_model += temp;
    else
        sys_model += temp;
    emit added(temp.dev.id);
    return temp.dev.id;
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
    return nullptr;
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
    QDataStream str(f);

    str << loops_counter;
    str << env_model.size();

    foreach(record temp, env_model)
    {
        save_record(temp, &str);
    }

    str << sys_model.size();

    foreach(record temp, sys_model)
    {
        save_record(temp, &str);
        str << qobject_cast <effector*> (temp.pointer)->ruleset.size();
        foreach(rule temp_rule, qobject_cast <effector*> (temp.pointer)->ruleset)
        {
            str << temp_rule.pre.size();
            foreach (condition temp_c, temp_rule.pre)
            {
                str << temp_c.p.type;
                str << temp_c.p.index;
                str << temp_c.p.value;
                str << temp_c.dev_id;
            }
            str << temp_rule.operation.size();
            foreach (parameter temp_o, temp_rule.operation)
            {
                str << temp_o.type;
                str << temp_o.index;
                str << temp_o.value;
            }
            str << temp_rule.timer;
            str << temp_rule.period;
        }
    }
}

void knowledge::save_record(record temp, QDataStream* str)
{
    *str << temp.pointer->get_type();
    *str << temp.pointer->name;

    *str << temp.dev.par.size();
    foreach(parameter temp_p, temp.dev.par)
    {
        *str << temp_p.index;
        *str << temp_p.type;
        *str << temp_p.value;
    }

    *str << temp.goal_model.size();
    foreach (goal temp_g, temp.goal_model)
    {
        *str << temp_g.index;
        *str << temp_g.value;
        *str << temp_g.not_care;
    }

    *str << temp.history.size();
    foreach (history_value temp_h, temp.history)
    {
        *str << temp_h.value;
        *str << temp_h.static_period;
    }
}

int knowledge::import_from_file(QFile* f)
{
    QDataStream str(f);
    str >> loops_counter;

    int n;

    str >> n;

    for (int i = 0; i < n; i++) {
        if (f->atEnd()) return 1;
        env_model += import_record(&str);
    }

    str >> n;

    for (int i = 0; i < n; i++) {
        if (f->atEnd()) return 1;
        sys_model += import_record(&str);

        int k;

        if (f->atEnd()) return 1;

        str >> k;
        for (int j = 0; j < k; k++) {
            rule r;

            int m;
            str >> m; // .pre.size
            for (int l = 0; l < m; l++) {
                condition c;
                str >> c.p.type;
                str >> c.p.index;
                str >> c.p.value;
                str >> c.dev_id;
                r.pre.append(c);
            }

            str >> m; //.operation.size
            for (int l = 0; l < m; l++) {
                parameter o;
                str >> o.type;
                str >> o.index;
                str >> o.value;
                r.operation.append(o);
            }

            str >> r.timer;
            str >> r.period;

            qobject_cast <effector*>(sys_model.back().pointer)->add_rule(r);
        }
    }
    return 0;
}

record knowledge::import_record(QDataStream* str)
{
    record temp;
    int type;
    *str >> type;

    switch (type) {
    case THERMOMETER:
        temp.pointer = new thermometer;
        break;
    }

    temp.dev.id = -id_counter;
    id_counter++;

    *str >> temp.pointer->name;

    temp.names = temp.pointer->get_names();

    int k;
    *str >> k;
    for (int j = 0; j < k; j++) {
        parameter p;
        *str >> p.index;
        *str >> p.type;
        *str >> p.value;
        temp.dev.par.append(p);
    }

    *str >> k;

    for (int j = 0; j < k; j++) {
        goal g;
        *str >> g.index;
        *str >> g.value;
        *str >> g.not_care;
        temp.goal_model.append(g);
    }

    *str >> k;

    for (int j = 0; j < k; j++) {
        history_value h;
        *str >> h.value;
        *str >> h.static_period;
        temp.history.append(h);
    }
    return temp;
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
                int delta_t, delta_a, delta_p;
                switch (p.type) {
                case TEMPERATURE:
                    delta_t = temp.value - p.value;
                    if(delta_t < 0) delta_t = - delta_t;
                    if(delta_t > 100) delta_t = 100;
                    sum+=delta_t;
                    break;
                case ON_OFF:
                    if(temp.value != p.value)
                        sum+=100;
                    break;
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
                int delta_t, delta_a, delta_p;
                switch (p.type) {
                case TEMPERATURE:
                    delta_t = temp.value - p.value;
                    if(delta_t < 0) delta_t = - delta_t;
                    if(delta_t > 100) delta_t = 100;
                    sum+=delta_t;
                    break;
                case ON_OFF:
                    if(temp.value != p.value)
                        sum+=100;
                    break;
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
