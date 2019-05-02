#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("Monitor: loop 0");
}

Widget::~Widget()
{
    delete ui;
}

void Widget::set(mape_loop *pointer)
{
    manager = pointer;
    connect(ui->pushButton, SIGNAL(clicked(bool)), SLOT(loop()));
    connect(manager, SIGNAL(plan_completed(int)), SLOT(plan(int)));
    connect(manager, SIGNAL(executed(int)), SLOT(execute(int)));
    ui->groupBox->setLayout(new QVBoxLayout);
    ui->groupBox_2->setLayout(new QVBoxLayout);
}

QString Widget::create_par_string(parameter p)
{
    QString s;
    switch (p.type) {
    case TEMPERATURE:
    case PERCENT:
        s = QString::number(p.value.i);
        break;
    case COEFF:
    case F_SIZE:
        s = QString::number(static_cast <double> (p.value.f));
        break;
    case ON_OFF:
        s = QString::number(p.value.b);
        break;
    }
    return s;
}

void Widget::create_dev_par_labels(QLayout *l, record r)
{
    QWidget *w = new QWidget;
    QVBoxLayout *v = new QVBoxLayout;
    w->setLayout(v);
    l->addWidget(w);
    parameter p;
    foreach (p, r.dev.par)
    {
        v->addWidget(new QLabel(r.pointer->name + ":" + r.names[p.index] + ": " + create_par_string(p)));
    }
}

void Widget::monitor()
{
    int i, size;
    QLayout *l = ui->groupBox->layout();
    size = l->count();
    for(i = 0; i< size; i++)
    {
        QLayoutItem *it = l->takeAt(0);
        delete it->widget();
        delete it;
    }
    record temp;
    foreach(temp, manager->k->sys_model) {
        create_dev_par_labels(l, temp);
    }
    l = ui->groupBox_2->layout();
    size = l->count();
    for(i = 0; i< size; i++)
    {
        QLayoutItem *it = l->takeAt(0);
        delete it->widget();
        delete it;
    }
    foreach(temp, manager->k->env_model) {
       create_dev_par_labels(l, temp);
    }
}

void Widget::analysis()
{
    QString status = "Analysis status: tolerance = " + QString::number(manager->tolerance) + ", distance = " + QString::number(manager->dist);
    if(manager->dist > manager->tolerance)
        status+=", adaptation is necessary";
    else {
        status+=", adaptation is not necessary";
    }
    ui->label->setText(status);
}

void Widget::plan(int n)
{
    planned_counter+=n;
    p_last = n;
}

void Widget::execute(int n)
{
    executed_counter+=n;
    ex_last = n;
}

void Widget::loop()
{
    executed_counter = 0;
    planned_counter = 0;
    p_last = 0;
    ex_last = 0;
    emit loop(ui->spinBox->value());
}

void Widget::status()
{
    monitor();
    analysis();
    ui->label_2->setText("Plan status: " + QString::number(planned_counter) + " rules were planned for execution, " + QString::number(p_last) + " on last loop");
    ui->label_3->setText("Execute status: " + QString::number(executed_counter) + " rules executed, " + QString::number(ex_last) + " on last loop");
    setWindowTitle("Monitor: loop " + QString::number(manager->k->loops_counter) + ", or " + QString::number(manager->k->loops_counter * 5) + " seconds from the start");
}
