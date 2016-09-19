#include "stdafx.h"
#include <chrono>
#include <queue>

report * solve_cosvm(svm_problem * train_prob, const svm_parameter * param)
{
	cosvm_report* result = new cosvm_report();
	cosvm_model* model = new cosvm_model();
	result->model = model;

	svm_problem* full_prob = prob_formalise_unbal_bin(*train_prob, model->index_label, model->label_index, model->switch_label);
	svm_problem* prob = get_sub_problem(*full_prob, 1);
	model->prob = prob;

	int N = prob->l;

	//just in case dataset is not random
	std::vector<int> n_index;
	for (int n = 0; n < N; n++)
		n_index.push_back(n);
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	shuffle(n_index.begin(), n_index.end(), std::default_random_engine(seed));

	mydouble C = param->C;
	mydouble mu = param->mu;
	mydouble max_num_tolerance = (1.0 - C) * N;

	mydouble* center_alpha = new mydouble[N]; 
	int* center_index = new int[N];
	int center_l = 0;
	mydouble R = 0;

	std::vector<std::vector<mydouble>*> KInvert;
	std::vector<std::vector<mydouble>*> K;
	std::vector<mydouble> kappa;
	mydouble* kn = new mydouble[N];
	mydouble* KInvert_kn = new mydouble[N];
	mydouble* knT_KInvert = new mydouble[N];

	int nt = n_index[0];
	center_index[center_l] = nt;
	center_alpha[center_l] = 1;
	center_l++;
	R = 0; //not sure
	std::vector<mydouble>* KI_tmp = new std::vector<mydouble>();
	KI_tmp->push_back(1.0); //RBF kernel = 1.0 => invert is 1.0
	KInvert.push_back(KI_tmp);
	std::vector<mydouble>* K_tmp = new std::vector<mydouble>();
	K_tmp->push_back(1.0); //RBF kernel = 1.0
	K.push_back(K_tmp);
	kappa.push_back(1.0);

	std::priority_queue<mydouble> r_queue;

	for (int n = 1; n < N; n++)
	{
		nt = n_index[n];
		mydouble max_coh = -MYMAXDBL;
		for (int ci = 0; ci < center_l; ci++)
			kn[ci] = Kernel::k_function(prob->x[center_index[ci]], prob->x[nt], *param);
		for (int ci = 0; ci < center_l; ci++)
			if (max_coh < kn[ci])
				max_coh = kn[ci];
		if (max_coh > mu)
		{
			for (int ci = 0; ci < center_l; ci++)
				kappa[ci] = (kappa[ci] * n + kn[ci]) / (n+1);
			for (int c1 = 0; c1 < center_l; c1++)
			{
				KI_tmp = KInvert[c1];
				mydouble tmp = 0;
				for (int c2 = 0; c2 < center_l; c2++)
					tmp += (*KI_tmp)[c2] * kn[c2];
				KInvert_kn[c1] = tmp;
			}
			for (int ci = 0; ci < center_l; ci++)
				center_alpha[ci] = (n * center_alpha[ci] + KInvert_kn[ci]) / (n + 1);
			//Update R
			r_queue.empty();
			for (int c1 = 0; c1 < center_l; c1++)
			{
				K_tmp = K[c1];
				mydouble r_tmp = 1.0; //RBF kernel;
				for (int c2 = 0; c2 < center_l; c2++)
					r_tmp -= 2 * center_alpha[c2] * (*K_tmp)[c2];
				if (r_queue.size() < max_num_tolerance)
					r_queue.push(-r_tmp);
				else if (r_queue.top() > -r_tmp)
				{
					r_queue.push(-r_tmp);
					r_queue.pop();
				}
			}
			R = -r_queue.top();
		}
		else
		{
			//update K
			for (int ci = 0; ci < center_l; ci++)
				K[ci]->push_back(kn[ci]);
			K_tmp = new std::vector<mydouble>();
			for (int ci = 0; ci < center_l; ci++)
				K_tmp->push_back(kn[ci]);
			K_tmp->push_back(1.0); //RBF kernel;
			K.push_back(K_tmp);

			//update KInvert
			for (int c1 = 0; c1 < center_l; c1++)
			{
				KI_tmp = KInvert[c1];a
				mydouble tmp = 0;
				for (int c2 = 0; c2 < center_l; c2++)
					tmp += (*KI_tmp)[c2] * kn[c2];
				KInvert_kn[c1] = tmp;
			}

			for (int ci = 0; ci < center_l; ci++)
				knT_KInvert[ci] = 0;
			for (int c1 = 0; c1 < center_l; c1++)
			{
				mydouble knc1 = kn[c1];
				KI_tmp = KInvert[c1];
				for (int c2 = 0; c2 < center_l; c2++)
					knT_KInvert[c2] += knc1 * (*KI_tmp)[c2];
			}
			mydouble kn_K_kn = 0;
			for (int ci = 0; ci < center_l; ci++)
				kn_K_kn += kn[ci] * KInvert_kn[ci];
			mydouble c_tmp = 1.0 - kn_K_kn; //RBF kernel = 1
			for (int c1 = 0; c1 < center_l; c1++)
			{
				KI_tmp = KInvert[c1];
				for (int c2 = 0; c2 < center_l; c2++)
					(*KI_tmp)[c2] = (*KI_tmp)[c2] + KInvert_kn[c1] * knT_KInvert[c2] / c_tmp;
			}
			for (int ci = 0; ci < center_l; ci++)
				KInvert[ci]->push_back(-KInvert_kn[ci] / c_tmp);
			KI_tmp = new std::vector<mydouble>();
			for (int ci = 0; ci < center_l; ci++)
				KI_tmp->push_back(-knT_KInvert[ci] / c_tmp);
			KI_tmp->push_back(1.0 / c_tmp); //RBF kernel = 1
			KInvert.push_back(KI_tmp);

			//update kappa
			for (int ci = 0; ci < center_l; ci++)
				kappa[ci] = (kappa[ci] * n + kn[ci]) / (n + 1);
			kappa.push_back(1.0); //approximate not sure

			//update alpha
			mydouble alpha_scale = 0;
			for (int ci = 0; ci < center_l; ci++)
				alpha_scale += kn[ci] * center_alpha[ci];
			alpha_scale *= n;
			alpha_scale += (kn_K_kn - 1.0); //aproximate kappa_(n+1)
			for(int ci = 0; ci < center_l; ci++)
				center_alpha[ci] = (n * center_alpha[ci] + KInvert_kn[ci] - alpha_scale * KInvert_kn[ci] / c_tmp) / (n + 1);
			center_alpha[center_l] = -alpha_scale / ((n + 1) * c_tmp);

			//update center_index, l
			center_index[center_l] = nt;
			center_l++;

			//Update R
			r_queue.empty();
			for (int c1 = 0; c1 < center_l; c1++)
			{
				K_tmp = K[c1];
				mydouble r_tmp = 1.0; //RBF kernel;
				for (int c2 = 0; c2 < center_l; c2++)
					r_tmp -= 2 * center_alpha[c2] * (*K_tmp)[c2];
				if (r_queue.size() < max_num_tolerance)
					r_queue.push(-r_tmp);
				else if (r_queue.top() > -r_tmp)
				{
					r_queue.push(-r_tmp);
					r_queue.pop();
				}
			}
			R = -r_queue.top();
		}
	}

	model->center_alpha = center_alpha;
	model->center_index = center_index;
	model->center_l = center_l;
	model->R = R;

	model->param = *param;

	return result;
}

void cosvm_predict(report * report, const svm_problem * test_prob, mydouble *& predict)
{
	cosvm_model* model = (cosvm_model*)report->model;
	report->reset();
	report->start();

	mydouble* center_alpha = model->center_alpha;
	int* center_index = model->center_index;
	int center_l = model->center_l;

	predict = new mydouble[test_prob->l];

	for (int n = 0; n < test_prob->l; n++)
	{
		svm_node* x = test_prob->x[n];

		mydouble obj = 1.0; //RBF kernel = 1
		for (int bi = 0; bi < center_l; bi++)
			obj -= 2 * center_alpha[bi] * Kernel::k_function(model->prob->x[center_index[bi]], x, model->param);

		predict[n] = (obj < model->R) ? 1 : -1;
		if (model->switch_label)
			predict[n] = -predict[n];
	}

	report->predict_time = report->stop();
}
