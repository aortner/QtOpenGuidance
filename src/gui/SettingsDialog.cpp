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

#include <QMap>

#include <QtWidgets>
#include <QObject>

#include <QFileDialog>

#include <QDebug>

#include "qneblock.h"
#include "qneconnection.h"
#include "qneport.h"
#include "qnodeseditor.h"

#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

#include "../block/VectorObject.h"
#include "../block/NumberObject.h"
#include "../block/StringObject.h"

#include "../block/CameraController.h"
#include "../block/TractorModel.h"
#include "../block/TrailerModel.h"
#include "../block/GridModel.h"

#include "../block/PoseSimulation.h"

#include "../block/PoseSynchroniser.h"

#include "../block/NmeaParser.h"
#include "../block/TransverseMercatorConverter.h"

#include "../block/DebugSink.h"
#include "../block/PrintLatency.h"

#include "../block/UdpSocket.h"
#include "../block/SerialPort.h"
#include "../block/FileStream.h"


#include "../kinematic/FixedKinematic.h"
#include "../kinematic/TrailerKinematic.h"

#include "../kinematic/Tile.h"

SettingsDialog::SettingsDialog( Qt3DCore::QEntity* rootEntity, QWidget* parent ) :
  QDialog( parent ),
  ui( new Ui::SettingsDialog ),
  rootEntity( rootEntity ) {
  ui->setupUi( this );

  // load states of checkboxes from global config
  {
    QSettings settings( QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) + "/config.ini",
                        QSettings::IniFormat );

    ui->cbSaveConfigOnExit->setCheckState( settings.value( "SaveConfigOnExit", false ).toBool() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
    ui->cbLoadConfigOnStart->setCheckState( settings.value( "LoadConfigOnStart", false ).toBool() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
    ui->cbOpenSettingsDialogOnStart->setCheckState( settings.value( "OpenSettingsDialogOnStart", false ).toBool() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
    ui->cbShowCameraToolbarOnStart->setCheckState( settings.value( "ShowCameraToolbarOnStart", false ).toBool() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
    ui->cbRunSimulatorOnStart->setCheckState( settings.value( "RunSimulatorOnStart", false ).toBool() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );

    ui->gbGrid->setChecked( settings.value( "Grid/Enabled", true ).toBool() );
    ui->dsbGridXStep->setValue( settings.value( "Grid/XStep", 10 ).toDouble() );
    ui->dsbGridYStep->setValue( settings.value( "Grid/YStep", 10 ).toDouble() );
    ui->dsbGridSize->setValue( settings.value( "Grid/Size", 10 ).toDouble() );
    gridColor = settings.value( "Grid/Color", QColor( 0xa2, 0xe3, 0xff ) ).value<QColor>();
  }

  ui->gvNodeEditor->setDragMode( QGraphicsView::RubberBandDrag );

  QGraphicsScene* scene = new QGraphicsScene();
  ui->gvNodeEditor->setScene( scene );

  ui->gvNodeEditor->setRenderHint( QPainter::Antialiasing, true );

  QNodesEditor* nodesEditor = new QNodesEditor( this );
  nodesEditor->install( scene );

  // Models for the tableview
  filterModel = new QSortFilterProxyModel( scene );
  vectorBlockModel = new VectorBlockModel( scene );
  numberBlockModel = new NumberBlockModel( scene );
  stringBlockModel = new StringBlockModel( scene );

  vectorBlockModel->addToCombobox( ui->cbValues );
  numberBlockModel->addToCombobox( ui->cbValues );
  stringBlockModel->addToCombobox( ui->cbValues );
  filterModel->setSourceModel( vectorBlockModel );
  filterModel->sort( 0, Qt::AscendingOrder );
  ui->twValues->setModel( filterModel );

  // IMPORTANT: the order of the systemblocks should not change, as they get their id in order of creation
  // DON'T BREAK SAVED CONFIGS!

  // initialise tiling
  Tile* tile = new Tile( &tileRoot, 0, 0, rootEntity );

  // initialise the wrapper for the Transverse Mercator conversion, so all offsets are the same application-wide
  TransverseMercatorWrapper* tmw = new TransverseMercatorWrapper();

  // simulator
  poseSimulationFactory = new PoseSimulationFactory( tile );
  poseSimulation = poseSimulationFactory->createNewObject();
  poseSimulationFactory->createBlock( ui->gvNodeEditor->scene(), poseSimulation );

  // grid
  gridModelFactory = new GridModelFactory( rootEntity );
  gridModel = gridModelFactory->createNewObject();
  gridModelFactory->createBlock( ui->gvNodeEditor->scene(), gridModel );

  QObject::connect( this, SIGNAL( setGrid( bool ) ),
                    gridModel, SLOT( setGrid( bool ) ) );
  QObject::connect( this, SIGNAL( setGridValues( float, float, float, QColor ) ),
                    gridModel, SLOT( setGridValues( float, float, float, QColor ) ) );


  // Factories for the blocks
  transverseMercatorConverterFactory = new TransverseMercatorConverterFactory( tile, tmw );
  poseCacheFactory = new PoseSynchroniserFactory( tile );
  tractorModelFactory = new TractorModelFactory( rootEntity );
  trailerModelFactory = new TrailerModelFactory( rootEntity );
  fixedKinematicFactory = new FixedKinematicFactory;
  trailerKinematicFactory = new TrailerKinematicFactory( tile );
  vectorFactory = new VectorFactory();
  numberFactory = new NumberFactory();
  stringFactory = new StringFactory();
  debugSinkFactory = new DebugSinkFactory();
  printLatencyFactory = new PrintLatencyFactory();
  udpSocketFactory = new UdpSocketFactory();
  serialPortFactory = new SerialPortFactory();
  fileStreamFactory = new FileStreamFactory();
  nmeaParserFactory = new NmeaParserFactory();

//  fieldFactory = new FieldFactory( rootEntity );

  transverseMercatorConverterFactory->addToCombobox( ui->cbNodeType );
  poseCacheFactory->addToCombobox( ui->cbNodeType );
  tractorModelFactory->addToCombobox( ui->cbNodeType );
  trailerModelFactory->addToCombobox( ui->cbNodeType );
  fixedKinematicFactory->addToCombobox( ui->cbNodeType );
  trailerKinematicFactory->addToCombobox( ui->cbNodeType );
  vectorFactory->addToCombobox( ui->cbNodeType );
  numberFactory->addToCombobox( ui->cbNodeType );
  stringFactory->addToCombobox( ui->cbNodeType );
  debugSinkFactory->addToCombobox( ui->cbNodeType );
  printLatencyFactory->addToCombobox( ui->cbNodeType );
  udpSocketFactory->addToCombobox( ui->cbNodeType );
  serialPortFactory->addToCombobox( ui->cbNodeType );
  fileStreamFactory->addToCombobox( ui->cbNodeType );
  nmeaParserFactory->addToCombobox( ui->cbNodeType );

//  fieldFactory->addToCombobox( ui->cbNodeType );

  // grid color picker
  ui->lbColor->setText( gridColor.name() );
  ui->lbColor->setPalette( QPalette( gridColor ) );
  ui->lbColor->setAutoFillBackground( true );
}

SettingsDialog::~SettingsDialog() {
  delete ui;

  transverseMercatorConverterFactory->deleteLater();
  poseCacheFactory->deleteLater();
  tractorModelFactory->deleteLater();
  trailerModelFactory->deleteLater();
  fixedKinematicFactory->deleteLater();
  trailerKinematicFactory->deleteLater();
  vectorFactory->deleteLater();
  numberFactory->deleteLater();
  stringFactory->deleteLater();
  debugSinkFactory->deleteLater();
  printLatencyFactory->deleteLater();
  udpSocketFactory->deleteLater();
  serialPortFactory->deleteLater();
  fileStreamFactory->deleteLater();
  nmeaParserFactory->deleteLater();

//  fieldFactory->deleteLater();

  vectorBlockModel->deleteLater();
  numberBlockModel->deleteLater();

  poseSimulationFactory->deleteLater();
  gridModelFactory->deleteLater();

  poseSimulation->deleteLater();
  gridModel->deleteLater();
}

QGraphicsScene* SettingsDialog::getSceneOfConfigGraphicsView() {
  return ui->gvNodeEditor->scene();
}

void SettingsDialog::toggleVisibility() {
  setVisible( ! isVisible() );
}

void SettingsDialog::loadConfigOnStart() {
  // save the current config if enabled
  if( ui->cbLoadConfigOnStart->isChecked() ) {
    loadDefaultConfig();
  }
}

void SettingsDialog::saveConfigOnExit() {
  // save the current config if enabled
  if( ui->cbSaveConfigOnExit->isChecked() ) {
    saveDefaultConfig();
  }
}

void SettingsDialog::loadDefaultConfig() {
  QFile saveFile( QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) + "/default.json" );

  if( saveFile.open( QIODevice::ReadOnly ) ) {
    loadConfigFromFile( saveFile );
  }
}

void SettingsDialog::saveDefaultConfig() {
  QFile saveFile( QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) + "/default.json" );

  if( saveFile.open( QIODevice::WriteOnly ) ) {

    // select all items, so everything gets saved
    foreach( QGraphicsItem* item, ui->gvNodeEditor->scene()->items() ) {
      item->setSelected( true );
    }

    saveConfigToFile( saveFile );

    // deselect all items
    foreach( QGraphicsItem* item, ui->gvNodeEditor->scene()->items() ) {
      item->setSelected( false );
    }
  }
}

void SettingsDialog::on_cbValues_currentIndexChanged( int /*index*/ ) {
  QAbstractTableModel* model = qobject_cast<QAbstractTableModel*>( qvariant_cast<QAbstractTableModel*>( ui->cbValues->currentData() ) );

  if( model ) {
    filterModel->setSourceModel( model );
    ui->twValues->resizeColumnsToContents();
  }

  ui->pbSetStringToFilename->setEnabled( ui->cbValues->currentText() == "String" );
}

void SettingsDialog::on_pbSaveSelected_clicked() {
  QString selectedFilter = QStringLiteral( "JSON Files (*.json)" );
  QString dir;
  QString fileName = QFileDialog::getSaveFileName( this,
                     tr( "Open Saved Config" ),
                     dir,
                     tr( "All Files (*);;JSON Files (*.json)" ),
                     &selectedFilter );

  if( !fileName.isEmpty() ) {

    QFile saveFile( fileName );

    if( !saveFile.open( QIODevice::WriteOnly ) ) {
      qWarning( "Couldn't open save file." );
      return;
    }

    saveConfigToFile( saveFile );
  }
}

void SettingsDialog::saveConfigToFile( QFile& file ) {
  QJsonObject jsonObject;
  QJsonArray blocks;
  QJsonArray connections;
  jsonObject["blocks"] = blocks;
  jsonObject["connections"] = connections;

  foreach( QGraphicsItem* item, ui->gvNodeEditor->scene()->selectedItems() ) {
    {
      QNEBlock* block = qgraphicsitem_cast<QNEBlock*>( item );

      if( block ) {
        block->toJSON( jsonObject );
      }
    }

    {
      QNEConnection* connection = qgraphicsitem_cast<QNEConnection*>( item );

      if( connection ) {
        connection->toJSON( jsonObject );
      }
    }
  }

  QJsonDocument jsonDocument( jsonObject );
  file.write( jsonDocument.toJson() );
}

QNEBlock* SettingsDialog::getBlockWithId( int id ) {
  foreach( QGraphicsItem* item, ui->gvNodeEditor->scene()->items() ) {
    QNEBlock* block = qgraphicsitem_cast<QNEBlock*>( item );

    if( block ) {
      if( block->id == id ) {
        return block;
      }
    }
  }

  return nullptr;
}

void SettingsDialog::on_pbLoad_clicked() {
  QString selectedFilter = QStringLiteral( "JSON Files (*.json)" );
  QString dir;
  QString fileName = QFileDialog::getOpenFileName( this,
                     tr( "Open Saved Config" ),
                     dir,
                     tr( "All Files (*);;JSON Files (*.json)" ),
                     &selectedFilter );

  if( !fileName.isEmpty() ) {
    QFile loadFile( fileName );


    if( !loadFile.open( QIODevice::ReadOnly ) ) {
      qWarning( "Couldn't open save file." );
      return;
    }

    loadConfigFromFile( loadFile );
  }
}

void SettingsDialog::loadConfigFromFile( QFile& file ) {
  QByteArray saveData = file.readAll();

  QJsonDocument loadDoc( QJsonDocument::fromJson( saveData ) );
  QJsonObject json = loadDoc.object();

  // as the new object get new id, here is a QMap to hold the conversions
  // first int: id in file, second int: id in the graphicsview
  QMap<int, int> idMap;

  if( json.contains( "blocks" ) && json["blocks"].isArray() ) {
    QJsonArray blocksArray = json["blocks"].toArray();

    for( int blockIndex = 0; blockIndex < blocksArray.size(); ++blockIndex ) {
      QJsonObject blockObject = blocksArray[blockIndex].toObject();
      int id = blockObject["id"].toInt( 0 );

      // if id is a system-id -> search the block and set the values
      if( id != 0 ) {
        // system id -> don't create new blocks
        if( id < int( QNEBlock::IdRange::UserIdStart ) ) {
          QNEBlock* block = getBlockWithId( id );

          if( block ) {
            idMap.insert( id, block->id );
            block->setX( blockObject["positionX"].toDouble( 0 ) );
            block->setY( blockObject["positionY"].toDouble( 0 ) );
          }

          // id is not a system-id -> create new blocks
        } else {
          int index = ui->cbNodeType->findText( blockObject["type"].toString(), Qt::MatchExactly );
          GuidanceFactory* factory = qobject_cast<GuidanceFactory*>( qvariant_cast<QObject*>( ui->cbNodeType->itemData( index ) ) );

          if( factory ) {
            GuidanceBase* obj = factory->createNewObject();
            QNEBlock* block = factory->createBlock( ui->gvNodeEditor->scene(), obj );

            idMap.insert( id, block->id );

            block->setX( blockObject["positionX"].toDouble( 0 ) );
            block->setY( blockObject["positionY"].toDouble( 0 ) );
            block->setName( blockObject["name"].toString( factory->getNameOfFactory() ) );
            block->fromJSON( blockObject );
            block->setSelected( true );
          }
        }
      }
    }
  }

  if( json.contains( "connections" ) && json["connections"].isArray() ) {
    QJsonArray connectionsArray = json["connections"].toArray();

    for( int connectionsIndex = 0; connectionsIndex < connectionsArray.size(); ++connectionsIndex ) {
      QJsonObject connectionsObject = connectionsArray[connectionsIndex].toObject();

      if( !connectionsObject["idFrom"].isUndefined() &&
          !connectionsObject["idTo"].isUndefined() &&
          !connectionsObject["portFrom"].isUndefined() &&
          !connectionsObject["portTo"].isUndefined() ) {

        int idFrom = idMap[connectionsObject["idFrom"].toInt()];
        int idTo = idMap[connectionsObject["idTo"].toInt()];

        if( idFrom != 0 && idTo != 0 ) {
          QNEBlock* blockFrom = getBlockWithId( idFrom );
          QNEBlock* blockTo = getBlockWithId( idTo );

          if( blockFrom && blockTo ) {
            QString portFromName = connectionsObject["portFrom"].toString();
            QString portToName = connectionsObject["portTo"].toString();

            QNEPort* portFrom = blockFrom->getPortWithName( portFromName, true );
            QNEPort* portTo = blockTo->getPortWithName( portToName, false );

            if( portFrom && portTo ) {
              QNEConnection* conn = new QNEConnection();
              conn->setPort1( portFrom );

              if( conn->setPort2( portTo ) ) {
                blockFrom->scene()->addItem( conn );
                conn->updatePosFromPorts();
                conn->updatePath();
                conn->setSelected( true );
              } else {
                delete conn;
              }
            }
          }
        }
      }
    }
  }

  // as new values for the blocks are added above, emit all signals now, when the connections are made
  foreach( QGraphicsItem* item, ui->gvNodeEditor->scene()->items() ) {
    QNEBlock* block = qgraphicsitem_cast<QNEBlock*>( item );

    if( block ) {
      block->emitConfigSignals();
    }
  }

  // reset the models
  vectorBlockModel->resetModel();
  numberBlockModel->resetModel();
  stringBlockModel->resetModel();

  // rescale the tableview
  ui->twValues->resizeColumnsToContents();
}

void SettingsDialog::on_pbAddBlock_clicked() {
  QString currentText = ui->cbNodeType->currentText();
  GuidanceFactory* factory = qobject_cast<GuidanceFactory*>( qvariant_cast<GuidanceFactory*>( ui->cbNodeType->currentData() ) );

  if( factory ) {
    GuidanceBase* obj = factory->createNewObject();
    factory->createBlock( ui->gvNodeEditor->scene(), obj );

    // reset the models
    if( qobject_cast<VectorObject*>( obj ) ) {
      vectorBlockModel->resetModel();
    }

    if( qobject_cast<NumberObject*>( obj ) ) {
      numberBlockModel->resetModel();
    }

    if( qobject_cast<StringObject*>( obj ) ) {
      stringBlockModel->resetModel();
    }
  }
}

void SettingsDialog::on_pbZoomOut_clicked() {
  ui->gvNodeEditor->zoomOut();
}

void SettingsDialog::on_pbZoomIn_clicked() {
  ui->gvNodeEditor->zoomIn();
}

void SettingsDialog::on_pbDeleteSelected_clicked() {
  foreach( QGraphicsItem* item, ui->gvNodeEditor->scene()->selectedItems() ) {
    QNEConnection* connection = qgraphicsitem_cast<QNEConnection*>( item );

    if( connection != nullptr ) {
      delete connection;
    }
  }

  foreach( QGraphicsItem* item, ui->gvNodeEditor->scene()->selectedItems() ) {
    QNEBlock* block = qgraphicsitem_cast<QNEBlock*>( item );

    if( block != nullptr ) {
      if( !block->systemBlock ) {
        delete block;
      }
    }
  }
}

void SettingsDialog::on_gbGrid_toggled( bool arg1 ) {
  saveGridValuesInSettings();
  emit setGrid( arg1 );
}

void SettingsDialog::on_dsbGridXStep_valueChanged( double /*arg1*/ ) {
  saveGridValuesInSettings();
  emit setGridValues( float( ui->dsbGridXStep->value() ), float( ui->dsbGridYStep->value() ), float( ui->dsbGridSize->value() ), gridColor );
}

void SettingsDialog::on_dsbGridYStep_valueChanged( double /*arg1*/ ) {
  saveGridValuesInSettings();
  emit setGridValues( float( ui->dsbGridXStep->value() ), float( ui->dsbGridYStep->value() ), float( ui->dsbGridSize->value() ), gridColor );
}

void SettingsDialog::on_dsbGridSize_valueChanged( double /*arg1*/ ) {
  saveGridValuesInSettings();
  emit setGridValues( float( ui->dsbGridXStep->value() ), float( ui->dsbGridYStep->value() ), float( ui->dsbGridSize->value() ), gridColor );
}

void SettingsDialog::on_pbColor_clicked() {
  const QColor color = QColorDialog::getColor( gridColor, this, "Select Grid Color" );

  if( color.isValid() ) {
    gridColor = color;
    ui->lbColor->setText( gridColor.name() );
    ui->lbColor->setPalette( QPalette( gridColor ) );
    ui->lbColor->setAutoFillBackground( true );

    saveGridValuesInSettings();
    emit setGridValues( float( ui->dsbGridXStep->value() ), float( ui->dsbGridYStep->value() ), float( ui->dsbGridSize->value() ), gridColor );
  }
}

void SettingsDialog::saveGridValuesInSettings() {

  QSettings settings( QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) + "/config.ini",
                      QSettings::IniFormat );

  settings.setValue( "Grid/Enabled", bool( ui->gbGrid->isChecked() ) );
  settings.setValue( "Grid/XStep", ui->dsbGridXStep->value() );
  settings.setValue( "Grid/YStep", ui->dsbGridYStep->value() );
  settings.setValue( "Grid/Size", ui->dsbGridSize->value() );
  settings.setValue( "Grid/Color", gridColor );
  settings.sync();
}

