#include "mainwindow.h"
#include <QImage>
#include <QPixmap>
#include <QImageReader>
#include <QFileDialog>
#include <map>
#include <math.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    draw_widget = new QLabel("", this);

    hlayout = new QHBoxLayout();
    open_button = new QPushButton("Open Image", this);
    hlayout->addWidget(open_button);
    for (int i = 0; i < BUTTONS_COUNT; ++i) {
        buttons[i] = new QPushButton("Button " + QString::number(i + 1), this);
        hlayout->addWidget(buttons[i]);
    }

    vlayout = new QVBoxLayout();
    vlayout->addWidget(draw_widget);
    vlayout->addLayout(hlayout);

    QWidget *central = new QWidget(this);
    central->setLayout(vlayout);
    setCentralWidget(central);

    setFixedSize(800, 600);

    connect(open_button, SIGNAL(clicked(bool)), this, SLOT(openImage()));
    connect(buttons[0], SIGNAL(clicked(bool)), this, SLOT(detect()));
    connect(buttons[1], SIGNAL(clicked(bool)), this, SLOT(gaussianBlur()));

    alpha = 1;
    sigma = 1;

    buttons[1]->setEnabled(false);
    buttons[0]->setEnabled(false);

    buttons[0]->setText("Detect!");
    buttons[1]->setText("Gaussian Blur");

    srand(time(0));
}

MainWindow::~MainWindow()
{
    delete open_button;
    delete draw_widget;
    for (int i = 0; i < BUTTONS_COUNT; ++i)
        delete buttons[i];
    delete hlayout;
    delete vlayout;
    if (image) delete image;
}

void MainWindow::openImage() {
    QFileDialog dialog(this);
    dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));
    dialog.setViewMode(QFileDialog::Detail);
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "C:/",
                                                    tr("Images (*.png *.xpm *.jpg)"));
    if (filename == "") image = nullptr;

    QImageReader reader(filename);
    reader.setAutoTransform(true);
    QImage newImage = reader.read();
    if (newImage.isNull()) image = nullptr;
    image = new QImage(newImage);
    updateImage();

    buttons[1]->setEnabled(true);
    buttons[0]->setEnabled(true);
}

void MainWindow::updateImage() {
    if (!image) return;
    draw_widget->setPixmap(QPixmap::fromImage(*image));
}

// Makes image black and white in a right way
void MainWindow::toBinaryImage() {
    if (!image) return;
    int pix_brightness;
    int border = findBrightnessBorder();

    for (int i = 0; i < image->width(); ++i)
        for (int j = 0; j < image->height(); ++j) {
            pix_brightness = qGray(image->pixel(i, j));
            if (pix_brightness < border)
                image->setPixel(i, j, qRgb(0, 0, 0));
            else
                image->setPixel(i, j, qRgb(255, 255, 255));
        }
}

// Makes binarised image clearer
void MainWindow::dilation() {
    if (!image) return;
    box.set_size(3);
    initBuffImage();
    buffer_image2 = buffer_image;

    for (int i = shift; i < buffer_image.width() - shift; ++i)
        for (int j = shift; j < buffer_image.height() - shift; ++j)
            if(whitePixFound(i, j))
                buffer_image.setPixel(i, j, qRgb(255, 255, 255));

    extractResult();
}

void MainWindow::erosion() {
    if (!image) return;
    box.set_size(3);
    initBuffImage();
    buffer_image2 = buffer_image;

    for (int i = shift; i < buffer_image.width() - shift; ++i)
        for (int j = shift; j < buffer_image.height() - shift; ++j)
            if(blackPixFound(i, j))
                buffer_image.setPixel(i, j, qRgb(0, 0, 0));

    extractResult();
}

// Opening and closing just calls erosion and dilation in diferrent order
void MainWindow::closing() {
    if (!image) return;
    dilation();
    erosion();
}

void MainWindow::opening() {
    if (!image) return;
    erosion();
    dilation();
}

