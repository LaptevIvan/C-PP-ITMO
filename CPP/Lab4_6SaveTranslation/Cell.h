#ifndef CELL_H
#define CELL_H

#include "QMouseEvent"
#include "QPushButton"
class QMouseEvent;

typedef enum stCell
{
	redMine,
	grayUnknown,
	whiteEmpty,
	brownFlag,
	greenNeighbour,
	explodedMine
} cellCategory;

class Cell : public QPushButton
{
	Q_OBJECT

  public:
	bool visible;
	cellCategory stateReal, stateVisible;
	uint y, x;
	Cell(uint row, uint column, QWidget *parent = nullptr);

	void setVisibleState(cellCategory newStateVisible, bool save = true);

	void setCntMinesText(uint cntMines);

	void discover(uint countMineNear);

  protected:
	void mousePressEvent(QMouseEvent *event) override;

  signals:
	void haveOpened();
	void pressLeft();
	void pressRight();
	void pressMid();
	void openEmptyCell();
};

#endif	  // CELL_H
