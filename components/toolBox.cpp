#include "toolBox.h"
#include "ui_toolBox.h"

ToolBox::ToolBox(QWidget *parent) : QWidget(parent), ui(new Ui::ToolBox){
    ui->setupUi(this);

    networkManager = new QNetworkAccessManager(this);
    history = new QList<QUrl>;
    historyIndex = -1;

    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(this, SIGNAL(newHistoryIndex()), this, SLOT(historyIndeyChanged()));
}

ToolBox::~ToolBox()
{
    delete ui;
}

void ToolBox::setFindFocus(QString keyWord)
{
    ui->toolBox->setCurrentIndex(0);
    ui->searchInput->setText(keyWord);
    ui->searchInput->setFocus();
}

void ToolBox::setReplaceFocus(QString keyWord)
{
    ui->toolBox->setCurrentIndex(1);
    ui->replaceInput->setText(keyWord);
    ui->replaceInput->setFocus();
}

void ToolBox::setHelpFocus(QString keyWord)
{
    ui->toolBox->setCurrentIndex(2);
    ui->helpFilter->setFocus();

    if(!keyWord.isNull()){
        ui->helpFilter->setText(keyWord);
        on_helpFilter_returnPressed();
    }
}

void ToolBox::setPhpCompleter(QStandardItemModel *keyWords)
{
    phpCompleter = new QCompleter(keyWords, this);
    phpCompleter->setMaxVisibleItems(10);
    phpCompleter->setCompletionMode(QCompleter::PopupCompletion);
    ui->helpFilter->setCompleter(phpCompleter);
}

void ToolBox::on_closeButton_clicked(){
    this->hide();
}

void ToolBox::on_findButton_clicked(){
    findNext(ui->searchInput->text());
}

void ToolBox::on_replaceNextButton_clicked(){
    findNext(ui->replaceInput->text());
}

void ToolBox::on_findAllButton_clicked(){
    oldCursor = mEditor->textCursor();
    QTextCursor newCursor = oldCursor;
    newCursor.setPosition(0);
    mEditor->setTextCursor(newCursor);

    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QColor(Qt::yellow).lighter(120));

    hitPositions.clear();
    ui->hitList->clear();

    while(mEditor->find(ui->searchInput->text(), flag)){
        selection.cursor = mEditor->textCursor();
        extraSelections.append(selection);

        newCursor = mEditor->textCursor();
        hitPositions.append(newCursor.position());
        newCursor.select(QTextCursor::LineUnderCursor);
        QString hit = newCursor.selectedText();
        hit.replace(QRegularExpression("\\s{2,}|\\t"), "");
        ui->hitList->addItem(hit);
    }

    mEditor->setTextCursor(oldCursor);

    if(hitPositions.count() > 0){
        mEditor->setExtraSelections(extraSelections);
        ui->hitList->show();
    }else{
        QMessageBox::information(this, tr("Search Error"), tr("The search string could not be found in the document."), QMessageBox::Ok);
    }
}

void ToolBox::on_hitList_doubleClicked(const QModelIndex &index)
{
    QTextCursor curs;
    curs = mEditor->textCursor();
    curs.setPosition(hitPositions.at(index.row()));
    mEditor->setTextCursor(curs);
    mEditor->setFocus();
}

bool ToolBox::findNext(QString keyWord)
{
    oldCursor = mEditor->textCursor();
    bool reverse = ui->reverseCheckBox->isChecked();

    if(mEditor->find(keyWord, flag)){
        mEditor->setFocus();
        return true;
    }

    // Try to start searching from the beginning/end
    QTextCursor newCursor = oldCursor;
    if(reverse){
        newCursor.setPosition(mEditor->document()->characterCount()-1);
    }else{
        newCursor.setPosition(0);
    }

    if((!reverse && !oldCursor.atStart()) || (reverse && !oldCursor.atEnd())){
        mEditor->setTextCursor(newCursor);
        if(mEditor->find(keyWord, flag)){
            mEditor->setFocus();
            return true;
        }
    }

    mEditor->setTextCursor(oldCursor);
    QMessageBox::information(this, tr("Search Error"), tr("The search string could not be found in the document."), QMessageBox::Ok);
    return false;
}

