#include "control.h"
#include "ui_control.h"

control::control(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::control)
{
    ui->setupUi(this);

    //ui->addwidget->hide();
    //ui->Goal_widget->hide();
    //ui->start->setDisabled(1);

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
    QAction* imit_edit = m->addAction("Edit imitation parametres");
    connect(imit_edit, SIGNAL(triggered()), i.get_ctrl_pointer(), SLOT(show()));

    QAction* start = m->addAction("Start");

    connect(start, SIGNAL(triggered()), &w, SLOT(show()));
    start->setEnabled(0);

    //connect(ui->start, SIGNAL(clicked(bool)), ui->set, SIGNAL(clicked(bool)));
    //По нажатию старта открывается окно и выключается добавление

    //connect(ui->add, SIGNAL(clicked()), ui->addwidget, SLOT(show()));
    //connect(ui->add, SIGNAL(clicked()), ui->Goal_widget, SLOT(hide()));
    //По нажатию add добавится компонент и закроется окно с целью

    //connect(ui->set, SIGNAL(clicked()), ui->Goal_widget, SLOT(show()));
    //connect(ui->set, SIGNAL(clicked()), ui->addwidget, SLOT(hide()));
    //По нажатую set открывается редактирование цели
    //connect(ui->pushButton_3, SIGNAL(clicked(bool)),ui->pushButton_3, SLOT(setEnabled(bool)));

    //connect(ui->buttonGroup, SIGNAL(buttonClicked(QAbstractButton*)), SLOT(add_record(QAbstractButton*)));
    //Кнопки добавления компонентов

    connect(manager.k, SIGNAL(added(int)), SLOT(upd_goal(int)));

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
    //Получаем тип

    device* temp;
    temp = new_device(type);
    temp->name = qobject_cast<QAction*>(sender())->text() + " " + QString::number(manager.k->id_counter);

    //Создаем объект и даем ему имя

    add_dev_widget(temp, manager.add_device(temp));
}

void control::add_dev_widget(device* temp, int id)
{   
    QWidget* dev_widget = new QWidget();
    //И виджет

    QVBoxLayout *vertical = new QVBoxLayout;
    dev_widget->setLayout(vertical);
    //Лейаут для виджета

    QHBoxLayout *check_and_name = new QHBoxLayout;
    //Специальный лэйаут для имени и галочки

    QLineEdit *name = new QLineEdit(temp->name);
    connect(name, SIGNAL(textEdited(QString)), SLOT(edit_name(QString)));
    //Строка с именем компонента, слот для редактирования

    QCheckBox *c = new QCheckBox("Broken");
    connect(c, SIGNAL(toggled(bool)), temp, SLOT(set_broken(bool)));
    //Галочка, чтобы ломать компонент

    check_and_name->addWidget(name);
    check_and_name->addWidget(c);
    //Добавляем в строку имя и галочку

    vertical->addLayout(check_and_name);
    //Строку в виджет

    QHBoxLayout *buttons = new QHBoxLayout;
    //Специальный лэйаут для кнопок

    QPushButton* a = new QPushButton("Parametres");
    a->setCheckable(1);
    //Можно нажать и отжать

    buttons->addWidget(a);
    //Добавляем одну кнопку

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
    //Добавляем в лейаут кнопки

    QWidget* parametres_widget = new QWidget;
    parametres_widget->hide();
    QVBoxLayout* form = new QVBoxLayout;
    form->setContentsMargins(2,0,1,0);
    parametres_widget->setLayout(form);
    //Добавляем параметры

    QList<parameter> parametres = temp->get_list();
    QList<QString> names = temp->get_names();
    //Получаем от устройства список параметров и значений

    connect(a, SIGNAL(toggled(bool)), parametres_widget, SLOT(setVisible(bool)));

    int i = 0;
    foreach(parameter p, parametres)
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
    vertical->addWidget(parametres_widget);

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
    //Чтобы было красиво
}

