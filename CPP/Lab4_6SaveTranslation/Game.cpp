#include "Game.h"

uint Game::checkStateCells(Cell* curCell, bool stateReal, cellCategory catCell, QVector< Cell* >** neigh)
{
	QVector< Cell* >* n;
	n = *neigh ? *neigh : checkingNeighbours(curCell);
	uint sz = n->size();
	uint ans = 0;
	for (uint i = 0; i < sz; ++i)
	{
		Cell* curCell = n->at(i);
		cellCategory curCatCell = stateReal ? curCell->stateReal : curCell->stateVisible;
		if (curCatCell == catCell)
			++ans;
	}
	*neigh = n;
	return ans;
}

QVector< Cell* >* Game::checkingNeighbours(Cell* cell)
{
	uint i = cell->y;
	uint j = cell->x;
	QVector< Cell* >* ans = new QVector< Cell* >;
	if (j > 0)
	{
		if (!table[i][j - 1]->visible)
			ans->push_back(table[i][j - 1]);

		if (i > 0 && !table[i - 1][j - 1]->visible)
			ans->push_back(table[i - 1][j - 1]);

		if (i < rows - 1 && !table[i + 1][j - 1]->visible)
			ans->push_back(table[i + 1][j - 1]);
	}
	if (j < columns - 1)
	{
		if (!table[i][j + 1]->visible)
			ans->push_back(table[i][j + 1]);

		if (i > 0 && !table[i - 1][j + 1]->visible)
			ans->push_back(table[i - 1][j + 1]);

		if (i < rows - 1 && !table[i + 1][j + 1]->visible)
			ans->push_back(table[i + 1][j + 1]);
	}
	if (i < rows - 1 && !table[i + 1][j]->visible)
		ans->push_back(table[i + 1][j]);
	if (i > 0 && !table[i - 1][j]->visible)
		ans->push_back(table[i - 1][j]);
	return ans;
}

void Game::openCells(Cell* cell)
{
	if (cell->stateVisible == brownFlag)
		return;

	QVector< Cell* >* neighbours = nullptr;
	uint cnt = checkStateCells(cell, true, redMine, &neighbours);
	cell->discover(cnt);
	if (!cnt)
	{
		uint sz = neighbours->size();
		for (uint i = 0; i < sz; ++i)
			openCells(neighbours->at(i));
	}
	delete neighbours;
}

void Game::openTable()
{
	for (uint i = 0; i < rows; ++i)
	{
		for (uint j = 0; j < columns; ++j)
		{
			Cell* cell = table[i][j];
			QVector< Cell* >* neighb = nullptr;
			cell->discover(checkStateCells(cell, true, redMine, &neighb));
			cell->setEnabled(false);
			delete neighb;
		}
	}
}

void Game::backlight(QVector< Cell* >* cells, bool forward)
{
	uint sz = cells->size();
	for (uint i = 0; i < sz; ++i)
	{
		Cell* cell = cells->at(i);
		if (!cell->visible)
			if (forward)
				cell->setVisibleState(greenNeighbour, false);
			else
				cell->setVisibleState(cell->stateReal == redMine && visibleMines ? redMine : cell->stateVisible, false);
	}
}

void Game::lose(Cell* mine)
{
	mine->stateReal = explodedMine;
	openTable();
	emit endGame();
	QMessageBox::information(nullptr, QMessageBox::tr("LOSE"), QMessageBox::tr("You've lost."));
}

void Game::win()
{
	openTable();
	emit endGame();
	QMessageBox::information(nullptr, QMessageBox::tr("WIN!!!"), QMessageBox::tr("You've won!"));
}

Game::Game(uint rows, uint columns, uint countMines, QWidget* parent) :
	QWidget(parent), visibleMines(false), rows(rows), columns(columns), countMines(countMines), cntRemainingCells(rows * columns)
{
	QGridLayout* gridTable = new QGridLayout(this);
	gridTable->setSpacing(0);
	setLayout(gridTable);
	table.resize(rows);
	for (uint i = 0; i < rows; ++i)
	{
		table[i].resize(columns);
		for (uint j = 0; j < columns; ++j)
		{
			Cell* button = new Cell(i, j, this);
			table[i][j] = button;
			gridTable->addWidget(button, i, j);
			QObject::connect(button, &Cell::pressLeft, this, &Game::pressedLeft);
			QObject::connect(button, &Cell::pressRight, this, &Game::pressedRight);
			QObject::connect(button, &Cell::pressMid, this, &Game::pressedMid);
			QObject::connect(button, &Cell::haveOpened, this, &Game::cellIsOpen);
		}
	}
}

