#include "driver_loader.h"

#include <QDir>
#include <QSettings>
#include <QFileDialog>

DriverLoader::DriverLoader(QWidget* parent)
  : QMainWindow(parent)
{
  ui.setupUi(this);
  QSettings config("config.ini", QSettings::IniFormat);
  QStringList app_list;
  app_list = config.value("sys_path", app_list).value<QStringList>();
  for (auto i : app_list) {
    ui.comboBox_path->addItem(i);
  }
}

DriverLoader::~DriverLoader()
{}

bool DriverLoader::InitInfo() {
  file_path_ = ui.comboBox_path->currentText();
  QFileInfo fileinfo(file_path_);
  if (!fileinfo.isFile()) {
    ui.textBrowser_log->append(QString::fromUtf8("<h1><font color=red>选择的文件不存在!</font></h1>"));
    return false;
  }
  file_name_ = fileinfo.fileName();
  file_native_path_ = QDir::toNativeSeparators(file_path_).toStdWString();

  if (sc_manager_ == NULL) {
    sc_manager_ = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  }
  if (!sc_manager_) {
    auto err = GetLastError();
    ui.textBrowser_log->append(QString("<h1><font color=red>OpenSCManager GetLastError 0x%1</font></h1>").arg(QString::number(err, 16)));
    return false;
  }
  return true;
}

bool DriverLoader::OpenSysService() {
  if (!InitInfo()) {
    return false;
  }

  sc_handle_ = OpenService(sc_manager_, file_name_.toStdWString().c_str(), SERVICE_ALL_ACCESS);
  if (sc_handle_ == NULL) {
    DWORD error = GetLastError();
    if (error == ERROR_SERVICE_DOES_NOT_EXIST) {
      ui.textBrowser_log->append(QString::fromUtf8("<h1><font color=red>服务已经不存在!</font></h1>"));
    }
    else {
      ui.textBrowser_log->append(QString("<h1><font color=red>OpenService error！GetLastError 0x%1</font></h1>").arg(QString::number(error, 16)));
    }
    return false;
  }

  return true;
}

void DriverLoader::CloseSysService() {
  CloseServiceHandle(sc_handle_);
  sc_handle_ = nullptr;
}

void DriverLoader::on_btn_register_clicked() {
  if (!InitInfo()) {
    return;
  }

  SC_HANDLE serviceHandle = CreateService(sc_manager_, file_name_.toStdWString().c_str(), file_name_.toStdWString().c_str(), SERVICE_ALL_ACCESS,
    SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, file_native_path_.c_str(), NULL, NULL, NULL, NULL, NULL);

  if (serviceHandle == NULL) {
    DWORD error = GetLastError();
    if (error == ERROR_SERVICE_EXISTS) {
      ui.textBrowser_log->append(QString::fromUtf8("<h1><font>服务已经存在不需要创建了!</font></h1>"));
    }
    else {
      ui.textBrowser_log->append(QString::fromUtf8("<h1><font color=red>创建服务失败！GetLastError 0x%1</font></h1>").arg(QString::number(error, 16)));
    }
    return;
  }
  CloseServiceHandle(serviceHandle);
  ui.textBrowser_log->append(QString::fromUtf8("<h1><font>服务创建成功!</font></h1>"));
}

void DriverLoader::on_btn_run_clicked() {
  if (!OpenSysService()) {
    return;
  }

  bool result = StartService(sc_handle_, 0, NULL);
  if (result == false) {
    DWORD error = GetLastError();
    if (error == ERROR_SERVICE_ALREADY_RUNNING) {
      ui.textBrowser_log->append(QString::fromUtf8("<h1><font color=red>服务已经运行!</font></h1>"));
    }
    else {
      ui.textBrowser_log->append(QString("<h1><font color=red>StartService error！GetLastError 0x%1</font></h1>").arg(QString::number(error, 16)));
    }
  }

  CloseSysService();
  ui.textBrowser_log->append(QString::fromUtf8("<h1><font>服务运行成功!</font></h1>"));
}

void DriverLoader::on_btn_stop_clicked() {
  if (!OpenSysService()) {
    return;
  }

  SERVICE_STATUS error = { 0 };
  int result = ControlService(sc_handle_, SERVICE_CONTROL_STOP, &error);
  if (result == 0) {
    DWORD error = GetLastError();
    if (error == ERROR_SERVICE_NOT_ACTIVE) {
      ui.textBrowser_log->append(QString::fromUtf8("<h1><font color=red>服务没有运行!</font></h1>"));
    }
    else {
      DWORD error = GetLastError();
      ui.textBrowser_log->append(QString("<h1><font color=red>ControlService error！GetLastError 0x%1</font></h1>").arg(QString::number(error, 16)));
    }
  }

  CloseSysService();
  ui.textBrowser_log->append(QString::fromUtf8("<h1><font>服务停止成功!</font></h1>"));
}

void DriverLoader::on_btn_unregister_clicked() {
  if (!OpenSysService()) {
    return;
  }

  if (!DeleteService(sc_handle_))
  {
    DWORD error = GetLastError();
    ui.textBrowser_log->append(QString("<h1><font color=red>DeleteService error！GetLastError 0x%1</font></h1>").arg(QString::number(error, 16)));
    return;
  }

  CloseSysService();
  CloseServiceHandle(sc_manager_);
  sc_manager_ = NULL;
  ui.textBrowser_log->append(QString::fromUtf8("<h1><font>服务卸载成功!</font></h1>"));
}

void DriverLoader::on_comboBox_path_editTextChanged(const QString& text) {
  if (text.isEmpty())
    return;
  QSettings config("config.ini", QSettings::IniFormat);
  QStringList app_list;
  app_list = config.value("sys_path", app_list).value<QStringList>();
  if (!app_list.contains(text)) {
    app_list.insert(0, text);
    config.setValue("sys_path", app_list);
    config.sync();
  }
}

void DriverLoader::on_btn_sys_selecte_clicked() {
  QString path = QDir::currentPath();
  QString file_path = ui.comboBox_path->currentText();
  if (!file_path.isEmpty()) {
    QFileInfo fileinfo(file_path);
    path = fileinfo.absolutePath();
  }
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
    path,
    tr("Images (*.sys)"));
  if (!fileName.isEmpty())
    ui.comboBox_path->setCurrentText(fileName);
}
