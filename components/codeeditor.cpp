/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>

#include "codeeditor.h"
#include "highlighter.h"


CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent), c(nullptr)
{
    lineNumberArea = new LineNumberArea(this);
    includedFilesModel = new QStandardItemModel;
    lockBlockState = true;
    endOfWord = "~!@#$%^&*()+{}|:<>?,./;'[]\\= ";
    eow = "~!@#%^&*()+{}|:<>?,./;'[]\\=\" ";
    cDeligate = new CompleterDelegate;

    setTabChangesFocus(false);
    setTabStopWidth(20);

    QSettings settings("InnoBiz", "phpPad");

    settings.beginGroup("fonts");
        int fontSize = settings.value("standardFontSize", 0).toInt();
        defaultFont.setFamily(settings.value("standardFont", "Lucida Console").toString());
    settings.endGroup();

    if(QSysInfo::productType() == "macos" && fontSize == 0)
        defaultFont.setPointSize(11);
    else if(fontSize == 0)
        defaultFont.setPointSize(10);
    setFont(defaultFont);

    setPlaceholderText(tr("File is opened..."));

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(matchParentheses()));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(currentCurserPosition()));
    connect(this, SIGNAL(redoAvailable(bool)), this, SLOT(setRedoAvailable(bool)));
    connect(this, SIGNAL(textChanged()), this, SLOT(textHasChanged()));
    connect(this, SIGNAL(cursorPosition(QPoint)), cDeligate, SLOT(cursorPosition(QPoint)));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine(); // <- currently not working
}

void CodeEditor::keyReleaseEvent(QKeyEvent *e){
    if(e->text().trimmed() != ""){
        this->lastKey = e->text();
    }else if(e->key() == Qt::Key_Control){
        emit ctrlReleased();
    }
}

void CodeEditor::keyPressEvent(QKeyEvent *e){
    if (c && c->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }else if(e->key() == Qt::Key_Backtab){
        decreaseSelectionIndent();
        return;
    }else if(e->key() == Qt::Key_Tab){
        if(e->modifiers() & Qt::ControlModifier)
            emit switchTab();
        else
            increaseSelectionIndent();
        return;
    }else if(e->text() == "\""){
        if(lastKey == "\""){
            this->moveCursor(QTextCursor::Right, QTextCursor::MoveAnchor);
        }else{
            matchChracter("\"", "\"");
        }
        return;
    }else if(e->text() == "'"){
        if(lastKey == "'"){
            this->moveCursor(QTextCursor::Right, QTextCursor::MoveAnchor);
        }else{
            matchChracter("'", "'");
        }
        return;
    }else if(e->text() == "("){
        matchChracter("(", ")");
        return;
    }else if(e->text() == ")" && lastKey == "("){
        this->moveCursor(QTextCursor::Right, QTextCursor::MoveAnchor);
        return;
    }else if(e->text() == "["){
        matchChracter("[", "]");
        return;
    }else if(e->text() == "]" && lastKey == "["){
        this->moveCursor(QTextCursor::Right, QTextCursor::MoveAnchor);
        return;
    }else if(e->text() == "{"){
        matchChracter("{", "}");
        return;
    }else if(e->text() == "}" && lastKey == "{"){
        this->moveCursor(QTextCursor::Right, QTextCursor::MoveAnchor);
        return;
    }else if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter){
        if (currentTextBlockState == 10 && (e->modifiers() & Qt::ShiftModifier))
            this->insertPlainText("<br />");

        // make correct indent for new line
        matchTabstop(lastKey);
        return;
    }else if((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_W){
        emit closeEditor();
    }

    // Completer
    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space); // CTRL+Space
    if (!c || !isShortcut) // do not process the shortcut when we have a completer
        QPlainTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!c || (ctrlOrShift && e->text().isEmpty()))
        return;

    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;

    completionPrefix = textUnderCursor();

    // Check if in String or Comment
    bool inString = true;
    if(currentTextBlockState%10 == 0 || currentTextBlockState == 12 || currentTextBlockState == 21){
        inString = false;
    }

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 3 || endOfWord.contains(e->text().right(1)) || inString)) {
        c->popup()->hide();
        return;
    }

    // hide if nothing would be completed
    if(completionPrefix == c->completionModel()->data(c->completionModel()->index(0, 0)).toString()){
        c->popup()->hide();
        return;
    }

    if (completionPrefix != c->completionPrefix()) {
        c->setCompletionPrefix(completionPrefix);
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    if(c->objectName() != "js"){
        emit cursorPosition(QPoint(this->mapToGlobal(cr.topRight())));
    }
    cr.setWidth(5+c->popup()->sizeHintForColumn(0)+ c->popup()->sizeHintForColumn(1) + c->popup()->verticalScrollBar()->sizeHint().width());
    c->complete(cr); // popup it up!
}

