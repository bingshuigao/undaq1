/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.10.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QTabWidget *aaa;
    QWidget *tab;
    QTextBrowser *textBrowser;
    QLabel *label_2;
    QPushButton *pushButton_4;
    QWidget *tab_2;
    QLabel *label_3;
    QLabel *label_4;
    QLabel *label_5;
    QWidget *tab_3;
    QTextBrowser *textBrowser_2;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_8;
    QTextBrowser *textBrowser_3;
    QComboBox *comboBox;
    QPushButton *pushButton_5;
    QLabel *label_9;
    QPushButton *pushButton_6;
    QPushButton *pushButton_7;
    QPushButton *pushButton_8;
    QSpinBox *spinBox;
    QLabel *label_10;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QLabel *label;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(792, 506);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        aaa = new QTabWidget(centralWidget);
        aaa->setObjectName(QStringLiteral("aaa"));
        aaa->setGeometry(QRect(0, 30, 831, 451));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        textBrowser = new QTextBrowser(tab);
        textBrowser->setObjectName(QStringLiteral("textBrowser"));
        textBrowser->setGeometry(QRect(0, 30, 781, 331));
        label_2 = new QLabel(tab);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(0, 0, 91, 31));
        pushButton_4 = new QPushButton(tab);
        pushButton_4->setObjectName(QStringLiteral("pushButton_4"));
        pushButton_4->setGeometry(QRect(620, 0, 161, 28));
        aaa->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        label_3 = new QLabel(tab_2);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(10, 10, 181, 31));
        label_4 = new QLabel(tab_2);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(10, 40, 181, 31));
        label_5 = new QLabel(tab_2);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(10, 70, 181, 31));
        aaa->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        textBrowser_2 = new QTextBrowser(tab_3);
        textBrowser_2->setObjectName(QStringLiteral("textBrowser_2"));
        textBrowser_2->setGeometry(QRect(0, 40, 221, 321));
        label_6 = new QLabel(tab_3);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setGeometry(QRect(10, 60, 191, 31));
        label_7 = new QLabel(tab_3);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setGeometry(QRect(10, 80, 191, 31));
        label_8 = new QLabel(tab_3);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setGeometry(QRect(10, 100, 191, 31));
        textBrowser_3 = new QTextBrowser(tab_3);
        textBrowser_3->setObjectName(QStringLiteral("textBrowser_3"));
        textBrowser_3->setGeometry(QRect(250, 40, 531, 331));
        comboBox = new QComboBox(tab_3);
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->setObjectName(QStringLiteral("comboBox"));
        comboBox->setGeometry(QRect(250, 10, 251, 27));
        pushButton_5 = new QPushButton(tab_3);
        pushButton_5->setObjectName(QStringLiteral("pushButton_5"));
        pushButton_5->setGeometry(QRect(640, 10, 71, 28));
        label_9 = new QLabel(tab_3);
        label_9->setObjectName(QStringLiteral("label_9"));
        label_9->setGeometry(QRect(260, 60, 451, 31));
        pushButton_6 = new QPushButton(tab_3);
        pushButton_6->setObjectName(QStringLiteral("pushButton_6"));
        pushButton_6->setGeometry(QRect(0, 10, 91, 28));
        pushButton_7 = new QPushButton(tab_3);
        pushButton_7->setObjectName(QStringLiteral("pushButton_7"));
        pushButton_7->setGeometry(QRect(100, 10, 121, 28));
        pushButton_8 = new QPushButton(tab_3);
        pushButton_8->setObjectName(QStringLiteral("pushButton_8"));
        pushButton_8->setGeometry(QRect(710, 10, 71, 28));
        spinBox = new QSpinBox(tab_3);
        spinBox->setObjectName(QStringLiteral("spinBox"));
        spinBox->setGeometry(QRect(580, 10, 53, 28));
        spinBox->setValue(10);
        label_10 = new QLabel(tab_3);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setGeometry(QRect(510, 10, 71, 31));
        aaa->addTab(tab_3, QString());
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(0, 0, 121, 31));
        pushButton_2 = new QPushButton(centralWidget);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(130, 0, 101, 28));
        pushButton_3 = new QPushButton(centralWidget);
        pushButton_3->setObjectName(QStringLiteral("pushButton_3"));
        pushButton_3->setGeometry(QRect(240, 0, 101, 28));
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(360, 0, 141, 31));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 792, 24));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        aaa->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        label_2->setText(QApplication::translate("MainWindow", "Messages:", nullptr));
        pushButton_4->setText(QApplication::translate("MainWindow", "Save to file...", nullptr));
        aaa->setTabText(aaa->indexOf(tab), QApplication::translate("MainWindow", "Log", nullptr));
        label_3->setText(QApplication::translate("MainWindow", "Data rate:  98 KB/s", nullptr));
        label_4->setText(QApplication::translate("MainWindow", "Trigger rate:  45/s", nullptr));
        label_5->setText(QApplication::translate("MainWindow", "Free Disk:   450 GB", nullptr));
        aaa->setTabText(aaa->indexOf(tab_2), QApplication::translate("MainWindow", "Stats", nullptr));
        label_6->setText(QApplication::translate("MainWindow", "Ring buffer1 load: 43%", nullptr));
        label_7->setText(QApplication::translate("MainWindow", "Ring buffer2 load: 41%", nullptr));
        label_8->setText(QApplication::translate("MainWindow", "Ring buffer3 load: 13%", nullptr));
        comboBox->setItemText(0, QApplication::translate("MainWindow", "Ring buffer 1", nullptr));
        comboBox->setItemText(1, QApplication::translate("MainWindow", "Ring buffer 2", nullptr));
        comboBox->setItemText(2, QApplication::translate("MainWindow", "Ring buffer 3", nullptr));

        pushButton_5->setText(QApplication::translate("MainWindow", "Dump", nullptr));
        label_9->setText(QApplication::translate("MainWindow", "Dumped ring buffer containt in hexdecimal format", nullptr));
        pushButton_6->setText(QApplication::translate("MainWindow", "Update", nullptr));
        pushButton_7->setText(QApplication::translate("MainWindow", "Auto Update...", nullptr));
        pushButton_8->setText(QApplication::translate("MainWindow", "Clear", nullptr));
        label_10->setText(QApplication::translate("MainWindow", "Number:", nullptr));
        aaa->setTabText(aaa->indexOf(tab_3), QApplication::translate("MainWindow", "Ring buffers", nullptr));
        pushButton->setText(QApplication::translate("MainWindow", "Start", nullptr));
        pushButton_2->setText(QApplication::translate("MainWindow", "Stop", nullptr));
        pushButton_3->setText(QApplication::translate("MainWindow", "Quit", nullptr));
        label->setText(QApplication::translate("MainWindow", "Status: Started", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
