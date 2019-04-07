#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <mape_loop.h>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    void set(mape_loop*);
    ~Widget();

private:
    Ui::Widget *ui;
    mape_loop *manager;
    int executed_counter, ex_last;
    int planned_counter, p_last;
    QString create_par_string(parameter p);
    void create_dev_par_labels(QLayout *l, record r);
public slots:
    void monitor();
    void analysis();
    void plan(int n);
    void execute(int n);
    void loop();
    void status();
signals:
    void loop(int);
};

#endif // WIDGET_H
