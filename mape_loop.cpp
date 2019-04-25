#include "mape_loop.h"

mape_loop::mape_loop()
{
    tolerance = 30;
    k = new knowledge;
}

device* new_device(int type)
{
    switch(type)
    {
    case 1:
        return new thermometer;
    case 2:
        return new heater;
    case 3:
        return new window;
    default:
        return nullptr;
    }
}

int mape_loop::add_device(device *pointer)
{
    int id = k->add(pointer);
    connect(this, SIGNAL(system_update()), pointer, SLOT(update()));
    return id;
}

int mape_loop::indexof(sensor* s)
{
    return k->indexof(s);
}
int mape_loop::indexof(effector* e)
{
    return k->indexof(e);
}

int mape_loop::indexof(int id)
{
    return k->indexof(id);
}

void mape_loop::monitor()
{
    emit system_update();
    k->upd();
    emit monitor_completed();
}

void mape_loop::analysis()
{
    QList<executing_rule>::iterator i = k->exec_rules.begin();
    for(; i != k->exec_rules.end(); i++)
    {
        if((k->loops_counter - (*i).start_loop) == 1)
        {
            //здесь будет прерывание исполнения, когда я его сделаю
        }
        if ((*i).timer == 0)
        {
            k->finish_execution(i);
        }
    }

    dist = k->distance();
    emit analysis_completed();
}

