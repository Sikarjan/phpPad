#include "toolBox.h"
#include "ui_ToolBox.h"

ToolBox::ToolBox(QWidget *parent) : QWidget(parent), ui(new Ui::ToolBox){
    ui->setupUi(this);
}

ToolBox::~ToolBox()
{
    delete ui;
}

void ToolBox::on_closeButton_clicked()
{
    this->hide();
}

void ToolBox::on_findButton_clicked()
{
    mEditor->find(ui->searchInput->text());
}

void ToolBox::on_findAllButton_clicked()
{
    mEditor->find(ui->searchInput->text());
}
