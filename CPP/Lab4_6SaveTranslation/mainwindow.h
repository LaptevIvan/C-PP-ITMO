#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Cell.h"
#include "Explanation.h"
#include "Game.h"
#include "Settingsmenu.h"

#include <QAction>
#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QTranslator>

class MainWindow : public QMainWindow
{
	Q_OBJECT

	SettingsMenu* settings;
	Explanation* help;
	QAction *actSettings, *actPlaying, *actExplanations, *translateRussian, *translateEnglish, *actLookMines;
	QMenuBar* mB;
	QMenu* m;
	QToolBar* t;
	Game* table;
	QTranslator* transl;
	uint rows, columns, cntMines;
	bool playingGame, settingsMines, isRussian;

	void localTranslate();

  public:
	MainWindow(QString arg, QWidget* parent = nullptr);

  protected:
	void closeEvent(QCloseEvent* event) override;

  private slots:
	void createTable();

	void onSettingsSelected(uint rows, uint columns, uint mines);

	void endGameField();

	void appearedMines();

	void showSettins();

	void russianTranslate();

	void englishTranslate();

  signals:
	void doTranslate();
};

#endif	  // MAINWINDOW_H