void CodeEditor::matchChracter(QString startChar, QString endChar){
    QTextCursor curs = textCursor();

    if(!curs.hasSelection()){
        this->insertPlainText(startChar+endChar);
        this->moveCursor(QTextCursor::Left, QTextCursor::MoveAnchor);
        return;
    }
    QString insert = startChar+curs.selectedText()+endChar;
    curs.removeSelectedText();
    curs.insertText(insert);
}

void CodeEditor::matchTabstop(QString lastChar){
    // Get number of tab stops
    QString tabs = "";
    QTextCursor curs = textCursor();
    curs.select(QTextCursor::BlockUnderCursor);

    QRegularExpression rx("(?<=^.)(\\s+)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = rx.match(curs.selectedText());

    if(match.hasMatch())
        tabs = match.captured(0);

    this->insertPlainText("\n"+tabs);
    if(lastChar == "{"){
        this->insertPlainText("\t\n"+tabs);
        this->moveCursor(QTextCursor::Up, QTextCursor::MoveAnchor);
        this->moveCursor(QTextCursor::Right, QTextCursor::MoveAnchor);
        this->lastKey = " ";
    }
}

void CodeEditor::increaseSelectionIndent(){
    QTextCursor curs = textCursor();

    if(!curs.hasSelection()){
        this->insertPlainText("\t");
        return;
    }

    // Get the first and count of lines to indent.
    int spos = curs.anchor();
    int epos = curs.position();

    if(spos > epos){
        std::swap(spos, epos);
    }

    curs.setPosition(spos, QTextCursor::MoveAnchor);
    int sblock = curs.block().blockNumber();

    curs.setPosition(epos, QTextCursor::MoveAnchor);
    int eblock = curs.block().blockNumber();

    // Do the indent.
    curs.setPosition(spos, QTextCursor::MoveAnchor);
    curs.beginEditBlock();

    const int blockDifference = eblock - sblock;

    for(int i = 0; i <= blockDifference; ++i){
        curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        curs.insertText("\t");
        curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
    }
    curs.endEditBlock();

    // Set our cursor's selection to span all of the involved lines.
    curs.setPosition(spos, QTextCursor::MoveAnchor);
    curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);

    while(curs.block().blockNumber() < eblock){
        curs.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
    }

    curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    setTextCursor(curs);
}

