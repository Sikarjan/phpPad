#ifndef NEWPROJECT_H
#define NEWPROJECT_H

#include <QDialog>
#include <QFileDialog>
#include <QSettings>
#include <QDir>
#include <QTranslator>

namespace Ui {
class NewProject;
}

class NewProject : public QDialog
{
    Q_OBJECT

public:
    explicit NewProject(QWidget *parent = 0);
    ~NewProject();

signals:
    void newProjectCreated(QString name, QString path);

private slots:
    void on_pushButton_clicked();
    void on_projectName_textChanged(const QString &name);
    void on_buttonBox_accepted();

private:
    Ui::NewProject *ui;

    QString mPath;
    QString rPath;
};

#endif // NEWPROJECT_H
