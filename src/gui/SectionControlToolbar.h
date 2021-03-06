#ifndef SECTIONCONTROLTOOLBAR_H
#define SECTIONCONTROLTOOLBAR_H

#include <QGroupBox>
#include <QPushButton>
#include <QPalette>

namespace Ui {
  class SectionControlToolbar;
}

class SectionControlToolbar : public QGroupBox {
    Q_OBJECT

  public:
    explicit SectionControlToolbar( QWidget* parent = nullptr );
    ~SectionControlToolbar();

  private:

    enum class ButtonState : quint8 {
      Normal = 0,
      ForceOn,
      ForceOff
    };

    void showHideButtons();
    void advanceState( QWidget* button, SectionControlToolbar::ButtonState& state );

  public slots:
    void setNumberOfSections( float );
    void setName( QString );

  private slots:
    void on_pbAll_clicked();

    void on_pb1_clicked();
    void on_pb2_clicked();
    void on_pb3_clicked();
    void on_pb4_clicked();
    void on_pb5_clicked();
    void on_pb6_clicked();
    void on_pb7_clicked();
    void on_pb8_clicked();
    void on_pb9_clicked();
    void on_pb10_clicked();
    void on_pb11_clicked();
    void on_pb12_clicked();
    void on_pb13_clicked();
    void on_pb14_clicked();

  private:
    Ui::SectionControlToolbar* ui;

    int8_t numberOfSections = 0;

    QPalette buttonYellow, buttonRed, buttonGreen, buttonDefault;

    ButtonState pbAllState = ButtonState::Normal,
                pb1State = ButtonState::Normal,
                pb2State = ButtonState::Normal,
                pb3State = ButtonState::Normal,
                pb4State = ButtonState::Normal,
                pb5State = ButtonState::Normal,
                pb6State = ButtonState::Normal,
                pb7State = ButtonState::Normal,
                pb8State = ButtonState::Normal,
                pb9State = ButtonState::Normal,
                pb10State = ButtonState::Normal,
                pb11State = ButtonState::Normal,
                pb12State = ButtonState::Normal,
                pb13State = ButtonState::Normal,
                pb14State = ButtonState::Normal;
};

#endif // SECTIONCONTROLTOOLBAR_H
