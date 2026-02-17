/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SmartResearchMainWindow
{
public:
    QWidget *SR_centralwidget;
    QHBoxLayout *SR_horizontalLayoutMain;
    QFrame *SR_sidebarFrame;
    QVBoxLayout *SR_verticalLayoutSidebar;
    QFrame *SR_logoFrame;
    QVBoxLayout *SR_verticalLayoutLogo;
    QLabel *SR_logoLabel;
    QVBoxLayout *SR_verticalLayoutNav;
    QPushButton *SR_btnPublications;
    QPushButton *SR_btnChercheurs;
    QPushButton *SR_btnLaboratoires;
    QPushButton *SR_btnProjets;
    QPushButton *SR_btnFinances;
    QPushButton *SR_btnEvenements;
    QSpacerItem *SR_verticalSpacerSidebar;
    QVBoxLayout *SR_verticalLayoutMainContent;
    QFrame *SR_headerFrame;
    QHBoxLayout *SR_horizontalLayoutHeader;
    QVBoxLayout *SR_verticalLayoutTitle;
    QLabel *SR_titleLabel;
    QLabel *SR_subtitleLabel;
    QSpacerItem *SR_horizontalSpacerHeader;
    QFrame *SR_userFrame;
    QHBoxLayout *SR_horizontalLayoutUser;
    QLabel *SR_userAvatar;
    QLabel *SR_userName;
    QFrame *SR_toolbarFrame;
    QHBoxLayout *SR_horizontalLayoutToolbar;
    QFrame *SR_tabsFrame;
    QHBoxLayout *SR_horizontalLayoutTabs;
    QPushButton *SR_btnVueListe;
    QPushButton *SR_btnAjouter;
    QPushButton *SR_btnSupprimer;
    QSpacerItem *SR_horizontalSpacerToolbar;
    QLineEdit *SR_lineEditRecherche;
    QPushButton *SR_btnRecherche;
    QPushButton *SR_btnTri;
    QPushButton *SR_btnExport;
    QPushButton *SR_btnStatistiques;
    QStackedWidget *SR_stackedWidget;
    QWidget *SR_pageListe;
    QVBoxLayout *SR_verticalLayoutListe;
    QFrame *SR_tableFrame;
    QVBoxLayout *SR_tableLayout;
    QLabel *SR_lblListeTitre;
    QTableWidget *SR_tablePublications;
    QWidget *SR_pageAjout;
    QVBoxLayout *SR_verticalLayoutAjout;
    QFrame *SR_formFrame;
    QVBoxLayout *SR_verticalLayoutForm;
    QVBoxLayout *SR_verticalLayoutFormTitle;
    QLabel *SR_formTitle;
    QLabel *SR_formSubtitle;
    QFrame *SR_lineFrame;
    QGridLayout *SR_gridLayoutInputs;
    QVBoxLayout *SR_verticalLayoutTitre;
    QLabel *SR_labelTitre;
    QLineEdit *SR_lineEditTitre;
    QVBoxLayout *SR_verticalLayoutAuteurs;
    QLabel *SR_labelAuteurs;
    QLineEdit *SR_lineEditAuteurs;
    QVBoxLayout *SR_verticalLayoutRevue;
    QLabel *SR_labelRevue;
    QLineEdit *SR_lineEditRevue;
    QVBoxLayout *SR_verticalLayoutDate;
    QLabel *SR_labelDate;
    QDateEdit *SR_dateEditPublication;
    QVBoxLayout *SR_verticalLayoutStatut;
    QLabel *SR_labelStatut;
    QComboBox *SR_comboBoxStatut;
    QHBoxLayout *SR_horizontalLayoutFormButtons;
    QSpacerItem *SR_horizontalSpacerForm;
    QPushButton *SR_btnAnnulerAjout;
    QPushButton *SR_btnAjouterPublication;
    QWidget *SR_pageStats;
    QVBoxLayout *SR_verticalLayoutStats;
    QHBoxLayout *SR_horizontalLayoutStatCards;
    QFrame *SR_cardTotal;
    QHBoxLayout *SR_horizontalLayoutCardTotal;
    QFrame *SR_iconFrameTotal;
    QVBoxLayout *SR_verticalLayoutIconTotal;
    QLabel *SR_lblTotalIcon;
    QVBoxLayout *SR_verticalLayoutTotalText;
    QLabel *SR_lblTotalNumber;
    QLabel *SR_lblTotalLabel;
    QSpacerItem *SR_horizontalSpacerCardTotal;
    QFrame *SR_cardThisYear;
    QHBoxLayout *SR_horizontalLayoutCardThisYear;
    QFrame *SR_iconFrameThisYear;
    QVBoxLayout *SR_verticalLayoutIconThisYear;
    QLabel *SR_lblThisYearIcon;
    QVBoxLayout *SR_verticalLayoutThisYearText;
    QLabel *SR_lblThisYearNumber;
    QLabel *SR_lblThisYearLabel;
    QSpacerItem *SR_horizontalSpacerCardThisYear;
    QFrame *SR_cardPlanS;
    QHBoxLayout *SR_horizontalLayoutCardPlanS;
    QFrame *SR_iconFramePlanS;
    QVBoxLayout *SR_verticalLayoutIconPlanS;
    QLabel *SR_lblPlanSIcon;
    QVBoxLayout *SR_verticalLayoutPlanSText;
    QLabel *SR_lblPlanSNumber;
    QLabel *SR_lblPlanSLabel;
    QSpacerItem *SR_horizontalSpacerCardPlanS;
    QFrame *SR_chartsContainerFrame;
    QHBoxLayout *SR_horizontalLayoutChartsContainer;
    QFrame *SR_barChartFrame;
    QVBoxLayout *SR_verticalLayoutBarChart;
    QLabel *SR_lblBarChartTitle;
    QWidget *SR_placeholderBars;
    QHBoxLayout *SR_barsLayout;
    QVBoxLayout *SR_bar1Layout;
    QFrame *SR_bar1;
    QLabel *SR_lblBar1;
    QVBoxLayout *SR_bar2Layout;
    QFrame *SR_bar2;
    QLabel *SR_lblBar2;
    QVBoxLayout *SR_bar3Layout;
    QFrame *SR_bar3;
    QLabel *SR_lblBar3;
    QVBoxLayout *SR_bar4Layout;
    QFrame *SR_bar4;
    QLabel *SR_lblBar4;
    QFrame *SR_pieChartFrame;
    QVBoxLayout *SR_verticalLayoutPieChart;
    QLabel *SR_lblPieChartTitle;
    QHBoxLayout *SR_horizontalLayoutPieContent;
    QFrame *SR_pieChartCircle;
    QVBoxLayout *SR_verticalLayoutLegend;
    QLabel *SR_lblStatPublie;
    QLabel *SR_lblStatSoumis;
    QLabel *SR_lblStatRevision;
    QLabel *SR_lblStatAccepte;
    QSpacerItem *SR_verticalSpacerLegend;

    void setupUi(QMainWindow *SmartResearchMainWindow)
    {
        if (SmartResearchMainWindow->objectName().isEmpty())
            SmartResearchMainWindow->setObjectName("SmartResearchMainWindow");
        SmartResearchMainWindow->resize(1612, 900);
        SmartResearchMainWindow->setStyleSheet(QString::fromUtf8("background-color: #f1f5f9;"));
        SR_centralwidget = new QWidget(SmartResearchMainWindow);
        SR_centralwidget->setObjectName("SR_centralwidget");
        SR_horizontalLayoutMain = new QHBoxLayout(SR_centralwidget);
        SR_horizontalLayoutMain->setSpacing(0);
        SR_horizontalLayoutMain->setObjectName("SR_horizontalLayoutMain");
        SR_horizontalLayoutMain->setContentsMargins(0, 0, 0, 0);
        SR_sidebarFrame = new QFrame(SR_centralwidget);
        SR_sidebarFrame->setObjectName("SR_sidebarFrame");
        SR_sidebarFrame->setMinimumSize(QSize(260, 0));
        SR_sidebarFrame->setMaximumSize(QSize(260, 16777215));
        SR_sidebarFrame->setStyleSheet(QString::fromUtf8("background-color: #1e293b;"));
        SR_sidebarFrame->setFrameShape(QFrame::Shape::NoFrame);
        SR_verticalLayoutSidebar = new QVBoxLayout(SR_sidebarFrame);
        SR_verticalLayoutSidebar->setSpacing(0);
        SR_verticalLayoutSidebar->setObjectName("SR_verticalLayoutSidebar");
        SR_verticalLayoutSidebar->setContentsMargins(0, 0, 0, 0);
        SR_logoFrame = new QFrame(SR_sidebarFrame);
        SR_logoFrame->setObjectName("SR_logoFrame");
        SR_logoFrame->setMinimumSize(QSize(0, 100));
        SR_logoFrame->setMaximumSize(QSize(16777215, 100));
        SR_logoFrame->setStyleSheet(QString::fromUtf8("background-color: #0f172a;"));
        SR_logoFrame->setFrameShape(QFrame::Shape::NoFrame);
        SR_verticalLayoutLogo = new QVBoxLayout(SR_logoFrame);
        SR_verticalLayoutLogo->setSpacing(0);
        SR_verticalLayoutLogo->setObjectName("SR_verticalLayoutLogo");
        SR_verticalLayoutLogo->setContentsMargins(20, 0, 20, 0);
        SR_logoLabel = new QLabel(SR_logoFrame);
        SR_logoLabel->setObjectName("SR_logoLabel");
        SR_logoLabel->setMinimumSize(QSize(200, 60));
        SR_logoLabel->setMaximumSize(QSize(200, 60));
        SR_logoLabel->setStyleSheet(QString::fromUtf8("color: white;\n"
"font-size: 20px;\n"
"font-weight: 700;\n"
"background: transparent;\n"
"border: none;"));
        SR_logoLabel->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/logo.png-removebg-preview.png")));
        SR_logoLabel->setScaledContents(true);
        SR_logoLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        SR_verticalLayoutLogo->addWidget(SR_logoLabel);


        SR_verticalLayoutSidebar->addWidget(SR_logoFrame);

        SR_verticalLayoutNav = new QVBoxLayout();
        SR_verticalLayoutNav->setSpacing(8);
        SR_verticalLayoutNav->setObjectName("SR_verticalLayoutNav");
        SR_verticalLayoutNav->setContentsMargins(0, 20, 0, 20);
        SR_btnPublications = new QPushButton(SR_sidebarFrame);
        SR_btnPublications->setObjectName("SR_btnPublications");
        SR_btnPublications->setMinimumSize(QSize(0, 50));
        SR_btnPublications->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnPublications->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,\n"
"        stop:0 #3b82f6, stop:1 #10b981);\n"
"    color: white;\n"
"    border: none;\n"
"    border-radius: 12px;\n"
"    padding: 14px 20px;\n"
"    font-size: 14px;\n"
"    font-weight: 600;\n"
"    text-align: left;\n"
"    margin: 4px 12px;\n"
"}\n"
"QPushButton:hover {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,\n"
"        stop:0 #2563eb, stop:1 #059669);\n"
"}"));
        SR_btnPublications->setCheckable(true);
        SR_btnPublications->setChecked(true);
        SR_btnPublications->setAutoExclusive(true);

        SR_verticalLayoutNav->addWidget(SR_btnPublications);

        SR_btnChercheurs = new QPushButton(SR_sidebarFrame);
        SR_btnChercheurs->setObjectName("SR_btnChercheurs");
        SR_btnChercheurs->setMinimumSize(QSize(0, 50));
        SR_btnChercheurs->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnChercheurs->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: transparent;\n"
