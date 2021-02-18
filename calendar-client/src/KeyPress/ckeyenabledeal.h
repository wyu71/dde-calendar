/*
   * Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
   *
   * Author:     chenhaifeng <chenhaifeng@uniontech.com>
   *
   * Maintainer: chenhaifeng <chenhaifeng@uniontech.com>
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
#ifndef CKEYENABLEDEAL_H
#define CKEYENABLEDEAL_H

#include "ckeypressdealbase.h"

#include <QDate>
#include <QWidget>

/**
 * @brief The CKeyEnableDeal class
 * 回车键处理
 */
class CKeyEnableDeal : public CKeyPressDealBase
{
public:
    explicit CKeyEnableDeal(QGraphicsScene *scene = nullptr);
protected:
    //焦点项处理
    bool focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene) override;
private:
    //创建日程
    void createSchedule(const QDate &createDate, QWidget *parent);
};

#endif // CKEYENABLEDEAL_H
