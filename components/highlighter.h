/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the documentation of Qt. It was originally
** published as part of Qt Quarterly.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights.  These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QVector>
#include <QStringListModel>
#include <QRegularExpression>

class QString;
class QTextDocument;

struct ParenthesisInfo {
    char character;
    int position;
};

class TextBlockData : public QTextBlockUserData {
public:
    TextBlockData();
    QVector<ParenthesisInfo *> parentheses();
    void insert(ParenthesisInfo *info);

private:
    QVector<ParenthesisInfo *> m_parentheses;
};

class Highlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    Highlighter(QTextDocument *document, QStringListModel *keyWords, QStringListModel *funcNames, QStringList *cssKeyWords, QStringList *jsKeyWords);
    void setDocType(const int docType);

signals:
    void currentBlockStateChanged(int);

protected:
    void highlightBlock(const QString &text);

private:
    QMap<QString, QTextCharFormat> phpKeyWordsRules;
    QMap<QString, QTextCharFormat> htmlKeyWordsRules;
    QMap<QString, QTextCharFormat> cssKeyWordsRules;
    QMap<QString, QTextCharFormat> jsKeyWordsRules;

    QTextCharFormat currentTagFormat;
    QTextCharFormat standardTagFormat;

    QTextCharFormat phpKeywordFormat;
    QTextCharFormat phpTagFormat;
    QTextCharFormat phpCommentFormat;
    QTextCharFormat phpQuoteFormat;
    QTextCharFormat phpFunctionFormat;
    QTextCharFormat phpVariableFormat;
    QTextCharFormat phpNumberFormat;
    QTextCharFormat phpBoolFormat;

    QTextCharFormat htmlTagFormat;
    QTextCharFormat htmlGenTagFormat;
    QTextCharFormat htmlTableFormat;
    QTextCharFormat htmlFormFormat;
    QTextCharFormat htmlQuoteFormat;
    QTextCharFormat htmlCommentFormat;

    QTextCharFormat cssKeyWordFormat;
    QTextCharFormat cssClassFormat;
    QTextCharFormat cssIdFormat;
    QTextCharFormat cssTagFormat;
    QTextCharFormat cssCommentFormat;

    QTextCharFormat jsKeyWordFormat;
    QTextCharFormat jsStringFormat;
    QTextCharFormat jsNumberFormat;
    QTextCharFormat jsBoolFormat;
    QTextCharFormat jsCommentFormat;

    QRegularExpressionMatchIterator parser;
    QString currentLine;
    int currentTextBlockState;
    int phpReturnState;
    int htmlReturnState;
    int mDocType;

    void closeTag(QString tag, int returnState, int startPos = 0);
};

#endif