"    color: #94a3b8;\n"
"    border: none;\n"
"    border-radius: 12px;\n"
"    padding: 14px 20px;\n"
"    font-size: 14px;\n"
"    font-weight: 500;\n"
"    text-align: left;\n"
"    margin: 4px 12px;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #334155;\n"
"    color: #e2e8f0;\n"
"}"));
        SR_btnChercheurs->setCheckable(true);
        SR_btnChercheurs->setAutoExclusive(true);

        SR_verticalLayoutNav->addWidget(SR_btnChercheurs);

        SR_btnLaboratoires = new QPushButton(SR_sidebarFrame);
        SR_btnLaboratoires->setObjectName("SR_btnLaboratoires");
        SR_btnLaboratoires->setMinimumSize(QSize(0, 50));
        SR_btnLaboratoires->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnLaboratoires->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: transparent;\n"
"    color: #94a3b8;\n"
"    border: none;\n"
"    border-radius: 12px;\n"
"    padding: 14px 20px;\n"
"    font-size: 14px;\n"
"    font-weight: 500;\n"
"    text-align: left;\n"
"    margin: 4px 12px;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #334155;\n"
"    color: #e2e8f0;\n"
"}"));
        SR_btnLaboratoires->setCheckable(true);
        SR_btnLaboratoires->setAutoExclusive(true);

        SR_verticalLayoutNav->addWidget(SR_btnLaboratoires);

        SR_btnProjets = new QPushButton(SR_sidebarFrame);
        SR_btnProjets->setObjectName("SR_btnProjets");
        SR_btnProjets->setMinimumSize(QSize(0, 50));
        SR_btnProjets->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnProjets->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: transparent;\n"
"    color: #94a3b8;\n"
"    border: none;\n"
"    border-radius: 12px;\n"
"    padding: 14px 20px;\n"
"    font-size: 14px;\n"
"    font-weight: 500;\n"
"    text-align: left;\n"
"    margin: 4px 12px;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #334155;\n"
"    color: #e2e8f0;\n"
"}"));
        SR_btnProjets->setCheckable(true);
        SR_btnProjets->setAutoExclusive(true);

        SR_verticalLayoutNav->addWidget(SR_btnProjets);

        SR_btnFinances = new QPushButton(SR_sidebarFrame);
        SR_btnFinances->setObjectName("SR_btnFinances");
        SR_btnFinances->setMinimumSize(QSize(0, 50));
        SR_btnFinances->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnFinances->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: transparent;\n"
"    color: #94a3b8;\n"
"    border: none;\n"
"    border-radius: 12px;\n"
"    padding: 14px 20px;\n"
"    font-size: 14px;\n"
"    font-weight: 500;\n"
"    text-align: left;\n"
"    margin: 4px 12px;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #334155;\n"
"    color: #e2e8f0;\n"
"}"));
        SR_btnFinances->setCheckable(true);
        SR_btnFinances->setAutoExclusive(true);

        SR_verticalLayoutNav->addWidget(SR_btnFinances);

        SR_btnEvenements = new QPushButton(SR_sidebarFrame);
        SR_btnEvenements->setObjectName("SR_btnEvenements");
        SR_btnEvenements->setMinimumSize(QSize(0, 50));
        SR_btnEvenements->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnEvenements->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: transparent;\n"
