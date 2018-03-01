#include "newproject.h"
#include "ui_newproject.h"

NewProject::NewProject(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewProject)
{
    ui->setupUi(this);

    QTranslator translator;
    translator.load("phpPad_"+QLocale::system().name().left(2) ,":/translations/translations");
    qApp->installTranslator(&translator);
    ui->retranslateUi(this);

    QSettings settings("InnoBiz", "phpPad");
    settings.beginGroup("newProject");
    mPath = settings.value("mainFolder", "").toString();
    settings.endGroup();

    ui->mainPath->setText(QDir::toNativeSeparators(mPath));
    ui->rootPath->setText(QDir::toNativeSeparators(mPath));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

NewProject::~NewProject()
{
    delete ui;
}

void NewProject::on_pushButton_clicked()
{
    QString path =  QFileDialog::getExistingDirectory(this, tr("Open Directory"), this->mPath,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(path.isEmpty())
        return;

    ui->mainPath->setText(QDir::toNativeSeparators(path));
    on_projectName_textChanged(ui->projectName->text());
}

void NewProject::on_projectName_textChanged(const QString &name)
{
    this->rPath = ui->mainPath->text()+"/"+name;
    ui->rootPath->setText(QDir::toNativeSeparators(rPath));

    if(QDir(rPath).exists()){
        ui->errorMessage->setText(tr("This folder already exists."));
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }else{
        ui->errorMessage->setText("");
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void NewProject::on_buttonBox_accepted(){
    this->mPath = ui->mainPath->text();
    QDir dir = QDir(mPath);
    if(dir.exists()){
        this->rPath = ui->rootPath->text();
        dir.mkpath(rPath);
        if(ui->checkBox->isChecked()){
            QSettings settings("InnoBiz", "phpPad");
            settings.beginGroup("newProject");
            settings.setValue("mainFolder", this->mPath);
            settings.endGroup();
        }
        QString projectName = ui->projectName->text();
        emit newProjectCreated(projectName, this->rPath);
    }
}
