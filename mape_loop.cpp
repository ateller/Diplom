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



    record rec;
    foreach(rec, k->sys_model)
    {
        effector* temp = qobject_cast<effector*> (rec.pointer);
        rule temp_r;
        int i = 0;
        foreach(temp_r, temp->ruleset)
        {
            bool add = true;
            condition temp_c;
            foreach(temp_c, temp_r.pre)
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
            if(k->loops_counter - temp_r.last_use < temp_r.period)
            {
                add = false;
            }
            if(add == true)
            {
                to_execute r;
                r.id = rec.dev.id;
                r.operation = temp_r.operation;
                r.timer = temp_r.period;
                temp->ruleset[i].last_use = k->loops_counter;
                ex_plan += r;
            }
            i++;
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

to_execute *mape_loop::generate_rule()
{
    return nullptr;
}

QList<class_list> mape_loop::split(record r)
{
    QList<class_list> list;
    list.reserve(NUM_OF_CLASSES);

    foreach(par_class temp, r.classes)//Для каждого параметра, имебщего класс
    {
        parameter temp_p = r.dev.par[temp.index];
        //Берем его запись
        QList<history_value> temp_hv;
        //Берем его историю
        foreach(history temp_h, r.histories)
        {
            if(temp_h.index == temp.index)
            {
                temp_hv = temp_h.series;
            }
        }

        foreach(int i, temp.classes)//Для каждого класса этого параметра
        {
            list[i].dev.id = r.dev.id;
            //Кладем в нужный класс его id
            list[i].dev.par.append(temp_p);
            //Его запись
            list[i].hist.append(temp_hv);
            //Его историю
        }
    }

    return list;
}
