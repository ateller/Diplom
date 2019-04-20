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
    temp.classes = s->get_classes();
    QList <bool> c = s->get_changeables();
    foreach(temp_p, temp.dev.par)
    {
        if (!c[temp_p.index]) continue;

        history h;
        history_value h_v;

        h_v.cycle_number = loops_counter;
        h_v.value = temp_p.value;

        h.index = temp_p.index;
        h.series.append(h_v);

        temp.histories.append(h);

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

void knowledge::update_goal(int id, int par, val new_val)
{
    if (id < 0)
        env_model[indexof(id)].goal_model[par].value = new_val;
    else
        sys_model[indexof(id)].goal_model[par].value = new_val;
}

void knowledge::upd_history(record temp)
{
    QList<history>::iterator j = (temp).histories.begin();
    for(; j != (temp).histories.end(); j++)
    {
        switch (temp.dev.par[(*j).index].type) {
            case TEMPERATURE:
            case PERCENT:
                if((*j).series.back().value.i == (temp).dev.par[(*j).index].value.i) continue;
                break;
            case ON_OFF:
                if((*j).series.back().value.b == (temp).dev.par[(*j).index].value.b) continue;
                break;
            case COEFF:
            case F_SIZE:
                if((*j).series.back().value.f == (temp).dev.par[(*j).index].value.f) continue;
                break;
        }
        history_value h_v;
        h_v.cycle_number = loops_counter;
        h_v.value = (temp).dev.par[(*j).index].value;
        (*j).series.append(h_v);
    }
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
    QList<QList<history_value>>::iterator t;
    for(; i != env_model.end(); i++)
    {
        (*i).dev.par = (*i).pointer->get_list();
        upd_history(*i);
    }
    i = sys_model.begin();
    for(; i != sys_model.end(); i++)
    {
        (*i).dev.par = (*i).pointer->get_list();
        upd_history(*i);
    }
    QList<executing_rule>::iterator r = exec_rules.begin();
    for (; r!= exec_rules.end(); r++) {
        (*r).timer--;
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

void knowledge::finish_execution(QList<executing_rule>::iterator i)
{
    exec_rules.erase(i);
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
                str << temp_c.p.value.f;
                str << temp_c.dev_id;
            }
            str << temp_rule.operation.size();
            foreach (parameter temp_o, temp_rule.operation)
            {
                str << temp_o.type;
                str << temp_o.index;
                str << temp_o.value.f;
            }
            str << temp_rule.last_use;
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
        *str << temp_p.value.f;
    }

    *str << temp.goal_model.size();
    foreach (goal temp_g, temp.goal_model)
    {
        *str << temp_g.index;
        *str << temp_g.value.f;
        *str << temp_g.not_care;
    }

    *str << temp.histories.size();

    foreach(history temp_h, temp.histories)
    {
        *str << temp_h.index;
        *str << temp_h.series.size();
        foreach (history_value h_v, temp_h.series)
        {
            *str << h_v.value.f;
            *str << h_v.cycle_number;
        }
    }

    *str << temp.classes.size();

    foreach(par_class temp_cl, temp.classes)
    {
        *str << temp_cl.index;
        *str << temp_cl.classes.size();
        foreach (int temp_class, temp_cl.classes) *str << temp_class;
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
                str >> c.p.value.f;
                str >> c.dev_id;
                r.pre.append(c);
            }

            str >> m; //.operation.size
            for (int l = 0; l < m; l++) {
                parameter o;
                str >> o.type;
                str >> o.index;
                str >> o.value.f;
                r.operation.append(o);
            }

            str >> r.last_use;
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
        temp.dev.id = -id_counter;
        break;
    case HEATER:
        temp.pointer = new heater;
        temp.dev.id = id_counter;
        break;
    case WINDOW:
        temp.pointer = new window;
        temp.dev.id = id_counter;
        break;
    }
    id_counter++;

    *str >> temp.pointer->name;

    temp.names = temp.pointer->get_names();

    int k;
    *str >> k;
    for (int j = 0; j < k; j++) {
        parameter p;
        *str >> p.index;
        *str >> p.type;
        *str >> p.value.f;
        temp.dev.par.append(p);
    }

    *str >> k;

    for (int j = 0; j < k; j++) {
        goal g;
        *str >> g.index;
        *str >> g.value.f;
        *str >> g.not_care;
        temp.goal_model.append(g);
    }

    *str >> k;

    for(int j = 0; j < k; j++)
    {
        history temp_h;
        *str >> temp_h.index;

        int m;
        *str >> m;

        for(int l = 0; l < m; l++)
        {
            history_value h_v;
            *str >> h_v.value.f;
            *str >> h_v.cycle_number;
            temp_h.series.append(h_v);
        }

        temp.histories.append(temp_h);

    }

    *str >> k;

    for(int j = 0; j < k; j++)
    {
        par_class temp_cl;
        *str >> temp_cl.index;

        int m;
        *str >> m;

        for(int l = 0; l < m; l++)
        {
            int temp_class;
            *str >> temp_class;
            temp_cl.classes += temp_class;
        }

        temp.classes.append(temp_cl);
    }

    return temp;
}

