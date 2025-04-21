#include <iostream>

using namespace std;



int main()
    // Función para realizar XOR entre dos imágenes
    void applyXOR(unsigned char* image1, unsigned char* image2, int width, int height) {
    for (int i = 0; i < width * height * 3; i++) {
        image1[i] = image1[i] ^ image2[i];
    }
}

// Función para rotar bits a la derecha
unsigned char rotateRight(unsigned char value, int bits) {
    bits = bits % 8;
    return (value >> bits) | (value << (8 - bits));
}

// Función para rotar bits a la izquierda (necesaria para revertir)
unsigned char rotateLeft(unsigned char value, int bits) {
    bits = bits % 8;
    return (value << bits) | (value >> (8 - bits));
}

// Aplicar rotación a toda una imagen
void applyRotation(unsigned char* image, int width, int height, int bits, bool right) {
    for (int i = 0; i < width * height * 3; i++) {
        if (right) {
            image[i] = rotateRight(image[i], bits);
        } else {
            image[i] = rotateLeft(image[i], bits);
        }
    }
}
{
    bool verifyMasking(unsigned char* image, unsigned char* mask, int width, int height,
                       int maskWidth, int maskHeight, int seed, unsigned int* maskingData) {
        int maskSize = maskWidth * maskHeight * 3;
        int imageSize = width * height * 3;

        for (int k = 0; k < maskSize; k++) {
            int pos = (seed + k) % imageSize;
            unsigned char sum = image[pos] + mask[k % (maskWidth * maskHeight * 3)];

            if (sum != maskingData[k]) {
                return false;
            }
        }
        return true;
    }
}