"    color: #94a3b8;\n"
"    border: none;\n"
"    border-radius: 12px;\n"
"    padding: 14px 20px;\n"
"    font-size: 14px;\n"
"    font-weight: 500;\n"
"    text-align: left;\n"
"    margin: 4px 12px;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #334155;\n"
"    color: #e2e8f0;\n"
"}"));
        SR_btnEvenements->setCheckable(true);
        SR_btnEvenements->setAutoExclusive(true);

        SR_verticalLayoutNav->addWidget(SR_btnEvenements);

        SR_verticalSpacerSidebar = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        SR_verticalLayoutNav->addItem(SR_verticalSpacerSidebar);


        SR_verticalLayoutSidebar->addLayout(SR_verticalLayoutNav);


        SR_horizontalLayoutMain->addWidget(SR_sidebarFrame);

        SR_verticalLayoutMainContent = new QVBoxLayout();
        SR_verticalLayoutMainContent->setSpacing(0);
        SR_verticalLayoutMainContent->setObjectName("SR_verticalLayoutMainContent");
        SR_verticalLayoutMainContent->setContentsMargins(0, 0, 0, 0);
        SR_headerFrame = new QFrame(SR_centralwidget);
        SR_headerFrame->setObjectName("SR_headerFrame");
        SR_headerFrame->setMinimumSize(QSize(0, 90));
        SR_headerFrame->setMaximumSize(QSize(16777215, 90));
        SR_headerFrame->setStyleSheet(QString::fromUtf8("background-color: white;\n"
"border-bottom: 1px solid #e2e8f0;"));
        SR_headerFrame->setFrameShape(QFrame::Shape::NoFrame);
        SR_horizontalLayoutHeader = new QHBoxLayout(SR_headerFrame);
        SR_horizontalLayoutHeader->setSpacing(20);
        SR_horizontalLayoutHeader->setObjectName("SR_horizontalLayoutHeader");
        SR_horizontalLayoutHeader->setContentsMargins(30, 0, 30, 0);
        SR_verticalLayoutTitle = new QVBoxLayout();
        SR_verticalLayoutTitle->setSpacing(4);
        SR_verticalLayoutTitle->setObjectName("SR_verticalLayoutTitle");
        SR_titleLabel = new QLabel(SR_headerFrame);
        SR_titleLabel->setObjectName("SR_titleLabel");
        SR_titleLabel->setStyleSheet(QString::fromUtf8("color: #1e293b;\n"
"font-size: 28px;\n"
"font-weight: 700;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutTitle->addWidget(SR_titleLabel);

        SR_subtitleLabel = new QLabel(SR_headerFrame);
        SR_subtitleLabel->setObjectName("SR_subtitleLabel");
        SR_subtitleLabel->setStyleSheet(QString::fromUtf8("color: #64748b;\n"
"font-size: 14px;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutTitle->addWidget(SR_subtitleLabel);


        SR_horizontalLayoutHeader->addLayout(SR_verticalLayoutTitle);

        SR_horizontalSpacerHeader = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        SR_horizontalLayoutHeader->addItem(SR_horizontalSpacerHeader);

        SR_userFrame = new QFrame(SR_headerFrame);
        SR_userFrame->setObjectName("SR_userFrame");
        SR_userFrame->setMinimumSize(QSize(180, 50));
        SR_userFrame->setMaximumSize(QSize(180, 50));
        SR_userFrame->setStyleSheet(QString::fromUtf8("background-color: #f8fafc;\n"
"border-radius: 25px;\n"
"border: 1px solid #e2e8f0;"));
        SR_userFrame->setFrameShape(QFrame::Shape::NoFrame);
        SR_horizontalLayoutUser = new QHBoxLayout(SR_userFrame);
        SR_horizontalLayoutUser->setSpacing(10);
        SR_horizontalLayoutUser->setObjectName("SR_horizontalLayoutUser");
        SR_horizontalLayoutUser->setContentsMargins(10, 5, 10, 5);
        SR_userAvatar = new QLabel(SR_userFrame);
        SR_userAvatar->setObjectName("SR_userAvatar");
        SR_userAvatar->setMinimumSize(QSize(36, 36));
        SR_userAvatar->setMaximumSize(QSize(36, 36));
        SR_userAvatar->setStyleSheet(QString::fromUtf8("background-color: #3b82f6;\n"
"border-radius: 18px;\n"
"color: white;\n"
"font-weight: bold;\n"
"font-size: 14px;\n"
"border: none;"));
        SR_userAvatar->setAlignment(Qt::AlignmentFlag::AlignCenter);

        SR_horizontalLayoutUser->addWidget(SR_userAvatar);

        SR_userName = new QLabel(SR_userFrame);
        SR_userName->setObjectName("SR_userName");
        SR_userName->setStyleSheet(QString::fromUtf8("color: #334155;\n"
"font-size: 13px;\n"
"font-weight: 600;\n"
"background: transparent;\n"
"border: none;"));

        SR_horizontalLayoutUser->addWidget(SR_userName);


        SR_horizontalLayoutHeader->addWidget(SR_userFrame);


        SR_verticalLayoutMainContent->addWidget(SR_headerFrame);

        SR_toolbarFrame = new QFrame(SR_centralwidget);
        SR_toolbarFrame->setObjectName("SR_toolbarFrame");
        SR_toolbarFrame->setMinimumSize(QSize(0, 100));
        SR_toolbarFrame->setMaximumSize(QSize(16777215, 100));
        SR_toolbarFrame->setStyleSheet(QString::fromUtf8("background-color: transparent;\n"
"border: none;"));
        SR_toolbarFrame->setFrameShape(QFrame::Shape::NoFrame);
        SR_horizontalLayoutToolbar = new QHBoxLayout(SR_toolbarFrame);
        SR_horizontalLayoutToolbar->setSpacing(15);
        SR_horizontalLayoutToolbar->setObjectName("SR_horizontalLayoutToolbar");
        SR_horizontalLayoutToolbar->setContentsMargins(30, 20, 30, 20);
        SR_tabsFrame = new QFrame(SR_toolbarFrame);
        SR_tabsFrame->setObjectName("SR_tabsFrame");
        SR_tabsFrame->setMinimumSize(QSize(360, 50));
        SR_tabsFrame->setMaximumSize(QSize(360, 50));
        SR_tabsFrame->setStyleSheet(QString::fromUtf8("background-color: white;\n"
"border-radius: 12px;\n"
"border: 1px solid #e2e8f0;"));
        SR_tabsFrame->setFrameShape(QFrame::Shape::NoFrame);
        SR_horizontalLayoutTabs = new QHBoxLayout(SR_tabsFrame);
        SR_horizontalLayoutTabs->setSpacing(0);
        SR_horizontalLayoutTabs->setObjectName("SR_horizontalLayoutTabs");
        SR_horizontalLayoutTabs->setContentsMargins(6, 6, 6, 6);
        SR_btnVueListe = new QPushButton(SR_tabsFrame);
        SR_btnVueListe->setObjectName("SR_btnVueListe");
        SR_btnVueListe->setMinimumSize(QSize(110, 36));
        SR_btnVueListe->setMaximumSize(QSize(110, 36));
        SR_btnVueListe->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnVueListe->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,\n"
"        stop:0 #3b82f6, stop:1 #10b981);\n"
"    color: white;\n"
"    border: none;\n"
"    border-radius: 8px;\n"
"    padding: 8px 16px;\n"
"    font-size: 13px;\n"
"    font-weight: 600;\n"
"}\n"
"QPushButton:hover {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,\n"
"        stop:0 #2563eb, stop:1 #059669);\n"
"}"));
        SR_btnVueListe->setCheckable(true);
        SR_btnVueListe->setChecked(true);

        SR_horizontalLayoutTabs->addWidget(SR_btnVueListe);

        SR_btnAjouter = new QPushButton(SR_tabsFrame);
        SR_btnAjouter->setObjectName("SR_btnAjouter");
        SR_btnAjouter->setMinimumSize(QSize(110, 36));
        SR_btnAjouter->setMaximumSize(QSize(110, 36));
        SR_btnAjouter->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnAjouter->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: transparent;\n"
"    color: #64748b;\n"
"    border: none;\n"
"    border-radius: 8px;\n"
"    padding: 8px 16px;\n"
"    font-size: 13px;\n"
"    font-weight: 500;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #f1f5f9;\n"
"    color: #334155;\n"
"}"));
        SR_btnAjouter->setCheckable(true);

        SR_horizontalLayoutTabs->addWidget(SR_btnAjouter);

        SR_btnSupprimer = new QPushButton(SR_tabsFrame);
        SR_btnSupprimer->setObjectName("SR_btnSupprimer");
        SR_btnSupprimer->setMinimumSize(QSize(110, 36));
        SR_btnSupprimer->setMaximumSize(QSize(110, 36));
        SR_btnSupprimer->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnSupprimer->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: transparent;\n"
"    color: #ef4444;\n"
"    border: none;\n"
"    border-radius: 8px;\n"
"    padding: 8px 16px;\n"
"    font-size: 13px;\n"
"    font-weight: 500;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #fef2f2;\n"
"    color: #dc2626;\n"
"}"));

        SR_horizontalLayoutTabs->addWidget(SR_btnSupprimer);


        SR_horizontalLayoutToolbar->addWidget(SR_tabsFrame);

        SR_horizontalSpacerToolbar = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        SR_horizontalLayoutToolbar->addItem(SR_horizontalSpacerToolbar);

        SR_lineEditRecherche = new QLineEdit(SR_toolbarFrame);
        SR_lineEditRecherche->setObjectName("SR_lineEditRecherche");
        SR_lineEditRecherche->setMinimumSize(QSize(280, 48));
        SR_lineEditRecherche->setMaximumSize(QSize(280, 48));
        SR_lineEditRecherche->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    background-color: white;\n"
"    border: 2px solid #e2e8f0;\n"
"    border-radius: 12px;\n"
"    padding: 12px 16px;\n"
"    font-size: 14px;\n"
"    color: #334155;\n"
"}\n"
"QLineEdit:focus {\n"
"    border-color: #3b82f6;\n"
"}"));

        SR_horizontalLayoutToolbar->addWidget(SR_lineEditRecherche);

        SR_btnRecherche = new QPushButton(SR_toolbarFrame);
        SR_btnRecherche->setObjectName("SR_btnRecherche");
        SR_btnRecherche->setMinimumSize(QSize(100, 48));
        SR_btnRecherche->setMaximumSize(QSize(100, 48));
        SR_btnRecherche->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnRecherche->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: #3b82f6;\n"
"    color: white;\n"
"    border: none;\n"
"    border-radius: 12px;\n"
"    padding: 12px 16px;\n"
"    font-size: 13px;\n"
"    font-weight: 600;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #2563eb;\n"
"}"));

        SR_horizontalLayoutToolbar->addWidget(SR_btnRecherche);

        SR_btnTri = new QPushButton(SR_toolbarFrame);
        SR_btnTri->setObjectName("SR_btnTri");
        SR_btnTri->setMinimumSize(QSize(90, 48));
        SR_btnTri->setMaximumSize(QSize(90, 48));
        SR_btnTri->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnTri->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: white;\n"
"    color: #334155;\n"
"    border: 2px solid #e2e8f0;\n"
"    border-radius: 12px;\n"
"    padding: 12px 16px;\n"
"    font-size: 13px;\n"
"    font-weight: 600;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #f8fafc;\n"
"    border-color: #cbd5e1;\n"
"}"));

        SR_horizontalLayoutToolbar->addWidget(SR_btnTri);

        SR_btnExport = new QPushButton(SR_toolbarFrame);
        SR_btnExport->setObjectName("SR_btnExport");
        SR_btnExport->setMinimumSize(QSize(100, 48));
        SR_btnExport->setMaximumSize(QSize(100, 48));
        SR_btnExport->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnExport->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: white;\n"
"    color: #334155;\n"
"    border: 2px solid #e2e8f0;\n"
"    border-radius: 12px;\n"
"    padding: 12px 16px;\n"
"    font-size: 13px;\n"
"    font-weight: 600;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #f8fafc;\n"
"    border-color: #cbd5e1;\n"
"}"));

        SR_horizontalLayoutToolbar->addWidget(SR_btnExport);

        SR_btnStatistiques = new QPushButton(SR_toolbarFrame);
        SR_btnStatistiques->setObjectName("SR_btnStatistiques");
        SR_btnStatistiques->setMinimumSize(QSize(120, 48));
        SR_btnStatistiques->setMaximumSize(QSize(120, 48));
        SR_btnStatistiques->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnStatistiques->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,\n"
