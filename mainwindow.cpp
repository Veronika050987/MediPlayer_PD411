#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "SoundWaveWidget.h"
#include <QStyle>
#include <QFileDialog>
#include <QUrl>//подключение файла
#include <QDateTime>// Для форматирования времени
#include <QTime>
#include <QDir>
#include <QFileDialog>

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
    ui->pushButtonDir->setIcon(style()->standardIcon(QStyle::SP_DirOpenIcon));
    ui->pushButtonMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    //ui->pushButtonShuffle->setIcon(style()->standardIcon(QStyle::SP_MediaShuffle));
    ui->pushButtonLoop->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));

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

     //         Playlist init
     m_playlist_model=new QStandardItemModel(this);
     initPlayList();
//     this->ui->tableViewPlaylist->setModel(m_playlist_model);
//     m_playlist_model->setHorizontalHeaderLabels(QStringList() << "Audio track" << "File path" << "Duration");
//     this->ui->tableViewPlaylist->setEditTriggers(QAbstractItemView::NoEditTriggers);
//     this->ui->tableViewPlaylist->setSelectionBehavior(QAbstractItemView::SelectRows);

//     this->ui->tableViewPlaylist->hideColumn(1);
//     int duration_width = 64;
//     this->ui->tableViewPlaylist->setColumnWidth(2, duration_width);
//     this->ui->tableViewPlaylist->setColumnWidth(0, this->ui->tableViewPlaylist->width()-duration_width*1.7);

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

         shuffle = false;
         loop = false;

         loadPlaylist();

         connect(qApp, &QCoreApplication::aboutToQuit, this, &MainWindow::savePlaylist);
         //connect(this->ui->pushButtonClr, &QPushButton::clicked, this->m_playlist, &QMediaPlaylist::clear);
         //connect(this->ui->pushButtonClr, &QPushButton::clicked, this->m_playlist_model, &QStandardItemModel::clear);
         //connect(ui->pushButtonDir, &QPushButton::clicked, this, &MainWindow::on_pushButtonDir_clicked);

         //m_soundWaveWidget = new SoundWaveWidget(this);
         //ui->setupUi(this)->addWidget(m_soundWaveWidget, 3, 0, 1, 4);
         m_soundWaveWidget = ui->soundWaveWidget;

         if (m_soundWaveWidget)
         {
            m_soundWaveWidget->setVolumeLevel(m_player->volume() / 100.0);
         }
}

MainWindow::~MainWindow()
{
    delete m_playlist_model;
    delete m_playlist;
    delete m_player;
    delete ui;
}

void MainWindow::initPlayList()
{
    this->ui->tableViewPlaylist->setModel(m_playlist_model);
    m_playlist_model->setHorizontalHeaderLabels(QStringList() << "Audio track" << "File path" << "Duration");
    this->ui->tableViewPlaylist->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->tableViewPlaylist->setSelectionBehavior(QAbstractItemView::SelectRows);

    this->ui->tableViewPlaylist->hideColumn(1);
    int duration_width = 64;
    this->ui->tableViewPlaylist->setColumnWidth(2, duration_width);
    this->ui->tableViewPlaylist->setColumnWidth(0, this->ui->tableViewPlaylist->width()-duration_width*1.7);
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

    m_soundWaveWidget->setVolumeLevel(value/100.0);
}

// Реализация нового слота для обновления позиции
void MainWindow::on_player_positionChanged(qint64 position)
{
    this->ui->labelPosition->setText(QString(QTime::fromMSecsSinceStartOfDay(position).toString("hh:mm:ss")));
    //this->ui->horizontalSliderTime->setValue(position);

    // Обновляем слайдер времени
        if (m_player->duration() > 0) {
            // Если слайдер time не имеет максимального значения duration,
            // то его установка может вызвать проблемы.
            // Если ui->horizontalSliderTime настроен на диапазон 0 до duration,
            // то при ручном скраббинге нам нужно его обходить.
            if (!m_isUserScrubbing) {
                // Здесь мы устанавливаем значение слайдера в соответствии с позицией
                // *** ВНИМАНИЕ: Если максимальное значение слайдера 0, это вызовет сбой! ***
                if (ui->horizontalSliderTime->maximum() > 0) {
                     ui->horizontalSliderTime->setValue(position);
                }
            }
        }

        // --- ОБНОВЛЕНИЕ ВИЗУАЛИЗАТОРА АКТИВНОСТИ ---
        if (m_player->state() == QMediaPlayer::PlayingState) {
            // При воспроизведении показываем небольшую активность,
            // основанную на громкости (для имитации VU Meter)
            double activity = 0.1 + (0.8 * (m_player->volume() / 100.0));
            m_soundWaveWidget->setVolumeLevel(activity);
        } else if (!m_isUserScrubbing) {
            // Если остановлено или на паузе, плавно затухаем
            m_soundWaveWidget->setVolumeLevel(0.0);
        }

}

// Реализация нового слота для установки максимального значения слайдера
void MainWindow::on_player_durationChanged(qint64 duration)
{
    this->ui->horizontalSliderTime->setRange(0, duration);
    this->ui->labelDuration->setText(QTime::fromMSecsSinceStartOfDay(duration).toString("hh:mm:ss"));

}

