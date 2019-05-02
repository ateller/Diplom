#include "rule_editing.h"
#include "ui_rule_editing.h"

rule_editing::rule_editing(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::rule_editing)
{
    ui->setupUi(this);
    r.period = 0;
}

void rule_editing::init(knowledge* k, int eff_index)
{
    record temp;
    QVBoxLayout *g1 = new QVBoxLayout;
    ui->scrollAreaWidgetContents->setLayout(g1);
    //В область условия добавляем лэйаут

    foreach(temp, k->sys_model)
    {
        goal p;
        int i = 0;
        foreach (p, temp.goal_model)
        {
            QWidget* condition = new QWidget;
            condition->setProperty("id", QVariant(temp.dev.id));
            condition->setProperty("Par_index", QVariant(i));

            QHBoxLayout* l = new QHBoxLayout;
            condition->setLayout(l);
            //Под условие лэйаут

            QPushButton *add = new QPushButton("Add to rule");
            connect(add, SIGNAL(clicked()), SLOT(add_cond()));
            connect(add, SIGNAL(clicked(bool)), add, SLOT(setEnabled(bool)));
            l->addWidget(add,1);
            //Кнопка добавления


            l->addWidget(new QLabel(temp.pointer->name + ": " + temp.names[p.index]),2,Qt::AlignHCenter);
            //Название

            if(temp.dev.par[p.index].type == ON_OFF) {
                QCheckBox* val = createCheckBox(temp.names[p.index], i, 0);
                val->setDisabled(1);
                l->addWidget(val,2);
            }
            else {
                QAbstractSpinBox* val = nullptr;
                switch (temp.dev.par[p.index].type) {
                case TEMPERATURE:
                    val = createSpinBox(" degrees", p.value.i, 0, 100, i, 0);
                    break;
                case PERCENT:
                    val = createSpinBox(" percents", p.value.i, 0, 100, i, 0);
                    break;
                case COEFF:
                    val = createDoubleSpinBox("", p.value.f, -1, 1, i, 0);
                    break;
                case F_SIZE:
                    val = createDoubleSpinBox(" meters", p.value.f, 0, 100, i, 0);
                }

                QComboBox *cond_type = new QComboBox;
                cond_type->addItem("Less then");
                cond_type->addItem("Greater then");
                cond_type->addItem("Equal");
                cond_type->setDisabled(1);
                l->addWidget(cond_type,1);
                l->addWidget(val,1);
                //Больше меньше равно
            }
            g1->addWidget(condition,0,Qt::AlignTop);
            i++;
        }
    }
    foreach(temp, k->env_model)
    {
        goal p;
        int i = 0;
        foreach (p, temp.goal_model)
        {
            QWidget* condition = new QWidget;
            condition->setProperty("id", QVariant(temp.dev.id));
            condition->setProperty("Par_index", QVariant(i));

            QHBoxLayout* l = new QHBoxLayout;
            condition->setLayout(l);
            //Под условие лэйаут

            QPushButton *add = new QPushButton("Add to rule");
            connect(add, SIGNAL(clicked()), SLOT(add_cond()));
            connect(add, SIGNAL(clicked(bool)), add, SLOT(setEnabled(bool)));
            l->addWidget(add,1);
            //Кнопка добавления


            l->addWidget(new QLabel(temp.pointer->name + ": " + temp.names[p.index]),2,Qt::AlignHCenter);
            //Название

            if(temp.dev.par[p.index].type == ON_OFF) {
                QCheckBox* val = createCheckBox(temp.names[p.index], i, 0);
                val->setDisabled(1);
                l->addWidget(val,2);
            }
            else {
                QAbstractSpinBox* val = nullptr;
                switch (temp.dev.par[p.index].type) {
                case TEMPERATURE:
                    val = createSpinBox(" degrees", p.value.i, 0, 100, i, 0);
                    break;
                case PERCENT:
                    val = createSpinBox(" percents", p.value.i, 0, 100, i, 0);
                    break;
                case COEFF:
                    val = createDoubleSpinBox("", p.value.f, -1, 1, i, 0);
                    break;
                case F_SIZE:
                    val = createDoubleSpinBox(" meters", p.value.f, 0, 100, i, 0);
                }

                QComboBox *cond_type = new QComboBox;
                cond_type->addItem("Less then");
                cond_type->addItem("Greater then");
                cond_type->addItem("Equal");
                cond_type->setDisabled(1);
                l->addWidget(cond_type,1);
                l->addWidget(val,1);
                //Больше меньше равно
            }
            g1->addWidget(condition,0,Qt::AlignTop);
            i++;
        }
    }
    g1->setStretch(g1->count()-1,1);

    QVBoxLayout *g2 = new QVBoxLayout;
    ui->groupBox_2->setLayout(g2);
    //Начинаем заполнять операцию

    temp = k->sys_model[eff_index];
    goal p;
    int i = 0;
    foreach(p, temp.goal_model)
    {
        QWidget *instr = new QWidget;
        QHBoxLayout *l = new QHBoxLayout;
        instr->setLayout(l);
        instr->setProperty("Par_index", QVariant(i));
        i++;

        QPushButton *add = new QPushButton("Add to rule");
        connect(add, SIGNAL(clicked(bool)), SLOT(add_instr()));
        connect(add, SIGNAL(clicked(bool)), add, SLOT(setEnabled(bool)));
        l->addWidget(add,1);
        //Кнопка

        l->addWidget(new QLabel(temp.names[p.index]), 2, Qt::AlignHCenter);
        //Название

        if(temp.dev.par[p.index].type == ON_OFF) {
            QCheckBox* val = createCheckBox(temp.names[p.index], i, 0);
            val->setDisabled(1);
            l->addWidget(val,2);
        }
        else {
            QAbstractSpinBox* val = nullptr;
            switch (temp.dev.par[p.index].type) {
            case TEMPERATURE:
                val = createSpinBox(" degrees", p.value.i, 0, 100, i, 0);
                break;
            case PERCENT:
                val = createSpinBox(" percents", p.value.i, 0, 100, i, 0);
                break;
            case COEFF:
                val = createDoubleSpinBox("", p.value.f, -1, 1, i, 0);
                break;
            case F_SIZE:
                val = createDoubleSpinBox(" meters", p.value.f, 0, 100, i, 0);
            }

            QComboBox *op_type = new QComboBox;
            op_type->addItem("Increase by");
            op_type->addItem("Decrease by");
            op_type->addItem("Assign");
            op_type->setDisabled(1);
            l->addWidget(op_type, 1);
            l->addWidget(val, 1);
        }
        g2->addWidget(instr, 0, Qt::AlignTop);
    }
    g2->setStretch(g2->count()-1,1);

    QHBoxLayout *period = new QHBoxLayout;
    period->addWidget(new QLabel ("The maximum frequency of rule application – once per"), 1, Qt::AlignRight);
    //Надпись

    QSpinBox *n = new QSpinBox();
    connect(n, SIGNAL(valueChanged(int)), SLOT(change_period(int)));
    n->setMinimum(1);
    n->setSuffix(" cycles");
    period->addWidget(n);
    //Счетчик

    g2->addLayout(period);

    stage = 2;
    //Счетчик ступеней

    ui->groupBox_2->hide();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Next");
    connect(ui->buttonBox, SIGNAL(accepted()), SLOT(interstage()));
}

