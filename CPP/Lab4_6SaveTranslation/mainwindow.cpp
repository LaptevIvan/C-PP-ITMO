#include "mainwindow.h"

MainWindow::MainWindow(QString arg, QWidget* parent) :
	QMainWindow(parent), actLookMines(nullptr), table(nullptr), rows(10), columns(10), cntMines(10), playingGame(false),
	settingsMines(false), isRussian(false)
{
	this->statusBar()->show();
	settings = new SettingsMenu(this);
	QObject::connect(settings, &SettingsMenu::selectedSettings, this, &MainWindow::onSettingsSelected);
	QObject::connect(this, &MainWindow::doTranslate, settings, &SettingsMenu::translate);

	help = new Explanation(this);
	QObject::connect(this, &MainWindow::doTranslate, help, &Explanation::translate);

	actSettings = new QAction(QAction::tr("Settings"), this);
	QObject::connect(actSettings, &QAction::triggered, this, &MainWindow::showSettins);

	actPlaying = new QAction(QAction::tr("Play game"), this);
	QObject::connect(actPlaying, &QAction::triggered, this, &MainWindow::createTable);

	actExplanations = new QAction(QAction::tr("Help"), this);
	QObject::connect(actExplanations, &QAction::triggered, help, &Explanation::show);
	transl = new QTranslator(this);
	transl->load(":/Lab4_6SaveTranslation_en_US.qm");
	qApp->installTranslator(transl);

	translateRussian = new QAction("Русский", this);
	QObject::connect(translateRussian, &QAction::triggered, this, &MainWindow::russianTranslate);

	translateEnglish = new QAction("English", this);
	QObject::connect(translateEnglish, &QAction::triggered, this, &MainWindow::englishTranslate);

	mB = new QMenuBar(this);
	setMenuBar(mB);

	m = new QMenu(QMenu::tr("Settings"), mB);
	mB->addMenu(m);
	m->addAction(actSettings);
	m->addAction(actPlaying);
	m->addAction(actExplanations);
	m->addAction(translateRussian);
	m->addAction(translateEnglish);

	t = new QToolBar(QToolBar::tr("Toolbar"));
	addToolBar(t);
	t->addAction(actSettings);
	t->addAction(actPlaying);
	t->addAction(actExplanations);
	t->addAction(translateRussian);
	t->addAction(translateEnglish);

	if (arg == "dbg")
	{
		actLookMines = new QAction(QAction::tr("Look at mines"), this);
		m->addAction(actLookMines);
		t->addAction(actLookMines);
	}
	emit(createTable());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	QSettings settings("save.ini", QSettings::IniFormat);
	if (!playingGame)
		settings.clear();
	else
	{
		settings.beginGroup("Exists");
		settings.setValue("playingGame", playingGame);
		settings.endGroup();

		settings.beginGroup("General");
		settings.setValue("rows", rows);
		settings.setValue("columns", columns);
		settings.setValue("cntMines", cntMines);
		settings.setValue("settingsMines", settingsMines);
		settings.setValue("isRussian", isRussian);
		settings.endGroup();
		table->saveCells();
	}
}

void MainWindow::createTable()
{
	QSettings settings("save.ini", QSettings::IniFormat);
	settings.beginGroup("Exists");
	playingGame = settings.value("playingGame", false).toBool();
	settings.endGroup();
	if (playingGame)
	{
		settings.beginGroup("General");
		rows = settings.value("rows").toUInt();
		columns = settings.value("columns").toUInt();
		cntMines = settings.value("cntMines").toUInt();
		settingsMines = settings.value("settingsMines").toBool();
		isRussian = settings.value("isRussian").toBool();
		if (isRussian)
			russianTranslate();
		settings.endGroup();
		table = new Game(this);
		settings.clear();
	}
	else
	{
		delete table;
		table = new Game(rows, columns, cntMines, this);
	}
	if (actLookMines)
	{
		QObject::connect(actLookMines, &QAction::triggered, table, &Game::changeVisibleMines);
		QObject::connect(table, &Game::endGame, this, &MainWindow::endGameField);
		QObject::connect(table, &Game::appearedMines, this, &MainWindow::appearedMines);
		actLookMines->setEnabled(settingsMines);
	}
	playingGame = true;
	setCentralWidget(table);
	table->show();
}

void MainWindow::onSettingsSelected(uint rows, uint columns, uint mines)
{
	this->rows = rows;
	this->columns = columns;
	this->cntMines = mines;
    emit createTable();

	actSettings->setToolTip(QAction::tr("Settings"));
	actPlaying->setToolTip(QAction::tr("Play game"));
	actExplanations->setToolTip(QAction::tr("Help"));
}

void MainWindow::endGameField()
{
	playingGame = false;
	actLookMines->setEnabled(false);
}

void MainWindow::appearedMines()
{
	settingsMines = true;
	actLookMines->setEnabled(settingsMines);
}

void MainWindow::showSettins()
{
	settings->show(rows, columns, cntMines);
}

void MainWindow::russianTranslate()
{
	isRussian = true;
	transl->load(":/Lab4_6SaveTranslation_ru_RU.qm");
	qApp->installTranslator(transl);
	localTranslate();
}

void MainWindow::englishTranslate()
{
	isRussian = false;
	transl->load(":/Lab4_6SaveTranslation_en_US.qm");
	qApp->installTranslator(transl);
	localTranslate();
}

void MainWindow::localTranslate()
{
	actSettings->setText(QAction::tr("Settings"));
	m->setTitle(QAction::tr("Settings"));
	actSettings->setToolTip(QAction::tr("Settings"));
	actPlaying->setText(QAction::tr("Play game"));
	actExplanations->setText(QAction::tr("Help"));
	if (actLookMines)
		actLookMines->setText(QAction::tr("Look at mines"));
	emit doTranslate();
}
