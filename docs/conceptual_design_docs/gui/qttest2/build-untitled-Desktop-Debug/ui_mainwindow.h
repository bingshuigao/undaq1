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
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QTabWidget *tabWidget;
    QWidget *tab;
    QListWidget *listWidget;
    QLabel *label;
    QLabel *label_2;
    QListWidget *listWidget_2;
    QListWidget *listWidget_3;
    QPushButton *pushButton_3;
    QLabel *label_3;
    QSpinBox *spinBox;
    QPushButton *pushButton_4;
    QComboBox *comboBox;
    QLabel *label_4;
    QWidget *tab_2;
    QWidget *tab_3;
    QWidget *tab_4;
    QWidget *tab_5;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_5;
    QMenuBar *menuBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(1047, 751);
        MainWindow->setLayoutDirection(Qt::LeftToRight);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        tabWidget->setGeometry(QRect(0, 30, 1041, 671));
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        listWidget = new QListWidget(tab);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        listWidget->setObjectName(QStringLiteral("listWidget"));
        listWidget->setGeometry(QRect(10, 50, 251, 401));
        label = new QLabel(tab);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(10, 20, 161, 19));
        label_2 = new QLabel(tab);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(590, 20, 160, 19));
        listWidget_2 = new QListWidget(tab);
        new QListWidgetItem(listWidget_2);
        new QListWidgetItem(listWidget_2);
        new QListWidgetItem(listWidget_2);
        new QListWidgetItem(listWidget_2);
        new QListWidgetItem(listWidget_2);
        new QListWidgetItem(listWidget_2);
        new QListWidgetItem(listWidget_2);
        new QListWidgetItem(listWidget_2);
        new QListWidgetItem(listWidget_2);
        new QListWidgetItem(listWidget_2);
        new QListWidgetItem(listWidget_2);
        new QListWidgetItem(listWidget_2);
        new QListWidgetItem(listWidget_2);
        listWidget_2->setObjectName(QStringLiteral("listWidget_2"));
        listWidget_2->setGeometry(QRect(600, 50, 249, 411));
        listWidget_3 = new QListWidget(tab);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        new QListWidgetItem(listWidget_3);
        listWidget_3->setObjectName(QStringLiteral("listWidget_3"));
        listWidget_3->setGeometry(QRect(500, 50, 101, 411));
        pushButton_3 = new QPushButton(tab);
        pushButton_3->setObjectName(QStringLiteral("pushButton_3"));
        pushButton_3->setGeometry(QRect(330, 260, 101, 28));
        label_3 = new QLabel(tab);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(330, 140, 41, 31));
        spinBox = new QSpinBox(tab);
        spinBox->setObjectName(QStringLiteral("spinBox"));
        spinBox->setGeometry(QRect(370, 140, 53, 28));
        spinBox->setValue(6);
        pushButton_4 = new QPushButton(tab);
        pushButton_4->setObjectName(QStringLiteral("pushButton_4"));
        pushButton_4->setGeometry(QRect(20, 500, 121, 28));
        comboBox = new QComboBox(tab);
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->addItem(QString());
        comboBox->setObjectName(QStringLiteral("comboBox"));
        comboBox->setGeometry(QRect(370, 180, 91, 27));
        label_4 = new QLabel(tab);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(320, 180, 51, 31));
        tabWidget->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QStringLiteral("tab_2"));
        tabWidget->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QStringLiteral("tab_3"));
        tabWidget->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName(QStringLiteral("tab_4"));
        tabWidget->addTab(tab_4, QString());
        tab_5 = new QWidget();
        tab_5->setObjectName(QStringLiteral("tab_5"));
        tabWidget->addTab(tab_5, QString());
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(0, 0, 101, 28));
        pushButton_2 = new QPushButton(centralWidget);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(100, 0, 101, 28));
        pushButton_5 = new QPushButton(centralWidget);
        pushButton_5->setObjectName(QStringLiteral("pushButton_5"));
        pushButton_5->setGeometry(QRect(200, 0, 101, 28));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1047, 24));
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));

        const bool __sortingEnabled = listWidget->isSortingEnabled();
        listWidget->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = listWidget->item(0);
        ___qlistwidgetitem->setText(QApplication::translate("MainWindow", "V2718", nullptr));
        QListWidgetItem *___qlistwidgetitem1 = listWidget->item(1);
        ___qlistwidgetitem1->setText(QApplication::translate("MainWindow", "MADC32", nullptr));
        QListWidgetItem *___qlistwidgetitem2 = listWidget->item(2);
        ___qlistwidgetitem2->setText(QApplication::translate("MainWindow", "V1190B", nullptr));
        QListWidgetItem *___qlistwidgetitem3 = listWidget->item(3);
        ___qlistwidgetitem3->setText(QApplication::translate("MainWindow", "V830", nullptr));
        listWidget->setSortingEnabled(__sortingEnabled);

        label->setText(QApplication::translate("MainWindow", "Supported modues", nullptr));
        label_2->setText(QApplication::translate("MainWindow", "Selected modues", nullptr));

        const bool __sortingEnabled1 = listWidget_2->isSortingEnabled();
        listWidget_2->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem4 = listWidget_2->item(0);
        ___qlistwidgetitem4->setText(QApplication::translate("MainWindow", "V2718 ", nullptr));
        QListWidgetItem *___qlistwidgetitem5 = listWidget_2->item(1);
        ___qlistwidgetitem5->setText(QApplication::translate("MainWindow", "----------", nullptr));
        QListWidgetItem *___qlistwidgetitem6 = listWidget_2->item(2);
        ___qlistwidgetitem6->setText(QApplication::translate("MainWindow", "MADC32 (T)", nullptr));
        QListWidgetItem *___qlistwidgetitem7 = listWidget_2->item(3);
        ___qlistwidgetitem7->setText(QApplication::translate("MainWindow", "MADC32 (T)", nullptr));
        QListWidgetItem *___qlistwidgetitem8 = listWidget_2->item(4);
        ___qlistwidgetitem8->setText(QApplication::translate("MainWindow", "MADC32 (T)", nullptr));
        QListWidgetItem *___qlistwidgetitem9 = listWidget_2->item(5);
        ___qlistwidgetitem9->setText(QApplication::translate("MainWindow", "----------", nullptr));
        QListWidgetItem *___qlistwidgetitem10 = listWidget_2->item(6);
        ___qlistwidgetitem10->setText(QApplication::translate("MainWindow", "----------", nullptr));
        QListWidgetItem *___qlistwidgetitem11 = listWidget_2->item(7);
        ___qlistwidgetitem11->setText(QApplication::translate("MainWindow", "V1190B (T)", nullptr));
        QListWidgetItem *___qlistwidgetitem12 = listWidget_2->item(8);
        ___qlistwidgetitem12->setText(QApplication::translate("MainWindow", "V1190B (T)", nullptr));
        QListWidgetItem *___qlistwidgetitem13 = listWidget_2->item(9);
        ___qlistwidgetitem13->setText(QApplication::translate("MainWindow", "----------", nullptr));
        QListWidgetItem *___qlistwidgetitem14 = listWidget_2->item(10);
        ___qlistwidgetitem14->setText(QApplication::translate("MainWindow", "----------", nullptr));
        QListWidgetItem *___qlistwidgetitem15 = listWidget_2->item(11);
        ___qlistwidgetitem15->setText(QApplication::translate("MainWindow", "----------", nullptr));
        QListWidgetItem *___qlistwidgetitem16 = listWidget_2->item(12);
        ___qlistwidgetitem16->setText(QApplication::translate("MainWindow", "V830 (S)", nullptr));
        listWidget_2->setSortingEnabled(__sortingEnabled1);


        const bool __sortingEnabled2 = listWidget_3->isSortingEnabled();
        listWidget_3->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem17 = listWidget_3->item(0);
        ___qlistwidgetitem17->setText(QApplication::translate("MainWindow", "SLOT 01-->", nullptr));
        QListWidgetItem *___qlistwidgetitem18 = listWidget_3->item(1);
        ___qlistwidgetitem18->setText(QApplication::translate("MainWindow", "SLOT 02-->", nullptr));
        QListWidgetItem *___qlistwidgetitem19 = listWidget_3->item(2);
        ___qlistwidgetitem19->setText(QApplication::translate("MainWindow", "SLOT 03-->", nullptr));
        QListWidgetItem *___qlistwidgetitem20 = listWidget_3->item(3);
        ___qlistwidgetitem20->setText(QApplication::translate("MainWindow", "SLOT 04-->", nullptr));
        QListWidgetItem *___qlistwidgetitem21 = listWidget_3->item(4);
        ___qlistwidgetitem21->setText(QApplication::translate("MainWindow", "SLOT 05-->", nullptr));
        QListWidgetItem *___qlistwidgetitem22 = listWidget_3->item(5);
        ___qlistwidgetitem22->setText(QApplication::translate("MainWindow", "SLOT 06-->", nullptr));
        QListWidgetItem *___qlistwidgetitem23 = listWidget_3->item(6);
        ___qlistwidgetitem23->setText(QApplication::translate("MainWindow", "SLOT 07", nullptr));
        QListWidgetItem *___qlistwidgetitem24 = listWidget_3->item(7);
        ___qlistwidgetitem24->setText(QApplication::translate("MainWindow", "SLOT 08", nullptr));
        QListWidgetItem *___qlistwidgetitem25 = listWidget_3->item(8);
        ___qlistwidgetitem25->setText(QApplication::translate("MainWindow", "SLOT 09", nullptr));
        QListWidgetItem *___qlistwidgetitem26 = listWidget_3->item(9);
        ___qlistwidgetitem26->setText(QApplication::translate("MainWindow", "SOLT 10", nullptr));
        QListWidgetItem *___qlistwidgetitem27 = listWidget_3->item(10);
        ___qlistwidgetitem27->setText(QApplication::translate("MainWindow", "SLOT 11", nullptr));
        QListWidgetItem *___qlistwidgetitem28 = listWidget_3->item(11);
        ___qlistwidgetitem28->setText(QApplication::translate("MainWindow", "SLOT 12", nullptr));
        QListWidgetItem *___qlistwidgetitem29 = listWidget_3->item(12);
        ___qlistwidgetitem29->setText(QApplication::translate("MainWindow", "SLOT 13", nullptr));
        QListWidgetItem *___qlistwidgetitem30 = listWidget_3->item(13);
        ___qlistwidgetitem30->setText(QApplication::translate("MainWindow", "SLOT 14", nullptr));
        QListWidgetItem *___qlistwidgetitem31 = listWidget_3->item(14);
        ___qlistwidgetitem31->setText(QApplication::translate("MainWindow", "SLOT 15", nullptr));
        QListWidgetItem *___qlistwidgetitem32 = listWidget_3->item(15);
        ___qlistwidgetitem32->setText(QApplication::translate("MainWindow", "SLOT 16", nullptr));
        QListWidgetItem *___qlistwidgetitem33 = listWidget_3->item(16);
        ___qlistwidgetitem33->setText(QApplication::translate("MainWindow", "SLOT 17", nullptr));
        QListWidgetItem *___qlistwidgetitem34 = listWidget_3->item(17);
        ___qlistwidgetitem34->setText(QApplication::translate("MainWindow", "SLOT 18", nullptr));
        QListWidgetItem *___qlistwidgetitem35 = listWidget_3->item(18);
        ___qlistwidgetitem35->setText(QApplication::translate("MainWindow", "SLOT 19", nullptr));
        QListWidgetItem *___qlistwidgetitem36 = listWidget_3->item(19);
        ___qlistwidgetitem36->setText(QApplication::translate("MainWindow", "SLOT 20", nullptr));
        QListWidgetItem *___qlistwidgetitem37 = listWidget_3->item(20);
        ___qlistwidgetitem37->setText(QApplication::translate("MainWindow", "SLOT 21", nullptr));
        listWidget_3->setSortingEnabled(__sortingEnabled2);

        pushButton_3->setText(QApplication::translate("MainWindow", "Add ==>", nullptr));
        label_3->setText(QApplication::translate("MainWindow", "Slot: ", nullptr));
        pushButton_4->setText(QApplication::translate("MainWindow", "Advanced...", nullptr));
        comboBox->setItemText(0, QApplication::translate("MainWindow", "Trigger", nullptr));
        comboBox->setItemText(1, QApplication::translate("MainWindow", "Scaler", nullptr));
        comboBox->setItemText(2, QApplication::translate("MainWindow", "NULL", nullptr));

        label_4->setText(QApplication::translate("MainWindow", "Type:", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("MainWindow", "Frontend", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("MainWindow", "EvtBld", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_3), QApplication::translate("MainWindow", "Analyser", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_4), QApplication::translate("MainWindow", "Logger", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_5), QApplication::translate("MainWindow", "Control", nullptr));
        pushButton->setText(QApplication::translate("MainWindow", "Open...", nullptr));
        pushButton_2->setText(QApplication::translate("MainWindow", "Create", nullptr));
        pushButton_5->setText(QApplication::translate("MainWindow", "Save", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
