#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QLabel>
#include <gdal_priv.h>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QLabel *imageLabel;

private:
    Ui::MainWindow *ui;
    void loadImage(const QString &filename);
    void displaySubImage(GDALDataset *dataset);
};

#endif // MAINWINDOW_H