rule_editing::~rule_editing()
{
    delete ui;
}

void rule_editing::change_period(int period)
{
    r.period = period;
}

void rule_editing::add_cond()
{
    QWidget *cond = qobject_cast<QWidget*>(sender()->parent());
    condition n;
    n.dev_id = cond->property("id").value<int>();
    n.p.index = cond->property("Par_index").value<int>();
    n.p.type = 0;
    //Определяем параметры условия

    cond->setProperty("Pre_index", QVariant(r.pre.size()));

    QLayout *par = cond->layout();
    int size = par->count();

    if(size == 4) {
        if(qobject_cast<QSpinBox*>(par->itemAt(3)->widget()) == nullptr)
        {
            QDoubleSpinBox* l = qobject_cast<QDoubleSpinBox*>(par->itemAt(3)->widget());
            n.p.value.f = static_cast<float> (l->value());
            l->setEnabled(1);
            connect(l, SIGNAL(valueChanged(double)), SLOT(change_cond_val(double)));
        }
        else {
            QSpinBox* l = qobject_cast<QSpinBox*>(par->itemAt(3)->widget());
            n.p.value.i = l->value();
            l->setEnabled(1);
            connect(l, SIGNAL(valueChanged(int)), SLOT(change_cond_val(int)));
        }

        QComboBox *type = qobject_cast<QComboBox*>(par->itemAt(2)->widget());
        connect(type, SIGNAL(currentIndexChanged(int)), SLOT(change_cond_mode(int)));
        type->setEnabled(1);
        //Включаем выбор
    }
    else {
        QCheckBox* l = qobject_cast<QCheckBox*>(par->itemAt(2)->widget());
        n.p.value.b = l->isChecked();
        l->setEnabled(1);
        connect(l, SIGNAL(toggled(bool)), SLOT(change_cond_val(bool)));

        n.p.type = EQUAL;
    }
    r.pre += n;
    //Добавляем
}
void rule_editing::change_cond_val(int new_val)
{
    QWidget *cond = qobject_cast<QWidget*>(sender()->parent());
    r.pre[cond->property("Pre_index").value<int>()].p.value.i = new_val;
}