"        stop:0 #3b82f6, stop:1 #10b981);\n"
"    color: white;\n"
"    border: none;\n"
"    border-radius: 12px;\n"
"    padding: 12px 16px;\n"
"    font-size: 13px;\n"
"    font-weight: 600;\n"
"}\n"
"QPushButton:hover {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,\n"
"        stop:0 #2563eb, stop:1 #059669);\n"
"}"));

        SR_horizontalLayoutToolbar->addWidget(SR_btnStatistiques);


        SR_verticalLayoutMainContent->addWidget(SR_toolbarFrame);

        SR_stackedWidget = new QStackedWidget(SR_centralwidget);
        SR_stackedWidget->setObjectName("SR_stackedWidget");
        SR_stackedWidget->setStyleSheet(QString::fromUtf8("background-color: transparent;"));
        SR_pageListe = new QWidget();
        SR_pageListe->setObjectName("SR_pageListe");
        SR_verticalLayoutListe = new QVBoxLayout(SR_pageListe);
        SR_verticalLayoutListe->setSpacing(0);
        SR_verticalLayoutListe->setObjectName("SR_verticalLayoutListe");
        SR_verticalLayoutListe->setContentsMargins(30, 0, 30, 30);
        SR_tableFrame = new QFrame(SR_pageListe);
        SR_tableFrame->setObjectName("SR_tableFrame");
        SR_tableFrame->setStyleSheet(QString::fromUtf8("background-color: white;\n"
"border: 1px solid #e2e8f0;\n"
"border-radius: 16px;"));
        SR_tableFrame->setFrameShape(QFrame::Shape::NoFrame);
        SR_tableLayout = new QVBoxLayout(SR_tableFrame);
        SR_tableLayout->setSpacing(20);
        SR_tableLayout->setObjectName("SR_tableLayout");
        SR_tableLayout->setContentsMargins(25, 25, 25, 25);
        SR_lblListeTitre = new QLabel(SR_tableFrame);
        SR_lblListeTitre->setObjectName("SR_lblListeTitre");
        SR_lblListeTitre->setStyleSheet(QString::fromUtf8("font-size: 20px;\n"
"font-weight: bold;\n"
"color: #1e293b;\n"
"background-color: transparent;\n"
"border: none;"));

        SR_tableLayout->addWidget(SR_lblListeTitre);

        SR_tablePublications = new QTableWidget(SR_tableFrame);
        if (SR_tablePublications->columnCount() < 6)
            SR_tablePublications->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        SR_tablePublications->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        SR_tablePublications->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        SR_tablePublications->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        SR_tablePublications->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        SR_tablePublications->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        SR_tablePublications->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        SR_tablePublications->setObjectName("SR_tablePublications");
        SR_tablePublications->setStyleSheet(QString::fromUtf8("QTableWidget {\n"
"    background-color: white;\n"
"    border: none;\n"
"    border-radius: 12px;\n"
"    gridline-color: #f1f5f9;\n"
"    font-size: 13px;\n"
"    outline: none;\n"
"}\n"
"QHeaderView::section {\n"
"    background-color: #f8fafc;\n"
"    padding: 16px;\n"
"    border: none;\n"
"    border-bottom: 2px solid #e2e8f0;\n"
"    font-weight: 600;\n"
"    color: #475569;\n"
"    font-size: 12px;\n"
"    text-transform: uppercase;\n"
"}\n"
"QTableWidget::item {\n"
"    padding: 16px;\n"
"    color: #334155;\n"
"    border-bottom: 1px solid #f1f5f9;\n"
"}\n"
"QTableWidget::item:selected {\n"
"    background-color: #dbeafe;\n"
"    color: #1e40af;\n"
"}\n"
"QTableWidget::item:hover {\n"
"    background-color: #f8fafc;\n"
"}"));
        SR_tablePublications->setAlternatingRowColors(false);
        SR_tablePublications->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
        SR_tablePublications->setShowGrid(false);
        SR_tablePublications->setColumnCount(6);
        SR_tablePublications->horizontalHeader()->setDefaultSectionSize(200);
        SR_tablePublications->horizontalHeader()->setStretchLastSection(true);
        SR_tablePublications->verticalHeader()->setVisible(false);

        SR_tableLayout->addWidget(SR_tablePublications);


        SR_verticalLayoutListe->addWidget(SR_tableFrame);

        SR_stackedWidget->addWidget(SR_pageListe);
        SR_pageAjout = new QWidget();
        SR_pageAjout->setObjectName("SR_pageAjout");
        SR_verticalLayoutAjout = new QVBoxLayout(SR_pageAjout);
        SR_verticalLayoutAjout->setSpacing(0);
        SR_verticalLayoutAjout->setObjectName("SR_verticalLayoutAjout");
        SR_verticalLayoutAjout->setContentsMargins(30, 0, 30, 30);
        SR_formFrame = new QFrame(SR_pageAjout);
        SR_formFrame->setObjectName("SR_formFrame");
        SR_formFrame->setStyleSheet(QString::fromUtf8("background-color: white;\n"
"border-radius: 20px;\n"
"border: 1px solid #e2e8f0;"));
        SR_formFrame->setFrameShape(QFrame::Shape::NoFrame);
        SR_verticalLayoutForm = new QVBoxLayout(SR_formFrame);
        SR_verticalLayoutForm->setSpacing(25);
        SR_verticalLayoutForm->setObjectName("SR_verticalLayoutForm");
        SR_verticalLayoutForm->setContentsMargins(50, 40, 50, 40);
        SR_verticalLayoutFormTitle = new QVBoxLayout();
        SR_verticalLayoutFormTitle->setSpacing(8);
        SR_verticalLayoutFormTitle->setObjectName("SR_verticalLayoutFormTitle");
        SR_formTitle = new QLabel(SR_formFrame);
        SR_formTitle->setObjectName("SR_formTitle");
        SR_formTitle->setStyleSheet(QString::fromUtf8("color: #1e293b;\n"
"font-size: 24px;\n"
"font-weight: 700;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutFormTitle->addWidget(SR_formTitle);

        SR_formSubtitle = new QLabel(SR_formFrame);
        SR_formSubtitle->setObjectName("SR_formSubtitle");
        SR_formSubtitle->setStyleSheet(QString::fromUtf8("color: #64748b;\n"
"font-size: 14px;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutFormTitle->addWidget(SR_formSubtitle);


        SR_verticalLayoutForm->addLayout(SR_verticalLayoutFormTitle);

        SR_lineFrame = new QFrame(SR_formFrame);
        SR_lineFrame->setObjectName("SR_lineFrame");
        SR_lineFrame->setMinimumSize(QSize(0, 1));
        SR_lineFrame->setMaximumSize(QSize(16777215, 1));
        SR_lineFrame->setStyleSheet(QString::fromUtf8("background-color: #e2e8f0;\n"
"border: none;"));
        SR_lineFrame->setFrameShape(QFrame::Shape::NoFrame);

        SR_verticalLayoutForm->addWidget(SR_lineFrame);

        SR_gridLayoutInputs = new QGridLayout();
        SR_gridLayoutInputs->setObjectName("SR_gridLayoutInputs");
        SR_gridLayoutInputs->setHorizontalSpacing(30);
        SR_gridLayoutInputs->setVerticalSpacing(20);
        SR_verticalLayoutTitre = new QVBoxLayout();
        SR_verticalLayoutTitre->setSpacing(8);
        SR_verticalLayoutTitre->setObjectName("SR_verticalLayoutTitre");
        SR_labelTitre = new QLabel(SR_formFrame);
        SR_labelTitre->setObjectName("SR_labelTitre");
        SR_labelTitre->setStyleSheet(QString::fromUtf8("color: #334155;\n"
"font-size: 14px;\n"
"font-weight: 600;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutTitre->addWidget(SR_labelTitre);

        SR_lineEditTitre = new QLineEdit(SR_formFrame);
        SR_lineEditTitre->setObjectName("SR_lineEditTitre");
        SR_lineEditTitre->setMinimumSize(QSize(0, 50));
        SR_lineEditTitre->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    background-color: #f8fafc;\n"
"    border: 2px solid #e2e8f0;\n"
"    border-radius: 12px;\n"
"    padding: 14px 16px;\n"
"    font-size: 14px;\n"
"    color: #334155;\n"
"}\n"
"QLineEdit:focus {\n"
"    border-color: #3b82f6;\n"
"    background-color: white;\n"
"}"));

        SR_verticalLayoutTitre->addWidget(SR_lineEditTitre);


        SR_gridLayoutInputs->addLayout(SR_verticalLayoutTitre, 0, 0, 1, 1);

        SR_verticalLayoutAuteurs = new QVBoxLayout();
        SR_verticalLayoutAuteurs->setSpacing(8);
        SR_verticalLayoutAuteurs->setObjectName("SR_verticalLayoutAuteurs");
        SR_labelAuteurs = new QLabel(SR_formFrame);
        SR_labelAuteurs->setObjectName("SR_labelAuteurs");
        SR_labelAuteurs->setStyleSheet(QString::fromUtf8("color: #334155;\n"
"font-size: 14px;\n"
"font-weight: 600;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutAuteurs->addWidget(SR_labelAuteurs);

        SR_lineEditAuteurs = new QLineEdit(SR_formFrame);
        SR_lineEditAuteurs->setObjectName("SR_lineEditAuteurs");
        SR_lineEditAuteurs->setMinimumSize(QSize(0, 50));
        SR_lineEditAuteurs->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    background-color: #f8fafc;\n"
"    border: 2px solid #e2e8f0;\n"
"    border-radius: 12px;\n"
"    padding: 14px 16px;\n"
"    font-size: 14px;\n"
"    color: #334155;\n"
"}\n"
"QLineEdit:focus {\n"
"    border-color: #3b82f6;\n"
"    background-color: white;\n"
"}"));

        SR_verticalLayoutAuteurs->addWidget(SR_lineEditAuteurs);


        SR_gridLayoutInputs->addLayout(SR_verticalLayoutAuteurs, 0, 1, 1, 1);

        SR_verticalLayoutRevue = new QVBoxLayout();
        SR_verticalLayoutRevue->setSpacing(8);
        SR_verticalLayoutRevue->setObjectName("SR_verticalLayoutRevue");
        SR_labelRevue = new QLabel(SR_formFrame);
        SR_labelRevue->setObjectName("SR_labelRevue");
        SR_labelRevue->setStyleSheet(QString::fromUtf8("color: #334155;\n"
"font-size: 14px;\n"
"font-weight: 600;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutRevue->addWidget(SR_labelRevue);

        SR_lineEditRevue = new QLineEdit(SR_formFrame);
        SR_lineEditRevue->setObjectName("SR_lineEditRevue");
        SR_lineEditRevue->setMinimumSize(QSize(0, 50));
        SR_lineEditRevue->setStyleSheet(QString::fromUtf8("QLineEdit {\n"
"    background-color: #f8fafc;\n"
"    border: 2px solid #e2e8f0;\n"
"    border-radius: 12px;\n"
"    padding: 14px 16px;\n"
"    font-size: 14px;\n"
"    color: #334155;\n"
"}\n"
"QLineEdit:focus {\n"
"    border-color: #3b82f6;\n"
"    background-color: white;\n"
"}"));

        SR_verticalLayoutRevue->addWidget(SR_lineEditRevue);


        SR_gridLayoutInputs->addLayout(SR_verticalLayoutRevue, 1, 0, 1, 1);

        SR_verticalLayoutDate = new QVBoxLayout();
        SR_verticalLayoutDate->setSpacing(8);
        SR_verticalLayoutDate->setObjectName("SR_verticalLayoutDate");
        SR_labelDate = new QLabel(SR_formFrame);
        SR_labelDate->setObjectName("SR_labelDate");
        SR_labelDate->setStyleSheet(QString::fromUtf8("color: #334155;\n"
"font-size: 14px;\n"
"font-weight: 600;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutDate->addWidget(SR_labelDate);

        SR_dateEditPublication = new QDateEdit(SR_formFrame);
        SR_dateEditPublication->setObjectName("SR_dateEditPublication");
        SR_dateEditPublication->setMinimumSize(QSize(0, 50));
        SR_dateEditPublication->setStyleSheet(QString::fromUtf8("QDateEdit {\n"
"    background-color: #f8fafc;\n"
"    border: 2px solid #e2e8f0;\n"
"    border-radius: 12px;\n"
"    padding: 14px 16px;\n"
"    font-size: 14px;\n"
"    color: #334155;\n"
"}\n"
"QDateEdit:focus {\n"
"    border-color: #3b82f6;\n"
"    background-color: white;\n"
"}\n"
"QDateEdit::drop-down {\n"
"    border: none;\n"
"    width: 30px;\n"
"}"));
        SR_dateEditPublication->setCalendarPopup(true);

        SR_verticalLayoutDate->addWidget(SR_dateEditPublication);


        SR_gridLayoutInputs->addLayout(SR_verticalLayoutDate, 1, 1, 1, 1);

        SR_verticalLayoutStatut = new QVBoxLayout();
        SR_verticalLayoutStatut->setSpacing(8);
        SR_verticalLayoutStatut->setObjectName("SR_verticalLayoutStatut");
        SR_labelStatut = new QLabel(SR_formFrame);
        SR_labelStatut->setObjectName("SR_labelStatut");
        SR_labelStatut->setStyleSheet(QString::fromUtf8("color: #334155;\n"
"font-size: 14px;\n"
"font-weight: 600;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutStatut->addWidget(SR_labelStatut);

        SR_comboBoxStatut = new QComboBox(SR_formFrame);
        SR_comboBoxStatut->addItem(QString());
        SR_comboBoxStatut->addItem(QString());
        SR_comboBoxStatut->addItem(QString());
        SR_comboBoxStatut->addItem(QString());
        SR_comboBoxStatut->addItem(QString());
        SR_comboBoxStatut->setObjectName("SR_comboBoxStatut");
        SR_comboBoxStatut->setMinimumSize(QSize(0, 50));
        SR_comboBoxStatut->setStyleSheet(QString::fromUtf8("QComboBox {\n"
"    background-color: #f8fafc;\n"
"    border: 2px solid #e2e8f0;\n"
"    border-radius: 12px;\n"
"    padding: 14px 16px;\n"
"    font-size: 14px;\n"
"    color: #334155;\n"
"}\n"
"QComboBox:focus {\n"
"    border-color: #3b82f6;\n"
"    background-color: white;\n"
"}\n"
"QComboBox::drop-down {\n"
"    border: none;\n"
"    width: 30px;\n"
"}\n"
"QComboBox QAbstractItemView {\n"
"    background-color: white;\n"
"    border: 1px solid #e2e8f0;\n"
"    border-radius: 8px;\n"
"    selection-background-color: #eff6ff;\n"
"    selection-color: #3b82f6;\n"
"    padding: 8px;\n"
"}"));

        SR_verticalLayoutStatut->addWidget(SR_comboBoxStatut);


        SR_gridLayoutInputs->addLayout(SR_verticalLayoutStatut, 2, 0, 1, 2);


        SR_verticalLayoutForm->addLayout(SR_gridLayoutInputs);

        SR_horizontalLayoutFormButtons = new QHBoxLayout();
        SR_horizontalLayoutFormButtons->setSpacing(15);
        SR_horizontalLayoutFormButtons->setObjectName("SR_horizontalLayoutFormButtons");
        SR_horizontalSpacerForm = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        SR_horizontalLayoutFormButtons->addItem(SR_horizontalSpacerForm);

        SR_btnAnnulerAjout = new QPushButton(SR_formFrame);
        SR_btnAnnulerAjout->setObjectName("SR_btnAnnulerAjout");
        SR_btnAnnulerAjout->setMinimumSize(QSize(140, 50));
        SR_btnAnnulerAjout->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnAnnulerAjout->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background-color: white;\n"
"    color: #64748b;\n"
"    border: 2px solid #e2e8f0;\n"
"    border-radius: 12px;\n"
"    padding: 14px 32px;\n"
"    font-size: 15px;\n"
"    font-weight: 600;\n"
"}\n"
"QPushButton:hover {\n"
"    background-color: #f1f5f9;\n"
"    border-color: #cbd5e1;\n"
"}"));

        SR_horizontalLayoutFormButtons->addWidget(SR_btnAnnulerAjout);

        SR_btnAjouterPublication = new QPushButton(SR_formFrame);
        SR_btnAjouterPublication->setObjectName("SR_btnAjouterPublication");
        SR_btnAjouterPublication->setMinimumSize(QSize(180, 50));
        SR_btnAjouterPublication->setCursor(QCursor(Qt::CursorShape::PointingHandCursor));
        SR_btnAjouterPublication->setStyleSheet(QString::fromUtf8("QPushButton {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,\n"
"        stop:0 #10b981, stop:1 #3b82f6);\n"
"    color: white;\n"
"    border: none;\n"
"    border-radius: 12px;\n"
"    padding: 14px 32px;\n"
"    font-size: 15px;\n"
"    font-weight: 600;\n"
"}\n"
"QPushButton:hover {\n"
"    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,\n"
"        stop:0 #059669, stop:1 #2563eb);\n"
"}"));

        SR_horizontalLayoutFormButtons->addWidget(SR_btnAjouterPublication);


        SR_verticalLayoutForm->addLayout(SR_horizontalLayoutFormButtons);


        SR_verticalLayoutAjout->addWidget(SR_formFrame);

        SR_stackedWidget->addWidget(SR_pageAjout);
        SR_pageStats = new QWidget();
        SR_pageStats->setObjectName("SR_pageStats");
        SR_verticalLayoutStats = new QVBoxLayout(SR_pageStats);
        SR_verticalLayoutStats->setSpacing(30);
        SR_verticalLayoutStats->setObjectName("SR_verticalLayoutStats");
        SR_verticalLayoutStats->setContentsMargins(30, 0, 30, 30);
        SR_horizontalLayoutStatCards = new QHBoxLayout();
        SR_horizontalLayoutStatCards->setSpacing(20);
        SR_horizontalLayoutStatCards->setObjectName("SR_horizontalLayoutStatCards");
        SR_cardTotal = new QFrame(SR_pageStats);
        SR_cardTotal->setObjectName("SR_cardTotal");
        SR_cardTotal->setMinimumSize(QSize(0, 140));
        SR_cardTotal->setStyleSheet(QString::fromUtf8("QFrame {\n"
"    background-color: white;\n"
"    border-radius: 16px;\n"
"    border: 1px solid #e2e8f0;\n"
"}"));
        SR_cardTotal->setFrameShape(QFrame::Shape::NoFrame);
        SR_horizontalLayoutCardTotal = new QHBoxLayout(SR_cardTotal);
        SR_horizontalLayoutCardTotal->setSpacing(20);
        SR_horizontalLayoutCardTotal->setObjectName("SR_horizontalLayoutCardTotal");
        SR_horizontalLayoutCardTotal->setContentsMargins(25, 20, 25, 20);
        SR_iconFrameTotal = new QFrame(SR_cardTotal);
        SR_iconFrameTotal->setObjectName("SR_iconFrameTotal");
        SR_iconFrameTotal->setMinimumSize(QSize(60, 60));
        SR_iconFrameTotal->setMaximumSize(QSize(60, 60));
        SR_iconFrameTotal->setStyleSheet(QString::fromUtf8("background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 #3b82f6, stop:1 #10b981);\n"
"border-radius: 12px;"));
        SR_iconFrameTotal->setFrameShape(QFrame::Shape::NoFrame);
        SR_verticalLayoutIconTotal = new QVBoxLayout(SR_iconFrameTotal);
        SR_verticalLayoutIconTotal->setContentsMargins(0, 0, 0, 0);
        SR_verticalLayoutIconTotal->setObjectName("SR_verticalLayoutIconTotal");
        SR_lblTotalIcon = new QLabel(SR_iconFrameTotal);
        SR_lblTotalIcon->setObjectName("SR_lblTotalIcon");
        SR_lblTotalIcon->setStyleSheet(QString::fromUtf8("color: white;\n"
"font-size: 28px;\n"
"background: transparent;\n"
"border: none;"));
        SR_lblTotalIcon->setAlignment(Qt::AlignmentFlag::AlignCenter);

        SR_verticalLayoutIconTotal->addWidget(SR_lblTotalIcon);


        SR_horizontalLayoutCardTotal->addWidget(SR_iconFrameTotal);

        SR_verticalLayoutTotalText = new QVBoxLayout();
        SR_verticalLayoutTotalText->setSpacing(5);
        SR_verticalLayoutTotalText->setObjectName("SR_verticalLayoutTotalText");
        SR_lblTotalNumber = new QLabel(SR_cardTotal);
        SR_lblTotalNumber->setObjectName("SR_lblTotalNumber");
        SR_lblTotalNumber->setStyleSheet(QString::fromUtf8("color: #1e293b;\n"
"font-size: 32px;\n"
"font-weight: bold;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutTotalText->addWidget(SR_lblTotalNumber);

        SR_lblTotalLabel = new QLabel(SR_cardTotal);
        SR_lblTotalLabel->setObjectName("SR_lblTotalLabel");
        SR_lblTotalLabel->setStyleSheet(QString::fromUtf8("color: #64748b;\n"
"font-size: 14px;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutTotalText->addWidget(SR_lblTotalLabel);


        SR_horizontalLayoutCardTotal->addLayout(SR_verticalLayoutTotalText);

        SR_horizontalSpacerCardTotal = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        SR_horizontalLayoutCardTotal->addItem(SR_horizontalSpacerCardTotal);


        SR_horizontalLayoutStatCards->addWidget(SR_cardTotal);

        SR_cardThisYear = new QFrame(SR_pageStats);
        SR_cardThisYear->setObjectName("SR_cardThisYear");
        SR_cardThisYear->setMinimumSize(QSize(0, 140));
        SR_cardThisYear->setStyleSheet(QString::fromUtf8("QFrame {\n"
"    background-color: white;\n"
"    border-radius: 16px;\n"
"    border: 1px solid #e2e8f0;\n"
"}"));
        SR_cardThisYear->setFrameShape(QFrame::Shape::NoFrame);
        SR_horizontalLayoutCardThisYear = new QHBoxLayout(SR_cardThisYear);
        SR_horizontalLayoutCardThisYear->setSpacing(20);
        SR_horizontalLayoutCardThisYear->setObjectName("SR_horizontalLayoutCardThisYear");
        SR_horizontalLayoutCardThisYear->setContentsMargins(25, 20, 25, 20);
        SR_iconFrameThisYear = new QFrame(SR_cardThisYear);
        SR_iconFrameThisYear->setObjectName("SR_iconFrameThisYear");
        SR_iconFrameThisYear->setMinimumSize(QSize(60, 60));
        SR_iconFrameThisYear->setMaximumSize(QSize(60, 60));
        SR_iconFrameThisYear->setStyleSheet(QString::fromUtf8("background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 #10b981, stop:1 #059669);\n"
"border-radius: 12px;"));
        SR_iconFrameThisYear->setFrameShape(QFrame::Shape::NoFrame);
        SR_verticalLayoutIconThisYear = new QVBoxLayout(SR_iconFrameThisYear);
        SR_verticalLayoutIconThisYear->setContentsMargins(0, 0, 0, 0);
        SR_verticalLayoutIconThisYear->setObjectName("SR_verticalLayoutIconThisYear");
        SR_lblThisYearIcon = new QLabel(SR_iconFrameThisYear);
        SR_lblThisYearIcon->setObjectName("SR_lblThisYearIcon");
        SR_lblThisYearIcon->setStyleSheet(QString::fromUtf8("color: white;\n"
"font-size: 28px;\n"
"background: transparent;\n"
"border: none;"));
        SR_lblThisYearIcon->setAlignment(Qt::AlignmentFlag::AlignCenter);

        SR_verticalLayoutIconThisYear->addWidget(SR_lblThisYearIcon);


        SR_horizontalLayoutCardThisYear->addWidget(SR_iconFrameThisYear);

        SR_verticalLayoutThisYearText = new QVBoxLayout();
        SR_verticalLayoutThisYearText->setSpacing(5);
        SR_verticalLayoutThisYearText->setObjectName("SR_verticalLayoutThisYearText");
        SR_lblThisYearNumber = new QLabel(SR_cardThisYear);
        SR_lblThisYearNumber->setObjectName("SR_lblThisYearNumber");
        SR_lblThisYearNumber->setStyleSheet(QString::fromUtf8("color: #1e293b;\n"
"font-size: 32px;\n"
"font-weight: bold;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutThisYearText->addWidget(SR_lblThisYearNumber);

        SR_lblThisYearLabel = new QLabel(SR_cardThisYear);
        SR_lblThisYearLabel->setObjectName("SR_lblThisYearLabel");
        SR_lblThisYearLabel->setStyleSheet(QString::fromUtf8("color: #64748b;\n"
"font-size: 14px;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutThisYearText->addWidget(SR_lblThisYearLabel);


        SR_horizontalLayoutCardThisYear->addLayout(SR_verticalLayoutThisYearText);

        SR_horizontalSpacerCardThisYear = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        SR_horizontalLayoutCardThisYear->addItem(SR_horizontalSpacerCardThisYear);


        SR_horizontalLayoutStatCards->addWidget(SR_cardThisYear);

        SR_cardPlanS = new QFrame(SR_pageStats);
        SR_cardPlanS->setObjectName("SR_cardPlanS");
        SR_cardPlanS->setMinimumSize(QSize(0, 140));
        SR_cardPlanS->setStyleSheet(QString::fromUtf8("QFrame {\n"
"    background-color: white;\n"
"    border-radius: 16px;\n"
"    border: 1px solid #e2e8f0;\n"
"}"));
        SR_cardPlanS->setFrameShape(QFrame::Shape::NoFrame);
        SR_horizontalLayoutCardPlanS = new QHBoxLayout(SR_cardPlanS);
        SR_horizontalLayoutCardPlanS->setSpacing(20);
        SR_horizontalLayoutCardPlanS->setObjectName("SR_horizontalLayoutCardPlanS");
        SR_horizontalLayoutCardPlanS->setContentsMargins(25, 20, 25, 20);
        SR_iconFramePlanS = new QFrame(SR_cardPlanS);
        SR_iconFramePlanS->setObjectName("SR_iconFramePlanS");
        SR_iconFramePlanS->setMinimumSize(QSize(60, 60));
        SR_iconFramePlanS->setMaximumSize(QSize(60, 60));
        SR_iconFramePlanS->setStyleSheet(QString::fromUtf8("background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\n"
"        stop:0 #f59e0b, stop:1 #d97706);\n"
"border-radius: 12px;"));
        SR_iconFramePlanS->setFrameShape(QFrame::Shape::NoFrame);
        SR_verticalLayoutIconPlanS = new QVBoxLayout(SR_iconFramePlanS);
        SR_verticalLayoutIconPlanS->setContentsMargins(0, 0, 0, 0);
        SR_verticalLayoutIconPlanS->setObjectName("SR_verticalLayoutIconPlanS");
        SR_lblPlanSIcon = new QLabel(SR_iconFramePlanS);
        SR_lblPlanSIcon->setObjectName("SR_lblPlanSIcon");
        SR_lblPlanSIcon->setStyleSheet(QString::fromUtf8("color: white;\n"
"font-size: 28px;\n"
"background: transparent;\n"
"border: none;"));
        SR_lblPlanSIcon->setAlignment(Qt::AlignmentFlag::AlignCenter);

        SR_verticalLayoutIconPlanS->addWidget(SR_lblPlanSIcon);


        SR_horizontalLayoutCardPlanS->addWidget(SR_iconFramePlanS);

        SR_verticalLayoutPlanSText = new QVBoxLayout();
        SR_verticalLayoutPlanSText->setSpacing(5);
        SR_verticalLayoutPlanSText->setObjectName("SR_verticalLayoutPlanSText");
        SR_lblPlanSNumber = new QLabel(SR_cardPlanS);
        SR_lblPlanSNumber->setObjectName("SR_lblPlanSNumber");
        SR_lblPlanSNumber->setStyleSheet(QString::fromUtf8("color: #1e293b;\n"
"font-size: 32px;\n"
"font-weight: bold;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutPlanSText->addWidget(SR_lblPlanSNumber);

        SR_lblPlanSLabel = new QLabel(SR_cardPlanS);
        SR_lblPlanSLabel->setObjectName("SR_lblPlanSLabel");
        SR_lblPlanSLabel->setStyleSheet(QString::fromUtf8("color: #64748b;\n"
"font-size: 14px;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutPlanSText->addWidget(SR_lblPlanSLabel);


        SR_horizontalLayoutCardPlanS->addLayout(SR_verticalLayoutPlanSText);

        SR_horizontalSpacerCardPlanS = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        SR_horizontalLayoutCardPlanS->addItem(SR_horizontalSpacerCardPlanS);


        SR_horizontalLayoutStatCards->addWidget(SR_cardPlanS);


        SR_verticalLayoutStats->addLayout(SR_horizontalLayoutStatCards);

        SR_chartsContainerFrame = new QFrame(SR_pageStats);
        SR_chartsContainerFrame->setObjectName("SR_chartsContainerFrame");
        SR_chartsContainerFrame->setStyleSheet(QString::fromUtf8("background-color: white;\n"
"border-radius: 20px;\n"
"border: 1px solid #e2e8f0;"));
        SR_chartsContainerFrame->setFrameShape(QFrame::Shape::NoFrame);
        SR_horizontalLayoutChartsContainer = new QHBoxLayout(SR_chartsContainerFrame);
        SR_horizontalLayoutChartsContainer->setSpacing(30);
        SR_horizontalLayoutChartsContainer->setObjectName("SR_horizontalLayoutChartsContainer");
        SR_horizontalLayoutChartsContainer->setContentsMargins(30, 30, 30, 30);
        SR_barChartFrame = new QFrame(SR_chartsContainerFrame);
        SR_barChartFrame->setObjectName("SR_barChartFrame");
        SR_barChartFrame->setMinimumSize(QSize(0, 0));
        SR_barChartFrame->setStyleSheet(QString::fromUtf8("background-color: #f8fafc;\n"
"border-radius: 16px;\n"
"border: 1px solid #e2e8f0;"));
        SR_barChartFrame->setFrameShape(QFrame::Shape::NoFrame);
        SR_verticalLayoutBarChart = new QVBoxLayout(SR_barChartFrame);
        SR_verticalLayoutBarChart->setSpacing(20);
        SR_verticalLayoutBarChart->setObjectName("SR_verticalLayoutBarChart");
        SR_verticalLayoutBarChart->setContentsMargins(25, 25, 25, 25);
        SR_lblBarChartTitle = new QLabel(SR_barChartFrame);
        SR_lblBarChartTitle->setObjectName("SR_lblBarChartTitle");
        SR_lblBarChartTitle->setStyleSheet(QString::fromUtf8("font-size: 16px;\n"
"font-weight: 600;\n"
"color: #1e293b;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutBarChart->addWidget(SR_lblBarChartTitle);

        SR_placeholderBars = new QWidget(SR_barChartFrame);
        SR_placeholderBars->setObjectName("SR_placeholderBars");
        SR_placeholderBars->setMinimumSize(QSize(0, 200));
        SR_placeholderBars->setStyleSheet(QString::fromUtf8("background-color: transparent;"));
        SR_barsLayout = new QHBoxLayout(SR_placeholderBars);
        SR_barsLayout->setSpacing(30);
        SR_barsLayout->setObjectName("SR_barsLayout");
        SR_bar1Layout = new QVBoxLayout();
        SR_bar1Layout->setObjectName("SR_bar1Layout");
        SR_bar1 = new QFrame(SR_placeholderBars);
        SR_bar1->setObjectName("SR_bar1");
        SR_bar1->setMinimumSize(QSize(50, 80));
        SR_bar1->setMaximumSize(QSize(50, 80));
        SR_bar1->setStyleSheet(QString::fromUtf8("background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"    stop:0 #3b82f6, stop:1 #1e40af);\n"
"border-radius: 6px 6px 0 0;"));
        SR_bar1->setFrameShape(QFrame::Shape::NoFrame);

        SR_bar1Layout->addWidget(SR_bar1);

        SR_lblBar1 = new QLabel(SR_placeholderBars);
        SR_lblBar1->setObjectName("SR_lblBar1");
        SR_lblBar1->setStyleSheet(QString::fromUtf8("font-size: 12px;\n"
"color: #64748b;\n"
"font-weight: 600;\n"
"background: transparent;\n"
"border: none;"));
        SR_lblBar1->setAlignment(Qt::AlignmentFlag::AlignCenter);

        SR_bar1Layout->addWidget(SR_lblBar1);


        SR_barsLayout->addLayout(SR_bar1Layout);

        SR_bar2Layout = new QVBoxLayout();
        SR_bar2Layout->setObjectName("SR_bar2Layout");
        SR_bar2 = new QFrame(SR_placeholderBars);
        SR_bar2->setObjectName("SR_bar2");
        SR_bar2->setMinimumSize(QSize(50, 100));
        SR_bar2->setMaximumSize(QSize(50, 100));
        SR_bar2->setStyleSheet(QString::fromUtf8("background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"    stop:0 #10b981, stop:1 #059669);\n"
"border-radius: 6px 6px 0 0;"));
        SR_bar2->setFrameShape(QFrame::Shape::NoFrame);

        SR_bar2Layout->addWidget(SR_bar2);

        SR_lblBar2 = new QLabel(SR_placeholderBars);
        SR_lblBar2->setObjectName("SR_lblBar2");
        SR_lblBar2->setStyleSheet(QString::fromUtf8("font-size: 12px;\n"
"color: #64748b;\n"
"font-weight: 600;\n"
"background: transparent;\n"
"border: none;"));
        SR_lblBar2->setAlignment(Qt::AlignmentFlag::AlignCenter);

        SR_bar2Layout->addWidget(SR_lblBar2);


        SR_barsLayout->addLayout(SR_bar2Layout);

        SR_bar3Layout = new QVBoxLayout();
        SR_bar3Layout->setObjectName("SR_bar3Layout");
        SR_bar3 = new QFrame(SR_placeholderBars);
        SR_bar3->setObjectName("SR_bar3");
        SR_bar3->setMinimumSize(QSize(50, 120));
        SR_bar3->setMaximumSize(QSize(50, 120));
        SR_bar3->setStyleSheet(QString::fromUtf8("background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"    stop:0 #f59e0b, stop:1 #d97706);\n"
"border-radius: 6px 6px 0 0;"));
        SR_bar3->setFrameShape(QFrame::Shape::NoFrame);

        SR_bar3Layout->addWidget(SR_bar3);

        SR_lblBar3 = new QLabel(SR_placeholderBars);
        SR_lblBar3->setObjectName("SR_lblBar3");
        SR_lblBar3->setStyleSheet(QString::fromUtf8("font-size: 12px;\n"
"color: #64748b;\n"
"font-weight: 600;\n"
"background: transparent;\n"
"border: none;"));
        SR_lblBar3->setAlignment(Qt::AlignmentFlag::AlignCenter);

        SR_bar3Layout->addWidget(SR_lblBar3);


        SR_barsLayout->addLayout(SR_bar3Layout);

        SR_bar4Layout = new QVBoxLayout();
        SR_bar4Layout->setObjectName("SR_bar4Layout");
        SR_bar4 = new QFrame(SR_placeholderBars);
        SR_bar4->setObjectName("SR_bar4");
        SR_bar4->setMinimumSize(QSize(50, 140));
        SR_bar4->setMaximumSize(QSize(50, 140));
        SR_bar4->setStyleSheet(QString::fromUtf8("background: qlineargradient(x1:0, y1:0, x2:0, y2:1,\n"
"    stop:0 #ec4899, stop:1 #db2777);\n"
"border-radius: 6px 6px 0 0;"));
        SR_bar4->setFrameShape(QFrame::Shape::NoFrame);

        SR_bar4Layout->addWidget(SR_bar4);

        SR_lblBar4 = new QLabel(SR_placeholderBars);
        SR_lblBar4->setObjectName("SR_lblBar4");
        SR_lblBar4->setStyleSheet(QString::fromUtf8("font-size: 12px;\n"
"color: #64748b;\n"
"font-weight: 600;\n"
"background: transparent;\n"
"border: none;"));
        SR_lblBar4->setAlignment(Qt::AlignmentFlag::AlignCenter);

        SR_bar4Layout->addWidget(SR_lblBar4);


        SR_barsLayout->addLayout(SR_bar4Layout);


        SR_verticalLayoutBarChart->addWidget(SR_placeholderBars);


        SR_horizontalLayoutChartsContainer->addWidget(SR_barChartFrame);

        SR_pieChartFrame = new QFrame(SR_chartsContainerFrame);
        SR_pieChartFrame->setObjectName("SR_pieChartFrame");
        SR_pieChartFrame->setMinimumSize(QSize(350, 0));
        SR_pieChartFrame->setMaximumSize(QSize(350, 16777215));
        SR_pieChartFrame->setStyleSheet(QString::fromUtf8("background-color: #f8fafc;\n"
"border-radius: 16px;\n"
"border: 1px solid #e2e8f0;"));
        SR_pieChartFrame->setFrameShape(QFrame::Shape::NoFrame);
        SR_verticalLayoutPieChart = new QVBoxLayout(SR_pieChartFrame);
        SR_verticalLayoutPieChart->setSpacing(20);
        SR_verticalLayoutPieChart->setObjectName("SR_verticalLayoutPieChart");
        SR_verticalLayoutPieChart->setContentsMargins(25, 25, 25, 25);
        SR_lblPieChartTitle = new QLabel(SR_pieChartFrame);
        SR_lblPieChartTitle->setObjectName("SR_lblPieChartTitle");
        SR_lblPieChartTitle->setStyleSheet(QString::fromUtf8("font-size: 16px;\n"
"font-weight: 600;\n"
"color: #1e293b;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutPieChart->addWidget(SR_lblPieChartTitle);

        SR_horizontalLayoutPieContent = new QHBoxLayout();
        SR_horizontalLayoutPieContent->setSpacing(20);
        SR_horizontalLayoutPieContent->setObjectName("SR_horizontalLayoutPieContent");
        SR_pieChartCircle = new QFrame(SR_pieChartFrame);
        SR_pieChartCircle->setObjectName("SR_pieChartCircle");
        SR_pieChartCircle->setMinimumSize(QSize(150, 150));
        SR_pieChartCircle->setMaximumSize(QSize(150, 150));
        SR_pieChartCircle->setStyleSheet(QString::fromUtf8("background: conic-gradient(\n"
"    #3b82f6 0deg 144deg,\n"
"    #10b981 144deg 234deg,\n"
"    #f59e0b 234deg 306deg,\n"
"    #8b5cf6 306deg 360deg\n"
");\n"
"border-radius: 75px;"));
        SR_pieChartCircle->setFrameShape(QFrame::Shape::NoFrame);

        SR_horizontalLayoutPieContent->addWidget(SR_pieChartCircle);

        SR_verticalLayoutLegend = new QVBoxLayout();
        SR_verticalLayoutLegend->setSpacing(12);
        SR_verticalLayoutLegend->setObjectName("SR_verticalLayoutLegend");
        SR_lblStatPublie = new QLabel(SR_pieChartFrame);
        SR_lblStatPublie->setObjectName("SR_lblStatPublie");
        SR_lblStatPublie->setStyleSheet(QString::fromUtf8("font-size: 13px;\n"
"color: #3b82f6;\n"
"font-weight: 600;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutLegend->addWidget(SR_lblStatPublie);

        SR_lblStatSoumis = new QLabel(SR_pieChartFrame);
        SR_lblStatSoumis->setObjectName("SR_lblStatSoumis");
        SR_lblStatSoumis->setStyleSheet(QString::fromUtf8("font-size: 13px;\n"
"color: #10b981;\n"
"font-weight: 600;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutLegend->addWidget(SR_lblStatSoumis);

        SR_lblStatRevision = new QLabel(SR_pieChartFrame);
        SR_lblStatRevision->setObjectName("SR_lblStatRevision");
        SR_lblStatRevision->setStyleSheet(QString::fromUtf8("font-size: 13px;\n"
"color: #f59e0b;\n"
"font-weight: 600;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutLegend->addWidget(SR_lblStatRevision);

        SR_lblStatAccepte = new QLabel(SR_pieChartFrame);
        SR_lblStatAccepte->setObjectName("SR_lblStatAccepte");
        SR_lblStatAccepte->setStyleSheet(QString::fromUtf8("font-size: 13px;\n"
"color: #8b5cf6;\n"
"font-weight: 600;\n"
"background: transparent;\n"
"border: none;"));

        SR_verticalLayoutLegend->addWidget(SR_lblStatAccepte);

        SR_verticalSpacerLegend = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        SR_verticalLayoutLegend->addItem(SR_verticalSpacerLegend);


        SR_horizontalLayoutPieContent->addLayout(SR_verticalLayoutLegend);


        SR_verticalLayoutPieChart->addLayout(SR_horizontalLayoutPieContent);


        SR_horizontalLayoutChartsContainer->addWidget(SR_pieChartFrame);


        SR_verticalLayoutStats->addWidget(SR_chartsContainerFrame);

        SR_stackedWidget->addWidget(SR_pageStats);

        SR_verticalLayoutMainContent->addWidget(SR_stackedWidget);


        SR_horizontalLayoutMain->addLayout(SR_verticalLayoutMainContent);

        SmartResearchMainWindow->setCentralWidget(SR_centralwidget);

        retranslateUi(SmartResearchMainWindow);

        QMetaObject::connectSlotsByName(SmartResearchMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *SmartResearchMainWindow)
    {
        SmartResearchMainWindow->setWindowTitle(QCoreApplication::translate("SmartResearchMainWindow", "Smart Research - Gestion des Publications", nullptr));
        SR_logoLabel->setText(QString());
        SR_btnPublications->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\223\204 Publications", nullptr));
        SR_btnChercheurs->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\221\245 Chercheurs", nullptr));
        SR_btnLaboratoires->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\247\252 Laboratoires", nullptr));
        SR_btnProjets->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\223\201 Projets", nullptr));
        SR_btnFinances->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\222\260 Finances", nullptr));
        SR_btnEvenements->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\223\205 Evenements", nullptr));
        SR_titleLabel->setText(QCoreApplication::translate("SmartResearchMainWindow", "Gestion des Publications", nullptr));
        SR_subtitleLabel->setText(QCoreApplication::translate("SmartResearchMainWindow", "G\303\251rez vos publications scientifiques et suivez les m\303\251triques de recherche", nullptr));
        SR_userAvatar->setText(QCoreApplication::translate("SmartResearchMainWindow", "RP", nullptr));
        SR_userName->setText(QCoreApplication::translate("SmartResearchMainWindow", "Responsable Pub", nullptr));
        SR_btnVueListe->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\223\213 Liste", nullptr));
        SR_btnAjouter->setText(QCoreApplication::translate("SmartResearchMainWindow", "\342\236\225 Ajouter", nullptr));
        SR_btnSupprimer->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\227\221\357\270\217 Supprimer", nullptr));
        SR_lineEditRecherche->setPlaceholderText(QCoreApplication::translate("SmartResearchMainWindow", "Rechercher...", nullptr));
        SR_btnRecherche->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\224\215", nullptr));
        SR_btnTri->setText(QCoreApplication::translate("SmartResearchMainWindow", "\342\207\205 Trier", nullptr));
        SR_btnExport->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\223\244 Export", nullptr));
        SR_btnStatistiques->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\223\212 Stats", nullptr));
        SR_lblListeTitre->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\223\213 Liste des Publications", nullptr));
        QTableWidgetItem *___qtablewidgetitem = SR_tablePublications->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("SmartResearchMainWindow", "Titre", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = SR_tablePublications->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("SmartResearchMainWindow", "Auteur(s)", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = SR_tablePublications->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("SmartResearchMainWindow", "Date", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = SR_tablePublications->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("SmartResearchMainWindow", "Revue", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = SR_tablePublications->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("SmartResearchMainWindow", "Statut", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = SR_tablePublications->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("SmartResearchMainWindow", "Actions", nullptr));
        SR_formTitle->setText(QCoreApplication::translate("SmartResearchMainWindow", "Ajouter une Nouvelle Publication", nullptr));
        SR_formSubtitle->setText(QCoreApplication::translate("SmartResearchMainWindow", "Remplissez les informations ci-dessous pour ajouter une publication", nullptr));
        SR_labelTitre->setText(QCoreApplication::translate("SmartResearchMainWindow", "Titre *", nullptr));
        SR_lineEditTitre->setPlaceholderText(QCoreApplication::translate("SmartResearchMainWindow", "Entrez le titre de la publication", nullptr));
        SR_labelAuteurs->setText(QCoreApplication::translate("SmartResearchMainWindow", "Auteurs *", nullptr));
        SR_lineEditAuteurs->setPlaceholderText(QCoreApplication::translate("SmartResearchMainWindow", "Nom des auteurs s\303\251par\303\251s par des virgules", nullptr));
        SR_labelRevue->setText(QCoreApplication::translate("SmartResearchMainWindow", "Revue/Journal *", nullptr));
        SR_lineEditRevue->setPlaceholderText(QCoreApplication::translate("SmartResearchMainWindow", "Nom de la revue ou conf\303\251rence", nullptr));
        SR_labelDate->setText(QCoreApplication::translate("SmartResearchMainWindow", "Date de Publication *", nullptr));
        SR_labelStatut->setText(QCoreApplication::translate("SmartResearchMainWindow", "Statut", nullptr));
        SR_comboBoxStatut->setItemText(0, QCoreApplication::translate("SmartResearchMainWindow", "Publi\303\251", nullptr));
        SR_comboBoxStatut->setItemText(1, QCoreApplication::translate("SmartResearchMainWindow", "Soumis", nullptr));
        SR_comboBoxStatut->setItemText(2, QCoreApplication::translate("SmartResearchMainWindow", "En r\303\251vision", nullptr));
        SR_comboBoxStatut->setItemText(3, QCoreApplication::translate("SmartResearchMainWindow", "Accept\303\251", nullptr));
        SR_comboBoxStatut->setItemText(4, QCoreApplication::translate("SmartResearchMainWindow", "Rejet\303\251", nullptr));

        SR_btnAnnulerAjout->setText(QCoreApplication::translate("SmartResearchMainWindow", "Annuler", nullptr));
        SR_btnAjouterPublication->setText(QCoreApplication::translate("SmartResearchMainWindow", "Ajouter la Publication", nullptr));
        SR_lblTotalIcon->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\223\232", nullptr));
        SR_lblTotalNumber->setText(QCoreApplication::translate("SmartResearchMainWindow", "0", nullptr));
        SR_lblTotalLabel->setText(QCoreApplication::translate("SmartResearchMainWindow", "Publications totales", nullptr));
        SR_lblThisYearIcon->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\206\225", nullptr));
        SR_lblThisYearNumber->setText(QCoreApplication::translate("SmartResearchMainWindow", "0", nullptr));
        SR_lblThisYearLabel->setText(QCoreApplication::translate("SmartResearchMainWindow", "Cette ann\303\251e (2024)", nullptr));
        SR_lblPlanSIcon->setText(QCoreApplication::translate("SmartResearchMainWindow", "\360\237\224\223", nullptr));
        SR_lblPlanSNumber->setText(QCoreApplication::translate("SmartResearchMainWindow", "0", nullptr));
        SR_lblPlanSLabel->setText(QCoreApplication::translate("SmartResearchMainWindow", "Open Access (Plan S)", nullptr));
        SR_lblBarChartTitle->setText(QCoreApplication::translate("SmartResearchMainWindow", "Publications par Ann\303\251e", nullptr));
        SR_lblBar1->setText(QCoreApplication::translate("SmartResearchMainWindow", "2021", nullptr));
        SR_lblBar2->setText(QCoreApplication::translate("SmartResearchMainWindow", "2022", nullptr));
        SR_lblBar3->setText(QCoreApplication::translate("SmartResearchMainWindow", "2023", nullptr));
        SR_lblBar4->setText(QCoreApplication::translate("SmartResearchMainWindow", "2024", nullptr));
        SR_lblPieChartTitle->setText(QCoreApplication::translate("SmartResearchMainWindow", "R\303\251partition par Statut", nullptr));
        SR_lblStatPublie->setText(QCoreApplication::translate("SmartResearchMainWindow", "\342\227\217 Publi\303\251 (40%)", nullptr));
        SR_lblStatSoumis->setText(QCoreApplication::translate("SmartResearchMainWindow", "\342\227\217 Soumis (25%)", nullptr));
        SR_lblStatRevision->setText(QCoreApplication::translate("SmartResearchMainWindow", "\342\227\217 En r\303\251vision (20%)", nullptr));
        SR_lblStatAccepte->setText(QCoreApplication::translate("SmartResearchMainWindow", "\342\227\217 Accept\303\251 (15%)", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SmartResearchMainWindow: public Ui_SmartResearchMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
