#include "blurboxwidget.h"
#include "src/global_util/calculate_util.h"
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE
BlurBoxWidget::BlurBoxWidget(QWidget *parent)
    : DBlurEffectWidget(parent)
    , qvLayout(new QVBoxLayout)
    , m_maskLayer(new QWidget(parent))
    , m_calcUtil(CalculateUtil::instance())
{
    setMaskColor(DBlurEffectWidget::AutoColor);
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setMaskAlpha(DLauncher::APPHBOX_ALPHA);
    setBlurRectXRadius(DLauncher::APPHBOX_RADIUS);
    setBlurRectYRadius(DLauncher::APPHBOX_RADIUS);
    setFixedWidth(m_calcUtil->getAppBoxSize().width());

//    QPalette pal(m_maskLayer->palette());
//    pal.setColor(QPalette::Background, QColor(0x0, 0x0, 0x0, 40));
//    m_maskLayer->setAutoFillBackground(true);
//    m_maskLayer->setPalette(pal);
//    m_maskLayer->setFixedSize(size());
    //m_maskLayer->show();
    //m_maskLayer->setParent(parent);

    setLayout(qvLayout);
    qvLayout->setSpacing(10);
    qvLayout->setAlignment(Qt::AlignTop);

    initconnect();

}

void BlurBoxWidget::initconnect()
{

}

void BlurBoxWidget::layoutAddWidget(QWidget *child)
{
    qvLayout->addWidget(child);
}

void BlurBoxWidget::layoutAddWidget(QWidget *child, int stretch, Qt::Alignment alignment)
{
    qvLayout->addWidget(child, stretch, alignment);
}

void BlurBoxWidget::mousePressEvent(QMouseEvent *ev)
{
    mousePos = QPoint(ev->x(), ev->y());
}

void BlurBoxWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    if(mousePos == QPoint(ev->x(), ev->y())) emit maskClick(getCategory());

}

void BlurBoxWidget::setMaskSize(QSize size)
{
    m_maskLayer->setFixedSize(size);
}

void BlurBoxWidget::setMaskVisible(bool visible)
{
    m_maskLayer->setVisible(visible);
}


