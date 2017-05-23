#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QDebug>

namespace Ui {
class PreferenceDialog;
}

class PreferenceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferenceDialog(QWidget *parent = 0);
    ~PreferenceDialog();

signals:
    void defaultFontChanged(QFont font);

private slots:
    void on_buttonBox_accepted();
    void on_fontComboBox_currentFontChanged(const QFont &f);
    void on_fontSizeBox_valueChanged(int arg1);

private:
    Ui::PreferenceDialog *ui;

    void fontChanged();
};

#endif // PREFERENCEDIALOG_H
