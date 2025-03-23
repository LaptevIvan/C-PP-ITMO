#include "Settingsmenu.h"

SettingsMenu::SettingsMenu(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QDialog::tr("Game's settings"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    labelRows = new QLabel(QLabel::tr("Count rows"), this);
    rows = new QSpinBox(this);
    rows->setRange(1, 30);
    mainLayout->addWidget(labelRows);
    mainLayout->addWidget(rows);

    labelColumns = new QLabel(QLabel::tr("Count columns"), this);
    columns = new QSpinBox(this);
    columns->setRange(1, 30);
    mainLayout->addWidget(labelColumns);
    mainLayout->addWidget(columns);

    labelMines = new QLabel(QLabel::tr("Count mines"), this);
    mines = new QSpinBox(this);
    mines->setRange(2, (rows->maximum() * columns->maximum()) - 1);
    mainLayout->addWidget(labelMines);
    mainLayout->addWidget(mines);

    btnOk = new QPushButton(QPushButton::tr("OK"), this);
    mainLayout->addWidget(btnOk);

    setLayout(mainLayout);
    QObject::connect(btnOk, &QPushButton::clicked, this, &SettingsMenu::exit);
}

void SettingsMenu::exit()
{
    uint r = rows->value();
    uint c = columns->value();
    uint m = mines->value();

    if (r * c <= 3)
        QMessageBox::information(
                    nullptr,
                    QMessageBox::tr("Error"),
                    QMessageBox::tr("You cannot play on such a small field!"));
    else if (m > r * c - 2)
        QMessageBox::information(
                    nullptr,
                    QMessageBox::tr("Error"),
                    QMessageBox::tr("You have chosen the 'number of mines' more than the number of cells in the field minus "
                                    "2!"));
    else
    {
        emit selectedSettings(r, c, m);
        accept();
    }
}

void SettingsMenu::show(uint startValRow, uint startValColumns, uint startValMines)
{
    rows->setValue(startValRow);
    columns->setValue(startValColumns);
    mines->setValue(startValMines);
    exec();
}

void SettingsMenu::translate()
{
    setWindowTitle(QDialog::tr("Game's settings"));
    labelRows->setText(QLabel::tr("Count rows"));
    labelColumns->setText(QLabel::tr("Count columns"));
    labelMines->setText(QLabel::tr("Count mines"));
    btnOk->setText(QPushButton::tr("OK"));
}
