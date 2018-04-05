#ifndef INSERTIMAGEDIALOG_H
#define INSERTIMAGEDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QTranslator>
#include <QFileDialog>

namespace Ui {
class InsertImageDialog;
}

class InsertImageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InsertImageDialog(QWidget *parent = 0);
    ~InsertImageDialog();

    QString mPath;

signals:
    void insertNewImage(QString path, QString alt, QString caption, int width, int height);

private slots:
    void on_pushButton_clicked();
    void on_buttonBox_accepted();
    void on_imgWidth_valueChanged(int width);
    void on_imgHeight_valueChanged(int height);

private:
    Ui::InsertImageDialog *ui;
    double ratio;
    QImage img;
};

#endif // INSERTIMAGEDIALOG_H
