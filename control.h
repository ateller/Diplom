#ifndef CONTROL_H
#define CONTROL_H

#include <QWidget>
#include "widget.h"
#include "imitation.h"
#include <QAbstractButton>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <rule_editing.h>
#include <QCheckBox>
#include <rule_text.h>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>

//Это интерфейс управления симуляцией. На нем все держится

namespace Ui {
class control;
}

class control : public QWidget
{
    Q_OBJECT

public:
    explicit control(QWidget *parent = nullptr);
    ~control();

private:
    Ui::control *ui;
    Widget w;
    mape_loop manager; //Петля. Понятно
    imitation i; //Имитация. Понятно
    QString file_path;

    void add_dev_widget(device*, int id);
    void add_rule_widget(QVBoxLayout* rules_layout, int i);
    void save();
    void delete_all_dev_widgets();
    void create_all_dev_widgets();
private slots:
    void add_record();
    void edit_name(QString);
    void upd_goal(int);
    //void change_goal(QString);
    //void control_device(QString);
    void add_rule();
    void goal_ignore(bool);
    void loop(int n);
    void show_rule();
    void delete_rule();
    void control_device(int);
    void control_device(bool);
    void control_device(double);
    void change_goal(int);
    void change_goal(double);
    void change_goal(bool);
    void open_file();
    void save_slot();
    void save_to_new_file();
public slots:
    void add_gen_rule(int id, int i);
signals:
    void done();
};

#endif // CONTROL_H
