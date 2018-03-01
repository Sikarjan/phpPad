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

#include <QtGui>

#include "highlighter.h"

TextBlockData::TextBlockData()
{
    // Nothing to do
}

QVector<ParenthesisInfo *> TextBlockData::parentheses()
{
    return m_parentheses;
}


void TextBlockData::insert(ParenthesisInfo *info)
{
    int i = 0;
    while (i < m_parentheses.size() &&
        info->position > m_parentheses.at(i)->position)
        ++i;

    m_parentheses.insert(i, info);
}

Highlighter::Highlighter(QTextDocument *document, QStringListModel *keyWords, QStringListModel *funcNames, QStringList *cssKeyWords, QStringList *jsKeyWords): QSyntaxHighlighter(document){

    currentTextBlockState = -1;
    phpReturnState = -1;
    htmlReturnState = 10;
    mDocType = 10;

// PHP formatting
    phpTagFormat.setForeground(Qt::red);
    phpVariableFormat.setForeground(Qt::blue);
    phpCommentFormat.setForeground(Qt::gray);
    phpQuoteFormat.setForeground(Qt::darkGreen);
    phpBoolFormat.setForeground(QColor(0,100,0));
    phpNumberFormat.setForeground(Qt::darkRed);

    phpKeywordFormat.setForeground(QColor(128, 128, 128));
    phpKeywordFormat.setFontWeight(QFont::Normal);
    phpKeywordFormat.setFontItalic(true);
    int rows = keyWords->rowCount();
    for (int j = 0; j < rows; j++) {
       phpKeyWordsRules.insert(keyWords->data(keyWords->index(j), Qt::DisplayRole).toString(), phpKeywordFormat);
    }
    phpKeyWordsRules.insert("true", phpBoolFormat);
    phpKeyWordsRules.insert("false", phpBoolFormat);

    phpFunctionFormat.setForeground(Qt::darkBlue);
    rows = funcNames->rowCount();
    for (int k = 0; k < rows; k++) {
        phpKeyWordsRules.insert(funcNames->data(funcNames->index(k), Qt::DisplayRole).toString(), phpFunctionFormat);
    }

// HTML formatting
    htmlGenTagFormat.setForeground(Qt::blue);
    htmlTableFormat.setForeground(Qt::darkCyan);
    htmlFormFormat.setForeground(QColor(255, 153, 0));
    htmlQuoteFormat.setForeground(Qt::darkGreen);
    htmlCommentFormat.setForeground(Qt::gray);

    QStringList htmlList;
    htmlList << "DOCTYPE" << "a" << "abbr" << "address" << "area" << "article" << "aside" << "audio" << "b" << "base" << "bdi" << "bdo" << "blockquote" << "body" << "br" << "button" << "canvas" << "caption" << "cite" << "code" << "col" << "colgroup" << "command" << "datalist" << "dd" << "del" << "details" << "dfn" << "dir" << "div" << "dl" << "dt" << "em" << "embed" << "fieldset" << "figcaption" << "figure" << "footer" << "h" << "head" << "header" << "hgroup" << "hr" << "html" << "i" << "iframe" << "img" << "ins" << "kbd" << "keygen" << "label" << "legend" << "li" << "link" << "map" << "mark" << "menu" << "meta" << "meter" << "nav" << "noscript" << "object" << "ol" << "optgroup" << "option" << "output" << "p" << "param" << "pre" << "progress" << "q" << "rp" << "rt" << "ruby" << "s" << "samp" << "script" << "section" << "small" << "source" << "span" << "strong" << "style" << "sub" << "summary" << "sup" << "textarea" << "tfoot" << "time" << "title" << "track" << "u" << "ul" << "var" << "video" << "wbr";
    foreach (QString tag, htmlList) {
        htmlKeyWordsRules.insert(tag, htmlGenTagFormat);
    }

    QStringList htmlTableList;
    htmlTableList << "table" << "tr" << "th" << "td" << "tbody" << "thead";
    foreach (QString tag, htmlTableList) {
        htmlKeyWordsRules.insert(tag, htmlTableFormat);
    }

    QStringList htmlFormList;
    htmlFormList << "form" << "input" << "select";
    foreach (QString tag, htmlFormList) {
        htmlKeyWordsRules.insert(tag, htmlFormFormat);
    }

// CSS formatting
    cssKeyWordFormat.setForeground(Qt::darkBlue);
    cssClassFormat.setForeground(Qt::darkMagenta);
    cssIdFormat.setForeground(QColor(204, 0, 51));
    cssTagFormat.setForeground(Qt::darkRed);
    cssCommentFormat.setForeground(Qt::gray);

    rows = cssKeyWords->count();
    for (int l = 0; l < rows; l++) {
        cssKeyWordsRules.insert(cssKeyWords->at(l), cssKeyWordFormat);
    }

// JS formatting
    jsKeyWordFormat.setForeground(Qt::darkBlue);
    jsStringFormat.setForeground(Qt::darkGreen);
    jsNumberFormat.setForeground(Qt::darkRed);
    jsBoolFormat.setForeground(Qt::darkRed);
    jsCommentFormat.setForeground(Qt::gray);

    rows = jsKeyWords->count();
    for (int m = 0; m < rows; m++){
        jsKeyWordsRules.insert(jsKeyWords->at(m), jsKeyWordFormat);
    }

    jsKeyWordsRules.insert("true", jsBoolFormat);
    jsKeyWordsRules.insert("false", jsBoolFormat);
}

