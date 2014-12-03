#include "sidewindowstyle.h"
#include "tooldockwidget.h"
#include "rotationtoolbutton.h"

#include <QStatusBar>

SideActionBar::SideActionBar(QSize _iconSize, QMainWindow *_window, Qt::DockWidgetArea _area)
    : QObject(_window), iconSize(_iconSize), window(_window),area(_area), bHideToolBar(false)
{
    toolBar = new QToolBar;
    toolBar->hide();
    toolBar->setObjectName(QString("side_tool_%1").arg(area));
    toolBar->setMovable(false);

    QWidget *spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    spacerAct = toolBar->addWidget(spacer);
    toolBar->addSeparator();
}

SideActionBar::~SideActionBar()
{
    qDeleteAll(m_actionStateMap);
}

void SideActionBar::addAction(QAction *action, QWidget *widget, const QString &id, const QString &title, QList<QAction *> widgetActions)
{
    RotationToolButton *btn = new RotationToolButton;
    btn->setDefaultAction(action);    
    if (area == Qt::LeftDockWidgetArea) {
        btn->setRotation(RotationToolButton::CounterClockwise);
    } else if (area == Qt::RightDockWidgetArea) {
        btn->setRotation(RotationToolButton::Clockwise);
    }

    SideDockWidget *dock = new SideDockWidget(iconSize, window);
    dock->setObjectName(QString("side_dock_%1").arg(area));
    dock->setWindowTitle(QString("side_dock_%1").arg(area));
    dock->setFeatures(QDockWidget::DockWidgetClosable);
    dock->hide();
    dock->createMenu(area);

    window->addDockWidget(area,dock);

    connect(dock,SIGNAL(visibilityChanged(bool)),this,SLOT(dockVisible(bool)));
    connect(dock,SIGNAL(moveActionTo(Qt::DockWidgetArea,QAction*)),this,SIGNAL(moveActionTo(Qt::DockWidgetArea,QAction*)));
    connect(dock,SIGNAL(currenActionChanged(QAction*,QAction*)),this,SLOT(currenActionChanged(QAction*,QAction*)));

    SideActionState *state = new SideActionState;
    state->toolBtn = btn;
    state->widget = widget;
    state->id = id;
    state->title = title;
    state->widgetActions = widgetActions;
    m_actionStateMap.insert(action,state);
    dock->setAction(action,title);
    toolBar->insertWidget(spacerAct,btn);
    if (toolBar->isHidden() && !bHideToolBar) {
        toolBar->show();
    }
    m_dockList.append(dock);
    connect(action,SIGNAL(toggled(bool)),this,SLOT(toggledAction(bool)));
    //update actions
    foreach(SideDockWidget *dock, m_dockList) {
        dock->setActions(m_actionStateMap.keys());
    }
}

void SideActionBar::removeAction(QAction *action)
{
    SideActionState *state = m_actionStateMap.value(action);
    if (state) {
        delete state->toolBtn;
    }
    m_actionStateMap.remove(action);
    delete state;
}

void SideActionBar::setHideToolBar(bool b)
{
    bHideToolBar = b;
    if (bHideToolBar) {
        toolBar->hide();
    } else {
        toolBar->show();
    }
}

QAction *SideActionBar::findToolAction(QWidget *widget)
{
    QMapIterator<QAction*,SideActionState*> i(m_actionStateMap);
    while (i.hasNext()) {
        i.next();
        if (i.value()->widget == widget) {
            return i.key();
        }
    }
    return 0;
}

void SideActionBar::dockVisible(bool b)
{
    SideDockWidget *dock = (SideDockWidget*)sender();
    QAction *action = dock->checkedAction();
    if (action) {
        action->setChecked(dock->isVisible());
    } else if (b && !dock->actions().isEmpty()) {
        dock->actions().first()->setChecked(true);
    }
}

void SideActionBar::updateAction(QAction *action)
{
    SideActionState *state = m_actionStateMap.value(action);
    foreach (SideDockWidget *dock, m_dockList) {
        if (dock->checkedAction() == action) {
            if (action->isChecked()) {
                if (dock->isHidden()) {
                    dock->show();
                }
                dock->setWidget(state->widget);
                dock->setWidgetActions(state->widgetActions);
                dock->setWindowTitle(state->title);
            } else {
                dock->hide();
            }
            break;
        }
    }
}


void SideActionBar::toggledAction(bool)
{
    QAction *action = (QAction*)sender();
    updateAction(action);
}

void SideActionBar::currenActionChanged(QAction *org, QAction *act)
{
    if (org == act) {
        return;
    }
    SideDockWidget *curDock = (SideDockWidget*)sender();
    foreach (SideDockWidget *dock, m_dockList) {
        if ((dock->checkedAction() == act) && (dock != curDock)) {
            dock->setAction(org,org->text());
            org->setChecked(act->isChecked());
            updateAction(org);
            break;
        }
    }
    curDock->setAction(act,act->text());
    act->setChecked(true);
    updateAction(act);
}


