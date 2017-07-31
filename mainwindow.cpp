#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    isCtrlPressed = false;
    tabPressedCount = 0;
    fileTypes << ".php" << ".html" << ".css" << ".js" << ".txt" << ".*";

    initDirContextMenu();
    initKeyWords();

    ui->tabWidget->removeTab(1);

    curserPosition = new QLabel;
    ui->statusBar->addPermanentWidget(curserPosition);
    completerName = new QLabel;
    ui->statusBar->addPermanentWidget(completerName);

    dirModel = new QFileSystemModel(this);
    dirModel->setReadOnly(false);
    connect(dirModel, SIGNAL(fileRenamed(QString,QString,QString)), this, SLOT(fileRenamed(QString,QString,QString)));
    ui->treeView->setModel(dirModel);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showDirContextMenu(QPoint)));

    prefDialog = new PreferenceDialog;
    prefDialog->setModal(true);

    loadSettings();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(on_action_Close_triggered()){
        saveSettings();
    } else {
        event->ignore();
    }
}

MainWindow::~MainWindow(){
    delete ui;
}

// Support functions
bool MainWindow::closeTab(int index){
    QWidget* widget = ui->tabWidget->widget(index);
    if(widget->objectName() != "startTab"){
        CodeEditor *tabEditor = NULL;
        tabEditor = widget->findChild<CodeEditor *>();
        if(tabEditor->isFileChanged){
            QMessageBox msgBox;
            msgBox.setText("The document "+ ui->tabWidget->tabText(index) +" has been modified.");
            msgBox.setInformativeText("Do you want to save your changes?");
            msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Save);
            int ret = msgBox.exec();
            switch (ret) {
                case QMessageBox::Save:
                    ui->tabWidget->setCurrentIndex(index);
                    on_actionSave_triggered();
                    break;
                case QMessageBox::Discard:
                    tabEditor->isFileChanged = false;
                    break;
                case QMessageBox::Cancel:
                    return false;
                    break;
                default:
                    // should never be reached
                    break;
              }
        }

// The trash bin needs to be reworked entirely
//        if(trashBin->count() == 10)
//            trashBin->remove(0);
        trashBin = tabEditor;
        ui->actionRestore_Tab->setEnabled(true);
        ui->tabWidget->removeTab(index);
        ui->actionUndo->setEnabled(false);
        ui->actionRedo->setEnabled(false);
        ui->actionReload->setEnabled(false);
        if(index>0)
            ui->tabWidget->setCurrentIndex(index-1);

        return true;
    }else{
        addEditor();
        return true;
    }
    return false;
}

