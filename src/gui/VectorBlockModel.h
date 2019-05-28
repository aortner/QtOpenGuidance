#ifndef VECTORMODEL_H
#define VECTORMODEL_H

#include <QComboBox>

#include <QtCore/QAbstractTableModel>

#include <QList>

#include "vectorobject.h"

class VectorBlockModel : public QAbstractTableModel {
    Q_OBJECT

  public:
    explicit VectorBlockModel( QGraphicsScene* scene );

    // Header:
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    bool setHeaderData( int section, Qt::Orientation orientation, const QVariant& value, int role = Qt::EditRole ) override;

    // Basic functionality:
    int rowCount( const QModelIndex& parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex& parent = QModelIndex() ) const override;

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;

    // Editable:
    bool setData( const QModelIndex& index, const QVariant& value,
                  int role = Qt::EditRole ) override;

    Qt::ItemFlags flags( const QModelIndex& index ) const override;


  public:
    void addToCombobox( QComboBox* combobox );

  public slots:
    void resetModel();

  private:
    QGraphicsScene* scene;
};

#endif // VECTORMODEL_H