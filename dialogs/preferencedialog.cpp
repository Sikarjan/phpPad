#include "preferencedialog.h"
#include "ui_preferencedialog.h"

PreferenceDialog::PreferenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferenceDialog)
{
    ui->setupUi(this);

    QCoreApplication::setOrganizationName("InnoBiz");
    QCoreApplication::setApplicationName("phpPad");

    QTranslator translator;

    lang = settings.value("window/uiLanguage", "").toString();
    if(!lang.isEmpty()){
        translator.load("phpPad_"+lang ,":/translations/translations");
        qApp->installTranslator(&translator);
        ui->retranslateUi(this);
    }

    settings.beginGroup("fonts");
        fSize = settings.value("standardFontSize", 0).toInt();
        cFont = QFont(settings.value("standardFont", "Lucida Console").toString());
        ui->fontComboBox->setCurrentFont(cFont);
    settings.endGroup();

    if(fSize == 0 && QSysInfo::productType() == "macos")
        fSize = 11;
    else if(fSize == 0)
        fSize = 10;

    ui->fontSizeBox->setValue(fSize);

    if(lang == "de"){
        ui->uiLangSelector->setCurrentIndex(1);
    }
}

PreferenceDialog::~PreferenceDialog()
{
    delete ui;
}

void PreferenceDialog::on_buttonBox_accepted()
{
    QFont newFont(ui->fontComboBox->currentFont());
    if(fSize != ui->fontSizeBox->value() || cFont != newFont){
        settings.beginGroup("fonts");
            settings.setValue("standardFont", ui->fontComboBox->currentText());
            settings.setValue("standardFontSize", ui->fontSizeBox->value());
        settings.endGroup();

        newFont.setPixelSize(ui->fontSizeBox->value());
        emit defaultFontChanged(newFont);
    }

    QString uiLang = "";
    switch (ui->uiLangSelector->currentIndex()) {
        case 1:
            uiLang = "de";
            break;
        default:
            break;
    }

    if(lang != uiLang){
        settings.setValue("window/uiLanguage", uiLang);
        emit uiLanguageChanged(uiLang);
    }
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
