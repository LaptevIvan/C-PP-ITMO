#include "Cell.h"

Cell::Cell(uint row, uint column, QWidget *parent) :
	QPushButton(parent), visible(false), stateReal(whiteEmpty), stateVisible(grayUnknown), y(row), x(column)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setStyleSheet("QPushButton { background-color: gray; padding: 0px; font-size: 20px; }");
	setMinimumSize(31, 31);
}

void Cell::setVisibleState(cellCategory newStateVisible, bool save)
{
	QString color;
	switch (newStateVisible)
	{
	case redMine:
		color = "red";
		break;

	case grayUnknown:
		color = "gray";
		break;

	case whiteEmpty:
		color = "white";
		break;

	case brownFlag:
		color = "brown";
		break;

	case greenNeighbour:
		color = "green";
		break;

	case explodedMine:
		color = "yellow";
	}

	setStyleSheet(QString("background-color: %1; padding: 0px; font-size: 20px;").arg(color));
	setText(newStateVisible == brownFlag ? "!" : "");
	if (save)
		stateVisible = newStateVisible;
}

void Cell::setCntMinesText(uint cntMines)
{
	if (!cntMines)
		return;

	QFont newFnt;
	QString color;
	switch (cntMines)
	{
	case 1:
		color = "blue";
		break;
	case 2:
		color = "green";
		break;

	case 3:
		color = "red";
		break;

	case 4:
		color = "darkblue";
		break;

	case 5:
		color = "darkred";
		break;

	case 6:
		color = "turquoise";
		break;

	case 7:
		color = "black";
		break;

	default:
		color = "gray";
	}
	setStyleSheet(QString("background-color: white; padding: 0px; font-size: 20px; color: %1").arg(color));
	setText(QString::number(cntMines));
}

void Cell::discover(uint countMineNear)
{
	if (visible)
		return;

	visible = true;
	setVisibleState(stateReal);
	if (stateReal != redMine && stateReal != explodedMine)
		setCntMinesText(countMineNear);
	emit haveOpened();
}

void Cell::mousePressEvent(QMouseEvent *event)
{
	Qt::MouseButton pressed = event->button();
	if (pressed == Qt::LeftButton)
		emit pressLeft();
	else if (pressed == Qt::RightButton)
		emit pressRight();
	else
		emit pressMid();
}