int MainWindow::addEditor(QString filePath, bool isNew){
    editor = new CodeEditor;
    editor->url = filePath;
    editor->lockBlockState = true;

    toolBox = new ToolBox;
    toolBox->mEditor = editor;
    toolBox->setPhpCompleter(phpCompleterModel);

    QString fileName;
    QString fileType;
    QString text = "";

    highlighter = new Highlighter(editor->document(), phpKeyWordModel, phpFuncNameModel, cssKeyWords, jsKeyWords);

    connect(editor, SIGNAL(undoAvailable(bool)), ui->actionUndo, SLOT(setEnabled(bool)));
    connect(editor, SIGNAL(redoAvailable(bool)), ui->actionRedo, SLOT(setEnabled(bool)));
    connect(highlighter, SIGNAL(currentBlockStateChanged(int)), editor, SLOT(textBlockStateChanged(int)));
    connect(editor, SIGNAL(updateIncludedFiles()), this, SLOT(updateIncFilesView()));
    connect(editor, SIGNAL(newCurserPosition(QString)), this, SLOT(updateCurserPosition(QString)));
    connect(editor, SIGNAL(newCompleter(QString)), this, SLOT(newCompleter(QString)));
    connect(prefDialog, SIGNAL(defaultFontChanged(QFont)), editor, SLOT(defaultFontChanged(QFont)));
    connect(editor, SIGNAL(switchTab()), this, SLOT(switchTab()));
    connect(editor, SIGNAL(ctrlReleased()), this, SLOT(saveTabOrder()));
    connect(editor, SIGNAL(closeEditor()), this, SLOT(on_actionClose_triggered()));
    connect(editor, SIGNAL(fileChanged(QString)), this, SLOT(fileUpdater(QString)));
    connect(this, SIGNAL(fileUpdated(QString)), editor, SLOT(fileChangeListener(QString)));

    QSplitter * editorSplitter = new QSplitter();
    editorSplitter->setHandleWidth(1);
    editorSplitter->setChildrenCollapsible(false);
    editorSplitter->addWidget(editor);
    editorSplitter->addWidget(toolBox);
    QList<int> editorSplitterWidth;
    editorSplitterWidth << 50000 << 10;
    editorSplitter->setSizes(editorSplitterWidth);
    toolBox->hide();

    int index = ui->tabWidget->insertTab(ui->tabWidget->count()-1, editorSplitter, "opening...");
    ui->tabWidget->setCurrentIndex(index);

    if(filePath == "" || isNew){
        if(isNew){
            fileName = filePath.section("/",-1,-1);
            fileType = "."+fileName.section(".",-1);
        }else{
            // Show dialog to select type of the new file
            fileType = QInputDialog::getItem(this, tr("Create new file"), tr("File type"),fileTypes,0,false);
            fileName = "new" + fileType;
        }

        // Get basic file according to selection
        QString defaultPath = "";
        if(fileType == ".php" || fileType == ".html") {
            defaultPath = ":/xml/xml/defaultHtml.txt";
        }

        QFile file(defaultPath);
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream in(&file);
            text = in.readAll();
            file.close();
        }
    }else{
        QFile *file = new QFile(filePath);
        if(!file->open(QFile::ReadOnly | QFile::Text)){
            return -1;
        }
        fileName = filePath.section("/",-1,-1);
        fileType = "."+fileName.section(".",-1);

        QTextStream in(file);
        in.setCodec("UTF-8");
        in.setAutoDetectUnicode(true);
        text = in.readAll();
        file->close();

        editor->textFile = file;
        ui->statusBar->showMessage(tr("File loaded"), 2000);
    }

    int blockState = fileTypes.indexOf(fileType)*10;
    highlighter->setDocType(blockState == 0 ? 10:blockState);

    if(fileType == ".php")
        editor->setPhpCompleterList(phpCompleterModel);
    if(fileType == ".html" || fileType == ".php")
        editor->setHtmlCompleterList(htmlCompleterList);
    if(fileType == ".css" || fileType == ".html" || fileType == ".php")
        editor->setCssCompleterList(cssCompleterList);
    if(fileType == ".js" || fileType == ".html" || fileType == ".php")
        editor->setJsCompleterList(jsCompleterList);

    editor->insertPlainText(text);
    connect(editor, SIGNAL(fileChanged(QString)), this, SLOT(fileChanged(QString)));
    editor->isFileChanged = false;
    editor->docType = fileTypes.indexOf(fileType);

    ui->tabWidget->setTabText(index, fileName);
    editor->setFocus();
    QTextCursor curs = editor->textCursor();
    curs.setPosition(0);
    editor->setTextCursor(curs);
    editor->scanDocument();
    editor->textBlockStateChanged(blockState == 0 ? 10:blockState);

    ui->menu_Insert->setEnabled(true);
    ui->actionFind->setEnabled(true);
    ui->actionReplace->setEnabled(true);
    ui->actionShowToolbox->setEnabled(true);
    ui->actionShowToolbox->setChecked(false);
    ui->actionContext_help->setEnabled(true);
    tabOrder.insert(0, index);
    return index;
}

void MainWindow::fileChanged(QString url){
    Q_UNUSED(url);
    int index = ui->tabWidget->currentIndex();

    ui->tabWidget->setTabText(index, ui->tabWidget->tabText(index)+"*");
    ui->actionSave->setEnabled(true);
    ui->actionUndo->setEnabled(true);
    ui->actionReload->setEnabled(true);
}

// Handling Actions
void MainWindow::on_actionSave_triggered(){
    CodeEditor *mEditor = checkForEditor();
    if(mEditor == NULL)
        return;

    if(mEditor->url == ""){
        on_actionSave_As_triggered();
        return;
    }

    QFile sFile(mEditor->url);
    if(!sFile.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "we have an error =( " << mEditor->url << " " << sFile.errorString();
    }else{
        #ifndef QT_NO_CURSOR
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        #endif
        QTextStream out(&sFile);
        out.setCodec("UTF-8");
        out << mEditor->toPlainText();

        sFile.flush();
        sFile.close();

        mEditor->isFileChanged = false;
        mEditor->includedFilesList.clear();
        mEditor->scanDocument();

        #ifndef QT_NO_CURSOR
            QApplication::restoreOverrideCursor();
        #endif

        ui->actionSave->setEnabled(false);
        ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), mEditor->url.section("/",-1,-1));
        ui->statusBar->showMessage(tr("File saved"), 2000);
    }
}

void MainWindow::on_actionSave_As_triggered(){
    CodeEditor *mEditor = checkForEditor();
    if(mEditor == NULL){
        return;
    }
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), projects[ui->projectSelector->currentText()], tr("PHP file (*.php *.php3);;CSS file (*.css);; JavaScript file (*.js);;HTML file (*.html *.htm)"));

    if(!fileName.isEmpty()){
        mEditor->url = fileName;
        ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), fileName.section("/",-1,-1));
        on_actionSave_triggered();
    }
}