/*void control::control_device(QString new_val)
{
    QLineEdit *par = qobject_cast<QLineEdit*>(sender());
    //Строка, которую меняют
    QWidget *comp = qobject_cast<QWidget*>(par->parentWidget()->parentWidget());
    //Девайс виджет, который это вызвал
    int i = par->property("Par_index").value<int>();
    //Номер параметра, указанный в динамическом свойстве строки
    device* to_control = manager.k->get_device(comp->property("id").value<int>());
    //Найти девайс по id
    to_control->to_be_controlled(i,new_val);
    //Поихали
}*/

void control::control_device(int new_val)
{
    QSpinBox *par = qobject_cast<QSpinBox*>(sender());
    QWidget *comp = qobject_cast<QWidget*>(par->parentWidget()->parentWidget());
    //Девайс виджет, который это вызвал
    int i = par->property("Par_index").value<int>();
    //Номер параметра, указанный в динамическом свойстве
    device* to_control = manager.k->get_device(comp->property("id").value<int>());
    //Найти девайс по id
    val v;
    v.i = new_val;
    to_control->to_be_controlled(i, v);
    //Поихали
}

void control::control_device(double new_val)
{
    QDoubleSpinBox *par = qobject_cast<QDoubleSpinBox*>(sender());
    QWidget *comp = qobject_cast<QWidget*>(par->parentWidget()->parentWidget());
    //Девайс виджет, который это вызвал
    int i = par->property("Par_index").value<int>();
    //Номер параметра, указанный в динамическом свойстве
    device* to_control = manager.k->get_device(comp->property("id").value<int>());
    //Найти девайс по id
    val v;
    v.f = static_cast<float> (new_val);
    to_control->to_be_controlled(i,v);
    //Поихали
}

void control::control_device(bool new_val)
{
    QCheckBox *par = qobject_cast<QCheckBox*>(sender());
    QWidget *comp = qobject_cast<QWidget*>(par->parentWidget()->parentWidget());
    //Девайс виджет, который это вызвал
    int i = par->property("Par_index").value<int>();
    //Номер параметра, указанный в динамическом свойстве
    device* to_control = manager.k->get_device(comp->property("id").value<int>());
    //Найти девайс по id
    val v;
    v.b = new_val;
    to_control->to_be_controlled(i,v);
    //Поихали
}

void control::edit_name(QString new_name)
{
    QWidget* comp = qobject_cast<QWidget*>(qobject_cast<QLineEdit*>(sender())->parentWidget());
    //Получаем виджет компонента

    int id = comp->property("id").value<int>();
    //Запоминаем его id

    device* component = manager.k->get_device(comp->property("id").value<int>());
    component->name = new_name;
    //Сменили имя объекту

    int i;
    QWidget *area;
    if (id > 0) {
         area = ui->sys_area;
    }
    else {

         area = ui->env_area;
    }
    //Узнали, что за компонент
    i = manager.indexof(id);
    //Узнали его индекс

    QGroupBox *l = qobject_cast<QGroupBox*>(area->layout()->itemAt(i)->widget());
    //Нашли его бокс с целью

    l->setTitle(new_name);
    //Поменяли боксу имя

    /*
    size = l->count();
    //Узнали, сколько параметров
    for(i = 0; i < size; i++)
    {
        QString text;
        QLabel *to_replace;
        to_replace = qobject_cast<QLabel*>(l->itemAt(i)->widget()->layout()->itemAt(0)->widget());
        text = to_replace->text();
        to_replace->setText(new_name + ":" + text.split(':')[1]);
    }
    */
    //И каждому сменили имя
}

