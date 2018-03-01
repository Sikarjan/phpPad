#include "newtabledialog.h"
#include "ui_newtabledialog.h"

NewTableDialog::NewTableDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewTableDialog)
{
    ui->setupUi(this);

    QTranslator translator;
    translator.load("phpPad_"+QLocale::system().name().left(2) ,":/translations/translations");
    qApp->installTranslator(&translator);
    ui->retranslateUi(this);
}

NewTableDialog::~NewTableDialog()
{
    delete ui;
}

void NewTableDialog::on_buttonBox_accepted()
{
    emit insertNewTable(ui->columns->value(), ui->rows->value(), ui->headerGroup->checkedId(), ui->caption->text());
}
