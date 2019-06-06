#include "control.h"
#include "ui_control.h"

control::control(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::control)
{
    ui->setupUi(this);

    QMenuBar *m = new QMenuBar(this);
    layout()->setMenuBar(m);

    QMenu *file = new QMenu("File", m);
    file->addAction("Open home", this, SLOT(open_file()));
    file->addSeparator();
    file->addAction("Save home", this, SLOT(save_slot()));
    file->addAction("Save home as...", this, SLOT(save_to_new_file()));

    QMenu *add = new QMenu("Add component", m);
    add->addAction("Thermometer", this, SLOT(add_record()))->setProperty("comp_type", QVariant(1));
    add->addSeparator();
    add->addAction("Heater", this, SLOT(add_record()))->setProperty("comp_type", QVariant(2));
    add->addAction("Window", this, SLOT(add_record()))->setProperty("comp_type", QVariant(3));

    m->addMenu(file);
    m->addMenu(add);
    QAction* imit_edit = m->addAction("Edit imitation parameters");
    connect(imit_edit, SIGNAL(triggered()), i.get_ctrl_pointer(), SLOT(show()));

    QAction* start = m->addAction("Start");

    connect(start, SIGNAL(triggered()), &w, SLOT(show()));
    start->setEnabled(0);

    connect(manager.k, SIGNAL(added(int)), SLOT(upd_goal(int)));
    connect(&manager, SIGNAL(rule_generated(int, int)), this, SLOT(add_gen_rule(int, int)));
    connect(manager.k, SIGNAL(r_del(int)), SLOT(delete_rule(int)));

    w.set(&manager);

    connect(&w, SIGNAL(loop(int)), SLOT(loop(int)));
    ui->componentsListScrollArea->setLayout(new QVBoxLayout);

    file_path.clear();
}

control::~control()
{
    delete ui;
}

void control::add_record()
{
    int type = sender()->property("comp_type").value<int>();

    device* temp;
    temp = new_device(type);
    temp->name = qobject_cast<QAction*>(sender())->text() + " " + QString::number(manager.k->id_counter);

    add_dev_widget(temp, manager.add_device(temp));
}

