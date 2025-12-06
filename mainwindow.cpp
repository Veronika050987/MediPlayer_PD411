#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStyle>
#include <QFileDialog>
#include <QUrl>//подключение файла
#include <QDateTime>// Для форматирования времени
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // buttons style;
    ui->pushButtonAdd->setIcon(style()->standardIcon(QStyle::SP_DriveCDIcon));
    ui->pushButtonPrev->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    ui->pushButtonNext->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    ui->pushButtonPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->pushButtonPause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    ui->pushButtonStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->pushButtonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));

    ui->horizontalSliderVolume->setRange(0,100);

    //PlayerInit:
    m_player = new QMediaPlayer();
    m_player->setVolume(70);
    ui->labelVolume->setText(QString("Volume: ").append(QString::number(m_player->volume())));
    ui->horizontalSliderVolume->setValue(m_player->volume());

    // Подключение сигналов плеера
    connect(ui->horizontalSliderVolume, &QSlider::valueChanged, this, &MainWindow::on_horizontalSliderVolume_valueChanged);

    // Подключение сигналов времени
    connect(m_player, &QMediaPlayer::positionChanged, this, &MainWindow::on_player_positionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MainWindow::on_player_durationChanged);

    // Дополнительное подключение для перемещения по времени (scrubbing)
    connect(ui->horizontalSliderTime, &QSlider::sliderPressed, this, [this]() {
           m_isUserScrubbing = true;
       });
       connect(ui->horizontalSliderTime, &QSlider::sliderReleased, this, [this]() {
           if (m_player->duration() > 0) {
               // Преобразуем значение слайдера (0-1000) в миллисекунды
               qint64 position_ms = static_cast<qint64>(ui->horizontalSliderTime->value()) * m_player->duration() / ui->horizontalSliderTime->maximum();
               m_player->setPosition(position_ms);
           }
           m_isUserScrubbing = false;
       });
}

MainWindow::~MainWindow()
{
    delete m_player;
    delete ui;
}


void MainWindow::on_pushButtonAdd_clicked()
{
    QString file = QFileDialog::getOpenFileName
    (
      this,
      "Open file",
      "",
      "Audio files (*.mp3 *.flac);; MP-3 (*.mp3);; Flac (*.flac)"
    );
    if (!file.isEmpty()) {
            ui->labelFileName->setText(QString("File: ").append(QFileInfo(file).fileName())); // Отображаем только имя файла
            m_player->setMedia(QUrl::fromLocalFile(file));
            // m_player->play(); // Пока не будем автоматически играть после добавления
            ui->horizontalSliderTime->setEnabled(true); // Включаем слайдер времени
            // Сброс слайдера времени
            ui->horizontalSliderTime->setValue(0);
            ui->labelPosition->setText("00:00"); // Обновляем метку текущего времени
            ui->labelDuration->setText("00:00"); // Обновляем метку общего времени
        }
}


void MainWindow::on_pushButtonPlay_clicked()
{
    this->m_player->play();
}


void MainWindow::on_pushButtonPause_clicked()
{
   m_player->state() == QMediaPlayer::State::PausedState? m_player->play() :  this->m_player->pause();
}

void MainWindow::on_pushButtonStop_clicked()
{
    this->m_player->stop();
    // Сброс слайдера времени и меток времени при остановке
    //ui->horizontalSliderTime->setValue(0);
    //ui->labelTime->setText("00:00");
    //ui->labelDuration->setText("00:00");

}

void MainWindow::on_pushButtonMute_clicked()
{
    m_player->setMuted(!m_player->isMuted());
    ui->pushButtonMute->setIcon(style()->standardIcon(m_player->isMuted()?QStyle::SP_MediaVolumeMuted:QStyle::SP_MediaVolume));
    //if (m_player->isMuted())
   // {
      //  m_player->setMuted(false);
       // ui->pushButtonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
        // При разблокировке звука, можно восстановить громкость, если она была 0
             //   if (m_player->volume() == 0)
              //  {
                //    m_player->setVolume(70); // Или последнее сохраненное значение
                //    ui->horizontalSliderVolume->setValue(m_player->volume());
               //     ui->labelVolume->setText(QString("Volume: ").append(QString::number(m_player->volume())));
              //  }
 //   }
 //   else
   // {
    //    m_player->setMuted(true);
   //     ui->pushButtonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
   // }
}


void MainWindow::on_horizontalSliderVolume_valueChanged(int value)
{
    m_player->setVolume(value);
    ui->labelVolume->setText(QString("Volume: ").append(QString::number(value)));

    //m_player->setVolume(value);
    //ui->labelVolume->setText(QString("Volume: ").append(QString::number(value)));
    //if (m_player->isMuted() && value > 0)
        //{ // Если громкость стала больше 0, а Mute был включен, то выключаем Mute
           // m_player->setMuted(false);
            //ui->pushButtonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
       // } else if (value == 0)
        //{ // Если громкость стала 0, включаем Mute
           // m_player->setMuted(true);
           // ui->pushButtonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
       // }
}

// Реализация нового слота для обновления позиции
void MainWindow::on_player_positionChanged(qint64 position)
{
    this->ui->labelPosition->setText(QString(QTime::fromMSecsSinceStartOfDay(position).toString("hh:mm:ss")));
    this->ui->horizontalSliderTime->setValue(position);
    //if (!m_isUserScrubbing)
    //{ // Обновляем слайдер, только если пользователь не перетаскивает его
       // if (m_player->duration() > 0)
        //{
            // Преобразуем миллисекунды в значение для слайдера (0-1000)
            //int sliderValue = static_cast<int>(static_cast<double>(position) / m_player->duration() * ui->horizontalSliderTime->maximum());
           // ui->horizontalSliderTime->setValue(sliderValue);

            // Обновляем метку текущего времени
            //QTime time(0, 0, 0);
           // time = time.addMSecs(position);
           // ui->labelTime->setText(time.toString("mm:ss"));
       // } else
       // {
            // Если длительность неизвестна, устанавливаем 0
           // ui->horizontalSliderTime->setValue(0);
           // ui->labelTime->setText("00:00");
        //}
    //}
}

// Реализация нового слота для установки максимального значения слайдера
void MainWindow::on_player_durationChanged(qint64 duration)
{
    this->ui->horizontalSliderTime->setRange(0, duration);
    this->ui->labelDuration->setText(QTime::fromMSecsSinceStartOfDay(duration).toString("hh:mm:ss"));
    //if (duration > 0) {
       // ui->horizontalSliderTime->setRange(0, 1000); // Диапазон от 0 до 1000 для удобства масштабирования
       // ui->horizontalSliderTime->setEnabled(true);

        // Обновляем метку общего времени
       // QTime time(0, 0, 0);
       // time = time.addMSecs(duration);
       // ui->labelDuration->setText(time.toString("mm:ss"));
   // } else {
      //  ui->horizontalSliderTime->setRange(0, 100); // Временный диапазон, если длительность неизвестна
        //ui->horizontalSliderTime->setEnabled(false);
        //ui->labelDuration->setText("00:00");
   //}
}
