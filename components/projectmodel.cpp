#include "projectmodel.h"

ProjectModel::ProjectModel(QObject *parent) : QAbstractTableModel(parent) {
    mHeaders.append("Projects");
}

bool ProjectModel::add(QString name, QString url){
    QStringList newProject;
    newProject << name << url;
    projects->insert(0, newProject);
    return true;
}

QVariant ProjectModel::data(const QModelIndex &index, int role) const{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        return projects[index.row()][index.column()];
    }

    return QVariant();
}

QVariant ProjectModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        return mHeaders.value(section);
    }else{
        return QVariant();
    }
}

int ProjectModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return projects->count();
}

int ProjectModel::columnCount(const QModelIndex &parent) const {
    return projects[parent.row()].count();
}

ProjectModel::~ProjectModel(){

}
