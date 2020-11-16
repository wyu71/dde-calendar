/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
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

#include "calendardbus.h"
#include "todaybutton.h"
#include "scheduledatamanage.h"
#include "daymonthview.h"
#include "constants.h"
#include "dayhuangliview.h"

#include <DPalette>
#include <DHorizontalLine>
#include <DHiDPIHelper>

#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QQueue>
#include <QSpacerItem>

DGUI_USE_NAMESPACE
CDayMonthView::CDayMonthView(QWidget *parent)
    : CustomFrame(parent)
{
    m_DBusInter = new CalendarDBus("com.deepin.api.LunarCalendar",
                                   "/com/deepin/api/LunarCalendar",
                                   QDBusConnection::sessionBus(), this);
    if (!queue)
        queue = new QQueue<int>;
    if (!lunarCache)
        lunarCache = new QMap<QDate, CaHuangLiDayInfo>;
    if (!emptyCaHuangLiDayInfo)
        emptyCaHuangLiDayInfo = new CaHuangLiDayInfo;

    m_weeklist.append(tr("Monday"));
    m_weeklist.append(tr("Tuesday"));
    m_weeklist.append(tr("Wednesday"));
    m_weeklist.append(tr("Thursday"));
    m_weeklist.append(tr("Friday"));
    m_weeklist.append(tr("Saturday"));
    m_weeklist.append(tr("Sunday"));
    initUI();
    initConnection();
}

CDayMonthView::~CDayMonthView()
{
    if (lunarCache !=nullptr) {
        delete lunarCache;
        lunarCache = nullptr;
    }
    if ( queue !=nullptr) {
        delete queue;
        queue = nullptr;
    }
    if (emptyCaHuangLiDayInfo != nullptr) {
        delete emptyCaHuangLiDayInfo;
        emptyCaHuangLiDayInfo = nullptr;
    }
}
void CDayMonthView::handleCurrentDateChanged(const QDate date, const CaHuangLiDayInfo &detail)
{
    Q_UNUSED(detail);
    if (date != m_currentDate) {
        setCurrentDate(date);
    }
}

void CDayMonthView::setFirstWeekday(int weekday)
{
    m_firstWeekDay = weekday;
    updateDate();
}
void CDayMonthView::setLunarVisible(bool visible)
{
    m_huanglistate = visible;
    m_yiLabel->setVisible(visible);
    m_jiLabel->setVisible(visible);
    m_currentLuna->setVisible(visible);
    m_splitline->setVisible(visible);
    update();
}

