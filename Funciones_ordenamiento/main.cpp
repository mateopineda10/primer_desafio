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

// Verificación enmascaramiento

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

// ==============================================
// RECONSTRUCCIÓN DE IMAGEN
// ==============================================

unsigned char* applyInverseTransformations(unsigned char* finalImage,
                                           unsigned char* IM,
                                           const Transformation* transformations,
                                           int numTransformations,
                                           int width, int height) {
    int size = width * height * 3;
    unsigned char* current = new unsigned char[size];
    memcpy(current, finalImage, size);

    // Aplicar en orden inverso
    for (int i = numTransformations - 1; i >= 0; --i) {
        switch (transformations[i].type) {
        case XOR_OP:
            applyXOR(current, IM, size);
            break;
        case ROTATE_RIGHT_OP:
            // Para revertir rotación derecha, aplicamos rotación izquierda
            applyRotation(current, size, transformations[i].bits, false);
            break;
        case ROTATE_LEFT_OP:
            // Para revertir rotación izquierda, aplicamos rotación derecha
            applyRotation(current, size, transformations[i].bits, true);
            break;
        }
    }
    return current;
}

void generatePossibleTransformations(Transformation* transforms, int& count) {
    // Generar todas las transformaciones posibles
    count = 0;

    // XOR
    transforms[count++] = {XOR_OP, 0};

    // Rotaciones derecha (1-8 bits)
    for (int bits = 1; bits <= MAX_BITS; ++bits) {
        transforms[count++] = {ROTATE_RIGHT_OP, bits};
    }

    // Rotaciones izquierda (1-8 bits)
    for (int bits = 1; bits <= MAX_BITS; ++bits) {
        transforms[count++] = {ROTATE_LEFT_OP, bits};
    }
}

//Verifica si una secuencia específica de transformaciones (en orden inverso)
// - puede reconstruir correctamente la imagen original.
//Comprueba si la imagen resultante después de aplicar las transformaciones inversas
// - coincide con los datos de enmascaramiento guardados en los archivos.

bool testTransformations(unsigned char* finalImage, unsigned char* IM,
                         unsigned char* mask, int width, int height,
                         int maskWidth, int maskHeight,
                         unsigned int** maskingDataArray, int* seeds,
                         int numTransformations,
                         const Transformation* candidateTransformations) {

    unsigned char* testImage = applyInverseTransformations(finalImage, IM,
                                                           candidateTransformations,
                                                           numTransformations,
                                                           width, height);

    bool valid = true;
    for (int i = 0; i < numTransformations; ++i) {
        if (!verifyMasking(testImage, mask, width, height,
                           maskWidth, maskHeight,
                           seeds[i], maskingDataArray[i])) {
            valid = false;
            break;
        }
    }

    delete[] testImage;
    return valid;
}

// ==============================================

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
