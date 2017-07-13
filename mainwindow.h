#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QtXml>
#include <QFile>
#include <QFileSystemModel>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QVariant>
#include <QDebug>
#include "components/codeeditor.h"
#include "components/highlighter.h"
#include "dialogs/newproject.h"
#include "dialogs/newtabledialog.h"
#include "dialogs/preferencedialog.h"
#include "components/toolBox.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    PreferenceDialog *prefDialog;

    int addEditor(QString filePath = "", bool isNew = false);

    QStringList fileTypes;

signals:
    void fileUpdated(QString);

public slots:
    void newProjectCreated(QString projectName, QString projectPath);
    void fileUpdater(QString url);
    void handleAppOpenMessage(QString message);

private slots:
    void fileChanged(QString url);
    void showDirContextMenu(const QPoint &pos);
    void updateIncFilesView();
    void updateCurserPosition(const QString &pos);
    void newCompleter(const QString &completer);
    void switchTab();
    void saveTabOrder();
    void fileRenamed(QString path, QString oldName, QString newName);
    void on_tabWidget_tabCloseRequested(int index);
    bool on_action_Close_triggered();
    void on_action_New_triggered();
    void on_action_Open_triggered();
    void on_actionClose_triggered();
    void on_actionPaste_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionSave_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionSave_As_triggered();
    void on_action_Table_triggered();
    void on_actionCreateNewProject_triggered();
    void on_actionOpenExisingProject_triggered();
    void on_treeView_doubleClicked(const QModelIndex &index);
    void on_tabWidget_currentChanged(int index);
    void on_actionReload_triggered();
    void on_projectSelector_activated(int index);
    void on_fileListView_doubleClicked(const QModelIndex &index);
    void on_actionRenameCurrentProject_triggered();
    void on_actionRemoveCurrentProject_triggered();
    void on_actionPreferences_triggered();
    void on_actionShow_Toolbar_triggered(bool checked);
    void on_actionRestore_Tab_triggered();
    void on_actionGo2Line_triggered();
    void on_actionFind_triggered();
    void on_actionShowToolbox_triggered(bool checked);
    void pDel_triggered();
    void pCopy_triggered();
    void pCut_triggered();
    void pPast_triggered();
    void on_tabWidget_tabBarClicked(int index);

    void on_actionContext_help_triggered();

    void on_actionReplace_triggered();

private:
    Ui::MainWindow *ui;
    CodeEditor *editor;
    ToolBox *toolBox;
    Highlighter *highlighter;
    QWidget *newTab;
    QVBoxLayout *tabLayout;
    QFileSystemModel *dirModel;
    QStringList phpCompleterList;
    QStringList htmlCompleterList;
    QStringList cssCompleterList;
    QStringList jsCompleterList;
    QStringList *cssKeyWords;
    QStringList *jsKeyWords;
    QStringListModel *phpKeyWordModel;
    QStringListModel *phpFuncNameModel;
    QMap<QString, QString> projects;
    QLabel *curserPosition;
    QLabel *completerName;
    bool isCtrlPressed;
    QVector<int> tabOrder;
    CodeEditor *trashBin;
    int tabPressedCount;

    QAction *pOpen;
    QAction *pRename;
    QAction *pDel;
    QAction *pCopy;
    QAction *pCut;
    QAction *pPast;
    QAction *pAddFolder;
    QAction *pAddFile;
    QMenu *fileContextMenu;
    QString pCopyPath;
    bool isCutAction;

    bool closeTab(int index);
    int isFileOpen(QString filePath);
    void initKeyWords();
    void initDirContextMenu();
    void saveSettings();
    void loadSettings();
    CodeEditor* checkForEditor();
};

#endif // MAINWINDOW_H
