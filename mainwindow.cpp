#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <gdal_priv.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    imageLabel = new QLabel(this);
    setCentralWidget(imageLabel);

    QString fileName = QFileDialog::getOpenFileName(this, "Open Image File", "", "TIFF Images (*.tif *.tiff)");
    // QString fileName = "/home/bravo/Documents/sic/map/Natural_Earth_quick_start/packages/Natural_Earth_quick_start/50m_raster/NE1_50M_SR_W/NE1_50M_SR_W.tif";
    if (!fileName.isEmpty()) {
        GDALDataset *dataset = (GDALDataset *)GDALOpen(fileName.toUtf8().constData(), GA_ReadOnly);
        if (dataset == nullptr) {
            QMessageBox::critical(this, "Error", "Could not open image file!");
            return;
        }

        displaySubImage(dataset);

        GDALClose(dataset); // Закрываем набор данных
    }
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::displaySubImage(GDALDataset *dataset) {
    qDebug() << "Called displaySubImage"; // Это поможет отслеживать вызовы этого метода
    // Определяем и выводим информацию о формате изображения
    GDALDriver* driver = dataset->GetDriver();
    if (driver != nullptr) {
        const char* driverName = driver->GetDescription();
        qDebug() << "Driver name:" << driverName; // Выводим имя драйвера
    }

    // Получаем метаданные
    char **metadata = dataset->GetMetadata("");
    if (metadata != nullptr) {
        for (int i = 0; metadata[i] != nullptr; ++i) {
            qDebug() << metadata[i]; // Выводим все метаданные
        }
    }

    int width = dataset->GetRasterXSize();
    int height = dataset->GetRasterYSize();

    // Определяем центр изображения
    int startX = (width / 2) - 200; // 200 для выравнивания 400 на 400
    int startY = (height / 2) - 200;

    // Убедимся, что мы не выходим за пределы изображения
    startX = qMax(0, startX);
    startY = qMax(0, startY);

    // Чтение 400x400 области
    const int subWidth = 400;
    const int subHeight = 400;

    // Определяем количество каналов (п-band) в изображении
    int numBands = dataset->GetRasterCount();

    // Создаем буфер для каждого канала (RGB)
    std::vector<GByte> buffer(subWidth * subHeight * numBands); // RGB или Grayscale, в зависимости от числа каналов

    // Считываем данные
    for (int i = 0; i < numBands; ++i) {
        dataset->GetRasterBand(i + 1)->RasterIO(GF_Read, startX, startY, subWidth, subHeight,
                                                buffer.data() + i * subWidth * subHeight, // Смещение в буфере для текущего канала
                                                subWidth, subHeight, GDT_Byte, 0, 0);
    }

    // Создаем QImage из буфера
    QImage image(subWidth, subHeight, QImage::Format_RGB888);

    if (numBands == 1) {
        // Если только один канал (градации серого)
        image = QImage(subWidth, subHeight, QImage::Format_Grayscale8);
        for (int y = 0; y < subHeight; ++y) {
            for (int x = 0; x < subWidth; ++x) {
                int gray = buffer[(y * subWidth + x) * numBands + 0];
                image.setPixel(x, y, qRgb(gray, gray, gray)); // Установка в градациях серого
            }
        }
    } else if (numBands == 3) {
        // Если три канала (RGB)
        for (int y = 0; y < subHeight; ++y) {
            for (int x = 0; x < subWidth; ++x) {
                int r = buffer[(y * subWidth + x) * numBands + 0]; // Красный канал
                int g = buffer[(y * subWidth + x) * numBands + 1]; // Зеленый канал
                int b = buffer[(y * subWidth + x) * numBands + 2]; // Синий канал
                image.setPixel(x, y, qRgb(r, g, b)); // Установка пикселя
            }
        }
    } else if (numBands == 4) {
        // Если четыре канала (RGBA)
        image = QImage(subWidth, subHeight, QImage::Format_ARGB32);
        for (int y = 0; y < subHeight; ++y) {
            for (int x = 0; x < subWidth; ++x) {
                int r = buffer[(y * subWidth + x) * numBands + 0]; // Красный канал
                int g = buffer[(y * subWidth + x) * numBands + 1]; // Зеленый канал
                int b = buffer[(y * subWidth + x) * numBands + 2]; // Синий канал
                int a = buffer[(y * subWidth + x) * numBands + 3]; // Альфа-канал
                image.setPixel(x, y, qRgba(r, g, b, a)); // Установка пикселя с альфа
            }
        }


}

    // // Создаем буфер
    // std::vector<GByte> buffer(subWidth * subHeight * 3); // RGB

    // // Считываем данные
    // dataset->GetRasterBand(1)->RasterIO(GF_Read, startX, startY, subWidth, subHeight,
    //                                     buffer.data(), subWidth, subHeight, GDT_Byte, 0, 0);

    // // Создаем QImage из буфера
    // QImage image(reinterpret_cast<const uchar *>(buffer.data()), subWidth, subHeight, QImage::Format_RGB888);

    // Отображаем изображение в QLabel
    // QLabel *
    imageLabel = qobject_cast<QLabel*>(centralWidget());
    if (imageLabel) {
        imageLabel->clear(); // Очистить QLabel перед обновлением
        imageLabel->setPixmap(QPixmap::fromImage(image));
        imageLabel->setScaledContents(true);
        imageLabel->adjustSize();
    }
}
