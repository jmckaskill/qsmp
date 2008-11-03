/********************************************************************************
** Form generated from reading ui file 'untitled.ui'
**
** Created: Sat 4. Oct 21:00:32 2008
**      by: Qt User Interface Compiler version 4.4.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_UNTITLED_H
#define UI_UNTITLED_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QHBoxLayout *hboxLayout;
    QPushButton *playButton;
    QPushButton *stopButton;
    QLabel *volumeLabel;
    QSlider *volumeBar;
    QVBoxLayout *vboxLayout;
    QLabel *id3Art;
    QHBoxLayout *hboxLayout1;
    QSlider *seekBar;
    QLabel *seekLabel;
    QHBoxLayout *hboxLayout2;
    QLineEdit *pathBox;
    QPushButton *changeButton;
    QLabel *artistLabel;
    QLabel *albumLabel;
    QLabel *titleLabel;
    QLineEdit *seekBox;

    void setupUi(QMainWindow *MainWindow)
    {
    if (MainWindow->objectName().isEmpty())
        MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
    MainWindow->setWindowModality(Qt::NonModal);
    MainWindow->resize(363, 187);
    centralwidget = new QWidget(MainWindow);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    gridLayout = new QGridLayout(centralwidget);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    playButton = new QPushButton(centralwidget);
    playButton->setObjectName(QString::fromUtf8("playButton"));

    hboxLayout->addWidget(playButton);

    stopButton = new QPushButton(centralwidget);
    stopButton->setObjectName(QString::fromUtf8("stopButton"));

    hboxLayout->addWidget(stopButton);

    volumeLabel = new QLabel(centralwidget);
    volumeLabel->setObjectName(QString::fromUtf8("volumeLabel"));

    hboxLayout->addWidget(volumeLabel);

    volumeBar = new QSlider(centralwidget);
    volumeBar->setObjectName(QString::fromUtf8("volumeBar"));
    volumeBar->setMinimumSize(QSize(129, 0));
    volumeBar->setMaximum(100);
    volumeBar->setOrientation(Qt::Horizontal);

    hboxLayout->addWidget(volumeBar);


    gridLayout->addLayout(hboxLayout, 0, 0, 1, 1);

    vboxLayout = new QVBoxLayout();
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    id3Art = new QLabel(centralwidget);
    id3Art->setObjectName(QString::fromUtf8("id3Art"));

    vboxLayout->addWidget(id3Art);


    gridLayout->addLayout(vboxLayout, 0, 1, 7, 1);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    seekBar = new QSlider(centralwidget);
    seekBar->setObjectName(QString::fromUtf8("seekBar"));
    seekBar->setOrientation(Qt::Horizontal);

    hboxLayout1->addWidget(seekBar);

    seekLabel = new QLabel(centralwidget);
    seekLabel->setObjectName(QString::fromUtf8("seekLabel"));

    hboxLayout1->addWidget(seekLabel);


    gridLayout->addLayout(hboxLayout1, 1, 0, 1, 1);

    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    pathBox = new QLineEdit(centralwidget);
    pathBox->setObjectName(QString::fromUtf8("pathBox"));

    hboxLayout2->addWidget(pathBox);

    changeButton = new QPushButton(centralwidget);
    changeButton->setObjectName(QString::fromUtf8("changeButton"));

    hboxLayout2->addWidget(changeButton);


    gridLayout->addLayout(hboxLayout2, 2, 0, 1, 1);

    artistLabel = new QLabel(centralwidget);
    artistLabel->setObjectName(QString::fromUtf8("artistLabel"));

    gridLayout->addWidget(artistLabel, 3, 0, 1, 1);

    albumLabel = new QLabel(centralwidget);
    albumLabel->setObjectName(QString::fromUtf8("albumLabel"));

    gridLayout->addWidget(albumLabel, 4, 0, 1, 1);

    titleLabel = new QLabel(centralwidget);
    titleLabel->setObjectName(QString::fromUtf8("titleLabel"));

    gridLayout->addWidget(titleLabel, 5, 0, 1, 1);

    seekBox = new QLineEdit(centralwidget);
    seekBox->setObjectName(QString::fromUtf8("seekBox"));

    gridLayout->addWidget(seekBox, 6, 0, 1, 1);

    MainWindow->setCentralWidget(centralwidget);

    retranslateUi(MainWindow);

    QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
    MainWindow->setWindowTitle(QApplication::translate("MainWindow", "n00bar", 0, QApplication::UnicodeUTF8));
    playButton->setText(QApplication::translate("MainWindow", "Play/Pause", 0, QApplication::UnicodeUTF8));
    stopButton->setText(QApplication::translate("MainWindow", "Stop", 0, QApplication::UnicodeUTF8));
    volumeLabel->setText(QApplication::translate("MainWindow", "Volume", 0, QApplication::UnicodeUTF8));
    id3Art->setText(QString());
    seekLabel->setText(QApplication::translate("MainWindow", "0:0", 0, QApplication::UnicodeUTF8));
    changeButton->setText(QApplication::translate("MainWindow", "Browse", 0, QApplication::UnicodeUTF8));
    artistLabel->setText(QString());
    albumLabel->setText(QString());
    titleLabel->setText(QString());
    Q_UNUSED(MainWindow);
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UNTITLED_H