void MainWindow::on_tabWidget_tabCloseRequested(int index){
    closeTab(index);
}

void MainWindow::on_action_Open_triggered(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), projects[ui->projectSelector->currentText()], "PHP (*.php);; CSS (*.css);; HTML (*.html *.htm)All files (.*)");

    if(!fileName.isEmpty()){
        int index = addEditor(fileName);
        if(index == -1){
            QMessageBox::critical(this, tr("File Error"), tr("phpPad was not able to open the file."), QMessageBox::Ok);
        }
    }
}

void MainWindow::on_action_New_triggered(){
    addEditor();
}

void MainWindow::on_actionClose_triggered(){
    int index = ui->tabWidget->currentIndex();
    closeTab(index);
}

bool MainWindow::on_action_Close_triggered(){ // Closing the program
    // Check if there are any unsaved changes
    int tabs = ui->tabWidget->count();
    int tabToDelete = 0;
    for(int i = 0; i<tabs; i++){
        QWidget* widget = ui->tabWidget->widget(tabToDelete);
        if(widget->objectName() != "startTab"){
            if(!closeTab(tabToDelete)){
            return false;
            }
        }else{
            tabToDelete = 1;
        }
    }
    this->close();

    if(ui->tabWidget->count() == 1)
        ui->menu_Insert->setEnabled(false);

    return true;
}

// Editing Actions
void MainWindow::on_actionPaste_triggered(){
    CodeEditor *mEditor = checkForEditor();
    if(mEditor == NULL){
        return;
    }
    mEditor->paste();
}

void MainWindow::on_actionCut_triggered(){
    CodeEditor *mEditor = checkForEditor();
    if(mEditor == NULL){
        return;
    }
    mEditor->cut();
}

void MainWindow::on_actionCopy_triggered(){
    CodeEditor *mEditor = checkForEditor();
    if(mEditor == NULL){
        return;
    }
    mEditor->copy();
}

void MainWindow::on_actionUndo_triggered(){
    CodeEditor *mEditor = checkForEditor();
    if(mEditor == NULL){
        return;
    }
    mEditor->undo();
}

void MainWindow::on_actionRedo_triggered(){
    CodeEditor *mEditor = checkForEditor();
    if(mEditor == NULL){
        return;
    }
    mEditor->redo();
}

void MainWindow::on_actionReload_triggered(){
    CodeEditor *mEditor = checkForEditor();
    if(mEditor == NULL){
        return;
    }

    // Ask user if he really wants to loose all changes
    QMessageBox msgBox;
    msgBox.setText("The document "+ ui->tabWidget->tabText(ui->tabWidget->currentIndex()) +" has been modified.");
    msgBox.setInformativeText(tr("Do you really want to reset the file to the first opened state? This action cannot be undone. If you want to keep your changes save the document first."));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Cancel){
        return;
    }

    QFile *file = mEditor->textFile;
    file->open(QFile::ReadOnly | QFile::Text);
    QTextStream in(file);
    mEditor->setPlainText(in.readAll());

    ui->actionReload->setEnabled(false);
    mEditor->isFileChanged = false;
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), mEditor->url.section("/",-1,-1));
}

void MainWindow::saveSettings(){
    QSettings settings("InnoBiz", "phpPad");
    settings.beginGroup("window");
        settings.setValue("position", this->geometry());
        settings.setValue("fullScreen", this->isFullScreen());
        if(ui->projectSelector->count() > 1)
            settings.setValue("lastProject", ui->projectSelector->currentText());
        settings.setValue("mainSplitter", ui->mainSplitter->saveState());
        settings.setValue("toolSplitter", ui->toolSplitter->saveState());
        settings.setValue("treeViewColumn0Width",ui->treeView->columnWidth(0));
    settings.endGroup();
    settings.remove("projects");
    settings.beginGroup("projects");
        foreach (const QString &key, projects.keys()) {
            settings.setValue(key, projects[key]);
        }
    settings.endGroup();
}