void CodeEditor::decreaseSelectionIndent(){
    QTextCursor curs = textCursor();
    QString line;

    if(!curs.hasSelection()){
        curs.select(QTextCursor::BlockUnderCursor);
        line = curs.selectedText();

        if(line.isEmpty()){

        }else if(line.at(1) == '\t'){
            curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
            curs.deleteChar();
        }else if(line.at(1) == ' '){
            curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
            curs.deleteChar();
        }
        return;
    }

    // Get the first and count of lines to indent.
    int spos = curs.anchor();
    int epos = curs.position();

    if(spos > epos){
        std::swap(spos, epos);
    }

    curs.setPosition(epos, QTextCursor::MoveAnchor);
    int eblock = curs.block().blockNumber();


    curs.setPosition(spos, QTextCursor::MoveAnchor);
    int sblock = curs.block().blockNumber();

    const int blockDifference = eblock - sblock;

    // Do the indent.
    curs.beginEditBlock();
    for(int i = 0; i <= blockDifference; ++i){
        curs.select(QTextCursor::BlockUnderCursor);
        line = curs.selectedText();

        if(line.isEmpty()){

        }else if(line.at(1) == '\t'){
            curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
            curs.deleteChar();
        }else if(line.at(1) == ' '){
            curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
            curs.deleteChar();
        }

        curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
    }
    curs.endEditBlock();

    // Set our cursor's selection to span all of the involved lines.
    curs.setPosition(spos, QTextCursor::MoveAnchor);
    curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);

    while(curs.block().blockNumber() < eblock){
        curs.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
    }

    curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    setTextCursor(curs);
}

void CodeEditor::setPhpCompleterList(QStandardItemModel *compList){
    phpCompleterModel = compList;
    phpCompleter = new QCompleter(compList, this);
    phpCompleter->setObjectName("php");
    phpCompleter->popup()->setItemDelegate(cDeligate);
}
void CodeEditor::setHtmlCompleterList(QStandardItemModel *compList){
    htmlCompleterModel = compList;
    htmlCompleter = new QCompleter(compList, this);
    htmlCompleter->setObjectName("html");
    htmlCompleter->popup()->setItemDelegate(cDeligate);
}

void CodeEditor::setCssCompleterList(QStandardItemModel *compList){
    cssCompleter = new QCompleter(compList, this);
    cssCompleter->setObjectName("css");
    cssCompleter->popup()->setItemDelegate(cDeligate);
}

void CodeEditor::setJsCompleterList(QStringList compList){
    jsCompleterList = compList;
    jsCompleter = new QCompleter(compList, this);
    jsCompleter->setObjectName("js");
}

void CodeEditor::setCompleter(QCompleter *completer){

    if (c)
        QObject::disconnect(c, nullptr, this, nullptr);

    c = completer;

    if (!c)
        return;

    emit newCompleter(c->objectName());
    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
//    c->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(c, SIGNAL(activated(QModelIndex)), this, SLOT(insertCompletion(QModelIndex)));
}
QCompleter *CodeEditor::completer() const {
    return c;
}

void CodeEditor::insertCompletion(const QModelIndex &index){
    if (c->widget() != this)
        return;
    QTextCursor tc = textCursor();
    QString completion = c->completionModel()->data(index).toString();
    int extra = completion.length() - c->completionPrefix().length();

    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);

    if(c->objectName() != "js"){
        QModelIndex sibling = index.sibling(index.row(), 3);
        QString after = c->completionModel()->data(sibling).toString();
        if(after.isEmpty()){
            tc.insertText(completion.right(extra));
        }else{
            completion += after;
            tc.insertText(completion.right(extra+after.length()));
            tc.movePosition(QTextCursor::Left);
        }
    }else{
        tc.insertText(completion.right(extra));
    }
    setTextCursor(tc);
    completionPrefix.clear();
}

