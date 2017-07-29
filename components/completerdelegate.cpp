#include "completerdelegate.h"

CompleterDelegate::CompleterDelegate()
{

}

void CompleterDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem optionV4 = option;
    initStyleOption(&optionV4, index);
    QStyleOptionViewItem source = option;
    initStyleOption(&source, index.sibling(index.row(),1));

    QStyle *style = optionV4.widget? optionV4.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml(optionV4.text+" <b>"+source.text+"</b>");

    // Painting item without text
    optionV4.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;
    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4);
    qDebug() << textRect;
    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));

    // Highlighting text if item is selected
    if (optionV4.state & QStyle::State_Selected){
        ctx.palette.setColor(QPalette::Text, optionV4.palette.color(QPalette::Active, QPalette::HighlightedText));
        QStyleOptionViewItem overload = option;
        initStyleOption(&overload, index.sibling(index.row(),2));
        QToolTip::showText(QPoint(textRect.right() + 15, textRect.top()), overload.text);
    }

    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

QSize CompleterDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem optionV4 = option;
    initStyleOption(&optionV4, index);
    QStyleOptionViewItem source = option;
    initStyleOption(&source, index.sibling(index.row(),1));

    QTextDocument doc;
    doc.setHtml(optionV4.text+" <b>"+source.text+"</b>");
    doc.setTextWidth(optionV4.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}