void MainWindow::loadSettings(){
    QSettings settings("InnoBiz", "phpPad");

    settings.beginGroup("projects");
        QStringList keys = settings.allKeys();
        foreach (QString key, keys) {
             projects[key] = settings.value(key).toString();
             ui->projectSelector->insertItem(0, key);
        }
    settings.endGroup();

    settings.beginGroup("window");
        bool fScreen = settings.value("fullScreen", true).toBool();
        if(fScreen){
            this->setWindowState(Qt::WindowFullScreen);
        }else{
            QRect window = settings.value("position").toRect();
            this->setGeometry(window);
        }
        QString lastProject = settings.value("lastProject", "").toString();
        if(!projects.value(lastProject).isEmpty()){
            ui->treeView->setRootIndex(dirModel->setRootPath(projects.value(lastProject)));
            ui->treeView->sortByColumn(0, Qt::AscendingOrder);
            ui->projectSelector->setCurrentIndex(ui->projectSelector->findText(lastProject));
            ui->actionRenameCurrentProject->setEnabled(true);
            ui->actionRemoveCurrentProject->setEnabled(true);
        }
        ui->mainSplitter->restoreState(settings.value("mainSplitter").toByteArray());
        ui->toolSplitter->restoreState(settings.value("toolSplitter").toByteArray());
        ui->treeView->setColumnWidth(0, settings.value("treeViewColumn0Width",200).toInt());
    settings.endGroup();
}

CodeEditor *MainWindow::checkForEditor(){
    CodeEditor *tabEditor = NULL;
    QWidget *widget = ui->tabWidget->currentWidget();
    if(widget->objectName() != "startTab"){
        tabEditor = widget->findChild<CodeEditor *>();
    }
    return tabEditor;
}

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    QString path = dirModel->fileInfo(index).absoluteFilePath();
    int i = isFileOpen(path);

    if(QFileInfo(path).isFile() && i < 0){
        addEditor(path);
    }else if(i >= 0){
        ui->tabWidget->setCurrentIndex(i);
    }
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    Q_UNUSED(index);
    CodeEditor *mEditor = checkForEditor();

    if(mEditor == NULL){
        ui->actionSave_As->setEnabled(false);
        ui->actionGo2Line->setEnabled(false);
        ui->actionFind->setEnabled(false);
        ui->actionReplace->setEnabled(false);
        ui->actionShowToolbox->setEnabled(false);
        ui->actionContext_help->setEnabled(false);
        return;
    }

    ui->fileListView->setModel(mEditor->includedFilesModel);

    // Set action according to state of the document in the tab
    ui->actionSave->setEnabled(mEditor->isFileChanged);
    ui->actionSave_As->setEnabled(true);
    ui->actionUndo->setEnabled(mEditor->isFileChanged);
    ui->actionRedo->setEnabled(mEditor->isRedoAvailable);
    ui->actionReload->setEnabled(mEditor->isFileChanged);
    ui->actionGo2Line->setEnabled(true);
    ui->actionFind->setEnabled(true);
    ui->actionReplace->setEnabled(true);
    ui->actionContext_help->setEnabled(true);

    QWidget *widget = ui->tabWidget->currentWidget();
    ToolBox *toolBox = widget->findChild<ToolBox *>();
    ui->actionShowToolbox->setChecked(toolBox->isVisible());

    mEditor->updateIncFiles();
}

void MainWindow::on_projectSelector_activated(int index){
    if(index == ui->projectSelector->count()-1){ // create new project
        on_actionCreateNewProject_triggered();
        return;
    }

    ui->treeView->setRootIndex(dirModel->index(projects.value(ui->projectSelector->currentText())));
}