QString CodeEditor::textUnderCursor(const QString task) const{
    QTextCursor tc = textCursor();

    if(tc.hasSelection() || tc.atBlockStart())
        return "";

    if(task == "word"){
        tc.select(tc.WordUnderCursor);
        return tc.selectedText();
    }

    tc.movePosition(QTextCursor::StartOfBlock,QTextCursor::KeepAnchor);
    QString qText = tc.selectedText();
    QString mText = "";

    if(qText.length() < 3){
        return "";
    }

    for(int i=qText.length()-1;i>=0; i--){
        if(eow.contains(qText.at(i))){
            if(qText.at(i) == "\"" && i > 2){
                if(qText.mid(i-3,3) == "ss="){
                    mText = "."+mText;
                }else if(qText.mid(i-3,3) == "id="){
                    mText = "#"+mText;
                }
            }
            break;
        }else{
            mText = qText.at(i) + mText;
        }
    }

    return mText;
}
void CodeEditor::focusInEvent(QFocusEvent *e){
    if(c)
        c->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void CodeEditor::defaultFontChanged(QFont font)
{
    setFont(font);
}

int CodeEditor::lineNumberAreaWidth(){
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = fontMetrics().width(QLatin1Char('9')) * digits + fontMetrics().width(QLatin1Char(' '))*2;

    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine() // currently not working
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);


    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::darkGray);
            painter.setFont(defaultFont);
            painter.drawText(0, top, lineNumberArea->width()-5, fontMetrics().height(),
                             Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void CodeEditor::matchParentheses()
{
    QList<QTextEdit::ExtraSelection> selections;
    setExtraSelections(selections);

    TextBlockData *data = static_cast<TextBlockData *>(textCursor().block().userData());

    if (data) {
        QVector<ParenthesisInfo *> infos = data->parentheses();

        int pos = textCursor().block().position();
        for (int i = 0; i < infos.size(); ++i) {
            ParenthesisInfo *info = infos.at(i);

            int curPos = textCursor().position() - textCursor().block().position();
            if (info->position == curPos - 1 && info->character == '(') {
                if (matchLeftParenthesis(textCursor().block(), i + 1, 0))
                    createParenthesisSelection(pos + info->position);
            } else if (info->position == curPos - 1 && info->character == ')') {
                if (matchRightParenthesis(textCursor().block(), i - 1, 0))
                    createParenthesisSelection(pos + info->position);
            }
        }
    }
}

bool CodeEditor::matchLeftParenthesis(QTextBlock currentBlock, int i, int numLeftParentheses)
{
    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> infos = data->parentheses();

    int docPos = currentBlock.position();
    for (; i < infos.size(); ++i) {
        ParenthesisInfo *info = infos.at(i);

        if (info->character == '(') {
            ++numLeftParentheses;
            continue;
        }

        if (info->character == ')' && numLeftParentheses == 0) {
            createParenthesisSelection(docPos + info->position);
            return true;
        } else
            --numLeftParentheses;
    }

    currentBlock = currentBlock.next();
    if (currentBlock.isValid())
        return matchLeftParenthesis(currentBlock, 0, numLeftParentheses);

    return false;
}

bool CodeEditor::matchRightParenthesis(QTextBlock currentBlock, int i, int numRightParentheses)
{
    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> parentheses = data->parentheses();

    int docPos = currentBlock.position();
    for (; i > -1 && parentheses.size() > 0; --i) {
        ParenthesisInfo *info = parentheses.at(i);
        if (info->character == ')') {
            ++numRightParentheses;
            continue;
        }
        if (info->character == '(' && numRightParentheses == 0) {
            createParenthesisSelection(docPos + info->position);
            return true;
        } else
            --numRightParentheses;
    }

    currentBlock = currentBlock.previous();
    if (currentBlock.isValid())
        return matchRightParenthesis(currentBlock, 0, numRightParentheses);

    return false;
}

void CodeEditor::createParenthesisSelection(int pos)
{
    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    QTextEdit::ExtraSelection selection;
    QTextCharFormat format = selection.format;
    format.setBackground(Qt::green);
    selection.format = format;

    QTextCursor cursor = textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;

    selections.append(selection);

    setExtraSelections(selections);
}

void CodeEditor::setRedoAvailable(bool state){
    this->isRedoAvailable = state;
}

void CodeEditor::currentCurserPosition(){
    QTextCursor cursor = textCursor();
    QString pos = tr("Line")+" "+QString::number(cursor.blockNumber()+1)+", "+tr("Row")+" "+QString::number(cursor.positionInBlock());
    emit newCurserPosition(pos);
}

void CodeEditor::scanDocument(){
// This function is called when a new tab was added, the file is saved or the tab is activated (on included files changed)
// (Re)Scans the entire document and included files. Rebuilds the included file list and custom completers

    lockBlockState = true;
    bool newIncludedFiles = false;
    QString projectBasePath = url.section("/",0,-2) + "/";

    if(docType == 0){
        phpCustomCompModel = phpCompleterModel;
        includedFilesList.clear();

        QRegularExpression phpExp = QRegularExpression("<?.*?>", QRegularExpression::DotMatchesEverythingOption);
        QRegularExpression includeExpression = QRegularExpression("(?<=include)(?>.)*(?<='|\\\")(?<path>.+\\.\\w{2,4})(?='|}\\\")");

        QRegularExpressionMatchIterator phpCode = phpExp.globalMatch(this->toPlainText());
        while(phpCode.hasNext()){
            QRegularExpressionMatch phpCodeText = phpCode.next();

            // Scan for variables in other files
            QRegularExpressionMatchIterator matches = includeExpression.globalMatch(phpCodeText.captured());
            while (matches.hasNext()) {
                QRegularExpressionMatch match = matches.next();
                QString path = projectBasePath + match.captured("path");
                QFileInfo info(path);
                if(!includedFilesList.contains(path) && info.exists()){
                    includedFilesList << path;
                    newIncludedFiles = true;
                    QFile file(path);

                    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
                        QTextStream in(&file);
                        QString text = in.readAll();
                        file.close();

                        QRegularExpressionMatchIterator phpIncludeCode = phpExp.globalMatch(text);
                        while(phpIncludeCode.hasNext()){
                            QRegularExpressionMatch phpIncludedCodeText = phpIncludeCode.next();
                            scanForPhp(phpIncludedCodeText.captured(), url.section("/",-1,-1));
                        }
                    }
                }
            }

            // Scan for variables in current file
            scanForPhp(phpCodeText.captured(), tr("local"));
        }

        phpCustomCompModel->sort(0);

        phpCompleter = new QCompleter(phpCustomCompModel, this);
        phpCompleter->setObjectName("php");
        phpCompleter->popup()->setItemDelegate(cDeligate);
        setCompleter(phpCompleter);
    }

    if(docType == 0 || docType == 2){
        // look for includes in html
        bool newHtml = false;
        htmlCustomCompModel = htmlCompleterModel;

        QRegularExpression htmlHead = QRegularExpression("<head>.*</head>", QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatchIterator htmlHeader = htmlHead.globalMatch(this->toPlainText());

        while(htmlHeader.hasNext()){
            // CSS
            QRegularExpressionMatch headerText = htmlHeader.next();
            QRegularExpression cssLinkExp = QRegularExpression("(?<=link).*\"(?<path>.+\\.css.*?)\"");
            QRegularExpressionMatchIterator css = cssLinkExp.globalMatch(headerText.captured());

            while(css.hasNext()){
                QRegularExpressionMatch match = css.next();
                QString path = projectBasePath + match.captured("path");

                QFileInfo info(path);
                if(!includedFilesList.contains(path) && info.exists()){
                    includedFilesList << path;
                    newIncludedFiles = true;
                    newHtml = true;
                    QFile file(path);
                    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
                        QTextStream in(&file);
                        QString text = in.readAll();
                        file.close();
                        scanForCss(text);
                    }
                }
            }
        }

        if(newHtml){
            htmlCustomCompModel->sort(0);

            htmlCompleter = new QCompleter(htmlCustomCompModel, this);
            htmlCompleter->setObjectName("html");
            htmlCompleter->popup()->setItemDelegate(cDeligate);
            setCompleter(htmlCompleter);
        }
    }

    if(docType == 0 || docType == 3){
        // look for includes in html
        bool newJs = false;
        jsCustomCompList = jsCompleterList;

        QRegularExpression htmlHead = QRegularExpression("<head>.*</head>", QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatchIterator htmlHeader = htmlHead.globalMatch(this->toPlainText());

        while(htmlHeader.hasNext()){
            // JS
            QRegularExpressionMatch headerText = htmlHeader.next();
            QRegularExpression jsLinkExp = QRegularExpression("(?<=script).*\"(?<path>.+\\.js.*?)\"");
            QRegularExpressionMatchIterator js = jsLinkExp.globalMatch(headerText.captured());

            while(js.hasNext()){
                QRegularExpressionMatch match = js.next();
                QString path = projectBasePath + match.captured("path");

                QFileInfo info(path);
                if(!includedFilesList.contains(path) && info.exists()){
                    includedFilesList << path;
                    newIncludedFiles = true;
                    newJs = true;
                    QFile file(path);
                    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
                        QTextStream in(&file);
                        QString text = in.readAll();
                        file.close();
                        scanForJs(text);
                    }
                }
            }
        }

        if(newJs){
            std::sort(jsCustomCompList.begin(), jsCustomCompList.end());

            jsCompleter = new QCompleter(jsCustomCompList, this);
            jsCompleter->setObjectName("js");
            setCompleter(jsCompleter);
        }
    }

    if(newIncludedFiles){
        std::sort(includedFilesList.begin(), includedFilesList.end());
        includedFilesModel->clear();

        foreach(QString path, includedFilesList){
            QStandardItem *item = new QStandardItem();
            item->setData(path, Qt::StatusTipRole);
            item->setData(path.section("/",-1,-1), Qt::DisplayRole);

            includedFilesModel->appendRow(item);
        }
        emit updateIncludedFiles();
    }

    lockBlockState = false;
}

