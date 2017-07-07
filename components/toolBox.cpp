#include "toolBox.h"
#include "ui_ToolBox.h"

ToolBox::ToolBox(QWidget *parent) : QWidget(parent), ui(new Ui::ToolBox){
    ui->setupUi(this);
}

ToolBox::~ToolBox()
{
    delete ui;
}

void ToolBox::setFindFocus()
{
    ui->searchInput->setFocus();
}

void ToolBox::on_closeButton_clicked()
{
    this->hide();
}

void ToolBox::on_findButton_clicked()
{
    oldCursor = mEditor->textCursor();

    if(mEditor->find(ui->searchInput->text())){
        mEditor->setFocus();
        return;
    }

    // Try to start searching from the beginning
    QTextCursor newCursor = oldCursor;
    newCursor.setPosition(0);

    if(!oldCursor.atStart()){
        mEditor->setTextCursor(newCursor);
        if(mEditor->find(ui->searchInput->text())){
            mEditor->setFocus();
            return;
        }
    }

    mEditor->setTextCursor(oldCursor);
    QMessageBox::information(this, tr("Search Error"), tr("The search string could not be found in the document."), QMessageBox::Ok);
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