void ToolBox::updateFindFlags()
{
    flag = NULL;
    if (ui->reverseCheckBox->isChecked())
        flag |= QTextDocument::FindBackward;

    if (ui->caseCheckBox->isChecked())
        flag |= QTextDocument::FindCaseSensitively;

    if (ui->wholeWordCheckBox->isChecked())
        flag |= QTextDocument::FindWholeWords;
}

void ToolBox::on_replaceButton_clicked()
{
    if(!findNext(ui->replaceInput->text()))
        return;

    QTextCursor cursor = mEditor->textCursor();

    if(cursor.hasSelection())
        cursor.insertText(ui->replacementInput->text());

    on_replaceInput_editingFinished();
}

void ToolBox::on_replaceAllButton_clicked()
{
    QString replacedText = mEditor->toPlainText();

    replacedText.replace(ui->replaceInput->text(), ui->replacementInput->text());
    mEditor->setPlainText(replacedText);
    on_replaceInput_editingFinished();
}

void ToolBox::on_replaceInput_editingFinished(){
    if(ui->replaceInput->text().isEmpty()){
        ui->replaceInfoLabel->setText("");
        return;
    }

    QString replacedText = mEditor->toPlainText();
    int n = replacedText.count(ui->replaceInput->text());

    QString info = "\""+ui->replaceInput->text()+"\""+tr(" found %n time(s)","", n);
    ui->replaceInfoLabel->setText(info);
}

void ToolBox::on_caseCheckBox_clicked(){
    updateFindFlags();
}

void ToolBox::on_reverseCheckBox_clicked(){
    updateFindFlags();
}

void ToolBox::on_wholeWordCheckBox_clicked(){
    updateFindFlags();
}

void ToolBox::on_toolBox_currentChanged(int index)
{
    if(index != 1)
        return;

    ui->replaceInput->setText(ui->searchInput->text());
    ui->replaceCaseCheck->setChecked(ui->caseCheckBox->isChecked());
}

void ToolBox::on_searchInput_cursorPositionChanged(int arg1, int arg2)
{
    if(arg1 < arg2)
        return;

    ui->hitList->clear();
}

// Help
void ToolBox::on_helpFilter_returnPressed()
{
    QString tagName = ui->helpFilter->text();
    tagName.replace(QString("_"), QString("-"));
    extUrl = QUrl("http://php.net/manual/en/function."+tagName+".php");
    history->append(extUrl);
    historyIndex = history->length()-1;
    newHistoryIndex();
    networkManager->get(QNetworkRequest(extUrl));
}

void ToolBox::on_helpDisplay_anchorClicked(const QUrl &arg1){
    extUrl = arg1;
    history->append(arg1);
    historyIndex = history->length()-1;
    newHistoryIndex();
    networkManager->get(QNetworkRequest(extUrl));
    ui->helpFilter->setText("");
}

void ToolBox::on_navigateBack_clicked(){
    historyIndex--;
    newHistoryIndex();
    networkManager->get(QNetworkRequest(history->at(historyIndex)));
}

void ToolBox::on_navigateForward_clicked(){
    historyIndex++;
    newHistoryIndex();
    networkManager->get(QNetworkRequest(history->at(historyIndex)));
}

void ToolBox::replyFinished(QNetworkReply *pReply)
{
    QByteArray data = pReply->readAll();
    QString str(data);
    QRegularExpression rxStart = QRegularExpression("^.*?(?=<h1)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpression rxEnd = QRegularExpression("(?<=usernotes).*?$", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpression link = QRegularExpression("href=\"");
    str.replace(rxStart, QString(""));
    str.replace(rxEnd, QString(""));
    str.replace(link, QString("href=\"http://php.net/manual/en/"));
    ui->helpDisplay->setHtml(str);
    ui->helpDisplay->setFocus();
}

void ToolBox::historyIndeyChanged(){
    int range = history->length()-1;
    if(range == 0){
        ui->navigateBack->setEnabled(false);
        ui->navigateForward->setEnabled(false);
        return;
    }

    if(historyIndex == 0){
        ui->navigateBack->setEnabled(false);
    }else{
        ui->navigateBack->setEnabled(true);
    }

    if(historyIndex == range){
        ui->navigateForward->setEnabled(false);
    }else{
        ui->navigateForward->setEnabled(true);
    }
}
