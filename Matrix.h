#pragma once
#include <vector>

class Matrix
{
public:
    Matrix(int nrRows, int nrCols);
    void Print();
    void RandomizeData();

    Matrix AddBiasWeight();
    Matrix Dot(const Matrix& other);
    void Mutate(float rate);

    Matrix Activate();

    float Sigmoid(float x);

    std::vector<std::vector<float>> m_Data;

private:
    int m_nrRows;
    int m_nrCols;
};