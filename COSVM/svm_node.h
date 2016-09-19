#pragma once

//index = 1
struct svm_node
{
	int index;
	mydouble value;

	svm_node()
	{
		index = 1;
		value = 0;
	}

	svm_node(int i, mydouble v)
	{
		index = i;
		value = v;
	}
};