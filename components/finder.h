#ifndef FINDER_H
#define FINDER_H

#include <QWidget>

namespace Ui {
class Finder;
}

class Finder : public QWidget
{
    Q_OBJECT

public:
    explicit Finder(QWidget *parent = 0);
    ~Finder();

signals:
    findNext(QString text);

private slots:
    void on_closeButton_clicked();

    void on_findButton_clicked();

private:
    Ui::Finder *ui;
};

#endif // FINDER_H