void CDayMonthView::setTheMe(int type)
{
    m_themetype = type;
    if (type == 0 || type == 1) {
        DPalette anipa = this->palette();
        QColor tbcolor = "#FFFFFF";
        anipa.setColor(DPalette::Background, tbcolor);
        setPalette(anipa);
        setBackgroundRole(DPalette::Background);
        setBColor(tbcolor);

        DPalette todaypa = m_today->palette();
        QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        todaypa.setColor(DPalette::ButtonText, todayColor);
        todaypa.setColor(DPalette::Dark, Qt::white);
        todaypa.setColor(DPalette::Light, Qt::white);
        QColor sbcolor("#002A57");
        sbcolor.setAlphaF(0.05);
        todaypa.setColor(DPalette::Shadow, sbcolor);
        m_today->setPalette(todaypa);
        QColor todayhover = "#000000";
        todayhover.setAlphaF(0.1);
        QColor todaypress = "#000000";
        todaypress.setAlphaF(0.2);
        m_today->setBColor("#FFFFFF", todayhover, todaypress, "#FFFFFF", todayhover, todaypress);
        m_today->setTColor(todayColor, "#001A2E", "#0081FF");
        m_today->setshadowColor(sbcolor);
        DPalette prevpa = m_prevButton->palette();
        prevpa.setColor(DPalette::Dark, QColor("#E6E6E6"));
        prevpa.setColor(DPalette::Light, QColor("#E3E3E3"));

        DPalette nextvpa = m_nextButton->palette();
        nextvpa.setColor(DPalette::Dark, QColor("#E6E6E6"));
        nextvpa.setColor(DPalette::Light, QColor("#E3E3E3"));

        m_currentMouth->setTextColor(QColor("#3B3B3B"));

        m_currentDay->setTextColor(todayColor);

        m_currentWeek->setTextColor(QColor("#414D68"));

        m_currentLuna->setTextColor(QColor("#414D68"));

        m_currentYear->setTextColor(QColor("#414D68"));

        QFont hlabelF;
        hlabelF.setPixelSize(DDECalendar::FontSizeFourteen);

        QColor yicolor = QColor("#75C18E");
        yicolor.setAlphaF(0.1);
        m_yiLabel->setbackgroundColor(yicolor);
        m_yiLabel->setTextInfo(QColor("#7B7B7B"), hlabelF);
        m_yiLabel->update();

        QColor jicolor = QColor("#C17575");
        jicolor.setAlphaF(0.1);
        m_jiLabel->setbackgroundColor(jicolor);
        m_jiLabel->setTextInfo(QColor("#7B7B7B"), hlabelF);
        m_jiLabel->update();

        m_topBorderColor = Qt::red;
        m_backgroundCircleColor = "#0081FF";

        m_defaultTextColor = Qt::black;
        m_currentDayTextColor = todayColor;
        m_weekendsTextColor = Qt::black;
        m_selectedTextColor = Qt::white;
        m_festivalTextColor = Qt::black;
        m_notCurrentTextColor = "#b2b2b2";
        m_ceventColor = QColor(255, 93, 0);
    } else if (type == 2) {
        DPalette anipa = this->palette();
        QColor tbcolor = "#282828";
        anipa.setColor(DPalette::Background, tbcolor);
        setPalette(anipa);
        setBackgroundRole(DPalette::Background);
        setBColor(tbcolor);

        DPalette todaypa = m_today->palette();
        QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        todaypa.setColor(DPalette::ButtonText, todayColor);
        todaypa.setColor(DPalette::Light, "#484848");
        todaypa.setColor(DPalette::Dark, "#414141");
        QColor sbcolor("#000000");
        sbcolor.setAlphaF(0.05);
        todaypa.setColor(DPalette::Shadow, sbcolor);
        m_today->setPalette(todaypa);
        m_today->setBColor("#484848", "#727272", "#242424", "#414141", "#535353", "#282828");
        m_today->setTColor(todayColor, "#FFFFFF", "#0081FF");
        m_today->setshadowColor(sbcolor);
        DPalette prevpa = m_prevButton->palette();
        prevpa.setColor(DPalette::Dark, QColor("#484848"));
        prevpa.setColor(DPalette::Light, QColor("#414141"));
        DPalette nextvpa = m_nextButton->palette();
        nextvpa.setColor(DPalette::Dark, QColor("#484848"));
        nextvpa.setColor(DPalette::Light, QColor("#414141"));

        m_currentMouth->setTextColor(QColor("#C0C6D4"));

        m_currentDay->setTextColor(todayColor);

        m_currentWeek->setTextColor(QColor("#C0C6D4"));

        m_currentLuna->setTextColor(QColor("#C0C6D4"));

        m_currentYear->setTextColor(QColor("#C0C6D4"));
        QFont hlabelF;
        hlabelF.setPixelSize(DDECalendar::FontSizeFourteen);

        QColor yicolor = QColor("#2F8C4D");
        yicolor.setAlphaF(0.2);
        m_yiLabel->setbackgroundColor(yicolor);
        m_yiLabel->setTextInfo(QColor("#C0C6D4"), hlabelF);
        m_yiLabel->update();

        QColor jicolor = QColor("#A43B3B");
        jicolor.setAlphaF(0.2);
        m_jiLabel->setbackgroundColor(jicolor);
        m_jiLabel->setTextInfo(QColor("#C0C6D4"), hlabelF);
        m_jiLabel->update();

        m_topBorderColor = Qt::red;
        m_backgroundCircleColor = "#0059D2";

        m_defaultTextColor = "#C0C6D4";
        m_currentDayTextColor = todayColor;
        m_weekendsTextColor = Qt::black;
        m_selectedTextColor = "#B8D3FF";
        m_festivalTextColor = Qt::black;
        m_notCurrentTextColor = "#C0C6D4";
        m_notCurrentTextColor.setAlphaF(0.5);
        m_ceventColor = QColor(204, 77, 3);
    }

    for (int i(0); i != DDEDayCalendar::PainterCellNum; ++i) {
        m_cellList.at(i)->update();
    }
}

void CDayMonthView::setSearchFlag(bool flag)
{
    m_searchflag = flag;
    update();
}

