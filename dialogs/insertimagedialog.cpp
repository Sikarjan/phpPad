#include "insertimagedialog.h"
#include "ui_insertimagedialog.h"

InsertImageDialog::InsertImageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InsertImageDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    QTranslator translator;
    translator.load("phpPad_"+QLocale::system().name().left(2) ,":/translations/translations");
    qApp->installTranslator(&translator);
    ui->retranslateUi(this);

    qDebug() << mPath;
}

InsertImageDialog::~InsertImageDialog()
{
    delete ui;
}

void InsertImageDialog::on_pushButton_clicked()
{
    QString path =  QFileDialog::getOpenFileName(this, tr("Select image"), mPath, tr("Image Files (*.png *.jpg *.gif)"));
    if(path.isEmpty()){
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->imgWidth->setEnabled(false);
        ui->imgHeight->setEnabled(false);
        return;
    }

    img = QImage(path);

    if(img.isNull()){
        // img could not be loaded
        return;
    }

    ui->imgPath->setText(QDir::toNativeSeparators(path));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    ui->imgWidth->setEnabled(true);
    ui->imgHeight->setEnabled(true);
    ui->imgWidth->setValue(img.width());
    ui->imgHeight->setValue(img.height());
    ui->imgRelation->setChecked(true);
    ratio = static_cast<double>(img.width())/img.height();

    QPixmap pixmap(path);
    int w = ui->canvas->width();
    int h = ui->canvas->height();

    if(img.width() > w || img.height() > h){
        pixmap = pixmap.scaled(w,h, Qt::KeepAspectRatio);
    }
    ui->canvas->setPixmap(pixmap);
}

void InsertImageDialog::on_buttonBox_accepted()
{
    emit insertNewImage(ui->imgPath->text(), ui->altText->text(), ui->captionText->text(), ui->imgWidth->value(), ui->imgHeight->value());
}

void InsertImageDialog::on_imgWidth_valueChanged(int width)
{
    if(!ui->imgRelation->isChecked())
        return;
    else
        ui->imgRelation->setChecked(false);

    ui->imgHeight->setValue(qRound(width/ratio));
    ui->imgRelation->setChecked(true);
}

void InsertImageDialog::on_imgHeight_valueChanged(int height)
{
    if(!ui->imgRelation->isChecked())
        return;
    else
        ui->imgRelation->setChecked(false);

    ui->imgWidth->setValue(qRound(height*ratio));
    ui->imgRelation->setChecked(true);
}
