#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadFileToPlaylist(const QString& filename);

private slots:
    void on_pushButtonAdd_clicked();

    void on_pushButtonPlay_clicked();

    void on_pushButtonPause_clicked();

    void on_pushButtonMute_clicked();

    void on_horizontalSliderVolume_valueChanged(int value);

    void on_pushButtonStop_clicked();

    // Новый слот для обновления времени проигрывания
    void on_player_positionChanged(qint64 position);
    // Новый слот для установки максимального значения слайдера времени
    void on_player_durationChanged(qint64 duration);


    void on_pushButtonShuffle_toggled(bool checked);

    void on_pushButtonRepeat_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    QMediaPlayer* m_player;
    bool m_isUserScrubbing = false;// Флаг, чтобы не реагировать на изменения слайдера во время его перетаскивания пользователем
    QMediaPlaylist* m_playlist;
    QStandardItemModel* m_playlist_model;
};
#endif // MAINWINDOW_H
