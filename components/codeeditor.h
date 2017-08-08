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

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>
#include <QKeyEvent>
#include <QCompleter>
#include <QStandardItemModel>
#include <QSysInfo>
#include <QVariant>
#include <QSettings>
#include <QDebug>

#include "completerdelegate.h"

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;

class LineNumberArea;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = 0);
    QString textFile;
    QString url;
    int docType;
    bool isFileChanged = true;
    bool isRedoAvailable = false;
    QStandardItemModel *includedFilesModel;
    int currentTextBlockState;
    QCompleter *completer() const;
    QFont defaultFont;
    QStringList includedFilesList;
    bool lockBlockState;
    QString textUnderCursor() const;

    void setPhpCompleterList(QStandardItemModel *compList);
    void setHtmlCompleterList(QStandardItemModel *compList);
    void setCssCompleterList(QStringList compList);
    void setJsCompleterList(QStringList compList);
    void setCompleter(QCompleter *completer);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void scanDocument();
    void updateIncFiles();

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *e)  Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void focusInEvent(QFocusEvent *e) Q_DECL_OVERRIDE;

signals:
    void fileChanged(QString);
    void newCurserPosition(QString);
    void cursorPosition(QPoint);
    void updateIncludedFiles();
    void switchTab();
    void ctrlReleased();
    void closeEditor();
    void newCompleter(QString);

public slots:
    void defaultFontChanged(QFont font);
    void textBlockStateChanged(int state);
    void fileChangeListener(QString url);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void matchParentheses();
    void increaseSelectionIndent();
    void decreaseSelectionIndent();
    void insertCompletion(const QModelIndex &index);
    void setRedoAvailable(bool state);
    void currentCurserPosition();
    void insertTable(int columns, int rows, int header, QString caption);
    void textHasChanged();

private:
    QWidget *lineNumberArea;
    QString lastKey;
    QCompleter *c;
    QCompleter *phpCompleter;
    QCompleter *htmlCompleter;
    QCompleter *cssCompleter;
    QCompleter *jsCompleter;
    QStandardItemModel *phpCompleterModel;
    QStringList jsCompleterList;
    QStandardItemModel *htmlCompleterModel;
    QStandardItemModel *phpCustomCompModel;
    QStringList jsCustomCompList;
    QStandardItemModel *htmlCustomCompModel;
    QStringList filesForRescan;
    QString endOfWord;
    QString eow;
    QString completionPrefix;
    CompleterDelegate *cDeligate;

    bool matchLeftParenthesis(QTextBlock currentBlock, int index, int numRightParentheses);
    bool matchRightParenthesis(QTextBlock currentBlock, int index, int numLeftParentheses);
    void createParenthesisSelection(int pos);
    void matchChracter(QString startChar, QString endChar);
    void matchTabstop(QString lastChar);
    void scanForPhp(QString testString, QString file);
    void scanForJs(QString code, int scanOption = 0);
    void scanForCss(QString code);
};


class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor) {
        codeEditor = editor;
    }

    QSize sizeHint() const Q_DECL_OVERRIDE {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;
};


#endif
