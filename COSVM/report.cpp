#include "stdafx.h"

void report::start()
{
	timer = clock();
}

double report::stop()
{
	seconds += (clock() - timer) / (double)CLOCKS_PER_SEC;
	return get_elapse();
}

void report::reset()
{
	seconds = 0;
}

double report::get_elapse()
{
	return seconds;
}

void report::report_predict(mydouble * y_test, mydouble * y_pred, int N)
{
	r_predict.model = model;
	r_predict.report(y_test, y_pred, N);
}

void report::print_report()
{
	printf("Acc Total: %f\n", r_predict.accuracy_total);
	printf("Acc: %f; Prec: %f\n", r_predict.accuracy_avg, r_predict.precision_avg);
	printf("Cls in detail: ");
	for (int m = 0; m < model->index_label->size(); m++)
		printf("%d ", r_predict.true_pop[m]);
	printf("\n");
	printf("Acc in detail: ");
	for (int m = 0; m < model->index_label->size(); m++)
		printf("%f ", r_predict.accuracy[m]);
	printf("\n");
	printf("Train: %f; Test: %f\n", train_time, predict_time);
}
