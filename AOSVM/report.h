#pragma once

struct report
{
private:
	clock_t timer;
	double seconds;

public:
	svm_model* model;
	report_predict r_predict;

	double train_time;
	double predict_time;

	std::vector<double> learning_rate;
	std::vector<double>* learning_class_rate; //detail for each class

	void start();
	double stop();
	void reset();
	double get_elapse();
	void report_predict(mydouble* y_test, mydouble* y_pred, int N);
	virtual void write_report(char* filename) {};

	virtual void print_report();
};