void control::add_dev_widget(device* temp, int id)
{   
    QWidget* dev_widget = new QWidget();

    QVBoxLayout *vertical = new QVBoxLayout;
    dev_widget->setLayout(vertical);

    QHBoxLayout *check_and_name = new QHBoxLayout;

    QLineEdit *name = new QLineEdit(temp->name);
    connect(name, SIGNAL(textEdited(QString)), SLOT(edit_name(QString)));

    QCheckBox *c = new QCheckBox("Broken");
    connect(c, SIGNAL(toggled(bool)), temp, SLOT(set_broken(bool)));

    check_and_name->addWidget(name);
    check_and_name->addWidget(c);

    vertical->addLayout(check_and_name);

    QHBoxLayout *buttons = new QHBoxLayout;

    QPushButton* a = new QPushButton("Parameters");
    a->setCheckable(1);

    buttons->addWidget(a);

    QPushButton* s = nullptr;

    if(id > 0)
    {
        QPushButton* b = new QPushButton("Add rule");
        connect(b, SIGNAL(clicked()), SLOT(add_rule()));
        buttons->addWidget(b);

        s = new QPushButton("Show rules");
        s->setCheckable(1);
        s->setDisabled(1);
        buttons->addWidget(s);


    }
    dev_widget->setProperty("id", QVariant(id));

    vertical->addLayout(buttons);

    QWidget* parameters_widget = new QWidget;
    parameters_widget->hide();
    QVBoxLayout* form = new QVBoxLayout;
    form->setContentsMargins(2,0,1,0);
    parameters_widget->setLayout(form);

    QList<parameter> parameters = temp->get_list();
    QList<QString> names = temp->get_names();

    connect(a, SIGNAL(toggled(bool)), parameters_widget, SLOT(setVisible(bool)));

    int i = 0;
    foreach(parameter p, parameters)
    {
        QHBoxLayout *row = new QHBoxLayout;
        QLabel *p_name = new QLabel(names[p.index]);
        row->addWidget(p_name,1);

        if(p.type == ON_OFF) {
            QCheckBox* p_val = createCheckBox(names[p.index], i, 0);
            connect(p_val, SIGNAL(toggled(bool)), SLOT(control_device(bool)));
            row->addWidget(p_val,1);
        }
        else {
            QAbstractSpinBox* p_val = nullptr;
            switch (p.type) {
            case TEMPERATURE:
                p_val = createSpinBox(" degrees", p.value.i, 0, 100, i, 0);
                connect(p_val, SIGNAL(valueChanged(int)), SLOT(control_device(int)));
                break;
            case PERCENT:
                p_val = createSpinBox(" percents", p.value.i, 0, 100, i, 0);
                connect(p_val, SIGNAL(valueChanged(int)), SLOT(control_device(int)));
                break;
            case COEFF:
                p_val = createDoubleSpinBox("", p.value.f, -1, 1, i, 0);
                connect(p_val, SIGNAL(valueChanged(double)), SLOT(control_device(double)));
                break;
            case F_SIZE:
                p_val = createDoubleSpinBox(" meters", p.value.f, 0, 100, i, 0);
                connect(p_val, SIGNAL(valueChanged(double)), SLOT(control_device(double)));
            }
            row->addWidget(p_val,1);
        }

        form->addLayout(row);
        i++;
    }
    vertical->addWidget(parameters_widget);

    if (s != nullptr) {
        QWidget* rules_widget = new QWidget;
        rules_widget->setLayout(new QVBoxLayout);
        rules_widget->layout()->setContentsMargins(0,3,0,0);
        rules_widget->hide();
        vertical->addWidget(rules_widget);

        connect(s, SIGNAL(toggled(bool)), rules_widget, SLOT(setVisible(bool)));
    }

    QVBoxLayout* l = qobject_cast<QVBoxLayout*>(ui->componentsListScrollArea->layout());
    l->setStretch(l->count() - 1, 0);
    l->addWidget(dev_widget, 1, Qt::AlignTop);
}

void control::control_device(int new_val)
{
    QSpinBox *par = qobject_cast<QSpinBox*>(sender());
    QWidget *comp = qobject_cast<QWidget*>(par->parentWidget()->parentWidget());

    int i = par->property("Par_index").value<int>();
    device* to_control = manager.k->get_device(comp->property("id").value<int>());
    val v;
    v.i = new_val;
    to_control->to_be_controlled(i, v);
}

void control::control_device(double new_val)
{
    QDoubleSpinBox *par = qobject_cast<QDoubleSpinBox*>(sender());
    QWidget *comp = qobject_cast<QWidget*>(par->parentWidget()->parentWidget());

    int i = par->property("Par_index").value<int>();
    device* to_control = manager.k->get_device(comp->property("id").value<int>());
    val v;
    v.f = static_cast<float> (new_val);
    to_control->to_be_controlled(i,v);
}

void control::control_device(bool new_val)
{
    QCheckBox *par = qobject_cast<QCheckBox*>(sender());
    QWidget *comp = qobject_cast<QWidget*>(par->parentWidget()->parentWidget());

    int i = par->property("Par_index").value<int>();
    device* to_control = manager.k->get_device(comp->property("id").value<int>());
    val v;
    v.b = new_val;
    to_control->to_be_controlled(i,v);
}

void control::edit_name(QString new_name)
{
    QWidget* comp = qobject_cast<QWidget*>(qobject_cast<QLineEdit*>(sender())->parentWidget());

    int id = comp->property("id").value<int>();

    device* component = manager.k->get_device(comp->property("id").value<int>());
    component->name = new_name;

    int i;
    QWidget *area;
    if (id > 0) {
         area = ui->sys_area;
    }
    else {

         area = ui->env_area;
    }
    i = manager.indexof(id);

    QGroupBox *l = qobject_cast<QGroupBox*>(area->layout()->itemAt(i)->widget());

    l->setTitle(new_name);
}

