#ifndef NEWTABLEDIALOG_H
#define NEWTABLEDIALOG_H

#include <QDialog>

namespace Ui {
class NewTableDialog;
}

class NewTableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewTableDialog(QWidget *parent = 0);
    ~NewTableDialog();

signals:
    void insertNewTable(int colums, int rows, int header, QString caption);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::NewTableDialog *ui;
};

#endif // NEWTABLEDIALOG_H
