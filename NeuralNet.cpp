#include "pch.h"
#include "NeuralNet.h"
#include <iostream>

NeuralNet::NeuralNet(int nrInputsNodes, int nrHiddenNodes, int nrOutputNodes)
	: m_NrInputsNodes(nrInputsNodes),
	m_NrHiddenNodes(nrHiddenNodes),
	m_NrOutputNodes(nrOutputNodes),
	m_MatInput{ Matrix(m_NrHiddenNodes, m_NrInputsNodes + 1) },
	m_MatHidden{ Matrix(m_NrHiddenNodes, m_NrHiddenNodes + 1) },
	m_MatOutput{ Matrix(m_NrOutputNodes, m_NrHiddenNodes + 1) }
{
	m_MatInput.RandomizeData();
	m_MatHidden.RandomizeData();
	m_MatOutput.RandomizeData();
}

void NeuralNet::CalculateOutput(float* nnInput, float* nnOutput)
{
    // Create a 1 row matrix from the input array
    Matrix inputs = Matrix(m_NrInputsNodes, 1);
    for (int i = 0; i < m_NrInputsNodes; i++)
    {
        inputs.m_Data[i][0] = nnInput[i];
    }

    // Add bias weight
    Matrix inputsBias = inputs.AddBiasWeight();

    // Apply weights to the inputs
    Matrix hiddenInputs = m_MatInput.Dot(inputsBias);
    Matrix hiddenOutputs = hiddenInputs.Activate();

    // Add bias weight
    Matrix hiddenOutputsBias = hiddenOutputs.AddBiasWeight();

    // Apply weights to hidden layer
    Matrix hiddenInputs2 = m_MatHidden.Dot(hiddenOutputsBias);
    Matrix hiddenOutputs2 = hiddenInputs2.Activate();
    Matrix hiddenOutputsBias2 = hiddenOutputs2.AddBiasWeight();

    // Apply weights to output layer
    Matrix outputInputs = m_MatOutput.Dot(hiddenOutputsBias2);
    Matrix outputs = outputInputs.Activate();

    // Return the output data as an array
    for (int i{}; i < m_NrOutputNodes; i++)
    {
        nnOutput[i] = outputs.m_Data[i][0];
    }
}