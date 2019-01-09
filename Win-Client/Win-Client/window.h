#ifndef WINDOW_H
#define WINDOW_H

#include <QLineEdit>
#include <QWidget>
#include <QFileDialog>
#include <QLabel>
using namespace std;

class QPushButton;
class Window : public QWidget
{
	Q_OBJECT
public:
	explicit Window(QWidget *parent = 0);
	string hostName;
	string ipAdd;
	string filePath;
private:
	QPushButton *m_button;
	QPushButton *m_button2;
	QLineEdit *line_edit;
	QLineEdit *line_edit2;
	QLabel *label;
	QLabel *label2;
	QLabel *label3;
	private slots:
	void slotButtonClicked();
	void slotButtonClicked2();
};

#endif // WINDOW_H