void CDayMonthView::updateFlag()
{
    getlineflag();
    update();
}
void CDayMonthView::setCurrentDate(const QDate date, int type)
{
    Q_UNUSED(type);

    if (date.year() < DDECalendar::QueryEarliestYear)
        return;

    if (date == m_currentDate) {
        return;
    }

    if (date == QDate::currentDate()) {
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
    }

    m_currentDate = date;
    updateCurrentLunar();
}

int CDayMonthView::getDateIndex(const QDate &date) const
{
    for (int i = 0; i != DDEDayCalendar::PainterCellNum; ++i)
        if (m_days[i] == date)
            return i;

    return 0;
}

bool CDayMonthView::eventFilter(QObject *o, QEvent *e)
{
    QWidget *cell = qobject_cast<QWidget *>(o);

    if (cell && m_cellList.contains(cell)) {
        const int pos = m_cellList.indexOf(cell);
        QDate date = m_days[pos];

        if (date.year() < DDECalendar::QueryEarliestYear)
            return false;

        if (e->type() == QEvent::Paint) {
            paintCell(cell);
        } else if (e->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(e);
            if (mouseEvent->button() ==Qt::LeftButton) {
                cellClicked(cell);
            }
        }
    }
    return false;
}

void CDayMonthView::updateDate()
{
    const QDate firstDay(m_currentDate.year(), m_currentDate.month(), 1);
    const int day = (firstDay.dayOfWeek() + m_firstWeekDay) % 7;
    const int currentIndex = day + m_currentDate.day() - 1;

    if (currentIndex < 0) {
        return;
    }

    for (int i(0); i != DDEDayCalendar::PainterCellNum; ++i) {
        m_days[i] = firstDay.addDays(i - day);
    }

    setSelectedCell(currentIndex);
    getlineflag();
    update();
}

void CDayMonthView::getlineflag()
{
    QLocale locale;
    CScheduleDataManage *tdataManage = CScheduleDataManage::getScheduleDataManage();
    m_vlineflag.resize(DDEDayCalendar::PainterCellNum);
    m_vlineflag.fill(false);
    QVector<ScheduleDateRangeInfo> out;

    if (tdataManage->getscheduleDataCtrl()->getScheduleInfo(m_days[0], m_days[41], out)) {
        if (out.count() == DDEDayCalendar::PainterCellNum)
            for (int i = 0; i < DDEDayCalendar::PainterCellNum; i++) {
                if (!out.at(i).vData.isEmpty()) {
                    m_vlineflag[i] = true;
                }
            }
    }
}

