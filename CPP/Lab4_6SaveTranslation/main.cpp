#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	MainWindow game(argc > 1 ? argv[1] : "");
	game.show();

	return app.exec();
}
