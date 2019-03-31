#include "mape_loop.h"

mape_loop::mape_loop()
{
    tolerance = 3;
    must_adapt = false;
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
    if(dist>tolerance)
        must_adapt = true;
    else
        must_adapt = false;
    emit analysis_completed();
}

void mape_loop::plan()
{
    ex_plan.clear();
    record rec;
    foreach(rec, k->sys_model)
    {
        effector* temp = qobject_cast<effector*> (rec.pointer);
        rule temp_r;
        int i = 0;
        foreach(temp_r, temp->ruleset)
        {
            bool add = 1;
            condition temp_c;
            foreach(temp_c, temp_r.pre)
            {
                parameter model;
                if(temp_c.dev_id > 0)
                    model = k->sys_model[indexof(temp_c.dev_id)].dev.par[temp_c.p.index];
                else
                    model = k->env_model[indexof(temp_c.dev_id)].dev.par[temp_c.p.index];
                if(model.type == ON_OFF) {
                    if(model.value != temp_c.p.value){
                        add = false;
                    }
                }
                else {
                        switch (temp_c.p.type) {
                        case LESS:
                            if(model.value >= temp_c.p.value)
                                add = false;
                            break;
                        case LARGER:
                            if(model.value <= temp_c.p.value)
                                add = false;
                            break;
                        case EQUAL:
                            if(model.value != temp_c.p.value)
                                add = false;
                            break;
                        }
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
    if(must_adapt == true) {
        plan();
        execute();
    }
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