void mape_loop::plan()
{
    ex_plan.clear();
    if(dist <= tolerance) return;
    //Проверяем дистанцию

    QList<class_list> classes;
    applicable_rule applicable;
    for(int i = 0; i < NUM_OF_CLASSES; i++)
    {
        class_list t;
        t.delta = 0;
        t.cl = i;
        classes.append(t);
    }

    foreach(record rec, k->env_model)
    {
        int i = 0;
        foreach(splited temp, split(rec))
        {
            if(!temp.el.deltas.isEmpty())
            {
                classes[i].list.append(temp.el);
                classes[i].delta += temp.delta;
            }
            i++;
        }
    }
    foreach(record rec, k->sys_model)
    {
        int i = 0;
        foreach(splited temp, split(rec))
        {
            if(!temp.el.deltas.isEmpty())
            {
                classes[i].list.append(temp.el);
                classes[i].delta += temp.delta;
            }
            i++;
        }
    }

    for(int i = 0; i < NUM_OF_CLASSES; i++)
    {
        int d = 0, min = 0, j = 0;
        QList<class_list>::iterator temp = classes.begin();
        for(; temp != classes.end(); temp++)
        {
            if((*temp).delta > d)
            {
                d = (*temp).delta;
                min = j;
            }
            j++;
        }
        temp = classes.begin() + min;

        if((*temp).delta == 0) break;
        //Если дошли до нулевых дельт, делать нечего
        applicable.delta = -1;
        QList<class_list_el>::iterator dev = (*temp).list.begin();
        for(;dev != (*temp).list.end(); dev++)
        {
            if((*dev).dev.id > 0) break;
        }
        for(;dev != (*temp).list.end(); dev++)
        {
            record rec = k->sys_model[k->indexof((*dev).dev.id)];
            QList<rule> *rules = &qobject_cast<effector*>(rec.pointer)->ruleset;
            QList<rule>::iterator r = rules->begin();
            int i;
            for (i = 0; r != rules->end(); i++, r++)
            {
                bool add = true;
                foreach(condition temp_c, (*r).pre)
                {
                    parameter model;

                    if(temp_c.dev_id > 0)
                        model = k->sys_model[indexof(temp_c.dev_id)].dev.par[temp_c.p.index];
                    else
                        model = k->env_model[indexof(temp_c.dev_id)].dev.par[temp_c.p.index];

                    switch (model.type) {
                        case ON_OFF:
                            if(model.value.b != temp_c.p.value.b) add = false;
                            break;
                    case PERCENT:
                    case TEMPERATURE:
                        add = compare(model.value.i, temp_c.p.value.i, temp_c.p.type);
                        break;
                    case COEFF:
                    case F_SIZE:
                        add = compare(model.value.f, temp_c.p.value.f, temp_c.p.type);
                    }
                    if(add == false) break;
                }
                if (add == false) continue;
                //Проверили по условиям

                foreach(parameter temp_o, (*r).operation)
                {
                    QList<int> cl;
                    foreach(par_class temp_pc, rec.classes)
                    {
                        if(temp_pc.index == temp_o.index)
                        {
                            cl = temp_pc.classes;
                            break;
                        }
                    }
                    if(cl.indexOf((*temp).cl) == -1)
                    {
                        add = false;
                        break;
                    }
                    add = check_par((*dev).dev.id, temp_o);
                    if(add == false) break;
                    //Проверяем, не используется ли где
                }
                if (add == false) continue;
                //Проверили по операции

               post_state post = k->create_postcond((*dev).dev.id, (*r).operation);
               int delta = prognose_distance(post.post, post.time, ex_plan);
               if (delta <= 0) continue;
               if (delta > applicable.delta)
               {
                   applicable.post = post.post;
                   applicable.id = (*dev).dev.id;
                   applicable.index = i;
                   applicable.delta = delta;
                   (*r).period = post.time;
                   applicable.r = (*r);
               }
            }
        }

        if(applicable.delta < 0)
        {
            generated_rule r = generate_rule(&(*temp));
            if(r.delta > 0)
            {
                applicable.post = r.post.post;
                applicable.id = r.id;

                applicable.delta = r.delta;
                applicable.r = r.r;
                applicable.r.period = r.post.time;

                effector* e = qobject_cast<effector*>(k->sys_model[k->indexof(r.id)].pointer);
                applicable.index = e->ruleset.size();

                e->add_rule(r.r);
                applicable.r.r_id = e->ruleset.back().r_id;
                emit rule_generated(r.id, applicable.index);
            }
        }

        classes.removeAt(j);
        if(applicable.delta > 0)
        {
            to_execute r;
            r.id = applicable.id;
            r.operation = applicable.r.operation;
            r.timer = applicable.r.period;
            r.post = applicable.post;
            r.r_id = applicable.r.r_id;
            qobject_cast<effector*>(k->get_device(applicable.id))->ruleset[applicable.index].last_use = k->loops_counter;
            ex_plan += r;

            temp = classes.begin();
            for(; temp!=classes.end(); temp++)
            {
                QList<class_list_el>::iterator el = (*temp).list.begin();
                for(; el != (*temp).list.end(); el++)
                {
                    if((*el).dev.id == r.id)
                    {
                        foreach(parameter op_to_del, r.operation)
                        {
                            int j = 0;
                            foreach(parameter p_to_del, (*el).dev.par)
                            {
                                if(p_to_del.index == op_to_del.index)
                                {
                                    temp->delta -= (*el).deltas.takeAt(j);
                                    (*el).dev.par.removeAt(j);
                                    (*el).hist.removeAt(j);
                                }
                                j++;
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    emit plan_completed(ex_plan.size());
}

void mape_loop::execute()
{
    int i = 0;
    to_execute temp;
    foreach (temp, ex_plan) {
        effector* subj = qobject_cast<effector*>(k->get_device(temp.id));
        if(!subj->broken) {
            subj->exec_rule(temp.operation);
            i++;
        }
        executing_rule r;
        r.id = temp.id;
        r.timer = temp.timer;
        r.start_loop = k->loops_counter;
        r.operation = temp.operation;
        r.post = temp.post;
        r.r_id = temp.r_id;
        k->exec_rules.append(r);
    }
    emit executed(i);
}

void mape_loop::loop()
{
    monitor();
    analysis();
    plan();
    execute();
}

int mape_loop::import_knowledge(QFile *f)
{
    knowledge* n = new knowledge;
    if (n->import_from_file(f) == 0)
    {
        delete k;
        k = n;
        foreach(record temp, k->env_model)
        {
            connect(this, SIGNAL(system_update()), temp.pointer, SLOT(update()));
        }
        foreach(record temp, k->sys_model)
        {
            connect(this, SIGNAL(system_update()), temp.pointer, SLOT(update()));
        }
        return 0;
    }
    else {
        delete n;
        return 1;
    }

}

generated_rule mape_loop::generate_rule(class_list *temp)
{
    QList<out_of_tol> list;
    int i = 0;
    foreach(class_list_el c, temp->list)
    {
        int j = 0;
        foreach(int d, c.deltas)
        {
            if(d > 0)
            {
                out_of_tol t;
                t.delta = d;
                t.dev_index = i;
                t.par_index = j;
                list.append(t);
            }
            j++;
        }
        i++;
    }

    std::sort(list.begin(), list.end(), compare_out);

    generated_rule res;
    res.delta = 0;
    foreach(out_of_tol t, list)
    {
        QList<class_list_el>::iterator dev = temp->list.begin() + t.dev_index;
        post_state post;
        QList<parameter> op;
        int id = 0;
        int delta = 0;

        if((*dev).dev.id > 0)
        {
            val g;
            parameter p = (*dev).dev.par[t.par_index];
            id = (*dev).dev.id;
            foreach(goal gl, k->sys_model[k->indexof((*dev).dev.id)].goal_model)
            {
                if(gl.index == p.index)
                {
                    g = gl.value;
                    break;
                }
            }

            parameter o = p;

            switch (p.type) {
            case TEMPERATURE:
            case PERCENT:
                o.value.i = g.i - p.value.i;
                o = k->get_post(o);
                break;
            case ON_OFF:
                if (!p.value.b && g.b) {
                    o.value.b = 1;
                    o.type = INCREASE;
                }
                else {
                    o.value.b = 0;
                    o.type = INCREASE;
                }
                break;
            case COEFF:
            case F_SIZE:
                o.value.f = g.f - p.value.f;
                o = k->get_post(o);
                break;
            }
            op.append(o);
            post = k->create_postcond((*dev).dev.id, op);
            delta = prognose_distance(post.post, post.time, ex_plan);
        }
        else
        {
            val g;
            parameter p = (*dev).dev.par[t.par_index];
            QList<history_value> h = (*dev).hist[t.par_index];

            foreach(goal gl, k->env_model[k->indexof((*dev).dev.id)].goal_model)
            {
                if(gl.index == p.index)
                {
                    g = gl.value;
                    break;
                }
            }
            switch (p.type) {
            case TEMPERATURE:
                case PERCENT:
                    g.i = g.i - p.value.i;
                    break;
                case ON_OFF:
                    if (!p.value.b && g.b) {
                        g.b = 1;
                    }
                    else {
                        g.b = 0;
                    }
                    break;
                case COEFF:
                case F_SIZE:
                    g.f = g.f - p.value.f;
                    break;
            }

            QList<class_list_el>::iterator it = temp->list.begin();
            for(;it != temp->list.end(); it++)
            {
                if((*it).dev.id > 0) break;
            }
            for(;it != temp->list.end(); it++)
            {
                int delta_t = 0;
                foreach(parameter inf, (*it).dev.par)
                {
                    QList<int> cl;
                    foreach(par_class c, k->sys_model[k->indexof((*it).dev.id)].classes)
                    {
                        if(c.index == p.index)
                        {
                            cl = c.classes;
                            break;
                        }
                    }

                    relation *rel = k->correlate(h, p.type, (*it).dev.id, inf.index, inf.type, cl, g, true);
                    if(rel == nullptr) continue;
                    inf.value = rel->d2;
                    parameter o = inf;

                    o = k->get_post(o);
                    QList<parameter> op_t;
                    op_t.append(o);

                    post_state post_t;
                    post_t = k->create_postcond((*it).dev.id, op_t);
                    delta_t = prognose_distance(post_t.post, post_t.time, ex_plan);
                    if(delta_t > delta)
                    {
                        id = (*it).dev.id;
                        post = post_t;
                        delta = delta_t;
                        op = op_t;
                    }
                }
            }
        }

        if(delta > res.delta)
        {
            res.r.pre = k->create_pre();
            res.r.period = post.time;
            res.r.last_use = - post.time;
            res.r.operation = op;
            res.id = id;
            res.post = post;
            res.delta = delta;
        }
    }
    return res;
}

QList<splited> mape_loop::split(record r)
{
    QList<splited> list;
    list.reserve(NUM_OF_CLASSES);
    for(int i = 0; i < NUM_OF_CLASSES; i++)
    {
        splited t;
        t.delta = 0;
        t.el.dev.id = r.dev.id;
        list.append(t);
    }
    //Обнуляем дельты и пишем id

    foreach(par_class temp, r.classes)//Для каждого параметра, имеющего класс
    {

        parameter temp_p = r.dev.par[temp.index];
        //Берем его запись
        if(r.dev.id > 0)
            if(check_par(r.dev.id, temp_p) == false) continue;
        //Если это параметр эффектора и он уже используется в правиле, он нам не нужен

        goal temp_g;
        foreach(temp_g, r.goal_model)
        {
            if(temp_g.index == temp.index)
            {
                break;
            }
        }
        //Ищем его цель
        int delta;
        if (temp_g.not_care == true)
        {
            delta = 0;
        }
        else
        {
            delta = k->delta(temp_p,temp_g.value);
        }

        QList<history_value> temp_hv;
        //Берем его историю
        foreach(history temp_h, r.histories)
        {
            if(temp_h.index == temp.index)
            {
                temp_hv = temp_h.series;
                break;
            }
        }

        foreach(int i, temp.classes)//Для каждого класса этого параметра
        {
            list[i].el.dev.par.append(temp_p);
            //Его запись
            list[i].el.hist.append(temp_hv);
            //Его историю
            list[i].el.deltas.append(delta);
            //Его дельту
            list[i].delta += delta;
        }
    }

    return list;
}

bool mape_loop::uses(int id_1, QList<parameter> operation, int id_2, parameter p)
//Возвращает true, если не использует
{
    if(id_1 != id_2) return false;

    foreach(parameter temp, operation)
    {
        if(temp.index == p.index) return true;
    }
    return false;
}

bool mape_loop::check_par(int id, parameter p)
{
    foreach(executing_rule temp_ex, k->exec_rules)
    {
        if(uses(id, temp_ex.operation, temp_ex.id, p) == true) return false;
    }

    foreach(to_execute temp_ex, ex_plan)
    {
        if(uses(id, temp_ex.operation, temp_ex.id, p) == true) return false;
    }
    return true;
}

bool compare_cl(const class_list l1, const class_list l2)
{
    if(l1.delta > l2.delta) return true;
    else return false;
}

bool compare_out(const out_of_tol l1, const out_of_tol l2)
{
    if(l1.delta > l2.delta) return true;
    else return false;
}

int mape_loop::prognose_distance(QList<post_cond> post, int time, QList<to_execute> additional)
{
    QList<dev_parameters>* state = new QList<dev_parameters>;
    foreach(record r, k->sys_model)
    {
        dev_parameters dev;
        dev.id = r.dev.id;
        foreach(goal g, r.goal_model)
        {
            dev.par.append(r.dev.par[g.index]);
        }
        state->append(dev);
    }
    foreach(record r, k->env_model)
    {
        dev_parameters dev;
        dev.id = r.dev.id;
        foreach(goal g, r.goal_model)
        {
            dev.par.append(r.dev.par[g.index]);
        }
        state->append(dev);
    }

    foreach(executing_rule r, k->exec_rules)
    {
        k->apply_post(state, r.post, k->loops_counter - r.start_loop, r.timer);
    }
    foreach(to_execute r, additional)
    {
        k->apply_post(state, r.post, 0, r.timer);
    }

    QList<dev_parameters> without = *state;
    k->apply_post(state, post, 0, time);

    int dist = k->distance(without) - k->distance(*state);
    return dist;
}
