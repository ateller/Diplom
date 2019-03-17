#include "rule_text.h"
#include "ui_rule_text.h"

rule_text::rule_text(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::rule_text)
{
    ui->setupUi(this);
}

rule_text::~rule_text()
{
    delete ui;
}

void rule_text::setText(QString text, QString title)
{
    ui->label->setText(text);
    setWindowTitle(title);
    if(sizeHint().width() < size().width()) {
        setFixedSize(size().width(), sizeHint().height());
    }
    else {
        setFixedSize(sizeHint());
    }
}