// Needes for binarisation
int MainWindow::getMaxOfBrightness() {
    int max = 0;
    int maxBrightnessValue;
    std::map<int, int>histogram;
    for (int it = 0; it < 256; ++it)
        histogram.insert( std::pair<int, int>(it, 0));

    for (int i = 0; i < image->width(); ++i)
        for (int j = 0; j < image->height(); ++j)
            histogram[qGray(image->pixel(i, j))]++;

    for (int it = 0; it < 256; ++it)
        if (histogram[it] > max) {
            max = histogram[it];
            maxBrightnessValue = it;
        }
    return maxBrightnessValue;
}

int MainWindow::findBrightnessBorder() {
    if (!image) return 0;
    int maxBrightnessValue = getMaxOfBrightness();
    int part = image->width()*image->height()*0.535;
    int pixels_count = 0;

    for (int it = maxBrightnessValue; it < 256; ++it){
        for (int i = 0; i < image->width(); ++i)
            for (int j = 0; j < image->height(); ++j) {
                if (qGray(image->pixel(i, j)) == it)
                    pixels_count++;

                if (pixels_count == part)
                    return it;
                if (it == 255)
                    return it;

            }
    }

    return -1;
}

// Filters
void MainWindow::medialFilter() {
    if (!image) return;
    box.set_size(3);
    initBuffImage();

    //now working with buffer image that filed up by black on edges
    for (int i = shift; i < buffer_image.width() - shift; ++i)
        for (int j = shift; j < buffer_image.height() - shift; ++j) {
            std::vector<QRgb> pixels;

            //adding neighboring pixels to the vector of pixels
            for (int m = 0; m < box.size(); ++m)
                for (int k = 0; k < box.size(); ++k)
                    pixels.push_back(buffer_image.pixel(i + m - shift, j + k - shift));


            std::sort(pixels.begin(), pixels.end()); //sorting pixels
            buffer_image.setPixel(i, j, pixels[4]); // taking mid pixel
        }

    extractResult(); //taking result from buffer image and putting it to image
    updateImage();
}

void MainWindow::gaussianBlur() {
    if (!image) return;
    box.set_size(5);
    initBuffImage();


    for (int i = 0; i < box.size(); ++i)
        for (int j = 0; j < box.size(); ++j)
            box.box[i][j] = pow(M_E, ( -((i - shift)*(i - shift) + (j - shift)*(j - shift)) / (2 * sigma * sigma)) ) /
                                        (2 * M_PI * sigma * sigma);         //normal distribution formula

    acceptBoxFilter();
    extractResult();
    updateImage();
}

// Functions for working with extended image
void MainWindow::initBuffImage() {
    shift = box.size() / 2;
    QImage _buffer_image(image->width() + 2*shift, image->height() + 2*shift, image->format());
    buffer_image = _buffer_image;

    for (int i = 0; i < buffer_image.width(); ++i)
        for (int j = 0; j < buffer_image.width(); ++j)
            if (!(i < shift || j < shift || i > buffer_image.width() - 1 - shift || j > buffer_image.height() - 1 - shift)) // if not edges
                buffer_image.setPixel(i, j, image->pixel(i - shift, j - shift)); //copying image pixels
}

void MainWindow::acceptBoxFilter() {

    for (int i = shift; i < buffer_image.width() - shift - 1; ++i)
        for (int j = shift; j < buffer_image.height() - shift - 1; ++j) {
            double red = 0, green = 0, blue = 0;

            for (int m = 0; m < box.size(); ++m)
                for (int k = 0; k < box.size(); ++k) {

                        red += qRed(buffer_image.pixel(i + m - shift, j + k - shift))*box.box[m][k];
                        green += qGreen(buffer_image.pixel(i + m - shift, j + k - shift))*box.box[m][k];
                        blue += qBlue(buffer_image.pixel(i + m - shift, j + k - shift))*box.box[m][k];

                }

            if (red > 255) red = 255;
            if (green > 255) green = 255;
            if (blue > 255) blue = 255;
            buffer_image.setPixel(i, j, qRgb(red, green, blue));
        }
}

void MainWindow::extractResult() {
    for (int i = 0; i < image->width(); ++i)
        for (int j = 0; j < image->height(); ++j)
            image->setPixel(i, j, buffer_image.pixel(i + shift, j + shift));
}

