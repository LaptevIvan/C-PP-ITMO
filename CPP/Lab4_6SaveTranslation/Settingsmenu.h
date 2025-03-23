#ifndef SETTINGSMENU_H
#define SETTINGSMENU_H

#include "QDialog"
#include "QGroupBox"
#include "QLabel"
#include "QMessageBox"
#include "QPushButton"
#include "QRadioButton"
#include "QSpinBox"
#include <QVBoxLayout>

class SettingsMenu : public QDialog
{
	Q_OBJECT

	QLabel *labelRows, *labelColumns, *labelMines;
	QSpinBox *rows, *columns, *mines;
	QGroupBox *langVar;
	QRadioButton *eng, *rus;
	QPushButton *btnOk;

  public:
	SettingsMenu(QWidget *parent = nullptr);
	void show(uint startValRow, uint startValColumns, uint startValMines);

  private slots:
	void exit();

  public slots:
	void translate();

  signals:
	void selectedSettings(uint rows, uint columns, uint mines);
};

#endif	  // SETTINGSMENU_H