int knowledge::delta(parameter p, val g)
{
    int delta = 0;
    switch (p.type) {
    case TEMPERATURE:
        delta = g.i - p.value.i;
        if(delta < 0) delta = - delta;
        if(delta > 100) delta = 100;
        break;
    case ON_OFF:
        if(g.b != p.value.b)
            delta = 100;
        break;
    case PERCENT:
        delta = g.i - p.value.i;
        if(delta < 0) delta = - delta;
        break;
    case COEFF:
        delta = static_cast<int>(round(static_cast<double>(g.f - p.value.f) * 50));
        if (delta < 0) delta = - delta;
        break;
    case F_SIZE:
        delta = static_cast<int>(round(static_cast<double>(g.f - p.value.f)));
        if(delta < 0) delta = - delta;
        delta *= 50;
        if(delta > 100) delta = 100;
    }
    return delta;
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
                sum += delta(temp_comp.dev.par[temp.index], temp.value);
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
                sum += delta(temp_comp.dev.par[temp.index], temp.value);
                count++;
            }
        }
    }
    if(count) return sum/count;
    else return 0;
}

int knowledge::distance(QList<dev_parameters> one, QList<dev_parameters> two)
{
    int sum = 0, count = 0;
    if(one.size() < two.size())
    {
        foreach(dev_parameters temp, one)
        {
            foreach(dev_parameters temp2, two)
            {
                if(temp.id == temp2.id)
                {
                    intermed_dist d = distance(temp.par, temp2.par);
                    sum+=d.sum;
                    count+=d.count;
                    break;
                }
            }
        }
    }
    else
    {
        foreach(dev_parameters temp, two)
        {
            foreach(dev_parameters temp2, one)
            {
                if(temp.id == temp2.id)
                {
                    intermed_dist d = distance(temp.par, temp2.par);
                    sum+=d.sum;
                    count+=d.count;
                    break;
                }
            }
        }
    }
    if(count) return sum/count;
    else return 0;
}

int knowledge::distance(int loop)
{
    int sum = 0, count = 0;
    record temp_comp;
    foreach(temp_comp, env_model)
    {
        foreach (history h,temp_comp.histories)
        {
            foreach(parameter temp, temp_comp.dev.par)
            {
                if(temp.index == h.index)
                {
                    QList<history_value>::iterator i = h.series.begin();
                    for(; i != h.series.end(); i++)
                    {
                        if((*i).cycle_number > loop)
                        {
                            count++;
                            sum+=delta(temp,(*(i-1)).value);
                        }
                    }
                    break;
                }
            }
        }
    }
    foreach(temp_comp, sys_model)
    {
        foreach (history h,temp_comp.histories)
        {
            foreach(parameter temp, temp_comp.dev.par)
            {
                if(temp.index == h.index)
                {
                    QList<history_value>::iterator i = h.series.begin();
                    for(; i != h.series.end(); i++)
                    {
                        if((*i).cycle_number > loop)
                        {
                            count++;
                            sum+=delta(temp,(*(i-1)).value);
                        }
                    }
                    break;
                }
            }
        }
    }
    if(count) return sum/count;
    else return 0;
}

