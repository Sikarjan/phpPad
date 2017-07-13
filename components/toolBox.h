#ifndef ToolBox_H
#define ToolBox_H

#include <QWidget>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
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
    void setFindFocus(QString keyWord);
    void setReplaceFocus(QString keyWord);
    void setHelpFocus(QString keyWord);

    CodeEditor *mEditor;

public slots:
    void replyFinished(QNetworkReply*pReply);

private slots:
    void on_closeButton_clicked();
    void on_findButton_clicked();
    void on_findAllButton_clicked();
    void on_hitList_doubleClicked(const QModelIndex &index);
    void on_helpFilter_returnPressed();
    void on_replaceNextButton_clicked();
    void on_replaceButton_clicked();
    void on_replaceAllButton_clicked();
    void on_replaceInput_editingFinished();

private:
    Ui::ToolBox *ui;
    QTextCursor oldCursor;
    QVector<int> hitPositions;
    QNetworkAccessManager* networkManager;

    bool findNext(QString keyWord);
};

#endif // ToolBox_H
