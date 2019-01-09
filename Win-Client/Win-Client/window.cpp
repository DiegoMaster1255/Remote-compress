#include "window.h"
#include <QPushButton>
#include <QObject>
#include <QtWidgets>
#include <QApplication>
#include <iostream>
using namespace std;

Window::Window(QWidget *parent) :
	QWidget(parent)
{
	// Set size of the window
	setFixedSize(400, 300);

	// Create and position the button
	label = new QLabel("Server name:", this);
	label->setGeometry(100, 50, 200, 50);

	line_edit = new QLineEdit(this);
	line_edit->setGeometry(150, 100, 100, 30);

	label2 = new QLabel("Port:", this);
	label2->setGeometry(100, 150, 200, 50);

	line_edit2 = new QLineEdit(this);
	line_edit2->setGeometry(150, 200, 100, 30);

	m_button = new QPushButton("Pick a file", this);
	m_button->setGeometry(50, 250, 100, 30);
	m_button->setCheckable(true);

	m_button2 = new QPushButton("Pick a directory", this);
	m_button2->setGeometry(250, 250, 100, 30);
	m_button2->setCheckable(true);

	label3 = new QLabel("OR", this);
	label3->setGeometry(190, 235, 20, 50);

	connect(m_button, SIGNAL(clicked()), this, SLOT(slotButtonClicked()));
	connect(m_button2, SIGNAL(clicked()), this, SLOT(slotButtonClicked2()));
}

void Window::slotButtonClicked()
{
	QString temp = line_edit->text();
	hostName = temp.toStdString();
	cout << hostName << endl;

	temp = line_edit2->text();
	ipAdd = temp.toStdString();
	cout << ipAdd << endl;

	temp = QFileDialog::getOpenFileName(this,
		tr("Pick a file"), "",
		tr("All Files (*)"));

	filePath = temp.toStdString();
	/*	for (int i = filePath.length(); i > 0; i--) {
	if (filePath[i] == '/') {
	filePath.erase(0, i + 1);
	break;
	}
	} */
	cout << filePath << endl;

	close();

}

void Window::slotButtonClicked2()
{
	QString temp = line_edit->text();
	hostName = temp.toStdString();
	cout << hostName << endl;

	temp = line_edit2->text();
	ipAdd = temp.toStdString();
	cout << ipAdd << endl;

	temp = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
		"/home");
	filePath = temp.toStdString();
	/*	for (int i = filePath.length(); i > 0; i--) {
	if (filePath[i] == '/') {
	filePath.erase(0, i + 1);
	break;
	}
	}*/
	cout << filePath << endl;

	close();

}