void Highlighter::highlightBlock(const QString &text){
    currentLine = text;
    // highlight matching parenthesis
    TextBlockData *data = new TextBlockData;

    int leftPos = text.indexOf('(');
    while (leftPos != -1) {
        ParenthesisInfo *info = new ParenthesisInfo;
        info->character = '(';
        info->position = leftPos;

        data->insert(info);
        leftPos = text.indexOf('(', leftPos + 1);
    }

    int rightPos = text.indexOf(')');
    while (rightPos != -1) {
        ParenthesisInfo *info = new ParenthesisInfo;
        info->character = ')';
        info->position = rightPos;

        data->insert(info);

        rightPos = text.indexOf(')', rightPos +1);
    }

    setCurrentBlockUserData(data);

//- Text parser -
    // check if there is something to parse
    if(text.simplified().trimmed().isEmpty() || mDocType == -10){
        setCurrentBlockState(previousBlockState());
        return;
    }
    // Split line by coding language identifiers, here <? and ?>, then by (key)words and then by any other char that is not escaped.
    QRegularExpression splitter = QRegularExpression("(<\\?)|(\\?>)|(<script type=\"text\\/javascript\">)|(<\\/script>)|(<style>)|(<\\/style>)|(\\/\\*)|(\\*\\/)|(<!--)|(-->)|([a-zA-Z0-9-_]+)|((?<!\\\\)([^\\s\\\\]))");
    parser = splitter.globalMatch(text);

//qDebug() <<"Starting with" << currentBlockState() << "last state was" << previousBlockState() << "looking at" << text;
/*
Block states:
0 standard php      10 standard html        20 standard css         30 standard js
1 php "-quote       11 html comment         21 css declaration      31 js comment
2 php '-quote       12 html "-quote         22 css comment
3 php comment       13
4
5
6
7
8
9
 */
    // check if the last block can tell us what language we should start in
    if(previousBlockState() != -1){
        setCurrentBlockState(previousBlockState());
    }else{
        // Assuming it should be the standard doc type then
        setCurrentBlockState(mDocType);
    }

    // Close any open comments/strings if necessary
    if(currentBlockState() == 1){
        currentTagFormat = phpQuoteFormat;
        closeTag("\"", 0);
    }else if(currentBlockState() == 2){
        currentTagFormat = phpQuoteFormat;
        closeTag("'", 0);
    }else if(currentBlockState() == 3){
        currentTagFormat = phpCommentFormat;
        closeTag("*/", 0);
    }else if(currentBlockState() == 11){
        currentTagFormat = htmlCommentFormat;
        closeTag("-->",10);
    }else if(currentBlockState() == 22){
        currentTagFormat = cssCommentFormat;
        closeTag("*/",20);
    }else if(currentBlockState() == 31){
        currentTagFormat = jsCommentFormat;
        closeTag("*/",30);
    }

    while(parser.hasNext()){
        QRegularExpressionMatch word = parser.next();

        if(word.captured() == "<?"){
            // from now on this is PHP
            phpReturnState = currentBlockState();

            setCurrentBlockState(0);
            int start = word.capturedStart();
            if(parser.hasNext() && parser.peekNext().captured() == "php"){
                word = parser.next();
            }else if(parser.hasNext() && parser.peekNext().captured() == "="){
                word = parser.next();
            }

            setFormat(start, word.capturedEnd()-start, phpTagFormat);
            if(parser.hasNext())
                word = parser.next();
        }else if(word.captured() == "<script type=\"text/javascript\">"){
            setCurrentBlockState(30);
            setFormat(word.capturedStart(), 13, htmlTagFormat);
            setFormat(14, 17, htmlQuoteFormat);
            setFormat(word.capturedEnd()-1, 1, htmlTagFormat);
        }else if(word.captured() == "<style>"){
            setCurrentBlockState(20);
            setFormat(word.capturedStart(), word.capturedLength(), htmlTagFormat);
        }

        // Start highlighting according to coding language
        if(currentBlockState() < 10){
            // *** PHP highlighting ***
            int startExp = 0;

            if(word.captured() == "$" && parser.hasNext()){
               // we found a variable
               startExp = word.capturedStart();

               word = parser.next();
               if(word.captured() == "$" || word.captured() == "_")
                   word = parser.next();
               setFormat(startExp,word.capturedEnd()-startExp, phpVariableFormat);
           }else if(phpKeyWordsRules.contains(word.captured().toLower())){
               setFormat(word.capturedStart(), word.capturedLength(), phpKeyWordsRules[word.captured()]);
           }else if(word.captured() == "/" && parser.hasNext() && parser.peekNext().captured() == "/"){
               setFormat(word.capturedStart(), text.length()-word.capturedStart(), phpCommentFormat);
               return;
           }else if(word.captured() == "?>"){
               setCurrentBlockState(phpReturnState ==-1? mDocType:phpReturnState);
               setFormat(word.capturedStart(), word.capturedLength(), phpTagFormat);
           }else if(word.captured().contains(QRegExp("((?<=\\-)\\d+$|^\\d+$)"))){
               startExp = word.capturedStart();
               if(parser.hasNext() && parser.peekNext().captured() == ".")
                   word = parser.next();
               setFormat(startExp, word.capturedEnd()-startExp, phpNumberFormat);
           }else if(word.captured() == "\""){
               currentTagFormat = phpQuoteFormat;
               setCurrentBlockState(1);
               closeTag("\"", 0, word.capturedStart());
           }else if(word.captured() == "'"){
               currentTagFormat = phpQuoteFormat;
               setCurrentBlockState(2);
               closeTag("'", 0, word.capturedStart());
           }else if(word.captured() == "/*"){
               currentTagFormat = phpCommentFormat;
               setCurrentBlockState(3);
               closeTag("*/", 0, word.capturedStart());
           }
        }else if(currentBlockState() < 20){
            // HTML highlighting
            if(currentBlockState() == 12){
                if(word.captured() == "\""){
                    setCurrentBlockState(10);
                    setFormat(word.capturedStart(), 1, htmlQuoteFormat);
                }else{
                    currentTagFormat = htmlQuoteFormat;
                    closeTag("\"",10, word.capturedStart());
                }
            }else if(word.captured() == "<" || htmlReturnState == 1){
                htmlReturnState = 0;
                int startExp = word.capturedStart();

                if(parser.hasNext() && parser.peekNext().captured().toLower().contains(QRegExp("[^a-z]"))){
                    word = parser.next();
                }

                if(parser.hasNext()){
                    QString htmlTag = parser.peekNext().captured().toLower();

                    if(htmlKeyWordsRules.contains(htmlTag))
                        htmlTagFormat = htmlKeyWordsRules[htmlTag];

                    setFormat(startExp, word.capturedEnd()-startExp, htmlTagFormat);
                }

                bool tagAtEnd = false;
                while(parser.hasNext() && !tagAtEnd){
                    word = parser.next();
                    if(word.captured() == "\""){
                        setCurrentBlockState(12);
                        currentTagFormat = htmlQuoteFormat;
                        closeTag("\"", 10, word.capturedStart());
                        if(currentBlockState() < 10){
                            htmlReturnState = 1;
                            phpReturnState = 12;
                            break;
                        }
                    }else{
                        setFormat(word.capturedStart(), word.capturedLength(), htmlTagFormat);
                        if(word.captured() == ">")
                            tagAtEnd = true;
                    }
                }
            }else if(word.captured() == "<!--"){
                currentTagFormat = htmlCommentFormat;
                setCurrentBlockState(11);
                closeTag("-->",10,word.capturedStart());
            }else if(word.captured() == "</script>"){
                setFormat(word.capturedStart(), word.capturedLength(), htmlTagFormat);
                setCurrentBlockState(10);
            }
        }else if(currentBlockState() < 30){
            // CSS highlighting
            int startExp = 0;

            if(word.captured() == "."){
                // we found a class
                startExp = word.capturedStart();
                word = parser.next();
                setFormat(startExp,word.capturedEnd()-startExp, cssClassFormat);
            }else if(word.captured() == "#"){
                // we found an id
                startExp = word.capturedStart();
                word = parser.next();
                setFormat(startExp,word.capturedEnd()-startExp, cssIdFormat);
            }else if(cssKeyWordsRules.contains(word.captured().toLower())){
                setFormat(word.capturedStart(), word.capturedLength(), cssKeyWordFormat);
            }else if(htmlKeyWordsRules.contains(word.captured().toLower())){
                setFormat(word.capturedStart(), word.capturedLength(), cssTagFormat);
            }else if(word.captured() == ":" && currentBlockState() == 21){
                currentTagFormat = standardTagFormat;
                closeTag(";", 21, word.capturedStart());
            }else if(word.captured() == "/*"){
                currentTagFormat = cssCommentFormat;
                int state = currentBlockState();
                setCurrentBlockState(22);
                closeTag("*/", state, word.capturedStart());
            }else if(word.captured() == "{"){
                setCurrentBlockState(21);
            }else if(word.captured() == "}"){
                setCurrentBlockState(20);
            }else if(word.captured() == "</style>"){
                setFormat(word.capturedStart(), word.capturedEnd(), htmlTagFormat);
                setCurrentBlockState(10);
            }
        }else if(currentBlockState() < 40){
            // JS highlighting
            if(jsKeyWordsRules.contains(word.captured())){
                setFormat(word.capturedStart(), word.capturedLength(), jsKeyWordsRules[word.captured()]);
            }else if(word.captured().contains(QRegExp("^\\d+$"))){
                int startExp = word.capturedStart();
                if(parser.hasNext() && parser.peekNext().captured() == ".")
                    word = parser.next();
                setFormat(startExp, word.capturedEnd()-startExp, jsNumberFormat);
            }else if(word.captured() == "\""){
                currentTagFormat = jsStringFormat;
                closeTag("\"", 30, word.capturedStart());
            }else if(word.captured() == "'"){
                currentTagFormat = jsStringFormat;
                closeTag("'", 30, word.capturedStart());
            }else if(word.captured() == "/*"){
                setCurrentBlockState(31);
                closeTag("*/", 30, word.capturedStart());
            }else if(word.captured() == "/" && parser.hasNext() && parser.peekNext().captured() == "/"){
                setFormat(word.capturedStart(), text.length()-word.capturedStart(), jsCommentFormat);
                return;
            }else if(word.captured() == "</script>"){
                setFormat(word.capturedStart(), word.capturedLength(), htmlTagFormat);
                setCurrentBlockState(10);
            }
        }else{
            qDebug() << "Could not highlight code, no valid block state:" << currentBlockState();
        }
    }

    if(currentBlockState() != currentTextBlockState){
        emit currentBlockStateChanged(currentBlockState());
        currentTextBlockState = currentBlockState();
//        qDebug() << "changed block state to" << currentBlockState() << "with line" << text;
    }
//        qDebug() << "exeting line" << text << "with status" << currentBlockState();
}

void Highlighter::closeTag(QString tag, int returnState, int startPos){
    int length = 0;

    while(parser.hasNext()){
        QRegularExpressionMatch word = parser.next();
        if(word.captured() == "<?"){
            length = word.capturedStart() - startPos;
            phpReturnState = currentBlockState();

            setCurrentBlockState(0);

            int start = word.capturedStart();
            if(parser.hasNext() && parser.peekNext().captured() == "php"){
                word = parser.next();
            }else if(parser.hasNext() && parser.peekNext().captured() == "="){
                word = parser.next();
            }

            setFormat(start, word.capturedEnd(), phpTagFormat);

            break;
        }else if(word.captured() == "?>"){
            setCurrentBlockState(10); // was ist wenn ich nicht zurück zu html sondern zu css oder js will???
            length = word.capturedStart() - startPos;
            break;
        }else if(word.captured() == tag){
            setCurrentBlockState(returnState);
            length = word.capturedEnd() - startPos;
            break;
        }
    }

    if(length == 0){
        // No end was found, multiline continues on next line
        length = currentLine.length();
    }

    setFormat(startPos, length, currentTagFormat);
}

void Highlighter::setDocType(const int docType){
    mDocType = docType;
    setCurrentBlockState(docType);
}
