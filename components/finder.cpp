#include "finder.h"
#include "ui_finder.h"

Finder::Finder(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Finder)
{
    ui->setupUi(this);
}

Finder::~Finder()
{
    delete ui;
}

void Finder::on_closeButton_clicked()
{
    this->hide();
}

void Finder::on_findButton_clicked()
{
    emit findNext(ui->searchInput->text());
}
