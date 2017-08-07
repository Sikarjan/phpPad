#include "completerdelegate.h"

CompleterDelegate::CompleterDelegate()
{
    cursor = QPoint(0, 0);
    cursorOffset = QPoint(20, -2);
}

void CompleterDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem optionV4 = option;
    initStyleOption(&optionV4, index);
    QStyleOptionViewItem source = option;
    initStyleOption(&source, index.sibling(index.row(),1));

    QStyle *style = optionV4.widget? optionV4.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml("<table><tr><td width='130'>"+optionV4.text+"</td><td>"+source.text+"</td></tr></table>");

    // Painting item without text
    optionV4.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;
    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4);
    painter->save();
    painter->translate(textRect.topLeft());
    painter->setClipRect(textRect.translated(-textRect.topLeft()));

    // Show tool tip if highlighted
    if (optionV4.state & QStyle::State_Selected){
        QStyleOptionViewItem overload = option;
        initStyleOption(&overload, index.sibling(index.row(),2));
        QPoint pos = textRect.topRight()+cursor+cursorOffset;
        QToolTip::showText(pos, overload.text);
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
    doc.setHtml("<table><tr><td width='80'>"+optionV4.text+"</td><td>"+source.text+"</td></tr></table>");
    doc.setTextWidth(optionV4.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}

void CompleterDelegate::cursorPosition(QPoint pos){
    cursor = pos;
}