OutputActionBar::OutputActionBar(QSize iconSize, QMainWindow *window, Qt::DockWidgetArea _area)
    : QObject(window), area(_area), bHideToolBar(false)
{
    toolBar = new QToolBar;
    toolBar->hide();
    toolBar->setObjectName(QString("side_tool_%1").arg(area));
    toolBar->setMovable(false);

    QWidget *spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    spacerAct = toolBar->addWidget(spacer);
    toolBar->addSeparator();

    dock = new OutputDockWidget(iconSize, window);
    dock->setObjectName(QString("side_dock_%1").arg(area));
    dock->setWindowTitle(QString("side_dock_%1").arg(area));
    dock->setFeatures(QDockWidget::DockWidgetClosable);
    dock->hide();
    dock->createMenu(area);

    window->addDockWidget(area,dock);

    connect(dock,SIGNAL(visibilityChanged(bool)),this,SLOT(dockVisible(bool)));
    connect(dock,SIGNAL(moveActionTo(Qt::DockWidgetArea,QAction*)),this,SIGNAL(moveActionTo(Qt::DockWidgetArea,QAction*)));
}

OutputActionBar::~OutputActionBar()
{
    qDeleteAll(m_actionStateMap);
}

OutputDockWidget *OutputActionBar::dockWidget() const
{
    return dock;
}

void OutputActionBar::addAction(QAction *action, QWidget *widget, const QString &id, const QString &title, QList<QAction *> widgetActions)
{
    RotationToolButton *btn = new RotationToolButton;
    btn->setDefaultAction(action);
    if (area == Qt::LeftDockWidgetArea) {
        btn->setRotation(RotationToolButton::CounterClockwise);
    } else if (area == Qt::RightDockWidgetArea) {
        btn->setRotation(RotationToolButton::Clockwise);
    }
    OutputActionState *state = new OutputActionState;
    state->toolBtn = btn;
    state->widget = widget;
    state->id = id;
    state->title = title;
    state->widgetActions = widgetActions;
    m_actionStateMap.insert(action,state);
    dock->addAction(action,title);
    toolBar->insertWidget(spacerAct,btn);
    if (toolBar->isHidden() && !bHideToolBar) {
        toolBar->show();
    }
    connect(action,SIGNAL(toggled(bool)),this,SLOT(toggledAction(bool)));
}

void OutputActionBar::removeAction(QAction *action)
{
    OutputActionState *state = m_actionStateMap.value(action);
    if (state) {
        delete state->toolBtn;
    }
    m_actionStateMap.remove(action);
    delete state;
    dock->removeAction(action);
    if (dock->actions().isEmpty()) {
        toolBar->hide();
    }
}

void OutputActionBar::setHideToolBar(bool b)
{
    bHideToolBar = b;
    if (bHideToolBar) {
        toolBar->hide();
    } else {
        if (!dock->actions().isEmpty()){
            toolBar->show();
        }
    }
}

QAction *OutputActionBar::findToolAction(QWidget *widget)
{
    QMapIterator<QAction*,OutputActionState*> i(m_actionStateMap);
    while (i.hasNext()) {
        i.next();
        if (i.value()->widget == widget) {
            return i.key();
        }
    }
    return 0;
}

void OutputActionBar::dockVisible(bool b)
{
    QAction *action = dock->checkedAction();
    if (action) {
        action->setChecked(dock->isVisible());
    } else if (b && !dock->actions().isEmpty()) {
        dock->actions().first()->setChecked(true);
    }
}

void OutputActionBar::toggledAction(bool b)
{
    QAction *action = (QAction*)sender();
    OutputActionState *state = m_actionStateMap.value(action);
    if (!state) {
        return;
    }
    if (action->isChecked()) {
        if (dock->isHidden()) {
            dock->show();
        }
        dock->setWidget(state->widget);
        dock->setWidgetActions(state->widgetActions);
        dock->setWindowTitle(state->title);
    } else {
        if (!dock->checkedAction()) {
            dock->hide();
        }
    }
}

SideWindowStyle::SideWindowStyle(QSize iconSize, QMainWindow *window, QObject *parent)
    : IWindowStyle(parent),m_mainWindow(window)
{
    m_sideBar = new SideActionBar(iconSize,window,Qt::LeftDockWidgetArea);
    m_outputBar = new OutputActionBar(iconSize,window,Qt::BottomDockWidgetArea);

    m_mainWindow->addToolBar(Qt::LeftToolBarArea,m_sideBar->toolBar);
    //m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea,m_sideBar->dock);

    m_mainWindow->setCorner(Qt::BottomLeftCorner,Qt::LeftDockWidgetArea);

    m_mainWindow->addDockWidget(Qt::BottomDockWidgetArea,m_outputBar->dock);

    m_mainWindow->setDockNestingEnabled(true);
    m_mainWindow->setDockOptions(QMainWindow::AllowNestedDocks);

    m_statusBar = new QStatusBar;

    m_hideSideAct = new QAction(tr("Hide Sidebars"),this);
    m_hideSideAct->setIcon(QIcon("icon:images/hidesidebar.png"));
    m_hideSideAct->setCheckable(true);

    QToolButton *btn = new QToolButton;
    btn->setDefaultAction(m_hideSideAct);
    btn->setStyleSheet("QToolButton {border:0}"
                       "QToolButton:checked {background : qlineargradient(spread:pad, x1:0, y1:1, x2:1, y2:0, stop:0 rgba(55, 57, 59, 255), stop:1 rgba(255, 255, 255, 255));}");
    m_statusBar->addWidget(btn);

    m_statusBar->setContentsMargins(0,0,0,0);

    m_statusBar->addWidget(m_outputBar->toolBar,1);

    m_mainWindow->setStatusBar(m_statusBar);

    connect(m_hideSideAct,SIGNAL(toggled(bool)),this,SLOT(hideSideBar(bool)));
}