void control::upd_goal(int id)
{
    QList <goal> to_upd;
    //Запись для добавления
    QList <QString> names;
    QList <int> types;
    QString name;
    QVBoxLayout *l;
    //Лэйаут куда будем добавлять

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
    //Забираем нужную запись и лэйаут

    QGroupBox *w = new QGroupBox(name);
    w->setProperty("id", QVariant(id));
    //Создаем групбокс

    l->setStretch(l->count() - 1, 0);
    l->addWidget(w,1,Qt::AlignTop);

    l = new QVBoxLayout;
    l->setContentsMargins(5,5,5,5);
    w->setLayout(l);
    //В нужный лейаут добавляем бокс и меняем л на его лэйаут

    int i = 0;
    foreach(goal temp, to_upd)
    {
        QGridLayout *par = new QGridLayout;
        //Лейаут под параметр

        par->addWidget((new QLabel(names[temp.index])),0,0,1,2);
        //Его имя

        if(types[temp.index] == ON_OFF) {
            QCheckBox* val = createCheckBox(names[temp.index], i, 0);
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
        connect(c, SIGNAL(toggled(bool)), SLOT(goal_ignore(bool)));
        c->setProperty("Par_index", QVariant(i));
        par->addWidget(c, 1, 1, 1, 1);
        //Чекбокс

        i++;
        l->addLayout(par);
    }
    //Для каждого параметра создаем горизонтальный лэйаут с названием, значением и галочкой

    qobject_cast<QMenuBar*>(layout()->menuBar())->actions()[3]->setEnabled(1);
}

/*void control::change_goal(QString val)
{
    int i = sender()->property("Par_index").value<int>();
    int id = qobject_cast<QLineEdit*>(sender())->parentWidget()->property("id").value<int>();
    manager.k->update_goal(id,i,val);
}*/

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
    //Находим id и запоминаем девайс

    int index = manager.indexof(id);
    //Узнаем его индекс

    rule_editing *edit = new rule_editing;    
    edit->init(manager.k, index);
    edit->show();
    //Открываем окно

    if(edit->exec() == QDialog::Accepted)
    {
        rule r = edit->get_rule();
        if((!r.operation.isEmpty()) && (!r.pre.isEmpty())) {
            temp->add_rule(r);

            dev_w->layout()->itemAt(1)->layout()->itemAt(2)->widget()->setEnabled(1);
            //Включаем кнопку показа списка правил

            QVBoxLayout* rules_layout = qobject_cast<QVBoxLayout*>(dev_w->layout()->itemAt(3)->widget()->layout());
            int i = temp->ruleset.size() - 1;
            //Индекс правила

            add_rule_widget(rules_layout, i);

        }
        //Добавляем
    }
    delete edit;
}

void control::add_rule_widget(QVBoxLayout* rules_layout, int i)
{
    QHBoxLayout* rule = new QHBoxLayout;
    //Лэйаут под правило

    rule->addWidget(new QLabel ("Rule " + QString::number(i)), 0, Qt::AlignHCenter);
    //Название правила

    QPushButton* show = new QPushButton ("Show");
    show->setProperty("Rule_index", QVariant(i));
    connect(show, SIGNAL(clicked()), SLOT(show_rule()));
    //Кнопка показа правила

    QPushButton* delet = new QPushButton ("Delete");
    delet->setProperty("Rule_index", QVariant(i));
    connect(delet, SIGNAL(clicked()), SLOT(delete_rule()));
    //Кнопка удаления правила

    rule->addWidget(show);
    rule->addWidget(delet);

    rules_layout->addLayout(rule);
}

void control::show_rule()
{
    effector* temp = qobject_cast<effector*>(manager.k->get_device(sender()->parent()->parent()->property("id").value<int>()));

    int i = sender()->property("Rule_index").value<int>();
    rule r = temp->ruleset[i];
    //Эффектор и его правило


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

    text.append("\nThe max. frequency of application: " + QString::number(r.period));
    rule_text* r_t = new rule_text;
    r_t->setText(text, temp->name + ": rule " + QString::number(i));
    r_t->show();
    connect(qobject_cast<QPushButton*>(sender()), SIGNAL(clicked()), r_t, SLOT(deleteLater()));
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

void control::loop(int n)
{
    for(int j = 0; j < n; j++)
    {
        manager.loop();
        foreach(record temp, manager.k->sys_model)
        {
            i.effect(qobject_cast<effector*>(temp.pointer));
        }
        i.calculate_physics();
        foreach(record temp, manager.k->env_model)
        {
            i.sense(qobject_cast<sensor*>(temp.pointer));
        }
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
    int i = 0;;
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
            j++;
        }
        i++;
    }
}
