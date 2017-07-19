#include "toolBox.h"
#include "ui_toolBox.h"

ToolBox::ToolBox(QWidget *parent) : QWidget(parent), ui(new Ui::ToolBox){
    ui->setupUi(this);

    networkManager = new QNetworkAccessManager(this);

    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
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
        keyWord.replace(QString("_"), QString("-"));
        networkManager->get(QNetworkRequest(QUrl("http://php.net/manual/en/function."+keyWord+".php")));
    }
}

void ToolBox::setPhpCompleter(QStringList keyWords)
{
    phpCompleter = new QCompleter(keyWords, this);
    phpCompleter->setMaxVisibleItems(10);
    phpCompleter->setCompletionMode(QCompleter::PopupCompletion);
    ui->helpFilter->setCompleter(phpCompleter);
}

void ToolBox::replyFinished(QNetworkReply *pReply)
{
    QByteArray data = pReply->readAll();
    QString str(data);
    QRegularExpression rxStart = QRegularExpression("^.*?(?=<h1)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpression rxEnd = QRegularExpression("(?<=usernotes).*?$", QRegularExpression::DotMatchesEverythingOption);
    str.replace(rxStart, QString(""));
    str.replace(rxEnd, QString(""));
    ui->helpDisplay->setText(str);
}

void ToolBox::on_closeButton_clicked()
{
    this->hide();
}

void ToolBox::on_findButton_clicked()
{
    findNext(ui->searchInput->text());
}

void ToolBox::on_replaceNextButton_clicked()
{
    findNext(ui->replaceInput->text());
}

void ToolBox::on_findAllButton_clicked()
{
    oldCursor = mEditor->textCursor();
    QTextCursor newCursor = oldCursor;
    newCursor.setPosition(0);
    mEditor->setTextCursor(newCursor);

    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QColor(Qt::yellow).lighter(120));

    hitPositions.clear();
    ui->hitList->clear();

    while(mEditor->find(ui->searchInput->text())){
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

void ToolBox::on_helpFilter_returnPressed()
{
    QString tagName = ui->helpFilter->text();
    tagName.replace(QString("_"), QString("-"));
    networkManager->get(QNetworkRequest(QUrl("http://php.net/manual/en/function."+tagName+".php")));
}

bool ToolBox::findNext(QString keyWord)
{
    oldCursor = mEditor->textCursor();

    if(mEditor->find(keyWord)){
        mEditor->setFocus();
        return true;
    }

    // Try to start searching from the beginning
    QTextCursor newCursor = oldCursor;
    newCursor.setPosition(0);

    if(!oldCursor.atStart()){
        mEditor->setTextCursor(newCursor);
        if(mEditor->find(ui->searchInput->text())){
            mEditor->setFocus();
            return true;
        }
    }

    mEditor->setTextCursor(oldCursor);
    QMessageBox::information(this, tr("Search Error"), tr("The search string could not be found in the document."), QMessageBox::Ok);
    return false;
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

void ToolBox::on_replaceInput_editingFinished()
{
    if(ui->replaceInput->text().isEmpty()){
        ui->replaceInfoLabel->setText("");
        return;
    }

    QString replacedText = mEditor->toPlainText();
    int n = replacedText.count(ui->replaceInput->text());

    QString info = "\""+ui->replaceInput->text()+"\""+tr(" found %n time(s)","", n);
    ui->replaceInfoLabel->setText(info);
}