void control::upd_goal(int id)
{
    QList <goal> to_upd;
    QList <QString> names;
    QList <int> types;
    QString name;
    QVBoxLayout *l;

    if(id > 0) {
        record r = manager.k->sys_model[manager.k->indexof(id)];
        to_upd = r.goal_model;
        names = r.names;
        name = r.pointer->name;
        foreach (parameter temp, r.dev.par) {
            types.append(temp.type);
        }
        l = qobject_cast<QVBoxLayout*>(ui->sys_area->layout());
    }
    else {
        record r = manager.k->env_model[manager.k->indexof(id)];
        to_upd = r.goal_model;
        names = r.names;
        name = r.pointer->name;
        foreach (parameter temp, r.dev.par) {
            types.append(temp.type);
        }
        l = qobject_cast<QVBoxLayout*>(ui->env_area->layout());
    }

    QGroupBox *w = new QGroupBox(name);
    w->setProperty("id", QVariant(id));

    l->setStretch(l->count() - 1, 0);
    l->addWidget(w,1,Qt::AlignTop);

    l = new QVBoxLayout;
    l->setContentsMargins(5,5,5,5);
    w->setLayout(l);

    int i = 0;
    foreach(goal temp, to_upd)
    {
        QGridLayout *par = new QGridLayout;

        par->addWidget((new QLabel(names[temp.index])),0,0,1,2);

        if(types[temp.index] == ON_OFF) {
            QCheckBox* val = createCheckBox(names[temp.index], i, 0);
            val->setChecked(temp.value.b);
            connect(val, SIGNAL(toggled(bool)), SLOT(change_goal(bool)));
            par->addWidget(val, 1, 0, 1, 1);
        }
        else {
            QAbstractSpinBox* val = nullptr;
            switch (types[temp.index]) {
            case TEMPERATURE:
                val = createSpinBox(" degrees", temp.value.i, 0, 100, i, 0);
                connect(val, SIGNAL(valueChanged(int)), SLOT(change_goal(int)));
                break;
            case COEFF:
                val = createDoubleSpinBox("", temp.value.f, -1, 1, i, 0);
                connect(val, SIGNAL(valueChanged(int)), SLOT(control_device(int)));
                break;
            case PERCENT:
                val = createSpinBox(" percents", temp.value.i, 0, 100, i, 0);
                connect(val, SIGNAL(valueChanged(int)), SLOT(change_goal(int)));
                break;
            case F_SIZE:
                val = createDoubleSpinBox(" meters", temp.value.f, 0, 100, i, 0);
                connect(val, SIGNAL(valueChanged(double)), SLOT(change_goal(double)));
            }
            par->addWidget(val, 1, 0, 1, 1);
        }

        QCheckBox *c = new QCheckBox("Doesn't care");
        c->setChecked(temp.not_care);
        connect(c, SIGNAL(toggled(bool)), SLOT(goal_ignore(bool)));
        c->setProperty("Par_index", QVariant(i));
        par->addWidget(c, 1, 1, 1, 1);

        i++;
        l->addLayout(par);
    }
    qobject_cast<QMenuBar*>(layout()->menuBar())->actions()[3]->setEnabled(1);
}

void control::change_goal(int value)
{
    int i = sender()->property("Par_index").value<int>();
    int id = qobject_cast<QWidget*>(sender())->parentWidget()->property("id").value<int>();
    val v;
    v.i = value;
    manager.k->update_goal(id,i,v);
}

void control::change_goal(double value)
{
    int i = sender()->property("Par_index").value<int>();
    int id = qobject_cast<QWidget*>(sender())->parentWidget()->property("id").value<int>();
    val v;
    v.f = static_cast<float> (value);
    manager.k->update_goal(id,i,v);
}

void control::change_goal(bool value)
{
    int i = sender()->property("Par_index").value<int>();
    int id = qobject_cast<QWidget*>(sender())->parentWidget()->property("id").value<int>();
    val v;
    v.b = value;
    manager.k->update_goal(id,i,v);
}