void CDayMonthView::initUI()
{
    m_dayNumFont.setPixelSize(DDECalendar::FontSizeTwelve);

    m_today = new CTodayButton;

    m_today->setText(QCoreApplication::translate("today", "Today", "Today"));

    m_today->setFixedSize(100, DDEDayCalendar::D_MLableHeight);
    DPalette todaypa = m_today->palette();
    QColor todayColor = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
    todaypa.setColor(DPalette::ButtonText, todayColor);
    todaypa.setColor(DPalette::Dark, Qt::white);
    todaypa.setColor(DPalette::Light, Qt::white);
    QColor sbcolor("#002A57");
    sbcolor.setAlphaF(0.05);
    todaypa.setColor(DPalette::Shadow, sbcolor);
    QFont todayfont;
    todayfont.setPixelSize(DDECalendar::FontSizeFourteen);
    m_today->setFont(todayfont);
    m_today->setPalette(todaypa);
    m_prevButton = new DIconButton(DStyle::SP_ArrowLeft, this);
    m_prevButton->setFixedSize(36, 36);

    m_nextButton = new DIconButton(DStyle::SP_ArrowRight, this);
    m_nextButton->setFixedSize(36, 36);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 3);
    //add separator line
    m_currentMouth = new CustomFrame(this);
    m_currentMouth->setFixedSize(74, DDEDayCalendar::D_MLableHeight);
    QFont mlabelF;
    mlabelF.setWeight(QFont::Medium);
    mlabelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_currentMouth->setTextFont(mlabelF);
    m_currentMouth->setTextAlign(Qt::AlignCenter);
    titleLayout->addWidget(m_prevButton);
    titleLayout->addWidget(m_currentMouth);
    titleLayout->addWidget(m_nextButton);
    titleLayout->addStretch();
    titleLayout->addWidget(m_today, 0, Qt::AlignRight);
    // cells grid
    m_gridLayout = new QGridLayout;
    m_gridLayout->setMargin(0);
    m_gridLayout->setSpacing(0);

    for (int r = 0; r != 6; ++r) {
        for (int c = 0; c != 7; ++c) {
            QWidget *cell = new QWidget;
            cell->setFixedSize(DDEDayCalendar::DCellWidth, DDEDayCalendar::DCellHeight);
            cell->installEventFilter(this);
            cell->setFocusPolicy(Qt::ClickFocus);
            m_gridLayout->addWidget(cell, r, c);
            m_cellList.append(cell);
        }
    }
    //上半部分
    m_upLayout = new QVBoxLayout;
    m_upLayout->setMargin(0);
    m_upLayout->setSpacing(0);
    m_upLayout->setContentsMargins(22, 9, 0, 7);
    m_upLayout->addLayout(titleLayout);
    m_upLayout->addLayout(m_gridLayout);

    //中间部分
    QVBoxLayout *midLayout = new QVBoxLayout;
    midLayout->setMargin(0);
    midLayout->setSpacing(0);
    midLayout->setContentsMargins(0, 0, 0, 20);
    m_currentDay = new CustomFrame(this);
    m_currentDay->setFixedHeight(DDEDayCalendar::DDLableHeight);
    m_currentDay->setMinimumWidth(width());
    m_currentDay->setTextAlign(Qt::AlignCenter);
    QFont daylabelF;
    daylabelF.setWeight(QFont::Medium);
    daylabelF.setPixelSize(DDECalendar::FontSizeOneHundred);
    m_currentDay->setTextFont(daylabelF);
    midLayout->addWidget(m_currentDay);

    m_currentWeek = new CustomFrame(this);
    m_currentWeek->setFixedHeight(DDEDayCalendar::DWLableHeight);
    m_currentWeek->setTextAlign(Qt::AlignCenter);
    QFont wlabelF;
    wlabelF.setPixelSize(DDECalendar::FontSizeSixteen);
    m_currentWeek->setTextFont(wlabelF);
    midLayout->addWidget(m_currentWeek);
    midLayout->addSpacing(2);

    m_currentYear = new CustomFrame(this);
    m_currentYear->setFixedHeight(DDEDayCalendar::DWLableHeight);
    m_currentYear->setTextAlign(Qt::AlignCenter);
    m_currentYear->setTextFont(wlabelF);
    midLayout->addWidget(m_currentYear);
    midLayout->addSpacing(2);

    m_currentLuna = new CustomFrame(this);
    m_currentLuna->setFixedHeight(DDEDayCalendar::DHualiInfoLableHeight);
    m_currentLuna->setTextAlign(Qt::AlignCenter);
    QFont hlabelF;
    hlabelF.setPixelSize(DDECalendar::FontSizeTwelve);
    m_currentLuna->setTextFont(hlabelF);
    midLayout->addWidget(m_currentLuna);

    m_yidownLayout = new QVBoxLayout;
    m_yidownLayout->setMargin(0);
    m_yidownLayout->setSpacing(0);
    m_yidownLayout->setContentsMargins(10, 5, 10, 0);
    hlabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_yiLabel = new CDayHuangLiLabel(this);
    m_yiLabel->setbackgroundColor(QColor("#75C18E"));
    m_yiLabel->setTextInfo(QColor("#7B7B7B "), hlabelF);
    m_yiLabel->setFixedSize(DDEDayCalendar::DHualiDtailLableWidth, DDEDayCalendar::DHualiDtailLableHeight);
    m_yidownLayout->addWidget(m_yiLabel);

    m_jidownLayout = new QVBoxLayout;
    m_jidownLayout->setMargin(0);
    m_jidownLayout->setSpacing(0);
    m_jidownLayout->setContentsMargins(10, 10, 10, 10);

    m_jiLabel = new CDayHuangLiLabel(this);
    m_jiLabel->setbackgroundColor(QColor("#C17575"));
    m_jiLabel->setTextInfo(QColor("#7B7B7B "), hlabelF);
    m_jiLabel->setFixedSize(DDEDayCalendar::DHualiDtailLableWidth, DDEDayCalendar::DHualiDtailLableHeight);
    m_jidownLayout->addWidget(m_jiLabel);

    m_hhLayout = new QVBoxLayout;
    m_hhLayout->setMargin(0);
    m_hhLayout->setSpacing(0);
    m_hhLayout->addLayout(m_upLayout);
    m_hhLayout->addLayout(midLayout);

    m_splitline = new DHorizontalLine;

    m_splitline->setFixedSize(241, 2);
    QHBoxLayout *hlineLayout = new QHBoxLayout;
    hlineLayout->setMargin(0);
    hlineLayout->setSpacing(0);
    hlineLayout->setContentsMargins(0, 0, 0, 3);
    hlineLayout->addStretch(1);
    hlineLayout->addWidget(m_splitline);
    hlineLayout->addStretch(1);
    m_hhLayout->addLayout(hlineLayout);
    m_hhLayout->addLayout(m_yidownLayout);
    m_hhLayout->addLayout(m_jidownLayout);
    m_hhLayout->addStretch();

    setLayout(m_hhLayout);
}