void SettingsDialog::emitAllConfigSignals() {
  emit setGridValues( float( ui->dsbGridXStep->value() ), float( ui->dsbGridYStep->value() ), float( ui->dsbGridSize->value() ), gridColor );
}

QComboBox* SettingsDialog::getCbNodeType() {
  return ui->cbNodeType;
}

void SettingsDialog::on_cbSaveConfigOnExit_stateChanged( int arg1 ) {
  QSettings settings( QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) + "/config.ini",
                      QSettings::IniFormat );

  settings.setValue( "SaveConfigOnExit", bool( arg1 == Qt::CheckState::Checked ) );
  settings.sync();
}

void SettingsDialog::on_cbLoadConfigOnStart_stateChanged( int arg1 ) {
  QSettings settings( QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) + "/config.ini",
                      QSettings::IniFormat );

  settings.setValue( "LoadConfigOnStart", bool( arg1 == Qt::CheckState::Checked ) );
  settings.sync();
}

void SettingsDialog::on_cbOpenSettingsDialogOnStart_stateChanged( int arg1 ) {
  QSettings settings( QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) + "/config.ini",
                      QSettings::IniFormat );

  settings.setValue( "OpenSettingsDialogOnStart", bool( arg1 == Qt::CheckState::Checked ) );
  settings.sync();
}