SideWindowStyle::~SideWindowStyle()
{

}

void SideWindowStyle::restoreHideToolWindows()
{
    foreach(QAction *action,m_hideActionList) {
        action->setChecked(true);
    }
    m_hideActionList.clear();
}

void SideWindowStyle::restoreHideSideToolWindows()
{
    foreach(QAction *action,m_hideSideActionList) {
        action->setChecked(true);
    }
    m_hideSideActionList.clear();
}

void SideWindowStyle::hideSideToolWindows()
{
    m_hideSideActionList.clear();

    foreach(QAction *action, m_sideBar->m_actionStateMap.keys()) {
        if (action->isChecked()) {
            m_hideSideActionList.append(action);
            action->setChecked(false);
        }
    }
}

void SideWindowStyle::hideAllToolWindows()
{
    m_hideActionList.clear();

    foreach(QAction *action, m_sideBar->m_actionStateMap.keys()) {
        if (action->isChecked()) {
            m_hideActionList.append(action);
            action->setChecked(false);
        }
    }
    foreach(QAction *action, m_outputBar->m_actionStateMap.keys()) {
        if (action->isChecked()) {
            m_hideActionList.append(action);
            action->setChecked(false);
        }
    }
}

void SideWindowStyle::hideSideBar(bool b)
{
    if (b) {
        hideSideToolWindows();
    } else {
        restoreHideSideToolWindows();
    }
}

void SideWindowStyle::toggledSideBar(bool b)
{
    if (b) {
        m_hideSideAct->setChecked(false);
    }
}

void SideWindowStyle::showOrHideToolWindow()
{
    bool hide = false;
    foreach(QAction *action, m_sideBar->m_actionStateMap.keys()) {
        if (action->isChecked()) {
            hide = true;
            break;
        }
    }
    if (hide) {
        hideAllToolWindows();
    } else {
        restoreHideToolWindows();
    }
}

void SideWindowStyle::hideToolWindow(Qt::DockWidgetArea area)
{

}

static int VersionMarker = 0xffe1;

QByteArray SideWindowStyle::saveToolState(int version) const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << VersionMarker;
    stream << version;
    QMapIterator<QAction*,SideActionState*> it(m_sideBar->m_actionStateMap);
    while (it.hasNext()) {
        it.next();
        SideActionState *state = it.value();
        stream << state->id;
        stream << it.key()->isChecked();
    }
    return data;
}

bool SideWindowStyle::restoreToolsState(const QByteArray &state, int version)
{
    if (state.isEmpty())
        return false;

    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    int marker, v;
    stream >> marker;
    stream >> v;
    if (stream.status() != QDataStream::Ok || marker != VersionMarker || v != version)
        return false;

    QString id;
    bool checked = false;
    QMap<QString,bool> idMap;
    while(!stream.atEnd()) {
        stream >> id;
        stream >> checked;
        idMap.insert(id,checked);
    }
    if (stream.status() != QDataStream::Ok) {
        return false;
    }
    QMapIterator<QAction*,SideActionState*> it(m_sideBar->m_actionStateMap);
    while (it.hasNext()) {
        it.next();
        SideActionState *state = it.value();
        bool bcheck = idMap.value(state->id);
        if (bcheck) {
            it.key()->setChecked(bcheck);
        }
    }
    return true;
}


void SideWindowStyle::moveToolWindow(Qt::DockWidgetArea area, QAction *action, bool split)
{

}

QAction *SideWindowStyle::findToolWindow(QWidget *widget)
{
    QAction *act = m_sideBar->findToolAction(widget);
    if (act) {
        return act;
    }
    return m_outputBar->findToolAction(widget);
}

void SideWindowStyle::removeToolWindow(QAction *action)
{

}

QAction *SideWindowStyle::addToolWindow(LiteApi::IApplication *app, Qt::DockWidgetArea area, QWidget *widget, const QString &id, const QString &title, bool split, QList<QAction *> widgetActions)
{
    QAction *action = new QAction(this);
    action->setText(title);
    action->setCheckable(true);
    action->setObjectName(id);
    if (area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea) {
        m_outputBar->addAction(action,widget,id,title,widgetActions);
    } else {
        m_sideBar->addAction(action,widget,id,title,widgetActions);
        connect(action,SIGNAL(toggled(bool)),this,SLOT(toggledSideBar(bool)));
    }
    return action;
}