void control::goal_ignore(bool not_care)
{
    int i = sender()->property("Par_index").value<int>();
    int id = qobject_cast<QCheckBox*>(sender())->parentWidget()->property("id").value<int>();
    manager.k->goal_ignore(id,i,not_care);
}

void control::add_rule()
{
    QWidget* dev_w = qobject_cast<QPushButton*>(sender())->parentWidget();
    int id = dev_w->property("id").value<int>();
    effector* temp = qobject_cast<effector*>(manager.k->get_device(id));

    int index = manager.indexof(id);

    rule_editing *edit = new rule_editing;    
    edit->init(manager.k, index);
    edit->show();

    if(edit->exec() == QDialog::Accepted)
    {
        rule r = edit->get_rule();
        if((!r.operation.isEmpty()) && (!r.pre.isEmpty())) {
            temp->add_rule(r, false);

            dev_w->layout()->itemAt(1)->layout()->itemAt(2)->widget()->setEnabled(1);

            QVBoxLayout* rules_layout = qobject_cast<QVBoxLayout*>(dev_w->layout()->itemAt(3)->widget()->layout());
            int i = temp->ruleset.size() - 1;

            add_rule_widget(rules_layout, i);

        }
    }
    delete edit;
}



void control::add_rule_widget(QVBoxLayout* rules_layout, int i)
{
    QHBoxLayout* rule = new QHBoxLayout;

    rule->addWidget(new QLabel ("Rule " + QString::number(i)), 0, Qt::AlignHCenter);

    QPushButton* show = new QPushButton ("Show");
    show->setProperty("Rule_index", QVariant(i));
    connect(show, SIGNAL(clicked()), SLOT(show_rule()));

    QPushButton* delet = new QPushButton ("Delete");
    delet->setProperty("Rule_index", QVariant(i));
    connect(delet, SIGNAL(clicked()), SLOT(delete_rule()));

    rule->addWidget(show);
    rule->addWidget(delet);

    rules_layout->addLayout(rule);
}

void control::show_rule()
{
    effector* temp = qobject_cast<effector*>(manager.k->get_device(sender()->parent()->parent()->property("id").value<int>()));

    int i = sender()->property("Rule_index").value<int>();
    rule r = temp->ruleset[i];

    QString text;

    text.append("\nPrecondition:\n\n");
    foreach (condition temp_c, r.pre) {
        device* temp_d = manager.k->get_device(temp_c.dev_id);
        text.append(" " + temp_d->get_names()[temp_c.p.index] + " of " + temp_d->name);
        switch (temp_c.p.type) {
        case EQUAL:
            text.append(" is ");
            break;
        case LESS:
            text.append(" less than ");
            break;
        case LARGER:
            text.append(" greater than ");
        }
        switch (temp_d->get_list()[temp_c.p.index].type) {
            case TEMPERATURE:
            case PERCENT:
                text.append(QString::number(temp_c.p.value.i) + "\n");
                break;
            case COEFF:
            case F_SIZE:
                text.append(QString::number(static_cast<double>(temp_c.p.value.f)) + "\n");
                break;
            case ON_OFF:
                text.append(QString::number(temp_c.p.value.b) + "\n");
                break;
        }
    }

    text.append("\nOperation:\n\n");
    foreach (parameter temp_i, r.operation) {
        QList<QString> names = temp->get_names();
        switch (temp_i.type) {
        case ASSIGN:
            text.append(" Set " + names[temp_i.index] + " to ");
            break;
        case DECREASE:
            text.append(" Decrease " + names[temp_i.index] + " by ");
            break;
        case INCREASE:
            text.append(" Increase " + names[temp_i.index] + " by ");
        }

        switch (temp->get_list()[temp_i.index].type) {
            case TEMPERATURE:
            case PERCENT:
                text.append(QString::number(temp_i.value.i) + "\n");
                break;
            case COEFF:
            case F_SIZE:
                text.append(QString::number(static_cast<double>(temp_i.value.f)) + "\n");
                break;
            case ON_OFF:
                text.append(QString::number(temp_i.value.b) + "\n");
                break;
        }
    }

    if(r.last_use < 0)
        text.append("\nThis rule wasn't applied yet ");
    else
        text.append("\nThe last time the rule was applied in loop " + QString::number(r.last_use));

    text.append("\nCurrent failure rate of rule is " + QString::number(r.failure_rate));

    text.append("\nCurrent max. frequency of application: " + QString::number(r.period));
    rule_text* r_t = new rule_text;
    r_t->setText(text, temp->name + ": rule " + QString::number(i));
    r_t->show();
    connect(qobject_cast<QPushButton*>(sender()), SIGNAL(clicked()), r_t, SLOT(deleteLater()));
}