void MainWindow::on_actionCreateNewProject_triggered(){
    NewProject newProjectDialog;
    newProjectDialog.setModal(true);
    connect(&newProjectDialog, SIGNAL(newProjectCreated(QString,QString)), this, SLOT(newProjectCreated(QString, QString)));
    newProjectDialog.exec();
}
void MainWindow::on_actionOpenExisingProject_triggered(){
    QSettings settings("InnoBiz", "phpPad");
    settings.beginGroup("newProject");
        QString mPath = settings.value("mainFolder", "").toString();
    settings.endGroup();

    QString path =  QFileDialog::getExistingDirectory(this, tr("Open Directory"), mPath,QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(path.isEmpty())
        return;

    QString projectName = QInputDialog::getText(this, tr("Set Project Name"), tr("Project Name"), QLineEdit::Normal, path.section("/",-1));
    if(projectName.isEmpty())
        return;

    newProjectCreated(projectName, path);
}

void MainWindow::newProjectCreated(QString projectName, QString projectPath){
    projects[projectName] = projectPath;
    ui->projectSelector->insertItem(0, projectName);
    ui->projectSelector->setCurrentIndex(0);
    ui->treeView->setRootIndex(dirModel->setRootPath(projectPath));
}

void MainWindow::fileUpdater(QString url)
{
    qDebug() << "emitting: "+url;
    emit fileUpdated(url);
}

void MainWindow::handleAppOpenMessage(QString message)
{
    message.replace(QString("\\"), QString("/"));
    addEditor(message);
}

void MainWindow::showDirContextMenu(const QPoint &pos){
    QModelIndex index=ui->treeView->indexAt(pos);
    bool isRoot = false;

    if(!index.isValid()){
        index = dirModel->index(projects[ui->projectSelector->currentText()]);
        isRoot = true;
    }

    bool isDir = dirModel->isDir(index);

    if(isRoot){
        pRename->setEnabled(false);
        pDel->setEnabled(false);
        pCopy->setEnabled(false);
        pCut->setEnabled(false);
    }else{
        pRename->setEnabled(true);
        pDel->setEnabled(true);
        pCopy->setEnabled(true);
        pCut->setEnabled(true);
    }

    if(pCopyPath != "" && isDir){
        pPast->setEnabled(true);
        pPast->setStatusTip(tr("Insert")+ ": " + pCopyPath.section("/",-1,-1));
    }else{
        pPast->setEnabled(false);
    }

    QAction* selectedItem = fileContextMenu->exec(ui->treeView->viewport()->mapToGlobal(pos));

    if(selectedItem == pOpen){
        on_treeView_doubleClicked(index);
    }else if(selectedItem == pRename){
        ui->treeView->edit(index);
    }else if(selectedItem == pDel){

    }else if(selectedItem == pAddFolder){
        QString mName = QInputDialog::getText(this, tr("Create Folder"), tr("Folder Name"));
        if(!mName.isEmpty()){
            dirModel->mkdir(index, mName);
        }
    }else if(selectedItem == pAddFile){
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), dirModel->filePath(index), tr("PHP file (*.php, *php3)")); // ,QDir::homePath(),tr("PHP file (*.php, *php3)"),tr("PHP files (*.php)"), QFileDialog::HideNameFilterDetails

        if(!fileName.isEmpty()){
            QFile sFile(fileName);
            if(!sFile.open(QFile::WriteOnly | QFile::Text)){
                qDebug() << "we have an error =( " << " " << sFile.errorString();
            }else{
                QTextStream out(&sFile);
                out << "";

                sFile.flush();
                sFile.close();

                ui->treeView->expand(index);
                addEditor(fileName, true);
            }
        }
    }
}

void MainWindow::on_action_Table_triggered()
{
    CodeEditor *mEditor = checkForEditor();
    NewTableDialog newTableDialog;
    newTableDialog.setModal(true);
    connect(&newTableDialog,SIGNAL(insertNewTable(int,int,int,QString)), mEditor, SLOT(insertTable(int,int,int,QString)));
    newTableDialog.exec();
}

void MainWindow::updateIncFilesView(){
    CodeEditor *mEditor = checkForEditor();

    ui->fileListView->setModel(mEditor->includedFilesModel);
}

void MainWindow::on_fileListView_doubleClicked(const QModelIndex &index)
{
    QString path = index.data(Qt::StatusTipRole).toString();
    int i = isFileOpen(path);

    if(QFileInfo(path).isFile() && i < 0){
        addEditor(path);
    }else if(i >= 0){
        ui->tabWidget->setCurrentIndex(i);
    }
}

int MainWindow::isFileOpen(QString filePath){
    for(int i=0; i<ui->tabWidget->count();i++){
        QWidget *widget = ui->tabWidget->widget(i);

        if(widget->objectName() != "startTab"){
            CodeEditor *mEditor = NULL;
            mEditor = widget->findChild<CodeEditor *>();
            if(mEditor->url == filePath){
                return i;
            }
        }
    }
    return -1;
}