void MainWindow::on_pushButtonShuffle_clicked()
{
    shuffle = !shuffle;
    this->ui->pushButtonShuffle->setCheckable(true);
    this->m_playlist->setPlaybackMode(shuffle ? QMediaPlaylist::PlaybackMode::Random : QMediaPlaylist::PlaybackMode::Sequential);
    this->ui->pushButtonShuffle->setChecked(shuffle);
}

void MainWindow::on_pushButtonLoop_clicked()
{
    loop = !loop;
    this->ui->pushButtonLoop->setCheckable(true);
    this->m_playlist->setPlaybackMode(loop ? QMediaPlaylist::PlaybackMode::Loop : QMediaPlaylist::Sequential);
    this->ui->pushButtonLoop->setChecked(loop);
}

void MainWindow::on_pushButtonDel_clicked()
{
//    QItemSelectionModel* selection = ui->tableViewPlaylist->selectionModel();
//    QModelIndexList indexes = selection->selectedRows();
//    for(QModelIndex i : indexes)
//    {
//        m_playlist_model->removeRow(i.row());
//        m_playlist->removeMedia(i.row());
//    }
    QItemSelectionModel* selection = nullptr;
    do
    {
        selection = ui->tableViewPlaylist->selectionModel();
//        if(selection==nullptr)break;
        QModelIndexList indexes = selection->selectedRows();
        if(selection->selectedRows().count()>0)
        {
            m_playlist_model->removeRow(indexes.first().row());
            m_playlist->removeMedia(indexes.first().row());
        }
    }while(selection->selectedRows().count());
}

void MainWindow::on_pushButtonClr_clicked()
{
    m_playlist->clear();
    m_playlist_model->clear();
    initPlayList();
}


void MainWindow::on_pushButtonDir_clicked()
{
    // Выбор директории
    QString dirPath = QFileDialog::getExistingDirectory(this,
                                                        tr("Select directory"),
                                                        QDir::homePath(),
                                                        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dirPath.isEmpty())
    {
        return;
    }
    // Настройка фильтра для поиска аудиофайлов
    QDir directory(dirPath);
    QStringList nameFilters;
    nameFilters << "*.mp3" << "*.flac";

    // Получение списка файлов, соответствующих фильтрам
    QFileInfoList fileList = directory.entryInfoList(nameFilters, QDir::Files);

    if(fileList.isEmpty())
    {
        qDebug() << "No audio files found in directory: " << dirPath;
        return;
    }

    // Добавление каждого найденного файла в плейлист
    for (const QFileInfo &fileInfo : fileList)
    {
        QString filePath = fileInfo.absoluteFilePath();
        loadFileToPlaylist(filePath);
    }
}

void MainWindow::savePlaylist()
{
    QSettings settings("MyCompany", "AudioPlayer");
    settings.beginWriteArray("Playlist");

    for(int i=0; i<m_playlist->mediaCount(); i++)
    {
        QMediaContent content = m_playlist->media(i);

        settings.setArrayIndex(i);
        settings.setValue("path", content.canonicalUrl().toString());
    }
    settings.endArray();

    if (m_playlist->currentIndex()>=0)
    {
        settings.setValue("currentIndex", m_playlist->currentIndex());
    }
    settings.sync();
}

void MainWindow::loadPlaylist()
{
    QSettings settings("MyCompany", "AudioPlayer");
        int size = settings.beginReadArray("Playlist");

        for (int i = 0; i < size; ++i)
        {
            settings.setArrayIndex(i);
            QString urlString = settings.value("path").toString();

            QUrl url = QUrl::fromUserInput(urlString);

            // Проверка существования файла и валидности URL
            if (url.isValid() && QFileInfo(url.toLocalFile()).exists())
            {

                m_playlist->addMedia(url);

                QList<QStandardItem*> items;
                QFileInfo fileInfo(url.toLocalFile());

                items.append(new QStandardItem(fileInfo.baseName()));
                items.append(new QStandardItem(url.toString()));

                items.append(new QStandardItem("--:--")); // Длительность

                m_playlist_model->appendRow(items);

            }
            else
            {
                qWarning() << "Skipping missing file during playlist restoration:" << urlString;
            }
        }
        settings.endArray();

        // 4. Восстановление текущего индекса
        int currentIndex = settings.value("currentIndex", -1).toInt();
        if (currentIndex >= 0 && currentIndex < m_playlist->mediaCount())
        {
            m_playlist->setCurrentIndex(currentIndex);

            // Обновление метки текущего файла на экране
            const QMediaContent& media = m_playlist->currentMedia();

            // **БЕЗОПАСНАЯ ПРОВЕРКА, ИСПОЛЬЗУЯ canonicalUrl() и проверяя, что результат не пуст**
            QUrl currentUrl = media.canonicalUrl();

            if (!currentUrl.isEmpty())
            {
                this->ui->labelFileName->setText(currentUrl.fileName());
                this->setWindowTitle(currentUrl.fileName());
            }
        }
}
