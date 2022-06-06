/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     leilong  <leilong@uniontech.com>
*
* Maintainer: leilong  <leilong@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "sidebarview.h"
#include "calendarmanage.h"
#include "widget/monthWidget/monthdayview.h"
#include "widget/dayWidget/daymonthview.h"
#include "DPaletteHelper"
#include <QVBoxLayout>
#include <QPixmap>

SidebarView::SidebarView(QWidget *parent) : QWidget(parent)
{
    initView();
    initConnection();
    //初始化数据
    initData();
}

/**
 * @brief SidebarView::initView
 * 初始化视图
 */
void SidebarView::initView()
{
    QVBoxLayout *hLayout = new QVBoxLayout(this);
    hLayout->setContentsMargins(0, 8, 0, 10);
    hLayout->setSpacing(0);

    m_treeWidget = new QTreeWidget();
    m_treeWidget->setHeaderHidden(true);
    m_treeWidget->setColumnCount(1);
    m_treeWidget->setExpandsOnDoubleClick(false);
    m_treeWidget->setSelectionMode(QAbstractItemView::NoSelection); //屏蔽item选择效果
    m_treeWidget->setFrameStyle(QFrame::NoFrame); //去除边框
    m_treeWidget->setIndentation(0);    //设置item前间距为0
    m_treeWidget->setFocusPolicy(Qt::TabFocus);
    m_calendarWidget = new SidebarCalendarWidget(this);
    m_calendarWidget->setFixedSize(180, 220);

    DPushButton *btn = new DPushButton;
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setFixedHeight(1);
    hLayout->addWidget(m_treeWidget, 1);
    hLayout->addWidget(btn);
    hLayout->addWidget(m_calendarWidget, 1);

    setLayout(hLayout);
    setFixedWidth(180);
}

void SidebarView::initConnection()
{
    //监听日程类型更新事件
    connect(gAccountManager, &AccountManager::signalAccountUpdate, this, &SidebarView::slotAccountUpdate);
    connect(gAccountManager, &AccountManager::signalScheduleTypeUpdate, this, &SidebarView::slotScheduleTypeUpdate);
}

/**
 * @brief SidebarView::initData
 * 初始化数据
 */
void SidebarView::initData()
{
    m_treeWidget->clear();

    initLocalAccountItem();
    initUnionAccountItem();
    //刷新列表展开状态
    initExpandStatus();
}

/**
 * @brief SidebarView::initExpandStatus
 * 初始化列表展开状态
 */
void SidebarView::initExpandStatus()
{
    //初始化列表展开状态
    QList<SidebarAccountItemWidget*> itemWidget;
    itemWidget << m_localItemWidget << m_unionItemWidget;
    for (SidebarAccountItemWidget *widget : itemWidget) {
        if (widget != nullptr) {
            //先进行一次反的展开状态目的是解决使用widget填充item时添加列表项后最后一项的widget没有隐藏问题
            bool ret = widget->getAccountItem()->getAccount()->isExpandDisplay();
            widget->getTreeItem()->setExpanded(!ret);
            widget->setSelectStatus(ret);
        }
    }
}

/**
 * @brief SidebarView::initLocalAccountItem
 * 初始化本地账户列表
 */
void SidebarView::initLocalAccountItem()
{
    if (nullptr != m_localItemWidget) {
        m_localItemWidget->deleteLater();
    }
    QSharedPointer<AccountItem> localAccount = gAccountManager->getLocalAccountItem();
    if (nullptr == localAccount) {
        return;
    }
    QTreeWidgetItem *localItem = new QTreeWidgetItem();
    m_treeWidget->addTopLevelItem(localItem);
    QString localName = localAccount->getAccount()->accountName();
    m_localItemWidget = new SidebarAccountItemWidget(localAccount);
    m_treeWidget->setItemWidget(localItem, 0, m_localItemWidget);
    m_localItemWidget->setItem(localItem);
}

/**
 * @brief SidebarView::initUnionAccountItem
 * 初始化unionID账户列表
 */
