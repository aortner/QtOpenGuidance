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

#ifndef POSECACHE_H
#define POSECACHE_H

#include <QObject>

#include <QQuaternion>
#include <QVector3D>

#include "GuidanceBase.h"

#include "qneblock.h"
#include "qneport.h"

#include "../kinematic/Tile.h"

class PoseSynchroniser : public GuidanceBase {
    Q_OBJECT

  public:
    explicit PoseSynchroniser( Tile* tile )
      : GuidanceBase() {
      rootTile = tile->getTileForOffset( 0, 0 );
    }

  public slots:
    void setPosition( QVector3D value ) {
      position = value;
      Tile* currentTile = rootTile->getTileForPosition( &position );
      emit poseChanged( currentTile, position, orientation );
    }

    void setTiledPosition( Tile* tile, QVector3D position ) {
      this->position = position;
      emit poseChanged( tile, this->position, orientation );
    }

    void setOrientation( QQuaternion value ) {
      orientation = value;
    }

    void setSteeringAngle( float value ) {
      steeringAngle = value;
      emit steeringAngleChanged( steeringAngle );
    }

  signals:
    void poseChanged( Tile* tile, QVector3D position, QQuaternion orientation );
    void steeringAngleChanged( float );

  public:
    virtual void emitConfigSignals() override {
      rootTile = rootTile->getTileForPosition( &position );
      emit poseChanged( rootTile, position, orientation );
      emit steeringAngleChanged( steeringAngle );
    }

  public:
    float steeringAngle = 0;
    Tile* rootTile = nullptr;
    QVector3D position = QVector3D();
    QQuaternion orientation = QQuaternion();
};

class PoseSynchroniserFactory : public GuidanceFactory {
    Q_OBJECT

  public:
    PoseSynchroniserFactory( Tile* tile )
      : GuidanceFactory(),
        tile( tile ) {}

    QString getNameOfFactory() override {
      return QStringLiteral( "Pose Synchroniser" );
    }

    virtual void addToCombobox( QComboBox* combobox ) override {
      combobox->addItem( getNameOfFactory(), QVariant::fromValue( this ) );
    }

    virtual GuidanceBase* createNewObject() override {
      return new PoseSynchroniser( tile );
    }

    virtual QNEBlock* createBlock( QGraphicsScene* scene, QObject* obj ) override {
      QNEBlock* b = new QNEBlock( obj );
      scene->addItem( b );

      b->addPort( getNameOfFactory(), QStringLiteral( "" ), 0, QNEPort::NamePort );
      b->addPort( getNameOfFactory(), QStringLiteral( "" ), 0, QNEPort::TypePort );

      b->addInputPort( "Tiled Position", SLOT( setTiledPosition( Tile*, QVector3D ) ) );
      b->addInputPort( "Position", SLOT( setPosition( QVector3D ) ) );
      b->addInputPort( "Orientation", SLOT( setOrientation( QQuaternion ) ) );
      b->addInputPort( "Steering Angle", SLOT( setSteeringAngle( float ) ) );

      b->addOutputPort( "Pose", SIGNAL( poseChanged( Tile*, QVector3D, QQuaternion ) ) );
      b->addOutputPort( "Steering Angle", SIGNAL( steeringAngleChanged( float ) ) );

      return b;
    }

  private:
    Tile* tile;
};

#endif // POSECACHE_H