void MainWindow::initKeyWords()
{
    phpCompleterModel = new QStandardItemModel(this);
    // Load XML file for code highlighting and completion
    QFile file(":/xml/xml/php.xml");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "Failed to open file";
    }else{
        QXmlStreamReader reader(file.readAll());
        file.close();
        QStringList phpFunctionNames;
        QStringList phpKeyWords;

        while (!reader.atEnd()) {
            reader.readNext();
            if(reader.isStartElement() && reader.name() == QLatin1String("KeyWord")){
                QList<QStandardItem *> tmpList;
                tmpList << new QStandardItem(reader.attributes().value("name").toString());
                if(reader.attributes().value("func").toString() == "key"){
                    phpKeyWords << reader.attributes().value("name").toString();
                    tmpList << new QStandardItem(tr("key"));
                }else if(reader.attributes().value("func").toString() == "function"){
                    QString functionName = reader.attributes().value("name").toString();
                    phpFunctionNames << functionName;
                    reader.readNextStartElement();
                    QString overload = reader.attributes().value("retVal").toString() + " <b>"+functionName+"</b>(";
                    reader.readNextStartElement();
                    while (reader.name() == QLatin1String("Param")) {
                        QString param = reader.attributes().value("name").toString();
                        overload += param == "" ? "":reader.attributes().value("name").toString()+", ";;
                        reader.readNextStartElement();
                    }

                    overload.replace(overload.length()-2,1,")");
                    overload.replace(" ", "&nbsp;");
                    tmpList << new QStandardItem(tr("global"));
                    tmpList << new QStandardItem(overload);;
                }else{
                    tmpList << new QStandardItem(tr("global"));
                }

                phpCompleterModel->appendRow(tmpList);
            }
        }
        if (reader.hasError()) {
            qDebug() << "issues reading file " << reader.errorString();
        }

        phpKeyWordModel = new QStringListModel(this);
        phpKeyWordModel->setStringList(phpKeyWords);
        phpFuncNameModel = new QStringListModel(this);
        phpFuncNameModel->setStringList(phpFunctionNames);
    }

    QFile htmlFile(":/xml/xml/html.xml");
    if(!htmlFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "Failed to open file";
    }else{
        QXmlStreamReader reader(htmlFile.readAll());
        htmlFile.close();

        QString tag;
        while (!reader.atEnd()) {
            reader.readNext();
            if(reader.isStartElement() && reader.name() == QLatin1String("KeyWord")){
                tag = reader.attributes().value("name").toString();
                if(tag.length() > 2){
                    htmlCompleterList << tag;
                }
            }
        }
        if (reader.hasError()) {
            qDebug() << "issues reading file " << reader.errorString();
        }
    }

    QFile cssFile(":/xml/xml/css.xml");
    if(!cssFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "Failed to open file";
    }else{
        QXmlStreamReader reader(cssFile.readAll());
        cssFile.close();

        QString tag;
        cssKeyWords = new QStringList;

        while (!reader.atEnd()) {
            reader.readNext();
            if(reader.isStartElement() && reader.name() == QLatin1String("KeyWord")){
                tag = reader.attributes().value("name").toString();
                cssKeyWords->append(tag);
                if(tag.length() > 2){
                    cssCompleterList << tag;
                }
            }
        }
        if (reader.hasError()) {
            qDebug() << "issues reading file " << reader.errorString();
        }
    }

    QFile jsFile(":/xml/xml/javascript.xml");
    if(!jsFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "Failed to open file";
    }else{
        QXmlStreamReader reader(jsFile.readAll());
        cssFile.close();

        QString tag;
        jsKeyWords = new QStringList;

        while (!reader.atEnd()) {
            reader.readNext();
            if(reader.isStartElement() && reader.name() == QLatin1String("KeyWord")){
                tag = reader.attributes().value("name").toString();
                jsKeyWords->append(tag);
                if(tag.length() > 2){
                    jsCompleterList << tag;
                }
            }
        }
        if (reader.hasError()) {
            qDebug() << "issues reading file " << reader.errorString();
        }
    }
}

void MainWindow::initDirContextMenu()
{
    pCopyPath = "";
    isCutAction = false;
    fileContextMenu = new QMenu(this);

    // Define actions for explorer
    pOpen = new QAction(QIcon(":/icon/icons/openEnable.png"),tr("open"), this);
    pRename = new QAction(tr("rename"), this);
    pRename->setShortcut(QKeySequence(Qt::Key_F2));
    pDel = new QAction(tr("delete"), this);
    pDel->setShortcut(QKeySequence(Qt::Key_Delete));
    connect(pDel, SIGNAL(triggered()), this, SLOT(pDel_triggered()));
    pCopy =new QAction(QIcon(":/icon/icons/copyEnable.png"),tr("&copy"), this);
    pCopy->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_C));
    connect(pCopy, SIGNAL(triggered()), this, SLOT(pCopy_triggered()));
    pCut = new QAction(QIcon(":/icon/icons/cutEnable.png"), tr("cut"), this);
    pCut->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_X));
    connect(pCut, SIGNAL(triggered()), this, SLOT(pCut_triggered()));
    pPast = new QAction(QIcon(":/icon/icons/pastEnable.png"), tr("past"), this);
    pPast->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_V));
    connect(pPast, SIGNAL(triggered()), this, SLOT(pPast_triggered()));
    pAddFolder = new QAction(tr("Create Folder"), this);
    pAddFile = new QAction(QIcon(":/icon/icons/newEnable.png"),tr("Add new file"), this);

    fileContextMenu->addAction(pRename);
    fileContextMenu->addSeparator();
    fileContextMenu->addAction(pCopy);
    fileContextMenu->addAction(pCut);
    fileContextMenu->addAction(pPast);
    fileContextMenu->addSeparator();
    fileContextMenu->addAction(pAddFile);
    fileContextMenu->addAction(pAddFolder);
    fileContextMenu->addSeparator();
    fileContextMenu->addAction(pDel);

    ui->treeView->addAction(pCopy);
    ui->treeView->addAction(pCut);
    ui->treeView->addAction(pPast);
    ui->treeView->addAction(pDel);
}

