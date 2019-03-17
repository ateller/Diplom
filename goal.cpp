#include "goal.h"

goal::goal()
{

}

/*goal::goal(QList<device *> source)
{
    device* temp;
    foreach(temp, source)
    {
        sensor* toadd = qobject_cast<sensor*>(temp);
        if (toadd == NULL)
            sys_goal.add(qobject_cast<effector*>(temp));
        else
            env_goal.add(toadd);
    }
}
*/

void goal::add_to_list(QList<goal_comp> *l, device* d)
{
    component_parametres source = d->get_list();
    goal_comp dest;
    dest.pointer = source.pointer;
    parameter temp;
    foreach(temp, source.list)
    {
        goal_param temp_g;
        temp_g.doesnt_care = 0;
        temp_g.par = temp;
        dest.list+=temp_g;
    }
    *l+=dest;
}

void goal::add(device *d)
{
    sensor* toadd = qobject_cast<sensor*>(d);
    if (toadd == NULL) {
        add_to_list(&sys_goal, d);
        emit added(1);
    }
    else {
        add_to_list(&env_goal, d);
        emit added(0);
    }
}

void goal::update(int i, QString par, QString new_val, bool list_type)
{
    int j = 0;
    QList<goal_comp> *for_search;
    /*
    if(qobject_cast<sensor*>(comp) == NULL)
    {
        for_search = &sys_goal;
    }
    else
    {
        for_search = &env_goal;
    }
    */
    if(list_type == 1) {
        for_search = &sys_goal;
    }
    else
    {
        for_search = &env_goal;
    }
    /*
    int i = 0,
    goal_comp search;
    foreach(search, *for_search)
    {
        if (search.pointer == comp) break;
        i++;
    }
    //Ижем компонент
    */
    goal_param to_change;
    foreach(to_change, (*for_search)[i].list)
    {
        if(to_change.par.name == par) {
            break;
        }
        j++;
    }
    (*for_search)[i].list[j].par.value = new_val;
}
