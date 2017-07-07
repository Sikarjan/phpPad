#include "newtabledialog.h"
#include "ui_newtabledialog.h"

NewTableDialog::NewTableDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewTableDialog)
{
    ui->setupUi(this);
}

NewTableDialog::~NewTableDialog()
{
    delete ui;
}

void NewTableDialog::on_buttonBox_accepted()
{
    emit insertNewTable(ui->columns->value(), ui->rows->value(), ui->headerGroup->checkedId(), ui->caption->text());
}
