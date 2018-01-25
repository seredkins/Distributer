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
    QImage buffer_image, buffer_image2;
    BoxFilter box;

    int y_max;
    int shift, sigma, alpha;

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
    void initElongation(const int&, const int&,
                        const int&, const int&,
                        int&, int&, int&, double&);

    void initCenterOfMass(const int&, const
                          int&, int&, int&,
                          int&, int&, int&);

    // Needes for binarisation
    int findBrightnessBorder();
    int getMaxOfBrightness();

    // Functions for erosin and delation functions
    bool whitePixFound(const int&, const int&);
    bool blackPixFound(const int&, const int&);

    void colorizeObject(const int&, const int&,
                        const int&, const int&);

    void medialFilter();

private slots:
    void openImage();
    void detect();
    void gaussianBlur();
};

#endif // MAINWINDOW_H
