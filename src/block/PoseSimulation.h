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

#ifndef POSESIMULATION_H
#define POSESIMULATION_H

#include <QObject>

#include <QTime>
#include <QEvent>
#include <QBasicTimer>
#include <QQuaternion>
#include <QVector3D>

#include <QtGlobal>
#include <QtDebug>

#include "GuidanceBase.h"

#include "../kinematic/Tile.h"

#include <GeographicLib/TransverseMercator.hpp>
using namespace std;
using namespace GeographicLib;

class PoseSimulation : public GuidanceBase {
    Q_OBJECT

  public:
    explicit PoseSimulation( Tile* tile )
      : GuidanceBase(),
        _tm( Constants::WGS84_a(), Constants::WGS84_f(), Constants::UTM_k0() ),
        tile( tile ) {
      setSimulation( false );
    }

  public slots:
    void setInterval( int interval ) {
      m_interval = interval;

      setSimulation( m_enabled );

      emit intervalChanged( m_interval );
    }

    void setFrequency( int frequency ) {
      setInterval( 1000 / frequency );
    }

    void setSimulation( bool enabled ) {
      m_enabled = enabled;

      if( enabled ) {
        m_timer.start( m_interval, Qt::PreciseTimer, this );
        m_time.start();
      } else {
        m_timer.stop();
      }

      emit simulationChanged( m_enabled );
    }

    void setSteerAngle( float steerAngle ) {
      m_steerAngle = steerAngle;
      emit steerAngleChanged( m_steerAngle );
    }

    void setVelocity( float velocity ) {
      m_velocity = velocity;
    }

    void setWheelbase( float wheelbase ) {
      if( !qFuzzyIsNull( wheelbase ) ) {
        m_wheelbase = wheelbase;
      }
    }

    void setAntennaPosition( QVector3D position ) {
      m_antennaPosition = position;
    }

    void setInitialWGS84Position( QVector3D position ) {
      m_initialWGS84Position = position;
    }

  protected:
    void timerEvent( QTimerEvent* event ) override;

  signals:
    void simulationChanged( bool enabled );
    void intervalChanged( int interval );

    void steerAngleChanged( float steerAngle );

    void antennaPositionChanged( QVector3D position );

    void steeringAngleChanged( float );
    void positionChanged( QVector3D position );
    void globalPositionChanged( double, double, double );
    void orientationChanged( QQuaternion orientation );

  public:
    virtual void emitConfigSignals() override {
      emit steerAngleChanged( m_steerAngle );
      emit positionChanged( QVector3D( float( x ), float( y ), float( height ) ) );
      emit orientationChanged( m_orientation );
    }

  private:
    bool m_enabled = false;
    int m_interval = 50;

    QBasicTimer m_timer;
    int m_timerId;
    QTime m_time;

    float m_steerAngle = 0;
    float m_velocity = 0;
    float m_wheelbase = 2.4f;

    QVector3D m_antennaPosition = QVector3D();
    QQuaternion m_orientation = QQuaternion();

    QVector3D m_initialWGS84Position = QVector3D();

    double x = 0;
    double y = 0;
    double height = 0;
    TransverseMercator _tm;

    Tile* tile;
};

class PoseSimulationFactory : public GuidanceFactory {
    Q_OBJECT

  public:
    PoseSimulationFactory( Tile* tile )
      : GuidanceFactory(),
        tile( tile ) {}

    QString getNameOfFactory() override {
      return QStringLiteral( "Pose Simulation" );
    }

    virtual void addToCombobox( QComboBox* ) override {
    }

    virtual GuidanceBase* createNewObject() override {
      return new PoseSimulation( tile );
    }

    virtual QNEBlock* createBlock( QGraphicsScene* scene, QObject* obj ) override {
      QNEBlock* b = new QNEBlock( obj, true );
      scene->addItem( b );

      b->addPort( getNameOfFactory(), QStringLiteral( "" ), 0, QNEPort::NamePort );
      b->addPort( getNameOfFactory(), QStringLiteral( "" ), 0, QNEPort::TypePort );

      b->addInputPort( "Antenna Position", SLOT( setAntennaPosition( QVector3D ) ) );
      b->addInputPort( "Length Wheelbase", SLOT( setWheelbase( float ) ) );
      b->addInputPort( "Initial WGS84 Position", SLOT( setInitialWGS84Position( QVector3D ) ) );

      b->addOutputPort( "WGS84 Position", SIGNAL( globalPositionChanged( double, double, double ) ) );
      b->addOutputPort( "Position", SIGNAL( positionChanged( QVector3D ) ) );
      b->addOutputPort( "Orientation", SIGNAL( orientationChanged( QQuaternion ) ) );
      b->addOutputPort( "Steering Angle", SIGNAL( steeringAngleChanged( float ) ) );

      return b;
    }

  private:
    Tile* tile;
};

#endif // POSESIMULATION_H