void CodeEditor::updateIncFiles()
{
    foreach(QString url, filesForRescan){
//        qDebug() << "Need to update "+url;
        if(url != this->url){
            QFile file(url);
            if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
                QTextStream in(&file);
                QString text = in.readAll();
                file.close();

                QString fileType = url.section(".",-1);
                if(fileType == "php")
                    scanForPhp(text, url.section("/",-1,-1));
                else if(fileType == "css")
                    scanForCss(text);
                else if(fileType == "js")
                    scanForJs(text);
            }
        }
    }
    filesForRescan.clear();
}

void CodeEditor::scanForPhp(QString testString, QString file){
    QRegularExpression variableExpression = QRegularExpression("\\$[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]{2,}");
    QRegularExpression functionExpression = QRegularExpression("(?<name>(?<=function )[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*(?=\\s*\\())(\\()(?<parameter>.+?(?=\\)))");

    QRegularExpressionMatchIterator matches = variableExpression.globalMatch(testString);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();

        QList<QStandardItem *> result = phpCustomCompModel->findItems(match.captured());
        if(result.length() == 0){
            QList<QStandardItem *> tmpList;
            tmpList << new QStandardItem(match.captured());
            tmpList << new QStandardItem(file);
            phpCustomCompModel->appendRow(tmpList);
        }
    }

    matches = functionExpression.globalMatch(testString);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        QList<QStandardItem *> tmpList;
        tmpList << new QStandardItem(match.captured("name"));
        tmpList << new QStandardItem(file);
        tmpList << new QStandardItem("<b>"+match.captured("name")+"</b>("+match.captured("parameter")+")");
        phpCustomCompModel->appendRow(tmpList);
    }
}

