//
//  main.cpp
//  Embedded_CDMA
//
//  Created by Greg on 10.01.17.
//  Copyright © 2017 Greg. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <math.h>

using namespace std;

void readFile(string& fileName, string& fileContent) {
    ifstream file(fileName);
    if (file.is_open())
    {
        // text files only contain one line
        getline(file, fileContent);
        file.close();
    }
    else cout << "Unable to open file";
}

vector<int> parseLineToVector(string& line) {
    vector<int> sequence;
    stringstream lineStream(line);
    int number;
    
    while(lineStream >> number)
    {
        sequence.push_back(number);
    }
    
    return sequence;
}

vector<int> getSignal(string& file){
    string content;
    readFile(file, content);
    return parseLineToVector(content);
}

void shiftSequenceByOne(int sequence[10]) {
    int temp = sequence[9];
    for(int i = 9; i > 0; i--){
        sequence[i] = sequence[i - 1];
    }
    sequence[0] = temp;
}

void createGoldCodeForSat(int goldCode[1023], int x, int y){
    int sequence1[10] = {1,1,1,1,1,1,1,1,1,1};
    int sequence2[10] = {1,1,1,1,1,1,1,1,1,1};
    
    int indexCounter = 0;
    while(indexCounter < 1023) {
        int temp2 = sequence2[x - 1] ^ sequence2[y - 1];
        int new2 = sequence2[9] ^ sequence2[8] ^ sequence2[7] ^ sequence2[5] ^ sequence2[2] ^ sequence2[1];
        int new1 = sequence1[9] ^ sequence1[2];
        
        goldCode[indexCounter] = (temp2 ^ sequence1[9]) == 0 ? -1 : 1;
        indexCounter++;
        
        shiftSequenceByOne(sequence1);
        shiftSequenceByOne(sequence2);
        sequence2[0] = new2;
        sequence1[0] = new1;
    }
}

void createGoldCodes(int goldCodes[24][1023]) {
    createGoldCodeForSat(goldCodes[0],  2, 6);
    createGoldCodeForSat(goldCodes[1],  3, 7);
    createGoldCodeForSat(goldCodes[2],  4, 8);
    createGoldCodeForSat(goldCodes[3],  5, 9);
    createGoldCodeForSat(goldCodes[4],  1, 9);
    createGoldCodeForSat(goldCodes[5],  2, 10);
    createGoldCodeForSat(goldCodes[6],  1, 8);
    createGoldCodeForSat(goldCodes[7],  2, 9);
    createGoldCodeForSat(goldCodes[8],  3, 10);
    createGoldCodeForSat(goldCodes[9],  2, 3);
    createGoldCodeForSat(goldCodes[10], 3, 4);
    createGoldCodeForSat(goldCodes[11], 5, 6);
    createGoldCodeForSat(goldCodes[12], 6, 7);
    createGoldCodeForSat(goldCodes[13], 7, 8);
    createGoldCodeForSat(goldCodes[14], 8, 9);
    createGoldCodeForSat(goldCodes[15], 9, 10);
    createGoldCodeForSat(goldCodes[16], 1, 4);
    createGoldCodeForSat(goldCodes[17], 2, 5);
    createGoldCodeForSat(goldCodes[18], 3, 6);
    createGoldCodeForSat(goldCodes[19], 4, 7);
    createGoldCodeForSat(goldCodes[20], 5, 8);
    createGoldCodeForSat(goldCodes[21], 6, 9);
    createGoldCodeForSat(goldCodes[22], 1, 3);
    createGoldCodeForSat(goldCodes[23], 4, 6);
}

float calculateScalar(vector<int> signal, int goldCode[1023], int delta) {
    float skalar = 0;
    for(int i = 0; i < 1023; i++) {
        skalar += signal[i] * goldCode[(i + delta) % 1023] / 1.0;
    }
    return skalar;
}

void interpretSignal(vector<int> signal, int goldCodes[24][1023]) {
    // calculated crosscorrelation values with formular for even register length
    // Peak = Rauschwert eines anderen Satelliten
    // deshalb 3 mal den rauschwert abziehen
    int numberOfInterferingSatellites = 3;
    float maxNoiceValue = 65.0;
    float upperPeak = 1023 -  numberOfInterferingSatellites * maxNoiceValue;
    float lowerPeak = -1023 + numberOfInterferingSatellites * maxNoiceValue;
    
    for(int satCounter = 0; satCounter < 24; satCounter++) {
        for(int delta = 0; delta < 1024; delta++) {
            // Das skalarprodukt wird nicht normalisiert. D.h. für eine 1 ist das skalar gleich 1023
            // und für eine 0 ist das skalar = -1023. Allerdings nur im idealfall.
            // Die anderen Satelliten stören das signal aber nur maximal mit dem wert,
            // der über die kreuzkorrelation berechnet werden kann. Das heißt wir suchen nach
            // einem Signal, dass in dem Bereich für 0 (-1023) bzw 1 (1023) plus/minus dem Rauschen
            // der anderen Satteliten liegt.
            float scalar = calculateScalar(signal, goldCodes[satCounter], delta);
            if(scalar >= upperPeak || scalar <= lowerPeak) {
                int bit = (scalar >= upperPeak) ? 1 : 0;
                cout << "Satellit " << satCounter + 1 << " hat folgedes Bit gesendet: " << bit << " (Delta: " << delta << ")" << endl;
                break;
            }
        }
    }
    
}

int main(int argc, const char * argv[]) {
    if(argc < 2)
    {
        cout << "please specify a file!\n Usage: program-name <file-location>" << endl;
        return 0;
    }
    
    string fileName = argv[1];
    vector<int> signal = getSignal(fileName);
    for (auto i = signal.begin(); i != signal.end(); ++i)
      cout << *i << ' ';
    cout << "\n-------\n";
    
    int goldCodes[24][1023];
    createGoldCodes(goldCodes);
    for(int i = 0; i < 1023; i++)
        cout << goldCodes[23][i] << ", ";
    cout << endl;
    
    interpretSignal(signal, goldCodes);
    
    return 0;
}
