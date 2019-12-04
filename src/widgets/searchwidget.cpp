/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
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

#include "searchwidget.h"
#include "src/global_util/util.h"

#include <QHBoxLayout>
#include <QEvent>
#include <QDebug>
#include <QKeyEvent>
#include <dimagebutton.h>
#include <DDBusSender>

#define BTN_WIDTH   40
#define BTN_HEIGHT  40

DWIDGET_USE_NAMESPACE

SearchWidget::SearchWidget(QWidget *parent) :
    QFrame(parent)
{
    m_leftSpacing = new QFrame(this);
    m_rightSpacing = new QFrame(this);

    m_leftSpacing->setFixedWidth(0);
    m_rightSpacing->setFixedWidth(0);

    m_toggleCategoryBtn = new DFloatingButton(this);
    m_toggleCategoryBtn->setAccessibleName("mode-toggle-button");
    m_toggleCategoryBtn->setIcon(QIcon(":/icons/skin/icons/category_normal_22px.png"));
    m_toggleCategoryBtn->setIconSize(QSize(BTN_WIDTH, BTN_HEIGHT));
    m_toggleCategoryBtn->setFixedSize(QSize(BTN_WIDTH, BTN_HEIGHT));
    m_toggleCategoryBtn->setBackgroundRole(DPalette::Button);

    m_toggleModeBtn = new DFloatingButton(this);
    m_toggleModeBtn->setIcon(QIcon(":/icons/skin/icons/unfullscreen_normal.png"));
    m_toggleModeBtn->setIconSize(QSize(BTN_WIDTH, BTN_HEIGHT));
    m_toggleModeBtn->setFixedSize(QSize(BTN_WIDTH, BTN_HEIGHT));
    m_toggleModeBtn->setBackgroundRole(DPalette::Button);

    m_searchEdit = new SearchLineEdit(this);
    m_searchEdit->setAccessibleName("search-edit");
    m_searchEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_searchEdit->setFixedWidth(290);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    mainLayout->addSpacing(30);
    mainLayout->addWidget(m_leftSpacing);
    mainLayout->addWidget(m_toggleCategoryBtn);
    mainLayout->addStretch();
    mainLayout->addWidget(m_searchEdit);
    mainLayout->addStretch();
    mainLayout->addWidget(m_toggleModeBtn);
    mainLayout->addWidget(m_rightSpacing);
    mainLayout->addSpacing(30);

    setLayout(mainLayout);

    connect(m_searchEdit, &SearchLineEdit::textChanged, [this] {
        emit searchTextChanged(m_searchEdit->text());
    });

    connect(m_toggleModeBtn, &DIconButton::clicked, this, [=] {

#if (DTK_VERSION >= DTK_VERSION_CHECK(2, 0, 8, 0))
        DDBusSender()
        .service("com.deepin.dde.daemon.Launcher")
        .interface("com.deepin.dde.daemon.Launcher")
        .path("/com/deepin/dde/daemon/Launcher")
        .property("Fullscreen")
        .set(false);
#else
        const QStringList args{
            "--print-reply",
            "--dest=com.deepin.dde.daemon.Launcher",
            "/com/deepin/dde/daemon/Launcher",
            "org.freedesktop.DBus.Properties.Set",
            "string:com.deepin.dde.daemon.Launcher",
            "string:Fullscreen",
            "variant:boolean:false"
        };

        QProcess::startDetached("dbus-send", args);
#endif
    });
    connect(m_toggleCategoryBtn, &DIconButton::clicked, this, &SearchWidget::toggleMode);
}

QLineEdit *SearchWidget::edit()
{
    return m_searchEdit;
}

void SearchWidget::clearSearchContent()
{
    m_searchEdit->normalMode();
    m_searchEdit->moveFloatWidget();
}

void SearchWidget::setLeftSpacing(int spacing)
{
    m_leftSpacing->setFixedWidth(spacing);
}

void SearchWidget::setRightSpacing(int spacing)
{
    m_rightSpacing->setFixedWidth(spacing);
}

void SearchWidget::showToggle()
{
    m_toggleCategoryBtn->show();
    m_toggleModeBtn->show();
}

void SearchWidget::hideToggle()
{
    m_toggleCategoryBtn->hide();
    m_toggleModeBtn->hide();
}
