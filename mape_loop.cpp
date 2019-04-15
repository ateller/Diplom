#include "mape_loop.h"

mape_loop::mape_loop()
{
    tolerance = 3;
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
        classes.append(t);
    }

    foreach(record rec, k->env_model)
    {
        int i = 0;
        foreach(splited temp, split(rec))
        {
            if(temp.delta > 0)
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
            if(temp.delta > 0)
            {
                classes[i].list.append(temp.el);
                classes[i].delta += temp.delta;
            }
            i++;
        }
    }

    std::sort(classes.begin(), classes.end(), compare_cl);

    foreach(class_list temp, classes)
    {
        applicable.delta = -1;
        foreach(record rec, k->sys_model)
        {
            QList<rule> rules = qobject_cast<effector*>(rec.pointer)->ruleset;
            QList<rule>::iterator r;
            int i;
            for (i = 0, r = rules.begin(); r != rules.end(); i++, r++)
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
                    add = check_par(rec.dev.id, temp_o);
                    if(add == false) break;
                    //Проверяем, не используется ли где
                }
                if (add == false) continue;
                //Проверили по операции

               int delta = prognose_distance(); //функцию допишу
               if (delta <= 0) continue;
               if (delta > applicable.delta)
               {
                   applicable.id = rec.dev.id;
                   applicable.index = i;
                   applicable.delta = delta;
                   applicable.r = (*r);
               }
            }
        }

        if(applicable.delta > -1)
        {
            to_execute r;
            r.id = applicable.id;
            r.operation = applicable.r.operation;
            r.timer = applicable.r.period;
            qobject_cast<effector*> (k->get_device(applicable.id))->ruleset[applicable.index].last_use = k->loops_counter;
            ex_plan += r;
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

rule* mape_loop::generate_rule()
{
    return nullptr;
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

    foreach(par_class temp, r.classes)//Для каждого параметра, имебщего класс
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
        if (temp_g.not_care == true) continue;
        //Если его цель не имеет значения, он нам не нужен
        int delta = k->delta(temp_p,temp_g.value);
        if(delta == 0) continue;
        //Если он в норме, тоже не нужен

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
            list[i].delta += delta;
        }
    }

    return list;
}

bool mape_loop::uses(int id_1, QList<parameter> operation, int id_2, parameter p)
{
    if(id_1 != id_2) return false;

    foreach(parameter temp, operation)
    {
        if(temp.index == p.index) return true;
    }
    return false;
}

int mape_loop::prognose_distance()
{
    return 1;
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
