#include "preferencedialog.h"
#include "ui_preferencedialog.h"

PreferenceDialog::PreferenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferenceDialog)
{
    ui->setupUi(this);

    QSettings settings("InnoBiz", "phpPad");

    settings.beginGroup("fonts");
        int fontSize = settings.value("standardFontSize", 0).toInt();
        ui->fontComboBox->setCurrentFont(QFont(settings.value("standardFont", "Lucida Console").toString()));
    settings.endGroup();

    if(fontSize != 0)
        ui->fontSizeBox->setValue(fontSize);
    else if(QSysInfo::productType() == "macos")
        ui->fontSizeBox->setValue(11);
    else
        ui->fontSizeBox->setValue(10);
}

PreferenceDialog::~PreferenceDialog()
{
    delete ui;
}

void PreferenceDialog::on_buttonBox_accepted()
{
    QSettings settings("InnoBiz", "phpPad");

    settings.beginGroup("fonts");
        settings.setValue("standardFont", ui->fontComboBox->currentText());
        settings.setValue("standardFontSize", ui->fontSizeBox->value());
    settings.endGroup();

    QFont newFont(ui->fontComboBox->currentFont());
    newFont.setPixelSize(ui->fontSizeBox->value());
    emit defaultFontChanged(newFont);
}

void PreferenceDialog::on_fontComboBox_currentFontChanged(const QFont &f)
{
    Q_UNUSED(f);
    fontChanged();
}

void PreferenceDialog::on_fontSizeBox_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    fontChanged();
}

void PreferenceDialog::fontChanged()
{
    QFont newFont(ui->fontComboBox->currentFont());
    newFont.setPixelSize(ui->fontSizeBox->value());

    ui->testLabel->setFont(newFont);
}
