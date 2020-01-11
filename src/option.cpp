#include "option.h"
#include "ui_option.h"
#include <QMessageBox>
#include <QDebug>
#include <QStandardItem>

Option::Option(QWidget *parent) :
    QDialog(parent),
    model(new QStandardItemModel(this)),
    ui(new Ui::Option)
{
    ui->setupUi(this);
    ui->tableView->setModel(model);

    initializeTable();
}



Option::~Option()
{
    delete ui;
}

#define CHECK_EDGE_FAIL(x, y) (x != 0 && y != 0 && x != col - 1 && y != row - 1)
#define CHECK_OUT_OF_RANGE(x, y) (x < 0 || x >= col || y < 0 || y >= row)

void Option::accept()
{

    // cleaner occupy (0, 0) and (col - 1, row - 1)
    cleaner = ui->checkBox->isChecked();

    // check size
    row = ui->spinBox_row->value();
    col = ui->spinBox_col->value();
    if(row < 3 && col < 3)
    {
        QMessageBox::critical(this, "size error", "row < 3 and col < 3, which is invaild!");
        return;
    }

    // check output port
    auto output_x = ui->spinbox_output_x->value() - 1;
    auto output_y = ui->spinbox_output_y->value() - 1;
    if(CHECK_EDGE_FAIL(output_x, output_y))
    {
        QMessageBox::critical(this, "output port error", "output port not on the edge!");
        return;
    }
    if(CHECK_OUT_OF_RANGE(output_x, output_y))
    {
        QMessageBox::critical(this, "output port error", "output port invaild!");
        return;
    }
    output = output_y * col + output_x;
    if(cleaner && ( output == 0 || output == (col * row - 1)))
    {
        QMessageBox::critical(this, "output port error", "same port for output and cleaner!");
        return;
    }

    // check input ports
    inputs.clear();
    for (int r = 0;r < model->rowCount();r++)
    {
        int x = model->item(r, 0)->text().toInt() - 1;
        int y = model->item(r, 1)->text().toInt() - 1;
        if(x==output_x && y==output_y)
        {
            QMessageBox::critical(this, "port confliction", "same port for input and output!");
            return;
        }
        if(CHECK_EDGE_FAIL(x, y))
        {
            QMessageBox::critical(this, "intput port error", "input port not on the edge!");
            return;
        }
        if(CHECK_OUT_OF_RANGE(x, y))
        {
            QMessageBox::critical(this, "intput port error", "input port invaild!");
            return;
        }
        int input = y * col + x;
        for(auto i: inputs)
        {
            if(i == input)
            {
                QMessageBox::critical(this, "intput port error", "some input port is repeated!");
                return;
            }
        }
        if(cleaner && ( input == 0 || input == (col * row - 1)))
        {
            QMessageBox::critical(this, "intput port error", "same port for input and cleaner!");
            return;
        }
        inputs.append(input);
    }

    QDialog::accept();
}

void Option::initializeTable()
{
    model->setHorizontalHeaderLabels(QString("x,y").split(","));
    int width = ui->tableView->width();
    ui->tableView->setColumnWidth(0, width/2-10);
    ui->tableView->setColumnWidth(1, width/2-10);

}

void Option::addInput()
{
    QList<QStandardItem*> list;
    list.append(new QStandardItem());
    list.append(new QStandardItem());
    model->appendRow(list);
    return;
}


void Option::removeInput()
{
    int row = ui->tableView->currentIndex().row();
    model->removeRow(row);
}
