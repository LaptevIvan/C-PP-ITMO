#include "Explanation.h"

Explanation::Explanation(QWidget *parent) : QDialog(parent)
{
	QGridLayout *mainLayout = new QGridLayout(this);

	brownFlagTxt =
		new QLabel(QLabel::tr("This is a flag that you can set on the cell to avoid accidentally revealing it."), this);
	QPixmap bFlag(":/img/brownFlag.png");
	brownFlag = new QLabel(this);
	brownFlag->setPixmap(bFlag);
	mainLayout->addWidget(brownFlag, 0, 0);
	mainLayout->addWidget(brownFlagTxt, 0, 1);

	redMineTxt = new QLabel(
		QLabel::tr("This is a mine, if you open the cell under which the mine is hidden, then you will lose."),
		this);
	QPixmap rMine(":/img/redMine.png");
	redMine = new QLabel(this);
	redMine->setPixmap(rMine);
	mainLayout->addWidget(redMine, 1, 0);
	mainLayout->addWidget(redMineTxt, 1, 1);

	yellowMineTxt = new QLabel(QLabel::tr("This is the mine that you clicked on before the losing move."), this);
	QPixmap yMine(":/img/yellowMine.png");
	yellowMine = new QLabel(this);
	yellowMine->setPixmap(yMine);
	mainLayout->addWidget(yellowMine, 2, 0);
	mainLayout->addWidget(yellowMineTxt, 2, 1);

	greenNeighbourTxt = new QLabel(QLabel::tr("This is the next cell in which there may be mines."), this);
	QPixmap gMine(":/img/greenNeighbour.png");
	greenNeighbour = new QLabel(this);
	greenNeighbour->setPixmap(gMine);
	mainLayout->addWidget(greenNeighbour, 3, 0);
	mainLayout->addWidget(greenNeighbourTxt, 3, 1);

	setLayout(mainLayout);
	setFixedSize(650, 250);
}

void Explanation::show()
{
	exec();
}

void Explanation::translate()
{
	brownFlagTxt->setText(QLabel::tr(
		"This is a flag that you can set on the cell to avoid accidentally revealing "
		"it."));
	redMineTxt->setText(QLabel::tr(
		"This is a mine, if you open the cell under which the mine is hidden, then you will "
		"lose."));
	yellowMineTxt->setText(QLabel::tr("This is the mine that you clicked on before the losing move."));
	greenNeighbourTxt->setText(QLabel::tr("This is the next cell in which there may be mines."));
}