void CodeEditor::scanForJs(QString code, int scanOption)
{
    QRegularExpression variableExpression;
    if(scanOption == 1){
        // Scan for variable names
        variableExpression.setPattern("var [a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]{2,}");

    }else{
        // Scan for gloabl variables
        variableExpression.setPattern("^var [a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]{2,}");
    }

    QRegularExpressionMatchIterator matches = variableExpression.globalMatch(code);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();

        if(match.captured().length() > 2){
            QStringList::const_iterator iter = std::find(jsCustomCompList.begin(), jsCustomCompList.end(), match.captured());
            if(iter == jsCustomCompList.end()){
                jsCustomCompList << match.captured();
                qDebug() << "adding "+match.captured()+" to JS completer";
            }
        }
    }

    // Scan for function names
    QRegularExpression functionExpression = QRegularExpression("(?<=function )[a-zA-Z_\x7f-\xff][a-zA-Z0-9_\x7f-\xff]*(?=\\s*\\()");
    matches = functionExpression.globalMatch(code);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        if(match.capturedLength() > 2)
            jsCustomCompList << match.captured();
    }
}

void CodeEditor::scanForCss(QString code)
{
    QRegularExpression classExpression = QRegularExpression("(?<type>\\.|#)(?<name>[a-zA-Z]+[\\w-]*)(?= *{|,)");
    QRegularExpressionMatchIterator matches = classExpression.globalMatch(code);
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();

        if(match.capturedLength() > 3){

            QList<QStandardItem *> result = htmlCustomCompModel->findItems(match.captured());
            if(result.length() == 0){
                QList<QStandardItem *> tmpList;
                tmpList << new QStandardItem(match.captured("type")+match.captured("name"));
                tmpList << new QStandardItem(match.captured("type") == "#" ? "id":"class");
                htmlCustomCompModel->appendRow(tmpList);
            }
        }
    }
}