void MainWindow::on_actionRenameCurrentProject_triggered()
{
    QString newName = QInputDialog::getText(this, tr("Rename project"), tr("New project name:"), QLineEdit::Normal, ui->projectSelector->currentText());

    if(newName.isEmpty())
        return;

    projects.insert(newName,projects[ui->projectSelector->currentText()]);
    projects.remove(ui->projectSelector->currentText());
    ui->projectSelector->removeItem(ui->projectSelector->currentIndex());
    ui->projectSelector->insertItem(0, newName);
    ui->projectSelector->setCurrentText(newName);
}

void MainWindow::on_actionRemoveCurrentProject_triggered()
{
    int ret = QMessageBox::question(this, tr("Remove Project"), tr("Do you want to remove the active project ")+ ui->projectSelector->currentText() +tr(" from your list of projects? No file will be deleted from your disk."), QMessageBox::Ok, QMessageBox::Cancel);

    if(ret == QMessageBox::Cancel)
        return;

    projects.remove(ui->projectSelector->currentText());
    ui->projectSelector->removeItem(ui->projectSelector->currentIndex());

    ui->projectSelector->setCurrentIndex(0);
    ui->treeView->setRootIndex(dirModel->index(projects.value(ui->projectSelector->currentText())));

    if(ui->projectSelector->count() == 1){
        ui->actionRemoveCurrentProject->setEnabled(false);
        ui->actionRenameCurrentProject->setEnabled(false);
    }
}

void MainWindow::updateCurserPosition(const QString &pos){
    curserPosition->setText(pos);
}

void MainWindow::newCompleter(const QString &completer)
{
    completerName->setText(tr("Completing: ")+completer);
}

void MainWindow::switchTab()
{
    if(isCtrlPressed){
        tabPressedCount++;
        if(tabPressedCount == tabOrder.count())
            tabPressedCount = 0;
    }else{
        if(tabOrder.count() == 1)
            return;

        isCtrlPressed = true;
        tabPressedCount = 1;
    }
    ui->tabWidget->setCurrentIndex(tabOrder[tabPressedCount]);
}

void MainWindow::saveTabOrder()
{
    if(!isCtrlPressed)
        return;

    isCtrlPressed = false;
    int index = ui->tabWidget->currentIndex();
    int pos = tabOrder.indexOf(index);
    tabOrder.move(pos,0);
}

void MainWindow::fileRenamed(QString path, QString oldName, QString newName)
{
    int i = isFileOpen(path+"/"+oldName);
    if(i >= 0){
        QWidget *widget = ui->tabWidget->widget(i);
        CodeEditor *tabEditor = (CodeEditor*)widget;
        tabEditor->url = path+"/"+newName;
        ui->tabWidget->setTabText(i, newName);
    }
}

void MainWindow::pDel_triggered()
{
    if(!ui->treeView->hasFocus())
        return;

    QString path = dirModel->filePath(ui->treeView->currentIndex());

    if(path == "")
        return;

    int ret = QMessageBox::question(this, tr("Delete file?"), tr("Are you sure you want to delete this file? This cannot be undone."), QMessageBox::Cancel | QMessageBox::Ok);
    if(ret == QMessageBox::Ok){
        bool deleted;
        QModelIndex index = ui->treeView->currentIndex();

        // files are not moved to trash! This feature is not available in Qt but the editor could be put in a trash bin while the program runs (see delte tab function)
        if(dirModel->isDir(index))
            deleted = dirModel->rmdir(index);
        else
            deleted = dirModel->remove(index);

        if(deleted){
            // check if the file was opend
            int i = isFileOpen(path);
            if(i >= 0){
                ret = QMessageBox::question(this, tr("Close editor?"), tr("The file is still opened in an editor. Do you want to keep the file in the editor?"), QMessageBox::No | QMessageBox::Yes);
                if(ret == QMessageBox::No){
                    ui->tabWidget->tabCloseRequested(i);
                }else{
                    QWidget *widget = ui->tabWidget->widget(i);
                    CodeEditor *tabEditor = (CodeEditor*)widget;
                    tabEditor->url = "";
                }
            }
        }else{
            QMessageBox::warning(this, tr("File error"), tr("The file could not be removed."));
        }
    }
}

void MainWindow::pCopy_triggered()
{
    if(!ui->treeView->hasFocus())
        return;

    QString path = dirModel->filePath(ui->treeView->currentIndex());

    if(path != ""){
        pCopyPath = path;
        isCutAction = false;
    }
}

