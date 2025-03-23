#ifndef GAME_H
#define GAME_H

#include "Cell.h"
#include "QGridLayout"
#include "QMessageBox"
#include "QRandomGenerator"
#include "QSettings"
#include "QTimer"
#include "QWidget"

class Game : public QWidget
{
	Q_OBJECT
	QVector< QVector< Cell* > > table;
	QVector< Cell* > mines;
	bool visibleMines;
	uint rows, columns, countMines, cntRemainingCells;

  private:
	uint checkStateCells(Cell* curCell, bool stateReal, cellCategory catCell, QVector< Cell* >** neigh);

	QVector< Cell* >* checkingNeighbours(Cell* cell);

	void openCells(Cell* cell);

	void openTable();

	void backlight(QVector< Cell* >* cells, bool forward);

	void lose(Cell* mine);

	void win();

  public:
	Game(uint rows, uint columns, uint countMines, QWidget* parent = nullptr);

	Game(QWidget* parent = nullptr);

	void saveCells();

  private slots:

	void pressedLeft();

	void pressedRight();

	void pressedMid();

	void cellIsOpen();

  public slots:
	void changeVisibleMines();

  signals:
	void endGame();
	void appearedMines();
};

#endif	  // GAME_H