void CDayMonthView::initConnection()
{
    connect(m_prevButton, &DIconButton::clicked, this, &CDayMonthView::slotprev);
    connect(m_today, &CTodayButton::clicked, this, &CDayMonthView::slottoday);
    connect(m_nextButton, &DIconButton::clicked, this, &CDayMonthView::slotnext);
    connect(this, &CDayMonthView::dateSelected, this, &CDayMonthView::handleCurrentDateChanged);
}

void CDayMonthView::updateDateShow()
{
    QLocale locale;
    m_currentMouth->setTextStr(locale.monthName(m_currentDate.month(), QLocale::ShortFormat));
    m_currentDay->setTextStr(QString::number(m_currentDate.day()));

    if (m_currentDate.dayOfWeek() > 0)
        m_currentWeek->setTextStr(m_weeklist.at(m_currentDate.dayOfWeek() - 1));
    m_currentYear->setTextStr(m_currentDate.toString("yyyy/M"));
}

void CDayMonthView::updateDateLunarDay()
{
    if (!m_huanglistate) return;
    CaHuangLiDayInfo info = getCaHuangLiDayInfo(getDateIndex(m_currentDate));
    m_currentLuna->setTextStr(info.mGanZhiYear + "年 " + "【" + info.mZodiac + "年】" + info.mGanZhiMonth + "月 " + info.mGanZhiDay + "日 ");
    QStringList yilist = info.mSuit.split(".", QString::SkipEmptyParts);
    QStringList jilist = info.mAvoid.split(".", QString::SkipEmptyParts);
    m_yiLabel->setHuangLiText(yilist);
    m_jiLabel->setHuangLiText(jilist, 1);
}
const QString CDayMonthView::getCellDayNum(int pos)
{
    return QString::number(m_days[pos].day());
}

const QDate CDayMonthView::getCellDate(int pos)
{
    return m_days[pos];
}
void CDayMonthView::paintCell(QWidget *cell)
{
    const QRect rect(0, 0, cellwidth, cellheight);

    const int pos = m_cellList.indexOf(cell);
    const bool isSelectedCell = pos == m_selectedCell;
    const bool isCurrentDay = getCellDate(pos) == QDate::currentDate();

    QPainter painter(cell);

    // draw selected cell background circle
    if (isSelectedCell) {
        int min = 0;
        //高度和宽度最小的一个
        min = cell->height() < cell->width() ? cell->height():cell->width();
        //最终rect
        QRectF fillRect;
        //宽度小于高度时的rect
        QRectF rectByWidth = QRectF(cell->width() * 0.15, (cell->height() - cell->width() * 0.75) / 2.0, min * 0.7, min * 0.7);
        //高度小于宽度时的rect
        QRectF rectByHeight = QRectF((cell->width() - cell->height() * 0.75) / 2.0, cell->height() * 0.15, min * 0.7, min * 0.7);
        //判断最终rect是哪一个
        fillRect = cell->height() < cell->width() ? rectByHeight : rectByWidth;
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QBrush(CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor()));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(fillRect);
        painter.restore();
    }

    painter.setPen(Qt::SolidLine);

    const QString dayNum = getCellDayNum(pos);

    // draw text of day
    if (isSelectedCell) {
        painter.setPen(m_selectedTextColor);
    } else if (isCurrentDay) {
        painter.setPen(m_currentDayTextColor);
    } else {
        if (m_currentDate.month() == getCellDate(pos).month())
            painter.setPen(m_defaultTextColor);
        else
            painter.setPen(m_notCurrentTextColor);
    }

    QRect test;
    painter.setFont(m_dayNumFont);

    painter.drawText(rect, Qt::AlignCenter, dayNum, &test);

    if (m_vlineflag.count() == DDEDayCalendar::PainterCellNum) {
        if (m_vlineflag[pos]) {
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::HighQualityAntialiasing);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            QPen pen;
            pen.setWidth(2);
            pen.setColor(m_ceventColor);
            painter.setPen(pen);
            painter.setBrush(QBrush(m_ceventColor));
            painter.setPen(Qt::NoPen);
            int r = cell->width() * (4 / 25);

            if (r < 4) {
                r = 4;
            } else if ( r > 7) {
                r = 7;
            }
            painter.drawEllipse(cell->width() - r - 6, 4, r, r);
            painter.restore();
        }
    }

    painter.end();
}

