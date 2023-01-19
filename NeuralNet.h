#pragma once
#include "Matrix.h"

class NeuralNet
{
public:
	NeuralNet(int nrInputsNodes, int nrHiddenNodes, int nrOutputNodes);
	~NeuralNet(){};

	void CalculateOutput(float* nnInput, float* nnOutput);

	void Mutate(float rate)
	{
		m_MatInput.Mutate(rate);
		m_MatHidden.Mutate(rate);
		m_MatOutput.Mutate(rate);
	}

private:
	int m_NrInputsNodes;
	int m_NrHiddenNodes;
	int m_NrOutputNodes;

	Matrix m_MatInput;
	Matrix m_MatHidden;
	Matrix m_MatOutput;
};