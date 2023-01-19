#include "pch.h"
#include "Matrix.h"
#include <iostream>

Matrix::Matrix(int nrRows, int nrCols)
    : m_nrRows{ nrRows },
      m_nrCols{ nrCols }
{
    
    m_Data.resize(nrRows);
    for (std::vector<float>& col : m_Data)
    {
        col.resize(nrCols);
    }

    //for (std::vector<float>& col : m_Data)
    //{
    //    for (int i{}; i < col.size(); i++)
    //    {
    //        std::cout << "[" << col[i] << "], ";
    //    }
    //    std::cout << std::endl;
    //}
    //std::cout << std::endl << std::endl;
}

void Matrix::Print()
{
    for (std::vector<float>& col : m_Data)
    {
        for (int i{}; i < col.size(); i++)
        {
            std::cout << "[" << col[i] << "], ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl << std::endl;
}

void Matrix::RandomizeData()
{
    for (std::vector<float>& col : m_Data)
    {
        for (int i{}; i < col.size(); i++)
        {
            col[i] = -1 + (std::rand() % 2000) / 1000.f;
            //std::cout << "[" << col[i] << "], ";
        }
        //std::cout << std::endl;
    }
   // std::cout << std::endl << std::endl;
}

Matrix Matrix::AddBiasWeight()
{
    Matrix mat = Matrix(m_nrRows + 1, 1);
    for (int i = 0; i < m_nrRows; i++)
    {
        mat.m_Data[i][0] = m_Data[i][0];
    }
    mat.m_Data[m_nrRows][0] = 1;
    return mat;
}

Matrix Matrix::Dot(const Matrix& other)
{
    Matrix mat = Matrix(m_nrRows, other.m_nrCols);
    if (m_nrCols == other.m_nrRows)
    {
        for (int i = 0; i < m_nrRows; i++)
        {
            for (int j = 0; j < other.m_nrCols; j++)
            {
                float dotSum = 0;
                for (int k = 0; k < m_nrCols; k++)
                {
                    dotSum += m_Data[i][k] * other.m_Data[k][j];
                }
                mat.m_Data[i][j] = dotSum;
            }
        }
    }

    return mat;
}

float Matrix::Sigmoid(float x)
{
    float y = 1.f / (1.f + std::pow((float)std::exp(1), -x));
    return y;
}

Matrix Matrix::Activate()
{
    Matrix mat = Matrix(m_nrRows, m_nrCols);
    for (int i = 0; i < m_nrRows; i++)
    {
        for (int j = 0; j < m_nrCols; j++)
        {
            mat.m_Data[i][j] = Sigmoid(m_Data[i][j]);
        }
    }

    return mat;
}

void Matrix::Mutate(float rate)
{
    for (int i = 0; i < m_nrRows; i++)
    {
        for (int j = 0; j < m_nrCols; j++)
        {
            float rand = (std::rand() % 1000) / 1000.f;
            if (rand < rate)
            {
                m_Data[i][j] += -1 + (std::rand() % 2000) / 1000.f; // Add a random value between -1 and 1

                // Clamp the boundaries to -1 and 1
                if (m_Data[i][j] <  -1) m_Data[i][j] = -1;
                else if (m_Data[i][j] > 1) m_Data[i][j] = 1;
            }
        }
    }
}