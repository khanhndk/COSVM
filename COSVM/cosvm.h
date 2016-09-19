#pragma once

struct cosvm_model : svm_model
{
	int switch_label;
	mydouble R;
	mydouble* center_alpha;
	int* center_index;
	int center_l;
};

struct cosvm_report : report
{

};

report* solve_cosvm(svm_problem *prob, const svm_parameter *param);

void cosvm_predict(report * report, const svm_problem * test_prob, mydouble *& predict);