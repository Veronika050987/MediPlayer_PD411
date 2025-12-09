#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStyle>
#include <QFileDialog>
#include <QUrl>//подключение файла
#include <QDateTime>// Для форматирования времени
#include <QTime>
#include <QPushButton> // Add this include
#include <QIcon>
#include <QHBoxLayout>
#include <QMediaPlaylist>

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

    // Shuffle Button
    ui->pushButtonShuffle = new QPushButton(this);
       //ui->pushButtonShuffle->setIcon(style()->standardIcon(QStyle::SP_MediaShuffle));
       ui->pushButtonShuffle->setCheckable(true);

       if (ui->horizontalLayout) { // Check if the layout exists
           ui->horizontalLayout->insertWidget(3, ui->pushButtonShuffle);
       } else
       {
           //QHBoxLayout *layout = new QHBoxLayout();
           //layout->addWidget(ui->pushButtonShuffle);
           //layout->addWidget(ui->pushButtonPrev);
           //layout->addWidget(ui->pushButtonPlay);
           //layout->addWidget(ui->pushButtonNext);
           //ui->tableViewPlaylist->setLayout(layout);

       }
       connect(ui->pushButtonShuffle, &QPushButton::toggled, this, &MainWindow::on_pushButtonShuffle_toggled);



   // Repeat Button
       ui->pushButtonRepeat = new QPushButton(this);
           ui->pushButtonRepeat->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
           ui->pushButtonRepeat->setCheckable(true);

           if (ui->horizontalLayout)
           {
             ui->horizontalLayout->insertWidget(4, ui->pushButtonRepeat);
           } else
           {

               if (ui->tableViewPlaylist->layout())
               {
                   ui->tableViewPlaylist->layout()->addWidget(ui->pushButtonRepeat);
               }

           }
           connect(ui->pushButtonRepeat, &QPushButton::toggled, this, &MainWindow::on_pushButtonRepeat_toggled);

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

     //         Playlist init
     m_playlist_model=new QStandardItemModel(this);
     this->ui->tableViewPlaylist->setModel(m_playlist_model);
     m_playlist_model->setHorizontalHeaderLabels(QStringList() << "Audio track" << "File path" << "Duration");
     this->ui->tableViewPlaylist->setEditTriggers(QAbstractItemView::NoEditTriggers);
     this->ui->tableViewPlaylist->setSelectionBehavior(QAbstractItemView::SelectRows);

     this->ui->tableViewPlaylist->hideColumn(1);
     int duration_width = 64;
     this->ui->tableViewPlaylist->setColumnWidth(2, duration_width);
     this->ui->tableViewPlaylist->setColumnWidth(0, this->ui->tableViewPlaylist->width()-duration_width*1.7);

     m_playlist = new QMediaPlaylist(m_player);
     m_player->setPlaylist(m_playlist);

     connect(this->ui->pushButtonPrev, &QPushButton::clicked, this->m_playlist, &QMediaPlaylist::previous);
         connect(this->ui->pushButtonNext, &QPushButton::clicked, this->m_playlist, &QMediaPlaylist::next);
         connect(this->m_playlist, &QMediaPlaylist::currentIndexChanged, this->ui->tableViewPlaylist, &QTableView::selectRow);
         connect(this->ui->tableViewPlaylist, &QTableView::doubleClicked,
                 [this](const QModelIndex& index){m_playlist->setCurrentIndex(index.row()); this->m_player->play();}
                 );
         connect(this->m_player, &QMediaPlayer::currentMediaChanged,
                 [this](const QMediaContent& media)
         {
             this->ui->labelFileName->setText(media.canonicalUrl().toString());
             this->setWindowTitle(this->ui->labelFileName->text().split('/').last());
         }
                 );
}

MainWindow::~MainWindow()
{
    delete ui->pushButtonShuffle;
    delete ui->pushButtonRepeat;
    delete m_playlist_model;
    delete m_playlist;
    delete m_player;
    delete ui;
}

void MainWindow::loadFileToPlaylist(const QString &filename)
{
    m_playlist->addMedia(QUrl(filename));
       QList<QStandardItem*> items;
       items.append(new QStandardItem(QDir(filename).dirName()));
       items.append(new QStandardItem(filename));
   //	QMediaPlayer player;
   //	m_duration_player.setMedia(QUrl(filename));
   //	m_duration_player.play();
   //	items.append(new QStandardItem(QTime::fromMSecsSinceStartOfDay(player.duration()).toString("mm:ss")));
   //	items.append(new QStandardItem(QString::number(m_duration_player.duration())));
   //	m_duration_player.pause();
       m_playlist_model->appendRow(items);
       //https://stackoverflow.com/questions/43156906/qmediaplayer-duration-returns-0-always
}

void MainWindow::on_pushButtonAdd_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames
                (
                    this,
                    "Open file",
                    "D:\\Users\\User\\Music\\",
                    "Audio files (*.mp3 *.flac *.flacc);; mp3 (*.mp3);; Flac (*.flac *.flacc)"
                );
        for(QString file:files)
        {
            loadFileToPlaylist(file);
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
}

void MainWindow::on_pushButtonMute_clicked()
{
    m_player->setMuted(!m_player->isMuted());
    ui->pushButtonMute->setIcon(style()->standardIcon(m_player->isMuted()?QStyle::SP_MediaVolumeMuted:QStyle::SP_MediaVolume));
}


void MainWindow::on_horizontalSliderVolume_valueChanged(int value)
{
    m_player->setVolume(value);
    ui->labelVolume->setText(QString("Volume: ").append(QString::number(value)));

}

// Реализация нового слота для обновления позиции
void MainWindow::on_player_positionChanged(qint64 position)
{
    this->ui->labelPosition->setText(QString(QTime::fromMSecsSinceStartOfDay(position).toString("hh:mm:ss")));
    this->ui->horizontalSliderTime->setValue(position);

}

// Реализация нового слота для установки максимального значения слайдера
void MainWindow::on_player_durationChanged(qint64 duration)
{
    this->ui->horizontalSliderTime->setRange(0, duration);
    this->ui->labelDuration->setText(QTime::fromMSecsSinceStartOfDay(duration).toString("hh:mm:ss"));

}


void MainWindow::on_pushButtonShuffle_toggled(bool checked)
{
   // m_playlist->setShuffle(checked);
}


void MainWindow::on_pushButtonRepeat_toggled(bool checked)
{
    if (checked) {
            m_playlist->setPlaybackMode(QMediaPlaylist::Loop);
        } else {
            m_playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        }
}

