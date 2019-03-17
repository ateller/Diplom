#include "mape_loop.h"

mape_loop::mape_loop()
{
    tolerance = 3;
    must_adapt = false;
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
        return NULL;
    }
}

void mape_loop::add_device(device *pointer, int id)
{
    k.add(pointer, id);
    connect(this, SIGNAL(system_update()), pointer, SLOT(update()));
}

int mape_loop::indexof(sensor* s)
{
    return k.indexof(s);
}
int mape_loop::indexof(effector* e)
{
    return k.indexof(e);
}

int mape_loop::indexof(int id)
{
    return k.indexof(id);
}

void mape_loop::monitor()
{
    emit system_update();
    k.upd();
    emit monitor_completed();
}

void mape_loop::analysis()
{
    dist = k.distance();
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
    foreach(rec, k.sys_model)
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
                    model = k.sys_model[indexof(temp_c.dev_id)].dev.par[temp_c.p.index];
                else
                    model = k.env_model[indexof(temp_c.dev_id)].dev.par[temp_c.p.index];
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
            if(temp_r.timer)
            {
                add = false;
                temp->ruleset[i].timer--;
            }
            if(add == true)
            {
                to_execute r;
                r.subj = temp;
                r.operation = temp_r.operation;
                temp->ruleset[i].timer = temp_r.period;
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
        if(!temp.subj->broken) {
            temp.subj->exec_rule(temp.operation);
            i++;
        }
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