void CDayMonthView::cellClicked(QWidget *cell)
{
    if (!m_cellSelectable)
        return;

    const int pos = m_cellList.indexOf(cell);

    if (pos == -1)
        return;

    setSelectedCell(pos);
}

void CDayMonthView::setSelectedCell(int index)
{
    if (m_selectedCell == index)
        return;

    const int prevPos = m_selectedCell;
    m_selectedCell = index;

    updateDate();
    m_cellList.at(prevPos)->update();
    m_cellList.at(index)->update();

    if (m_currentDate.year() < DDECalendar::QueryEarliestYear)
        return;
    emit signalcurrentDateChanged(m_days[index]);
}
void CDayMonthView::updateCurrentLunar()
{
    updateDate();
    updateDateShow();
    updateDateLunarDay();
    emit signalcurrentLunarDateChanged(m_currentDate, getCaHuangLiDayInfo(getDateIndex(m_currentDate)), 1);
}
const QString CDayMonthView::getLunar(int pos)
{
    CaHuangLiDayInfo info = getCaHuangLiDayInfo(pos);

    if (info.mLunarDayName == "初一") {
        info.mLunarDayName = info.mLunarMonthName + info.mLunarDayName;
    }

    if (info.mTerm.isEmpty())
        return info.mLunarDayName;

    return info.mTerm;
}

const CaHuangLiDayInfo CDayMonthView::getCaHuangLiDayInfo(int pos)
{
    const QDate date = m_days[pos];

    if (lunarCache->contains(date)) {
        return lunarCache->value(date);
    }

    if (lunarCache->size() > 40)
        lunarCache->clear();

    queue->push_back(pos);

    QTimer::singleShot(300, this, SLOT(getDbusData()));

    return *emptyCaHuangLiDayInfo;
}

void CDayMonthView::getDbusData()
{
    if (queue->isEmpty())
        return;

    const int pos = queue->head();
    queue->pop_front();
    const QDate date = m_days[pos];
    if (!date.isValid()) {
        return;
    }

    CaHuangLiDayInfo currentDayInfo;

    if (!lunarCache->contains(date)) {
        CaHuangLiDayInfo scurrentDayinfo;
        if (m_DBusInter->GetHuangLiDayCalendar(date.year(), date.month(), date.day(), scurrentDayinfo)) {
            lunarCache->insert(date, scurrentDayinfo);
            currentDayInfo = scurrentDayinfo;
        }


    } else {
        currentDayInfo = lunarCache->value(date);
    }

    m_cellList.at(pos)->update();

    // refresh   lunar info
    if (date == m_currentDate) {
        updateCurrentLunar();
    }
}

void CDayMonthView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    cellwidth = qRound(width() * 0.1005 + 0.5);
    cellheight = qRound(height() * 0.0496 + 0.5);
    m_gridLayout->setHorizontalSpacing(qRound(width() * 0.0287 + 0.5));
    m_gridLayout->setVerticalSpacing(0);
    int leftmagin = qRound(width() * 0.0332 + 0.5);
    int rightmagin = leftmagin;
    int topmagin = qRound(height() * 0.0164 + 0.5);
    int buttonmagin = topmagin;
    m_upLayout->setContentsMargins(leftmagin, topmagin, rightmagin, buttonmagin);
    m_dayNumFont.setPixelSize(qRound(12 + (width() - 347) / 71.66));

    for (int i(0); i != DDEDayCalendar::PainterCellNum; ++i) {
        m_cellList.at(i)->setFixedSize(cellwidth, cellheight);
        m_cellList.at(i)->update();
    }
    m_splitline->setFixedWidth(qRound(0.6925 * width() + 0.5));

    int hleftmagin = qRound(width() * 0.026 + 0.5);
    int hrightmagin = hleftmagin;
    int htopmagin = qRound(height() * 0.01773 + 0.5);
    int hbuttonmagin = htopmagin;
    int lw = width() - hleftmagin * 2;
    int lh = qRound(height() * 0.0992);
    m_yiLabel->setFixedSize(lw, lh);
    m_yidownLayout->setContentsMargins(hleftmagin, qRound(htopmagin * 0.5), hrightmagin, 0);
    m_jiLabel->setFixedSize(lw, lh);
    m_jidownLayout->setContentsMargins(hleftmagin, htopmagin, hrightmagin, hbuttonmagin);
}