void rule_editing::change_cond_val(double new_val)
{
    QWidget *cond = qobject_cast<QWidget*>(sender()->parent());
    r.pre[cond->property("Pre_index").value<int>()].p.value.f = static_cast <float> (new_val);
}

void rule_editing::change_cond_val(bool new_val)
{
    QWidget *cond = qobject_cast<QWidget*>(sender()->parent());
    r.pre[cond->property("Pre_index").value<int>()].p.value.b = new_val;
}

void rule_editing::change_cond_mode(int mode)
{
    QWidget *cond = qobject_cast<QWidget*>(sender()->parent());
    r.pre[cond->property("Pre_index").value<int>()].p.type = mode;
}

void rule_editing::add_instr()
{
    QWidget *instr = qobject_cast<QWidget*>(sender()->parent());
    parameter n;
    n.index = instr->property("Par_index").value<int>();
    n.type = 0;
    //Параметры

    instr->setProperty("Instr_index", QVariant(r.operation.size()));

    QLayout *par = instr->layout();
    int size = par->count();

    if(size == 4) {
        if(qobject_cast<QSpinBox*>(par->itemAt(3)->widget()) == nullptr)
        {
            QDoubleSpinBox* l = qobject_cast<QDoubleSpinBox*>(par->itemAt(3)->widget());
            n.value.f = static_cast<float> (l->value());
            l->setEnabled(1);
            connect(l, SIGNAL(valueChanged(double)), SLOT(change_new_val(double)));
        }
        else {
            QSpinBox* l = qobject_cast<QSpinBox*>(par->itemAt(3)->widget());
            n.value.i = l->value();
            l->setEnabled(1);
            connect(l, SIGNAL(valueChanged(int)), SLOT(change_new_val(int)));
        }

        QComboBox *type = qobject_cast<QComboBox*>(par->itemAt(2)->widget());
        connect(type, SIGNAL(currentIndexChanged(int)), SLOT(change_op_mode(int)));
        type->setEnabled(1);
        //Включаем выбор
    }
    else {
        QCheckBox* l = qobject_cast<QCheckBox*>(par->itemAt(2)->widget());
        n.value.b = l->isChecked();
        l->setEnabled(1);
        connect(l, SIGNAL(toggled(bool)), SLOT(change_new_val(bool)));

        n.type = ASSIGN;
    }
    r.operation += n;
}

QCheckBox *createCheckBox(QString text, int par_index, int id)
{
    QCheckBox* c = new QCheckBox(text);
    c->setProperty("Par_index", par_index);
    if(id) c->setProperty("id", id);
    return c;
}

QSpinBox* createSpinBox(QString text, int value, int min, int max, int par_index, int id)
{
    QSpinBox* c = new QSpinBox();

    c->setMaximum(max);
    c->setMinimum(min);
    c->setValue(value);
    c->setSuffix(text);
    c->setProperty("Par_index", par_index);
    if(id) c->setProperty("id", id);
    return c;
}

void rule_editing::change_new_val(int new_val)
{
    QWidget *instr = qobject_cast<QWidget*>(sender()->parent());
    r.operation[instr->property("Instr_index").value<int>()].value.i = new_val;
}

void rule_editing::change_new_val(double new_val)
{
    QWidget *instr = qobject_cast<QWidget*>(sender()->parent());
    r.operation[instr->property("Instr_index").value<int>()].value.f = static_cast<float> (new_val);
}

void rule_editing::change_new_val(bool new_val)
{
    QWidget *instr = qobject_cast<QWidget*>(sender()->parent());
    r.operation[instr->property("Instr_index").value<int>()].value.b = new_val;
}

void rule_editing::change_op_mode(int mode)
{
    QWidget *instr = qobject_cast<QWidget*>(sender()->parent());
    r.operation[instr->property("Instr_index").value<int>()].type = mode;
}

void rule_editing::interstage()
{
    if(stage == 2)
    {
        ui->groupBox->hide();
        ui->groupBox_2->show();
        ui->buttonBox->button(QDialogButtonBox::Ok)->setText("OK");
        stage = 1;
    }
    else
    {
        stage = 0;
        accept();
    }
}

rule rule_editing::get_rule()
{
    r.last_use = -r.period;
    return r;
}

QDoubleSpinBox *createDoubleSpinBox(QString text, float value, int min, int max, int par_index, int id)
{
    QDoubleSpinBox* c = new QDoubleSpinBox();

    c->setMaximum(max);
    c->setMinimum(min);
    c->setValue(static_cast<double>(value));
    c->setSuffix(text);
    c->setProperty("Par_index", par_index);
    if(id) c->setProperty("id", id);
    return c;
}
