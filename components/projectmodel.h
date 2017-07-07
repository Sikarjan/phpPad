#ifndef PROJECTMODEL_H
#define PROJECTMODEL_H

#include <QAbstractTableModel>
#include <QVector>

class ProjectModel : public QAbstractTableModel {
    Q_OBJECT

public:
    ProjectModel(QObject *parent = 0);
    ~ProjectModel();

    bool add(QString name, QString url);
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVector<QStringList> *projects; // List: 0 => Project name, 1 => Root URL
private:

    QStringList mHeaders;
};

#endif // PROJECTMODEL_H