void control::add_gen_rule(int id, int i)
{
    QVBoxLayout* l = qobject_cast<QVBoxLayout*>(ui->componentsListScrollArea->layout());
    for(int j = 0; j < l->count(); j++)
    {
        QWidget* w = l->itemAt(j)->widget();
        if(w->property("id").value<int>() == id)
        {
            QVBoxLayout* rules_layout = qobject_cast<QVBoxLayout*>(w->layout()->itemAt(3)->widget()->layout());
            add_rule_widget(rules_layout, i);
            w->layout()->itemAt(1)->layout()->itemAt(2)->widget()->setEnabled(1);
            break;
        }
    }
}

void control::delete_rule()
{
    QWidget* dev_rules = qobject_cast<QWidget*>(sender()->parent());
    effector* dev = qobject_cast<effector*>(manager.k->get_device(dev_rules->parent()->property("id").value<int>()));

    dev->delete_rule(sender()->property("Rule_index").value<int>());

    int n = dev_rules->layout()->count() - 1;
    QLayoutItem* del_rule = dev_rules->layout()->takeAt(n);

    QLayoutItem *child;
    while ((child = del_rule->layout()->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    delete del_rule;

    if (!n) {
        QPushButton* show_r = qobject_cast<QPushButton*>(dev_rules->parentWidget()->layout()->itemAt(1)->layout()->itemAt(2)->widget());
        show_r->toggle();
        show_r->setEnabled(0);
    }
}

void control::delete_rule(int id)
{
    QVBoxLayout* l = qobject_cast<QVBoxLayout*>(ui->componentsListScrollArea->layout());
    for(int j = 0; j < l->count(); j++)
    {
        QWidget* w = l->itemAt(j)->widget();
        if(w->property("id").value<int>() == id)
        {
            QVBoxLayout* rules_layout = qobject_cast<QVBoxLayout*>(w->layout()->itemAt(3)->widget()->layout());

            int n = rules_layout->count() - 1;
            QLayoutItem* del_rule = rules_layout->takeAt(n);

            QLayoutItem *child;
            while ((child = del_rule->layout()->takeAt(0)) != nullptr) {
                delete child->widget();
                delete child;
            }
            delete del_rule;

            if (!n) {
                QPushButton* show_r = qobject_cast<QPushButton*>(w->layout()->itemAt(1)->layout()->itemAt(2)->widget());
                show_r->toggle();
                show_r->setEnabled(0);
            }

            break;
        }
    }
}

void control::loop(int n)
{
    for(int j = 0; j < n; j++)
    {
        foreach(record temp, manager.k->env_model)
        {
            i.sense(qobject_cast<sensor*>(temp.pointer));
        }
        manager.loop();
        foreach(record temp, manager.k->sys_model)
        {
            i.effect(qobject_cast<effector*>(temp.pointer));
        }
        i.calculate_physics();
    }
    w.status();
}

void control::open_file()
{
    if(ui->componentsListScrollArea->layout()->count() > 0)
    {
        QMessageBox* are_you_sure = new QMessageBox(QMessageBox::Warning, "SmartHome",
                                                "All unsaved changes will be lost. Do you want to save changes to the current file?",
                                                QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        are_you_sure->show();
        int b = are_you_sure->exec();
        delete are_you_sure;
        if(b == QMessageBox::Cancel) return;
        if(b == QMessageBox::Yes) save_slot();
    }

    file_path = QFileDialog::getOpenFileName(nullptr, "Open Home", "", "*.shm");

    if (!file_path.isEmpty())
    {
        QFile* f = new QFile(file_path);
        if (f->open(QIODevice::ReadOnly)) {
            QByteArray i_arr = f->read(sizeof(double) * 5);
            if (i_arr.size() == sizeof(double) * 5) {
                int err = manager.import_knowledge(f);
                if (err == 0)
                {
                    i.set(i_arr);
                    f->close();
                    delete f;
                    delete_all_dev_widgets();
                    create_all_dev_widgets();
                    connect(manager.k, SIGNAL(r_del(int)), SLOT(delete_rule(int)));
                    return;
                }
            }

            QMessageBox* error = new QMessageBox(QMessageBox::Critical,"Error","Error during loading data from file. Corrupted file");
            error->show();
            error->exec();
            delete error;
        }
        else {
            QMessageBox* error = new QMessageBox(QMessageBox::Critical,"Error","Error during opening. Could not open file");
            error->show();
            error->exec();
            delete error;
        }
        f->close();
        delete f;
    }
}

void control::save_slot()
{
    if(file_path.isEmpty())
    {
        save_to_new_file();
    }
    else
    {
        save();
    }
}

void control::save_to_new_file()

{
    file_path = QFileDialog::getSaveFileName(nullptr, "Save Home", "Home", "*.shm");
    if (!file_path.isEmpty())
    {
        save();
    }
}

void control::save()
{
    QFile* f = new QFile(file_path);
    if (f->open(QIODevice::WriteOnly)) {
        QByteArray* i_arr = i.get();
        f->write(*i_arr);
        delete i_arr;

        manager.k->save(f);

        f->close();
        delete f;
    }
    else {
        QMessageBox* error = new QMessageBox(QMessageBox::Critical,"Error","Error during saving. Could not open file");
        error->show();
        error->exec();
        delete error;
    }

}

void control::delete_all_dev_widgets()
{
   delete ui->componentsListScrollArea;
   ui->componentsListScrollArea = new QWidget;
   ui->componentsListScrollArea->setLayout(new QVBoxLayout);
   ui->componentsScrollArea->setWidget(ui->componentsListScrollArea);

   delete ui->sys_area;
   ui->sys_area = new QWidget;
   ui->sys_area->setLayout(new QVBoxLayout);
   ui->sys_area->layout()->setContentsMargins(5,10,5,5);
   ui->sys_area->layout()->setSpacing(0);
   ui->scrollArea_2->setWidget(ui->sys_area);

   delete ui->env_area;
   ui->env_area = new QWidget;
   ui->env_area->setLayout(new QVBoxLayout);
   ui->env_area->layout()->setContentsMargins(5,10,5,5);
   ui->env_area->layout()->setSpacing(0);
   ui->scrollArea->setWidget(ui->env_area);
}

void control::create_all_dev_widgets()
{
    int i = 0;
    foreach(record temp, manager.k->env_model)
    {
        add_dev_widget(temp.pointer, temp.dev.id);
        upd_goal(temp.dev.id);
        i++;
    }
    foreach(record temp, manager.k->sys_model)
    {
        add_dev_widget(temp.pointer, temp.dev.id);
        upd_goal(temp.dev.id);

        int j = 0;
        foreach(rule temp_r, qobject_cast<effector*>(temp.pointer)->ruleset)
        {
            add_rule_widget(qobject_cast<QVBoxLayout*>(ui->componentsListScrollArea->layout()->itemAt(i)->widget()->layout()->itemAt(3)->widget()->layout()), j);
            ui->componentsListScrollArea->layout()->itemAt(i)->widget()->layout()->itemAt(1)->layout()->itemAt(2)->widget()->setEnabled(1);
            j++;
        }
        i++;
    }
}
