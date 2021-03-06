// Copyright( C ) 2019 Christian Riggenbach
//
// This program is free software:
// you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// ( at your option ) any later version.
//
// This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY;
// without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see < https : //www.gnu.org/licenses/>.

#ifndef TRAILERENTITY_H
#define TRAILERENTITY_H

#include <QObject>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>

#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QGeometryRenderer>

#include <Qt3DExtras/QTorusMesh>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QPhongMaterial>

#include "GuidanceBase.h"

#include "../kinematic/Tile.h"

class TrailerModel : public GuidanceBase {
    Q_OBJECT

  public:
    explicit TrailerModel( Qt3DCore::QEntity* rootEntity );
    ~TrailerModel();

  public slots:
    void setPoseHookPoint( Tile*, QVector3D, QQuaternion );
    void setPoseTowPoint( Tile*, QVector3D, QQuaternion );
    void setPosePivotPoint( Tile*, QVector3D, QQuaternion );

    void setOffsetHookPointPosition( QVector3D position );
    void setWheelbase( float wheelbase );

  private:
    Qt3DCore::QEntity* m_rootEntity;

    Qt3DCore::QEntity* m_hitchEntity;
    Qt3DCore::QEntity* m_axleEntity;
    Qt3DCore::QEntity* m_wheelLeftEntity;
    Qt3DCore::QEntity* m_wheelRightEntity;

    Qt3DExtras::QCylinderMesh* m_hitchMesh;
    Qt3DExtras::QCylinderMesh* m_axleMesh;
    Qt3DExtras::QCylinderMesh* m_wheelMesh;

    Qt3DCore::QTransform* m_rootEntityTransform;
    Qt3DCore::QTransform* m_hitchTransform;
    Qt3DCore::QTransform* m_axleTransform;
    Qt3DCore::QTransform* m_wheelLeftTransform;
    Qt3DCore::QTransform* m_wheelRightTransform;


    Qt3DCore::QEntity* m_towHookEntity;
    Qt3DCore::QEntity* m_pivotPointEntity;
    Qt3DCore::QEntity* m_towPointEntity;

    Qt3DExtras::QSphereMesh*  m_towHookMesh;
    Qt3DExtras::QSphereMesh* m_pivotPointMesh;
    Qt3DExtras::QSphereMesh* m_towPointMesh;

    Qt3DCore::QTransform* m_towHookTransform;
    Qt3DCore::QTransform* m_pivotPointTransform;
    Qt3DCore::QTransform* m_towPointTransform;

    QVector3D m_offsetHookPoint = QVector3D( 6, 0, 0 );
    float m_wheelbase = 2.4f;

    void setProportions();
};

class TrailerModelFactory : public GuidanceFactory {
    Q_OBJECT

  public:
    TrailerModelFactory( Qt3DCore::QEntity* rootEntity )
      : GuidanceFactory(),
        rootEntity( rootEntity ) {}

    QString getNameOfFactory() override {
      return QStringLiteral( "Trailer Model" );
    }

    virtual void addToCombobox( QComboBox* combobox ) override {
      combobox->addItem( getNameOfFactory(), QVariant::fromValue( this ) );
    }

    virtual GuidanceBase* createNewObject() override {
      return new TrailerModel( rootEntity );
    }

    virtual QNEBlock* createBlock( QGraphicsScene* scene, QObject* obj ) override {
      QNEBlock* b = new QNEBlock( obj );
      scene->addItem( b );

      b->addPort( getNameOfFactory(), QStringLiteral( "" ), 0, QNEPort::NamePort );
      b->addPort( getNameOfFactory(), QStringLiteral( "" ), 0, QNEPort::TypePort );

      b->addInputPort( "Length Wheelbase", SLOT( setWheelbase( float ) ) );
      b->addInputPort( "Offset Hook Point", SLOT( setOffsetHookPointPosition( QVector3D ) ) );
      b->addInputPort( "Pose Hook Point", SLOT( setPoseHookPoint( Tile*, QVector3D, QQuaternion ) ) );
      b->addInputPort( "Pose Pivot Point", SLOT( setPosePivotPoint( Tile*, QVector3D, QQuaternion ) ) );
      b->addInputPort( "Pose Tow Point", SLOT( setPoseTowPoint( Tile*, QVector3D, QQuaternion ) ) );

      return b;
    }

  private:
    Qt3DCore::QEntity* rootEntity;
};

#endif // TRAILERENTITY_H

