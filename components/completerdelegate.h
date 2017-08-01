#ifndef COMPLETERDELEGATE_H
#define COMPLETERDELEGATE_H

#include <QObject>
#include <QApplication>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QStyleOption>
#include <QToolTip>

#include <QDebug>

class CompleterDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    CompleterDelegate();

private:
    QPoint cursor;
    QPoint cursorOffset;

protected:
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;

public slots:
    void cursorPosition(QPoint pos);
};

#endif // COMPLETERDELEGATE_H