void CodeEditor::textBlockStateChanged(int state){
    if(state == currentTextBlockState || lockBlockState)
        return;

    currentTextBlockState = state;
    // change completer
    if(state >= 0 && state < 10){
        setCompleter(phpCompleter);
    }else if(state < 20){
        setCompleter(htmlCompleter);
    }else if(state < 30){
        setCompleter(cssCompleter);
    }else if(state < 40){
        setCompleter(jsCompleter);
    }

}

void CodeEditor::fileChangeListener(QString url)
{
//    qDebug() << includedFilesList;
//    qDebug() << url;
    if(includedFilesList.contains(url) && !filesForRescan.contains(url)){
        filesForRescan << url;
    }
}

void CodeEditor::insertTable(int columns, int rows, int header, QString caption){
    // Get number of tab stops
    QString tabs = "\n";
    QTextCursor curs = textCursor();
    curs.select(QTextCursor::BlockUnderCursor);

    QRegularExpression rx("(?<=^.)(\\s)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = rx.match(curs.selectedText());

    if(match.hasMatch())
        tabs += match.captured(0);

    this->insertPlainText(tabs+"<table>");
    if(!caption.isEmpty())
        this->insertPlainText(tabs+"\t<caption>"+caption+"</caption>");
    for(int i = 0; i<rows; ++i){
        this->insertPlainText(tabs+"\t<tr>");
        for(int j = 0; j<columns; ++j){
            if((header == -3 && i == 0)||(header == -4 && j == 0))
                this->insertPlainText(tabs+"\t\t<th></th>");
            else
                this->insertPlainText(tabs+"\t\t<td></td>");
        }
        this->insertPlainText(tabs+"\t</tr>");
    }

    this->insertPlainText(tabs+"</table>");
}

void CodeEditor::insertImage(QString path, QString alt, QString caption, int width, int height){
    QString tabs = "\n";
    QTextCursor curs = textCursor();
    curs.select(QTextCursor::BlockUnderCursor);

    QRegularExpression rx("(?<=^.)(\\s)", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = rx.match(curs.selectedText());

    if(match.hasMatch())
        tabs += match.captured(0);

    QFileInfo file(url);
    QDir dir(file.absolutePath());
    QString relPath = dir.relativeFilePath(path);

    if(caption.isEmpty()){
        this->insertPlainText(tabs+"<img src=\""+relPath+"\" alt=\""+alt+"\" width=\""+QString::number(width)+"\" height=\""+QString::number(height)+"\" >");
    }else{
        this->insertPlainText(tabs+"<figure>");
        this->insertPlainText(tabs+"\t<img src=\""+relPath+"\" alt=\""+alt+"\" width=\""+QString::number(width)+"\" height=\""+QString::number(height)+"\" >");
        this->insertPlainText(tabs+"\t<figcaption>"+caption+"</figcaption>");
        this->insertPlainText(tabs+"</figure>");
    }
}

void CodeEditor::textHasChanged()
{
    if(!this->isFileChanged){
        this->isFileChanged = true;
        emit fileChanged(this->url);
    }
}
