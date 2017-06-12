#ifndef ToolBox_H
#define ToolBox_H

#include <QWidget>
#include "codeeditor.h"

namespace Ui {
class ToolBox;
}

class ToolBox : public QWidget
{
    Q_OBJECT

public:
    explicit ToolBox(QWidget *parent = 0);
    ~ToolBox();

    CodeEditor *mEditor;

private slots:
    void on_closeButton_clicked();
    void on_findButton_clicked();
    void on_findAllButton_clicked();

private:
    Ui::ToolBox *ui;
};

#endif // ToolBox_H
