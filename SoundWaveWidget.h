#ifndef SOUNDWAVEWIDGET_H
#define SOUNDWAVEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QtMath>

class SoundWaveWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SoundWaveWidget(QWidget *parent = nullptr);

    // Метод для обновления уровня громкости (от 0.0 до 1.0)
    void setVolumeLevel(double level);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    double m_currentLevel;// Текущий уровень громкости (0.0 - 1.0)
    QTimer *m_updateTimer;
    QLinearGradient m_gradient;

    void drawVU(QPainter &painter);
};

#endif // SOUNDWAVEWIDGET_H
