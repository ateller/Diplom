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

void knowledge::upd_history(QList<record>::iterator temp)
{
    QList<history>::iterator j = (*temp).histories.begin();
    for(; j != (*temp).histories.end(); j++)
    {
        switch ((*temp).dev.par[(*j).index].type) {
            case TEMPERATURE:
            case PERCENT:
                if((*j).series.back().value.i == (*temp).dev.par[(*j).index].value.i) continue;
                break;
            case ON_OFF:
                if((*j).series.back().value.b == (*temp).dev.par[(*j).index].value.b) continue;
                break;
            case COEFF:
            case F_SIZE:
                if((*j).series.back().value.f == (*temp).dev.par[(*j).index].value.f) continue;
                break;
        }
        history_value h_v;
        h_v.cycle_number = loops_counter;
        h_v.value = (*temp).dev.par[(*j).index].value;
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
        upd_history(i);
    }
    i = sys_model.begin();
    for(; i != sys_model.end(); i++)
    {
        (*i).dev.par = (*i).pointer->get_list();
        upd_history(i);
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

void knowledge::finish_execution(executing_rule e)
{
    record *rec = &sys_model[indexof(e.id)];
    QList<rule> *set = &qobject_cast<effector*>(rec->pointer)->ruleset;
    QList<rule>::iterator f = set->begin();
    for(; f != set->end(); f++)
    {
        if((*f).r_id == e.r_id)
        {
            break;
        }
    }
    if(f == set->end()) return;

    if(!e.interrupted)
    {
        QList<dev_parameters> beg_state = history_state(e.start_loop);
        foreach(executing_rule r, exec_rules)
        {
            if(!r.interrupted)
            {
                if(r.start_loop <= e.start_loop)
                    apply_post(&beg_state, r.post, e.start_loop - r.start_loop, loops_counter - e.start_loop, false);
                else
                    apply_post(&beg_state, r.post, r.start_loop - e.start_loop, loops_counter - e.start_loop, true);
            }
        }
        foreach(post_state p, fin_posts)
        {
            if(p.time <= e.start_loop)
                apply_post(&beg_state, p.post, e.start_loop - p.time, loops_counter - e.start_loop, false);
            else
                apply_post(&beg_state, p.post, p.time - e.start_loop, loops_counter - e.start_loop, true);
        }
        //Применили к стейту все постусловия

        QList<post_cond> real_post = create_real_post(&beg_state, loops_counter - e.start_loop);

        double penny = rate_success(e.post, real_post);
        post_state fin;
        fin.time = e.start_loop;
        if(penny > 0.5)
        {
            fin.post = real_post;
        }
        else {
            fin.post = e.post;
        }

        (*f).failure_rate += penny;
    }

    if((*f).failure_rate > 1)
    {
        set->erase(f);
        emit r_del(e.id);
    }
}

QList<post_cond> knowledge::create_real_post(QList<dev_parameters> *state, int time)
{
    QList<post_cond> real_post;
    foreach(dev_parameters d, *state)
    {
        dev_parameters now;

        if(d.id < 0) now = env_model[indexof(d.id)].dev;
        else now = sys_model[indexof(d.id)].dev;

        foreach(parameter p, d.par)
        {
            post_cond temp;
            temp.dev_id = d.id;
            temp.time = time;
            parameter now_p = now.par[p.index];

            switch (p.type) {
            case TEMPERATURE:
            case PERCENT:
                now_p.value.i = now_p.value.i - p.value.i;
                temp.p = get_post(now_p);
                break;
            case ON_OFF:
                if(p.value.b == now_p.value.b)
                {
                    temp.p = now_p;
                    temp.p.type = SAME;
                }
                else if (!p.value.b && now_p.value.b) {
                    now_p.value.b = 1;
                    temp.p = now_p;
                    temp.p.type = INCREASE;
                }
                else {
                    now_p.value.b = 0;
                    temp.p = now_p;
                    temp.p.type = DECREASE;
                }
                break;
            case COEFF:
            case F_SIZE:
                now_p.value.f = now_p.value.f - p.value.f;
                temp.p = get_post(now_p);
                break;
            }
            real_post.append(temp);
        }
    }
    return real_post;
}

double knowledge::rate_success(QList<post_cond> post, QList<post_cond> real_post)
{
    double sum = 0;
    int count = 0;
    foreach(post_cond p, post)
    {
        foreach(post_cond r_p, real_post)
        {
            if((r_p.dev_id == p.dev_id) && (r_p.p.index == p.p.index))
            {
                int type;
                if(r_p.dev_id < 0)
                    type = env_model[indexof(r_p.dev_id)].dev.par[r_p.p.index].type;
                else
                    type = sys_model[indexof(r_p.dev_id)].dev.par[r_p.p.index].type;

                count ++;
                if(r_p.p.type == p.p.type)
                {
                    if(p.p.type == SAME)
                        break;
                    else
                    {
                        sum+=accuracy(p.p.value,r_p.p.value,type);
                    }
                }
                else
                {
                    switch (p.p.type) {
                    case INCREASE:
                        if(r_p.p.type == DECREASE)
                            sum+=accuracy(p.p.value, r_p.p.value, type);
                        else
                            sum+=1;
                        break;
                    case DECREASE:
                        if(r_p.p.type == INCREASE)
                            sum+=accuracy(p.p.value, r_p.p.value, type);
                        else
                            sum+=1;
                        break;
                    case SAME:
                            sum+=accuracy(r_p.p.value,type);
                        break;
                    }
                }
            }
        }
    }
    if(count) return (sum/count);
    else return 0;
}

double knowledge::accuracy(val post, val real, int type)
{
    double d = 0;
    switch (type) {
    case TEMPERATURE:
    case PERCENT:
        d = fabs(static_cast<double>(post.i - real.i)) / post.i;
        break;
    case ON_OFF:
        break;
    case COEFF:
    case F_SIZE:
        d = fabs(static_cast<double>(post.f - real.f)) / static_cast<double>(post.f);
        break;
    }
    if(d > 1)
    {
        d = 2 - d;
        if(d < 0)
            d = 0;
    }
    return d;
}

double knowledge::accuracy(val v, int type)
{
    double d = 0;
    switch (type) {
    case TEMPERATURE:
    case PERCENT:
        d = fabs(static_cast<double>(v.i)) / 5;
        break;
    case ON_OFF:
        d = 1;
        break;
    case COEFF:
    case F_SIZE:
        d = fabs(static_cast<double>(v.f)) / static_cast<double>(0.5);
        break;
    }
    if(d > 1)
    {
        d = 2 - d;
        if(d < 0)
            d = 0;
    }
    return d;
}

void knowledge::save(QFile* f)
{
    QDataStream str(f);

    str << loops_counter;
    str << id_counter;
    str << env_model.size();

    foreach(record temp, env_model)
    {
        save_record(temp, &str);
    }

    str << sys_model.size();

    foreach(record temp, sys_model)
    {
        save_record(temp, &str);
        str << qobject_cast<effector*>(temp.pointer)->r_id_counter;
        str << qobject_cast<effector*>(temp.pointer)->ruleset.size();
        foreach(rule temp_rule, qobject_cast <effector*> (temp.pointer)->ruleset)
        {
            str << temp_rule.r_id;
            str << temp_rule.failure_rate;
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

    str << exec_rules.size();

    foreach(executing_rule e, exec_rules)
    {
        str << e.id;

        str << e.post.size();
        foreach(post_cond temp, e.post)
        {
            str << temp.time;
            str << temp.dev_id;
            str << temp.p.index;
            str << temp.p.value.f;
            str << temp.p.type;
        }

        str << e.r_id;
        str << e.timer;

        str << e.operation.size();
        foreach(parameter temp, e.operation)
        {
            str << temp.index;
            str << temp.value.f;
            str << temp.type;
        }

        str << e.start_loop;
        str << e.interrupted;
    }

    str << first_start;

    str << fin_posts.size();
    foreach(post_state p, fin_posts)
    {
        str << p.time;
        str << p.post.size();
        foreach(post_cond temp, p.post)
        {
            str << temp.time;
            str << temp.dev_id;
            str << temp.p.index;
            str << temp.p.value.f;
            str << temp.p.type;
        }
    }
}

void knowledge::save_record(record temp, QDataStream* str)
{
    *str << temp.pointer->get_type();
    *str << temp.pointer->name;

    temp.pointer->update();
    *str << temp.dev.id;
    *str << temp.dev.par.size();
    foreach(parameter temp_p, temp.pointer->get_list())
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
    str >> id_counter;

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

        str >> qobject_cast<effector*>(sys_model.back().pointer)->r_id_counter;

        int k;

        if (f->atEnd()) return 1;

        str >> k;
        for (int j = 0; j < k; j++) {
            rule r;

            str >> r.r_id;
            str >> r.failure_rate;

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

            qobject_cast<effector*>(sys_model.back().pointer)->add_rule(r, true);
        }
    }

    str >> n; //exec_rules.size

    for (int i = 0; i < n; i++)
    {
        executing_rule e;
        str >> e.id;

        int m;
        str >> m; //e.post.size
        for (int l = 0; l < m; l++)
        {
            post_cond temp;
            str >> temp.time;
            str >> temp.dev_id;
            str >> temp.p.index;
            str >> temp.p.value.f;
            str >> temp.p.type;
            e.post.append(temp);
        }

        str >> e.r_id;
        str >> e.timer;

        str >> m; //e.operation.size
        for (int l = 0; l < m; l++)
        {
            parameter temp;
            str >> temp.index;
            str >> temp.value.f;
            str >> temp.type;
            e.operation.append(temp);
        }

        str >> e.start_loop;
        str >> e.interrupted;

        exec_rules.append(e);
    }

    str >> first_start;

    str >> n; //fin_posts.size()
    for (int i = 0; i < n; i++)
    {
        post_state p;
        str >> p.time;

        int m;
        str >> m; //p.post.size
        for (int l = 0; l < m; l++)
        {
            post_cond temp;
            str >> temp.time;
            str >> temp.dev_id;
            str >> temp.p.index;
            str >> temp.p.value.f;
            str >> temp.p.type;
            p.post.append(temp);
        }
        fin_posts.append(p);
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
    case HEATER:
        temp.pointer = new heater;
        break;
    case WINDOW:
        temp.pointer = new window;
        break;
    }

    *str >> temp.pointer->name;
    *str >> temp.dev.id;

    temp.names = temp.pointer->get_names();

    int k;
    *str >> k;
    for (int j = 0; j < k; j++) {
        parameter p;
        *str >> p.index;
        *str >> p.type;
        *str >> p.value.f;
        temp.pointer->to_be_controlled(p.index,p.value);
        temp.dev.par.append(p);
    }
    temp.pointer->update();

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
        delta = 10 * (g.i - p.value.i);
        if(delta < 0) delta = - delta;
        if(delta > 1000) delta = 1000;
        break;
    case ON_OFF:
        if(g.b != p.value.b)
            delta = 1000;
        break;
    case PERCENT:
        delta = 10 * (g.i - p.value.i);
        if(delta < 0) delta = - delta;
        break;
    case COEFF:
        delta = static_cast<int>(round(static_cast<double>(g.f - p.value.f) * 500));
        if (delta < 0) delta = - delta;
        break;
    case F_SIZE:
        delta = static_cast<int>(round(static_cast<double>(g.f - p.value.f)));
        if(delta < 0) delta = - delta;
        delta *= 500;
        if(delta > 1000) delta = 1000;
    }
    return delta;
}

post_state knowledge::create_postcond(int id, QList<parameter> operation)
{
    post_state post;
    QList<parameter> changes;
    record r = sys_model[indexof(id)];
    QList<par_class> classes;

    int i = 0;
    foreach(parameter o, operation)
    {
        post_cond p;
        p.dev_id = r.dev.id;
        p.time = 1;

        parameter par = r.dev.par[o.index];
        if (o.type == ASSIGN)
        {
            //Если ассигн, то считаем разницу между тем, что есть сейчас и что будет
            switch (par.type) {
            case TEMPERATURE:
            case PERCENT:
                par.value.i = o.value.i - par.value.i;
                p.p = get_post(par);
                break;
            case ON_OFF:
                if(par.value.b == o.value.b)
                {
                    p.p = par;
                    p.p.type = SAME;
                }
                else if (!par.value.b && o.value.b) {
                    par.value.b = 1;
                    p.p = par;
                    p.p.type = INCREASE;
                }
                else {
                    par.value.b = 0;
                    p.p = par;
                    p.p.type = DECREASE;
                }
                break;
            case COEFF:
            case F_SIZE:
                par.value.f = o.value.f - par.value.f;
                p.p = get_post(par);
                break;
            }
        }
        else {
            p.p = o;
            //Дикриз инкриз остался тот же
            //Еще нам нужна дельта со знаком
            if (o.type == DECREASE)
            {
                switch (par.type) {
                case TEMPERATURE:
                case PERCENT:
                    par.value.i = -o.value.i;
                    break;
                case ON_OFF:
                    par.value.b = 0;
                    break;
                case COEFF:
                case F_SIZE:
                    par.value.f = -o.value.f;
                    break;
                }
            }
            else
            {
                switch (par.type) {
                case TEMPERATURE:
                case PERCENT:
                    par.value.i = o.value.i;
                    break;
                case ON_OFF:
                    par.value.b = 1;
                    break;
                case COEFF:
                case F_SIZE:
                    par.value.f = o.value.f;
                    break;
                }
            }
        }
        post.post.append(p);
        //Добавили в постусловие

        if(p.p.type != SAME)
        {
            foreach(par_class cl, r.classes)
            {
                if(cl.index == o.index)
                {
                    cl.index = i;
                    classes.append(cl);
                }
            }
            //Запомнили классы параметров из операции, чтобы по нима потом смотреть

            changes.append(par);
            //Здесь индекс, тип, значение со знаком

            i++;
        }

    }
    //Сначала записали в пост изменения самих параметров

    post.time = 1;
    foreach(record rec, env_model)
    {
        foreach(par_class p_c, rec.classes)
        {
            parameter dep = rec.dev.par[p_c.index];
            post_cond p;
            p.dev_id = rec.dev.id;
            p.time = 0;
            p.p.index = p_c.index;
            double sum = 0;
            foreach(par_class o_c, classes)
            {
                parameter infl = changes[o_c.index];
                if(same_class(p_c.classes,o_c.classes))
                {
                    foreach(history h, rec.histories)
                    {
                        if(h.index == p_c.index)
                        {
                            relation *rel = correlate(h.series, dep.type, id, infl.index, infl.type, p_c.classes, infl.value, false);
                            if(rel != nullptr)
                            {
                                double check = sum;
                                switch (dep.type) {
                                case TEMPERATURE:
                                case PERCENT:
                                    sum += rel->d1.i;
                                    break;
                                case ON_OFF:
                                    if (rel->d1.b) sum += 1;
                                    else sum += -1;
                                    break;
                                case COEFF:
                                case F_SIZE:
                                    sum += static_cast<double>(rel->d1.f);
                                    break;
                                }
                                if((check != sum) && (rel->time > p.time)) p.time = rel->time;
                                delete  rel;
                            }
                            break;
                        }
                    }
                }
            }
            if(sum > 0)
            {
                p.p.type = INCREASE;
                switch (dep.type) {
                case TEMPERATURE:
                case PERCENT:
                    p.p.value.i = static_cast<int>(round(sum));
                    break;
                case ON_OFF:
                    p.p.value.b = 1;
                    break;
                case COEFF:
                case F_SIZE:
                    p.p.value.f = static_cast<float>(sum);
                    break;
                }
            }
            else if (sum < 0) {
                p.p.type = DECREASE;
                switch (dep.type) {
                case TEMPERATURE:
                case PERCENT:
                    p.p.value.i = -static_cast<int>(round(sum));
                    break;
                case ON_OFF:
                    p.p.value.b = 0;
                    break;
                case COEFF:
                case F_SIZE:
                    p.p.value.f = -static_cast<float>(sum);
                    break;
                }
            }
            else {
                p.p.type = SAME;
                p.p.value = dep.value;
                p.time = 1;
            }
            if(p.time > post.time) post.time = p.time;
            post.post.append(p);
        }
    }
    return post;
}

parameter knowledge::get_post(parameter delta)
{
    switch (delta.type) {
    case TEMPERATURE:
    case PERCENT:
        if(delta.value.i > 0) delta.type = INCREASE;
        else if (delta.value.i < 0) {
            delta.type = DECREASE;
            delta.value.i = -delta.value.i;
        }
        else delta.type = SAME;
        break;
    case ON_OFF:
        break;
    case COEFF:
    case F_SIZE:
        if(delta.value.f > 0) delta.type = INCREASE;
        else if (delta.value.f < 0) {
            delta.type = DECREASE;
            delta.value.f = -delta.value.f;
        }
        else delta.type = SAME;
        break;
    }
    return delta;
}

QList<condition> knowledge::create_pre()
{
    QList<condition> pre;
    foreach(record r, sys_model)
    {
        foreach(goal g, r.goal_model)
        {
            add_pre_cond(&pre, r.dev.par[g.index], r.dev.id);
        }
    }
    foreach(record r, env_model)
    {
        foreach(goal g, r.goal_model)
        {
            add_pre_cond(&pre, r.dev.par[g.index], r.dev.id);
        }
    }
    return pre;
}

void knowledge::apply_post(QList<dev_parameters> *state, QList<post_cond> post, int time_before, int time, bool after)
{
    foreach(post_cond p, post)
    {
        QList<dev_parameters>::iterator dev = state->begin();
        for(; dev!=state->end(); dev++)
        {
            if((*dev).id == p.dev_id)
            {
                QList<parameter>::iterator i = (*dev).par.begin();
                for(;i != (*dev).par.end(); i++)
                {
                    if((*i).index == p.p.index)
                    {
                        if(p.p.type != SAME)
                        {
                            int t;
                            if(!after)
                            {
                                t = p.time - time_before;
                                //Время, которое занимает полное изменение в пост минус время, прошедшее с
                                //начала правила до момента, в котором происходит наш стейт
                            }
                            else
                            {
                                t = time - time_before;
                                //Время, которое занимает стейт минус время с начала стейта до начала правила
                            }
                            if(t > 0)
                            {
                                if(!after)
                                {
                                    if(t > time) t = time;
                                    //Что больше - пересечение или период стейта
                                }
                                else
                                {
                                    if(t > p.time) t = p.time;
                                    //Что больше - пересечение или период поста
                                }

                                double k = static_cast<double>(t)/p.time;
                                //От времени зависит действие

                                if(p.p.type == INCREASE){
                                    switch ((*i).type) {
                                    case TEMPERATURE:
                                    case PERCENT:
                                        (*i).value.i += round(static_cast<double>(p.p.value.i) * k);
                                        break;
                                    case ON_OFF:
                                        if(k > 0.5) (*i).value.b = 1;
                                        break;
                                    case COEFF:
                                    case F_SIZE:
                                        (*i).value.f += static_cast<float>(static_cast<double>(p.p.value.f) * k);
                                        break;
                                    }
                                }
                                else {
                                    switch ((*i).type) {
                                    case TEMPERATURE:
                                    case PERCENT:
                                        (*i).value.i += -round(static_cast<double>(p.p.value.i) * k);
                                        break;
                                    case ON_OFF:
                                        if(k > 0.5) (*i).value.b = 0;
                                        break;
                                    case COEFF:
                                    case F_SIZE:
                                        (*i).value.f += -static_cast<float>(static_cast<double>(p.p.value.f) * k);
                                        break;
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
}

bool knowledge::same_class(QList<int> one, QList<int> two)
{
    foreach(int o, one)
    {
        foreach(int t, two)
        {
            if(o == t) return true;
        }
    }
    return false;
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

int knowledge::distance(QList<dev_parameters> temp)
{
    int sum = 0, count = 0;
    foreach(record r, env_model)
    {
        foreach(dev_parameters d, temp)
        {
            if(d.id == r.dev.id)
            {
                foreach(goal g, r.goal_model)
                {
                    if(!g.not_care)
                    {
                        foreach(parameter p, d.par)
                        {
                            if(p.index == g.index)
                            {
                                sum+=delta(p,g.value);
                                count++;
                                break;
                            }
                        }
                    }
                }
                break;
            }
        }
    }
    foreach(record r, sys_model)
    {
        foreach(dev_parameters d, temp)
        {
            if(d.id == r.dev.id)
            {
                foreach(goal g, r.goal_model)
                {
                    if(!g.not_care)
                    {
                        foreach(parameter p, d.par)
                        {
                            if(p.index == g.index)
                            {
                                sum+=delta(p,g.value);
                                count++;
                                break;
                            }
                        }
                    }
                }
                break;
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

relation *knowledge::correlate(QList<history_value> dep, int dep_type, int id, int index, int type, QList<int> cl, val d, bool whose)
{
    QList<history_value> infl;
    struct interference{QList<history_value>::iterator i; QList<history_value>::iterator end;};
    QList<interference> interferences;
    QList<record>::iterator r = sys_model.begin();;
    for(;r != sys_model.end(); r++)
    {
        QList<history>::iterator h = (*r).histories.begin();
        for(;h != (*r).histories.end(); h++)
        {
            if(((*r).dev.id == id)&&((*h).index == index))
            {
                infl = (*h).series;
            }
            else {
                foreach(par_class p, (*r).classes)
                {
                    if(p.index == (*h).index)
                    {
                        if(same_class(cl, p.classes))
                        {
                            interference itr;
                            itr.i = (*h).series.begin();
                            itr.end = (*h).series.end();
                            interferences.append(itr);
                            break;
                        }
                    }
                }
            }
        }
    }
    QList<history_value>::iterator i = dep.begin();
    //Ставим итератор в первое изменение
    QList<history_value>::iterator inf = infl.begin() + 1;
    //И влияющего тоже
    QList<weighed_rel> relations;

    for(;inf != infl.end(); inf++)
    {
        history_value v = *inf;
        int loop = v.cycle_number;
        //Находим, где было очередное изменение
        for(; i < (dep.end() - 1); i++)
        {
            if((*(i + 1)).cycle_number > loop) break;
        }
        if (i == (dep.end() - 1)) break;
        //Если дошли до конца истории, заканчиваем

        if((*i).cycle_number > loop) continue;
        //Это значит, история зависимого началась позже истории влияющего, надо проехать вперед

        if(is_peace(dep.begin(), i, loop - (*i).cycle_number ,dep_type) == false) continue;
        //Если изменение случилось не в покое, не подходит
        //Покой - не более чем одно изменение на 100 циклов

        QList<history_value>::iterator j = i + 2;
        //Итое в покое, i + 1 может быть как угодно, а на j изменение должно было отразиться.
        for(;j != dep.end(); j++) if(is_peace(dep.begin(), j, 0, dep_type)) break;
        //Нашли следующий период покоя после возмущения
        if(j == dep.end())
        {
            if((loops_counter - (*(j-1)).cycle_number) < 100) break;
            //Если дошли до конца, но последнее изменение было достаточно давно, есть все основания
            //предположить, что это покой. А если нет, то досвидания
        }

        int loop_end = (*(j-1)).cycle_number;

        bool interrupted = false;
        foreach(interference in, interferences)
        {
            for(;in.i != in.end; in.i++)
            {
                if((*(in.i)).cycle_number > (loop - 100))
                {
                    if ((*in.i).cycle_number < loop_end) interrupted = true;
                    break;
                }
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

bool knowledge::is_peace(QList<history_value>::iterator beg, QList<history_value>::iterator i, int add_time, int type)
{
    if(i == beg) return true;
    //Если изменение произошло сразу после старта, оно в покое

    int len = (*i).cycle_number - (*(i-1)).cycle_number + add_time;

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
        rel.r.d1 = must;
        rel.r.d2 = apply_k(k, type_i, d_i);
        //Равняемся на зависимого
    }
    else
    {
        k = calc_k(d_i, type_i, must);
        rel.r.d2 = must;
        rel.r.d1 = apply_k(k, type_d, d_d);
    }
    rel.w = 1000 - distance(start);
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

QList<dev_parameters> knowledge::history_state(int loop)
{
    QList<dev_parameters> res;
    foreach(record r, sys_model)
    {
        dev_parameters dev;
        dev.id = r.dev.id;
        foreach(history h, r.histories)
        {
            QList<history_value>::iterator i = h.series.begin();
            for(;i != h.series.end(); i++)
            {
                if((*i).cycle_number > loop)
                {
                    break;
                }
            }
            parameter p = r.dev.par[h.index];
            p.value = (*(i - 1)).value;
            dev.par.append(p);
        }
        res.append(dev);
    }
    foreach(record r, env_model)
    {
        dev_parameters dev;
        dev.id = r.dev.id;
        foreach(history h, r.histories)
        {
            QList<history_value>::iterator i = h.series.begin();
            for(;i != h.series.end(); i++)
            {
                if((*i).cycle_number > loop)
                {
                    break;
                }
            }
            parameter p = r.dev.par[h.index];
            p.value = (*(i - 1)).value;
            dev.par.append(p);
        }
        res.append(dev);
    }
    return res;
}

void knowledge::upd_posts()
{
    if(exec_rules.isEmpty())
    {
        first_start = 0;
        if(!fin_posts.isEmpty())
            fin_posts.clear();
    }
    else
    {
        first_start = exec_rules.first().start_loop;
        QList<post_state>::iterator i = fin_posts.begin();
        while(i != fin_posts.end())
        {
            QList<post_cond>::iterator j = (*i).post.begin();
            while(j != (*i).post.end())
            {
                if(((*i).time + (*j).time) <= first_start)
                    j = (*i).post.erase(j);
                else
                    j++;
            }
            if((*i).post.isEmpty())
                i = fin_posts.erase(i);
            else
                i++;
        }
    }
}

int knowledge::max_time(int time)
{
    if (exec_rules.isEmpty()) return time;
    else {
        QList<executing_rule>::iterator i = exec_rules.begin();
        for(;i != exec_rules.end(); i++)
        {
            if((*i).timer > time)
                time = (*i).timer;
        }
    }
    return time;
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

void knowledge::add_pre_cond(QList<condition> *pre, parameter p, int id)
{
    condition c;
    c.dev_id = id;
    c.p = p;
    switch (p.type) {
    case TEMPERATURE:
        c.p.value.i = p.value.i -5;
        c.p.type = LARGER;
        pre->append(c);

        c.p.value.i = p.value.i + 5;
        c.p.type = LESS;
        pre->append(c);
        break;
    case PERCENT:
        if(p.value.i > 5)
        {
            c.p.value.i = p.value.i -5;
            c.p.type = LARGER;
            pre->append(c);
        }
        if(p.value.i < 95)
        {
            c.p.value.i = p.value.i + 5;
            c.p.type = LESS;
            pre->append(c);
        }
        break;
    case ON_OFF:
        c.p.type = EQUAL;
        pre->append(c);
        break;
    case COEFF:
        c.p.value.f = p.value.f - static_cast<float>(0.5);
        c.p.type = LARGER;
        pre->append(c);

        c.p.value.f = p.value.f + static_cast<float>(0.5);
        c.p.type = LESS;
        pre->append(c);
        break;
    case F_SIZE:
        if(p.value.f > static_cast<float>(0.5))
        {
            c.p.value.f = p.value.f - static_cast<float>(0.5);
            c.p.type = LARGER;
            pre->append(c);
        }
        c.p.value.f = p.value.f + static_cast<float>(0.5);
        c.p.type = LESS;
        pre->append(c);
        break;
    }
}


