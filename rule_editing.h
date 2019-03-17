#ifndef RULE_EDITING_H
#define RULE_EDITING_H

#include <QDialog>
#include <knowledge.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>

//Интерфейс редактирования правил

namespace Ui {
class rule_editing;
}

class rule_editing : public QDialog
{
    Q_OBJECT

public:
    explicit rule_editing(QWidget *parent = nullptr);
    void init(knowledge *k, int eff_index);
    ~rule_editing();
    rule get_rule();

private:
    Ui::rule_editing *ui;
    rule r;
    int stage;
private slots:
    void add_cond();
    void change_cond_val(int new_val);
    void change_cond_val(bool new_val);
    void change_cond_mode(int mode);

    void add_instr();
    void change_op_mode(int mode);
    void change_new_val(int new_val);
    void change_new_val(bool new_val);

    void interstage();
    void change_period(int);
};

QCheckBox* createCheckBox(QString text, int par_index, int id);
QSpinBox* createSpinBox(QString text, int value, int min, int max, int par_index, int id);

#endif // RULE_EDITING_H