void SidebarView::initUnionAccountItem()
{
    if (nullptr != m_unionItemWidget) {
        m_unionItemWidget->deleteLater();
    }
    QSharedPointer<AccountItem> unionAccount = gAccountManager->getUnionAccountItem();
    if (nullptr == unionAccount) {
        return;
    }
    QTreeWidgetItem *unionItem = new QTreeWidgetItem();
    m_treeWidget->addTopLevelItem(unionItem);
    QString localName = unionAccount->getAccount()->accountName();
    m_unionItemWidget = new SidebarAccountItemWidget(unionAccount);
    m_treeWidget->setItemWidget(unionItem, 0, m_unionItemWidget);
    m_unionItemWidget->setItem(unionItem);
}

/**
 * @brief SidebarView::resetLocalChildItem
 * 重置日程类型选项
 * @param parentItemWidget 父列表控件
 */
void SidebarView::resetJobTypeChildItem(SidebarAccountItemWidget *parentItemWidget)
{
    if (nullptr == parentItemWidget) {
        return;
    }
    QTreeWidgetItem *parentItem = parentItemWidget->getTreeItem();
    int itemChildrenCounts = parentItem->childCount();
    while(itemChildrenCounts--)
    {
        QTreeWidgetItem * child =  parentItem->child(itemChildrenCounts); //index从大到小区做删除处理
        parentItem->removeChild(child);
        delete child;
        child = nullptr;
    }

    DScheduleType::List typeList = parentItemWidget->getAccountItem()->getScheduleTypeList();
    QTreeWidgetItem *item;
    for (DScheduleType::Ptr p : typeList) {
        if (nullptr != p) {
            item = new QTreeWidgetItem(parentItemWidget->getTreeItem());
            item->setTextAlignment(0, Qt::AlignVCenter | Qt::AlignLeft);
            item->setSizeHint(0, QSize(220, 40));
            parentItem->addChild(item);
            SidebarItemWidget *widget = new SidebarTypeItemWidget(p, this);
            m_treeWidget->setItemWidget(item, 0, widget);
        }
    }
    //刷新列表项位置，主要用于解决初始添加时最后一项显示位置和其他项位置不一致问题
    resetTreeItemPos(parentItemWidget->getTreeItem());
}

/**
 * @brief SidebarView::resetTreeItemPos
 * 刷新列表项位置，主要用于解决初始添加时最后一项显示位置和其他项位置不一致问题
 */
void SidebarView::resetTreeItemPos(QTreeWidgetItem *item)
{
    if (nullptr == item) {
        return;
    }
    QWidget* ptmpWidget = nullptr;
    ptmpWidget = m_treeWidget->itemWidget(item, 0);
    int x = ptmpWidget->x();
    QTreeWidgetItem *ptmp = nullptr;
    for(int i = 0; i < item->childCount(); i++)
    {
        ptmp = item->child(i);
        ptmpWidget = m_treeWidget->itemWidget(ptmp, 0);
        ptmpWidget->move(x, ptmpWidget->y());
    }
}

/**
 * @brief SidebarView::slotAccountUpdate
 * 账户更新事件
 */
void SidebarView::slotAccountUpdate()
{
    initData();
}

/**
 * @brief SidebarView::slotScheduleTypeUpdate
 * 日程类型更新事件
 */
void SidebarView::slotScheduleTypeUpdate()
{
    //初始化列表数据
    resetJobTypeChildItem(m_localItemWidget);
    resetJobTypeChildItem(m_unionItemWidget);
    //刷新列表展开状态
    initExpandStatus();
}

void SidebarView::paintEvent(QPaintEvent *event)
{
    //绘制背景
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette().color(QPalette::Base));
    int radius = 8;
    //绘制圆角
    painter.drawRoundedRect(0, 0, width(), height(), radius, radius);
    //绘制右侧直角
    painter.drawRect(radius, 0, width() - radius, height());

    QWidget::paintEvent(event);
}
