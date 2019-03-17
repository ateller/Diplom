#ifndef RULE_TEXT_H
#define RULE_TEXT_H

#include <QWidget>

namespace Ui {
class rule_text;
}

class rule_text : public QWidget
{
    Q_OBJECT

public:
    explicit rule_text(QWidget *parent = nullptr);
    ~rule_text();
    void setText(QString text, QString title);

private:
    Ui::rule_text *ui;
};

#endif // RULE_TEXT_H
