#include "SoundWaveWidget.h"

SoundWaveWidget::SoundWaveWidget(QWidget *parent)
    : QWidget(parent), m_currentLevel(0.0)
{
    m_gradient = QLinearGradient(0, 0, width(), 0);
    m_gradient.setColorAt(0.0, QColor(0, 150, 0));// Зеленый
    m_gradient.setColorAt(0.7, QColor(255, 200, 0));// Желтый
    m_gradient.setColorAt(1.0, QColor(255, 0, 0));// Красный

    // Таймер для плавного обновления (имитация эффекта "оживления")
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, [this]()
    {
        if(m_currentLevel > 0.01)
        {
            m_currentLevel *= 0.9; // Затухание
            update();// Перерисовка
        }
        else if(m_currentLevel <= 0.01 && m_currentLevel>0)
        {
            m_currentLevel = 0.0;
            update();
        }
    });
    m_updateTimer->start(30);// Обновление каждые 30 мс
}

void SoundWaveWidget::resizeEvent(QResizeEvent *event)
{
    // Обновляем градиент, чтобы он всегда занимал всю доступную ширину виджета
    m_gradient = QLinearGradient(0, 0, width(), 0);
    m_gradient.setColorAt(0.0, QColor(0, 150, 0));// Зеленый
    m_gradient.setColorAt(0.7, QColor(255, 200, 0));// Желтый
    m_gradient.setColorAt(1.0, QColor(255, 0, 0));// Красный

    // Вызываем базовую реализацию
    QWidget::resizeEvent(event);
}

void SoundWaveWidget::setVolumeLevel(double level)
{
    // Убеждаемся, что уровень не превышает 1.0
    m_currentLevel = qMin(1.0, level);
    update();
}

void SoundWaveWidget::drawVU(QPainter &painter)
{
    //сглаживание
    painter.setRenderHint(QPainter::Antialiasing);
    int h = height();
    int w = width();

    // 1. Рисуем фоновый прямоугольник (по умолчанию черный/серый)
    painter.fillRect(rect(), Qt::black);

    // 2. Рисуем градиентную полосу (VU Meter)
        // Ширина полосы пропорциональна уровню громкости
    int fillWidth = static_cast<int>(w * m_currentLevel);

    if(fillWidth > 0)
    {
        // Обновляем градиент с учетом текущей ширины
        m_gradient.setFinalStop(fillWidth, 0);
        QBrush gradientBrush(m_gradient);
        painter.setBrush(gradientBrush);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, 0, fillWidth, h);
    }
}

void SoundWaveWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    drawVU(painter);
}
