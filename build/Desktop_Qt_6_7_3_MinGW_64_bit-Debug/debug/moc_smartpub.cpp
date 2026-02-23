/****************************************************************************
** Meta object code from reading C++ file 'smartpub.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../smartpub.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'smartpub.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSLoginDialogENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSLoginDialogENDCLASS = QtMocHelpers::stringData(
    "LoginDialog",
    "onLoginClicked",
    "",
    "onForgotPasswordClicked",
    "onGuestClicked"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSLoginDialogENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   32,    2, 0x08,    1 /* Private */,
       3,    0,   33,    2, 0x08,    2 /* Private */,
       4,    0,   34,    2, 0x08,    3 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject LoginDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CLASSLoginDialogENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSLoginDialogENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSLoginDialogENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LoginDialog, std::true_type>,
        // method 'onLoginClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onForgotPasswordClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onGuestClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void LoginDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LoginDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onLoginClicked(); break;
        case 1: _t->onForgotPasswordClicked(); break;
        case 2: _t->onGuestClicked(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *LoginDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LoginDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSLoginDialogENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int LoginDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSSettingsDialogENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSSettingsDialogENDCLASS = QtMocHelpers::stringData(
    "SettingsDialog"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSSettingsDialogENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject SettingsDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CLASSSettingsDialogENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSSettingsDialogENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSSettingsDialogENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SettingsDialog, std::true_type>
    >,
    nullptr
} };

void SettingsDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *SettingsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SettingsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSSettingsDialogENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int SettingsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSFiltresDialogENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSFiltresDialogENDCLASS = QtMocHelpers::stringData(
    "FiltresDialog"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSFiltresDialogENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject FiltresDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CLASSFiltresDialogENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSFiltresDialogENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSFiltresDialogENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<FiltresDialog, std::true_type>
    >,
    nullptr
} };

void FiltresDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *FiltresDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FiltresDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSFiltresDialogENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int FiltresDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSIARecommandationsDialogENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSIARecommandationsDialogENDCLASS = QtMocHelpers::stringData(
    "IARecommandationsDialog"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSIARecommandationsDialogENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject IARecommandationsDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CLASSIARecommandationsDialogENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSIARecommandationsDialogENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSIARecommandationsDialogENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<IARecommandationsDialog, std::true_type>
    >,
    nullptr
} };

void IARecommandationsDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *IARecommandationsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IARecommandationsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSIARecommandationsDialogENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int IARecommandationsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSProjetDetailsDialogENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSProjetDetailsDialogENDCLASS = QtMocHelpers::stringData(
    "ProjetDetailsDialog"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSProjetDetailsDialogENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject ProjetDetailsDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CLASSProjetDetailsDialogENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSProjetDetailsDialogENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSProjetDetailsDialogENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ProjetDetailsDialog, std::true_type>
    >,
    nullptr
} };

void ProjetDetailsDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *ProjetDetailsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ProjetDetailsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSProjetDetailsDialogENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int ProjetDetailsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSStatistiquesDialogENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSStatistiquesDialogENDCLASS = QtMocHelpers::stringData(
    "StatistiquesDialog"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSStatistiquesDialogENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject StatistiquesDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CLASSStatistiquesDialogENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSStatistiquesDialogENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSStatistiquesDialogENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<StatistiquesDialog, std::true_type>
    >,
    nullptr
} };

void StatistiquesDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *StatistiquesDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StatistiquesDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSStatistiquesDialogENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int StatistiquesDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSFinStatistiquesDialogENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSFinStatistiquesDialogENDCLASS = QtMocHelpers::stringData(
    "FinStatistiquesDialog"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSFinStatistiquesDialogENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject FinStatistiquesDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CLASSFinStatistiquesDialogENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSFinStatistiquesDialogENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSFinStatistiquesDialogENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<FinStatistiquesDialog, std::true_type>
    >,
    nullptr
} };

void FinStatistiquesDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *FinStatistiquesDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FinStatistiquesDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSFinStatistiquesDialogENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int FinStatistiquesDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSSmartPubENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSSmartPubENDCLASS = QtMocHelpers::stringData(
    "SmartPub",
    "on_btnPublications_clicked",
    "",
    "on_btnChercheurs_clicked",
    "on_btnLaboratoires_clicked",
    "on_btnProjets_clicked",
    "on_btnFinances_clicked",
    "on_btnEvenements_clicked",
    "expandSidebar",
    "collapseSidebar",
    "onSettingsClicked",
    "on_cherchBtnVueListe_clicked",
    "on_cherchBtnAjouter_clicked",
    "on_cherchBtnRecherche_clicked",
    "on_cherchBtnTri_clicked",
    "on_cherchBtnExport_clicked",
    "on_cherchBtnStatistiques_clicked",
    "on_cherchBtnUploadPhoto_clicked",
    "on_cherchBtnAjouterChercheur_clicked",
    "on_cherchBtnAnnulerAjout_clicked",
    "on_cherchModifierChercheur",
    "id",
    "on_cherchSupprimerChercheur",
    "on_cherchVoirDetailsChercheur",
    "on_cherchLineEditRecherche_textChanged",
    "text",
    "on_cherchBtnLogin_clicked",
    "on_cherchBtnMotDePasseOublie_clicked",
    "on_cherchBtnRetourLogin_clicked",
    "on_cherchBtnForgotOk_clicked",
    "on_cherchBtnToggleVue_clicked",
    "on_cherchBtnExportDetails_clicked",
    "on_SR_btnVueListe_clicked",
    "on_SR_btnAjouter_clicked",
    "on_SR_btnRecherche_clicked",
    "on_SR_btnTri_clicked",
    "on_SR_btnExport_clicked",
    "on_SR_btnStatistiques_clicked",
    "on_SR_btnAjouterPublication_clicked",
    "on_SR_btnAnnulerAjout_clicked",
    "on_SR_modifierPublication_clicked",
    "on_SR_supprimerPublication_clicked",
    "SR_applyFilterListe",
    "SR_reinitFilterListe",
    "on_finBtnVueListe_clicked",
    "on_finBtnAjouter_clicked",
    "on_finBtnRecherche_clicked",
    "on_finBtnTri_clicked",
    "on_finBtnExport_clicked",
    "on_finBtnStatistiques_clicked",
    "on_finBtnAjouterTransaction_clicked",
    "on_finBtnAnnulerAjout_clicked",
    "on_finBtnModifierTransaction_clicked",
    "on_finBtnSupprimerTransaction_clicked",
    "on_finLineEditRecherche_textChanged",
    "on_evBtnAjouterEvent_clicked",
    "on_evBtnModifierEvent_clicked",
    "on_evBtnSupprimerEvent_clicked",
    "on_evBtnTrierDate_clicked",
    "on_evBtnRechercheLieu_clicked",
    "on_evBtnExportCalendrier_clicked",
    "on_evBtnLivreResumes_clicked",
    "on_evBtnCalculImpact_clicked",
    "on_evBtnStatsParticipation_clicked",
    "on_btnListeProjets_clicked",
    "on_btnAjouterProjet_clicked",
    "on_btnModifierProjet_clicked",
    "on_btnSupprimerProjet_clicked",
    "on_lineEditRechercheProjets_textChanged",
    "on_btnAnnulerForm_clicked",
    "on_btnEnregistrerForm_clicked",
    "on_tableSelectionChanged",
    "on_tableDoubleClicked",
    "row",
    "column",
    "on_triDateDebutClicked",
    "on_triDateFinClicked",
    "on_triEtatClicked",
    "on_triProgressionClicked",
    "on_statistiquesClicked",
    "on_santeProjetClicked",
    "on_optimiserChargeClicked",
    "on_iaRecommanderClicked",
    "on_filtresClicked",
    "on_exporterClicked",
    "onNavigationButtonClicked",
    "onCrudButtonClicked",
    "onRechercheTextChanged",
    "onAnnulerFormClicked",
    "onEnregistrerFormClicked",
    "onSupprimerProjetClicked",
    "onTableSelectionChanged",
    "onTableDoubleClicked",
    "onTriDateDebutClicked",
    "onTriDateFinClicked",
    "onTriEtatClicked",
    "onTriProgressionClicked",
    "onStatistiquesClicked",
    "onSanteProjetClicked",
    "onOptimiserChargeClicked",
    "onIARecommanderClicked",
    "onFiltresClicked",
    "onExporterClicked"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSSmartPubENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      97,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  596,    2, 0x08,    1 /* Private */,
       3,    0,  597,    2, 0x08,    2 /* Private */,
       4,    0,  598,    2, 0x08,    3 /* Private */,
       5,    0,  599,    2, 0x08,    4 /* Private */,
       6,    0,  600,    2, 0x08,    5 /* Private */,
       7,    0,  601,    2, 0x08,    6 /* Private */,
       8,    0,  602,    2, 0x08,    7 /* Private */,
       9,    0,  603,    2, 0x08,    8 /* Private */,
      10,    0,  604,    2, 0x08,    9 /* Private */,
      11,    0,  605,    2, 0x08,   10 /* Private */,
      12,    0,  606,    2, 0x08,   11 /* Private */,
      13,    0,  607,    2, 0x08,   12 /* Private */,
      14,    0,  608,    2, 0x08,   13 /* Private */,
      15,    0,  609,    2, 0x08,   14 /* Private */,
      16,    0,  610,    2, 0x08,   15 /* Private */,
      17,    0,  611,    2, 0x08,   16 /* Private */,
      18,    0,  612,    2, 0x08,   17 /* Private */,
      19,    0,  613,    2, 0x08,   18 /* Private */,
      20,    1,  614,    2, 0x08,   19 /* Private */,
      22,    1,  617,    2, 0x08,   21 /* Private */,
      23,    1,  620,    2, 0x08,   23 /* Private */,
      24,    1,  623,    2, 0x08,   25 /* Private */,
      26,    0,  626,    2, 0x08,   27 /* Private */,
      27,    0,  627,    2, 0x08,   28 /* Private */,
      28,    0,  628,    2, 0x08,   29 /* Private */,
      29,    0,  629,    2, 0x08,   30 /* Private */,
      30,    0,  630,    2, 0x08,   31 /* Private */,
      31,    0,  631,    2, 0x08,   32 /* Private */,
      32,    0,  632,    2, 0x08,   33 /* Private */,
      33,    0,  633,    2, 0x08,   34 /* Private */,
      34,    0,  634,    2, 0x08,   35 /* Private */,
      35,    0,  635,    2, 0x08,   36 /* Private */,
      36,    0,  636,    2, 0x08,   37 /* Private */,
      37,    0,  637,    2, 0x08,   38 /* Private */,
      38,    0,  638,    2, 0x08,   39 /* Private */,
      39,    0,  639,    2, 0x08,   40 /* Private */,
      40,    0,  640,    2, 0x08,   41 /* Private */,
      41,    0,  641,    2, 0x08,   42 /* Private */,
      42,    0,  642,    2, 0x08,   43 /* Private */,
      43,    0,  643,    2, 0x08,   44 /* Private */,
      44,    0,  644,    2, 0x08,   45 /* Private */,
      45,    0,  645,    2, 0x08,   46 /* Private */,
      46,    0,  646,    2, 0x08,   47 /* Private */,
      47,    0,  647,    2, 0x08,   48 /* Private */,
      48,    0,  648,    2, 0x08,   49 /* Private */,
      49,    0,  649,    2, 0x08,   50 /* Private */,
      50,    0,  650,    2, 0x08,   51 /* Private */,
      51,    0,  651,    2, 0x08,   52 /* Private */,
      52,    0,  652,    2, 0x08,   53 /* Private */,
      53,    0,  653,    2, 0x08,   54 /* Private */,
      54,    1,  654,    2, 0x08,   55 /* Private */,
      55,    0,  657,    2, 0x08,   57 /* Private */,
      56,    0,  658,    2, 0x08,   58 /* Private */,
      57,    0,  659,    2, 0x08,   59 /* Private */,
      58,    0,  660,    2, 0x08,   60 /* Private */,
      59,    0,  661,    2, 0x08,   61 /* Private */,
      60,    0,  662,    2, 0x08,   62 /* Private */,
      61,    0,  663,    2, 0x08,   63 /* Private */,
      62,    0,  664,    2, 0x08,   64 /* Private */,
      63,    0,  665,    2, 0x08,   65 /* Private */,
      64,    0,  666,    2, 0x08,   66 /* Private */,
      65,    0,  667,    2, 0x08,   67 /* Private */,
      66,    0,  668,    2, 0x08,   68 /* Private */,
      67,    0,  669,    2, 0x08,   69 /* Private */,
      68,    1,  670,    2, 0x08,   70 /* Private */,
      69,    0,  673,    2, 0x08,   72 /* Private */,
      70,    0,  674,    2, 0x08,   73 /* Private */,
      71,    0,  675,    2, 0x08,   74 /* Private */,
      72,    2,  676,    2, 0x08,   75 /* Private */,
      75,    0,  681,    2, 0x08,   78 /* Private */,
      76,    0,  682,    2, 0x08,   79 /* Private */,
      77,    0,  683,    2, 0x08,   80 /* Private */,
      78,    0,  684,    2, 0x08,   81 /* Private */,
      79,    0,  685,    2, 0x08,   82 /* Private */,
      80,    0,  686,    2, 0x08,   83 /* Private */,
      81,    0,  687,    2, 0x08,   84 /* Private */,
      82,    0,  688,    2, 0x08,   85 /* Private */,
      83,    0,  689,    2, 0x08,   86 /* Private */,
      84,    0,  690,    2, 0x08,   87 /* Private */,
      85,    0,  691,    2, 0x08,   88 /* Private */,
      86,    0,  692,    2, 0x08,   89 /* Private */,
      87,    1,  693,    2, 0x08,   90 /* Private */,
      88,    0,  696,    2, 0x08,   92 /* Private */,
      89,    0,  697,    2, 0x08,   93 /* Private */,
      90,    0,  698,    2, 0x08,   94 /* Private */,
      91,    0,  699,    2, 0x08,   95 /* Private */,
      92,    2,  700,    2, 0x08,   96 /* Private */,
      93,    0,  705,    2, 0x08,   99 /* Private */,
      94,    0,  706,    2, 0x08,  100 /* Private */,
      95,    0,  707,    2, 0x08,  101 /* Private */,
      96,    0,  708,    2, 0x08,  102 /* Private */,
      97,    0,  709,    2, 0x08,  103 /* Private */,
      98,    0,  710,    2, 0x08,  104 /* Private */,
      99,    0,  711,    2, 0x08,  105 /* Private */,
     100,    0,  712,    2, 0x08,  106 /* Private */,
     101,    0,  713,    2, 0x08,  107 /* Private */,
     102,    0,  714,    2, 0x08,  108 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   21,
    QMetaType::Void, QMetaType::Int,   21,
    QMetaType::Void, QMetaType::Int,   21,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   73,   74,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   73,   74,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject SmartPub::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_CLASSSmartPubENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSSmartPubENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSSmartPubENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SmartPub, std::true_type>,
        // method 'on_btnPublications_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnChercheurs_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnLaboratoires_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnProjets_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnFinances_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnEvenements_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'expandSidebar'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'collapseSidebar'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSettingsClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchBtnVueListe_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchBtnAjouter_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchBtnRecherche_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchBtnTri_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchBtnExport_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchBtnStatistiques_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchBtnUploadPhoto_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchBtnAjouterChercheur_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchBtnAnnulerAjout_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchModifierChercheur'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'on_cherchSupprimerChercheur'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'on_cherchVoirDetailsChercheur'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'on_cherchLineEditRecherche_textChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'on_cherchBtnLogin_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchBtnMotDePasseOublie_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchBtnRetourLogin_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchBtnForgotOk_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchBtnToggleVue_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_cherchBtnExportDetails_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_SR_btnVueListe_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_SR_btnAjouter_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_SR_btnRecherche_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_SR_btnTri_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_SR_btnExport_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_SR_btnStatistiques_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_SR_btnAjouterPublication_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_SR_btnAnnulerAjout_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_SR_modifierPublication_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_SR_supprimerPublication_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'SR_applyFilterListe'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'SR_reinitFilterListe'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_finBtnVueListe_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_finBtnAjouter_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_finBtnRecherche_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_finBtnTri_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_finBtnExport_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_finBtnStatistiques_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_finBtnAjouterTransaction_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_finBtnAnnulerAjout_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_finBtnModifierTransaction_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_finBtnSupprimerTransaction_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_finLineEditRecherche_textChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'on_evBtnAjouterEvent_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_evBtnModifierEvent_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_evBtnSupprimerEvent_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_evBtnTrierDate_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_evBtnRechercheLieu_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_evBtnExportCalendrier_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_evBtnLivreResumes_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_evBtnCalculImpact_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_evBtnStatsParticipation_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnListeProjets_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnAjouterProjet_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnModifierProjet_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnSupprimerProjet_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_lineEditRechercheProjets_textChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'on_btnAnnulerForm_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_btnEnregistrerForm_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_tableSelectionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_tableDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'on_triDateDebutClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_triDateFinClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_triEtatClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_triProgressionClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_statistiquesClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_santeProjetClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_optimiserChargeClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_iaRecommanderClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_filtresClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'on_exporterClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onNavigationButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCrudButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRechercheTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onAnnulerFormClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEnregistrerFormClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSupprimerProjetClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTableSelectionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTableDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onTriDateDebutClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTriDateFinClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTriEtatClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTriProgressionClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onStatistiquesClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSanteProjetClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onOptimiserChargeClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onIARecommanderClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onFiltresClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onExporterClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void SmartPub::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SmartPub *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->on_btnPublications_clicked(); break;
        case 1: _t->on_btnChercheurs_clicked(); break;
        case 2: _t->on_btnLaboratoires_clicked(); break;
        case 3: _t->on_btnProjets_clicked(); break;
        case 4: _t->on_btnFinances_clicked(); break;
        case 5: _t->on_btnEvenements_clicked(); break;
        case 6: _t->expandSidebar(); break;
        case 7: _t->collapseSidebar(); break;
        case 8: _t->onSettingsClicked(); break;
        case 9: _t->on_cherchBtnVueListe_clicked(); break;
        case 10: _t->on_cherchBtnAjouter_clicked(); break;
        case 11: _t->on_cherchBtnRecherche_clicked(); break;
        case 12: _t->on_cherchBtnTri_clicked(); break;
        case 13: _t->on_cherchBtnExport_clicked(); break;
        case 14: _t->on_cherchBtnStatistiques_clicked(); break;
        case 15: _t->on_cherchBtnUploadPhoto_clicked(); break;
        case 16: _t->on_cherchBtnAjouterChercheur_clicked(); break;
        case 17: _t->on_cherchBtnAnnulerAjout_clicked(); break;
        case 18: _t->on_cherchModifierChercheur((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 19: _t->on_cherchSupprimerChercheur((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 20: _t->on_cherchVoirDetailsChercheur((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 21: _t->on_cherchLineEditRecherche_textChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 22: _t->on_cherchBtnLogin_clicked(); break;
        case 23: _t->on_cherchBtnMotDePasseOublie_clicked(); break;
        case 24: _t->on_cherchBtnRetourLogin_clicked(); break;
        case 25: _t->on_cherchBtnForgotOk_clicked(); break;
        case 26: _t->on_cherchBtnToggleVue_clicked(); break;
        case 27: _t->on_cherchBtnExportDetails_clicked(); break;
        case 28: _t->on_SR_btnVueListe_clicked(); break;
        case 29: _t->on_SR_btnAjouter_clicked(); break;
        case 30: _t->on_SR_btnRecherche_clicked(); break;
        case 31: _t->on_SR_btnTri_clicked(); break;
        case 32: _t->on_SR_btnExport_clicked(); break;
        case 33: _t->on_SR_btnStatistiques_clicked(); break;
        case 34: _t->on_SR_btnAjouterPublication_clicked(); break;
        case 35: _t->on_SR_btnAnnulerAjout_clicked(); break;
        case 36: _t->on_SR_modifierPublication_clicked(); break;
        case 37: _t->on_SR_supprimerPublication_clicked(); break;
        case 38: _t->SR_applyFilterListe(); break;
        case 39: _t->SR_reinitFilterListe(); break;
        case 40: _t->on_finBtnVueListe_clicked(); break;
        case 41: _t->on_finBtnAjouter_clicked(); break;
        case 42: _t->on_finBtnRecherche_clicked(); break;
        case 43: _t->on_finBtnTri_clicked(); break;
        case 44: _t->on_finBtnExport_clicked(); break;
        case 45: _t->on_finBtnStatistiques_clicked(); break;
        case 46: _t->on_finBtnAjouterTransaction_clicked(); break;
        case 47: _t->on_finBtnAnnulerAjout_clicked(); break;
        case 48: _t->on_finBtnModifierTransaction_clicked(); break;
        case 49: _t->on_finBtnSupprimerTransaction_clicked(); break;
        case 50: _t->on_finLineEditRecherche_textChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 51: _t->on_evBtnAjouterEvent_clicked(); break;
        case 52: _t->on_evBtnModifierEvent_clicked(); break;
        case 53: _t->on_evBtnSupprimerEvent_clicked(); break;
        case 54: _t->on_evBtnTrierDate_clicked(); break;
        case 55: _t->on_evBtnRechercheLieu_clicked(); break;
        case 56: _t->on_evBtnExportCalendrier_clicked(); break;
        case 57: _t->on_evBtnLivreResumes_clicked(); break;
        case 58: _t->on_evBtnCalculImpact_clicked(); break;
        case 59: _t->on_evBtnStatsParticipation_clicked(); break;
        case 60: _t->on_btnListeProjets_clicked(); break;
        case 61: _t->on_btnAjouterProjet_clicked(); break;
        case 62: _t->on_btnModifierProjet_clicked(); break;
        case 63: _t->on_btnSupprimerProjet_clicked(); break;
        case 64: _t->on_lineEditRechercheProjets_textChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 65: _t->on_btnAnnulerForm_clicked(); break;
        case 66: _t->on_btnEnregistrerForm_clicked(); break;
        case 67: _t->on_tableSelectionChanged(); break;
        case 68: _t->on_tableDoubleClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 69: _t->on_triDateDebutClicked(); break;
        case 70: _t->on_triDateFinClicked(); break;
        case 71: _t->on_triEtatClicked(); break;
        case 72: _t->on_triProgressionClicked(); break;
        case 73: _t->on_statistiquesClicked(); break;
        case 74: _t->on_santeProjetClicked(); break;
        case 75: _t->on_optimiserChargeClicked(); break;
        case 76: _t->on_iaRecommanderClicked(); break;
        case 77: _t->on_filtresClicked(); break;
        case 78: _t->on_exporterClicked(); break;
        case 79: _t->onNavigationButtonClicked(); break;
        case 80: _t->onCrudButtonClicked(); break;
        case 81: _t->onRechercheTextChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 82: _t->onAnnulerFormClicked(); break;
        case 83: _t->onEnregistrerFormClicked(); break;
        case 84: _t->onSupprimerProjetClicked(); break;
        case 85: _t->onTableSelectionChanged(); break;
        case 86: _t->onTableDoubleClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 87: _t->onTriDateDebutClicked(); break;
        case 88: _t->onTriDateFinClicked(); break;
        case 89: _t->onTriEtatClicked(); break;
        case 90: _t->onTriProgressionClicked(); break;
        case 91: _t->onStatistiquesClicked(); break;
        case 92: _t->onSanteProjetClicked(); break;
        case 93: _t->onOptimiserChargeClicked(); break;
        case 94: _t->onIARecommanderClicked(); break;
        case 95: _t->onFiltresClicked(); break;
        case 96: _t->onExporterClicked(); break;
        default: ;
        }
    }
}

const QMetaObject *SmartPub::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SmartPub::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSSmartPubENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int SmartPub::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 97)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 97;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 97)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 97;
    }
    return _id;
}
QT_WARNING_POP