void MainWindow::pCut_triggered()
{
    if(!ui->treeView->hasFocus())
        return;

    QString path = dirModel->filePath(ui->treeView->currentIndex());

    if(path != ""){
        pCopyPath = path;
        isCutAction = true;
    }
}

void MainWindow::pPast_triggered()
{
    if(!ui->treeView->hasFocus())
        return;

    QModelIndex index = ui->treeView->currentIndex();

    QString path = dirModel->filePath(index);
    QFileInfo fi(pCopyPath);
    bool pasted;

    if(path == pCopyPath){ // copy to root folder. root folder cannot be clicked
        path = dirModel->filePath(dirModel->index(projects[ui->projectSelector->currentText()]));
    }

    if(isCutAction){
        QFile file(pCopyPath);
        if((pasted = file.rename(path+"/"+fi.fileName())))
            isCutAction = false;

        fileRenamed(fi.baseName(), fi.fileName(), fi.fileName());
    }else{
        pasted = QFile::copy(pCopyPath,path+"/"+fi.fileName());
    }

    if(!pasted)
        QMessageBox::critical(this,tr("Error Pasting"), tr("The file could not be pasted. Either the file already exists or you do not have permissions to write."));
}

void MainWindow::on_actionPreferences_triggered()
{
    prefDialog->exec();
}

void MainWindow::on_tabWidget_tabBarClicked(int index)
{
    int pos = tabOrder.indexOf(index);
    if(pos > -1){
        tabOrder.move(pos,0);
    }
}

void MainWindow::on_actionShow_Toolbar_triggered(bool checked)
{
    ui->mainToolBar->setVisible(checked);
}

void MainWindow::on_actionRestore_Tab_triggered()
{
//    CodeEditor *resEditor = trashBin->at(0);
    ui->tabWidget->insertTab(0, trashBin, trashBin->url.section("/",-1,-1));
    ui->tabWidget->setCurrentIndex(0);
    ui->actionRestore_Tab->setEnabled(false);
}

void MainWindow::on_actionGo2Line_triggered()
{
    CodeEditor *mEditor = checkForEditor();
    if(mEditor == NULL)
        return;

    int lineNumber = QInputDialog::getInt(this, tr("Go to line"),tr("Line Number"),0,0,mEditor->blockCount());

    if(lineNumber == 0)
        return;

    QTextCursor curs = mEditor->textCursor();
    lineNumber = lineNumber - curs.blockNumber()-1;
    if(lineNumber == 0){
        return;
    }else if(lineNumber > 0){
        curs.movePosition(QTextCursor::NextBlock,QTextCursor::MoveAnchor, lineNumber);
    }else {
        curs.movePosition(QTextCursor::PreviousBlock,QTextCursor::MoveAnchor, lineNumber*-1);
    }
    mEditor->setTextCursor(curs);
}

void MainWindow::on_actionFind_triggered(){
    QWidget *widget = ui->tabWidget->currentWidget();
    ToolBox *toolBox = widget->findChild<ToolBox *>();
    CodeEditor *mEditor = widget->findChild<CodeEditor *>();
    QString text = mEditor->textUnderCursor();

    toolBox->show();
    toolBox->setFindFocus(text);
    ui->actionShowToolbox->setChecked(true);

}

void MainWindow::on_actionShowToolbox_triggered(bool checked)
{
    QWidget *widget = ui->tabWidget->currentWidget();
    ToolBox *toolBox = widget->findChild<ToolBox *>();
    if(checked)
        toolBox->show();
    else
        toolBox->hide();
}

void MainWindow::on_actionContext_help_triggered()
{
    QWidget *widget = ui->tabWidget->currentWidget();
    ToolBox *toolBox = widget->findChild<ToolBox *>();
    CodeEditor *mEditor = widget->findChild<CodeEditor *>();
    QString text = mEditor->textUnderCursor();
    int blockState = mEditor->currentTextBlockState;

    if(blockState > 9 || text.length() < 2 || text.at(0) == '$'){
        text.clear();
    }

    toolBox->show();
    toolBox->setHelpFocus(text);
}

void MainWindow::on_actionReplace_triggered()
{
    CodeEditor *mEditor = checkForEditor();
    if(mEditor == NULL)
        return;

    QString text = mEditor->textUnderCursor();
    toolBox->show();
    toolBox->setReplaceFocus(text);
}

void MainWindow::on_actionAbout_triggered()
{
    QString msg = tr("Version: ")+APP_VERSION+"\n";
            msg += tr("Build: ")+GIT_VERSION;
    QMessageBox::information(this, tr("About phpPad"), msg, QMessageBox::Ok);
}
