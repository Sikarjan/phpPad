#ifndef ToolBox_H
#define ToolBox_H

#include <QWidget>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCompleter>
#include <QStringListModel>
#include <QTranslator>
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
    void setPhpCompleter(QStandardItemModel *keyWords);

    CodeEditor *mEditor;

signals:
    void newHistoryIndex();

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
    void on_caseCheckBox_clicked();
    void on_reverseCheckBox_clicked();
    void on_wholeWordCheckBox_clicked();
    void on_toolBox_currentChanged(int index);
    void on_searchInput_cursorPositionChanged(int arg1, int arg2);
    void on_helpDisplay_anchorClicked(const QUrl &arg1);
    void historyIndeyChanged();
    void on_navigateBack_clicked();
    void on_navigateForward_clicked();

private:
    Ui::ToolBox *ui;
    QTextCursor oldCursor;
    QVector<int> hitPositions;
    QNetworkAccessManager* networkManager;
    QCompleter *phpCompleter;
    QTextDocument::FindFlags flag;
    QUrl extUrl;
    QList<QUrl> *history;
    int historyIndex;

    bool findNext(QString keyWord);
    void updateFindFlags();
};

#endif // ToolBox_H
