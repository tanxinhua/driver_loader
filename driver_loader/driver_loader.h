#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_driver_loader.h"
#include <windows.h>
#include <WinSvc.h>

class DriverLoader : public QMainWindow
{
  Q_OBJECT

public:
  DriverLoader(QWidget* parent = nullptr);
  ~DriverLoader();

public slots:
  void on_btn_register_clicked();
  void on_btn_run_clicked();
  void on_btn_stop_clicked();
  void on_btn_unregister_clicked();
  void on_comboBox_path_editTextChanged(const QString& text);
  void on_btn_sys_selecte_clicked();

private:
  bool InitInfo();
  bool OpenSysService();
  void CloseSysService();

private:
  Ui::driver_loaderClass ui;
  SC_HANDLE sc_manager_ = NULL;
  SC_HANDLE sc_handle_ = NULL;

  QString file_path_;
  QString file_name_;
  std::wstring file_native_path_;
};
