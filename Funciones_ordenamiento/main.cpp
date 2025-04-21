#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>
#include <QString>
#include <vector>
#include <algorithm>

using namespace std;

// ==============================================
// DECLARACIONES FUNCIONES Y CODIGO
// ==============================================

enum TransformationType { XOR_OP, ROTATE_RIGHT_OP, ROTATE_LEFT_OP };

struct Transformation {
    TransformationType type;
    int bits;
};

const int MAX_BITS = 8;

// Funciones originales
unsigned char* loadPixels(QString input, int &width, int &height) {
    QImage imagen(input);
    if (imagen.isNull()) {
        cerr << "Error al cargar: " << input.toStdString() << endl;
        return nullptr;
    }
    imagen = imagen.convertToFormat(QImage::Format_RGB888);
    width = imagen.width();
    height = imagen.height();
    unsigned char* pixelData = new unsigned char[width * height * 3];
    for (int y = 0; y < height; ++y) {
        memcpy(pixelData + y * width * 3, imagen.scanLine(y), width * 3);
    }
    return pixelData;
}

bool exportImage(unsigned char* pixelData, int width, int height, QString archivoSalida) {
    QImage outputImage(width, height, QImage::Format_RGB888);
    for (int y = 0; y < height; ++y) {
        memcpy(outputImage.scanLine(y), pixelData + y * width * 3, width * 3);
    }
    return outputImage.save(archivoSalida, "BMP");
}

unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels) {
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cerr << "Error al abrir: " << nombreArchivo << endl;
        return nullptr;
    }
    archivo >> seed;
    n_pixels = 0;
    int r, g, b;
    while (archivo >> r >> g >> b) n_pixels++;

    archivo.close();
    archivo.open(nombreArchivo);
    archivo >> seed;

    unsigned int* data = new unsigned int[n_pixels * 3];
    for (int i = 0; i < n_pixels * 3; i += 3) {
        archivo >> data[i] >> data[i+1] >> data[i+2];
    }
    archivo.close();
    return data;
}

// Operaciones a nivel de bit
void applyXOR(unsigned char* image1, unsigned char* image2, int size);

unsigned char rotateRight(unsigned char value, int bits);

unsigned char rotateLeft(unsigned char value, int bits);

void applyRotation(unsigned char* image, int size, int bits, bool right);

// Verificación y reconstrucción
bool verifyMasking(unsigned char* image, unsigned char* mask,
                   int imgWidth, int imgHeight,
                   int maskWidth, int maskHeight,
                   int seed, unsigned int* maskingData) {
    int imgSize = imgWidth * imgHeight * 3;
    int maskSize = maskWidth * maskHeight * 3;

    for (int k = 0; k < maskSize; ++k) {
        int pos = (seed + k) % imgSize;
        unsigned char sum = image[pos] + mask[k % maskSize];
        if (sum != maskingData[k]) {
            return false;
        }
    }
    return true;
}


unsigned char* applyTransformationsInverse(unsigned char* finalImage, unsigned char* IM,
                                           const vector<Transformation> &transformations,
                                           int width, int height);

unsigned char* findCorrectReconstruction(unsigned char* finalImage, unsigned char* IM,
                                         unsigned char* mask, int width, int height,
                                         int maskWidth, int maskHeight,
                                         unsigned int** maskingDataArray, int* seeds,
                                         int numTransformations);

void applyXOR(unsigned char* img1, unsigned char* img2, int size) {
    for (int i = 0; i < size; ++i) {
        img1[i] ^= img2[i];
    }
}

unsigned char rotateRight(unsigned char value, int bits) {
    bits = bits % MAX_BITS;
    return (value >> bits) | (value << (MAX_BITS - bits));
}

unsigned char rotateLeft(unsigned char value, int bits) {
    bits = bits % MAX_BITS;
    return (value << bits) | (value >> (MAX_BITS - bits));
}

void applyRotation(unsigned char* img, int size, int bits, bool right) {
    for (int i = 0; i < size; ++i) {
        img[i] = right ? rotateRight(img[i], bits) : rotateLeft(img[i], bits);
    }
}