Game::Game(QWidget* parent) : QWidget(parent), visibleMines(false)
{
	QGridLayout* gridTable = new QGridLayout(this);
	QSettings settings("save.ini", QSettings::IniFormat);
	gridTable->setSpacing(0);
	setLayout(gridTable);

	settings.beginGroup("General");
	rows = settings.value("rows").toUInt();
	columns = settings.value("columns").toUInt();
	countMines = settings.value("cntMines").toUInt();
	settings.endGroup();

	settings.beginGroup("Rest");
	cntRemainingCells = settings.value("cntRemainingCell", cntRemainingCells).toInt();
	settings.endGroup();

	table.resize(rows);
	for (uint i = 0; i < rows; ++i)
	{
		table[i].resize(columns);
		for (uint j = 0; j < columns; ++j)
		{
			Cell* button = new Cell(i, j, this);
			table[i][j] = button;
			gridTable->addWidget(button, i, j);
			QObject::connect(button, &Cell::pressLeft, this, &Game::pressedLeft);
			QObject::connect(button, &Cell::pressRight, this, &Game::pressedRight);
			QObject::connect(button, &Cell::pressMid, this, &Game::pressedMid);
			QObject::connect(button, &Cell::haveOpened, this, &Game::cellIsOpen);

			settings.beginGroup(QString("MineAt_%1_%2").arg(i).arg(j));
			button->stateReal = (cellCategory)settings.value("stateReal").toUInt();
			button->stateVisible = (cellCategory)settings.value("stateVisible").toUInt();
			button->setVisibleState(button->stateVisible);
			button->visible = settings.value("visible").toBool();
			button->setCntMinesText(settings.value("text").toUInt());

			settings.endGroup();
		}
	}

	settings.beginGroup("cntSettedMines");
	uint cntSettedMines = settings.value("cntMines").toUInt();
	settings.endGroup();
	if (cntSettedMines)
	{
		for (uint i = 0; i < cntSettedMines; ++i)
		{
			settings.beginGroup(QString("mines_%1").arg(i));
			mines.push_back(table[settings.value("y").toUInt()][settings.value("x").toUInt()]);
			settings.endGroup();
		}
		settings.beginGroup("stateMines");
		visibleMines = settings.value("visibleMines").toBool();
		if (visibleMines)
		{
			for (uint i = 0; i < countMines; ++i)
				mines[i]->setVisibleState(redMine, false);
		}
		settings.endGroup();
	}
}

void Game::saveCells()
{
	QSettings settings("save.ini", QSettings::IniFormat);

	settings.beginGroup("Rest");
	settings.setValue("cntRemainingCell", cntRemainingCells);
	settings.endGroup();

	for (uint i = 0; i < rows; ++i)
	{
		for (uint j = 0; j < columns; ++j)
		{
			settings.beginGroup(QString("MineAt_%1_%2").arg(i).arg(j));
			settings.setValue("stateReal", (uint)table[i][j]->stateReal);
			settings.setValue("stateVisible", (uint)table[i][j]->stateVisible);
			settings.setValue("visible", table[i][j]->visible);
			settings.setValue("text", table[i][j]->text());
			settings.endGroup();
		}
	}
	uint sz = mines.size();
	settings.beginGroup("cntSettedMines");
	settings.setValue("cntMines", sz);
	settings.endGroup();
	for (uint i = 0; i < sz; ++i)
	{
		settings.beginGroup(QString("mines_%1").arg(i));
		settings.setValue("y", mines[i]->y);
		settings.setValue("x", mines[i]->x);
		settings.endGroup();
	}
	settings.beginGroup("stateMines");
	settings.setValue("visibleMines", visibleMines);
	settings.endGroup();
}

void Game::pressedLeft()
{
	Cell* pickedCell = qobject_cast< Cell* >(sender());
	if (!mines.size())
	{
		uint size = rows * columns - 1;
		QPair< uint, uint >* variants = new QPair< uint, uint >[size];
		uint added = 0;
		for (uint i = 0; i < rows; ++i)
		{
			for (uint j = 0; j < columns; ++j)
			{
				if (i != pickedCell->y || j != pickedCell->x)
					variants[added++] = QPair< uint, int >(i, j);
			}
		}

		for (uint i = 0; i < size; ++i)
		{
			uint rndInd1 = QRandomGenerator::global()->bounded(size);
			uint rndInd2 = QRandomGenerator::global()->bounded(size);
			qSwap(variants[rndInd1], variants[rndInd2]);
		}

		for (uint i = 0; i < countMines; ++i)
		{
			QPair< uint, uint > pair = variants[i];
			Cell* cell = table[pair.first][pair.second];
			mines.push_back(cell);
			cell->stateReal = redMine;
		}
		delete[] variants;
		emit appearedMines();
	}

	if (pickedCell->stateReal == redMine && pickedCell->stateVisible != brownFlag)
		lose(pickedCell);
	else if (!pickedCell->visible)
	{
		openCells(pickedCell);
		if (cntRemainingCells == countMines)
			win();
	}
}

void Game::pressedRight()
{
	Cell* pickedCell = qobject_cast< Cell* >(sender());
	if (!pickedCell->visible)
		pickedCell->setVisibleState(pickedCell->stateVisible == brownFlag ? grayUnknown : brownFlag);
	if (pickedCell->stateReal == redMine && visibleMines)
		pickedCell->setVisibleState(redMine, false);
}

void Game::pressedMid()
{
	Cell* pickedCell = qobject_cast< Cell* >(sender());

	uint cntMines = pickedCell->text().toUInt();
	if (!cntMines)
		return;

	QVector< Cell* >* neigh = checkingNeighbours(pickedCell);

	if (checkStateCells(pickedCell, false, brownFlag, &neigh) == cntMines)
	{
		uint sz = neigh->size();
		Cell* mine = nullptr;
		for (uint i = 0; i < sz; ++i)
		{
			Cell* cell = neigh->at(i);
			if (cell->stateVisible != brownFlag)
			{
				if (cell->stateReal == redMine)
				{
					mine = cell;
					break;
				}
				openCells(cell);
			}
		}
		delete neigh;
		if (mine)
			lose(mine);
		else if (cntRemainingCells == countMines)
			win();
	}
	else
	{
		backlight(neigh, true);
		QTimer::singleShot(
			100,
			this,
			[=]()
			{
				backlight(neigh, false);
				delete neigh;
			});
	}
}

void Game::cellIsOpen()
{
	--cntRemainingCells;
}

void Game::changeVisibleMines()
{
	visibleMines = !visibleMines;
	uint sz = mines.size();
	if (visibleMines)
	{
		for (uint i = 0; i < sz; ++i)
			mines[i]->setVisibleState(redMine, false);
	}
	else
	{
		for (uint i = 0; i < sz; ++i)
		{
			Cell* cur = mines[i];
			cur->setVisibleState(cur->stateVisible);
		}
	}
}
