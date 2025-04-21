#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>
#include <QString>
#include <vector>
#include <algorithm>

using namespace std;

// ==============================================
// DECLARACIONES
// ==============================================

enum TransformationType { XOR_OP, ROTATE_RIGHT_OP, ROTATE_LEFT_OP };

struct Transformation {
    TransformationType type;
    int bits;
};

// Funciones originales
unsigned char* loadPixels(QString input, int &width, int &height);
bool exportImage(unsigned char* pixelData, int width, int height, QString archivoSalida);
unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);

// Operaciones a nivel de bit
void applyXOR(unsigned char* image1, unsigned char* image2, int size);
unsigned char rotateRight(unsigned char value, int bits);
unsigned char rotateLeft(unsigned char value, int bits);
void applyRotation(unsigned char* image, int size, int bits, bool right);

// Verificación y reconstrucción
bool verifyMasking(unsigned char* image, unsigned char* mask, int width, int height,
                   int maskWidth, int maskHeight, int seed, unsigned int* maskingData);

unsigned char* applyTransformationsInverse(unsigned char* finalImage, unsigned char* IM,
                                           const vector<Transformation> &transformations,
                                           int width, int height);

unsigned char* findCorrectReconstruction(unsigned char* finalImage, unsigned char* IM,
                                         unsigned char* mask, int width, int height,
                                         int maskWidth, int maskHeight,
                                         unsigned int** maskingDataArray, int* seeds,
                                         int numTransformations);