void SettingsDialog::on_cbShowCameraToolbarOnStart_stateChanged( int arg1 ) {
  QSettings settings( QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) + "/config.ini",
                      QSettings::IniFormat );

  settings.setValue( "ShowCameraToolbarOnStart", bool( arg1 == Qt::CheckState::Checked ) );
  settings.sync();
}

void SettingsDialog::on_cbRunSimulatorOnStart_stateChanged( int arg1 ) {
  QSettings settings( QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) + "/config.ini",
                      QSettings::IniFormat );

  settings.setValue( "RunSimulatorOnStart", bool( arg1 == Qt::CheckState::Checked ) );
  settings.sync();
}

void SettingsDialog::on_pbDeleteSettings_clicked() {
  QSettings settings( QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) + "/config.ini",
                      QSettings::IniFormat );

  settings.clear();
  settings.sync();
}

void SettingsDialog::on_pbSaveAsDefault_clicked() {
  saveDefaultConfig();
}

void SettingsDialog::on_pbLoadSavedConfig_clicked() {
  loadDefaultConfig();
}

void SettingsDialog::on_pbSetStringToFilename_clicked() {
  QModelIndex index = ui->twValues->currentIndex().siblingAtColumn( 1 );

  if( index.isValid() ) {
    QString dir = ui->twValues->model()->data( index, Qt::DisplayRole ).toString();
    qDebug() << dir;
    QString fileName = QFileDialog::getOpenFileName( this,
                       tr( "Set Filename to String" ),
                       dir );

    if( !fileName.isEmpty() ) {
      ui->twValues->model()->setData( index, fileName );
    }
  }

  // as the size of the cell is most likely not enough for the filename, resize it
  ui->twValues->resizeColumnsToContents();
}