intermed_dist knowledge::distance(QList<parameter> one, QList<parameter> two)
{
    intermed_dist d;
    d.sum = 0;
    d.count = 0;
    if(one.size() < two.size())
    {
        foreach(parameter temp, one)
        {
            foreach(parameter temp2, two)
            {
                if((temp.index = temp2.index) && (temp.type == temp2.type))
                {
                    d.sum += delta(temp,temp2.value);
                    d.count++;
                    break;
                }
            }
        }
    }
    else
    {
        foreach(parameter temp, two)
        {
            foreach(parameter temp2, one)
            {
                if((temp.index = temp2.index) && (temp.type == temp2.type))
                {
                    d.sum += delta(temp,temp2.value);
                    d.count++;
                    break;
                }
            }
        }
    }
    return d;
}

relation *knowledge::correlate(QList<history_value> dep, int dep_type, int id, int index, int type, int cl, val d, bool whose)
{
    QList<history_value> infl;
    QList<QList<history_value>::iterator> interferences;
    foreach(record r, sys_model)
    {
        foreach(history h, r.histories)
        {
            if((r.dev.id == id)&&(h.index == index))
            {
                infl = h.series;
            }
            else {
                foreach(par_class p, r.classes)
                {
                    if(p.index == h.index)
                    {
                        foreach(int i, p.classes)
                        {
                            if (i == cl)
                            {
                                interferences.append(h.series.begin());
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    QList<history_value>::iterator i = dep.begin() + 1;
    //Ставим итератор зависимого во второе изменение (первое - это просто старт)
    QList<history_value>::iterator inf = infl.begin() + 1;
    //И влияющего тоже
    QList<weighed_rel> relations;

    for(;inf != infl.end(); inf++)
    {
        history_value v = *inf;
        int loop = v.cycle_number;
        //Находим, где было очередное изменение
        for(; i != (dep.end() - 1); i++)
        {
            if((*(i + 1)).cycle_number > loop) break;
            //
        }
        if (i == (dep.end() - 1)) break;
        //Если дошли до конца истории, заканчиваем

        if((*i).cycle_number > loop) continue;
        //Это значит, история зависимого началась позже истории влияющего, надо проехать вперед

        if(is_peace(i,dep_type) == false) continue;
        //Если изменение случилось не в покое, не подходит

        QList<history_value>::iterator j = i + 1;
        //Итое может быть как угодно, а на j изменение должно было отразиться.
        for(;j != dep.end(); j++) if(is_peace(j,dep_type)) break;
        //Нашли следующий период покоя после возмущения
        if(j == dep.end()) break;
        //Если не нашли, а просто дошли до конца, то больше искать смысла ничего нет

        int loop_end = (*(j-1)).cycle_number;

        bool interrupted = false;
        QList<QList<history_value>::iterator>::iterator in = interferences.begin();
        for(; in != interferences.end(); in++)
        {
            for(;(*(*in)).cycle_number < (loop - 100); (*in)++);
            if((*(*in)).cycle_number < loop_end)
            {
                interrupted = true;
                break;
            }
        }
        if(interrupted) continue;
        //Если во время активного периода менялся кто-то еще, этот период нам не нужон
        //И если у нас в итоге остался период, мы кладем его к другим нормальным периодам

        val d_d, d_i;
        d_d = subtract((*i).value, (*(j-1)).value, dep_type);
        d_i = subtract((*(inf - 1)).value ,v.value, type);
        relations.append(norm_rel(loop,loop_end,d_d,d_i,dep_type,type,whose,d));
    }

    if(relations.isEmpty()) return nullptr;
    //Ничего не нашли

    int sum = 0, count = 0;
    double sum_d = 0, sum_i = 0;
    foreach(weighed_rel r, relations)
    {
        sum+=r.r.time * r.w;
        sum_d+=add_val(r.r.d1, r.w, dep_type);
        sum_i+=add_val(r.r.d2, r.w, type);
        count+=r.w;
    }
    if(count == 0) return nullptr;
    //Опять ничего не нашли

    relation* res = new relation;
    res->time = sum/count;
    res->d1 = avg(sum_d,count,dep_type);
    res->d2 = avg(sum_i,count,type);
    return res;
}

bool knowledge::is_peace(QList<history_value>::iterator i, int type)
{
    int len = (*i).cycle_number - (*(i-1)).cycle_number;
    double derivative = 0;
    switch (type) {
    case TEMPERATURE:
    case PERCENT:
        derivative = ((*i).value.i - (*(i-1)).value.i)/static_cast<double>(len);
        break;
    case ON_OFF:
        derivative = ((*i).value.b - (*(i-1)).value.b)/static_cast<double>(len);
        break;
    case COEFF:
    case F_SIZE:
        derivative = static_cast<double>((*i).value.f - (*(i-1)).value.f)/static_cast<double>(len);
        break;
    }
    if(fabs(derivative) <= PEACE_TRESHOLD) return true;
    else return false;
}

weighed_rel knowledge::norm_rel(int start, int fin, val d_d, val d_i, int type_d, int type_i, bool whose, val must)
{
    weighed_rel rel;
    rel.r.time = fin - start;
    double k;
    if(whose)
    {
        k = calc_k(d_d, type_d, must);
        rel.r.d1 = d_d;
        rel.r.d2 = apply_k(k, type_i, d_i);
    }
    else
    {
        k = calc_k(d_i, type_i, must);
        rel.r.d2 = d_i;
        rel.r.d1 = apply_k(k, type_d, d_d);
    }
    rel.w = 100 - distance(start);
    if(rel.w < 0) rel.w = 0;
    return rel;
}

double knowledge::calc_k(val v, int t, val m)
{
    double k = 1;
    switch (t)
    {
    case TEMPERATURE:
    case PERCENT:
        k = m.i/static_cast<double>(v.i);
        break;
    case ON_OFF:
        if(v.b != m.b) k = -1;
        else k = 1;
        break;
    case COEFF:
    case F_SIZE:
        k = static_cast<double>(m.f)/static_cast<double>(v.f);
        break;
    }
    return k;
}

val knowledge::apply_k(double k, int t, val v)
{
    val r;
    switch (t)
    {
    case TEMPERATURE:
    case PERCENT:
        r.i = static_cast<int>(round(v.i * k));
        break;
    case ON_OFF:
        if(k < 0) r.b = !v.b;
        else r.b = v.b;
        break;
    case COEFF:
    case F_SIZE:
        r.f = static_cast<float>(static_cast<double>(v.f) * k);
        break;
    }
    return r;
}

val knowledge::subtract(val what, val from, int type)
{
    val res;
    switch (type)
    {
    case TEMPERATURE:
    case PERCENT:
        res.i = from.i - what.i;
        break;
    case ON_OFF:
        if(!what.b && from.b) res.b = true;
        //Увеличилось
        else res.b = false;
        //Уменьшилось
        break;
    case COEFF:
    case F_SIZE:
        res.f = from.f - what.f;
        break;
    }
    return res;
}

double knowledge::add_val(val what, int weight, int type)
{
    double res = 0;
    switch (type)
    {
    case TEMPERATURE:
    case PERCENT:
        res = static_cast<double>(what.i) * weight;
        break;
    case ON_OFF:
        if(what.b) res = 1.0 * weight;
        //Увеличилось
        else res = -1.0 * weight;
        //Уменьшилось
        break;
    case COEFF:
    case F_SIZE:
        res = static_cast<double>(what.f) * weight;
        break;
    }
    return res;
}

val knowledge::avg(double sum, int count, int type)
{
    val res;
    switch (type) {
    case TEMPERATURE:
    case PERCENT:
        res.i = static_cast<int>(round(sum/count));
        break;
    case ON_OFF:
        if((sum/count) > 0) res.b = true;
        else res.b = false;
        break;
    case COEFF:
    case F_SIZE:
        res.f = static_cast<float>(sum/count);
        break;
    }
    return res;
}
