#ifndef OPTION_H
#define OPTION_H

#include <QDialog>
#include <QStandardItemModel>
#include <QList>

namespace Ui {
class Option;
}

class Option : public QDialog
{
    Q_OBJECT

public:
    explicit Option(QWidget *parent = nullptr);
    ~Option();
    int row, col;
    void accept();
    QList<int> inputs;
    int output;
    bool cleaner;
public slots:
    void addInput();
    void removeInput();

private:
    QStandardItemModel * model;
    Ui::Option *ui;
    void initializeTable();

};

#endif // OPTION_H
