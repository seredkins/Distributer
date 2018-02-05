#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>

#include "boxfilter.h"

namespace Ui {
    class MainWindow;
}



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void sharpness();
    void updateImage();

private:
    static const int BUTTONS_COUNT = 2;
    QLabel* draw_widget;
    QHBoxLayout *hlayout;
    QVBoxLayout *vlayout;
    QPushButton *open_button;
    QPushButton *buttons[BUTTONS_COUNT];
    QImage* image;
    QImage buffer_image;
    BoxFilter box;

    int totalXCoordinates = 0, totalYCoordinates = 0, square = 0,
        centralMoment20 = 0, centralMoment02 = 0, centralMoment11 = 0,
        xCentralMass = 0, yCentralMass = 0;
    double elongation = 0;

    int shift;

    // Makes image black and white in a right way
    void toBinaryImage();

    // Makes binarised image clearer
    void erosion();
    void dilation();

    // Opening and closing just calls erosion and dilation in diferrent order
    void closing();
    void opening();

    // Functions for working with extended image
    void initBuffImage();
    void acceptBoxFilter();
    void extractResult();

    // Initialisers of values for elongation
    void initElongation(const int&, const int&);
    void initCenterOfMass(const int&, const int&);

    // Needes for binarisation
    int findBrightnessBorder();
    int getMaxOfBrightness();

    // Functions for erosin and delation functions
    bool whitePixFound(const QImage&, const int&, const int&);
    bool blackPixFound(const QImage&, const int&, const int&);

    void colorizeObject(const int&, const int&);

    void medialFilter();



private slots:
    void openImage();
    void distribute();
    void gaussianBlur(const int&);
};

#endif // MAINWINDOW_H