void CDayMonthView::wheelEvent(QWheelEvent *event)
{
    //如果是拖拽则退出
    bool isDragging = false;
    emit signalIsDragging(isDragging);

    if (isDragging)
        return;
    if (event->delta() < 0) {
        m_currentDate = m_currentDate.addDays(1);

        if (m_currentDate == QDate::currentDate()) {
            m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
        } else {
            m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
        }
        emit signalcurrentDateChanged(m_currentDate);
        updateCurrentLunar();
    } else {
        QDate t_curret = m_currentDate.addDays(-1);

        if (t_curret.year() < DDECalendar::QueryEarliestYear)
            return;

        if (m_currentDate.year() >= DDECalendar::QueryEarliestYear) {
            m_currentDate = t_curret;

            if (m_currentDate == QDate::currentDate()) {
                m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
            } else {
                m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
            }
            emit signalcurrentDateChanged(m_currentDate);
            updateCurrentLunar();
        } else {
        }
    }
}

void CDayMonthView::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    int labelwidth = width();
    int labelheight = height();
    DPalette anipa = this->palette();
    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setBrush(anipa.background());
    painter.setPen(Qt::NoPen);
    QPainterPath painterPath;
    painterPath.moveTo(m_radius, 0);

    painterPath.lineTo(0, 0);
    painterPath.lineTo(0, m_radius);

    painterPath.lineTo(0, labelheight - m_radius);

    painterPath.lineTo(0, labelheight);
    painterPath.lineTo(m_radius, labelheight);

    painterPath.lineTo(labelwidth - m_radius, labelheight);

    if (!m_searchflag) {
        painterPath.arcTo(QRect(labelwidth - m_radius * 2, labelheight - m_radius * 2, m_radius * 2, m_radius * 2), 270, 90);
    } else {
        painterPath.lineTo(labelwidth, labelheight);
        painterPath.lineTo(labelwidth, labelheight - m_radius);
    }
    painterPath.lineTo(labelwidth, m_radius);

    if (!m_searchflag) {

        painterPath.arcTo(QRect(labelwidth - m_radius * 2, 0, m_radius * 2, m_radius * 2), 0, 90);

    } else {
        painterPath.lineTo(labelwidth, 0);
        painterPath.lineTo(labelwidth - m_radius, 0);
    }
    painterPath.lineTo(m_radius, 0);
    painterPath.closeSubpath();
    painter.drawPath(painterPath);
    painter.restore();
}
void CDayMonthView::slotprev()
{
    emit signalSchedulHide();

    if (m_currentDate.year() == DDECalendar::QueryEarliestYear && m_currentDate.month() == 1)
        return;

    if (m_currentDate.year() >= DDECalendar::QueryEarliestYear) {
        m_currentDate = m_currentDate.addMonths(-1);

        if (m_currentDate == QDate::currentDate()) {
            m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
        } else {
            m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
        }
        emit signalcurrentDateChanged(m_currentDate);
        updateCurrentLunar();
    } else {
    }
}

void CDayMonthView::slotnext()
{
    emit signalSchedulHide();
    m_currentDate = m_currentDate.addMonths(1);

    if (m_currentDate == QDate::currentDate()) {
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        m_today->setText(QCoreApplication::translate("Return Today", "Today", "Return Today"));
    }
    emit signalcurrentDateChanged(m_currentDate);
    updateCurrentLunar();
}

void CDayMonthView::slottoday()
{
    emit signalSchedulHide();
    emit signalsReturnDay();
    setCurrentDate(QDate::currentDate());
}
