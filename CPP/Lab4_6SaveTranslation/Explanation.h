#ifndef EXPLANATION_H
#define EXPLANATION_H

#include "QGridLayout"

#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QWidget>

class Explanation : public QDialog
{
	Q_OBJECT

	QLabel *redMineTxt, *yellowMineTxt, *greenNeighbourTxt, *brownFlagTxt, *redMine, *yellowMine, *greenNeighbour, *brownFlag;

  public:
	Explanation(QWidget *parent = nullptr);

  public slots:
	void show();
	void translate();
};

#endif	  // EXPLANATION_H
