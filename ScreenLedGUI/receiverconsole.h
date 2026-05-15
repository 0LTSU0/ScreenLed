#ifndef RECEIVERCONSOLE_H
#define RECEIVERCONSOLE_H

#include <QWidget>
#include <vector>
#include <QString>

namespace Ui {
class ReceiverConsole;
}

class ReceiverConsole : public QWidget
{
    Q_OBJECT

public:
    explicit ReceiverConsole(QWidget *parent = nullptr);
    ~ReceiverConsole();

    // funcs
    void fillInitialOutput(std::vector<QString>&);
    void appendOutput(QString);

private slots:
    void on_closeButton_clicked();

private:
    Ui::ReceiverConsole *ui;
};

#endif // RECEIVERCONSOLE_H