// Functions for erosin and delation functions
bool MainWindow::whitePixFound(const int& pix_row, const int& pix_column) {

    for(int i = pix_row - shift; i <= pix_row + shift; ++i)
        for(int j = pix_column - shift; j <= pix_column + shift; ++j)
            if (buffer_image2.pixel(i, j) == qRgb(255, 255, 255)) return true;

    return false;
}

bool MainWindow::blackPixFound(const int& pix_row, const int& pix_column) {
    for(int i = pix_row - shift; i <= pix_row + shift; ++i)
        for(int j = pix_column - shift; j <= pix_column + shift; ++j)
            if (buffer_image2.pixel(i, j) == qRgb(0, 0, 0)) return true;

    return false;
}


void MainWindow::initElongation(const int& row, const int& column) {
    if (image->pixel(row, column) != qRgb(255, 255, 254)) return;

    centralMoment20 += pow((column - xCentralMass), 2);
    centralMoment02 += pow((row - yCentralMass), 2);
    centralMoment11 += (column - xCentralMass)*(row - yCentralMass);

    elongation = (centralMoment20 + centralMoment02 - sqrt(pow((centralMoment20 - centralMoment02), 2) + 4*centralMoment11*centralMoment11)) /
                 (centralMoment20 + centralMoment02 + sqrt(pow((centralMoment20 - centralMoment02), 2) + 4*centralMoment11*centralMoment11));


    image->setPixel(row, column, qRgb(255, 255, 253));
    if (row + 1 != image->width()) initElongation(row + 1, column);
    if (column + 1 != image->height()) initElongation(row, column + 1);
    if (row - 1 != -1) initElongation(row - 1, column);
    if (column - 1 != -1) initElongation(row, column - 1);
}

void MainWindow::initCenterOfMass(const int& row, const int& column) {
    if (image->pixel(row, column) != qRgb(255, 255, 255)) return;
    square++;

    totalXCoordinates += column;
    totalYCoordinates += row;

    xCentralMass = totalXCoordinates / square;
    yCentralMass = totalYCoordinates / square;

    image->setPixel(row, column, qRgb(255, 255, 254));
    if (row + 1 != image->width()) initCenterOfMass(row + 1, column);
    if (column + 1 != image->height()) initCenterOfMass(row, column + 1);
    if (row - 1 != -1) initCenterOfMass(row - 1, column);
    if (column - 1 != -1) initCenterOfMass(row, column - 1);
}

void MainWindow::colorizeObject(const int& row, const int& column) {
        if (image->pixel(row, column) != qRgb(255, 255, 253)) return;

        if(elongation < 0.7 && square > 1000)  // Means it is spoon
            image->setPixel(row, column, qRgb(0, 191, 255));
        else if (square > 250)  // Means it is shugar cube
            image->setPixel(row, column, qRgb(139, 0, 255));
        else
            image->setPixel(row, column, qRgb(0, 0, 0));


        if (row + 1 != image->width()) colorizeObject(row + 1, column);
        if (column + 1 != image->height()) colorizeObject(row, column + 1);
        if (row - 1 != -1) colorizeObject(row - 1, column);
        if (column - 1 != -1) colorizeObject(row, column - 1);
}


void MainWindow::detect() {
    if (!image) return;

    // Need to make image black and white first
    toBinaryImage();

    // Then clear some artefacts
    medialFilter();

    // Opening and closing functions clears up image
    // There will be objects only after calling them
    opening();
    closing();

    for (int i = 0; i < image->width(); ++i)
        for (int j = 0; j < image->height(); ++j) {

            // Need to zero all variables because every new pix is new object
            totalXCoordinates = 0; totalYCoordinates = 0;
            elongation = 0; square = 0;
            centralMoment20 = 0; centralMoment02 = 0; centralMoment11 = 0;

            // Elongation is main criterion
            // To calculat it center of mass needed
            initCenterOfMass(i, j);
            initElongation(i, j);

            colorizeObject(i, j);
    }

    updateImage();
}
