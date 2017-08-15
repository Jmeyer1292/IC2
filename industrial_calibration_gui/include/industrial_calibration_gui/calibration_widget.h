#ifndef CALIBRATION_WIDGET_H
#define CALIBRATION_WIDGET_H

#include <QWidget>
#include <ros/ros.h>

namespace Ui
{
  class CalibrationWidget;
}

namespace industrial_calibration_gui
{
class CalibrationWidget : public QWidget
{
  Q_OBJECT
public:
  CalibrationWidget(QWidget* parent = 0);

  virtual ~CalibrationWidget();

protected Q_SLOTS:
  void instructionsCheckbox(void);
  void startCalibrationButton(void);
  void selectCalibrationTypeComboBox(void);
  void updateCalibrationTypeText(int current_index);  
  
protected:
  Ui::CalibrationWidget* ui_;
  ros::NodeHandle nh_;

private:
  bool instructions_checkbox_state_;
};
} // namespace industrial_calibration_gui

#endif //CALIBRATION_WIDGET_H
